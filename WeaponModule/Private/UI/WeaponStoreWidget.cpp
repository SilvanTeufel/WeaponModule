// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/WeaponStoreWidget.h"
#include "UI/WeaponStoreItemWidget.h"
#include "Actors/WeaponStore.h"
#include "Components/WeaponComponent.h"
#include "Components/WeaponHUDComponent.h"
#include "Characters/Unit/UnitBase.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

void UWeaponStoreWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UWeaponStoreWidget::SetTargetUnit(AUnitBase* Unit)
{
	TargetUnit = Unit;
}

AUnitBase* UWeaponStoreWidget::GetTargetUnit() const
{
	return TargetUnit.Get();
}

void UWeaponStoreWidget::SetStoreActor(AWeaponStore* Store)
{
	StoreActor = Store;
}

AWeaponStore* UWeaponStoreWidget::GetStoreActor() const
{
	return StoreActor.Get();
}

UWeaponComponent* UWeaponStoreWidget::GetTargetWeaponComponent() const
{
	if (AUnitBase* Unit = TargetUnit.Get())
	{
		return Unit->FindComponentByClass<UWeaponComponent>();
	}
	return nullptr;
}

int32 UWeaponStoreWidget::GetCurrentGold() const
{
	if (UWeaponComponent* WC = GetTargetWeaponComponent())
	{
		return WC->GetGold();
	}
	return 0;
}

void UWeaponStoreWidget::RefreshStore(bool bRebuild)
{
	if (bRebuild)
	{
		RebuildItems();
	}

	if (GoldText)
	{
		GoldText->SetText(FText::AsNumber(GetCurrentGold()));
	}

	RefreshAffordability();
}

void UWeaponStoreWidget::RebuildItems()
{
	ItemWidgets.Empty();
	if (ItemsContainer)
	{
		ItemsContainer->ClearChildren();
	}

	AWeaponStore* Store = StoreActor.Get();
	if (!Store || !ItemWidgetClass || !ItemsContainer)
	{
		return;
	}

	for (int32 i = 0; i < Store->StoreItems.Num(); ++i)
	{
		UWeaponStoreItemWidget* Row = CreateWidget<UWeaponStoreItemWidget>(this, ItemWidgetClass);
		if (!Row)
		{
			continue;
		}
		ItemsContainer->AddChild(Row);
		Row->Setup(this, i, Store->StoreItems[i]);
		ItemWidgets.Add(Row);
	}
}

void UWeaponStoreWidget::RefreshAffordability()
{
	AWeaponStore* Store = StoreActor.Get();
	if (!Store)
	{
		return;
	}
	const int32 Gold = GetCurrentGold();
	for (int32 i = 0; i < ItemWidgets.Num(); ++i)
	{
		if (ItemWidgets[i] && Store->StoreItems.IsValidIndex(i))
		{
			ItemWidgets[i]->SetAffordable(Gold >= Store->StoreItems[i].GoldCost);
		}
	}
}

void UWeaponStoreWidget::TryBuyItem(int32 Index)
{
	AWeaponStore* Store = StoreActor.Get();
	if (!Store || !Store->StoreItems.IsValidIndex(Index))
	{
		return;
	}

	UWeaponComponent* WC = GetTargetWeaponComponent();
	if (!WC)
	{
		return;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			// Server validates the store, index and Gold, then applies. Client display refreshes
			// on the next selection-HUD tick once the replicated Gold arrives.
			HUDComp->Server_BuyStoreItem(WC, Index);
		}
	}
}
