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

	UPROPERTY(meta = (BindWidget))
	class UButton* ToggleCrowdControlTalentButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ToggleTalentTreeButton;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UButton* ToggleTalentChooserButton;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|UI")
	class AUnitBase* CurrentUnit;

	UFUNCTION()
	void OnToggleTalentClicked();

	UFUNCTION()
	void OnToggleEffectTalentClicked();

	UFUNCTION()
	void OnToggleEffectAreaTalentClicked();

	UFUNCTION()
	void OnToggleCrowdControlTalentClicked();

	UFUNCTION()
	void OnToggleTalentTreeClicked();

	UFUNCTION()
	void OnToggleTalentChooserClicked();

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

	// Weapon Icons
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage0;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage1;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage2;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage3;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage4;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage5;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage6;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage7;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage8;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage9;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage10;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage11;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage12;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UImage* WeaponIconImage13;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder0;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder1;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder2;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder3;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder4;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder5;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder6;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder7;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder8;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder9;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder10;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder11;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder12;
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UBorder* WeaponIconBorder13;

	UPROPERTY(EditAnywhere, Category = "Weapon|UI")
	FLinearColor HighlightColor = FLinearColor(1.f, 1.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Weapon|UI")
	FLinearColor DefaultColor = FLinearColor(0.f, 0.f, 0.f, 0.f);

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
