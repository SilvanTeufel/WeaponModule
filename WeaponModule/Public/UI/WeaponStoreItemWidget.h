// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Store/StoreTypes.h"
#include "WeaponStoreItemWidget.generated.h"

class UWeaponStoreWidget;

/**
 * One purchasable row in the store list. The parent UWeaponStoreWidget instantiates one of these
 * per FStoreItemEntry and binds it via Setup(). Clicking BuyButton routes back to the parent, which
 * fires the server purchase RPC. Design the visuals in a WBP child; all BindWidgets are optional.
 */
UCLASS()
class WEAPONMODULE_API UWeaponStoreItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Bind this row to a store item (index into the store's StoreItems). */
	UFUNCTION(BlueprintCallable, Category = "Store")
	void Setup(UWeaponStoreWidget* InOwner, int32 InIndex, const FStoreItemEntry& InEntry);

	/** Enable/disable + tint the buy button based on whether the unit can afford it. */
	UFUNCTION(BlueprintCallable, Category = "Store")
	void SetAffordable(bool bAffordable);

	UFUNCTION(BlueprintPure, Category = "Store")
	int32 GetItemIndex() const { return ItemIndex; }

protected:
	UPROPERTY(meta = (BindWidget))
	class UButton* BuyButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CostText;

	UPROPERTY(meta = (BindWidget))
	class UImage* IconImage;

	UPROPERTY(BlueprintReadOnly, Category = "Store")
	int32 ItemIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Store")
	TWeakObjectPtr<UWeaponStoreWidget> OwnerStoreWidget;

	UPROPERTY(EditAnywhere, Category = "Store")
	FLinearColor AffordableColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Store")
	FLinearColor UnaffordableColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.f);

	UFUNCTION()
	void OnBuyClicked();
};
