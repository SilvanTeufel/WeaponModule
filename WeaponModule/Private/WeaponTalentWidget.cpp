// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponTalentWidget.h"
#include "WeaponAttributeSet.h"
#include "WeaponHUDComponent.h"
#include "GameFramework/Pawn.h"
#include "Characters/Unit/UnitBase.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"

void UWeaponTalentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UpgradeDamageButton) UpgradeDamageButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnUpgradeDamageClicked);
	if (UpgradeCooldownButton) UpgradeCooldownButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnUpgradeCooldownClicked);
	if (UpgradeReloadButton) UpgradeReloadButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnUpgradeReloadClicked);
	if (UpgradePierceButton) UpgradePierceButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnUpgradePierceClicked);
	if (UpgradeProjectileButton) UpgradeProjectileButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnUpgradeProjectileClicked);
	if (UpgradeAmmoButton) UpgradeAmmoButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnUpgradeAmmoClicked);
	if (UpgradeMagazinesButton) UpgradeMagazinesButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnUpgradeMagazinesClicked);
	if (ResetButton) ResetButton->OnClicked.AddDynamic(this, &UWeaponTalentWidget::OnResetClicked);
}

void UWeaponTalentWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TalentPointsText)
	{
		TalentPointsText->SetText(FText::AsNumber(FMath::FloorToInt(GetTalentPoints())));
	}

	if (ExperienceProgressBar && TargetUnit)
	{
		float CurrentExp = (float)TargetUnit->LevelData.Experience;
		float MaxExp = (float)(TargetUnit->LevelUpData.ExperiencePerLevel * TargetUnit->LevelData.CharacterLevel);
		ExperienceProgressBar->SetPercent(MaxExp > 0.0f ? CurrentExp / MaxExp : 0.0f);
	}

	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (const UWeaponAttributeSet* Attrs = WeaponComp->WeaponAttributes)
		{
			if (DamageLevelText) DamageLevelText->SetText(FText::AsNumber(FMath::FloorToInt(Attrs->GetDamageTalentLevel())));
			if (CooldownLevelText) CooldownLevelText->SetText(FText::AsNumber(FMath::FloorToInt(Attrs->GetCooldownTalentLevel())));
			if (ReloadLevelText) ReloadLevelText->SetText(FText::AsNumber(FMath::FloorToInt(Attrs->GetReloadSpeedTalentLevel())));
			if (PierceLevelText) PierceLevelText->SetText(FText::AsNumber(FMath::FloorToInt(Attrs->GetPierceTalentLevel())));
			if (ProjectileLevelText) ProjectileLevelText->SetText(FText::AsNumber(FMath::FloorToInt(Attrs->GetProjectileTalentLevel())));
			if (AmmoLevelText) AmmoLevelText->SetText(FText::AsNumber(FMath::FloorToInt(Attrs->GetMaxAmmoTalentLevel())));
			if (MagazinesLevelText) MagazinesLevelText->SetText(FText::AsNumber(FMath::FloorToInt(Attrs->GetAmountMagazinesTalentLevel())));
		}
	}
}

void UWeaponTalentWidget::OnUpgradeDamageClicked()
{
	FWeaponUpgrade Upgrade;
	Upgrade.Name = FText::FromString("Damage");
	Upgrade.Attribute = UWeaponAttributeSet::GetDamageMultiplierAttribute();
	Upgrade.LevelAttribute = UWeaponAttributeSet::GetDamageTalentLevelAttribute();
	Upgrade.ModifierValue = 0.1f;
	Upgrade.ModifierOp = EGameplayModOp::Additive;
	BuyUpgrade(Upgrade);
}

void UWeaponTalentWidget::OnUpgradeCooldownClicked()
{
	FWeaponUpgrade Upgrade;
	Upgrade.Name = FText::FromString("Cooldown");
	Upgrade.Attribute = UWeaponAttributeSet::GetCooldownMultiplierAttribute();
	Upgrade.LevelAttribute = UWeaponAttributeSet::GetCooldownTalentLevelAttribute();
	Upgrade.ModifierValue = 0.95f;
	Upgrade.ModifierOp = EGameplayModOp::Multiplicitive;
	BuyUpgrade(Upgrade);
}

void UWeaponTalentWidget::OnUpgradeReloadClicked()
{
	FWeaponUpgrade Upgrade;
	Upgrade.Name = FText::FromString("Reload Speed");
	Upgrade.Attribute = UWeaponAttributeSet::GetReloadSpeedMultiplierAttribute();
	Upgrade.LevelAttribute = UWeaponAttributeSet::GetReloadSpeedTalentLevelAttribute();
	Upgrade.ModifierValue = 0.95f;
	Upgrade.ModifierOp = EGameplayModOp::Multiplicitive;
	BuyUpgrade(Upgrade);
}

void UWeaponTalentWidget::OnUpgradePierceClicked()
{
	FWeaponUpgrade Upgrade;
	Upgrade.Name = FText::FromString("Pierce");
	Upgrade.Attribute = UWeaponAttributeSet::GetPierceExtraCountAttribute();
	Upgrade.LevelAttribute = UWeaponAttributeSet::GetPierceTalentLevelAttribute();
	Upgrade.ModifierValue = 1.0f;
	Upgrade.ModifierOp = EGameplayModOp::Additive;
	BuyUpgrade(Upgrade);
}

void UWeaponTalentWidget::OnUpgradeProjectileClicked()
{
	FWeaponUpgrade Upgrade;
	Upgrade.Name = FText::FromString("Multi-Shot");
	Upgrade.Attribute = UWeaponAttributeSet::GetProjectileExtraCountAttribute();
	Upgrade.LevelAttribute = UWeaponAttributeSet::GetProjectileTalentLevelAttribute();
	Upgrade.ModifierValue = 1.0f;
	Upgrade.ModifierOp = EGameplayModOp::Additive;
	BuyUpgrade(Upgrade);
}

void UWeaponTalentWidget::OnUpgradeAmmoClicked()
{
	FWeaponUpgrade Upgrade;
	Upgrade.Name = FText::FromString("Max Ammo");
	Upgrade.Attribute = UWeaponAttributeSet::GetMaxAmmoAttribute();
	Upgrade.LevelAttribute = UWeaponAttributeSet::GetMaxAmmoTalentLevelAttribute();
	Upgrade.ModifierValue = 5.0f;
	Upgrade.ModifierOp = EGameplayModOp::Additive;
	BuyUpgrade(Upgrade);
}

void UWeaponTalentWidget::OnUpgradeMagazinesClicked()
{
	FWeaponUpgrade Upgrade;
	Upgrade.Name = FText::FromString("Magazines");
	Upgrade.Attribute = UWeaponAttributeSet::GetMaxMagazinesAttribute();
	Upgrade.LevelAttribute = UWeaponAttributeSet::GetAmountMagazinesTalentLevelAttribute();
	Upgrade.ModifierValue = 1.0f;
	Upgrade.ModifierOp = EGameplayModOp::Additive;
	BuyUpgrade(Upgrade);
}

void UWeaponTalentWidget::OnResetClicked()
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

float UWeaponTalentWidget::GetTalentPoints() const
{
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		if (WeaponComp->WeaponAttributes)
		{
			return WeaponComp->WeaponAttributes->GetWeaponTalentPoints();
		}
	}
	return 0.0f;
}

bool UWeaponTalentWidget::BuyUpgrade(FWeaponUpgrade Upgrade)
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

void UWeaponTalentWidget::SetTargetUnit(AUnitBase* InUnit)
{
	TargetUnit = InUnit;
}

UWeaponComponent* UWeaponTalentWidget::GetWeaponComponent() const
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
