// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponSelectionHUDWidget.h"
#include "WeaponHUDComponent.generated.h"

class ACustomControllerBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEAPONMODULE_API UWeaponHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWeaponHUDComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Weapon|UI")
	TSubclassOf<UWeaponSelectionHUDWidget> WeaponSelectionWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|UI")
	UWeaponSelectionHUDWidget* WeaponSelectionWidgetInstance;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void RegisterWeaponHUD(UWeaponSelectionHUDWidget* InWidget, ACustomControllerBase* InController);
};
