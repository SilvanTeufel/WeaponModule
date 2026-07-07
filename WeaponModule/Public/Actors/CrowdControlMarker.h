// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h" // EObjectTypeQuery
#include "GameFramework/Actor.h"
#include "Components/WeaponComponent.h" // FCrowdControlData (per-ability base values)
#include "Components/CrowdControlStateComponent.h" // FISMFreezeAnchor + HoldISMVertexClock (client ISM freeze)
#include "CrowdControlMarker.generated.h"

class UPointLightComponent;
class USpotLightComponent;
class UNiagaraComponent;
class AUnitBase;

/** How the crowd-control area is visually marked. */
UENUM(BlueprintType)
enum class ECrowdControlMarkerMode : uint8
{
	None  UMETA(DisplayName = "None"),
	Light UMETA(DisplayName = "Light"),
	VFX   UMETA(DisplayName = "VFX (Niagara)"),
	Both  UMETA(DisplayName = "Light + VFX")
};

/** Geometry used to gather affected units. */
UENUM(BlueprintType)
enum class ECrowdControlShape : uint8
{
	Sphere UMETA(DisplayName = "Sphere"),
	Cone   UMETA(DisplayName = "Cone (Multiple Sphere Traces)"),
	Ring   UMETA(DisplayName = "Ring / Annulus")
};

/** Which unit flag(s) the ability suppresses on affected enemies. */
UENUM(BlueprintType)
enum class ECrowdControlEffectType : uint8
{
	DisableMovement UMETA(DisplayName = "Disable Movement (CanMove=false)"),
	DisableAttack   UMETA(DisplayName = "Disable Attack (CanAttack=false)"),
	DisableBoth     UMETA(DisplayName = "Disable Movement + Attack")
};

/**
 * Crowd-control area marker. Two roles:
 *
 *  1) One-shot marker (InitMarker): a transient light/VFX that lives for a fixed lifespan
 *     (used by the old timed AoE path).
 *
 *  2) Flashlight FIELD (InitField): a persistent, caster-attached, ticking field. While on, it
 *     follows the owner, re-detects enemies every UpdateInterval and applies the crowd-control to
 *     units currently inside / releases it the instant they leave. Sphere/Ring use the point light,
 *     Cone uses the spot light (flashlight feel). Turn it off by destroying the actor (EndPlay
 *     releases every still-affected unit).
 *
 * Mode/Shape/Radius/Color etc. are replicated so clients render the light; the field's detection and
 * effect application run on the server only.
 */
UCLASS()
class WEAPONMODULE_API ACrowdControlMarker : public AActor
{
	GENERATED_BODY()

public:
	ACrowdControlMarker();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override; // remote-client per-frame ISM VAT clock hold (freeze fields)

	/** Configure a transient one-shot marker (no field logic). */
	void InitMarker(ECrowdControlMarkerMode InMode, ECrowdControlShape InShape, float InRadius, float InHalfAngleDeg, const FLinearColor& InColor, float InLightIntensity);

	/** Configure and start the continuous flashlight field, attached to and following OwnerUnit.
	 *  Attaches to the owner's skeletal mesh at SocketName (e.g. "rightHandSocket") if present,
	 *  otherwise falls back to the root component with a LightHeightOffset. */
	void InitField(AUnitBase* InOwnerUnit, FName SocketName, ECrowdControlMarkerMode InMode, ECrowdControlShape InShape, ECrowdControlEffectType InEffectType,
		bool bInFreezeUnits, float InRingInnerFraction, int32 InConeTraceSteps, const TArray<TEnumAsByte<EObjectTypeQuery>>& InObjectTypes,
		bool bInIgnoreCaster, bool bInIncludeDeadUnits, const FLinearColor& InColor, float InLightIntensity,
		bool bInDebugDraw, float InDebugDrawDuration, bool bInUseAbilityBaseValues, const FCrowdControlData& InAbilityBaseValues);

	/** Static area query shared by the ability (one-shot) and the field (continuous). */
	static void DetectUnitsInArea(UWorld* World, AUnitBase* Caster, const FVector& Origin, FVector Direction,
		ECrowdControlShape Shape, float Radius, float HalfHeight, float HalfAngleDeg, int32 MaxTargets,
		float RingInnerFraction, int32 ConeTraceSteps, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		bool bIgnoreCaster, bool bIncludeDeadUnits, bool bDebugDraw, float DebugDrawDuration,
		TArray<AUnitBase*>& OutUnits);

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrowdControl")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrowdControl")
	UPointLightComponent* PointLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrowdControl")
	USpotLightComponent* SpotLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrowdControl")
	UNiagaraComponent* NiagaraComp;

	// --- Replicated visual state ---
	UPROPERTY(ReplicatedUsing = OnRep_Visuals, BlueprintReadOnly, Category = "CrowdControl")
	ECrowdControlMarkerMode Mode = ECrowdControlMarkerMode::Light;

	UPROPERTY(ReplicatedUsing = OnRep_Visuals, BlueprintReadOnly, Category = "CrowdControl")
	ECrowdControlShape Shape = ECrowdControlShape::Sphere;

	UPROPERTY(ReplicatedUsing = OnRep_Visuals, BlueprintReadOnly, Category = "CrowdControl")
	float Radius = 600.f;

	UPROPERTY(ReplicatedUsing = OnRep_Visuals, BlueprintReadOnly, Category = "CrowdControl")
	float HalfAngleDeg = 35.f;

	UPROPERTY(ReplicatedUsing = OnRep_Visuals, BlueprintReadOnly, Category = "CrowdControl")
	FLinearColor MarkerColor = FLinearColor(0.2f, 0.4f, 1.0f, 1.0f);

	UPROPERTY(ReplicatedUsing = OnRep_Visuals, BlueprintReadOnly, Category = "CrowdControl")
	float LightIntensity = 5000.f;

	// --- Tunables ---
	/** Maps Radius (cm) to the Niagara user float "Radius" and component scale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float NiagaraRadiusScale = 0.01f;

	/** Point-light falloff exponent (low = nearly even across the radius, good for top-down). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float EvenLightFalloffExponent = 0.4f;

	/** Height above the owner the lights sit at (lets the disc/cone reach the ground in top-down). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float LightHeightOffset = 250.f;

	/** How often (seconds) the field re-detects and re-applies effects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float UpdateInterval = 0.1f;

	/** ISM vertex-animation (VAT) freeze: which per-instance custom-data floats hold the animation StartTime.
	 *  MUST match UUnitAnimationProcessor's StartTimeCustomDataIndex (default 3) / PrevStartTimeCustomDataIndex
	 *  (default 7). Only change if you remapped those indices on the processor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	int32 StartTimeCustomDataIndex = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	int32 PrevStartTimeCustomDataIndex = 7;

	/** Cone only: extra pitch (deg, negative = down) of the spot light relative to the unit's aim.
	 *  0 = points exactly along the unit direction; set negative to tilt the beam down onto the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float SpotLightPitchDeg = 0.f;

	/** Cone only: radial falloff exponent of the spot light (higher = fades out more toward the far end,
	 *  flashlight-like, instead of cutting off abruptly). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float ConeLightFalloffExponent = 2.0f;

	/** Cone only: inner cone as a fraction of the outer cone (lower = softer, wider angular fade at the
	 *  edges). The outer cone always matches the detection half-angle so the light covers the whole trace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ConeInnerAngleFraction = 0.3f;

	/** How often (seconds) units inside the field re-assert Idle. Fixes units that slip back into the
	 *  Attack state/animation while still inside the light. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float ReassertInterval = 1.0f;

	/** Release hysteresis (seconds): a detected unit is only RELEASED once it has been out of the field for
	 *  this long. Stops the flicker-thrash (detection drops a unit for a tick due to the MaxTargets cap or
	 *  the vertical boundary -> Release restores CanMove/CanAttack=true -> re-detect -> Hold again), which
	 *  was making the crowd-control intermittently turn itself off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float ReleaseGraceSeconds = 0.5f;

	/** Rotate-to-mouse trailing grace (seconds): after a commanded move ends we keep rotate-to-mouse
	 *  suppressed for this long, to bridge the velocity ramp-down / one-frame deferred RunTag removal /
	 *  momentary all-signals-false tick at arrival before re-engaging mouse-aim. Comfortably covers a few
	 *  0.1s field ticks + typical RTT. Set 0 to disable. Refreshed ONLY by move-intent, so a genuinely
	 *  idle unit re-engages immediately (no delay to the per-tick tag re-assert). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl")
	float RotateReaddGraceSeconds = 0.4f;

	// --- RotateToMouse (set by the ability before InitField; mirrors the ability's bRotateUnitsToMouse) ---
	/** While the field is active and the caster is NOT moving, rotate it to face the mouse (so the cone aims there). */
	bool bRotateToMouse = false;
	/** Which player's mouse the caster rotates toward (resolved from the ability's owning PlayerController). */
	int32 RotatorPlayerId = -1;

protected:
	UFUNCTION()
	void OnRep_Visuals();

	void ConfigureVisuals();

	/** Server: re-detect + apply/release effects on enter/exit, and refresh talent-tuned values. */
	void ServerUpdateField();

	/** Server: release the crowd-control from every currently affected unit. */
	void ReleaseAllAffected();

	/** Server: re-assert Idle/disable on units still inside the field (anti-stuck). */
	void ReassertAffected();

	/** Timer-driven (independent of actor tick / BP "Can Ever Tick"): owner check + ServerUpdateField. */
	void FieldUpdate();

	/** Timer-driven re-assert. */
	void ReassertTick();

	/** Client-only (remote): pause the SKELETAL animation (GlobalAnimRateScale=0) of units the server froze.
	 *  GlobalAnimRateScale is a non-replicated mesh property, so the server-side freeze never reaches clients;
	 *  this re-detects the field's units locally and mirrors the freeze off the replicated CanMove flag. */
	void ClientVisualFreezeTick();

	/** Client-only: restore GlobalAnimRateScale on every unit this marker froze. */
	void RestoreAllClientFrozen();

	/** Server: add/remove the RotateToMouse tag on the caster based on bRotateToMouse + whether it's moving. */
	void UpdateRotateToMouse(AUnitBase* Unit);

	/** Server: stop RotateToMouse on the caster (remove tag + fragment). */
	void StopRotateToMouse(AUnitBase* Unit);

private:
	// --- Field runtime state (server only) ---
	bool bIsField = false;
	TWeakObjectPtr<AUnitBase> OwnerUnit;
	ECrowdControlEffectType EffectType = ECrowdControlEffectType::DisableBoth;

	// Replicated so remote clients know this is a FREEZE field and run the client-side animation freeze
	// (GlobalAnimRateScale isn't replicated). Set on the server in InitField.
	UPROPERTY(Replicated)
	bool bFreezeUnits = false;

	bool bUseAbilityBaseValues = false;
	FCrowdControlData AbilityBaseValues;
	float RingInnerFraction = 0.5f;
	int32 ConeTraceSteps = 5;
	TArray<TEnumAsByte<EObjectTypeQuery>> DetectionObjectTypes;
	bool bIgnoreCaster = true;
	bool bIncludeDeadUnits = false;
	bool bDebugDraw = false;
	float DebugDrawDuration = 0.2f;
	FTimerHandle UpdateTimer;
	FTimerHandle ReassertTimer;
	bool bRotateToMouseActive = false; // tracks whether the RotateToMouse tag is currently applied
	float LastMoveIntentTime = -1000.f; // world time of last observed move-intent (drives RotateReaddGraceSeconds)

	// Server-only; weak pointers auto-null on destruction so no UPROPERTY/GC keepalive is needed
	// (and TSet<TWeakObjectPtr> is not a valid UPROPERTY container anyway).
	TSet<TWeakObjectPtr<AUnitBase>> AffectedUnits;

	// Last world time each affected unit was actually detected inside the field (release hysteresis).
	TMap<TWeakObjectPtr<AUnitBase>, double> LastSeenTime;

	// --- Client-only visual freeze (remote clients) ---
	FTimerHandle ClientVisualTimer;
	// Units this client froze -> their pre-freeze GlobalAnimRateScale (to restore on exit).
	TMap<TWeakObjectPtr<AUnitBase>, float> ClientFrozenRates;
	// Per-frozen-unit ISM VAT clock-freeze anchors (parallel to ClientFrozenRates), driven from Tick().
	TMap<TWeakObjectPtr<AUnitBase>, FISMFreezeAnchor> ClientISMAnchors;
};
