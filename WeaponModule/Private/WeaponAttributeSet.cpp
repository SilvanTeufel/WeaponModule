// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponAttributeSet.h"
#include "Net/UnrealNetwork.h"

UWeaponAttributeSet::UWeaponAttributeSet()
{
}

void UWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, Ammo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, MaxAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, AmountMagazines, COND_None, REPNOTIFY_Always);
}

void UWeaponAttributeSet::OnRep_Ammo(const FGameplayAttributeData& OldAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, Ammo, OldAmmo);
}

void UWeaponAttributeSet::OnRep_MaxAmmo(const FGameplayAttributeData& OldMaxAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, MaxAmmo, OldMaxAmmo);
}

void UWeaponAttributeSet::OnRep_AmountMagazines(const FGameplayAttributeData& OldAmountMagazines)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, AmountMagazines, OldAmountMagazines);
}

void UWeaponAttributeSet::SetAttributeAmmo(float NewValue)
{
	if (GetAmmoAttribute().GetUProperty())
	{
		InitAmmo(NewValue);
	}
}

void UWeaponAttributeSet::SetAttributeMaxAmmo(float NewValue)
{
	if (GetMaxAmmoAttribute().GetUProperty())
	{
		InitMaxAmmo(NewValue);
	}
}

void UWeaponAttributeSet::SetAttributeAmountMagazines(float NewValue)
{
	if (GetAmountMagazinesAttribute().GetUProperty())
	{
		InitAmountMagazines(NewValue);
	}
}
