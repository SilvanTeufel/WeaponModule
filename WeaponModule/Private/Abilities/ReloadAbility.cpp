// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Abilities/ReloadAbility.h"
#include "Abilities/WeaponAttributeSet.h"
#include "Components/WeaponComponent.h"
#include "AbilitySystemComponent.h"

UReloadAbility::UReloadAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UReloadAbility::PerformReload()
{
	UWeaponAttributeSet* WeaponAttributes = GetWeaponAttributeSet();

	if (WeaponAttributes && WeaponAttributes->GetAmountMagazines() > 0 && WeaponAttributes->GetAmmo() < WeaponAttributes->GetMaxAmmo())
	{
		UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ReloadAbility: Starting reload. Magazines left: %f"), WeaponAttributes->GetAmountMagazines());
		if (ReloadEffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ReloadEffectClass);
			if (SpecHandle.IsValid())
			{
				UWeaponComponent* WeaponComp = GetWeaponComponent();
				if (WeaponComp)
				{
					float Duration = WeaponComp->GetCurrentWeaponData().ReloadTime;
					if (WeaponAttributes->GetReloadSpeedMultiplier() > 0.0f)
					{
						Duration *= WeaponAttributes->GetReloadSpeedMultiplier();
					}

					if (SpecHandle.Data->Def && SpecHandle.Data->Def->DurationPolicy != EGameplayEffectDurationType::Instant)
					{
						SpecHandle.Data->SetDuration(Duration, true);
					}
				}

				ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
			}
		}
		else
		{
			// Fallback: Manually update attributes if no effect class provided
			UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ReloadAbility: No ReloadEffectClass, performing manual attribute update."));
			WeaponAttributes->SetAttributeAmmo(WeaponAttributes->GetMaxAmmo());
			WeaponAttributes->SetAttributeAmountMagazines(WeaponAttributes->GetAmountMagazines() - 1);
		}
	}
	else
	{
		if (!WeaponAttributes)
		{
			UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ReloadAbility: WeaponAttributeSet missing!"));
		}
		else if (WeaponAttributes->GetAmountMagazines() <= 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ReloadAbility: No magazines left!"));
		}
		else if (WeaponAttributes->GetAmmo() >= WeaponAttributes->GetMaxAmmo())
		{
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ReloadAbility: Ammo already full!"));
		}
	}
}
