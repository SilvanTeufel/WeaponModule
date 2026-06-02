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

bool UShootAbility::GetShootInfo(TSubclassOf<class AProjectile>& OutProjectileClass, FWeaponData& OutWeaponData, float& OutExtraDamage, int32& OutMaxPiercedTargets, TSubclassOf<class UGameplayEffect>& OutSelectedEffect, TSubclassOf<class UGameplayEffect>& OutSelectedEffect2, TSubclassOf<class UGameplayEffect>& OutSelectedEffect3)
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ShootAbility: WeaponComponent missing!"));
		return false;
	}

	WeaponComp->SaveAttributesToWeapon(WeaponComp->CurrentWeaponIndex);
	OutWeaponData = WeaponComp->GetCurrentWeaponData();
	OutProjectileClass = OutWeaponData.ProjectileClass;
	OutExtraDamage = 0.f;
	OutMaxPiercedTargets = 1;
	OutSelectedEffect = nullptr;
	OutSelectedEffect2 = nullptr;
	OutSelectedEffect3 = nullptr;

	UWeaponAttributeSet* WeaponAttributes = GetWeaponAttributeSet();
	if (WeaponAttributes)
	{
		int32 Index1 = FMath::FloorToInt(WeaponAttributes->GetSelectedEffectIndex());
		int32 Index2 = FMath::FloorToInt(WeaponAttributes->GetSelectedEffectIndex2());
		int32 Index3 = FMath::FloorToInt(WeaponAttributes->GetSelectedEffectIndex3());

		if (Index1 != -1 && OutWeaponData.EffectTalents.IsValidIndex(Index1))
			OutSelectedEffect = OutWeaponData.EffectTalents[Index1];

		if (Index2 != -1 && OutWeaponData.EffectTalents.IsValidIndex(Index2))
			OutSelectedEffect2 = OutWeaponData.EffectTalents[Index2];

		if (Index3 != -1 && OutWeaponData.EffectTalents.IsValidIndex(Index3))
			OutSelectedEffect3 = OutWeaponData.EffectTalents[Index3];
	}

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: GetShootInfo for WeaponTag: %s, Projectile: %s"), 
		*OutWeaponData.WeaponTag.ToString(), OutProjectileClass ? *OutProjectileClass->GetName() : TEXT("NULL"));

	float CalculatedDamage = OutWeaponData.BaseDamage;
	if (WeaponAttributes)
	{
		CalculatedDamage = OutWeaponData.BaseDamage * WeaponAttributes->GetDamageMultiplier();
		OutMaxPiercedTargets = 1 + FMath::FloorToInt(WeaponAttributes->GetPierceExtraCount());
		OutWeaponData.FireRate *= WeaponAttributes->GetFireRateMultiplier();
	}

	if (OutProjectileClass)
	{
		if (const AProjectile* ProjectileCDO = OutProjectileClass->GetDefaultObject<AProjectile>())
		{
			OutExtraDamage = CalculatedDamage - ProjectileCDO->Damage;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: Calculated - FinalDamage: %.2f (Extra: %.2f), Pierce: %d"), 
		CalculatedDamage, OutExtraDamage, OutMaxPiercedTargets);

	return true;
}

bool UShootAbility::GetEffectAreaInfo(int32 Index, FEffectAreaInfo& OutAreaInfo)
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	if (!WeaponComp) return false;

	if (WeaponComp->EffectAreas.IsValidIndex(Index))
	{
		FEffectAreaData& AreaData = WeaponComp->EffectAreas[Index];
		OutAreaInfo.Radius = AreaData.BaseRadius + (AreaData.RadiusInvestments * 50.f);
		OutAreaInfo.Damage = AreaData.BaseDamage + (AreaData.DamageInvestments * 5.f);

		if (AreaData.SelectedTalentIndices.IsValidIndex(0))
		{
			int32 TIndex = AreaData.SelectedTalentIndices[0];
			if (AreaData.PossibleEffects.IsValidIndex(TIndex))
				OutAreaInfo.Effect1 = AreaData.PossibleEffects[TIndex];
		}

		if (AreaData.SelectedTalentIndices.IsValidIndex(1))
		{
			int32 TIndex = AreaData.SelectedTalentIndices[1];
			if (AreaData.PossibleEffects.IsValidIndex(TIndex))
				OutAreaInfo.Effect2 = AreaData.PossibleEffects[TIndex];
		}

		if (AreaData.SelectedTalentIndices.IsValidIndex(2))
		{
			int32 TIndex = AreaData.SelectedTalentIndices[2];
			if (AreaData.PossibleEffects.IsValidIndex(TIndex))
				OutAreaInfo.Effect3 = AreaData.PossibleEffects[TIndex];
		}
		return true;
	}
	return false;
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

		if (UWeaponComponent* WeaponComp = GetWeaponComponent())
		{
			WeaponComp->SaveAttributesToWeapon(WeaponComp->CurrentWeaponIndex);
		}
	}
}

int32 UShootAbility::GetProjectileCount() const
{
	UWeaponAttributeSet* WeaponAttributes = GetWeaponAttributeSet();
	if (WeaponAttributes)
	{
		return 1 + FMath::FloorToInt(WeaponAttributes->GetProjectileExtraCount());
	}
	return 1;
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

				UWeaponAttributeSet* WeaponAttributes = GetWeaponAttributeSet();
				if (WeaponAttributes)
				{
					Duration *= WeaponAttributes->GetCooldownMultiplier();
				}
				
				if (SpecHandle.Data->Def && SpecHandle.Data->Def->DurationPolicy != EGameplayEffectDurationType::Instant)
				{
					SpecHandle.Data->SetDuration(Duration, true);
				}
				
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

		UWeaponAttributeSet* WeaponAttributes = GetWeaponAttributeSet();
		if (WeaponAttributes)
		{
			Duration *= WeaponAttributes->GetCooldownMultiplier();
		}
		
		UE_LOG(LogTemp, Verbose, TEXT("[WeaponModule] ShootAbility: Querying Cooldown. Duration: %.2fs"), Duration);
	}
}
