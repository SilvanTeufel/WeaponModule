// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/WeaponAbilityBase.h"
#include "PotionAbility.generated.h"

/** What a potion restores. The same ability class serves both (create two BP children). */
UENUM(BlueprintType)
enum class EPotionKind : uint8
{
	Health UMETA(DisplayName = "Health (Heal Potion)"),
	Mana   UMETA(DisplayName = "Mana (Mana Potion)")
};

/**
 * Consumable potion ability. On activation it consumes one potion charge from the unit's
 * UWeaponComponent (HealPotions / ManaPotions, bought at a AWeaponStore) and restores Health or
 * Mana. Health can optionally route through a GameplayEffect so it reuses the project's heal
 * indicator / healthbar / Mass-entity-sync pipeline; otherwise it restores directly.
 *
 * Grant it to a unit like any other ability (SelectableAbilities / DefaultAbilities with an
 * AbilityInputID). Create one BP child with PotionKind = Health and one with PotionKind = Mana.
 */
UCLASS()
class WEAPONMODULE_API UPotionAbility : public UWeaponAbilityBase
{
	GENERATED_BODY()

public:
	UPotionAbility();

	/** Which resource this potion restores. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion")
	EPotionKind PotionKind = EPotionKind::Health;

	/** Amount restored. <= 0 restores fully to the corresponding Max attribute. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion")
	float RestoreAmount = 100.f;

	/** If true, activation requires and consumes one HealPotions/ManaPotions charge on the WeaponComponent. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion")
	bool bConsumePotionCharge = true;

	/**
	 * Optional (Health only): a GameplayEffect applied to the owner instead of a direct heal.
	 * Use one that adds a positive magnitude to the EffectDamage meta attribute to get the green
	 * heal indicator + healthbar + Mass sync. When set, the GE's own magnitude is authoritative and
	 * RestoreAmount is ignored. When null, Health is restored directly by RestoreAmount.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Potion")
	TSubclassOf<class UGameplayEffect> HealEffectClass;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** BP hook fired on the AUTHORITY only (server / listen-host) after a successful apply - use for
	 *  server-side logic. For cross-client VFX/SFX use UWeaponComponent::OnPotionFX (driven by the
	 *  Multicast_PlayPotionFX this ability triggers), since the ability instance is server-only. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Potion")
	void OnPotionApplied();

	/** Server-side: consume a charge (if configured) and restore the resource. Returns false if it couldn't. */
	bool ConsumeAndApply();
};
