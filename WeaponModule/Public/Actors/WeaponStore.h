// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Store/StoreTypes.h"
#include "WeaponStore.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AUnitBase;
class UWeaponStoreWidget;

/**
 * A placeable "shop": when a unit that carries a UWeaponComponent and belongs to TeamId walks
 * into the trigger sphere, the owning player's client opens a Store widget where the unit's Gold
 * can be traded for the entries in StoreItems. All purchases are validated & applied server-side
 * on the unit's UWeaponComponent; the widget is presentation + input only.
 *
 * Composition happens entirely on this actor: add exactly the StoreItems entries you want to sell.
 */
UCLASS()
class WEAPONMODULE_API AWeaponStore : public AActor
{
	GENERATED_BODY()

public:
	AWeaponStore();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Team allowed to use this store (matched against AUnitBase::TeamId and the local player's SelectableTeamId). */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 TeamId = 1;

	/** The items this store sells. Freely composed in the editor; every EStoreItemType is optional.
	 *  Replicated so stores spawned at runtime (not just level-placed) populate the client widget. */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Store")
	TArray<FStoreItemEntry> StoreItems;

	/** Optional per-store widget class. If null, the player's selection-HUD default StoreWidgetClass is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	TSubclassOf<UWeaponStoreWidget> StoreWidgetClass;

	/** Radius (cm) of the overlap trigger. Applied to TriggerSphere in the constructor / OnConstruction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	float TriggerRadius = 250.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store")
	USphereComponent* TriggerSphere;

	/** Optional visual mesh so the store is visible in the world (no collision). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Store")
	UStaticMeshComponent* DisplayMesh;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Returns the unit if it is a valid, matching-team customer that carries a UWeaponComponent. */
	AUnitBase* ResolveCustomer(AActor* OtherActor) const;

	/** Opens the store widget on THIS machine's local player, if that player owns TeamId. */
	void TryOpenStoreUI(AUnitBase* Unit);

	/** Closes the store widget on THIS machine's local player. */
	void TryCloseStoreUI(AUnitBase* Unit);
};
