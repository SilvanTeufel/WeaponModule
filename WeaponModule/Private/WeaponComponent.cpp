// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponComponent.h"
#include "WeaponAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

#if WITH_EDITORONLY_DATA
	WeaponPreview = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponPreview"));
	WeaponPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponPreview->bIsEditorOnly = true;
	WeaponPreview->SetHiddenInGame(true);
#endif
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		for (FWeaponData& Weapon : AvailableWeapons)
		{
			Weapon.WeaponTalentPoints += StartTalentPoints;
		}
	}

	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
		if (ASC)
		{
			WeaponAttributes = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>());
			if (!WeaponAttributes)
			{
				WeaponAttributes = NewObject<UWeaponAttributeSet>(ASC->GetOwnerActor(), UWeaponAttributeSet::StaticClass());
				ASC->AddAttributeSetSubobject(WeaponAttributes);
			}

			if (WeaponAttributes && AvailableWeapons.IsValidIndex(CurrentWeaponIndex))
			{
				SyncAttributesFromWeapon(CurrentWeaponIndex);
			}
		}
	}
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponComponent, CurrentWeaponIndex);
}

void UWeaponComponent::OnRep_CurrentWeaponIndex()
{
	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

void UWeaponComponent::Server_SwitchWeapon_Implementation(int32 NewIndex)
{
	if (AvailableWeapons.IsValidIndex(NewIndex))
	{
		SaveAttributesToWeapon(CurrentWeaponIndex);
		CurrentWeaponIndex = NewIndex;
		SyncAttributesFromWeapon(CurrentWeaponIndex);
		OnRep_CurrentWeaponIndex();
	}
}

void UWeaponComponent::SyncAttributesFromWeapon(int32 Index)
{
	if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(Index)) return;
	FWeaponData& Data = AvailableWeapons[Index];

	WeaponAttributes->SetAttributeMaxAmmo(Data.MaxAmmo);
	WeaponAttributes->SetAttributeAmmo(Data.Ammo > 0 ? Data.Ammo : Data.MaxAmmo);
	WeaponAttributes->SetAttributeMaxMagazines(Data.MaxMagazinesSpec > 0 ? Data.MaxMagazinesSpec : Data.MaxMagazines);
	WeaponAttributes->SetAttributeAmountMagazines(Data.AmountMagazines);
	
	WeaponAttributes->SetAttributeWeaponTalentPoints(Data.WeaponTalentPoints);
	WeaponAttributes->SetDamageMultiplier(Data.DamageMultiplier);
	WeaponAttributes->SetCooldownMultiplier(Data.CooldownMultiplier);
	WeaponAttributes->SetReloadSpeedMultiplier(Data.ReloadSpeedMultiplier);
	WeaponAttributes->SetPierceExtraCount(Data.PierceExtraCount);
	WeaponAttributes->SetProjectileExtraCount(Data.ProjectileExtraCount);

	WeaponAttributes->SetDamageTalentLevel(Data.DamageTalentLevel);
	WeaponAttributes->SetCooldownTalentLevel(Data.CooldownTalentLevel);
	WeaponAttributes->SetReloadSpeedTalentLevel(Data.ReloadSpeedTalentLevel);
	WeaponAttributes->SetPierceTalentLevel(Data.PierceTalentLevel);
	WeaponAttributes->SetProjectileTalentLevel(Data.ProjectileTalentLevel);
	WeaponAttributes->SetMaxAmmoTalentLevel(Data.MaxAmmoTalentLevel);
	WeaponAttributes->SetAmountMagazinesTalentLevel(Data.AmountMagazinesTalentLevel);
}

void UWeaponComponent::SaveAttributesToWeapon(int32 Index)
{
	if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(Index)) return;
	FWeaponData& Data = AvailableWeapons[Index];

	Data.Ammo = WeaponAttributes->GetAmmo();
	Data.AmountMagazines = WeaponAttributes->GetAmountMagazines();
	Data.MaxMagazinesSpec = WeaponAttributes->GetMaxMagazines();
	Data.WeaponTalentPoints = WeaponAttributes->GetWeaponTalentPoints();
	Data.DamageMultiplier = WeaponAttributes->GetDamageMultiplier();
	Data.CooldownMultiplier = WeaponAttributes->GetCooldownMultiplier();
	Data.ReloadSpeedMultiplier = WeaponAttributes->GetReloadSpeedMultiplier();
	Data.PierceExtraCount = WeaponAttributes->GetPierceExtraCount();
	Data.ProjectileExtraCount = WeaponAttributes->GetProjectileExtraCount();

	Data.DamageTalentLevel = WeaponAttributes->GetDamageTalentLevel();
	Data.CooldownTalentLevel = WeaponAttributes->GetCooldownTalentLevel();
	Data.ReloadSpeedTalentLevel = WeaponAttributes->GetReloadSpeedTalentLevel();
	Data.PierceTalentLevel = WeaponAttributes->GetPierceTalentLevel();
	Data.ProjectileTalentLevel = WeaponAttributes->GetProjectileTalentLevel();
	Data.MaxAmmoTalentLevel = WeaponAttributes->GetMaxAmmoTalentLevel();
	Data.AmountMagazinesTalentLevel = WeaponAttributes->GetAmountMagazinesTalentLevel();
}

FWeaponData UWeaponComponent::GetCurrentWeaponData() const
{
	if (AvailableWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		return AvailableWeapons[CurrentWeaponIndex];
	}
	return FWeaponData();
}

bool UWeaponComponent::PurchaseUpgrade(FWeaponUpgrade Upgrade)
{
	if (!WeaponAttributes || WeaponAttributes->GetWeaponTalentPoints() < Upgrade.Cost)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return false;
	}

	// Punkte abziehen
	WeaponAttributes->SetAttributeWeaponTalentPoints(WeaponAttributes->GetWeaponTalentPoints() - Upgrade.Cost);

    // Increment Level Attribute (Raw points invested)
    float NewLevel = 0.0f;
	if (Upgrade.LevelAttribute.IsValid())
	{
		NewLevel = ASC->GetNumericAttributeBase(Upgrade.LevelAttribute) + 1.0f;
		ASC->SetNumericAttributeBase(Upgrade.LevelAttribute, NewLevel);
	}
    else
    {
        // Fallback if no level attribute, though there should be one
        return false; 
    }

    // Calculate Effective Tier based on exponential cost (doubling points for next tier)
    // 1 point -> Tier 1
    // 3 points -> Tier 2
    // 7 points -> Tier 3
    // Formula: Tier = floor(log2(Points + 1))
    float EffectiveTier = FMath::FloorToFloat(FMath::Log2(NewLevel + 1.0f));

	// Calculate New Attribute Value
	float NewValue = 0.0f;

    if (Upgrade.Attribute == UWeaponAttributeSet::GetProjectileExtraCountAttribute())
    {
        // Special logic for Projectiles: 1->1, 3->2, 7->4
        // Formula: Benefit = 2^(Tier - 1)
        NewValue = (EffectiveTier > 0) ? FMath::Pow(2.0f, EffectiveTier - 1.0f) : 0.0f;
    }
    else if (Upgrade.ModifierOp == EGameplayModOp::Additive)
	{
        // For others, we scale the modifier by the Tier
        // Damage, Pierce, MaxAmmo, MaxMagazines
        float BaseVal = 0.0f;
        if (Upgrade.Attribute == UWeaponAttributeSet::GetDamageMultiplierAttribute()) BaseVal = 1.0f;
        else if (Upgrade.Attribute == UWeaponAttributeSet::GetMaxAmmoAttribute()) {
             // For MaxAmmo, we use the weapon's base MaxAmmo as starting point
             if (AvailableWeapons.IsValidIndex(CurrentWeaponIndex)) BaseVal = AvailableWeapons[CurrentWeaponIndex].MaxAmmo;
        }
        else if (Upgrade.Attribute == UWeaponAttributeSet::GetMaxMagazinesAttribute()) {
             if (AvailableWeapons.IsValidIndex(CurrentWeaponIndex)) BaseVal = AvailableWeapons[CurrentWeaponIndex].MaxMagazines;
        }
        
		NewValue = BaseVal + (EffectiveTier * Upgrade.ModifierValue);
	}
	else if (Upgrade.ModifierOp == EGameplayModOp::Multiplicitive)
	{
        // Cooldown, Reload Speed
		NewValue = FMath::Pow(Upgrade.ModifierValue, EffectiveTier);
	}
	else if (Upgrade.ModifierOp == EGameplayModOp::Override)
	{
		NewValue = Upgrade.ModifierValue;
	}

	ASC->SetNumericAttributeBase(Upgrade.Attribute, NewValue);

	if (Upgrade.Attribute == UWeaponAttributeSet::GetMaxMagazinesAttribute())
	{
		float CurrentAmount = ASC->GetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesAttribute());
        // Since MaxMagazines changed, we should probably adjust current amount if it was added.
        // The user previously wanted: CurrentAmount + Upgrade.ModifierValue
        // But with tiers, it's more complex. Let's just keep it simple: if Tier increased, add a magazine.
        float OldTier = FMath::FloorToFloat(FMath::Log2(NewLevel)); // Tier before this point
        if (EffectiveTier > OldTier)
        {
            ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesAttribute(), CurrentAmount + 1.0f);
        }
	}
	
	SaveAttributesToWeapon(CurrentWeaponIndex);

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponComponent: Purchased upgrade %s. New Level: %.0f, Tier: %.0f, New Value: %.2f"), 
		*Upgrade.Name.ToString(), NewLevel, EffectiveTier, NewValue);
	
	return true;
}

void UWeaponComponent::Server_ResetCurrentWeaponTalents_Implementation()
{
	if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(CurrentWeaponIndex)) return;

	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return;

	float InvestedPoints = 0.0f;
	InvestedPoints += WeaponAttributes->GetDamageTalentLevel();
	InvestedPoints += WeaponAttributes->GetCooldownTalentLevel();
	InvestedPoints += WeaponAttributes->GetReloadSpeedTalentLevel();
	InvestedPoints += WeaponAttributes->GetPierceTalentLevel();
	InvestedPoints += WeaponAttributes->GetProjectileTalentLevel();
	InvestedPoints += WeaponAttributes->GetMaxAmmoTalentLevel();
	InvestedPoints += WeaponAttributes->GetAmountMagazinesTalentLevel();

	// Reset Multipliers / Extra Counts to default values
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetDamageMultiplierAttribute(), 1.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetCooldownMultiplierAttribute(), 1.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetReloadSpeedMultiplierAttribute(), 1.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetPierceExtraCountAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetProjectileExtraCountAttribute(), 0.0f);

	// Reset Ammo attributes to weapon base values
	FWeaponData& Data = AvailableWeapons[CurrentWeaponIndex];
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetMaxAmmoAttribute(), Data.MaxAmmo);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesAttribute(), Data.AmountMagazines);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetMaxMagazinesAttribute(), Data.MaxMagazines);

	// Reset Levels to zero
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetDamageTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetCooldownTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetReloadSpeedTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetPierceTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetProjectileTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetMaxAmmoTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesTalentLevelAttribute(), 0.0f);

	// Refund points to the weapon's talent pool
	float CurrentPoints = WeaponAttributes->GetWeaponTalentPoints();
	WeaponAttributes->SetAttributeWeaponTalentPoints(CurrentPoints + InvestedPoints);

	SaveAttributesToWeapon(CurrentWeaponIndex);

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponComponent: Reset talents for current weapon. Refunded %.1f points."), InvestedPoints);
}

#if WITH_EDITOR
void UWeaponComponent::OnRegister()
{
	Super::OnRegister();
	UpdatePreview();
}

void UWeaponComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty)
	{
		const FName MemberName = PropertyChangedEvent.MemberProperty->GetFName();
		
		// Falls die Komponente im Viewport manuell verschoben wurde:
		if (MemberName == GET_MEMBER_NAME_CHECKED(UWeaponComponent, WeaponPreview))
		{
			if (AvailableWeapons.IsValidIndex(PreviewWeaponIndex))
			{
				AvailableWeapons[PreviewWeaponIndex].Offset = WeaponPreview->GetRelativeTransform();
			}
		}

		// Falls Einstellungen im Detail-Panel geändert wurden:
		if (MemberName == GET_MEMBER_NAME_CHECKED(UWeaponComponent, PreviewWeaponIndex) || 
			MemberName == GET_MEMBER_NAME_CHECKED(UWeaponComponent, AvailableWeapons))
		{
			UpdatePreview();
		}
	}
}

void UWeaponComponent::UpdatePreview()
{
	if (!WeaponPreview || !AvailableWeapons.IsValidIndex(PreviewWeaponIndex))
	{
		if (WeaponPreview)
		{
			WeaponPreview->SetStaticMesh(nullptr);
		}
		return;
	}

	const FWeaponData& Data = AvailableWeapons[PreviewWeaponIndex];
	WeaponPreview->SetStaticMesh(Data.WeaponMesh);

	if (AActor* Owner = GetOwner())
	{
		// Suche Mesh der Unit für den Socket-Attach
		USkeletalMeshComponent* UnitMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		if (UnitMesh && Data.SocketName != NAME_None)
		{
			WeaponPreview->AttachToComponent(UnitMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, Data.SocketName);
		}
		else
		{
			WeaponPreview->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		}
	}

	WeaponPreview->SetRelativeTransform(Data.Offset);
}
#endif
