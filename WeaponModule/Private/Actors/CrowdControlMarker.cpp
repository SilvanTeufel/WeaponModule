// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Actors/CrowdControlMarker.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Characters/Unit/UnitBase.h"
#include "Characters/Unit/SpawnerUnit.h"  // ActiveRotationPlayerId
#include "Components/WeaponComponent.h"
#include "Components/CrowdControlStateComponent.h"
#include "Core/UnitData.h"
#include "Mass/UnitMassTag.h"              // FMassRotateToMouseTag / FMassRotateToMouseFragment
#include "MassEntityManager.h"
#include "MassMovementFragments.h"         // FMassVelocityFragment (is-moving check)
#include "MassNavigationFragments.h"        // FMassMoveTargetFragment / EMassMovementAction (move-intent check)
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

// ---------- file-local gather helpers (the actual SphereOverlap traces) ----------
static void CC_GatherSphere(UWorld* World, const FVector& Origin, float QueryRadius,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bDebugDraw, float DebugDur, TArray<AActor*>& Out)
{
	Out.Reset();
	if (!World) return;
	const TArray<AActor*> Ignore;
	UKismetSystemLibrary::SphereOverlapActors(World, Origin, FMath::Max(1.f, QueryRadius), ObjectTypes, AUnitBase::StaticClass(), Ignore, Out);
	if (bDebugDraw)
	{
		DrawDebugSphere(World, Origin, FMath::Max(1.f, QueryRadius), 16, FColor::Cyan, false, DebugDur, 0, 2.f);
	}
}

static void CC_GatherCone(UWorld* World, const FVector& Origin, const FVector& Dir, float Radius, float HalfAngleDeg,
	int32 ConeTraceSteps, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, bool bDebugDraw, float DebugDur, TArray<AActor*>& Out)
{
	Out.Reset();
	if (!World) return;
	const int32 Steps = FMath::Clamp(ConeTraceSteps, 1, 16);
	const float HalfAngleRad = FMath::DegreesToRadians(HalfAngleDeg);
	const TArray<AActor*> Ignore;
	TSet<AActor*> Unique;
	for (int32 i = 1; i <= Steps; ++i)
	{
		const float T = (float)i / (float)Steps;
		const FVector Center = Origin + Dir * (Radius * T);
		const float TraceRadius = FMath::Max(50.f, Radius * T * FMath::Tan(HalfAngleRad));
		if (bDebugDraw)
		{
			DrawDebugSphere(World, Center, TraceRadius, 12, FColor::Orange, false, DebugDur, 0, 1.5f);
		}
		TArray<AActor*> Hits;
		UKismetSystemLibrary::SphereOverlapActors(World, Center, TraceRadius, ObjectTypes, AUnitBase::StaticClass(), Ignore, Hits);
		for (AActor* H : Hits) Unique.Add(H);
	}
	Out = Unique.Array();
}

ACrowdControlMarker::ACrowdControlMarker()
{
	// Field detection/effect logic still runs on world timers (robust against BP "Can Ever Tick"); per-frame
	// Tick is used ONLY by remote clients to hold the ISM VAT clock of frozen units (must be per-frame — the
	// material advances the vertex animation every frame).
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetupAttachment(SceneRoot);
	PointLight->SetMobility(EComponentMobility::Movable);
	PointLight->SetCastShadows(false);
	PointLight->SetVisibility(false);

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(SceneRoot);
	SpotLight->SetMobility(EComponentMobility::Movable);
	SpotLight->SetCastShadows(false);
	SpotLight->SetVisibility(false);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(SceneRoot);
	NiagaraComp->SetAutoActivate(false);
	NiagaraComp->SetVisibility(false);
}

void ACrowdControlMarker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACrowdControlMarker, Mode);
	DOREPLIFETIME(ACrowdControlMarker, Shape);
	DOREPLIFETIME(ACrowdControlMarker, Radius);
	DOREPLIFETIME(ACrowdControlMarker, HalfAngleDeg);
	DOREPLIFETIME(ACrowdControlMarker, MarkerColor);
	DOREPLIFETIME(ACrowdControlMarker, LightIntensity);
	DOREPLIFETIME(ACrowdControlMarker, bFreezeUnits);
}

void ACrowdControlMarker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Remote clients only: hold the ISM vertex-animation (VAT) clock of the units this marker froze, so their
	// material-driven animation freezes IN PLACE. The server/host does this via UCrowdControlStateComponent's
	// tick; ISM custom data is not replicated, so each machine freezes its own render. The frozen SET (and its
	// skeletal freeze) is maintained by the 0.1s ClientVisualFreezeTick; here we only pin the clock per frame.
	if (HasAuthority() || !bFreezeUnits)
	{
		return;
	}
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const float Now = World->GetTimeSeconds();
	for (const TPair<TWeakObjectPtr<AUnitBase>, float>& Pair : ClientFrozenRates)
	{
		if (AUnitBase* U = Pair.Key.Get())
		{
			UCrowdControlStateComponent::HoldISMVertexClock(U, Now, ClientISMAnchors.FindOrAdd(Pair.Key),
				StartTimeCustomDataIndex, PrevStartTimeCustomDataIndex);
		}
	}
}

void ACrowdControlMarker::BeginPlay()
{
	Super::BeginPlay();
	ConfigureVisuals();

	// Remote clients freeze the SKELETAL animation locally (GlobalAnimRateScale is server-only / not
	// replicated). The host/server does it via UCrowdControlStateComponent. InitField is a server-only C++
	// call (not run on replicated client copies), so start this here. The tick self-gates on bFreezeUnits.
	if (!HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ClientVisualTimer, this, &ACrowdControlMarker::ClientVisualFreezeTick,
				FMath::Max(0.05f, UpdateInterval), true);
		}
	}
}

void ACrowdControlMarker::InitMarker(ECrowdControlMarkerMode InMode, ECrowdControlShape InShape, float InRadius, float InHalfAngleDeg, const FLinearColor& InColor, float InLightIntensity)
{
	Mode = InMode;
	Shape = InShape;
	Radius = InRadius;
	HalfAngleDeg = InHalfAngleDeg;
	MarkerColor = InColor;
	LightIntensity = InLightIntensity;
	ConfigureVisuals();
}

void ACrowdControlMarker::InitField(AUnitBase* InOwnerUnit, FName SocketName, ECrowdControlMarkerMode InMode, ECrowdControlShape InShape, ECrowdControlEffectType InEffectType,
	bool bInFreezeUnits, float InRingInnerFraction, int32 InConeTraceSteps, const TArray<TEnumAsByte<EObjectTypeQuery>>& InObjectTypes,
	bool bInIgnoreCaster, bool bInIncludeDeadUnits, const FLinearColor& InColor, float InLightIntensity,
	bool bInDebugDraw, float InDebugDrawDuration, bool bInUseAbilityBaseValues, const FCrowdControlData& InAbilityBaseValues)
{
	bIsField = true;
	OwnerUnit = InOwnerUnit;
	Mode = InMode;
	Shape = InShape;
	EffectType = InEffectType;
	bFreezeUnits = bInFreezeUnits;
	bUseAbilityBaseValues = bInUseAbilityBaseValues;
	AbilityBaseValues = InAbilityBaseValues;
	RingInnerFraction = InRingInnerFraction;
	ConeTraceSteps = InConeTraceSteps;
	DetectionObjectTypes = InObjectTypes;
	bIgnoreCaster = bInIgnoreCaster;
	bIncludeDeadUnits = bInIncludeDeadUnits;
	MarkerColor = InColor;
	LightIntensity = InLightIntensity;
	bDebugDraw = bInDebugDraw;
	DebugDrawDuration = InDebugDrawDuration;

	// Attach so the field + light follow the owner. Prefer the requested skeletal-mesh socket
	// (e.g. "rightHandSocket"); otherwise fall back to the mesh, then the root with a height offset.
	bool bAttached = false;
	if (ACharacter* Char = Cast<ACharacter>(InOwnerUnit))
	{
		if (USkeletalMeshComponent* MeshComp = Char->GetMesh())
		{
			if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
			{
				AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
				bAttached = true;
			}
			else
			{
				AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);
				SetActorRelativeLocation(FVector(0.f, 0.f, LightHeightOffset));
				bAttached = true;
			}
		}
	}
	if (!bAttached && InOwnerUnit && InOwnerUnit->GetRootComponent())
	{
		AttachToComponent(InOwnerUnit->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		SetActorRelativeLocation(FVector(0.f, 0.f, LightHeightOffset));
	}

	ConfigureVisuals();

	// Drive the field with world timers (independent of actor tick / BP "Can Ever Tick").
	if (HasAuthority())
	{
		// While the flashlight/field is active the CASTER itself cannot attack (it's occupied holding the
		// light). Ref-counted attack disable -> restored to its original value when the field ends (EndPlay).
		// The caster is never in AffectedUnits (enemy-only detection), so this is its own dedicated hold.
		if (UCrowdControlStateComponent* CasterCC = UCrowdControlStateComponent::FindOrAdd(InOwnerUnit))
		{
			CasterCC->HoldAttackDisable();
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(UpdateTimer, this, &ACrowdControlMarker::FieldUpdate, FMath::Max(0.02f, UpdateInterval), true);
			if (ReassertInterval > 0.f)
			{
				World->GetTimerManager().SetTimer(ReassertTimer, this, &ACrowdControlMarker::ReassertTick, ReassertInterval, true);
			}
		}
		FieldUpdate(); // apply immediately, don't wait for the first interval
	}
}

void ACrowdControlMarker::FieldUpdate()
{
	if (!bIsField || !HasAuthority())
	{
		return;
	}

	AUnitBase* Unit = OwnerUnit.Get();
	if (!Unit || Unit->GetUnitState() == UnitData::Dead)
	{
		Destroy(); // EndPlay releases every still-affected unit
		return;
	}

	ServerUpdateField();
	UpdateRotateToMouse(Unit); // aim the caster (and its cone) at the mouse while standing still
}

void ACrowdControlMarker::ReassertTick()
{
	if (bIsField && HasAuthority() && OwnerUnit.IsValid())
	{
		ReassertAffected();
		// NOTE: deliberately NOT reasserting the caster here — the caster must stay free to MOVE with the
		// flashlight, and ReassertDisable would force it back to Idle. Its CanAttack=false is self-maintained
		// (actor->fragment sync re-copies the false bool every tick; DetectionProcessor drops targets while false).
	}
}

void ACrowdControlMarker::ClientVisualFreezeTick()
{
	// Remote-client only (BeginPlay only starts the timer when !HasAuthority). Mirrors the server's skeletal
	// animation freeze locally, since GlobalAnimRateScale is not a replicated property.
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Not a freeze field (or it turned off) -> make sure nothing stays frozen.
	if (!bFreezeUnits)
	{
		if (ClientFrozenRates.Num() > 0) RestoreAllClientFrozen();
		return;
	}

	// Detect nearby units locally (a generous sphere around the owner; only the replicated Radius is needed —
	// this is purely cosmetic so exact cone shape doesn't matter). The marker is attached to the owner, so the
	// attach-parent is the caster on the client too.
	AActor* OwnerActor = GetAttachParentActor();
	const FVector Origin = OwnerActor ? OwnerActor->GetActorLocation() : GetActorLocation();
	const float QueryR = FMath::Max(50.f, Radius);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjTypes;
	ObjTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	const TArray<AActor*> Ignore;
	TArray<AActor*> Hits;
	UKismetSystemLibrary::SphereOverlapActors(World, Origin, QueryR, ObjTypes, AUnitBase::StaticClass(), Ignore, Hits);

	// Freeze the SKM of in-range units the server crowd-controlled (CanMove==false is the replicated signal).
	TSet<TWeakObjectPtr<AUnitBase>> NowFrozen;
	for (AActor* A : Hits)
	{
		AUnitBase* U = Cast<AUnitBase>(A);
		if (!U || U == OwnerActor || U->CanMove) continue; // only units the server stopped
		NowFrozen.Add(U);
		if (USkeletalMeshComponent* Mesh = U->GetMesh())
		{
			if (!ClientFrozenRates.Contains(U))
			{
				ClientFrozenRates.Add(U, Mesh->GlobalAnimRateScale);
			}
			Mesh->GlobalAnimRateScale = 0.f; // enforce each tick (PerformanceUnit can't undo a rate, only bPauseAnims)
		}
	}

	// Restore units that are no longer frozen (left the field / CC released / GC'd).
	TArray<TWeakObjectPtr<AUnitBase>> ToRestore;
	for (const TPair<TWeakObjectPtr<AUnitBase>, float>& Pair : ClientFrozenRates)
	{
		if (!NowFrozen.Contains(Pair.Key)) ToRestore.Add(Pair.Key);
	}
	for (const TWeakObjectPtr<AUnitBase>& W : ToRestore)
	{
		if (AUnitBase* U = W.Get())
		{
			if (USkeletalMeshComponent* Mesh = U->GetMesh())
			{
				const float* Saved = ClientFrozenRates.Find(W);
				Mesh->GlobalAnimRateScale = (Saved && *Saved > 0.f) ? *Saved : 1.f;
			}
		}
		ClientFrozenRates.Remove(W);
		ClientISMAnchors.Remove(W);
	}
}

void ACrowdControlMarker::RestoreAllClientFrozen()
{
	for (const TPair<TWeakObjectPtr<AUnitBase>, float>& Pair : ClientFrozenRates)
	{
		if (AUnitBase* U = Pair.Key.Get())
		{
			if (USkeletalMeshComponent* Mesh = U->GetMesh())
			{
				Mesh->GlobalAnimRateScale = (Pair.Value > 0.f) ? Pair.Value : 1.f;
			}
		}
	}
	ClientFrozenRates.Empty();
	ClientISMAnchors.Empty();
}

void ACrowdControlMarker::UpdateRotateToMouse(AUnitBase* Unit)
{
	if (!bRotateToMouse || !Unit || !HasAuthority())
	{
		return;
	}

	FMassEntityManager* EM = nullptr;
	FMassEntityHandle Entity;
	if (!(Unit->GetMassEntityData(EM, Entity) && EM && EM->IsEntityValid(Entity)))
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;

	// ---------------------------------------------------------------------------------------------
	// CASTING OVERRIDE (e.g. the Reload ability): a casting/charging ability must play its OWN animation.
	// FMassRotateToMouseTag resolves to EState::Aim, and in UnitClientTagSyncProcessor::ComputeState Aim is
	// checked BEFORE Casting -> while the aim tag is present the Casting/Reload animation is masked. Per the
	// design choice we fully DROP the aim tag for the duration of the cast (the caster stops following the
	// mouse, plays the cast/reload anim), then let the per-tick re-assert below re-engage mouse-aim the moment
	// the cast tag clears. Detected via the Mass tag (not GetUnitState(), which is itself masked to Aim here).
	// Server-only path; the tag is a replicated control bit, so removing it on the server drops Aim on clients
	// too and their ComputeState falls through to the (replicated) Casting state -> reload anim shows everywhere.
	const bool bCastingBusy =
		DoesEntityHaveTag(*EM, Entity, FMassStateCastingTag::StaticStruct()) ||
		DoesEntityHaveTag(*EM, Entity, FMassStateChargingTag::StaticStruct());
	if (bCastingBusy)
	{
		// Remove whether the marker (bRotateToMouseActive) or the ability itself put the tag there; idempotent.
		if (bRotateToMouseActive || DoesEntityHaveTag(*EM, Entity, FMassRotateToMouseTag::StaticStruct()))
		{
			if (bDebugDraw)
			{
				UE_LOG(LogTemp, Warning, TEXT("[CC RotateToMouse] '%s' OFF (casting -> yield to ability anim)"), *Unit->GetName());
			}
			StopRotateToMouse(Unit); // sets bRotateToMouseActive=false; re-added automatically once the cast ends
		}
		return;
	}

	// ---------------------------------------------------------------------------------------------
	// MOVE-INTENT test (replaces the old velocity-only "is moving" check that caused the stuck race).
	//
	// THE RACE: the player right-clicks -> the move command (RTSUnitTemplate UpdateMoveTarget) sets the
	// unit moving, but velocity is still ~0 while it ramps. The old test (velocity > 50) saw velocity<50,
	// RE-ADDED FMassRotateToMouseTag, and that tag EXCLUDES the unit from every Mass movement AND state
	// processor -> velocity can never ramp -> the unit is frozen/oscillates forever.
	//
	// Fix: gate on MOVE-INTENT, which the server writes SYNCHRONOUSLY the instant the order is processed
	// (UpdateMoveTarget -> FMassMoveTargetFragment::CreateNewAction(Move) + DesiredSpeed), long BEFORE the
	// velocity ramp, and which UpdateMoveTarget writes regardless of this tag (it is command-driven, not a
	// processor that the tag gates). Priority of signals:
	//   (1) FMassMoveTargetFragment::GetCurrentAction()==Move AND DesiredSpeed>0 AND not-yet-arrived (the
	//       unit is still farther from MoveTarget.Center than the acceptance radius). Earliest & synchronous,
	//       so it bridges the leading edge before the velocity ramp / before the deferred RunTag flushes.
	//       The "not-yet-arrived" gate is ESSENTIAL: action==Move is STICKY here — the normal right-click
	//       arrival path (RunStateProcessor::SwitchToIdleState) does NOT call StopMovement, so the action
	//       stays Move forever after a move. Without the distance gate a unit that finished one move would be
	//       pinned 'moving' forever and rotate-to-mouse would never re-engage. The DesiredSpeed>0 guard
	//       rejects the engine's stale DEFAULT CurrentAction=Move on a freshly-spawned/never-commanded unit.
	//   (2) FMassStateRunTag presence — added by every move command, removed at arrival/idle (OR-signal;
	//       it is added via Defer() so it can lag the fragment by one command-flush, hence not primary).
	//   (3) Actual velocity > 50 — keep ONLY as a final OR (a unit physically moving from any cause should
	//       not rotate-to-mouse). Never the sole gate: it is exactly what loses the race.
	//
	// CLIENT: this field is server-only (HasAuthority gate above). FMassRotateToMouseTag is a replicated
	// control bit that ApplyReplicatedTagBits hard-syncs onto clients unconditionally, so the moment the
	// server stops re-adding it the client tag is removed automatically — the client is fixed via
	// replication with no WeaponModule client code (a client-side removal would be re-stomped every
	// reconcile tick while the server bit is still ON, so it cannot durably bridge the RTT anyway).
	// ---------------------------------------------------------------------------------------------
	bool bMoveIntent = false;

	if (const FMassMoveTargetFragment* MoveTarget = EM->GetFragmentDataPtr<FMassMoveTargetFragment>(Entity))
	{
		// Gate the (sticky) action==Move on "not yet arrived": still farther from the target than acceptance.
		if (MoveTarget->GetCurrentAction() == EMassMovementAction::Move &&
			MoveTarget->DesiredSpeed.Get() > KINDA_SMALL_NUMBER)
		{
			const float Slack = FMath::Max(MoveTarget->SlackRadius, 50.f);
			if (FVector::DistSquared2D(Unit->GetActorLocation(), MoveTarget->Center) > FMath::Square(Slack))
			{
				bMoveIntent = true;
			}
		}
	}
	if (!bMoveIntent && DoesEntityHaveTag(*EM, Entity, FMassStateRunTag::StaticStruct()))
	{
		bMoveIntent = true;
	}
	if (!bMoveIntent)
	{
		if (const FMassVelocityFragment* Vel = EM->GetFragmentDataPtr<FMassVelocityFragment>(Entity))
		{
			bMoveIntent = !Vel->Value.IsNearlyZero(50.f);
		}
	}

	// Trailing grace: latch the last time we saw move-intent. We keep rotate-to-mouse SUPPRESSED for a
	// short window after move-intent drops, to bridge the settle edge (velocity ramp-down, the one-frame
	// deferred FMassStateRunTag removal, and a momentary all-signals-false tick right at arrival). This
	// grace is refreshed ONLY by move-intent, so a genuinely idle unit (no recent movement) re-engages
	// rotate-to-mouse immediately — it does NOT delay the per-tick re-assert that recovers the tag after
	// EndAbility/ApplyRunAnimationTag strip it while idle.
	if (bMoveIntent)
	{
		LastMoveIntentTime = Now;
	}
	const bool bInMoveGrace = (LastMoveIntentTime >= 0.f) &&
		((Now - LastMoveIntentTime) < FMath::Max(0.f, RotateReaddGraceSeconds));

	if (!bMoveIntent && !bInMoveGrace)
	{
		// ENFORCE every tick (not just on the false->true edge): the base ability's EndAbility resets
		// ActiveRotationPlayerId=-1 and strips this tag+fragment right after we toggle on; ApplyRunAnimationTag
		// does the same. AddTagToEntity/AddFragmentToEntity are idempotent (guarded), so re-asserting is cheap.
		// Only write a VALID id (>=0) so we never clobber a good value with -1.
		if (RotatorPlayerId >= 0)
		{
			if (ASpawnerUnit* Spawner = Cast<ASpawnerUnit>(Unit))
			{
				Spawner->ActiveRotationPlayerId = RotatorPlayerId; // which player's mouse to follow
			}
		}
		// Immediate (we're in a world-timer, outside the Mass pipeline). The tag is a replicated control bit,
		// so on clients ApplyReplicatedTagBits mirrors the tag AND auto-adds/removes the fragment.
		EM->AddTagToEntity(Entity, FMassRotateToMouseTag::StaticStruct());
		if (EM->GetFragmentDataPtr<FMassRotateToMouseFragment>(Entity) == nullptr)
		{
			EM->AddFragmentToEntity(Entity, FMassRotateToMouseFragment::StaticStruct());
		}
		if (!bRotateToMouseActive && bDebugDraw)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CC RotateToMouse] '%s' ON RotatorPlayerId=%d auth=%d"),
				*Unit->GetName(), RotatorPlayerId, (int32)HasAuthority());
		}
		bRotateToMouseActive = true;
	}
	else if (bRotateToMouseActive)
	{
		if (bDebugDraw)
		{
			UE_LOG(LogTemp, Warning, TEXT("[CC RotateToMouse] '%s' OFF (move-intent=%d grace=%d)"),
				*Unit->GetName(), (int32)bMoveIntent, (int32)bInMoveGrace);
		}
		StopRotateToMouse(Unit); // moving/commanded -> hand facing back to the movement-direction rotation
	}
}

void ACrowdControlMarker::StopRotateToMouse(AUnitBase* Unit)
{
	bRotateToMouseActive = false;
	if (!Unit || !HasAuthority())
	{
		return;
	}

	FMassEntityManager* EM = nullptr;
	FMassEntityHandle Entity;
	if (Unit->GetMassEntityData(EM, Entity) && EM && EM->IsEntityValid(Entity))
	{
		EM->RemoveTagFromEntity(Entity, FMassRotateToMouseTag::StaticStruct());
		if (EM->GetFragmentDataPtr<FMassRotateToMouseFragment>(Entity) != nullptr)
		{
			EM->RemoveFragmentFromEntity(Entity, FMassRotateToMouseFragment::StaticStruct());
		}
	}
}

void ACrowdControlMarker::ServerUpdateField()
{
	AUnitBase* Unit = OwnerUnit.Get();
	UWorld* World = GetWorld();
	if (!Unit || !World)
	{
		return;
	}

	// Live, talent-tuned values so investing points changes the running flashlight. Per-ability base
	// values (if set) are combined with the unit's invested talent levels.
	FCrowdControlData Eff;
	if (UWeaponComponent* W = Unit->FindComponentByClass<UWeaponComponent>())
	{
		if (bUseAbilityBaseValues)
		{
			Eff = AbilityBaseValues;
			Eff.CopyLevelsFrom(W->CrowdControl);
		}
		else
		{
			Eff = W->CrowdControl;
		}
	}
	else
	{
		Eff = bUseAbilityBaseValues ? AbilityBaseValues : FCrowdControlData();
	}

	const float R = Eff.GetEffectiveRadius();
	const float HH = Eff.GetEffectiveHalfHeight();
	const float HA = Eff.GetEffectiveHalfAngleDeg();
	const int32 MaxT = Eff.GetEffectiveMaxTargets();

	// Orient the marker (and thus the spot light) to the unit's facing so the cone points where the
	// character looks — the hand-socket bone forward is unreliable. Sphere/Ring are direction-agnostic.
	if (Shape == ECrowdControlShape::Cone)
	{
		const FVector Fwd2D = Unit->GetActorForwardVector().GetSafeNormal2D();
		if (!Fwd2D.IsNearlyZero())
		{
			SetActorRotation(Fwd2D.Rotation());
		}
	}

	// Push to the replicated visual values + refresh the light.
	Radius = R;
	HalfAngleDeg = HA;
	ConfigureVisuals();

	// Detect from the UNIT's location (ground/center), not the marker's (which sits at the elevated hand
	// socket and would push ground units out of the vertical HalfHeight filter). The light stays at the hand.
	const FVector Origin = Unit->GetActorLocation();
	const FVector Direction = Unit->GetActorForwardVector(); // where the character faces

	// The debug shapes are redrawn every detection tick, so their lifetime must be AT LEAST one update
	// interval or they vanish between redraws (this is why small DebugDrawDuration values "didn't show").
	// Clamp the effective lifetime to ~2 update intervals so any user value (even tiny) stays visible.
	const float DbgDur = FMath::Max(DebugDrawDuration, FMath::Max(0.02f, UpdateInterval) * 2.f + 0.05f);

	TArray<AUnitBase*> Current;
	DetectUnitsInArea(World, Unit, Origin, Direction, Shape, R, HH, HA, MaxT,
		RingInnerFraction, ConeTraceSteps, DetectionObjectTypes, bIgnoreCaster, bIncludeDeadUnits, bDebugDraw, DbgDur, Current);

	const bool bMove = (EffectType == ECrowdControlEffectType::DisableMovement || EffectType == ECrowdControlEffectType::DisableBoth);
	const bool bAttack = (EffectType == ECrowdControlEffectType::DisableAttack || EffectType == ECrowdControlEffectType::DisableBoth);

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CC Field] detected=%d Origin=%s R=%.0f Freeze=%d Move=%d Attack=%d NetMode=%d MarkerAuth=%d"),
			Current.Num(), *Origin.ToString(), R, bFreezeUnits, bMove, bAttack, (int32)World->GetNetMode(), (int32)HasAuthority());
	}

	TSet<TWeakObjectPtr<AUnitBase>> CurrentSet;
	CurrentSet.Reserve(Current.Num());
	for (AUnitBase* U : Current)
	{
		if (U) CurrentSet.Add(U);
	}

	const double Now = World->GetTimeSeconds();

	// Mark every currently-detected unit as seen now (drives the release hysteresis below).
	for (const TWeakObjectPtr<AUnitBase>& WUnit : CurrentSet)
	{
		LastSeenTime.Add(WUnit, Now);
	}

	// Units that ENTERED the field -> apply the hold ONCE (ref-counted).
	for (const TWeakObjectPtr<AUnitBase>& WUnit : CurrentSet)
	{
		if (AffectedUnits.Contains(WUnit)) continue;
		if (AUnitBase* U = WUnit.Get())
		{
			if (bDebugDraw)
			{
				UE_LOG(LogTemp, Warning, TEXT("[CC Field] ENTER '%s' Team=%d State=%d CanMove=%d CanAttack=%d HasAuthority=%d -> holding"),
					*U->GetName(), U->TeamId, (int32)U->GetUnitState(), U->CanMove, U->CanAttack, (int32)U->HasAuthority());
			}
			if (UCrowdControlStateComponent* CC = UCrowdControlStateComponent::FindOrAdd(U))
			{
				if (bFreezeUnits)
				{
					// Freeze = stop moving + stop attacking (proven immediate fragment writes) AND pause
					// the animation (Frozen tag + skeletal GlobalAnimRateScale=0). The move/attack holds
					// guarantee the CC even if the tag doesn't take; HoldFreeze adds the animation pause.
					CC->HoldFreeze();       // FIRST: FreezeCount>0 makes HoldAttackDisable skip its SwitchToIdle (ISM mid-motion freeze)
					CC->HoldMovementDisable();
					CC->HoldAttackDisable();
				}
				else
				{
					if (bMove) CC->HoldMovementDisable();
					if (bAttack) CC->HoldAttackDisable();
				}
				AffectedUnits.Add(WUnit);
				if (bDebugDraw)
				{
					UE_LOG(LogTemp, Warning, TEXT("[CC Field]   after hold '%s' State=%d CanMove=%d CanAttack=%d"),
						*U->GetName(), (int32)U->GetUnitState(), U->CanMove, U->CanAttack);
				}
			}
			else if (bDebugDraw)
			{
				UE_LOG(LogTemp, Warning, TEXT("[CC Field]   FindOrAdd state component FAILED for '%s'"), *U->GetName());
			}
		}
	}

	// Units that LEFT -> release, but only after a HYSTERESIS grace (or if the unit was destroyed). A single
	// missed detection tick (MaxTargets cap re-ordering, vertical-boundary jitter) no longer thrashes the CC
	// off-and-on (Release would restore CanMove/CanAttack=true). This was a main cause of "CC oft nicht".
	{
		const float Grace = FMath::Max(0.f, ReleaseGraceSeconds);
		TArray<TWeakObjectPtr<AUnitBase>> ToRelease;
		for (const TWeakObjectPtr<AUnitBase>& WUnit : AffectedUnits)
		{
			if (CurrentSet.Contains(WUnit)) continue; // still inside the field
			if (!WUnit.IsValid()) { ToRelease.Add(WUnit); continue; }
			const double* Seen = LastSeenTime.Find(WUnit);
			if (!Seen || (Now - *Seen) > Grace) ToRelease.Add(WUnit);
		}
		for (const TWeakObjectPtr<AUnitBase>& WUnit : ToRelease)
		{
			if (AUnitBase* U = WUnit.Get())
			{
				if (UCrowdControlStateComponent* CC = U->FindComponentByClass<UCrowdControlStateComponent>())
				{
					if (bFreezeUnits)
					{
						CC->ReleaseMovementDisable();
						CC->ReleaseAttackDisable();
						CC->ReleaseFreeze();
					}
					else
					{
						if (bMove) CC->ReleaseMovementDisable();
						if (bAttack) CC->ReleaseAttackDisable();
					}
				}
			}
			AffectedUnits.Remove(WUnit);
			LastSeenTime.Remove(WUnit);
		}
	}

	// PER-TICK RE-ASSERT (every UpdateInterval ~0.1s, NOT just the 1s ReassertTimer): keep the disable
	// enforced on every still-held unit so any transient failure — e.g. the deferred SwitchEntityTagByState
	// (Idle) not flushing one frame, or a processor re-acquiring — self-heals within 0.1s. Also covers units
	// inside the hysteresis grace window (held but momentarily undetected).
	for (const TWeakObjectPtr<AUnitBase>& WUnit : AffectedUnits)
	{
		if (AUnitBase* U = WUnit.Get())
		{
			if (UCrowdControlStateComponent* CC = U->FindComponentByClass<UCrowdControlStateComponent>())
			{
				CC->ReassertDisable();
			}
		}
	}
}

void ACrowdControlMarker::ReleaseAllAffected()
{
	const bool bMove = (EffectType == ECrowdControlEffectType::DisableMovement || EffectType == ECrowdControlEffectType::DisableBoth);
	const bool bAttack = (EffectType == ECrowdControlEffectType::DisableAttack || EffectType == ECrowdControlEffectType::DisableBoth);

	for (const TWeakObjectPtr<AUnitBase>& WUnit : AffectedUnits)
	{
		if (AUnitBase* U = WUnit.Get())
		{
			if (UCrowdControlStateComponent* CC = U->FindComponentByClass<UCrowdControlStateComponent>())
			{
				if (bFreezeUnits)
				{
					CC->ReleaseMovementDisable();
					CC->ReleaseAttackDisable();
					CC->ReleaseFreeze();
				}
				else
				{
					if (bMove) CC->ReleaseMovementDisable();
					if (bAttack) CC->ReleaseAttackDisable();
				}
			}
		}
	}
	AffectedUnits.Empty();
	LastSeenTime.Empty();
}

void ACrowdControlMarker::ReassertAffected()
{
	for (const TWeakObjectPtr<AUnitBase>& WUnit : AffectedUnits)
	{
		if (AUnitBase* U = WUnit.Get())
		{
			if (UCrowdControlStateComponent* CC = U->FindComponentByClass<UCrowdControlStateComponent>())
			{
				CC->ReassertDisable();
			}
		}
	}
}

void ACrowdControlMarker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
		World->GetTimerManager().ClearTimer(ReassertTimer);
		World->GetTimerManager().ClearTimer(ClientVisualTimer);
	}

	// Client: un-pause any SKM animations this marker froze.
	if (!HasAuthority())
	{
		RestoreAllClientFrozen();
	}

	if (bIsField && HasAuthority())
	{
		// Restore the caster's CanAttack (paired with the HoldAttackDisable in InitField).
		if (AUnitBase* Caster = OwnerUnit.Get())
		{
			if (UCrowdControlStateComponent* CasterCC = Caster->FindComponentByClass<UCrowdControlStateComponent>())
			{
				CasterCC->ReleaseAttackDisable();
			}
			// Stop rotate-to-mouse so the caster goes back to normal facing once the field is gone.
			if (bRotateToMouse)
			{
				StopRotateToMouse(Caster);
			}
		}
		ReleaseAllAffected();
	}
	Super::EndPlay(EndPlayReason);
}

void ACrowdControlMarker::OnRep_Visuals()
{
	ConfigureVisuals();
}

void ACrowdControlMarker::ConfigureVisuals()
{
	const bool bWantLight = (Mode == ECrowdControlMarkerMode::Light || Mode == ECrowdControlMarkerMode::Both);
	const bool bWantVFX = (Mode == ECrowdControlMarkerMode::VFX || Mode == ECrowdControlMarkerMode::Both);
	const bool bCone = (Shape == ECrowdControlShape::Cone);

	// Sphere / Ring -> point light, tuned to spread nearly evenly across the radius (top-down).
	if (PointLight)
	{
		if (bWantLight && !bCone)
		{
			PointLight->SetAttenuationRadius(FMath::Max(50.f, Radius));
			PointLight->SetSourceRadius(FMath::Max(10.f, Radius * 0.5f));
			PointLight->bUseInverseSquaredFalloff = false;
			PointLight->SetLightFalloffExponent(FMath::Max(0.01f, EvenLightFalloffExponent));
			PointLight->SetIntensity(LightIntensity);
			PointLight->SetLightColor(MarkerColor);
			PointLight->SetVisibility(true);
		}
		else
		{
			PointLight->SetVisibility(false);
		}
	}

	// Cone -> spot light (flashlight feel), cone angle = the detection half-angle.
	if (SpotLight)
	{
		if (bWantLight && bCone)
		{
			// Optional pitch relative to the unit's aim (0 = straight along the unit direction).
			SpotLight->SetRelativeRotation(FRotator(SpotLightPitchDeg, 0.f, 0.f));
			// Cover the full trace: reach = Radius, outer cone = the detection half-angle.
			SpotLight->SetAttenuationRadius(FMath::Max(50.f, Radius));
			const float OuterDeg = FMath::Clamp(HalfAngleDeg, 1.f, 80.f);
			SpotLight->SetOuterConeAngle(OuterDeg);
			// Soft, wide angular fade at the edges (inner cone well inside the outer).
			SpotLight->SetInnerConeAngle(FMath::Clamp(OuterDeg * ConeInnerAngleFraction, 0.f, OuterDeg));
			SpotLight->SetSourceRadius(FMath::Max(10.f, Radius * 0.25f));
			SpotLight->SetSoftSourceRadius(FMath::Max(10.f, Radius * 0.5f));
			// Non-inverse-squared falloff (so the same intensity as the point light works), but with a
			// higher exponent than the sphere so the beam FADES OUT toward the far end like a flashlight
			// instead of stopping abruptly at the attenuation radius.
			SpotLight->bUseInverseSquaredFalloff = false;
			SpotLight->SetLightFalloffExponent(FMath::Max(0.01f, ConeLightFalloffExponent));
			SpotLight->SetIntensity(LightIntensity);
			SpotLight->SetLightColor(MarkerColor);
			SpotLight->SetVisibility(true);
		}
		else
		{
			SpotLight->SetVisibility(false);
		}
	}

	if (NiagaraComp)
	{
		if (bWantVFX)
		{
			const float ScaledRadius = Radius * NiagaraRadiusScale;
			NiagaraComp->SetWorldScale3D(FVector(FMath::Max(0.01f, ScaledRadius)));
			NiagaraComp->SetVariableFloat(FName(TEXT("Radius")), Radius);
			NiagaraComp->SetVariableLinearColor(FName(TEXT("Color")), MarkerColor);
			NiagaraComp->SetVisibility(true);
			if (NiagaraComp->GetAsset())
			{
				NiagaraComp->Activate(true);
			}
		}
		else
		{
			NiagaraComp->Deactivate();
			NiagaraComp->SetVisibility(false);
		}
	}
}

void ACrowdControlMarker::DetectUnitsInArea(UWorld* World, AUnitBase* Caster, const FVector& Origin, FVector Direction,
	ECrowdControlShape InShape, float InRadius, float HalfHeight, float InHalfAngleDeg, int32 MaxTargets,
	float InRingInnerFraction, int32 InConeTraceSteps, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	bool bInIgnoreCaster, bool bInIncludeDeadUnits, bool bInDebugDraw, float InDebugDrawDuration,
	TArray<AUnitBase*>& OutUnits)
{
	OutUnits.Reset();
	if (!World || !Caster)
	{
		return;
	}

	const int32 CasterTeam = Caster->TeamId;
	const float HalfAngleRad = FMath::DegreesToRadians(InHalfAngleDeg);
	const float CosHalfAngle = FMath::Cos(HalfAngleRad);
	const float InnerRadius = InRadius * FMath::Clamp(InRingInnerFraction, 0.f, 0.99f);

	FVector Dir = Direction.GetSafeNormal2D();
	if (Dir.IsNearlyZero())
	{
		Dir = Caster->GetActorForwardVector().GetSafeNormal2D();
		if (Dir.IsNearlyZero()) Dir = FVector::ForwardVector;
	}

	TArray<AActor*> Candidates;
	if (InShape == ECrowdControlShape::Cone)
	{
		CC_GatherCone(World, Origin, Dir, InRadius, InHalfAngleDeg, InConeTraceSteps, ObjectTypes, bInDebugDraw, InDebugDrawDuration, Candidates);
	}
	else
	{
		CC_GatherSphere(World, Origin, FMath::Max(InRadius, HalfHeight), ObjectTypes, bInDebugDraw, InDebugDrawDuration, Candidates);
	}

	struct FScored { AUnitBase* Unit = nullptr; float DistSq = 0.f; };
	TArray<FScored> Scored;
	TSet<AUnitBase*> Seen;

	for (AActor* A : Candidates)
	{
		AUnitBase* U = Cast<AUnitBase>(A);
		if (!U || Seen.Contains(U)) continue;
		if (bInIgnoreCaster && U == Caster) continue;
		if (!bInIncludeDeadUnits && U->GetUnitState() == UnitData::Dead) continue;
		if (U->TeamId == CasterTeam) continue; // enemies only

		const FVector ToUnit = U->GetActorLocation() - Origin;
		if (FMath::Abs(ToUnit.Z) > HalfHeight) continue;

		const FVector Flat(ToUnit.X, ToUnit.Y, 0.f);
		const float Dist2D = Flat.Size();

		switch (InShape)
		{
		case ECrowdControlShape::Sphere:
			if (Dist2D > InRadius) continue;
			break;
		case ECrowdControlShape::Ring:
			if (Dist2D > InRadius || Dist2D < InnerRadius) continue;
			break;
		case ECrowdControlShape::Cone:
			if (Dist2D > InRadius) continue;
			if (Dist2D > KINDA_SMALL_NUMBER)
			{
				if (FVector::DotProduct(Flat / Dist2D, Dir) < CosHalfAngle) continue;
			}
			break;
		}

		Seen.Add(U);
		Scored.Add({ U, Dist2D * Dist2D });
	}

	Scored.Sort([](const FScored& L, const FScored& R) { return L.DistSq < R.DistSq; });

	for (const FScored& S : Scored)
	{
		if (MaxTargets > 0 && OutUnits.Num() >= MaxTargets) break;
		OutUnits.Add(S.Unit);
	}

	if (bInDebugDraw)
	{
		const FColor ShapeColor = FColor::Green;
		const FVector Top = Origin + FVector(0.f, 0.f, HalfHeight);
		const FVector Bottom = Origin - FVector(0.f, 0.f, HalfHeight);
		switch (InShape)
		{
		case ECrowdControlShape::Sphere:
			DrawDebugCylinder(World, Bottom, Top, InRadius, 32, ShapeColor, false, InDebugDrawDuration, 0, 2.f);
			break;
		case ECrowdControlShape::Ring:
			DrawDebugCylinder(World, Bottom, Top, InRadius, 32, ShapeColor, false, InDebugDrawDuration, 0, 2.f);
			DrawDebugCylinder(World, Bottom, Top, InnerRadius, 24, FColor::Yellow, false, InDebugDrawDuration, 0, 2.f);
			break;
		case ECrowdControlShape::Cone:
			DrawDebugCone(World, Origin, Dir, InRadius, HalfAngleRad, HalfAngleRad, 24, ShapeColor, false, InDebugDrawDuration, 0, 2.f);
			break;
		}
		for (const AUnitBase* U : OutUnits)
		{
			if (!U) continue;
			const FVector Loc = U->GetActorLocation();
			DrawDebugSphere(World, Loc, 60.f, 12, FColor::Red, false, InDebugDrawDuration, 0, 2.f);
			DrawDebugLine(World, Origin, Loc, FColor::Red, false, InDebugDrawDuration, 0, 2.f);
		}
	}
}
