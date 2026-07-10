// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/WeaponSelectionHUDWidget.h"
#include "UI/WeaponTalentWidget.h"
#include "UI/WeaponEffectTalentWidget.h"
#include "UI/EffectAreaTalentWidget.h"
#include "UI/CrowdControlTalentWidget.h"
#include "UI/TalentTreeWidget.h"
#include "UI/WeaponStoreWidget.h"
#include "Actors/WeaponStore.h"
#include "Widgets/TalentChooser.h"
#include "Characters/Unit/LevelUnit.h"
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
#include "Components/CanvasPanelSlot.h"

void UWeaponSelectionHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CreateHUDWidgets();
}

void UWeaponSelectionHUDWidget::NativeDestruct()
{
	// The store widget may have been added directly to the viewport (when no StoreContainer is bound),
	// so it is not torn down with this widget's tree. Remove it explicitly to avoid an orphan.
	if (StoreWidgetInstance)
	{
		StoreWidgetInstance->RemoveFromParent();
		StoreWidgetInstance = nullptr;
	}
	Super::NativeDestruct();
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

		if (IsDesignTime() && CrowdControlTalentContainer && CrowdControlTalentWidgetClass)
		{
			CrowdControlTalentContainer->ClearChildren();
			if (UCrowdControlTalentWidget* PreviewWidget = CreateWidget<UCrowdControlTalentWidget>(this, CrowdControlTalentWidgetClass))
			{
				CrowdControlTalentContainer->AddChild(PreviewWidget);
			}
		}

		if (IsDesignTime() && TalentTreeContainer && WidgetTree)
		{
			TalentTreeContainer->ClearChildren();
			TSubclassOf<UTalentTreeWidget> ClassToUse = TalentTreeWidgetClass;
			if (!ClassToUse) ClassToUse = UTalentTreeWidget::StaticClass();
			if (UTalentTreeWidget* PreviewWidget = WidgetTree->ConstructWidget<UTalentTreeWidget>(ClassToUse))
			{
				UPanelSlot* PreviewSlot = TalentTreeContainer->AddChild(PreviewWidget);
				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(PreviewSlot))
				{
					CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
					CanvasSlot->SetOffsets(FMargin(0.f));
				}
			}
		}

		if (IsDesignTime() && TalentChooserContainer && TalentChooserWidgetClass)
		{
			TalentChooserContainer->ClearChildren();
			if (UTalentChooser* PreviewWidget = CreateWidget<UTalentChooser>(this, TalentChooserWidgetClass))
			{
				TalentChooserContainer->AddChild(PreviewWidget);
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
		if (CrowdControlTalentWidgetInstance) CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentTreeWidgetInstance) TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentChooserWidgetInstance) { TalentChooserWidgetInstance->StopTimer(); TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); }
		if (StoreWidgetInstance) StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::ToggleTalentTreeWidget(AUnitBase* Unit)
{
	if (!TalentTreeContainer) return;

	if (TalentTreeWidgetInstance)
	{
		if (TalentTreeWidgetInstance->GetVisibility() == ESlateVisibility::Visible && TalentTreeWidgetInstance->GetTargetUnit() == Unit)
		{
			TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}
	else
	{
		TSubclassOf<UTalentTreeWidget> ClassToUse = TalentTreeWidgetClass;
		if (!ClassToUse) ClassToUse = UTalentTreeWidget::StaticClass();
		TalentTreeWidgetInstance = WidgetTree->ConstructWidget<UTalentTreeWidget>(ClassToUse);
		if (TalentTreeWidgetInstance)
		{
			UPanelSlot* NewSlot = TalentTreeContainer->AddChild(TalentTreeWidgetInstance);
			// Fill the container so the (self-centering) tree is centred in it. Make the container
			// itself a large / full-screen centred panel for a viewport-centred tree.
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(NewSlot))
			{
				CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
				CanvasSlot->SetOffsets(FMargin(0.f));
			}
		}
	}

	if (TalentTreeWidgetInstance)
	{
		TalentTreeWidgetInstance->SetTargetUnit(Unit);
		TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		// Hide other widgets
		if (TalentWidgetInstance) TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectTalentWidgetInstance) EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectAreaTalentWidgetInstance) EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (CrowdControlTalentWidgetInstance) CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentChooserWidgetInstance) { TalentChooserWidgetInstance->StopTimer(); TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); }
		if (StoreWidgetInstance) StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::ToggleTalentChooserWidget(AUnitBase* Unit)
{
	if (!TalentChooserContainer || !TalentChooserWidgetClass) return;

	ALevelUnit* LevelUnit = Cast<ALevelUnit>(Unit);

	if (TalentChooserWidgetInstance)
	{
		if (TalentChooserWidgetInstance->GetVisibility() == ESlateVisibility::Visible && TalentChooserWidgetInstance->GetOwnerActor() == LevelUnit)
		{
			TalentChooserWidgetInstance->StopTimer();
			TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}
	else
	{
		TalentChooserWidgetInstance = CreateWidget<UTalentChooser>(this, TalentChooserWidgetClass);
		if (TalentChooserWidgetInstance)
		{
			TalentChooserContainer->AddChild(TalentChooserWidgetInstance);
		}
	}

	if (TalentChooserWidgetInstance)
	{
		TalentChooserWidgetInstance->SetOwnerActor(LevelUnit);
		TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		TalentChooserWidgetInstance->CreateClassUIElements();
		TalentChooserWidgetInstance->StartUpdateTimer();

		// Hide other widgets
		if (TalentWidgetInstance) TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectTalentWidgetInstance) EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectAreaTalentWidgetInstance) EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (CrowdControlTalentWidgetInstance) CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentTreeWidgetInstance) TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (StoreWidgetInstance) StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::ToggleCrowdControlTalentWidget(AUnitBase* Unit)
{
	if (!CrowdControlTalentContainer || !CrowdControlTalentWidgetClass) return;

	if (CrowdControlTalentWidgetInstance)
	{
		if (CrowdControlTalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible && CrowdControlTalentWidgetInstance->GetTargetUnit() == Unit)
		{
			CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}
	}
	else
	{
		CrowdControlTalentWidgetInstance = CreateWidget<UCrowdControlTalentWidget>(this, CrowdControlTalentWidgetClass);
		if (CrowdControlTalentWidgetInstance)
		{
			CrowdControlTalentContainer->AddChild(CrowdControlTalentWidgetInstance);
		}
	}

	if (CrowdControlTalentWidgetInstance)
	{
		CrowdControlTalentWidgetInstance->SetTargetUnit(Unit);
		CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		// Hide other widgets
		if (TalentWidgetInstance) TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectTalentWidgetInstance) EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (EffectAreaTalentWidgetInstance) EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentTreeWidgetInstance) TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentChooserWidgetInstance) { TalentChooserWidgetInstance->StopTimer(); TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); }
		if (StoreWidgetInstance) StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
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
		if (CrowdControlTalentWidgetInstance) CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentTreeWidgetInstance) TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentChooserWidgetInstance) { TalentChooserWidgetInstance->StopTimer(); TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); }
		if (StoreWidgetInstance) StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
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
		if (CrowdControlTalentWidgetInstance) CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentTreeWidgetInstance) TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		if (TalentChooserWidgetInstance) { TalentChooserWidgetInstance->StopTimer(); TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); }
		if (StoreWidgetInstance) StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::OpenStoreForUnit(AUnitBase* Unit, AWeaponStore* Store)
{
	if (!Unit || !Store) return;

	TSubclassOf<UWeaponStoreWidget> ClassToUse = Store->StoreWidgetClass ? Store->StoreWidgetClass : StoreWidgetClass;
	if (!ClassToUse) return;

	// If a different store wants a different widget class, drop the cached instance so we honor the override.
	if (StoreWidgetInstance && StoreWidgetInstance->GetClass() != ClassToUse)
	{
		StoreWidgetInstance->RemoveFromParent();
		StoreWidgetInstance = nullptr;
	}

	if (!StoreWidgetInstance)
	{
		StoreWidgetInstance = CreateWidget<UWeaponStoreWidget>(GetOwningPlayer(), ClassToUse);
		if (!StoreWidgetInstance) return;

		if (StoreContainer)
		{
			StoreContainer->AddChild(StoreWidgetInstance);
		}
		else
		{
			StoreWidgetInstance->AddToViewport(1000); // high ZOrder so it is not hidden behind the main HUD
		}
	}

	StoreWidgetInstance->SetStoreActor(Store);
	StoreWidgetInstance->SetTargetUnit(Unit);
	StoreWidgetInstance->RefreshStore(true);
	StoreWidgetInstance->SetVisibility(ESlateVisibility::Visible);

	// Mutually exclusive with the talent panels.
	if (TalentWidgetInstance) TalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (EffectTalentWidgetInstance) EffectTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (EffectAreaTalentWidgetInstance) EffectAreaTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (CrowdControlTalentWidgetInstance) CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (TalentTreeWidgetInstance) TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	if (TalentChooserWidgetInstance) { TalentChooserWidgetInstance->StopTimer(); TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); }
}

void UWeaponSelectionHUDWidget::CloseStoreForUnit(AUnitBase* Unit, AWeaponStore* Store)
{
	if (!StoreWidgetInstance) return;

	// Only close if we are showing exactly this unit (+ store), so leaving store A doesn't close store B.
	if (StoreWidgetInstance->GetTargetUnit() == Unit && (Store == nullptr || StoreWidgetInstance->GetStoreActor() == Store))
	{
		StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponSelectionHUDWidget::ToggleStoreWidget(AUnitBase* Unit)
{
	if (!Unit) return;

	if (StoreWidgetInstance
		&& StoreWidgetInstance->GetVisibility() == ESlateVisibility::Visible
		&& StoreWidgetInstance->GetTargetUnit() == Unit)
	{
		StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	AWeaponStore* Store = nullptr;
	if (UWeaponComponent* WC = Unit->FindComponentByClass<UWeaponComponent>())
	{
		Store = WC->GetCurrentStore();
	}
	if (Store)
	{
		OpenStoreForUnit(Unit, Store);
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

	// Update Crowd-Control Talent Widget if visible
	if (CrowdControlTalentWidgetInstance && CrowdControlTalentWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SelectedUnitsWithWeapons.Num() > 0)
		{
			if (SelectedUnitsWithWeapons.Num() == 1 || !SelectedUnitsWithWeapons.Contains(CrowdControlTalentWidgetInstance->GetTargetUnit()))
			{
				CrowdControlTalentWidgetInstance->SetTargetUnit(SelectedUnitsWithWeapons[0]);
			}
		}
		else
		{
			CrowdControlTalentWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update Talent Tree Widget if visible
	if (TalentTreeWidgetInstance && TalentTreeWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SelectedUnitsWithWeapons.Num() > 0)
		{
			if (SelectedUnitsWithWeapons.Num() == 1 || !SelectedUnitsWithWeapons.Contains(TalentTreeWidgetInstance->GetTargetUnit()))
			{
				if (TalentTreeWidgetInstance->GetTargetUnit() != SelectedUnitsWithWeapons[0])
				{
					TalentTreeWidgetInstance->SetTargetUnit(SelectedUnitsWithWeapons[0]);
				}
			}
		}
		else
		{
			TalentTreeWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update Talent Chooser Widget if visible
	if (TalentChooserWidgetInstance && TalentChooserWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		if (SelectedUnitsWithWeapons.Num() > 0)
		{
			ALevelUnit* FirstLevelUnit = Cast<ALevelUnit>(SelectedUnitsWithWeapons[0]);
			if (FirstLevelUnit && TalentChooserWidgetInstance->GetOwnerActor() != FirstLevelUnit)
			{
				TalentChooserWidgetInstance->SetOwnerActor(FirstLevelUnit);
				TalentChooserWidgetInstance->CreateClassUIElements();
			}
		}
		else
		{
			TalentChooserWidgetInstance->StopTimer();
			TalentChooserWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Update Store Widget if visible. Its open/close lifecycle is driven by the store actor's LOCAL
	// overlap begin/end (OpenStoreForUnit / CloseStoreForUnit), so here we only refresh gold/affordability
	// and close if the shopping unit itself is gone. We deliberately do NOT close based on the replicated
	// WeaponComponent::CurrentStore, which can lag one round-trip behind the local overlap that opened us.
	if (StoreWidgetInstance && StoreWidgetInstance->GetVisibility() == ESlateVisibility::Visible)
	{
		if (StoreWidgetInstance->GetTargetUnit())
		{
			StoreWidgetInstance->RefreshStore(false);
		}
		else
		{
			StoreWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
