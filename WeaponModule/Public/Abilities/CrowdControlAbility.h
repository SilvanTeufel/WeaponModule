// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h" // EObjectTypeQuery
#include "Abilities/WeaponAbilityBase.h"
#include "Actors/CrowdControlMarker.h" // ECrowdControlMarkerMode + ACrowdControlMarker
#include "Components/WeaponComponent.h" // FCrowdControlData (per-ability base values)
#include "CrowdControlAbility.generated.h"

class AUnitBase;
class UWeaponComponent;

// ECrowdControlShape, ECrowdControlEffectType and ECrowdControlMarkerMode are defined in
// CrowdControlMarker.h (included above) so the marker and the ability can share them without a
// circular include.

/** Which variant ActivateCrowdControl() dispatches to. */
UENUM(BlueprintType)
enum class ECrowdControlActivationMode : uint8
{
	AtClick      UMETA(DisplayName = "A: At Click (origin = clicked location)"),
	SelfCentered UMETA(DisplayName = "B: Self-Centered (origin = caster, forward)"),
	Modular      UMETA(DisplayName = "C: Modular (detect + marker + apply at click)")
};

/**
 * A "Suppression Field" crowd-control ability.
 *
 * It does NOT auto-execute anything: it only exposes Blueprint-callable building
 * blocks that the designer wires together in the ability Blueprint (3.1):
 *   1) DetectUnitsInArea       - find enemy units (TeamId != caster) in the chosen shape
 *   2) ShowAreaMarker          - spawn the Light / VFX area marker
 *   3) ApplyCrowdControlToUnits - set CanMove/CanAttack=false for the tuned duration
 *   4) ExecuteCrowdControlField - convenience that runs all three at once
 *
 * Every tunable number (radius, duration, cone angle, max targets, vertical reach,
 * cooldown) is read from the owning UWeaponComponent's FCrowdControlData, so spent
 * talent points directly change what these functions do (6.1).
 */
UCLASS()
class WEAPONMODULE_API UCrowdControlAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	UCrowdControlAbility();

	// --- Configuration ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Config")
	ECrowdControlShape Shape = ECrowdControlShape::Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Config")
	ECrowdControlMarkerMode MarkerMode = ECrowdControlMarkerMode::Light;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Config")
	ECrowdControlEffectType EffectType = ECrowdControlEffectType::DisableBoth;

	/** Which variant the single ActivateCrowdControl() entry point runs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Config")
	ECrowdControlActivationMode ActivationMode = ECrowdControlActivationMode::AtClick;

	/** If true, affected units are frozen: CanMove=false + CanAttack=false AND their SKELETAL animation pauses
	 *  (GlobalAnimRateScale=0, mirrored on remote clients by the marker). If false, only CanMove/CanAttack +
	 *  Idle is applied (animation plays idle). No Mass freeze tag is used (it desynced clients). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Config")
	bool bFreezeUnits = false;

	// --- Mana drain (flashlight field) ---
	/** Mana drained from the caster every ManaDrainInterval while the field is active. 0 = no drain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Mana")
	float ManaPerInterval = 0.f;

	/** Seconds between mana drains (also the field upkeep/watchdog interval). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Mana", meta = (ClampMin = "0.05"))
	float ManaDrainInterval = 1.0f;

	// --- Marker look ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Marker")
	TSubclassOf<ACrowdControlMarker> MarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Marker")
	FLinearColor MarkerColor = FLinearColor(0.2f, 0.4f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Marker")
	float MarkerLightIntensity = 5000.f;

	/** Skeletal-mesh socket the flashlight field attaches to (so the light/area follow the unit's hand).
	 *  Falls back to the mesh / root if the socket doesn't exist. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Marker")
	FName HandSocketName = TEXT("rightHandSocket");

	// --- Per-ability base values ---
	/** If true, this ability uses its own AbilityBaseValues (range, duration, angle, etc.) instead of the
	 *  unit's WeaponComponent base values — so different ability BPs can have different ranges. Talents still
	 *  apply: the unit's invested talent LEVELS are combined with THIS ability's base + per-level values. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|BaseValues")
	bool bUseAbilityBaseValues = false;

	/** Per-ability base values + per-level increments (used when bUseAbilityBaseValues is true). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|BaseValues", meta = (EditCondition = "bUseAbilityBaseValues"))
	FCrowdControlData AbilityBaseValues;

	// --- Shape parameters ---
	/** For the Ring shape: inner edge as a fraction of the outer radius. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Shape", meta = (ClampMin = "0.0", ClampMax = "0.99"))
	float RingInnerFraction = 0.5f;

	/** For the Cone shape: how many sphere traces are stepped along the aim direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Shape", meta = (ClampMin = "1", ClampMax = "16"))
	int32 ConeTraceSteps = 5;

	// --- Detection options ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Detection")
	bool bIgnoreCaster = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Detection")
	bool bIncludeDeadUnits = false;

	/** Collision object types the area overlap tests against. Defaults to Pawn (units collide on ECC_Pawn,
	 *  same channel projectiles use to hit them). Add more channels here if your units use a custom one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Detection")
	TArray<TEnumAsByte<EObjectTypeQuery>> DetectionObjectTypes;

	// --- Debug ---
	/** Draw the actual overlap/trace spheres, the resolved shape and the hit units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Debug")
	bool bDebugDraw = false;

	/** How long the debug shapes stay on screen (seconds). Drawn where the function runs (server in the normal cast flow). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Debug", meta = (ClampMin = "0.0"))
	float DebugDrawDuration = 2.0f;

	// --- Blueprint building blocks (call these yourself in the ability BP) ---

	/** Gathers enemy units (TeamId != caster) inside the configured, talent-tuned shape. Returns true if any were found. */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	bool DetectUnitsInArea(FVector Origin, FVector Direction, TArray<AUnitBase*>& OutUnits);

	/** Spawns the Light / VFX area marker (replicated) at Origin, sized to the talent-tuned radius, living for the tuned duration. */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	AActor* ShowAreaMarker(FVector Origin, FVector Direction);

	/** Suppresses CanMove / CanAttack on the given units for the talent-tuned duration (server-authoritative, ref-counted, auto-restores). */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	void ApplyCrowdControlToUnits(const TArray<AUnitBase*>& Units);

	/** Convenience: ShowAreaMarker + DetectUnitsInArea + ApplyCrowdControlToUnits in one call. */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	bool ExecuteCrowdControlField(FVector Origin, FVector Direction, TArray<AUnitBase*>& OutAffectedUnits);

	// =====================================================================================
	// One-input entry points — pass only the clicked location (e.g. HitResult.Location).
	// Call these from OnAbilityCastComplete / OnAbilityMouseHit. Caster, origin and
	// direction are resolved internally. Authority is handled by the underlying functions.
	// =====================================================================================

	/**
	 * Single entry point: dispatches to one of the three variants based on the EditAnywhere ActivationMode.
	 *   AtClick      -> RunCrowdControlAtClick(ClickLocation)
	 *   SelfCentered -> RunCrowdControlSelfCentered()      (ClickLocation ignored)
	 *   Modular      -> DetectUnitsAtClick + ShowAreaMarkerAtClick + ApplyCrowdControlToUnits
	 * Call this from OnAbilityCastComplete / OnAbilityMouseHit and choose the behaviour per ability asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	bool ActivateCrowdControl(FVector ClickLocation, TArray<AUnitBase*>& OutAffectedUnits);

	/**
	 * Variant A — click-targeted AoE.
	 * Origin = ClickLocation, Direction = (ClickLocation - caster location).
	 * Spawns the marker, detects enemies and applies the crowd-control in one call.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	bool RunCrowdControlAtClick(FVector ClickLocation, TArray<AUnitBase*>& OutAffectedUnits);

	/**
	 * Variant B — self-centered AoE.
	 * Origin = caster location, Direction = caster forward vector. ClickLocation is not needed.
	 * Spawns the marker, detects enemies and applies the crowd-control in one call.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	bool RunCrowdControlSelfCentered(TArray<AUnitBase*>& OutAffectedUnits);

	/**
	 * Variant C (modular) — detect enemies around the clicked location only.
	 * Origin = ClickLocation, Direction = (ClickLocation - caster location).
	 * Filter the result yourself, then call ApplyCrowdControlToUnits / ShowAreaMarkerAtClick as you like.
	 */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	bool DetectUnitsAtClick(FVector ClickLocation, TArray<AUnitBase*>& OutUnits);

	/** Variant C (modular) — spawn just the marker at the clicked location. */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	AActor* ShowAreaMarkerAtClick(FVector ClickLocation);

	// =====================================================================================
	// Flashlight FIELD — continuous, caster-attached. Effects apply while units are inside the
	// light/trace and are released the instant they leave. Toggle it from BP (e.g. on every click):
	// if IsCrowdControlFieldActive() -> DeactivateCrowdControlField(), else ActivateCrowdControlField().
	// =====================================================================================

	/** Turn the flashlight ON: spawn the field marker, attach it to the caster (HandSocketName) and start it. */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	bool ActivateCrowdControlField();

	/** Turn the flashlight OFF: destroy the field marker (which releases every still-affected unit). */
	UFUNCTION(BlueprintCallable, Category = "CrowdControl")
	void DeactivateCrowdControlField();

	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	bool IsCrowdControlFieldActive() const;

	// --- Talent-tuned value getters (handy for BP cooldown logic / debug) ---
	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	AUnitBase* GetCasterUnit() const;

	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	float GetEffectiveRadius() const;

	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	float GetEffectiveDuration() const;

	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	float GetEffectiveHalfAngleDeg() const;

	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	int32 GetEffectiveMaxTargets() const;

	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	float GetEffectiveHalfHeight() const;

	UFUNCTION(BlueprintPure, Category = "CrowdControl")
	float GetEffectiveCooldown() const;

private:
	/** Builds the effective FCrowdControlData: this ability's base values (if bUseAbilityBaseValues) or the
	 *  unit's, combined with the unit's invested talent levels. Source of truth for the GetEffective* getters. */
	FCrowdControlData ResolveEffectiveData() const;

	/** Origin = ClickLocation; Direction = (ClickLocation - caster location) (zero if no caster -> internal fallback). */
	void ResolveClickOriginDirection(const FVector& ClickLocation, FVector& OutOrigin, FVector& OutDirection) const;

	/** Periodic while the field is on: drains mana and ends the ability if the field died or mana ran out. */
	void TickFieldUpkeep();

	/** End this ability instance NOW (if active). The flashlight FIELD is independent, so ending keeps the
	 *  unit fully commandable — crucially, an active+cancelable ability would make the first right-click
	 *  cancel the ability instead of issuing a move (TryCancelActiveAbilities). */
	void EndSelf();

	/** The currently active flashlight field marker (if any). */
	UPROPERTY()
	TWeakObjectPtr<ACrowdControlMarker> ActiveField;

	FTimerHandle FieldUpkeepTimer;
};
