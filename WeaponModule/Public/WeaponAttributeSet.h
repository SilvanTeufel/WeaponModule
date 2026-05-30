// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WeaponAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWeaponAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	// Ammo //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Ammo)
	FGameplayAttributeData Ammo;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, Ammo);

	UFUNCTION()
	virtual void OnRep_Ammo(const FGameplayAttributeData& OldAmmo);
	// Ammo //

	// MaxAmmo //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxAmmo)
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxAmmo);

	UFUNCTION()
	virtual void OnRep_MaxAmmo(const FGameplayAttributeData& OldMaxAmmo);
	// MaxAmmo //

	// AmountMagazines //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AmountMagazines)
	FGameplayAttributeData AmountMagazines;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, AmountMagazines);

	UFUNCTION()
	virtual void OnRep_AmountMagazines(const FGameplayAttributeData& OldAmountMagazines);
	// AmountMagazines //

	// MaxMagazines //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxMagazines)
	FGameplayAttributeData MaxMagazines;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxMagazines);

	UFUNCTION()
	virtual void OnRep_MaxMagazines(const FGameplayAttributeData& OldMaxMagazines);
	// MaxMagazines //

	// WeaponTalentPoints //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_WeaponTalentPoints)
	FGameplayAttributeData WeaponTalentPoints;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, WeaponTalentPoints);

	UFUNCTION()
	virtual void OnRep_WeaponTalentPoints(const FGameplayAttributeData& OldWeaponTalentPoints);
	// WeaponTalentPoints //

	// DamageMultiplier //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_DamageMultiplier)
	FGameplayAttributeData DamageMultiplier;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, DamageMultiplier);

	UFUNCTION()
	virtual void OnRep_DamageMultiplier(const FGameplayAttributeData& OldDamageMultiplier);
	// DamageMultiplier //

	// CooldownMultiplier //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_CooldownMultiplier)
	FGameplayAttributeData CooldownMultiplier;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CooldownMultiplier);

	UFUNCTION()
	virtual void OnRep_CooldownMultiplier(const FGameplayAttributeData& OldCooldownMultiplier);
	// CooldownMultiplier //

	// PierceExtraCount //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_PierceExtraCount)
	FGameplayAttributeData PierceExtraCount;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, PierceExtraCount);

	UFUNCTION()
	virtual void OnRep_PierceExtraCount(const FGameplayAttributeData& OldPierceExtraCount);
	// PierceExtraCount //

	// ProjectileExtraCount //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_ProjectileExtraCount)
	FGameplayAttributeData ProjectileExtraCount;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, ProjectileExtraCount);

	UFUNCTION()
	virtual void OnRep_ProjectileExtraCount(const FGameplayAttributeData& OldProjectileExtraCount);
	// ProjectileExtraCount //

	// ReloadSpeedMultiplier //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_ReloadSpeedMultiplier)
	FGameplayAttributeData ReloadSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, ReloadSpeedMultiplier);

	UFUNCTION()
	virtual void OnRep_ReloadSpeedMultiplier(const FGameplayAttributeData& OldReloadSpeedMultiplier);
	// ReloadSpeedMultiplier //

	// DamageTalentLevel //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_DamageTalentLevel)
	FGameplayAttributeData DamageTalentLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, DamageTalentLevel);

	UFUNCTION()
	virtual void OnRep_DamageTalentLevel(const FGameplayAttributeData& OldDamageTalentLevel);
	// DamageTalentLevel //

	// CooldownTalentLevel //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_CooldownTalentLevel)
	FGameplayAttributeData CooldownTalentLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CooldownTalentLevel);

	UFUNCTION()
	virtual void OnRep_CooldownTalentLevel(const FGameplayAttributeData& OldCooldownTalentLevel);
	// CooldownTalentLevel //

	// ReloadSpeedTalentLevel //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_ReloadSpeedTalentLevel)
	FGameplayAttributeData ReloadSpeedTalentLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, ReloadSpeedTalentLevel);

	UFUNCTION()
	virtual void OnRep_ReloadSpeedTalentLevel(const FGameplayAttributeData& OldReloadSpeedTalentLevel);
	// ReloadSpeedTalentLevel //

	// PierceTalentLevel //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_PierceTalentLevel)
	FGameplayAttributeData PierceTalentLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, PierceTalentLevel);

	UFUNCTION()
	virtual void OnRep_PierceTalentLevel(const FGameplayAttributeData& OldPierceTalentLevel);
	// PierceTalentLevel //

	// ProjectileTalentLevel //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_ProjectileTalentLevel)
	FGameplayAttributeData ProjectileTalentLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, ProjectileTalentLevel);

	UFUNCTION()
	virtual void OnRep_ProjectileTalentLevel(const FGameplayAttributeData& OldProjectileTalentLevel);
	// ProjectileTalentLevel //

	// MaxAmmoTalentLevel //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxAmmoTalentLevel)
	FGameplayAttributeData MaxAmmoTalentLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MaxAmmoTalentLevel);

	UFUNCTION()
	virtual void OnRep_MaxAmmoTalentLevel(const FGameplayAttributeData& OldMaxAmmoTalentLevel);
	// MaxAmmoTalentLevel //

	// AmountMagazinesTalentLevel //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AmountMagazinesTalentLevel)
	FGameplayAttributeData AmountMagazinesTalentLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, AmountMagazinesTalentLevel);

	UFUNCTION()
	virtual void OnRep_AmountMagazinesTalentLevel(const FGameplayAttributeData& OldAmountMagazinesTalentLevel);
	// AmountMagazinesTalentLevel //

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeAmmo(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeMaxAmmo(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeAmountMagazines(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeMaxMagazines(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeWeaponTalentPoints(float NewValue);

	// EffectTalentPoints //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_EffectTalentPoints)
	FGameplayAttributeData EffectTalentPoints;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, EffectTalentPoints);

	UFUNCTION()
	virtual void OnRep_EffectTalentPoints(const FGameplayAttributeData& OldEffectTalentPoints);
	// EffectTalentPoints //

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeEffectTalentPoints(float NewValue);

	// SelectedEffectIndex //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_SelectedEffectIndex)
	FGameplayAttributeData SelectedEffectIndex;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, SelectedEffectIndex);

	UFUNCTION()
	virtual void OnRep_SelectedEffectIndex(const FGameplayAttributeData& OldSelectedEffectIndex);
	// SelectedEffectIndex //

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeSelectedEffectIndex(float NewValue);

	// SelectedEffectIndex2 //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_SelectedEffectIndex2)
	FGameplayAttributeData SelectedEffectIndex2;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, SelectedEffectIndex2);

	UFUNCTION()
	virtual void OnRep_SelectedEffectIndex2(const FGameplayAttributeData& OldSelectedEffectIndex2);
	// SelectedEffectIndex2 //

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeSelectedEffectIndex2(float NewValue);

	// SelectedEffectIndex3 //
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_SelectedEffectIndex3)
	FGameplayAttributeData SelectedEffectIndex3;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, SelectedEffectIndex3);

	UFUNCTION()
	virtual void OnRep_SelectedEffectIndex3(const FGameplayAttributeData& OldSelectedEffectIndex3);
	// SelectedEffectIndex3 //

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeSelectedEffectIndex3(float NewValue);
};
