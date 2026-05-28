// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "ShootAbility.h"
#include "WeaponComponent.h"
#include "WeaponAttributeSet.h"
#include "Characters/Unit/UnitBase.h"
#include "AbilitySystemComponent.h"
#include "MassEntityTypes.h"

UShootAbility::UShootAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UShootAbility::PerformShoot()
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();
	UWeaponAttributeSet* WeaponAttributes = GetWeaponAttributeSet();
	AUnitBase* Unit = Cast<AUnitBase>(CurrentActorInfo->AvatarActor.Get());

	if (WeaponComp && WeaponAttributes && Unit && WeaponAttributes->GetAmmo() > 0)
	{
		FWeaponData WeaponData = WeaponComp->GetCurrentWeaponData();
		UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: Firing weapon %s"), *WeaponData.WeaponTag.ToString());

		// Spawn Projectile via UnitBase logic
		float ProjectileSpeed = 1000.f; 
		if (Unit->Attributes)
		{
			ProjectileSpeed = Unit->Attributes->GetProjectileSpeed();
		}

		Unit->IncrementMassProjectileFireCounter(
			WeaponData.ProjectileClass,
			ProjectileSpeed,
			FMassEntityHandle(), 
			FMassEntityHandle(), 
			0.f, 0.f, 0.f, 0.f, false, FVector::ZeroVector, FVector::OneVector, 0.f,
			WeaponData.BaseDamage
		);

		// Apply Ammo Cost
		if (AmmoCostEffectClass)
		{
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(AmmoCostEffectClass);
			if (SpecHandle.IsValid())
			{
				ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
			}
		}
	}
	else
	{
		if (!WeaponComp) UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ShootAbility: WeaponComponent missing!"));
		if (!WeaponAttributes) UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ShootAbility: WeaponAttributeSet missing!"));
		if (!Unit) UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] ShootAbility: AvatarActor is not AUnitBase!"));
		if (WeaponAttributes && WeaponAttributes->GetAmmo() <= 0) UE_LOG(LogTemp, Log, TEXT("[WeaponModule] ShootAbility: Out of ammo!"));
	}
}
