// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdControlStateComponent.generated.h"

class AUnitBase;

/**
 * Runtime helper component attached to a target AUnitBase by the Crowd-Control ability.
 *
 * It reference-counts movement / attack disables so that overlapping crowd-control
 * applications (from one or several casters) behave correctly:
 *   - The first push captures the unit's original CanMove / CanAttack value and sets it to false.
 *   - Each push schedules a timed pop after the given duration.
 *   - The last pop restores the captured original value.
 *
 * This guarantees we never leave a unit permanently stuck and never wrongly *enable* a unit
 * that was already disabled by some other system ("falls vorher true").
 *
 * All state mutation happens on the server (authority) only; CanMove / CanAttack are
 * replicated by AUnitBase, so clients follow automatically.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEAPONMODULE_API UCrowdControlStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCrowdControlStateComponent();

	/** Returns the existing component on Owner, or creates and registers a new one. Returns null for non-units. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	static UCrowdControlStateComponent* FindOrAdd(AActor* Owner);

	/** Disable CanMove for Duration seconds (ref-counted, auto-restores). No-op off authority. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void PushMovementDisable(float Duration);

	/** Disable CanAttack for Duration seconds (ref-counted, auto-restores). No-op off authority. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void PushAttackDisable(float Duration);

	// --- Held (no timer) — for the continuous flashlight field. Caller releases on exit. ---
	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void HoldMovementDisable();

	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void ReleaseMovementDisable();

	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void HoldAttackDisable();

	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void ReleaseAttackDisable();

	/** Re-apply the currently-active disables (re-clears target + re-switches to Idle) without changing the
	 *  ref counts. Used by the flashlight field to pull units that slipped back into Attack out again. */
	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void ReassertDisable();

	// --- Freeze: pauses the SKELETAL animation (GlobalAnimRateScale=0). Ref-counted. The move/attack stop
	//     comes from Hold Movement/Attack Disable (CanMove/CanAttack=false) — NOT a Mass tag. (No
	//     FMassStateFrozenTag: it desynced clients by yanking the unit out of the replicated state pipeline.) ---
	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void HoldFreeze();

	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void ReleaseFreeze();

	UFUNCTION(BlueprintCallable, Category = "Weapon|CrowdControl")
	void PushFreeze(float Duration);

	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	bool IsMovementDisabled() const { return MovementDisableCount > 0; }

	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	bool IsAttackDisabled() const { return AttackDisableCount > 0; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	AUnitBase* GetUnit() const;

	// Actual Mass/actor work, applied when a count transitions 0<->1.
	void DisableMovementInternal();
	void RestoreMovementInternal();
	void DisableAttackInternal();
	void RestoreAttackInternal();
	void FreezeInternal();
	void UnfreezeInternal();

	int32 MovementDisableCount = 0;
	int32 AttackDisableCount = 0;
	int32 FreezeCount = 0;

	bool bOriginalCanMove = true;
	bool bOriginalCanAttack = true;

	// Captured skeletal-mesh GlobalAnimRateScale before a freeze, restored on unfreeze. Freezing the SKM via
	// rate=0 is needed because APerformanceUnit re-sets bPauseAnims=!visibility every tick (so Frozen alone
	// can't pause a visible unit's skeletal animation); GlobalAnimRateScale is untouched by RTSUnitTemplate.
	float CachedAnimRateScale = 1.f;

	// Kept so we can cancel pending restores when the component / unit goes away.
	TArray<FTimerHandle> ActiveTimers;
};
