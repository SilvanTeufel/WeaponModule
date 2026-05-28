// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAbilityBase.h"
#include "ShootAbility.generated.h"

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UShootAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	UShootAbility();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void PerformShoot();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class UGameplayEffect> AmmoCostEffectClass;
};
