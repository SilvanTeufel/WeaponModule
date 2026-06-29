// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Abilities/CrowdControlAbility.h"
#include "Components/WeaponComponent.h"
#include "Components/CrowdControlStateComponent.h"
#include "Actors/CrowdControlMarker.h"
#include "Characters/Unit/UnitBase.h"
#include "Characters/Unit/MassUnitBase.h" // SwitchEntityTagByState
#include "Core/UnitData.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Mass/UnitMassTag.h"   // FMassAIStateFragment (read-back for debug)
#include "MassEntityManager.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSetBase.h" // Mana attribute
#include "TimerManager.h"
#include "GameFramework/PlayerController.h" // RotateToMouse: resolve commanding player
#include "GameFramework/PlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdControl, Log, All);

static const TCHAR* CC_NetModeStr(UWorld* W)
{
	if (!W) return TEXT("NoWorld");
	switch (W->GetNetMode())
	{
	case NM_Standalone:    return TEXT("Standalone");
	case NM_DedicatedServer:return TEXT("DedicatedServer");
	case NM_ListenServer:  return TEXT("ListenServer");
	case NM_Client:        return TEXT("Client");
	default:               return TEXT("?");
	}
}

UCrowdControlAbility::UCrowdControlAbility()
{
	// Instanced per actor so GetAvatarActorFromActorInfo() / GetWeaponComponent() are valid
	// when the designer calls the building-block functions from the ability graph.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// The flashlight FIELD is a separate, self-owning actor that lives independently of this ability.
	// So the ability can End right after toggling the field (keeping the unit commandable) while the
	// field + light persist. bIsContinuousAbility / AbilityCanBeCanceled stay at their defaults.

	// Units collide on the Pawn channel (same one projectiles query to hit them). An EMPTY object-type
	// array makes SphereOverlapActors return nothing, so this must be populated.
	DetectionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
}

AUnitBase* UCrowdControlAbility::GetCasterUnit() const
{
	return Cast<AUnitBase>(GetAvatarActorFromActorInfo());
}

FCrowdControlData UCrowdControlAbility::ResolveEffectiveData() const
{
	UWeaponComponent* W = GetWeaponComponent();
	if (bUseAbilityBaseValues)
	{
		// This ability's own base + per-level, combined with the unit's invested talent levels.
		FCrowdControlData Data = AbilityBaseValues;
		if (W) Data.CopyLevelsFrom(W->CrowdControl);
		return Data;
	}
	if (W) return W->CrowdControl; // unit's base + levels
	return FCrowdControlData();     // struct defaults (no weapon component)
}

float UCrowdControlAbility::GetEffectiveRadius() const     { return ResolveEffectiveData().GetEffectiveRadius(); }
float UCrowdControlAbility::GetEffectiveDuration() const   { return ResolveEffectiveData().GetEffectiveDuration(); }
float UCrowdControlAbility::GetEffectiveHalfAngleDeg() const { return ResolveEffectiveData().GetEffectiveHalfAngleDeg(); }
int32 UCrowdControlAbility::GetEffectiveMaxTargets() const { return ResolveEffectiveData().GetEffectiveMaxTargets(); }
float UCrowdControlAbility::GetEffectiveHalfHeight() const { return ResolveEffectiveData().GetEffectiveHalfHeight(); }
float UCrowdControlAbility::GetEffectiveCooldown() const   { return ResolveEffectiveData().GetEffectiveCooldown(); }

bool UCrowdControlAbility::DetectUnitsInArea(FVector Origin, FVector Direction, TArray<AUnitBase*>& OutUnits)
{
	OutUnits.Reset();

	AUnitBase* Caster = GetCasterUnit();
	if (!Caster)
	{
		if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("DetectUnitsInArea: NO CASTER (GetAvatarActorFromActorInfo is not an AUnitBase)"));
		return false;
	}

	// Shared detection (same code the continuous flashlight field uses).
	ACrowdControlMarker::DetectUnitsInArea(GetWorld(), Caster, Origin, Direction, Shape,
		GetEffectiveRadius(), GetEffectiveHalfHeight(), GetEffectiveHalfAngleDeg(), GetEffectiveMaxTargets(),
		RingInnerFraction, ConeTraceSteps, DetectionObjectTypes, bIgnoreCaster, bIncludeDeadUnits, bDebugDraw, DebugDrawDuration, OutUnits);

	if (bDebugDraw)
	{
		UE_LOG(LogCrowdControl, Warning, TEXT("DetectUnitsInArea: Caster='%s' Team=%d Shape=%d Radius=%.0f SELECTED %d NetMode=%s"),
			*Caster->GetName(), Caster->TeamId, (int32)Shape, GetEffectiveRadius(), OutUnits.Num(), CC_NetModeStr(GetWorld()));
	}

	return OutUnits.Num() > 0;
}

AActor* UCrowdControlAbility::ShowAreaMarker(FVector Origin, FVector Direction)
{
	if (MarkerMode == ECrowdControlMarkerMode::None)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// The marker is a replicated actor, so it must be spawned on the server; clients receive it
	// (and its server-side destruction) automatically. Spawning on a client would create a
	// local-only marker invisible to everyone else.
	AActor* OwnerActor = GetAvatarActorFromActorInfo();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("ShowAreaMarker: skipped (Owner=%s HasAuthority=%d NetMode=%s)"),
			OwnerActor ? *OwnerActor->GetName() : TEXT("NULL"), OwnerActor ? (int32)OwnerActor->HasAuthority() : 0, CC_NetModeStr(GetWorld()));
		return nullptr;
	}

	const FVector Dir = Direction.GetSafeNormal2D();
	const FRotator Rot = Dir.IsNearlyZero() ? FRotator::ZeroRotator : Dir.Rotation();

	UClass* SpawnClass = MarkerClass ? MarkerClass.Get() : ACrowdControlMarker::StaticClass();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = OwnerActor;
	Params.Instigator = Cast<APawn>(OwnerActor);

	ACrowdControlMarker* Marker = World->SpawnActor<ACrowdControlMarker>(SpawnClass, Origin, Rot, Params);
	if (Marker)
	{
		Marker->InitMarker(MarkerMode, Shape, GetEffectiveRadius(), GetEffectiveHalfAngleDeg(), MarkerColor, MarkerLightIntensity);
		Marker->SetLifeSpan(FMath::Max(0.1f, GetEffectiveDuration()));
	}
	return Marker;
}

void UCrowdControlAbility::ApplyCrowdControlToUnits(const TArray<AUnitBase*>& Units)
{
	const bool bMove = (EffectType == ECrowdControlEffectType::DisableMovement || EffectType == ECrowdControlEffectType::DisableBoth);
	const bool bAttack = (EffectType == ECrowdControlEffectType::DisableAttack || EffectType == ECrowdControlEffectType::DisableBoth);
	const float Duration = GetEffectiveDuration();

	if (bDebugDraw)
	{
		UE_LOG(LogCrowdControl, Warning, TEXT("ApplyCrowdControlToUnits: %d unit(s) EffectType=%d (move=%d attack=%d) Duration=%.2f NetMode=%s"),
			Units.Num(), (int32)EffectType, bMove, bAttack, Duration, CC_NetModeStr(GetWorld()));
	}

	for (AUnitBase* U : Units)
	{
		if (!IsValid(U))
		{
			continue;
		}

		const bool bAuth = U->HasAuthority();
		if (bDebugDraw)
		{
			UE_LOG(LogCrowdControl, Warning, TEXT("  Unit '%s' Team=%d HasAuthority=%d State=%d CanMove(before)=%d CanAttack(before)=%d"),
				*U->GetName(), U->TeamId, bAuth, (int32)U->GetUnitState(), U->CanMove, U->CanAttack);
		}

		// CanMove / CanAttack are replicated by AUnitBase, so they must be set on the server.
		if (!bAuth)
		{
			if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("    -> SKIPPED (no authority; CC must run on the server)"));
			continue;
		}

		UCrowdControlStateComponent* CC = UCrowdControlStateComponent::FindOrAdd(U);
		if (!CC)
		{
			if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("    -> SKIPPED (could not get/add CrowdControlStateComponent)"));
			continue;
		}
		if (bFreezeUnits)
		{
			CC->PushFreeze(Duration); // full freeze incl. animation
		}
		else
		{
			if (bMove)
			{
				CC->PushMovementDisable(Duration);
			}
			if (bAttack)
			{
				CC->PushAttackDisable(Duration);
			}
		}
		// The actual Mass work (direct CanMove/CanAttack fragment write to beat the sync lag,
		// clearing the target so Idle/Chase don't re-engage, SwitchEntityTagByState(Idle) to drop
		// the Attack tag, and re-adding the Detect tag on restore) lives in the state component so
		// the ref-counted push/pop drives it consistently.

		if (bDebugDraw)
		{
			// Read the Mass AI fragment back to confirm the flags reached the entity this tick.
			int32 FragMove = -1, FragAttack = -1;
			FMassEntityManager* EM = nullptr;
			FMassEntityHandle EntityHandle;
			if (U->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle))
			{
				if (const FMassAIStateFragment* AI = EM->GetFragmentDataPtr<FMassAIStateFragment>(EntityHandle))
				{
					FragMove = AI->CanMove ? 1 : 0;
					FragAttack = AI->CanAttack ? 1 : 0;
				}
			}
			UE_LOG(LogCrowdControl, Warning, TEXT("    -> APPLIED. State(after)=%d CanMove(actor)=%d CanAttack(actor)=%d Frag.CanMove=%d Frag.CanAttack=%d"),
				(int32)U->GetUnitState(), U->CanMove, U->CanAttack, FragMove, FragAttack);
		}
	}
}

bool UCrowdControlAbility::ExecuteCrowdControlField(FVector Origin, FVector Direction, TArray<AUnitBase*>& OutAffectedUnits)
{
	ShowAreaMarker(Origin, Direction);
	const bool bFound = DetectUnitsInArea(Origin, Direction, OutAffectedUnits);
	ApplyCrowdControlToUnits(OutAffectedUnits);
	return bFound;
}

void UCrowdControlAbility::ResolveClickOriginDirection(const FVector& ClickLocation, FVector& OutOrigin, FVector& OutDirection) const
{
	OutOrigin = ClickLocation;
	OutDirection = FVector::ZeroVector;
	if (AUnitBase* Caster = GetCasterUnit())
	{
		OutDirection = ClickLocation - Caster->GetActorLocation();
	}
	// If there is no caster (or click == caster), Direction stays zero and the
	// underlying functions fall back to the caster forward vector.
}

bool UCrowdControlAbility::ActivateCrowdControl(FVector ClickLocation, TArray<AUnitBase*>& OutAffectedUnits)
{
	if (bDebugDraw)
	{
		AUnitBase* Caster = GetCasterUnit();
		UE_LOG(LogCrowdControl, Warning, TEXT("ActivateCrowdControl: Mode=%d Click=%s Caster=%s NetMode=%s"),
			(int32)ActivationMode, *ClickLocation.ToString(),
			Caster ? *Caster->GetName() : TEXT("NULL"), CC_NetModeStr(GetWorld()));
	}

	switch (ActivationMode)
	{
	case ECrowdControlActivationMode::SelfCentered:
		return RunCrowdControlSelfCentered(OutAffectedUnits);

	case ECrowdControlActivationMode::Modular:
	{
		// Same outcome as AtClick, but routed through the modular building blocks.
		const bool bFound = DetectUnitsAtClick(ClickLocation, OutAffectedUnits);
		ShowAreaMarkerAtClick(ClickLocation);
		ApplyCrowdControlToUnits(OutAffectedUnits);
		return bFound;
	}

	case ECrowdControlActivationMode::AtClick:
	default:
		return RunCrowdControlAtClick(ClickLocation, OutAffectedUnits);
	}
}

bool UCrowdControlAbility::RunCrowdControlAtClick(FVector ClickLocation, TArray<AUnitBase*>& OutAffectedUnits)
{
	FVector Origin, Direction;
	ResolveClickOriginDirection(ClickLocation, Origin, Direction);
	return ExecuteCrowdControlField(Origin, Direction, OutAffectedUnits);
}

bool UCrowdControlAbility::RunCrowdControlSelfCentered(TArray<AUnitBase*>& OutAffectedUnits)
{
	OutAffectedUnits.Reset();
	AUnitBase* Caster = GetCasterUnit();
	if (!Caster)
	{
		return false;
	}
	const FVector Origin = Caster->GetActorLocation();
	const FVector Direction = Caster->GetActorForwardVector();
	return ExecuteCrowdControlField(Origin, Direction, OutAffectedUnits);
}

bool UCrowdControlAbility::DetectUnitsAtClick(FVector ClickLocation, TArray<AUnitBase*>& OutUnits)
{
	FVector Origin, Direction;
	ResolveClickOriginDirection(ClickLocation, Origin, Direction);
	return DetectUnitsInArea(Origin, Direction, OutUnits);
}

AActor* UCrowdControlAbility::ShowAreaMarkerAtClick(FVector ClickLocation)
{
	FVector Origin, Direction;
	ResolveClickOriginDirection(ClickLocation, Origin, Direction);
	return ShowAreaMarker(Origin, Direction);
}

bool UCrowdControlAbility::ActivateCrowdControlField()
{
	// Toggle: if it's already on, turn it off. End the ability immediately afterwards (the field is
	// independent and persists/dies on its own); see EndSelf() for why this must not stay active.
	if (ActiveField.IsValid())
	{
		DeactivateCrowdControlField();
		EndSelf();
		return false;
	}

	AUnitBase* Caster = GetCasterUnit();
	UWorld* World = GetWorld();
	if (!Caster || !World)
	{
		return false;
	}

	// The field marker is replicated, so it must be spawned on the server.
	if (!Caster->HasAuthority())
	{
		if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("ActivateCrowdControlField: skipped (no authority, NetMode=%s)"), CC_NetModeStr(World));
		return false;
	}

	UClass* SpawnClass = MarkerClass ? MarkerClass.Get() : ACrowdControlMarker::StaticClass();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = Caster;
	Params.Instigator = Cast<APawn>(Caster);

	ACrowdControlMarker* Field = World->SpawnActor<ACrowdControlMarker>(SpawnClass, Caster->GetActorTransform(), Params);
	if (!Field)
	{
		EndSelf();
		return false;
	}

	// RotateToMouse: while the field is on and the caster stands still, aim it (and its cone) at the mouse.
	// The base ability's bRotateUnitsToMouse only runs during the ability's lifetime; since we End the ability
	// right after toggling the field, the field itself drives it.
	// Resolve which player commands this caster: RTS units are AI-possessed, so ActorInfo->PlayerController is
	// usually NULL -> fall back to CurrentInstigatorPC (the controller that triggered the ability, stored by
	// ActivateAbilityByInputID). The id then replicates via ASpawnerUnit::ActiveRotationPlayerId.
	Field->bRotateToMouse = bRotateUnitsToMouse;
	APlayerController* RotPC = CurrentActorInfo ? CurrentActorInfo->PlayerController.Get() : nullptr;
	const TCHAR* RotPCSource = TEXT("ActorInfo");
	if (!RotPC)
	{
		RotPC = Caster->CurrentInstigatorPC.Get();
		RotPCSource = TEXT("InstigatorPC");
	}
	if (RotPC && RotPC->PlayerState)
	{
		Field->RotatorPlayerId = RotPC->PlayerState->GetPlayerId();
	}
	if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("Field RotateToMouse=%d RotatorPlayerId=%d (src=%s PCValid=%d)"),
		bRotateUnitsToMouse, Field->RotatorPlayerId, RotPCSource, RotPC ? 1 : 0);

	Field->InitField(Caster, HandSocketName, MarkerMode, Shape, EffectType, bFreezeUnits,
		RingInnerFraction, ConeTraceSteps, DetectionObjectTypes, bIgnoreCaster, bIncludeDeadUnits,
		MarkerColor, MarkerLightIntensity, bDebugDraw, DebugDrawDuration, bUseAbilityBaseValues, AbilityBaseValues);

	ActiveField = Field;

	// Upkeep timer: drains mana over time and ends the ability if the field dies / mana runs out.
	const float Interval = (ManaPerInterval > 0.f) ? FMath::Max(0.05f, ManaDrainInterval) : 0.5f;
	TWeakObjectPtr<UCrowdControlAbility> WeakThis(this);
	World->GetTimerManager().SetTimer(FieldUpkeepTimer, [WeakThis]()
	{
		if (WeakThis.IsValid()) WeakThis->TickFieldUpkeep();
	}, Interval, true);

	if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("ActivateCrowdControlField: ON (Caster=%s Shape=%d Socket=%s Freeze=%d)"),
		*Caster->GetName(), (int32)Shape, *HandSocketName.ToString(), bFreezeUnits);

	// Field is up; end the ability so the unit stays fully commandable (the field lives on independently).
	EndSelf();
	return true;
}

void UCrowdControlAbility::DeactivateCrowdControlField()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FieldUpkeepTimer);
	}

	if (ACrowdControlMarker* Field = ActiveField.Get())
	{
		Field->Destroy(); // EndPlay releases every still-affected unit
	}
	ActiveField = nullptr;

	if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("DeactivateCrowdControlField: OFF"));
}

bool UCrowdControlAbility::IsCrowdControlFieldActive() const
{
	return ActiveField.IsValid();
}

void UCrowdControlAbility::EndSelf()
{
	// Guard so we don't warn/double-end. The flashlight field is a separate actor, so ending here does NOT
	// turn it off. Leaving the ability active would let RightClickPressedMass->TryCancelActiveAbilities eat
	// the first right-click (cancel the ability) instead of issuing a move.
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UCrowdControlAbility::TickFieldUpkeep()
{
	// The field died on its own (e.g. the owner died) -> just stop this upkeep timer.
	if (!ActiveField.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FieldUpkeepTimer);
		}
		return;
	}

	if (ManaPerInterval <= 0.f)
	{
		return; // watchdog only, no mana drain configured
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	const float CurrentMana = ASC->GetNumericAttribute(UAttributeSetBase::GetManaAttribute());
	if (CurrentMana < ManaPerInterval)
	{
		if (bDebugDraw) UE_LOG(LogCrowdControl, Warning, TEXT("Field: out of mana (%.1f < %.1f) -> deactivating"), CurrentMana, ManaPerInterval);
		DeactivateCrowdControlField(); // turns the field off + clears this timer
		return;
	}

	ASC->SetNumericAttributeBase(UAttributeSetBase::GetManaAttribute(), CurrentMana - ManaPerInterval);
}
