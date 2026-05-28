// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UWeaponHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void GetWeaponAttributes(float& Ammo, float& MaxAmmo, float& Magazines);

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void GetUnitAttributes(float& Health, float& MaxHealth, float& Shield, float& MaxShield, float& Mana, float& MaxMana, float& Experience, float& MaxExperience);

protected:
	class UAbilitySystemComponent* GetSelectedUnitASC() const;
};
