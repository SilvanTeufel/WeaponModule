// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "ShootAbility.h"
#include "WeaponAttributeSet.h"
#include "Characters/Unit/UnitBase.h"
#include "AbilitySystemComponent.h"
#include "MassEntityTypes.h"
#include "Actors/Projectile.h"

UShootAbility::UShootAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UShootAbility::GetShootInfo(TSubclassOf<class AProjectile>& OutProjectileClass, FWeaponData& OutWeaponData)
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ShootAbility: WeaponComponent missing!"));
		return false;
	}

	OutWeaponData = WeaponComp->GetCurrentWeaponData();
	OutProjectileClass = OutWeaponData.ProjectileClass;

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: GetShootInfo for WeaponTag: %s, Projectile: %s"), 
		*OutWeaponData.WeaponTag.ToString(), OutProjectileClass ? *OutProjectileClass->GetName() : TEXT("NULL"));

	if (OutProjectileClass)
	{
		if (AProjectile* ProjectileCDO = OutProjectileClass->GetDefaultObject<AProjectile>())
		{
			ProjectileCDO->Damage = OutWeaponData.BaseDamage;
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: Overwriting Projectile Damage with BaseDamage: %.2f"), OutWeaponData.BaseDamage);
		}
	}

	return true;
}

void UShootAbility::ApplyGlobalCooldown()
{
	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: ApplyGlobalCooldown called manually."));
	ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
}

void UShootAbility::ModifyAmmo(float Amount)
{
	UWeaponAttributeSet* WeaponAttributes = GetWeaponAttributeSet();
	if (WeaponAttributes)
	{
		float NewAmmo = WeaponAttributes->GetAmmo() + Amount;
		WeaponAttributes->SetAttributeAmmo(NewAmmo);
		UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: Modified Ammo by %.1f. New Ammo: %.1f"), Amount, NewAmmo);
	}
}

UGameplayEffect* UShootAbility::GetCooldownGameplayEffect() const
{
	return CooldownGameplayEffectClass ? CooldownGameplayEffectClass->GetDefaultObject<UGameplayEffect>() : nullptr;
}

void UShootAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel());
		
		if (SpecHandle.IsValid())
		{
			UWeaponComponent* WeaponComp = GetWeaponComponent();
			if (WeaponComp)
			{
				float Duration = WeaponComp->GetCurrentWeaponData().CooldownTime;
				SpecHandle.Data->SetDuration(Duration, true);
				
				UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: Applying Cooldown. Weapon: %s, Duration: %.2fs"), 
					*WeaponComp->GetCurrentWeaponData().WeaponTag.ToString(), Duration);

				FActiveGameplayEffectHandle AppliedHandle = ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				
				if (AppliedHandle.IsValid())
				{
					// Optional: Log the actual duration from the active effect to verify
					const FActiveGameplayEffect* ActiveGE = ActorInfo->AbilitySystemComponent->GetActiveGameplayEffect(AppliedHandle);
					if (ActiveGE)
					{
						UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: Successfully applied Active GE with Duration: %.2fs"), ActiveGE->GetDuration());
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ShootAbility: ApplyCooldown - WeaponComponent missing!"));
				ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ShootAbility: ApplyCooldown - No Cooldown GE or ASC found."));
	}
}

void UShootAbility::GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& Duration) const
{
	Super::GetCooldownTimeRemainingAndDuration(Handle, ActorInfo, TimeRemaining, Duration);
	
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (WeaponComp)
	{
		Duration = WeaponComp->GetCurrentWeaponData().CooldownTime;
		UE_LOG(LogTemp, Verbose, TEXT("[WeaponModule] ShootAbility: Querying Cooldown. Duration: %.2fs"), Duration);
	}
}
