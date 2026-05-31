// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponComponent.h"
#include "EffectAreaTalentWidget.generated.h"

/**
 * Class for the Effect Area Talent Widget to select effect area types.
 */
UCLASS()
class WEAPONMODULE_API UEffectAreaTalentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	float GetEffectAreaTalentPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void SetTargetUnit(class AUnitBase* InUnit);

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|UI")
	int32 CurrentlyEditingIndex = 0;

	UFUNCTION(BlueprintPure, Category = "Weapon|UI")
	class AUnitBase* GetTargetUnit() const { return TargetUnit; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|UI")
	class AUnitBase* TargetUnit;

	UFUNCTION(BlueprintPure, Category = "Weapon|UI")
	UWeaponComponent* GetWeaponComponent() const;

	// --- BindWidget UI Members ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaTalentPointsText;

	// --- Index Selection Controls ---
	UPROPERTY(meta = (BindWidget))
	class UButton* IncreaseIndexButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* DecreaseIndexButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* IndexDisplayNameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* IndexValueText;

	UPROPERTY(meta = (BindWidget))
	class UButton* ResetButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
	TArray<FString> IndexDisplayNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
	int32 MaxEffectAreaIndex = 5;

	// --- Talent Toggle Buttons ---
	UPROPERTY(meta = (BindWidget))
	class UButton* EffectAreaOneButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectAreaTwoButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectAreaThreeButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectAreaFourButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectAreaFiveButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* EffectAreaSixButton;

	// --- Text Display ---
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaOneText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaTwoText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaThreeText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaFourText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaFiveText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EffectAreaSixText;

	// --- Button Handlers ---
	UFUNCTION()
	void OnEffectAreaOneClicked();

	UFUNCTION()
	void OnEffectAreaTwoClicked();

	UFUNCTION()
	void OnEffectAreaThreeClicked();

	UFUNCTION()
	void OnEffectAreaFourClicked();

	UFUNCTION()
	void OnEffectAreaFiveClicked();

	UFUNCTION()
	void OnEffectAreaSixClicked();

	// --- Index Handlers ---
	UFUNCTION()
	void OnIncreaseIndexClicked();

	UFUNCTION()
	void OnDecreaseIndexClicked();

	UFUNCTION()
	void OnResetClicked();
};
