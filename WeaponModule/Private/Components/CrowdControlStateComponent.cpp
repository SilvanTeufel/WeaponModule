// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Components/CrowdControlStateComponent.h"
#include "Characters/Unit/UnitBase.h"
#include "Characters/Unit/MassUnitBase.h" // GetMassEntityData, SwitchEntityTagByState
#include "Mass/UnitMassTag.h"             // FMassAIStateFragment (POD), FMassStateDetectTag
#include "Core/UnitData.h"
#include "MassEntityManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"          // GlobalAnimRateScale (skeletal anim freeze)
#include "Components/InstancedStaticMeshComponent.h"   // ISM SetCustomDataValue / NumCustomDataFloats
#include "Animations/UnitAnimationProcessor.h"         // FUnitAnimationFragment (Current/PrevStartTime)
#include "Mass/MassUnitVisualFragments.h"              // FMassUnitVisualFragment (VisualInstances / bUseSkeletalMovement)

UCrowdControlStateComponent::UCrowdControlStateComponent()
{
	// Ticks per-frame ONLY between FreezeInternal/UnfreezeInternal to hold the ISM VAT clock (the material
	// advances the vertex animation every frame, so a 10Hz timer would visibly stutter — this must be per-frame).
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(false); // server-only helper; the affected bools replicate via AUnitBase
}

UCrowdControlStateComponent* UCrowdControlStateComponent::FindOrAdd(AActor* Owner)
{
	if (!IsValid(Owner) || !Owner->IsA(AUnitBase::StaticClass()))
	{
		return nullptr;
	}

	if (UCrowdControlStateComponent* Existing = Owner->FindComponentByClass<UCrowdControlStateComponent>())
	{
		return Existing;
	}

	UCrowdControlStateComponent* NewComp = NewObject<UCrowdControlStateComponent>(Owner);
	if (NewComp)
	{
		NewComp->RegisterComponent();
	}
	return NewComp;
}

AUnitBase* UCrowdControlStateComponent::GetUnit() const
{
	return Cast<AUnitBase>(GetOwner());
}

// ---------- internal apply/restore (run when a count crosses 0<->1, authority assumed) ----------
void UCrowdControlStateComponent::DisableMovementInternal()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit) return;

	bOriginalCanMove = Unit->CanMove;
	Unit->CanMove = false;

	// Write the Mass AI fragment directly so the movement processors gate THIS tick (the
	// actor->fragment sync runs PrePhysics and would otherwise lag a frame).
	bool bWroteFragment = false;
	FMassEntityManager* EM = nullptr;
	FMassEntityHandle EntityHandle;
	if (Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle))
	{
		if (FMassAIStateFragment* AI = EM->GetFragmentDataPtr<FMassAIStateFragment>(EntityHandle))
		{
			AI->CanMove = false;
			bWroteFragment = true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[CC] DisableMovement '%s' CanMove(actor)=%d fragWritten=%d auth=%d"),
		*Unit->GetName(), Unit->CanMove, bWroteFragment, (int32)Unit->HasAuthority());
}

void UCrowdControlStateComponent::RestoreMovementInternal()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit) return;

	Unit->CanMove = bOriginalCanMove;
	FMassEntityManager* EM = nullptr;
	FMassEntityHandle EntityHandle;
	if (Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle))
	{
		if (FMassAIStateFragment* AI = EM->GetFragmentDataPtr<FMassAIStateFragment>(EntityHandle))
		{
			AI->CanMove = bOriginalCanMove;
		}
	}
}

void UCrowdControlStateComponent::DisableAttackInternal()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit) return;

	bOriginalCanAttack = Unit->CanAttack;
	Unit->CanAttack = false;

	// Immediate fragment write (POD AI-state fragment) so DetectionProcessor drops the target this tick.
	FMassEntityManager* EM = nullptr;
	FMassEntityHandle EntityHandle;
	if (Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle))
	{
		if (FMassAIStateFragment* AI = EM->GetFragmentDataPtr<FMassAIStateFragment>(EntityHandle))
		{
			AI->CanAttack = false;
		}
		// IMMEDIATE belt: strip the Attack/Chase state tags synchronously. SwitchEntityTagByState (below)
		// removes them via Defer(), which can lag a frame from this world-timer context; AttackStateProcessor
		// requires the Attack tag, so removing it now stops an in-progress attack this instant.
		EM->RemoveTagFromEntity(EntityHandle, FMassStateAttackTag::StaticStruct());
		EM->RemoveTagFromEntity(EntityHandle, FMassStateChaseTag::StaticStruct());
	}

	// Clear the current target so Idle/Chase don't re-engage (they don't check CanAttack).
	// FMassAITargetFragment is not trivially copyable -> use the public helper (clears it inside RTSUnitTemplate).
	const bool bClearedTarget = Unit->RemoveFocusEntityTarget();

	// Drop the Attack/Chase/Run state tag + set the unit state to Idle (SetUnitState is immediate; the tag
	// ops inside are deferred — the immediate strip above covers the lag). AttackStateProcessor never checks
	// CanAttack, so only removing the tag stops an in-progress attack.
	// FREEZE EXCEPTION: while frozen (FreezeCount>0) do NOT switch to Idle. The ISM vertex-animation freeze
	// needs the unit to keep its CURRENT animation state so it freezes mid-motion (not snapped to Idle).
	// FreezeInternal removes the primary state tags instead, leaving the enum with no primary tag —
	// UnitClientTagSyncProcessor leaves that untouched (ComputeState->None is not applied) => no re-anchor.
	const bool bSwitched = (FreezeCount == 0) ? Unit->SwitchEntityTagByState(UnitData::Idle, UnitData::Idle) : false;

	UE_LOG(LogTemp, Warning, TEXT("[CC] DisableAttack '%s' CanAttack(actor)=%d clearedTarget=%d switchedIdle=%d State(after)=%d auth=%d"),
		*Unit->GetName(), Unit->CanAttack, bClearedTarget, bSwitched, (int32)Unit->GetUnitState(), (int32)Unit->HasAuthority());
}

void UCrowdControlStateComponent::RestoreAttackInternal()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit) return;

	Unit->CanAttack = bOriginalCanAttack;
	FMassEntityManager* EM = nullptr;
	FMassEntityHandle EntityHandle;
	if (Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle))
	{
		if (FMassAIStateFragment* AI = EM->GetFragmentDataPtr<FMassAIStateFragment>(EntityHandle))
		{
			AI->CanAttack = bOriginalCanAttack;
		}
		// We dropped the Detect tag by switching to Idle while CanAttack was false; re-add it so the unit
		// resumes acquiring/attacking once the disable ends. IMMEDIATE (not Defer()): this runs from the
		// marker's world-timer where deferred commands don't reliably flush — the deferred add was why units
		// that left the cone kept CanAttack=true but never re-acquired/attacked.
		if (bOriginalCanAttack)
		{
			EM->AddTagToEntity(EntityHandle, FMassStateDetectTag::StaticStruct());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[CC] RestoreAttack '%s' CanAttack(actor)=%d reAddedDetect=%d auth=%d"),
		*Unit->GetName(), Unit->CanAttack, (int32)bOriginalCanAttack, (int32)Unit->HasAuthority());
}

// ---------- movement (ref-counted) ----------
void UCrowdControlStateComponent::HoldMovementDisable()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	if (MovementDisableCount <= 0)
	{
		MovementDisableCount = 0;
		DisableMovementInternal();
	}
	++MovementDisableCount;
}

void UCrowdControlStateComponent::ReleaseMovementDisable()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	MovementDisableCount = FMath::Max(0, MovementDisableCount - 1);
	if (MovementDisableCount == 0)
	{
		RestoreMovementInternal();
	}
}

void UCrowdControlStateComponent::PushMovementDisable(float Duration)
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	HoldMovementDisable();

	if (UWorld* World = GetWorld())
	{
		FTimerHandle TimerHandle;
		TWeakObjectPtr<UCrowdControlStateComponent> WeakThis(this);
		World->GetTimerManager().SetTimer(TimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid()) WeakThis->ReleaseMovementDisable();
		}, FMath::Max(0.01f, Duration), false);
		ActiveTimers.Add(TimerHandle);
	}
}

// ---------- attack (ref-counted) ----------
void UCrowdControlStateComponent::HoldAttackDisable()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	if (AttackDisableCount <= 0)
	{
		AttackDisableCount = 0;
		DisableAttackInternal();
	}
	++AttackDisableCount;
}

void UCrowdControlStateComponent::ReleaseAttackDisable()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	AttackDisableCount = FMath::Max(0, AttackDisableCount - 1);
	if (AttackDisableCount == 0)
	{
		RestoreAttackInternal();
	}
}

void UCrowdControlStateComponent::PushAttackDisable(float Duration)
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	HoldAttackDisable();

	if (UWorld* World = GetWorld())
	{
		FTimerHandle TimerHandle;
		TWeakObjectPtr<UCrowdControlStateComponent> WeakThis(this);
		World->GetTimerManager().SetTimer(TimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid()) WeakThis->ReleaseAttackDisable();
		}, FMath::Max(0.01f, Duration), false);
		ActiveTimers.Add(TimerHandle);
	}
}

void UCrowdControlStateComponent::FreezeInternal()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit) return;

	// NO FMassStateFrozenTag. That tag abruptly excludes the unit from every Mass movement/state processor
	// AND replicates as a control bit; removing it left the client's locally-derived move state diverging from
	// the server (units "tried to move" / desynced after unfreeze). The freeze is now exactly the reliable
	// CanMove=false + CanAttack=false (applied by the field's HoldMovement/AttackDisable) PLUS pausing the
	// skeletal animation here — same proven pipeline as the non-freeze crowd-control, so no desync.
	//
	// Pause the SKELETAL animation. APerformanceUnit re-sets SkelMesh->bPauseAnims = !visibility EVERY
	// visibility tick (PerformanceUnit.cpp:125), so a visible unit keeps animating; GlobalAnimRateScale is
	// NOT touched anywhere in RTSUnitTemplate, so setting it to 0 freezes the pose without being fought
	// (restored to CachedAnimRateScale on unfreeze). NOTE: GlobalAnimRateScale is server-only (not replicated)
	// -> remote clients drive their own copy from ACrowdControlMarker::ClientVisualFreezeTick.
	if (USkeletalMeshComponent* Mesh = Unit->GetMesh())
	{
		CachedAnimRateScale = Mesh->GlobalAnimRateScale;
		Mesh->GlobalAnimRateScale = 0.f;
	}

	// ISM (vertex/VAT) freeze — two parts:
	// (1) Prevent an animation RE-ANCHOR: remove the primary state tags so the unit has NONE. GetUnitState()
	//     then stays put (UnitClientTagSyncProcessor's ComputeState returns None, which ApplyStateToActor
	//     refuses to apply), so UUnitAnimationProcessor never sees a state change and never re-anchors the ISM
	//     custom data — the slots stay on the current (mid-motion) pose. The unit is also already excluded from
	//     the anim processor (CanMove=false -> StopMovement -> StopAnimation tag). We deliberately do NOT add a
	//     Frozen/StopMovement CONTROL tag here (that path desynced clients — see UnfreezeInternal's note); we
	//     only REMOVE tags, which replicates cleanly so remote clients suppress the re-anchor too.
	// (2) The material still advances the frame from global Time, so the real clock freeze happens per-frame in
	//     TickComponent via HoldISMVertexClock (holding the StartTime custom-data floats in place).
	{
		FMassEntityManager* EM = nullptr;
		FMassEntityHandle EntityHandle;
		if (Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle))
		{
			EM->RemoveTagFromEntity(EntityHandle, FMassStateRunTag::StaticStruct());
			EM->RemoveTagFromEntity(EntityHandle, FMassStateAttackTag::StaticStruct());
			EM->RemoveTagFromEntity(EntityHandle, FMassStateChaseTag::StaticStruct());
		}
	}
	ISMAnchor = FISMFreezeAnchor{};   // fresh capture for this freeze
	SetComponentTickEnabled(true);     // start the per-frame ISM VAT clock hold

	UE_LOG(LogTemp, Warning, TEXT("[CC] Freeze(anim) '%s' hasMesh=%d auth=%d"),
		*Unit->GetName(), Unit->GetMesh() ? 1 : 0, (int32)Unit->HasAuthority());
}

void UCrowdControlStateComponent::UnfreezeInternal()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit) return;

	// Restore the skeletal animation play rate (guard against a stale 0 cache). No tag to remove.
	if (USkeletalMeshComponent* Mesh = Unit->GetMesh())
	{
		Mesh->GlobalAnimRateScale = (CachedAnimRateScale > 0.f) ? CachedAnimRateScale : 1.f;
	}

	// Stop the ISM VAT clock hold (the material resumes advancing from the last-held frame).
	SetComponentTickEnabled(false);

	// CRITICAL: return the unit to a valid resting state. FreezeInternal left it in "limbo" — its old enum with
	// NO primary state tag — so the ISM freeze could hold the mid-motion pose without re-anchoring. That is
	// stable only WHILE frozen; on release the unit must get a real state again or it HANGS: no state
	// processor's query matches a tag-less unit, so it neither moves, re-acquires, nor re-anchors its
	// animation. Switch to Idle (exactly the state the pre-change freeze left it in), which also makes the anim
	// processor re-anchor to the Idle clip. Switching to Idle drops the Detect tag, so re-add it (gated on the
	// now-restored CanAttack — ReleaseAttackDisable runs before ReleaseFreeze) so the unit resumes acquiring.
	// Skip entirely if the unit died while frozen.
	if (Unit->GetUnitState() != UnitData::Dead)
	{
		Unit->SwitchEntityTagByState(UnitData::Idle, UnitData::Idle);

		if (Unit->CanAttack)
		{
			FMassEntityManager* EM = nullptr;
			FMassEntityHandle EntityHandle;
			if (Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle))
			{
				EM->AddTagToEntity(EntityHandle, FMassStateDetectTag::StaticStruct());
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[CC] Unfreeze(anim) '%s'"), *Unit->GetName());
}

void UCrowdControlStateComponent::HoldFreeze()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	if (FreezeCount <= 0)
	{
		FreezeCount = 0;
		FreezeInternal();
	}
	++FreezeCount;
}

void UCrowdControlStateComponent::ReleaseFreeze()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	FreezeCount = FMath::Max(0, FreezeCount - 1);
	if (FreezeCount == 0)
	{
		UnfreezeInternal();
	}
}

void UCrowdControlStateComponent::PushFreeze(float Duration)
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;

	HoldFreeze();

	if (UWorld* World = GetWorld())
	{
		FTimerHandle TimerHandle;
		TWeakObjectPtr<UCrowdControlStateComponent> WeakThis(this);
		World->GetTimerManager().SetTimer(TimerHandle, [WeakThis]()
		{
			if (WeakThis.IsValid()) WeakThis->ReleaseFreeze();
		}, FMath::Max(0.01f, Duration), false);
		ActiveTimers.Add(TimerHandle);
	}
}

void UCrowdControlStateComponent::ReassertDisable()
{
	AUnitBase* Unit = GetUnit();
	if (!Unit || !Unit->HasAuthority()) return;
	if (MovementDisableCount <= 0 && AttackDisableCount <= 0 && FreezeCount <= 0) return;

	FMassEntityManager* EM = nullptr;
	FMassEntityHandle EntityHandle;
	const bool bHaveEntity = Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle);
	FMassAIStateFragment* AI = bHaveEntity ? EM->GetFragmentDataPtr<FMassAIStateFragment>(EntityHandle) : nullptr;

	if (MovementDisableCount > 0)
	{
		Unit->CanMove = false;
		if (AI) AI->CanMove = false;
	}

	if (AttackDisableCount > 0)
	{
		Unit->CanAttack = false;
		if (AI) AI->CanAttack = false;
		Unit->RemoveFocusEntityTarget();
		// Immediate belt: strip Attack/Chase tags synchronously (stops an in-progress attack this instant).
		if (bHaveEntity)
		{
			EM->RemoveTagFromEntity(EntityHandle, FMassStateAttackTag::StaticStruct());
			EM->RemoveTagFromEntity(EntityHandle, FMassStateChaseTag::StaticStruct());
			// While frozen, ALSO keep the Run tag stripped so RunStateProcessor can't flip the unit to Idle on
			// arrival (velocity->0) — that would re-anchor the ISM animation off its held mid-motion pose.
			if (FreezeCount > 0)
			{
				EM->RemoveTagFromEntity(EntityHandle, FMassStateRunTag::StaticStruct());
			}
		}
		// Only re-switch if the unit slipped out of Idle (back into Attack/Chase) — re-fires
		// SetUnitState(Idle) which resets the state and animation. NOT while frozen: the freeze deliberately
		// keeps the unit's current animation state (no primary tag) so the ISM freeze holds the mid-motion pose.
		if (FreezeCount == 0 && Unit->GetUnitState() != UnitData::Idle)
		{
			Unit->SwitchEntityTagByState(UnitData::Idle, UnitData::Idle);
		}
	}

	// Keep the skeletal anim freeze re-applied (idempotent) in case the visibility tick / anything reset the rate.
	if (FreezeCount > 0)
	{
		if (USkeletalMeshComponent* Mesh = Unit->GetMesh())
		{
			Mesh->GlobalAnimRateScale = 0.f;
		}
	}
}

void UCrowdControlStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TM = World->GetTimerManager();
		for (FTimerHandle& Handle : ActiveTimers)
		{
			TM.ClearTimer(Handle);
		}
	}
	ActiveTimers.Reset();

	// Restore any still-active disable so the unit is never left stuck if this component is removed.
	if (AUnitBase* Unit = GetUnit())
	{
		if (Unit->HasAuthority())
		{
			if (MovementDisableCount > 0)
			{
				RestoreMovementInternal();
			}
			if (AttackDisableCount > 0)
			{
				RestoreAttackInternal();
			}
			if (FreezeCount > 0)
			{
				UnfreezeInternal();
			}
		}
	}
	MovementDisableCount = 0;
	AttackDisableCount = 0;
	FreezeCount = 0;

	Super::EndPlay(EndPlayReason);
}

void UCrowdControlStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Server/host only (this component lives on the authority; remote clients drive their own copy from
	// ACrowdControlMarker::Tick). Self-disable once the freeze is released; nothing to render on a dedicated
	// server, so skip the per-instance writes there.
	if (FreezeCount <= 0)
	{
		SetComponentTickEnabled(false);
		return;
	}
	const UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	HoldISMVertexClock(GetUnit(), World->GetTimeSeconds(), ISMAnchor, StartTimeCustomDataIndex, PrevStartTimeCustomDataIndex);
}

void UCrowdControlStateComponent::HoldISMVertexClock(AUnitBase* Unit, float NowSeconds, FISMFreezeAnchor& Anchor,
	int32 StartIdx, int32 PrevStartIdx)
{
	if (!IsValid(Unit))
	{
		return;
	}

	FMassEntityManager* EM = nullptr;
	FMassEntityHandle EntityHandle;
	if (!(Unit->GetMassEntityData(EM, EntityHandle) && EM && EM->IsEntityValid(EntityHandle)))
	{
		return;
	}

	const FMassUnitVisualFragment* Visual = EM->GetFragmentDataPtr<FMassUnitVisualFragment>(EntityHandle);
	const FUnitAnimationFragment*  Anim   = EM->GetFragmentDataPtr<FUnitAnimationFragment>(EntityHandle);
	if (!Visual || !Anim || Visual->bUseSkeletalMovement)
	{
		return; // skeletal units freeze via GlobalAnimRateScale, not the VAT clock
	}

	// Resolve the rendered instance exactly like UUnitAnimationProcessor: prefer the visual fragment, fall back
	// to the actor's own ISM (they can diverge under the pooled/shared ISM subsystem).
	UInstancedStaticMeshComponent* ISM = nullptr;
	int32 InstanceIndex = INDEX_NONE;
	if (Visual->VisualInstances.Num() > 0)
	{
		ISM = Visual->VisualInstances[0].TargetISM.Get();
		InstanceIndex = Visual->VisualInstances[0].InstanceIndex;
	}
	if (InstanceIndex == INDEX_NONE)
	{
		if (AMassUnitBase* MassUnit = Cast<AMassUnitBase>(Unit))
		{
			ISM = MassUnit->ISMComponent;
			InstanceIndex = MassUnit->InstanceIndex;
		}
	}
	if (!ISM || InstanceIndex == INDEX_NONE || ISM->NumCustomDataFloats <= FMath::Max(StartIdx, PrevStartIdx))
	{
		return;
	}

	// Hold (globalTime - StartTime) constant so the material samples a fixed VAT frame. Absolute recompute
	// (StartTime = Now - Elapsed) => no floating-point drift. Elapsed is captured once from the anim fragment
	// (which the excluded anim processor no longer updates); re-captured if StartTime ever changes (a re-anchor
	// slipped through), so it always freezes whatever frame is currently showing. Primary = current animation
	// (custom-data StartTime idx 3), secondary = the transition/prev animation (idx 7).
	const float S = Anim->CurrentStartTime;
	if (!Anchor.bHasPrim || S != Anchor.PrimStart)
	{
		Anchor.bHasPrim = true;
		Anchor.PrimStart = S;
		Anchor.PrimElapsed = NowSeconds - S;
	}
	ISM->SetCustomDataValue(InstanceIndex, StartIdx, NowSeconds - Anchor.PrimElapsed, /*bMarkRenderStateDirty*/ false);

	const float P = Anim->PrevStartTime;
	if (!Anchor.bHasSec || P != Anchor.SecStart)
	{
		Anchor.bHasSec = true;
		Anchor.SecStart = P;
		Anchor.SecElapsed = NowSeconds - P;
	}
	// Mark the render state dirty once per instance (this last write), covering both custom-data updates.
	ISM->SetCustomDataValue(InstanceIndex, PrevStartIdx, NowSeconds - Anchor.SecElapsed, /*bMarkRenderStateDirty*/ true);
}
