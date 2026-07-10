// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponStoreWidget.generated.h"

class AUnitBase;
class AWeaponStore;
class UWeaponComponent;
class UWeaponStoreItemWidget;

/**
 * The shop panel. Opened by AWeaponStore on the owning client for a unit that carries a
 * UWeaponComponent. It shows the unit's current Gold, builds one UWeaponStoreItemWidget per
 * StoreItems entry, and routes clicks to the PC-owned UWeaponHUDComponent server RPC. It follows
 * the same SetTargetUnit/GetTargetUnit toggle convention as the talent widgets.
 */
UCLASS()
class WEAPONMODULE_API UWeaponStoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Store")
	void SetTargetUnit(AUnitBase* Unit);

	UFUNCTION(BlueprintPure, Category = "Store")
	AUnitBase* GetTargetUnit() const;

	UFUNCTION(BlueprintCallable, Category = "Store")
	void SetStoreActor(AWeaponStore* Store);

	UFUNCTION(BlueprintPure, Category = "Store")
	AWeaponStore* GetStoreActor() const;

	/** Rebuild the item rows from the store when bRebuild is true; always refreshes gold + affordability. */
	UFUNCTION(BlueprintCallable, Category = "Store")
	void RefreshStore(bool bRebuild);

	/** Called by a row widget on click -> fires the server purchase RPC for StoreItems[Index]. */
	UFUNCTION(BlueprintCallable, Category = "Store")
	void TryBuyItem(int32 Index);

	UFUNCTION(BlueprintPure, Category = "Store")
	int32 GetCurrentGold() const;

protected:
	/** Class used to instantiate each item row. Set on the WBP. */
	UPROPERTY(EditAnywhere, Category = "Store")
	TSubclassOf<UWeaponStoreItemWidget> ItemWidgetClass;

	/** Panel the item rows are added to (a Vertical Box / Scroll Box named exactly "ItemsContainer"). */
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* ItemsContainer;

	/** Shows the target unit's current Gold. */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GoldText;

	UPROPERTY(BlueprintReadOnly, Category = "Store")
	TWeakObjectPtr<AUnitBase> TargetUnit;

	UPROPERTY(BlueprintReadOnly, Category = "Store")
	TWeakObjectPtr<AWeaponStore> StoreActor;

	UPROPERTY()
	TArray<UWeaponStoreItemWidget*> ItemWidgets;

	UWeaponComponent* GetTargetWeaponComponent() const;
	void RebuildItems();
	void RefreshAffordability();
};
