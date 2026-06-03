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
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void UpdateWidget(class AUnitBase* Unit);

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void GetWeaponAttributes(float& Ammo, float& MaxAmmo, float& Magazines, float& MaxMagazines);

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void GetUnitAttributes(float& Health, float& MaxHealth, float& Shield, float& MaxShield, float& Mana, float& MaxMana, float& Experience, float& MaxExperience);

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* ToggleTalentButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ToggleEffectTalentButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ToggleEffectAreaTalentButton;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|UI")
	class AUnitBase* CurrentUnit;

	UFUNCTION()
	void OnToggleTalentClicked();

	UFUNCTION()
	void OnToggleEffectTalentClicked();

	UFUNCTION()
	void OnToggleEffectAreaTalentClicked();

	// Health
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	// Shield
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ShieldBar;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ShieldText;

	// Mana
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ManaBar;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ManaText;

	// Unit Info
	UPROPERTY(meta = (BindWidget))
	class UImage* UnitIconImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* UnitNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterLevelText;

	// Weapon Info
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponNameText;
	UPROPERTY(meta = (BindWidget))
	class UImage* WeaponIconImage;

	// Ammo
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* AmmoBar;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoText;

	// Magazines
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MagazinesText;

	// Effect Areas
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaAmount0;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaAmount1;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaAmount2;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaAmount3;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaAmount4;

	class UAbilitySystemComponent* GetSelectedUnitASC() const;
};
