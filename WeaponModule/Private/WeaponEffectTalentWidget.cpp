// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponEffectTalentWidget.h"
#include "WeaponAttributeSet.h"
#include "WeaponHUDComponent.h"
#include "GameFramework/Pawn.h"
#include "Characters/Unit/UnitBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UWeaponEffectTalentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EffectOneButton) EffectOneButton->OnClicked.AddDynamic(this, &UWeaponEffectTalentWidget::OnEffectOneClicked);
	if (EffectTwoButton) EffectTwoButton->OnClicked.AddDynamic(this, &UWeaponEffectTalentWidget::OnEffectTwoClicked);
	if (EffectThreeButton) EffectThreeButton->OnClicked.AddDynamic(this, &UWeaponEffectTalentWidget::OnEffectThreeClicked);
	if (EffectFourButton) EffectFourButton->OnClicked.AddDynamic(this, &UWeaponEffectTalentWidget::OnEffectFourClicked);
	if (EffectFiveButton) EffectFiveButton->OnClicked.AddDynamic(this, &UWeaponEffectTalentWidget::OnEffectFiveClicked);
	if (EffectSixButton) EffectSixButton->OnClicked.AddDynamic(this, &UWeaponEffectTalentWidget::OnEffectSixClicked);
	if (ResetButton) ResetButton->OnClicked.AddDynamic(this, &UWeaponEffectTalentWidget::OnResetClicked);
}

void UWeaponEffectTalentWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	float CurrentPoints = GetEffectTalentPoints();
	if (EffectTalentPointsText)
	{
		EffectTalentPointsText->SetText(FText::AsNumber(FMath::FloorToInt(CurrentPoints)));
	}

	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp || !WeaponComp->WeaponAttributes) return;

	FWeaponData Data = WeaponComp->GetCurrentWeaponData();
	int32 SelectedIndex1 = FMath::FloorToInt(WeaponComp->WeaponAttributes->GetSelectedEffectIndex());
	int32 SelectedIndex2 = FMath::FloorToInt(WeaponComp->WeaponAttributes->GetSelectedEffectIndex2());
	int32 SelectedIndex3 = FMath::FloorToInt(WeaponComp->WeaponAttributes->GetSelectedEffectIndex3());

	int32 SelectionCount = 0;
	if (SelectedIndex1 != -1) SelectionCount++;
	if (SelectedIndex2 != -1) SelectionCount++;
	if (SelectedIndex3 != -1) SelectionCount++;

	auto UpdateEffectUI = [&](int32 Index, FString Label, UButton* Button, UTextBlock* TextBlock)
	{
		if (!Button || !TextBlock) return;

		if (Data.EffectTalents.IsValidIndex(Index))
		{
			Button->SetVisibility(ESlateVisibility::Visible);
			TextBlock->SetVisibility(ESlateVisibility::Visible);

			FString EffectName = Data.EffectTalents[Index] ? Data.EffectTalents[Index]->GetName() : FString("None");
			EffectName.RemoveFromEnd("_C");
			
			float Cost = ((float)Index + 1.0f) * WeaponComp->ProjectileEffectCostMultiplier;
			bool bCanAfford = CurrentPoints >= Cost;
			bool bIsSelected = (SelectedIndex1 == Index || SelectedIndex2 == Index || SelectedIndex3 == Index);
			float InvestedPoints = bIsSelected ? Cost : 0.0f;

			TextBlock->SetText(FText::AsNumber(FMath::FloorToInt(InvestedPoints)));

			if (bIsSelected)
			{
				Button->SetIsEnabled(false);
			}
			else
			{
				Button->SetIsEnabled(bCanAfford && SelectionCount < WeaponComp->MaxProjectileEffects);
			}
		}
		else
		{
			Button->SetVisibility(ESlateVisibility::Collapsed);
			TextBlock->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	UpdateEffectUI(0, "Effect One", EffectOneButton, EffectOneText);
	UpdateEffectUI(1, "Effect Two", EffectTwoButton, EffectTwoText);
	UpdateEffectUI(2, "Effect Three", EffectThreeButton, EffectThreeText);
	UpdateEffectUI(3, "Effect Four", EffectFourButton, EffectFourText);
	UpdateEffectUI(4, "Effect Five", EffectFiveButton, EffectFiveText);
	UpdateEffectUI(5, "Effect Six", EffectSixButton, EffectSixText);
}

void UWeaponEffectTalentWidget::OnEffectOneClicked() 
{ 
	if (UWeaponComponent* Comp = GetWeaponComponent()) 
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectTalent(Comp, 0);
				return;
			}
		}
		Comp->Server_SelectEffectTalent(0); 
	}
}

void UWeaponEffectTalentWidget::OnEffectTwoClicked() 
{ 
	if (UWeaponComponent* Comp = GetWeaponComponent()) 
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectTalent(Comp, 1);
				return;
			}
		}
		Comp->Server_SelectEffectTalent(1); 
	}
}

void UWeaponEffectTalentWidget::OnEffectThreeClicked() 
{ 
	if (UWeaponComponent* Comp = GetWeaponComponent()) 
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectTalent(Comp, 2);
				return;
			}
		}
		Comp->Server_SelectEffectTalent(2); 
	}
}

void UWeaponEffectTalentWidget::OnEffectFourClicked() 
{ 
	if (UWeaponComponent* Comp = GetWeaponComponent()) 
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectTalent(Comp, 3);
				return;
			}
		}
		Comp->Server_SelectEffectTalent(3); 
	}
}

void UWeaponEffectTalentWidget::OnEffectFiveClicked() 
{ 
	if (UWeaponComponent* Comp = GetWeaponComponent()) 
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectTalent(Comp, 4);
				return;
			}
		}
		Comp->Server_SelectEffectTalent(4); 
	}
}

void UWeaponEffectTalentWidget::OnEffectSixClicked() 
{ 
	if (UWeaponComponent* Comp = GetWeaponComponent()) 
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectTalent(Comp, 5);
				return;
			}
		}
		Comp->Server_SelectEffectTalent(5); 
	}
}

void UWeaponEffectTalentWidget::OnResetClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ResetCurrentWeaponTalents(WeaponComp);
				return;
			}
		}
		WeaponComp->Server_ResetCurrentWeaponTalents();
	}
}

float UWeaponEffectTalentWidget::GetEffectTalentPoints() const
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (WeaponComp->WeaponAttributes)
		{
			return WeaponComp->WeaponAttributes->GetEffectTalentPoints();
		}
	}
	return 0.0f;
}

bool UWeaponEffectTalentWidget::BuyUpgrade(FWeaponUpgrade Upgrade)
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp) return false;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			HUDComp->Server_PurchaseUpgrade(WeaponComp, Upgrade);
			return true;
		}
	}
	
	return WeaponComp->PurchaseUpgrade(Upgrade);
}

void UWeaponEffectTalentWidget::SetTargetUnit(AUnitBase* InUnit)
{
	TargetUnit = InUnit;
}

UWeaponComponent* UWeaponEffectTalentWidget::GetWeaponComponent() const
{
	if (TargetUnit)
	{
		return TargetUnit->FindComponentByClass<UWeaponComponent>();
	}
	
	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		return Pawn->FindComponentByClass<UWeaponComponent>();
	}
	return nullptr;
}
