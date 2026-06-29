// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/CrowdControlTalentWidget.h"
#include "Abilities/WeaponAttributeSet.h"
#include "Components/WeaponHUDComponent.h"
#include "Characters/Unit/UnitBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"

void UCrowdControlTalentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RangeButton)      RangeButton->OnClicked.AddDynamic(this, &UCrowdControlTalentWidget::OnRangeClicked);
	if (DurationButton)   DurationButton->OnClicked.AddDynamic(this, &UCrowdControlTalentWidget::OnDurationClicked);
	if (AngleButton)      AngleButton->OnClicked.AddDynamic(this, &UCrowdControlTalentWidget::OnAngleClicked);
	if (MaxTargetsButton) MaxTargetsButton->OnClicked.AddDynamic(this, &UCrowdControlTalentWidget::OnMaxTargetsClicked);
	if (HeightButton)     HeightButton->OnClicked.AddDynamic(this, &UCrowdControlTalentWidget::OnHeightClicked);
	if (CooldownButton)   CooldownButton->OnClicked.AddDynamic(this, &UCrowdControlTalentWidget::OnCooldownClicked);
	if (ResetButton)      ResetButton->OnClicked.AddDynamic(this, &UCrowdControlTalentWidget::OnResetClicked);
}

void UCrowdControlTalentWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const float Points = GetCrowdControlTalentPoints();
	if (TalentPointsText)
	{
		TalentPointsText->SetText(FText::AsNumber(Points));
	}

	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp)
	{
		return;
	}

	const FCrowdControlData& CC = WeaponComp->CrowdControl;
	const bool bCanInvest = (Points >= WeaponComp->CrowdControlTalentCost);

	// Levels
	if (RangeLevelText)      RangeLevelText->SetText(FText::AsNumber(CC.RangeLevel));
	if (DurationLevelText)   DurationLevelText->SetText(FText::AsNumber(CC.DurationLevel));
	if (AngleLevelText)      AngleLevelText->SetText(FText::AsNumber(CC.AngleLevel));
	if (MaxTargetsLevelText) MaxTargetsLevelText->SetText(FText::AsNumber(CC.MaxTargetsLevel));
	if (HeightLevelText)     HeightLevelText->SetText(FText::AsNumber(CC.HeightLevel));
	if (CooldownLevelText)   CooldownLevelText->SetText(FText::AsNumber(CC.CooldownLevel));

	// Effective values
	if (RangeValueText)      RangeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), WeaponComp->GetCrowdControlRadius())));
	if (DurationValueText)   DurationValueText->SetText(FText::FromString(FString::Printf(TEXT("%.1fs"), WeaponComp->GetCrowdControlDuration())));
	if (AngleValueText)      AngleValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f deg"), WeaponComp->GetCrowdControlHalfAngleDeg() * 2.f)));
	if (MaxTargetsValueText) MaxTargetsValueText->SetText(FText::AsNumber(WeaponComp->GetCrowdControlMaxTargets()));
	if (HeightValueText)     HeightValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), WeaponComp->GetCrowdControlHalfHeight() * 2.f)));
	if (CooldownValueText)   CooldownValueText->SetText(FText::FromString(FString::Printf(TEXT("%.1fs"), WeaponComp->GetCrowdControlCooldown())));

	// Enable/disable invest buttons by available points.
	if (RangeButton)      RangeButton->SetIsEnabled(bCanInvest);
	if (DurationButton)   DurationButton->SetIsEnabled(bCanInvest);
	if (AngleButton)      AngleButton->SetIsEnabled(bCanInvest);
	if (MaxTargetsButton) MaxTargetsButton->SetIsEnabled(bCanInvest);
	if (HeightButton)     HeightButton->SetIsEnabled(bCanInvest);
	if (CooldownButton)   CooldownButton->SetIsEnabled(bCanInvest);
}

float UCrowdControlTalentWidget::GetCrowdControlTalentPoints() const
{
	// Prefer the live, level-driven value from the weapon component (updates immediately on level-up).
	if (UWeaponComponent* WeaponComp = GetWeaponComponent())
	{
		return WeaponComp->GetAvailableCrowdControlPoints();
	}

	// Fallback: the replicated attribute mirror.
	if (!TargetUnit)
	{
		return 0.f;
	}
	UAbilitySystemComponent* ASC = TargetUnit->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return 0.f;
	}
	const UWeaponAttributeSet* WeaponAttributes = ASC->GetSet<UWeaponAttributeSet>();
	return WeaponAttributes ? WeaponAttributes->GetCrowdControlTalentPoints() : 0.f;
}

void UCrowdControlTalentWidget::SetTargetUnit(AUnitBase* InUnit)
{
	TargetUnit = InUnit;
}

UWeaponComponent* UCrowdControlTalentWidget::GetWeaponComponent() const
{
	return TargetUnit ? TargetUnit->FindComponentByClass<UWeaponComponent>() : nullptr;
}

void UCrowdControlTalentWidget::InvestTalent(ECrowdControlTalent Talent)
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp)
	{
		return;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			HUDComp->Server_InvestInCrowdControlTalent(WeaponComp, Talent);
			return;
		}
	}
	WeaponComp->Server_InvestInCrowdControlTalent(Talent);
}

void UCrowdControlTalentWidget::OnRangeClicked()      { InvestTalent(ECrowdControlTalent::Range); }
void UCrowdControlTalentWidget::OnDurationClicked()   { InvestTalent(ECrowdControlTalent::Duration); }
void UCrowdControlTalentWidget::OnAngleClicked()      { InvestTalent(ECrowdControlTalent::Angle); }
void UCrowdControlTalentWidget::OnMaxTargetsClicked() { InvestTalent(ECrowdControlTalent::MaxTargets); }
void UCrowdControlTalentWidget::OnHeightClicked()     { InvestTalent(ECrowdControlTalent::Height); }
void UCrowdControlTalentWidget::OnCooldownClicked()   { InvestTalent(ECrowdControlTalent::Cooldown); }

void UCrowdControlTalentWidget::OnResetClicked()
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp)
	{
		return;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			HUDComp->Server_ResetCrowdControlTalents(WeaponComp);
			return;
		}
	}
	WeaponComp->Server_ResetCrowdControlTalents();
}
