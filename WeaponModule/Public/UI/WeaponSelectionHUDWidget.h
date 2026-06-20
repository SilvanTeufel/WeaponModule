// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/WeaponHUDWidget.h"
#include "Components/PanelWidget.h"
#include "Layout/Margin.h"
#include "WeaponSelectionHUDWidget.generated.h"

class ACustomControllerBase;

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UWeaponSelectionHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void InitWidget(ACustomControllerBase* InController);

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<class UWeaponHUDWidget> WeaponHUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<class UWeaponTalentWidget> WeaponTalentWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<class UWeaponEffectTalentWidget> WeaponEffectTalentWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<class UEffectAreaTalentWidget> WeaponEffectAreaTalentWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 MaxDisplayedUnits = 3;

	UPROPERTY(EditAnywhere, Category = "Config")
	int32 MaxColumns = 3;

	UPROPERTY(EditAnywhere, Category = "Config")
	FMargin WidgetPadding = FMargin(5.f);

	UPROPERTY(EditAnywhere, Category = "Config")
	float UpdateInterval = 0.1f;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|UI")
	ACustomControllerBase* ControllerBase;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* WeaponHUDContainer;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* WeaponTalentContainer;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* WeaponEffectTalentContainer;

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* EffectAreaTalentContainer;

	UPROPERTY()
	TArray<UWeaponHUDWidget*> WeaponHUDWidgets;

	UPROPERTY()
	class UWeaponTalentWidget* TalentWidgetInstance;

	UPROPERTY()
	class UWeaponEffectTalentWidget* EffectTalentWidgetInstance;

	UPROPERTY()
	class UEffectAreaTalentWidget* EffectAreaTalentWidgetInstance;

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void ToggleTalentWidget(class AUnitBase* Unit);

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void ToggleEffectTalentWidget(class AUnitBase* Unit);

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void ToggleEffectAreaTalentWidget(class AUnitBase* Unit);

protected:

	void CreateHUDWidgets();
	void UpdateSelection();

	FTimerHandle UpdateTimerHandle;
};
