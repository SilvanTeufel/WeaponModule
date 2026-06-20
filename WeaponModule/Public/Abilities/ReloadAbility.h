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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class UGameplayEffect> ReloadEffectClass;
};
