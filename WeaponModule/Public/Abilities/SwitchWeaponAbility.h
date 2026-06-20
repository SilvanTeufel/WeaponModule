// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/WeaponAbilityBase.h"
#include "SwitchWeaponAbility.generated.h"

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API USwitchWeaponAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	USwitchWeaponAbility();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void PerformSwitchWeapon();
};
