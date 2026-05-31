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
	bool GetShootInfo(TSubclassOf<class AProjectile>& OutProjectileClass, FWeaponData& OutWeaponData, float& OutExtraDamage, int32& OutMaxPiercedTargets, TSubclassOf<class UGameplayEffect>& OutSelectedEffect, TSubclassOf<class UGameplayEffect>& OutSelectedEffect2, TSubclassOf<class UGameplayEffect>& OutSelectedEffect3);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool GetEffectAreaInfo(int32 Index, FEffectAreaInfo& OutAreaInfo);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyGlobalCooldown();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ModifyAmmo(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	int32 GetProjectileCount() const;

	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& Duration) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class UGameplayEffect> AmmoCostEffectClass;
};
