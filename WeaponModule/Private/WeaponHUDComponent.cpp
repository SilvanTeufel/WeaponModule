// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponHUDComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Controller/PlayerController/CustomControllerBase.h"

// Sets default values for this component's properties
UWeaponHUDComponent::UWeaponHUDComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UWeaponHUDComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// We could call InitializeWeaponHUD here automatically if we want
}

void UWeaponHUDComponent::RegisterWeaponHUD(UWeaponSelectionHUDWidget* InWidget, ACustomControllerBase* InController)
{
	if (InWidget)
	{
		WeaponSelectionWidgetInstance = InWidget;
	}

	if (WeaponSelectionWidgetInstance)
	{
		WeaponSelectionWidgetInstance->InitWidget(InController);
	}
}
