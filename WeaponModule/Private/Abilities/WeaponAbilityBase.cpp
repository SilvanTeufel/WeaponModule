// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Abilities/WeaponAbilityBase.h"
#include "Components/WeaponComponent.h"
#include "Abilities/WeaponAttributeSet.h"
#include "AbilitySystemComponent.h"

UWeaponAbilityBase::UWeaponAbilityBase()
{
}

UWeaponComponent* UWeaponAbilityBase::GetWeaponComponent() const
{
	if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
	{
		return AvatarActor->FindComponentByClass<UWeaponComponent>();
	}
	return nullptr;
}

UWeaponAttributeSet* UWeaponAbilityBase::GetWeaponAttributeSet() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		return const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>());
	}
	return nullptr;
}
