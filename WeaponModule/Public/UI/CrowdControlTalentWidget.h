// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WeaponComponent.h"
#include "CrowdControlTalentWidget.generated.h"

/**
 * Talent widget dedicated to tuning the Crowd-Control ability.
 * Six talents: Range, Duration, Cone Angle, Max Targets, Vertical Reach, Cooldown.
 *
 * All bindings are optional so designers can lay the widget out freely; missing
 * widgets are simply skipped. Expected widget names are the UPROPERTY names below.
 */
UCLASS()
class WEAPONMODULE_API UCrowdControlTalentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	float GetCrowdControlTalentPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void SetTargetUnit(class AUnitBase* InUnit);

	UFUNCTION(BlueprintPure, Category = "Weapon|UI")
	class AUnitBase* GetTargetUnit() const { return TargetUnit; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|UI")
	class AUnitBase* TargetUnit;

	UFUNCTION(BlueprintPure, Category = "Weapon|UI")
	UWeaponComponent* GetWeaponComponent() const;

	void InvestTalent(ECrowdControlTalent Talent);

	// --- Points display ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TalentPointsText;

	// --- Talent buttons ---
	UPROPERTY(meta = (BindWidget))
	class UButton* RangeButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* DurationButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* AngleButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* MaxTargetsButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* HeightButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CooldownButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ResetButton;

	// --- Level display (invested points) ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RangeLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DurationLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AngleLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MaxTargetsLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HeightLevelText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownLevelText;

	// --- Effective value display (current talent-tuned result) ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RangeValueText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DurationValueText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AngleValueText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MaxTargetsValueText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HeightValueText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownValueText;

	// --- Button handlers ---
	UFUNCTION()
	void OnRangeClicked();

	UFUNCTION()
	void OnDurationClicked();

	UFUNCTION()
	void OnAngleClicked();

	UFUNCTION()
	void OnMaxTargetsClicked();

	UFUNCTION()
	void OnHeightClicked();

	UFUNCTION()
	void OnCooldownClicked();

	UFUNCTION()
	void OnResetClicked();
};
