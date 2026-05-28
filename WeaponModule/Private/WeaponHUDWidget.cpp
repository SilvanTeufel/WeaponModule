// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponHUDWidget.h"
#include "WeaponAttributeSet.h"
#include "GAS/AttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Hud/HUDBase.h"
#include "Characters/Unit/UnitBase.h"
#include "Kismet/GameplayStatics.h"

void UWeaponHUDWidget::GetWeaponAttributes(float& Ammo, float& MaxAmmo, float& Magazines)
{
	Ammo = 0.f;
	MaxAmmo = 0.f;
	Magazines = 0.f;

	UAbilitySystemComponent* ASC = GetSelectedUnitASC();
	if (ASC)
	{
		const UWeaponAttributeSet* WeaponAttributes = ASC->GetSet<UWeaponAttributeSet>();
		if (WeaponAttributes)
		{
			Ammo = WeaponAttributes->GetAmmo();
			MaxAmmo = WeaponAttributes->GetMaxAmmo();
			Magazines = WeaponAttributes->GetAmountMagazines();
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
