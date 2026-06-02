// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UWeaponAttributeSet::UWeaponAttributeSet()
{
	InitDamageMultiplier(1.0f);
	InitCooldownMultiplier(1.0f);
	InitFireRateMultiplier(1.0f);
	InitReloadSpeedMultiplier(1.0f);
	InitPierceExtraCount(0.0f);
	InitProjectileExtraCount(0.0f);
	InitWeaponTalentPoints(0.0f);
	InitDamageTalentLevel(0.0f);
	InitCooldownTalentLevel(0.0f);
	InitFireRateTalentLevel(0.0f);
	InitReloadSpeedTalentLevel(0.0f);
	InitPierceTalentLevel(0.0f);
	InitProjectileTalentLevel(0.0f);
	InitMaxAmmoTalentLevel(0.0f);
	InitAmountMagazinesTalentLevel(0.0f);
	InitEffectTalentPoints(0.0f);
	InitSelectedEffectIndex(-1.0f);
	InitSelectedEffectIndex2(-1.0f);
	InitSelectedEffectIndex3(-1.0f);
	InitEffectAreaTalentPoints(0.0f);
	InitSelectedEffectAreaIndex(-1.0f);
}

void UWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, Ammo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, MaxAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, AmountMagazines, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, MaxMagazines, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, WeaponTalentPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, DamageMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, CooldownMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, FireRateMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, PierceExtraCount, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, ProjectileExtraCount, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, ReloadSpeedMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, DamageTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, CooldownTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, FireRateTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, ReloadSpeedTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, PierceTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, ProjectileTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, MaxAmmoTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, AmountMagazinesTalentLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, EffectTalentPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, SelectedEffectIndex, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, SelectedEffectIndex2, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, SelectedEffectIndex3, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, EffectAreaTalentPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWeaponAttributeSet, SelectedEffectAreaIndex, COND_None, REPNOTIFY_Always);
}

void UWeaponAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAmmo());
	}
	else if (Attribute == GetAmountMagazinesAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMagazines());
	}
}

void UWeaponAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetAmmoAttribute())
	{
		SetAmmo(FMath::Clamp(GetAmmo(), 0.0f, GetMaxAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetAmountMagazinesAttribute())
	{
		SetAmountMagazines(FMath::Clamp(GetAmountMagazines(), 0.0f, GetMaxMagazines()));
	}
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

void UWeaponAttributeSet::OnRep_MaxMagazines(const FGameplayAttributeData& OldMaxMagazines)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, MaxMagazines, OldMaxMagazines);
}

void UWeaponAttributeSet::OnRep_WeaponTalentPoints(const FGameplayAttributeData& OldWeaponTalentPoints)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, WeaponTalentPoints, OldWeaponTalentPoints);
}

void UWeaponAttributeSet::OnRep_DamageMultiplier(const FGameplayAttributeData& OldDamageMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, DamageMultiplier, OldDamageMultiplier);
}

void UWeaponAttributeSet::OnRep_CooldownMultiplier(const FGameplayAttributeData& OldCooldownMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, CooldownMultiplier, OldCooldownMultiplier);
}

void UWeaponAttributeSet::OnRep_FireRateMultiplier(const FGameplayAttributeData& OldFireRateMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, FireRateMultiplier, OldFireRateMultiplier);
}

void UWeaponAttributeSet::OnRep_PierceExtraCount(const FGameplayAttributeData& OldPierceExtraCount)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, PierceExtraCount, OldPierceExtraCount);
}

void UWeaponAttributeSet::OnRep_ProjectileExtraCount(const FGameplayAttributeData& OldProjectileExtraCount)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, ProjectileExtraCount, OldProjectileExtraCount);
}

void UWeaponAttributeSet::OnRep_ReloadSpeedMultiplier(const FGameplayAttributeData& OldReloadSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, ReloadSpeedMultiplier, OldReloadSpeedMultiplier);
}

void UWeaponAttributeSet::OnRep_DamageTalentLevel(const FGameplayAttributeData& OldDamageTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, DamageTalentLevel, OldDamageTalentLevel);
}

void UWeaponAttributeSet::OnRep_CooldownTalentLevel(const FGameplayAttributeData& OldCooldownTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, CooldownTalentLevel, OldCooldownTalentLevel);
}

void UWeaponAttributeSet::OnRep_FireRateTalentLevel(const FGameplayAttributeData& OldFireRateTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, FireRateTalentLevel, OldFireRateTalentLevel);
}

void UWeaponAttributeSet::OnRep_ReloadSpeedTalentLevel(const FGameplayAttributeData& OldReloadSpeedTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, ReloadSpeedTalentLevel, OldReloadSpeedTalentLevel);
}

void UWeaponAttributeSet::OnRep_PierceTalentLevel(const FGameplayAttributeData& OldPierceTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, PierceTalentLevel, OldPierceTalentLevel);
}

void UWeaponAttributeSet::OnRep_ProjectileTalentLevel(const FGameplayAttributeData& OldProjectileTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, ProjectileTalentLevel, OldProjectileTalentLevel);
}

void UWeaponAttributeSet::OnRep_MaxAmmoTalentLevel(const FGameplayAttributeData& OldMaxAmmoTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, MaxAmmoTalentLevel, OldMaxAmmoTalentLevel);
}

void UWeaponAttributeSet::OnRep_AmountMagazinesTalentLevel(const FGameplayAttributeData& OldAmountMagazinesTalentLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, AmountMagazinesTalentLevel, OldAmountMagazinesTalentLevel);
}

void UWeaponAttributeSet::OnRep_EffectTalentPoints(const FGameplayAttributeData& OldEffectTalentPoints)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, EffectTalentPoints, OldEffectTalentPoints);
}

void UWeaponAttributeSet::OnRep_SelectedEffectIndex(const FGameplayAttributeData& OldSelectedEffectIndex)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, SelectedEffectIndex, OldSelectedEffectIndex);
}

void UWeaponAttributeSet::OnRep_SelectedEffectIndex2(const FGameplayAttributeData& OldSelectedEffectIndex2)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, SelectedEffectIndex2, OldSelectedEffectIndex2);
}

void UWeaponAttributeSet::OnRep_SelectedEffectIndex3(const FGameplayAttributeData& OldSelectedEffectIndex3)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, SelectedEffectIndex3, OldSelectedEffectIndex3);
}

void UWeaponAttributeSet::SetAttributeAmmo(float NewValue)
{
	SetAmmo(NewValue);
}

void UWeaponAttributeSet::SetAttributeMaxAmmo(float NewValue)
{
	SetMaxAmmo(NewValue);
}

void UWeaponAttributeSet::SetAttributeAmountMagazines(float NewValue)
{
	SetAmountMagazines(NewValue);
}

void UWeaponAttributeSet::SetAttributeMaxMagazines(float NewValue)
{
	SetMaxMagazines(NewValue);
}

void UWeaponAttributeSet::SetAttributeWeaponTalentPoints(float NewValue)
{
	SetWeaponTalentPoints(NewValue);
}

void UWeaponAttributeSet::SetAttributeEffectTalentPoints(float NewValue)
{
	SetEffectTalentPoints(NewValue);
}

void UWeaponAttributeSet::SetAttributeSelectedEffectIndex(float NewValue)
{
	SetSelectedEffectIndex(NewValue);
}

void UWeaponAttributeSet::SetAttributeSelectedEffectIndex2(float NewValue)
{
	SetSelectedEffectIndex2(NewValue);
}

void UWeaponAttributeSet::SetAttributeSelectedEffectIndex3(float NewValue)
{
	SetSelectedEffectIndex3(NewValue);
}

void UWeaponAttributeSet::OnRep_EffectAreaTalentPoints(const FGameplayAttributeData& OldEffectAreaTalentPoints)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, EffectAreaTalentPoints, OldEffectAreaTalentPoints);
}

void UWeaponAttributeSet::SetAttributeEffectAreaTalentPoints(float NewValue)
{
	SetEffectAreaTalentPoints(NewValue);
}

void UWeaponAttributeSet::OnRep_SelectedEffectAreaIndex(const FGameplayAttributeData& OldSelectedGranadeIndex)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWeaponAttributeSet, SelectedEffectAreaIndex, OldSelectedGranadeIndex);
}

void UWeaponAttributeSet::SetAttributeSelectedEffectAreaIndex(float NewValue)
{
	SetSelectedEffectAreaIndex(NewValue);
}
