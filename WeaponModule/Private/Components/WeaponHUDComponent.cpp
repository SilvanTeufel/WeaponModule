// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Components/WeaponHUDComponent.h"
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

void UWeaponHUDComponent::Server_PurchaseUpgrade_Implementation(UWeaponComponent* WeaponComp, FWeaponUpgrade Upgrade)
{
	if (WeaponComp) WeaponComp->PurchaseUpgrade(Upgrade);
}

void UWeaponHUDComponent::Server_SelectEffectTalent_Implementation(UWeaponComponent* WeaponComp, int32 Index)
{
	if (WeaponComp) WeaponComp->Server_SelectEffectTalent(Index);
}

void UWeaponHUDComponent::Server_ToggleEffectAreaTalent_Implementation(UWeaponComponent* WeaponComp, int32 AreaIndex, int32 TalentIndex)
{
	if (WeaponComp) WeaponComp->Server_ToggleEffectAreaTalent(AreaIndex, TalentIndex);
}

void UWeaponHUDComponent::Server_InvestInEffectAreaRadius_Implementation(UWeaponComponent* WeaponComp, int32 AreaIndex)
{
	if (WeaponComp) WeaponComp->Server_InvestInEffectAreaRadius(AreaIndex);
}

void UWeaponHUDComponent::Server_InvestInEffectAreaDamage_Implementation(UWeaponComponent* WeaponComp, int32 AreaIndex)
{
	if (WeaponComp) WeaponComp->Server_InvestInEffectAreaDamage(AreaIndex);
}

void UWeaponHUDComponent::Server_ResetCurrentWeaponTalents_Implementation(UWeaponComponent* WeaponComp)
{
	if (WeaponComp) WeaponComp->Server_ResetCurrentWeaponTalents();
}

void UWeaponHUDComponent::Server_ResetEffectAreaTalents_Implementation(UWeaponComponent* WeaponComp, int32 AreaIndex)
{
	if (WeaponComp) WeaponComp->Server_ResetEffectAreaTalents(AreaIndex);
}

void UWeaponHUDComponent::Server_SelectEffectAreaIndex_Implementation(UWeaponComponent* WeaponComp, int32 Index)
{
	if (WeaponComp) WeaponComp->Server_SelectEffectAreaIndex(Index);
}

void UWeaponHUDComponent::Server_InvestInCrowdControlTalent_Implementation(UWeaponComponent* WeaponComp, ECrowdControlTalent Talent)
{
	if (WeaponComp) WeaponComp->Server_InvestInCrowdControlTalent(Talent);
}

void UWeaponHUDComponent::Server_ResetCrowdControlTalents_Implementation(UWeaponComponent* WeaponComp)
{
	if (WeaponComp) WeaponComp->Server_ResetCrowdControlTalents();
}
