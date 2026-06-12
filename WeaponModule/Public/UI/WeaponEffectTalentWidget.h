// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WeaponComponent.h"
#include "WeaponEffectTalentWidget.generated.h"

/**
 * Basis-Klasse für das Effekt-Talent-Widget, um Projektil-Effekte zu kaufen.
 */
UCLASS()
class WEAPONMODULE_API UWeaponEffectTalentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	float GetEffectTalentPoints() const;

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
	class UTextBlock* EffectTalentPointsText;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectOneButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectTwoButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectThreeButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectFourButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectFiveButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectSixButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ResetButton;

	// --- Text Display ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectOneText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectTwoText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectThreeText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectFourText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectFiveText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectSixText;

	// --- Button Handlers ---
	UFUNCTION()
	void OnEffectOneClicked();

	UFUNCTION()
	void OnEffectTwoClicked();

	UFUNCTION()
	void OnEffectThreeClicked();

	UFUNCTION()
	void OnEffectFourClicked();

	UFUNCTION()
	void OnEffectFiveClicked();

	UFUNCTION()
	void OnEffectSixClicked();

	UFUNCTION()
	void OnResetClicked();
};
