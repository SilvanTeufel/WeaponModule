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
