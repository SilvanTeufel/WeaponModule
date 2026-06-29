// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Components/CrowdControlStateComponent.h"
#include "Characters/Unit/UnitBase.h"
#include "Characters/Unit/MassUnitBase.h" // GetMassEntityData, SwitchEntityTagByState
#include "Mass/UnitMassTag.h"             // FMassAIStateFragment (POD), FMassStateDetectTag
#include "Core/UnitData.h"
#include "MassEntityManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h" // GlobalAnimRateScale (skeletal anim freeze)

UCrowdControlStateComponent::UCrowdControlStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
	const bool bSwitched = Unit->SwitchEntityTagByState(UnitData::Idle, UnitData::Idle);

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
		}
		// Only re-switch if the unit slipped out of Idle (back into Attack/Chase) — re-fires
		// SetUnitState(Idle) which resets the state and animation.
		if (Unit->GetUnitState() != UnitData::Idle)
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
