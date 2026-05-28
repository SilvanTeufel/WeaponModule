// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GameplayAbilityBase.h"
#include "WeaponAbilityBase.generated.h"

class UWeaponComponent;
class UWeaponAttributeSet;

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UWeaponAbilityBase : public UGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UWeaponAbilityBase();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UWeaponComponent* GetWeaponComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UWeaponAttributeSet* GetWeaponAttributeSet() const;
};
