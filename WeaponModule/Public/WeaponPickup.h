// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Pickup.h"
#include "WeaponComponent.h"
#include "WeaponPickup.generated.h"

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API AWeaponPickup : public APickup
{
	GENERATED_BODY()

public:
	AWeaponPickup();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UDataTable* WeaponDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponRowName = NAME_None;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	FWeaponData WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsMagazineOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 EffectAreaIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AmountToAdd = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SyncWeaponDataFromTable();

	UFUNCTION()
	void HandlePickupImpact(APickup* Pickup, AUnitBase* Unit);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
