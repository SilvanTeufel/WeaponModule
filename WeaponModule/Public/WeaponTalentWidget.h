// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponComponent.h"
#include "WeaponTalentWidget.generated.h"

/**
 * Basis-Klasse für das Talent-Widget, um Waffen-Spezialisierungen zu kaufen.
 */
UCLASS()
class WEAPONMODULE_API UWeaponTalentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	float GetTalentPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	bool BuyUpgrade(FWeaponUpgrade Upgrade);

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void SetTargetUnit(class AUnitBase* InUnit);

	UFUNCTION(BlueprintPure, Category = "Weapon|UI")
	class AUnitBase* GetTargetUnit() const { return TargetUnit; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|UI")
	class AUnitBase* TargetUnit;

	UFUNCTION(BlueprintPure, Category = "Weapon|UI")
	UWeaponComponent* GetWeaponComponent() const;

	// --- BindWidget UI Members ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TalentPointsText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ExperienceProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradeDamageButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradeCooldownButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradeFireRateButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradeReloadButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradePierceButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradeProjectileButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradeAmmoButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* UpgradeMagazinesButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ResetButton;

	// --- Level Display ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DamageLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* FireRateLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReloadLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PierceLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ProjectileLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MagazinesLevelText;

	// --- Button Handlers ---
	UFUNCTION()
	void OnUpgradeDamageClicked();

	UFUNCTION()
	void OnUpgradeCooldownClicked();

	UFUNCTION()
	void OnUpgradeFireRateClicked();

	UFUNCTION()
	void OnUpgradeReloadClicked();

	UFUNCTION()
	void OnUpgradePierceClicked();

	UFUNCTION()
	void OnUpgradeProjectileClicked();

	UFUNCTION()
	void OnUpgradeAmmoClicked();

	UFUNCTION()
	void OnUpgradeMagazinesClicked();

	UFUNCTION()
	void OnResetClicked();
};
