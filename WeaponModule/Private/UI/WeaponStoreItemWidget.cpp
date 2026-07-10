// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/WeaponStoreItemWidget.h"
#include "UI/WeaponStoreWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UWeaponStoreItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (BuyButton)
	{
		BuyButton->OnClicked.AddDynamic(this, &UWeaponStoreItemWidget::OnBuyClicked);
	}
}

void UWeaponStoreItemWidget::Setup(UWeaponStoreWidget* InOwner, int32 InIndex, const FStoreItemEntry& InEntry)
{
	OwnerStoreWidget = InOwner;
	ItemIndex = InIndex;

	if (NameText)
	{
		FText Label = InEntry.DisplayName;
		if (Label.IsEmpty())
		{
			// Fall back to the enum's display name so an unnamed row is still readable.
			if (const UEnum* EnumPtr = StaticEnum<EStoreItemType>())
			{
				Label = EnumPtr->GetDisplayNameTextByValue((int64)InEntry.ItemType);
			}
		}
		NameText->SetText(Label);
	}

	if (CostText)
	{
		CostText->SetText(FText::AsNumber(InEntry.GoldCost));
	}

	if (IconImage)
	{
		if (InEntry.Icon)
		{
			IconImage->SetBrushFromTexture(InEntry.Icon);
			IconImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UWeaponStoreItemWidget::SetAffordable(bool bAffordable)
{
	if (BuyButton)
	{
		BuyButton->SetIsEnabled(bAffordable);
	}
	const FLinearColor Tint = bAffordable ? AffordableColor : UnaffordableColor;
	if (NameText) NameText->SetColorAndOpacity(FSlateColor(Tint));
	if (CostText) CostText->SetColorAndOpacity(FSlateColor(Tint));
}

void UWeaponStoreItemWidget::OnBuyClicked()
{
	if (OwnerStoreWidget.IsValid())
	{
		OwnerStoreWidget->TryBuyItem(ItemIndex);
	}
}
