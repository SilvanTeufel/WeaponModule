// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/WeaponSelectionHUDWidget.h"
#include "UI/WeaponTalentWidget.h"
#include "UI/WeaponEffectTalentWidget.h"
#include "UI/EffectAreaTalentWidget.h"
#include "Hud/HUDBase.h"
#include "Characters/Unit/UnitBase.h"
#include "Components/WeaponComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Controller/PlayerController/CustomControllerBase.h"
#include "TimerManager.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"

void UWeaponSelectionHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CreateHUDWidgets();
}

void UWeaponSelectionHUDWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	CreateHUDWidgets();
}

void UWeaponSelectionHUDWidget::CreateHUDWidgets()
{
	if (WeaponHUDContainer && WeaponHUDWidgetClass)
	{
		WeaponHUDWidgets.Empty();
		WeaponHUDContainer->ClearChildren();

		for (int32 i = 0; i < MaxDisplayedUnits; ++i)
		{
			if (UWeaponHUDWidget* NewWidget = CreateWidget<UWeaponHUDWidget>(this, WeaponHUDWidgetClass))
			{
				UPanelSlot* PanelSlot = WeaponHUDContainer->AddChild(NewWidget);
				
				int32 Row = MaxColumns > 0 ? i / MaxColumns : 0;
				int32 Column = MaxColumns > 0 ? i % MaxColumns : i;

				if (UUniformGridSlot* UniformGridSlot = Cast<UUniformGridSlot>(PanelSlot))
				{
					UniformGridSlot->SetRow(Row);
					UniformGridSlot->SetColumn(Column);
				}
				else if (UGridSlot* GridSlot = Cast<UGridSlot>(PanelSlot))
				{
					GridSlot->SetRow(Row);
					GridSlot->SetColumn(Column);
					GridSlot->SetPadding(WidgetPadding);
				}
				
				WeaponHUDWidgets.Add(NewWidget);
				
				if (IsDesignTime())
				{
					NewWidget->SetVisibility(ESlateVisibility::Visible);
				}
				else
				{
					NewWidget->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}

		if (IsDesignTime() && WeaponTalentContainer && WeaponTalentWidgetClass)
		{
			WeaponTalentContainer->ClearChildren();
			if (UWeaponTalentWidget* PreviewWidget = CreateWidget<UWeaponTalentWidget>(this, WeaponTalentWidgetClass))
			{
				WeaponTalentContainer->AddChild(PreviewWidget);
			}
		}

		if (IsDesignTime() && WeaponEffectTalentContainer && WeaponEffectTalentWidgetClass)
		{
			WeaponEffectTalentContainer->ClearChildren();
			if (UWeaponEffectTalentWidget* PreviewWidget = CreateWidget<UWeaponEffectTalentWidget>(this, WeaponEffectTalentWidgetClass))
			{
				WeaponEffectTalentContainer->AddChild(PreviewWidget);
			}
		}

		if (IsDesignTime() && EffectAreaTalentContainer && WeaponEffectAreaTalentWidgetClass)
		{
			EffectAreaTalentContainer->ClearChildren();
			if (UEffectAreaTalentWidget* PreviewWidget = CreateWidget<UEffectAreaTalentWidget>(this, WeaponEffectAreaTalentWidgetClass))
			{
				EffectAreaTalentContainer->AddChild(PreviewWidget);
			}
		}
	}
}

void UWeaponSelectionHUDWidget::ToggleEffectAreaTalentWidget(AUnitBase* Unit)
{
	if (!EffectAreaTalentContainer || !WeaponEffectAreaTalentWidgetClass) return;

	if (EffectAreaTalentWidgetInstance)
	{
		if (EffectAreaTalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible && EffectAreaTalentWidgetInstance->GetTargetUnit() == Unit)
		{
			EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}
	else
	{
		EffectAreaTalentWidgetInstance = CreateWidget<UEffectAreaTalentWidget>(this, WeaponEffectAreaTalentWidgetClass);
		if (EffectAreaTalentWidgetInstance)
		{
			EffectAreaTalentContainer->AddChild(EffectAreaTalentWidgetInstance);
		}
	}

	if (EffectAreaTalentWidgetInstance)
	{
		EffectAreaTalentWidgetInstance->SetTargetUnit(Unit);
		EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		// Hide other widgets
		if (TalentWidgetInstance) TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectTalentWidgetInstance) EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::ToggleTalentWidget(AUnitBase* Unit)
{
	if (!WeaponTalentContainer || !WeaponTalentWidgetClass) return;

	// Wenn das Widget bereits existiert und die gleiche Unit anzeigt, dann toggeln wir es weg (Collapsed)
	if (TalentWidgetInstance)
	{
		if (TalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible && TalentWidgetInstance->GetTargetUnit() == Unit)
		{
			TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}
	else
	{
		// Erstellen, falls noch nicht vorhanden
		TalentWidgetInstance = CreateWidget<UWeaponTalentWidget>(this, WeaponTalentWidgetClass);
		if (TalentWidgetInstance)
		{
			WeaponTalentContainer->AddChild(TalentWidgetInstance);
		}
	}

	if (TalentWidgetInstance)
	{
		TalentWidgetInstance->SetTargetUnit(Unit);
		TalentWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		// Hide other widgets
		if (EffectTalentWidgetInstance) EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectAreaTalentWidgetInstance) EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::ToggleEffectTalentWidget(AUnitBase* Unit)
{
	if (!WeaponEffectTalentContainer || !WeaponEffectTalentWidgetClass) return;

	// Wenn das Widget bereits existiert und die gleiche Unit anzeigt, dann toggeln wir es weg (Collapsed)
	if (EffectTalentWidgetInstance)
	{
		if (EffectTalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible && EffectTalentWidgetInstance->GetTargetUnit() == Unit)
		{
			EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}
	else
	{
		// Erstellen, falls noch nicht vorhanden
		EffectTalentWidgetInstance = CreateWidget<UWeaponEffectTalentWidget>(this, WeaponEffectTalentWidgetClass);
		if (EffectTalentWidgetInstance)
		{
			WeaponEffectTalentContainer->AddChild(EffectTalentWidgetInstance);
		}
	}

	if (EffectTalentWidgetInstance)
	{
		EffectTalentWidgetInstance->SetTargetUnit(Unit);
		EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		// Hide other widgets
		if (TalentWidgetInstance) TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectAreaTalentWidgetInstance) EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::InitWidget(ACustomControllerBase* InController)
{
	if (InController)
	{
		ControllerBase = InController;
		GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UWeaponSelectionHUDWidget::UpdateSelection, UpdateInterval, true);
	}
}

void UWeaponSelectionHUDWidget::UpdateSelection()
{
	if (!ControllerBase || !ControllerBase->GetHUD()) return;

	AHUDBase* HUD = Cast<AHUDBase>(ControllerBase->GetHUD());
	if (!HUD) return;

	TArray<AUnitBase*> SelectedUnitsWithWeapons;
	for (AUnitBase* Unit : HUD->SelectedUnits)
	{
		if (Unit && Unit->FindComponentByClass<UWeaponComponent>())
		{
			SelectedUnitsWithWeapons.Add(Unit);
			if (SelectedUnitsWithWeapons.Num() >= MaxDisplayedUnits) break;
		}
	}

	for (int32 i = 0; i < WeaponHUDWidgets.Num(); ++i)
	{
		if (SelectedUnitsWithWeapons.IsValidIndex(i))
		{
			WeaponHUDWidgets[i]->UpdateWidget(SelectedUnitsWithWeapons[i]);
			// The UpdateWidget call already sets visibility to Visible if unit is valid
		}
		else
		{
			WeaponHUDWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update Talent Widget if visible
	if (TalentWidgetInstance && TalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SelectedUnitsWithWeapons.Num() > 0)
		{
			// If the current target unit is no longer in selection, or if it's a single re-selection,
			// update to the first selected unit with a weapon.
			if (SelectedUnitsWithWeapons.Num() == 1 || !SelectedUnitsWithWeapons.Contains(TalentWidgetInstance->GetTargetUnit()))
			{
				if (TalentWidgetInstance->GetTargetUnit() != SelectedUnitsWithWeapons[0])
				{
					TalentWidgetInstance->SetTargetUnit(SelectedUnitsWithWeapons[0]);
				}
			}
		}
		else
		{
			TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update Effect Talent Widget if visible
	if (EffectTalentWidgetInstance && EffectTalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SelectedUnitsWithWeapons.Num() > 0)
		{
			if (SelectedUnitsWithWeapons.Num() == 1 || !SelectedUnitsWithWeapons.Contains(EffectTalentWidgetInstance->GetTargetUnit()))
			{
				if (EffectTalentWidgetInstance->GetTargetUnit() != SelectedUnitsWithWeapons[0])
				{
					EffectTalentWidgetInstance->SetTargetUnit(SelectedUnitsWithWeapons[0]);
				}
			}
		}
		else
		{
			EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update Effect Area Talent Widget if visible
	if (EffectAreaTalentWidgetInstance && EffectAreaTalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SelectedUnitsWithWeapons.Num() > 0)
		{
			if (SelectedUnitsWithWeapons.Num() == 1 || !SelectedUnitsWithWeapons.Contains(EffectAreaTalentWidgetInstance->GetTargetUnit()))
			{
				EffectAreaTalentWidgetInstance->SetTargetUnit(SelectedUnitsWithWeapons[0]);
			}
		}
		else
		{
			EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
