// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/EffectAreaTalentWidget.h"
#include "Abilities/WeaponAttributeSet.h"
#include "Components/WeaponHUDComponent.h"
#include "Characters/Unit/UnitBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "AbilitySystemComponent.h"

void UEffectAreaTalentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EffectAreaOneButton) EffectAreaOneButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnEffectAreaOneClicked);
	if (EffectAreaTwoButton) EffectAreaTwoButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnEffectAreaTwoClicked);
	if (EffectAreaThreeButton) EffectAreaThreeButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnEffectAreaThreeClicked);
	if (EffectAreaFourButton) EffectAreaFourButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnEffectAreaFourClicked);
	if (EffectAreaFiveButton) EffectAreaFiveButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnEffectAreaFiveClicked);
	if (EffectAreaSixButton) EffectAreaSixButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnEffectAreaSixClicked);
	if (IncreaseRadiusButton) IncreaseRadiusButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnIncreaseRadiusClicked);
	if (IncreaseBaseDamageButton) IncreaseBaseDamageButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnIncreaseBaseDamageClicked);
	if (IncreaseIndexButton) IncreaseIndexButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnIncreaseIndexClicked);
	if (DecreaseIndexButton) DecreaseIndexButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnDecreaseIndexClicked);
	if (ResetButton) ResetButton->OnClicked.AddDynamic(this, &UEffectAreaTalentWidget::OnResetClicked);
}

void UEffectAreaTalentWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (EffectAreaTalentPointsText)
	{
		EffectAreaTalentPointsText->SetText(FText::AsNumber(GetEffectAreaTalentPoints()));
	}

	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (WeaponComp)
	{
		// 1. Update Index Selection UI
		if (IndexValueText) IndexValueText->SetText(FText::AsNumber(CurrentlyEditingIndex));
		if (IndexDisplayNameText)
		{
			if (IndexDisplayNames.IsValidIndex(CurrentlyEditingIndex))
			{
				IndexDisplayNameText->SetText(FText::FromString(IndexDisplayNames[CurrentlyEditingIndex]));
			}
			else
			{
				// Fallback to EffectAreas data name or default
				if (WeaponComp->EffectAreas.IsValidIndex(CurrentlyEditingIndex))
					IndexDisplayNameText->SetText(FText::FromString(WeaponComp->EffectAreas[CurrentlyEditingIndex].Name));
				else
					IndexDisplayNameText->SetText(FText::FromString(TEXT("Effect Area")));
			}
		}

		// 2. Update Talent Buttons for CurrentlyEditingIndex
		if (WeaponComp->EffectAreas.IsValidIndex(CurrentlyEditingIndex))
		{
			FEffectAreaData& AreaData = WeaponComp->EffectAreas[CurrentlyEditingIndex];
			float CurrentPoints = GetEffectAreaTalentPoints();
			
			auto SetTalentUI = [&](UButton* Button, UTextBlock* TextBlock, int32 Index) {
				if (!Button) return;
				if (AreaData.PossibleEffects.IsValidIndex(Index)) {
					UClass* GEClass = AreaData.PossibleEffects[Index];
					if (TextBlock) TextBlock->SetText(FText::FromString(GEClass ? GEClass->GetName() : TEXT("Empty")));
					
					bool bSelected = AreaData.SelectedTalentIndices.Contains(Index);
					Button->SetBackgroundColor(bSelected ? FLinearColor::Green : FLinearColor::Gray);
					
					bool bCanToggle = bSelected || (CurrentPoints >= WeaponComp->AreaEffectToggleCost && AreaData.SelectedTalentIndices.Num() < WeaponComp->MaxAreaEffects);
					Button->SetIsEnabled(bCanToggle);
				} else {
					if (TextBlock) TextBlock->SetText(FText::FromString(TEXT("Locked")));
					Button->SetBackgroundColor(FLinearColor::Black);
					Button->SetIsEnabled(false);
				}
			};

			SetTalentUI(EffectAreaOneButton, EffectAreaOneText, 0);
			SetTalentUI(EffectAreaTwoButton, EffectAreaTwoText, 1);
			SetTalentUI(EffectAreaThreeButton, EffectAreaThreeText, 2);
			SetTalentUI(EffectAreaFourButton, EffectAreaFourText, 3);
			SetTalentUI(EffectAreaFiveButton, EffectAreaFiveText, 4);
			SetTalentUI(EffectAreaSixButton, EffectAreaSixText, 5);

			if (RadiusLevelText) RadiusLevelText->SetText(FText::AsNumber(AreaData.RadiusInvestments));
			if (DamageLevelText) DamageLevelText->SetText(FText::AsNumber(AreaData.DamageInvestments));

			if (IncreaseRadiusButton) IncreaseRadiusButton->SetIsEnabled(CurrentPoints >= WeaponComp->AreaRadiusUpgradeCost);
			if (IncreaseBaseDamageButton) IncreaseBaseDamageButton->SetIsEnabled(CurrentPoints >= WeaponComp->AreaDamageUpgradeCost);
		}
	}
}

float UEffectAreaTalentWidget::GetEffectAreaTalentPoints() const
{
	if (!TargetUnit) return 0.f;
	UAbilitySystemComponent* ASC = TargetUnit->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return 0.f;

	const UWeaponAttributeSet* WeaponAttributes = ASC->GetSet<UWeaponAttributeSet>();
	return WeaponAttributes ? WeaponAttributes->GetEffectAreaTalentPoints() : 0.f;
}

void UEffectAreaTalentWidget::SetTargetUnit(AUnitBase* InUnit)
{
	TargetUnit = InUnit;
}

UWeaponComponent* UEffectAreaTalentWidget::GetWeaponComponent() const
{
	return TargetUnit ? TargetUnit->FindComponentByClass<UWeaponComponent>() : nullptr;
}

void UEffectAreaTalentWidget::OnEffectAreaOneClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ToggleEffectAreaTalent(WeaponComp, CurrentlyEditingIndex, 0);
				return;
			}
		}
		WeaponComp->Server_ToggleEffectAreaTalent(CurrentlyEditingIndex, 0);
	}
}

void UEffectAreaTalentWidget::OnEffectAreaTwoClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ToggleEffectAreaTalent(WeaponComp, CurrentlyEditingIndex, 1);
				return;
			}
		}
		WeaponComp->Server_ToggleEffectAreaTalent(CurrentlyEditingIndex, 1);
	}
}

void UEffectAreaTalentWidget::OnEffectAreaThreeClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ToggleEffectAreaTalent(WeaponComp, CurrentlyEditingIndex, 2);
				return;
			}
		}
		WeaponComp->Server_ToggleEffectAreaTalent(CurrentlyEditingIndex, 2);
	}
}

void UEffectAreaTalentWidget::OnEffectAreaFourClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ToggleEffectAreaTalent(WeaponComp, CurrentlyEditingIndex, 3);
				return;
			}
		}
		WeaponComp->Server_ToggleEffectAreaTalent(CurrentlyEditingIndex, 3);
	}
}

void UEffectAreaTalentWidget::OnEffectAreaFiveClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ToggleEffectAreaTalent(WeaponComp, CurrentlyEditingIndex, 4);
				return;
			}
		}
		WeaponComp->Server_ToggleEffectAreaTalent(CurrentlyEditingIndex, 4);
	}
}

void UEffectAreaTalentWidget::OnEffectAreaSixClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ToggleEffectAreaTalent(WeaponComp, CurrentlyEditingIndex, 5);
				return;
			}
		}
		WeaponComp->Server_ToggleEffectAreaTalent(CurrentlyEditingIndex, 5);
	}
}

void UEffectAreaTalentWidget::OnIncreaseRadiusClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_InvestInEffectAreaRadius(WeaponComp, CurrentlyEditingIndex);
				return;
			}
		}
		WeaponComp->Server_InvestInEffectAreaRadius(CurrentlyEditingIndex);
	}
}

void UEffectAreaTalentWidget::OnIncreaseBaseDamageClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_InvestInEffectAreaDamage(WeaponComp, CurrentlyEditingIndex);
				return;
			}
		}
		WeaponComp->Server_InvestInEffectAreaDamage(CurrentlyEditingIndex);
	}
}

void UEffectAreaTalentWidget::OnIncreaseIndexClicked()
{
	CurrentlyEditingIndex++;
	if (CurrentlyEditingIndex > MaxEffectAreaIndex) CurrentlyEditingIndex = 0;

	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectAreaIndex(WeaponComp, CurrentlyEditingIndex);
				return;
			}
		}
		WeaponComp->Server_SelectEffectAreaIndex(CurrentlyEditingIndex);
	}
}

void UEffectAreaTalentWidget::OnDecreaseIndexClicked()
{
	CurrentlyEditingIndex--;
	if (CurrentlyEditingIndex < 0) CurrentlyEditingIndex = MaxEffectAreaIndex;

	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_SelectEffectAreaIndex(WeaponComp, CurrentlyEditingIndex);
				return;
			}
		}
		WeaponComp->Server_SelectEffectAreaIndex(CurrentlyEditingIndex);
	}
}

void UEffectAreaTalentWidget::OnResetClicked()
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
			{
				HUDComp->Server_ResetEffectAreaTalents(WeaponComp, CurrentlyEditingIndex);
				return;
			}
		}
		WeaponComp->Server_ResetEffectAreaTalents(CurrentlyEditingIndex);
	}
}
