// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponHUDWidget.h"
#include "WeaponAttributeSet.h"
#include "WeaponComponent.h"
#include "GAS/AttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Hud/HUDBase.h"
#include "Characters/Unit/UnitBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "WeaponSelectionHUDWidget.h"
#include "WeaponHUDComponent.h"

void UWeaponHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ToggleTalentButton)
	{
		ToggleTalentButton->OnClicked.AddDynamic(this, &UWeaponHUDWidget::OnToggleTalentClicked);
	}

	if (ToggleEffectTalentButton)
	{
		ToggleEffectTalentButton->OnClicked.AddDynamic(this, &UWeaponHUDWidget::OnToggleEffectTalentClicked);
	}

	if (ToggleEffectAreaTalentButton)
	{
		ToggleEffectAreaTalentButton->OnClicked.AddDynamic(this, &UWeaponHUDWidget::OnToggleEffectAreaTalentClicked);
	}
}

void UWeaponHUDWidget::UpdateWidget(AUnitBase* Unit)
{
	CurrentUnit = Unit;
	if (!Unit)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	UAbilitySystemComponent* ASC = Unit->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC)
	{
		// Update Unit Attributes
		const UAttributeSetBase* Attributes = ASC->GetSet<UAttributeSetBase>();
		if (Attributes)
		{
			float Health = Attributes->GetHealth();
			float MaxHealth = Attributes->GetMaxHealth();
			float Shield = Attributes->GetShield();
			float MaxShield = Attributes->GetMaxShield();
			float Mana = Attributes->GetMana();
			float MaxMana = Attributes->GetMaxMana();

			if (HealthBar) HealthBar->SetPercent(MaxHealth > 0 ? Health / MaxHealth : 0);
			if (HealthText) HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));

			if (ShieldBar)
			{
				ShieldBar->SetPercent(MaxShield > 0 ? Shield / MaxShield : 0);
				ShieldBar->SetVisibility(MaxShield > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			}
			if (ShieldText)
			{
				ShieldText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Shield, MaxShield)));
				ShieldText->SetVisibility(MaxShield > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			}

			if (ManaBar)
			{
				ManaBar->SetPercent(MaxMana > 0 ? Mana / MaxMana : 0);
				ManaBar->SetVisibility(MaxMana > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			}
			if (ManaText)
			{
				ManaText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Mana, MaxMana)));
				ManaText->SetVisibility(MaxMana > 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			}
		}

		// Update Weapon Attributes
		const UWeaponAttributeSet* WeaponAttributes = ASC->GetSet<UWeaponAttributeSet>();
		if (WeaponAttributes)
		{
			float Ammo = WeaponAttributes->GetAmmo();
			float MaxAmmo = WeaponAttributes->GetMaxAmmo();
			float Magazines = WeaponAttributes->GetAmountMagazines();
			float MaxMagazines = WeaponAttributes->GetMaxMagazines();

			if (AmmoBar) AmmoBar->SetPercent(MaxAmmo > 0 ? Ammo / MaxAmmo : 0);
			if (AmmoText) AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Ammo, MaxAmmo)));
			if (MagazinesText) MagazinesText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Magazines, MaxMagazines)));
		}
	}

	// Update Unit Icon and Name
	if (UnitIconImage && Unit->UnitIcon)
	{
		UnitIconImage->SetBrushFromTexture(Unit->UnitIcon);
	}

	if (UnitNameText)
	{
		UnitNameText->SetText(FText::FromString(Unit->Name));
	}

	if (CharacterLevelText)
	{
		CharacterLevelText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), Unit->LevelData.CharacterLevel)));
	}

	// Update Current Weapon Info
	UWeaponComponent* WeaponComp = Unit->FindComponentByClass<UWeaponComponent>();
	if (WeaponComp)
	{
		FWeaponData CurrentWeapon = WeaponComp->GetCurrentWeaponData();
		if (WeaponNameText) WeaponNameText->SetText(FText::FromString(CurrentWeapon.WeaponName));
		if (WeaponIconImage && CurrentWeapon.WeaponIcon)
		{
			WeaponIconImage->SetBrushFromTexture(CurrentWeapon.WeaponIcon);
		}
	}
}

void UWeaponHUDWidget::GetWeaponAttributes(float& Ammo, float& MaxAmmo, float& Magazines, float& MaxMagazines)
{
	Ammo = 0.f;
	MaxAmmo = 0.f;
	Magazines = 0.f;
	MaxMagazines = 0.f;

	UAbilitySystemComponent* ASC = GetSelectedUnitASC();
	if (ASC)
	{
		const UWeaponAttributeSet* WeaponAttributes = ASC->GetSet<UWeaponAttributeSet>();
		if (WeaponAttributes)
		{
			Ammo = WeaponAttributes->GetAmmo();
			MaxAmmo = WeaponAttributes->GetMaxAmmo();
			Magazines = WeaponAttributes->GetAmountMagazines();
			MaxMagazines = WeaponAttributes->GetMaxMagazines();
		}
	}
}

void UWeaponHUDWidget::GetUnitAttributes(float& Health, float& MaxHealth, float& Shield, float& MaxShield, float& Mana, float& MaxMana, float& Experience, float& MaxExperience)
{
	Health = 0.f;
	MaxHealth = 0.f;
	Shield = 0.f;
	MaxShield = 0.f;
	Mana = 0.f;
	MaxMana = 0.f;
	Experience = 0.f;
	MaxExperience = 0.f;

	UAbilitySystemComponent* ASC = GetSelectedUnitASC();
	if (ASC)
	{
		const UAttributeSetBase* Attributes = ASC->GetSet<UAttributeSetBase>();
		if (Attributes)
		{
			Health = Attributes->GetHealth();
			MaxHealth = Attributes->GetMaxHealth();
			Shield = Attributes->GetShield();
			MaxShield = Attributes->GetMaxShield();
			Mana = Attributes->GetMana();
			MaxMana = Attributes->GetMaxMana();
		}

		AUnitBase* Unit = Cast<AUnitBase>(ASC->GetAvatarActor());
		if (Unit)
		{
			Experience = (float)Unit->LevelData.Experience;
			// Using the logic from UnitBaseHealthBar.cpp
			MaxExperience = (float)(Unit->LevelUpData.ExperiencePerLevel * Unit->LevelData.CharacterLevel);
			if (MaxExperience <= 0) MaxExperience = 1.f; // Avoid division by zero in UI
		}
	}
}

void UWeaponHUDWidget::OnToggleTalentClicked()
{
	if (!CurrentUnit) return;

	// Strategy 1: Via registered instance in PlayerController (Safest)
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			if (HUDComp->WeaponSelectionWidgetInstance)
			{
				HUDComp->WeaponSelectionWidgetInstance->ToggleTalentWidget(CurrentUnit);
				return;
			}
		}
	}

	// Strategy 2: Fallback - Search in the Outer hierarchy
	UObject* CurrentOuter = GetOuter();
	while (CurrentOuter)
	{
		if (UWeaponSelectionHUDWidget* SelectionWidget = Cast<UWeaponSelectionHUDWidget>(CurrentOuter))
		{
			SelectionWidget->ToggleTalentWidget(CurrentUnit);
			return;
		}
		CurrentOuter = CurrentOuter->GetOuter();
	}
}

void UWeaponHUDWidget::OnToggleEffectTalentClicked()
{
	if (!CurrentUnit) return;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			if (HUDComp->WeaponSelectionWidgetInstance)
			{
				HUDComp->WeaponSelectionWidgetInstance->ToggleEffectTalentWidget(CurrentUnit);
				return;
			}
		}
	}

	// Fallback - Search in the Outer hierarchy
	UObject* CurrentOuter = GetOuter();
	while (CurrentOuter)
	{
		if (UWeaponSelectionHUDWidget* SelectionWidget = Cast<UWeaponSelectionHUDWidget>(CurrentOuter))
		{
			SelectionWidget->ToggleEffectTalentWidget(CurrentUnit);
			return;
		}
		CurrentOuter = CurrentOuter->GetOuter();
	}
}

void UWeaponHUDWidget::OnToggleEffectAreaTalentClicked()
{
	if (!CurrentUnit) return;

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			if (HUDComp->WeaponSelectionWidgetInstance)
			{
				HUDComp->WeaponSelectionWidgetInstance->ToggleEffectAreaTalentWidget(CurrentUnit);
				return;
			}
		}
	}

	// Fallback - Search in the Outer hierarchy
	UObject* CurrentOuter = GetOuter();
	while (CurrentOuter)
	{
		if (UWeaponSelectionHUDWidget* SelectionWidget = Cast<UWeaponSelectionHUDWidget>(CurrentOuter))
		{
			SelectionWidget->ToggleEffectAreaTalentWidget(CurrentUnit);
			return;
		}
		CurrentOuter = CurrentOuter->GetOuter();
	}
}

UAbilitySystemComponent* UWeaponHUDWidget::GetSelectedUnitASC() const
{
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetHUD())
	{
		AHUDBase* HUD = Cast<AHUDBase>(PC->GetHUD());
		if (HUD && HUD->SelectedUnits.Num() > 0 && HUD->SelectedUnits[0])
		{
			AUnitBase* Unit = HUD->SelectedUnits[0];
			return Unit->FindComponentByClass<UAbilitySystemComponent>();
		}
	}
	return nullptr;
}
