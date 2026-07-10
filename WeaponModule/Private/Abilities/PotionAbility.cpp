// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Abilities/PotionAbility.h"
#include "Components/WeaponComponent.h"
#include "Characters/Unit/UnitBase.h"
#include "GAS/AttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UPotionAbility::UPotionAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// A potion should not root the unit like a cast ability.
	bStopMovementOnActivation = false;
}

bool UPotionAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (bConsumePotionCharge)
	{
		// Resolve from the passed ActorInfo (valid even for a CDO-time check) rather than the instance avatar.
		UWeaponComponent* WC = nullptr;
		if (ActorInfo && ActorInfo->AvatarActor.IsValid())
		{
			WC = ActorInfo->AvatarActor->FindComponentByClass<UWeaponComponent>();
		}
		if (!WC)
		{
			return false;
		}
		const int32 Count = (PotionKind == EPotionKind::Health) ? WC->GetHealPotions() : WC->GetManaPotions();
		if (Count <= 0)
		{
			return false;
		}
	}
	return true;
}

void UPotionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bAuthority = ActorInfo && ActorInfo->IsNetAuthority();

	// Apply FIRST on the authority so a no-op (no charge / no restore possible) does not spend the mana
	// cost or trigger the cooldown - cancel instead so nothing is wasted.
	if (bAuthority)
	{
		if (!ConsumeAndApply())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, /*bWasCancelled=*/true);
			return;
		}
	}

	// Commit cost (base ManaCost via ApplyCost) + cooldown (CooldownGameplayEffectClass, if set).
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (bAuthority)
	{
		OnPotionApplied(); // authority-side hook (server logic / listen-server host FX)
		// The ability instance only exists on the authority, so route cross-client cosmetics through the
		// replicated WeaponComponent multicast.
		if (UWeaponComponent* WC = GetWeaponComponent())
		{
			WC->Multicast_PlayPotionFX(PotionKind == EPotionKind::Health);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, /*bWasCancelled=*/false);
}

bool UPotionAbility::ConsumeAndApply()
{
	AUnitBase* Unit = Cast<AUnitBase>(GetAvatarActorFromActorInfo());
	if (!Unit)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UAttributeSetBase* AS = ASC ? ASC->GetSet<UAttributeSetBase>() : nullptr;

	const bool bHealViaGE = (PotionKind == EPotionKind::Health && HealEffectClass != nullptr);

	// Build/validate the heal effect BEFORE consuming a charge so a misconfigured GE never wastes a potion.
	FGameplayEffectSpecHandle SpecHandle;
	if (bHealViaGE)
	{
		SpecHandle = MakeOutgoingGameplayEffectSpec(HealEffectClass);
		if (!SpecHandle.IsValid())
		{
			return false;
		}
	}
	else if (!AS)
	{
		return false; // direct restore needs the attribute set
	}

	if (bConsumePotionCharge)
	{
		UWeaponComponent* WC = GetWeaponComponent();
		if (!WC)
		{
			return false;
		}
		const bool bConsumed = (PotionKind == EPotionKind::Health) ? WC->ConsumeHealPotion() : WC->ConsumeManaPotion();
		if (!bConsumed)
		{
			return false;
		}
	}

	if (PotionKind == EPotionKind::Health)
	{
		if (bHealViaGE)
		{
			ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
		}
		else
		{
			const float NewHealth = (RestoreAmount > 0.f) ? FMath::Min(AS->GetHealth() + RestoreAmount, AS->GetMaxHealth()) : AS->GetMaxHealth();
			Unit->SetHealth(NewHealth);
		}
	}
	else // Mana
	{
		const float NewMana = (RestoreAmount > 0.f) ? FMath::Min(AS->GetMana() + RestoreAmount, AS->GetMaxMana()) : AS->GetMaxMana();
		Unit->SetMana(NewMana);
	}

	return true;
}
