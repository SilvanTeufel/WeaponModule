// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/WeaponAbilityBase.h"
#include "ReloadAbility.generated.h"

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UReloadAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	UReloadAbility();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void PerformReload();

	/** Base reload time of the current weapon (FWeaponData.ReloadTime), without the speed multiplier. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetBaseReloadTime() const;

	/** Effective reload time = base ReloadTime * ReloadSpeedMultiplier — matches the duration PerformReload applies. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetReloadTime() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class UGameplayEffect> ReloadEffectClass;
};
