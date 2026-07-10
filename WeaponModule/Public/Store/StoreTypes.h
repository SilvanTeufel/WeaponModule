// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "StoreTypes.generated.h"

/**
 * Every kind of thing a AWeaponStore can sell for Gold. All types are optional -
 * a store only offers the entries a designer adds to its StoreItems list, so the
 * store can be freely composed per-actor.
 */
UENUM(BlueprintType)
enum class EStoreItemType : uint8
{
	// --- Instant restores (unit attributes) ---
	ManaRestore              UMETA(DisplayName = "Restore Mana"),
	HealthRestore            UMETA(DisplayName = "Restore Health (instant)"),
	ShieldRestore            UMETA(DisplayName = "Restore Shield"),

	// --- Weapon consumables / upgrades ---
	Magazines                UMETA(DisplayName = "Magazines (Weapon)"),
	InstantReload            UMETA(DisplayName = "Instant Reload"),
	MaxMagazineUpgrade       UMETA(DisplayName = "Max. Magazines +"),
	MaxAmmoUpgrade           UMETA(DisplayName = "Max. Ammo +"),

	// --- Grenades / area charges ---
	Grenades                 UMETA(DisplayName = "Grenades (EffectArea)"),

	// --- Weapons ---
	Weapon                   UMETA(DisplayName = "Weapon"),

	// --- Potions (inventory, consumed by UPotionAbility) ---
	HealPotion               UMETA(DisplayName = "Health Potion (Inventory)"),
	ManaPotion               UMETA(DisplayName = "Mana Potion (Inventory)"),

	// --- Talent points (feed the existing talent systems) ---
	WeaponTalentPoints       UMETA(DisplayName = "Weapon Talent Points"),
	EffectTalentPoints       UMETA(DisplayName = "Effect Talent Points"),
	EffectAreaTalentPoints   UMETA(DisplayName = "Effect-Area Talent Points"),
	CrowdControlTalentPoints UMETA(DisplayName = "Crowd-Control Talent Points"),
	TalentTreePoints         UMETA(DisplayName = "Talent-Tree Points")
};

/**
 * One purchasable entry in a store. Designer-composed directly on the AWeaponStore actor.
 * Only the fields relevant to ItemType are used; the rest can be left at their defaults.
 */
USTRUCT(BlueprintType)
struct FStoreItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	EStoreItemType ItemType = EStoreItemType::ManaRestore;

	/** Text shown in the store button. If empty, the widget falls back to the ItemType display name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	UTexture2D* Icon = nullptr;

	/** Gold price of one purchase of this entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 GoldCost = 100;

	/**
	 * How much a single purchase grants:
	 *  - Magazines / Grenades / HealPotion / ManaPotion: count added.
	 *  - ManaRestore / HealthRestore: amount restored (<= 0 => refill to max).
	 *  - ShieldRestore: amount (<= 0 => refill to max).
	 *  - MaxMagazineUpgrade / MaxAmmoUpgrade: increment steps (<= 0 => 1).
	 *  - *TalentPoints: points granted.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	float Amount = 1.f;

	/**
	 * Target weapon for Magazines / InstantReload / MaxMagazineUpgrade / MaxAmmoUpgrade.
	 * Empty tag => operate on the unit's currently equipped weapon.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Weapon")
	FGameplayTag WeaponTag;

	/** For ItemType == Weapon: which DataTable row to grant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Weapon")
	FName WeaponRowName = NAME_None;

	/**
	 * Optional weapon DataTable used to resolve WeaponRowName (ItemType == Weapon).
	 * If null, the unit's UWeaponComponent::WeaponDataTable is used.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Weapon")
	UDataTable* WeaponDataTable = nullptr;

	/** For ItemType == Grenades: index into UWeaponComponent::EffectAreas (the "area Id"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store|Grenade")
	int32 EffectAreaIndex = 0;
};
