// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/WeaponSelectionHUDWidget.h"
#include "Components/WeaponComponent.h"
#include "WeaponHUDComponent.generated.h"

class ACustomControllerBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WEAPONMODULE_API UWeaponHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWeaponHUDComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category = "Weapon|UI")
	TSubclassOf<UWeaponSelectionHUDWidget> WeaponSelectionWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|UI")
	UWeaponSelectionHUDWidget* WeaponSelectionWidgetInstance;

	UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
	void RegisterWeaponHUD(UWeaponSelectionHUDWidget* InWidget, ACustomControllerBase* InController);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_PurchaseUpgrade(UWeaponComponent* WeaponComp, FWeaponUpgrade Upgrade);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_SelectEffectTalent(UWeaponComponent* WeaponComp, int32 Index);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_ToggleEffectAreaTalent(UWeaponComponent* WeaponComp, int32 AreaIndex, int32 TalentIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_InvestInEffectAreaRadius(UWeaponComponent* WeaponComp, int32 AreaIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_InvestInEffectAreaDamage(UWeaponComponent* WeaponComp, int32 AreaIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_ResetCurrentWeaponTalents(UWeaponComponent* WeaponComp);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_ResetEffectAreaTalents(UWeaponComponent* WeaponComp, int32 AreaIndex);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_SelectEffectAreaIndex(UWeaponComponent* WeaponComp, int32 Index);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_InvestInCrowdControlTalent(UWeaponComponent* WeaponComp, ECrowdControlTalent Talent);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_ResetCrowdControlTalents(UWeaponComponent* WeaponComp);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_InvestInTalentTreeNode(UWeaponComponent* WeaponComp, FName NodeId);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Talents")
	void Server_ResetTalentTree(UWeaponComponent* WeaponComp);

	/** Buy StoreItems[ItemIndex] from the unit's currently-overlapped store (server-authoritative). */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Weapon|Store")
	void Server_BuyStoreItem(UWeaponComponent* WeaponComp, int32 ItemIndex);
};
