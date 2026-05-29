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

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeAmmo(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeMaxAmmo(float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeAmountMagazines(float NewValue);
};
