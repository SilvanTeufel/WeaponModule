// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAbilityBase.h"
#include "WeaponComponent.h"
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
	bool GetShootInfo(TSubclassOf<class AProjectile>& OutProjectileClass, FWeaponData& OutWeaponData);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyGlobalCooldown();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ModifyAmmo(float Amount);

	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& Duration) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class UGameplayEffect> AmmoCostEffectClass;
};
