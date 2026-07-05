// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Actors/Projectile.h"
#include "Save/RTSSaveGame.h"
#include "WeaponComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UStaticMesh* WeaponMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCastShadow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FVector MuzzleSpawnOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MaxAmmo = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AmountMagazines = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float MaxMagazines = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float CooldownTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ReloadTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FString WeaponName = "Unknown Weapon";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UTexture2D* WeaponIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FTransform Offset = FTransform::Identity;

	// --- Per-Weapon Specialization Data ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float Ammo = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float WeaponTalentPoints = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float CooldownMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float FireRateMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float ReloadSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float PierceExtraCount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float ProjectileExtraCount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float MaxAmmoSpec = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float MaxMagazinesSpec = 0.0f;

	// Talent Levels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float DamageTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float CooldownTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float FireRateTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float ReloadSpeedTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float PierceTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float ProjectileTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float MaxAmmoTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float AmountMagazinesTalentLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float EffectTalentPoints = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	TArray<TSubclassOf<UGameplayEffect>> EffectTalents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	int32 SelectedEffectIndex1 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	int32 SelectedEffectIndex2 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	int32 SelectedEffectIndex3 = -1;
};

USTRUCT(BlueprintType)
struct FEffectAreaData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	FString Name = TEXT("Effect Area");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float BaseRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	TArray<TSubclassOf<class UGameplayEffect>> PossibleEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	TArray<int32> SelectedTalentIndices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	int32 RadiusInvestments = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	int32 DamageInvestments = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float SpentPoints = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Specialization")
	float Amount = 0.0f;
};

/** Identifies which tunable value of the crowd-control ability a talent point is spent on. */
UENUM(BlueprintType)
enum class ECrowdControlTalent : uint8
{
	Range      UMETA(DisplayName = "Range / Radius"),
	Duration   UMETA(DisplayName = "Effect Duration"),
	Angle      UMETA(DisplayName = "Cone Angle"),
	MaxTargets UMETA(DisplayName = "Max Targets"),
	Height     UMETA(DisplayName = "Vertical Reach"),
	Cooldown   UMETA(DisplayName = "Cooldown Reduction")
};

/**
 * Per-unit tuning data for the Crowd-Control ability.
 * Base values + per-level increments are designer-tunable; the *Level fields are the
 * invested talent points (replicated, mutated at runtime via the talent widget).
 * UWeaponComponent exposes effective getters (GetCrowdControlRadius() etc.) that the
 * ability reads, so spent talent points directly drive the ability functions.
 */
USTRUCT(BlueprintType)
struct FCrowdControlData
{
	GENERATED_BODY()

	// --- Base values ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Base")
	float BaseRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Base")
	float BaseDurationSeconds = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Base")
	float BaseHalfAngleDeg = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Base")
	int32 BaseMaxTargets = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Base")
	float BaseHalfHeight = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Base")
	float BaseCooldownSeconds = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Base")
	float MinCooldownSeconds = 1.f;

	// --- Per-level increments ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|PerLevel")
	float RadiusPerLevel = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|PerLevel")
	float DurationPerLevel = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|PerLevel")
	float HalfAnglePerLevel = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|PerLevel")
	int32 MaxTargetsPerLevel = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|PerLevel")
	float HalfHeightPerLevel = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|PerLevel")
	float CooldownReductionPerLevel = 1.f;

	// --- Invested talent levels (runtime, replicated) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Levels")
	int32 RangeLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Levels")
	int32 DurationLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Levels")
	int32 AngleLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Levels")
	int32 MaxTargetsLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Levels")
	int32 HeightLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Levels")
	int32 CooldownLevel = 0;

	/** Talent points spent so far (used for the Reset refund). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdControl|Levels")
	float SpentPoints = 0.f;

	// --- Effective (talent-tuned) values: base + invested-level * per-level ---
	float GetEffectiveRadius() const { return BaseRadius + (float)RangeLevel * RadiusPerLevel; }
	float GetEffectiveDuration() const { return BaseDurationSeconds + (float)DurationLevel * DurationPerLevel; }
	float GetEffectiveHalfAngleDeg() const { return FMath::Clamp(BaseHalfAngleDeg + (float)AngleLevel * HalfAnglePerLevel, 1.f, 179.f); }
	int32 GetEffectiveMaxTargets() const { return FMath::Max(0, BaseMaxTargets + MaxTargetsLevel * MaxTargetsPerLevel); }
	float GetEffectiveHalfHeight() const { return BaseHalfHeight + (float)HeightLevel * HalfHeightPerLevel; }
	float GetEffectiveCooldown() const { return FMath::Max(MinCooldownSeconds, BaseCooldownSeconds - (float)CooldownLevel * CooldownReductionPerLevel); }

	/** Copy the invested talent levels (and spent points) from another data set, keeping THIS one's
	 *  base values + per-level increments. Lets a per-ability base config use the unit's invested points. */
	void CopyLevelsFrom(const FCrowdControlData& Src)
	{
		RangeLevel = Src.RangeLevel;
		DurationLevel = Src.DurationLevel;
		AngleLevel = Src.AngleLevel;
		MaxTargetsLevel = Src.MaxTargetsLevel;
		HeightLevel = Src.HeightLevel;
		CooldownLevel = Src.CooldownLevel;
		SpentPoints = Src.SpentPoints;
	}
};

USTRUCT(BlueprintType)
struct FWeaponUpgrade
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FGameplayAttribute LevelAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	float ModifierValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	TEnumAsByte<EGameplayModOp::Type> ModifierOp = EGameplayModOp::Additive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	int32 Cost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	UTexture2D* Icon = nullptr;
};

// ---------------------------------------------------------------------------
//  Radial Talent Tree
//  A DataTable-driven, ring-shaped talent tree that re-uses the existing
//  weapon / crowd-control / effect-area talents. Each node invests into ONE
//  concrete underlying talent; the tree owns its own point pool (1 per level).
// ---------------------------------------------------------------------------

/** Which of the existing talent systems a talent-tree node drives. */
UENUM(BlueprintType)
enum class ETalentTreeCategory : uint8
{
	WeaponTalent       UMETA(DisplayName = "Weapon Talent"),
	EffectAreaTalent   UMETA(DisplayName = "Effect Area Talent"),
	CrowdControlTalent UMETA(DisplayName = "Crowd Control Talent"),
	ProjectileEffect   UMETA(DisplayName = "Projectile Effect Talent")
};

/** Identifies which weapon talent a WeaponTalent node invests into (mirrors UWeaponTalentWidget's buttons). */
UENUM(BlueprintType)
enum class EWeaponTalentType : uint8
{
	Damage     UMETA(DisplayName = "Damage"),
	Cooldown   UMETA(DisplayName = "Cooldown"),
	FireRate   UMETA(DisplayName = "Fire Rate"),
	Reload     UMETA(DisplayName = "Reload Speed"),
	Pierce     UMETA(DisplayName = "Pierce"),
	Projectile UMETA(DisplayName = "Multi-Shot / Projectiles"),
	MaxAmmo    UMETA(DisplayName = "Max Ammo"),
	Magazines  UMETA(DisplayName = "Magazines")
};

/** Which sub-action an EffectAreaTalent node performs. */
UENUM(BlueprintType)
enum class EEffectAreaTalentAction : uint8
{
	ToggleEffect UMETA(DisplayName = "Toggle Effect"),
	InvestRadius UMETA(DisplayName = "Invest Radius"),
	InvestDamage UMETA(DisplayName = "Invest Damage")
};

/**
 * One node of the radial talent tree. The row NAME is the node's Id.
 * PrevId points at the parent row (a node can have many children -> branches).
 */
USTRUCT(BlueprintType)
struct FTalentTreeNodeRow : public FTableRowBase
{
	GENERATED_BODY()

	// --- Graph ---
	/** Parent node's row name. NAME_None = a root node (usually Ring 0). Several rows may share a PrevId to branch. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Graph")
	FName PrevId = NAME_None;

	/** Ring index: 0 = centre, 1 = first ring, 2 = second ring, ... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Graph")
	int32 Ring = 0;

	/** Angle on the ring in degrees. 0/360 = straight up (12 o'clock), 90 = right, 180 = down, 270 = left. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Graph")
	float Angle = 0.f;

	// --- Talent mapping (TalentType + concrete Id) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Talent")
	ETalentTreeCategory TalentType = ETalentTreeCategory::WeaponTalent;

	/** Used when TalentType == WeaponTalent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Talent", meta = (EditCondition = "TalentType == ETalentTreeCategory::WeaponTalent", EditConditionHides))
	EWeaponTalentType WeaponTalent = EWeaponTalentType::Damage;

	/** Used when TalentType == CrowdControlTalent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Talent", meta = (EditCondition = "TalentType == ETalentTreeCategory::CrowdControlTalent", EditConditionHides))
	ECrowdControlTalent CrowdControlTalent = ECrowdControlTalent::Range;

	/** EffectAreaTalent: index into UWeaponComponent::EffectAreas. ProjectileEffect: index into the weapon's EffectTalents. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Talent", meta = (EditCondition = "TalentType == ETalentTreeCategory::EffectAreaTalent || TalentType == ETalentTreeCategory::ProjectileEffect", EditConditionHides))
	int32 EffectAreaIndex = 0;

	/** Used when TalentType == EffectAreaTalent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Talent", meta = (EditCondition = "TalentType == ETalentTreeCategory::EffectAreaTalent", EditConditionHides))
	EEffectAreaTalentAction EffectAreaAction = EEffectAreaTalentAction::InvestRadius;

	/** EffectAreaTalent + ToggleEffect: which entry of the area's PossibleEffects to toggle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Talent", meta = (EditCondition = "TalentType == ETalentTreeCategory::EffectAreaTalent && EffectAreaAction == EEffectAreaTalentAction::ToggleEffect", EditConditionHides))
	int32 EffectAreaTalentIndex = 0;

	// --- Investment rules ---
	/** How many points can be sunk into this node. Children unlock only once this node is full. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Investment", meta = (ClampMin = "1"))
	int32 MaxPoints = 1;

	/** Tree points spent per invest into this node. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Investment", meta = (ClampMin = "1"))
	int32 PointCost = 1;

	// --- Presentation ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Presentation")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Presentation")
	FText ToolTipTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Presentation", meta = (MultiLine = true))
	FText ToolTipText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Presentation")
	UTexture2D* Icon = nullptr;

	/** Base tint of the node disc (state shading is applied on top). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Talent Tree|Presentation")
	FLinearColor NodeColor = FLinearColor(0.15f, 0.55f, 0.95f, 1.f);
};

/** Replicated per-node investment state for the talent tree (NodeId = the row name). */
USTRUCT(BlueprintType)
struct FTalentTreeNodeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Talent Tree")
	FName NodeId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Talent Tree")
	int32 Points = 0;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEAPONMODULE_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_AvailableWeapons, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FWeaponData> AvailableWeapons;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TArray<FEffectAreaData> EffectAreas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UDataTable* WeaponDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UDataTable* EffectAreaDataTable;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponIndex, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentWeaponIndex = 0;

	UFUNCTION()
	void OnRep_CurrentWeaponIndex();

	UFUNCTION()
	void OnRep_AvailableWeapons();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_SwitchWeapon(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FWeaponData GetCurrentWeaponData() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool PurchaseUpgrade(FWeaponUpgrade Upgrade);

	UFUNCTION(Server, Reliable)
	void Server_PurchaseUpgrade(FWeaponUpgrade Upgrade);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_SelectEffectTalent(int32 Index);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_SelectEffectAreaIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_ResetCurrentWeaponTalents();

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_ResetEffectAreaTalents(int32 AreaIndex);

	void OnUnitSave(AUnitBase* Unit, FUnitSaveData& SaveData);
	void OnUnitLoad(AUnitBase* Unit, FUnitSaveData& SaveData);

	void SyncAttributesFromWeapon(int32 Index);
	void SaveAttributesToWeapon(int32 Index);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_ToggleEffectAreaTalent(int32 AreaIndex, int32 TalentIndex);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_InvestInEffectAreaRadius(int32 AreaIndex);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon")
	void Server_InvestInEffectAreaDamage(int32 AreaIndex);

	void LoadDataFromTables();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	int32 PointsPerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	int32 LevelDivisor = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float StartTalentPoints = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float StartEffectTalentPoints = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float StartEffectAreaTalentPoints = 0.0f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float EffectAreaTalentPoints = 0.0f;

	// Projektil-Effekte
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float ProjectileEffectCostMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	int32 MaxProjectileEffects = 3;

	// Area-Effekte
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float AreaRadiusUpgradeCost = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float AreaDamageUpgradeCost = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	float AreaEffectToggleCost = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Leveling")
	int32 MaxAreaEffects = 3;

	// --- Crowd-Control ability talents ---
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Weapon|CrowdControl")
	FCrowdControlData CrowdControl;

	// Flat bonus points granted on top of the per-level points (designer default, optional).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|CrowdControl")
	float StartCrowdControlTalentPoints = 0.0f;

	// Talent points granted per character level. 1 = one point each level (LevelPoints = CharacterLevel / Divisor).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|CrowdControl")
	int32 CrowdControlLevelDivisor = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|CrowdControl")
	float CrowdControlTalentCost = 1.0f;

	// Currently available (spendable) crowd-control points = Start + (CharacterLevel / Divisor) - Spent.
	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	float GetAvailableCrowdControlPoints() const;

	/** Invest one talent point into the given crowd-control talent (server-authoritative). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon|CrowdControl")
	void Server_InvestInCrowdControlTalent(ECrowdControlTalent Talent);

	/** Refund all crowd-control talent points and reset every level to 0 (server-authoritative). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon|CrowdControl")
	void Server_ResetCrowdControlTalents();

	// Effective (talent-tuned) values consumed by UCrowdControlAbility and the talent widget.
	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	float GetCrowdControlRadius() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	float GetCrowdControlDuration() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	float GetCrowdControlHalfAngleDeg() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	int32 GetCrowdControlMaxTargets() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	float GetCrowdControlHalfHeight() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|CrowdControl")
	float GetCrowdControlCooldown() const;

	// ---------------------------------------------------------------------
	//  Radial Talent Tree
	// ---------------------------------------------------------------------

	/** DataTable of FTalentTreeNodeRow describing the ring-shaped tree. Set in the editor (identical on server & client). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree")
	UDataTable* TalentTreeDataTable = nullptr;

	/** Per-node invested points (replicated). Only nodes with >0 points are stored. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|TalentTree")
	TArray<FTalentTreeNodeState> TalentTreeNodes;

	/** Total tree points spent so far (replicated). Available = Start + Level/Divisor - Spent. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon|TalentTree")
	float TalentTreeSpentPoints = 0.f;

	/** Flat bonus tree points granted on top of the per-level points. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree")
	float StartTalentTreePoints = 0.f;

	/** Tree points granted per character level: LevelPoints = CharacterLevel / Divisor. 1 = one point each level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree", meta = (ClampMin = "1"))
	int32 TalentTreeLevelDivisor = 1;

	/** Default cost of one node invest when the row's PointCost is not set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree")
	float TalentTreePointCost = 1.f;

	// Per-weapon-talent modifier values used when a tree node drives a weapon talent
	// (defaults mirror UWeaponTalentWidget so tree and widget behave identically).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreeDamageModifierValue = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreeCooldownModifierValue = 0.95f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreeFireRateModifierValue = 0.85f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreeReloadModifierValue = 0.95f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreePierceModifierValue = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreeProjectileModifierValue = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreeAmmoModifierValue = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|TalentTree|WeaponModifiers")
	float TreeMagazinesModifierValue = 1.0f;

	/** Available (spendable) tree points = Start + (CharacterLevel / Divisor) - Spent. */
	UFUNCTION(BlueprintPure, Category = "Weapon|TalentTree")
	float GetAvailableTalentTreePoints() const;

	/** Points invested into a specific node (0 if none). */
	UFUNCTION(BlueprintPure, Category = "Weapon|TalentTree")
	int32 GetTalentTreeNodePoints(FName NodeId) const;

	/** A node is unlocked when it is a root, or its parent (PrevId) is fully invested. */
	UFUNCTION(BlueprintPure, Category = "Weapon|TalentTree")
	bool IsTalentTreeNodeUnlocked(FName NodeId) const;

	/** True when the node is unlocked, not yet full, and enough tree points remain. */
	UFUNCTION(BlueprintPure, Category = "Weapon|TalentTree")
	bool CanInvestInTalentTreeNode(FName NodeId) const;

	/** Look up a node row by Id (row name). Returns nullptr if missing. */
	const FTalentTreeNodeRow* FindTalentTreeRow(FName NodeId) const;

	/** Invest one step into a talent-tree node (server-authoritative). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon|TalentTree")
	void Server_InvestInTalentTreeNode(FName NodeId);

	/** Refund all tree points and clear every node + the talent effects it drives (server-authoritative). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Weapon|TalentTree")
	void Server_ResetTalentTree();

protected:
	/** Applies the concrete underlying talent for a node WITHOUT charging any per-system points.
	 *  Returns false when nothing actually changed (e.g. effect already selected / all slots full),
	 *  so the caller can avoid charging a tree point for a no-op. */
	bool ApplyTalentTreeNodeEffect(const FTalentTreeNodeRow& Row);

	/** Builds the FWeaponUpgrade matching a weapon talent enum (mirrors UWeaponTalentWidget). */
	FWeaponUpgrade MakeWeaponUpgrade(EWeaponTalentType Type) const;

	/** Applies a weapon upgrade's level/attribute effect (no point check/deduct). Extracted from PurchaseUpgrade. */
	bool ApplyWeaponUpgradeEffect(const FWeaponUpgrade& Upgrade);

	/** Increments the invested level of one crowd-control talent (no point check). Returns false on invalid talent. */
	bool ApplyCrowdControlTalentLevel(ECrowdControlTalent Talent);

public:

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> WeaponPreview;

	UPROPERTY(EditAnywhere, Category = "Preview")
	int32 PreviewWeaponIndex = 0;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnRegister() override;
	void UpdatePreview();
#endif

	UPROPERTY()
	class UWeaponAttributeSet* WeaponAttributes;
};
