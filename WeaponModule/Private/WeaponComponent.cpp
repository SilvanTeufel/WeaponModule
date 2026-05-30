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

	// Da wir für die Vielzahl an Talenten keine vordefinierten GE-Klassen erzwingen wollen,
	// modifizieren wir die Basiswerte der Attribute permanent direkt über den ASC.
	// Dies ist für permanente Talent-Upgrades im C++ der direkteste Weg.
	float CurrentBase = ASC->GetNumericAttributeBase(Upgrade.Attribute);
	float NewBase = CurrentBase;

	if (Upgrade.ModifierOp == EGameplayModOp::Additive)
	{
		NewBase += Upgrade.ModifierValue;
	}
	else if (Upgrade.ModifierOp == EGameplayModOp::Multiplicitive)
	{
		NewBase *= Upgrade.ModifierValue;
	}
	else if (Upgrade.ModifierOp == EGameplayModOp::Override)
	{
		NewBase = Upgrade.ModifierValue;
	}

	ASC->SetNumericAttributeBase(Upgrade.Attribute, NewBase);

	if (Upgrade.Attribute == UWeaponAttributeSet::GetMaxMagazinesAttribute() && Upgrade.ModifierOp == EGameplayModOp::Additive)
	{
		float CurrentAmount = ASC->GetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesAttribute());
		ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesAttribute(), CurrentAmount + Upgrade.ModifierValue);
	}
	
	// Increment Level Attribute if valid
	if (Upgrade.LevelAttribute.IsValid())
	{
		float CurrentLevel = ASC->GetNumericAttributeBase(Upgrade.LevelAttribute);
		ASC->SetNumericAttributeBase(Upgrade.LevelAttribute, CurrentLevel + 1.0f);
	}

	SaveAttributesToWeapon(CurrentWeaponIndex);

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponComponent: Purchased upgrade %s. New Base: %.2f. Points remaining: %.1f"), 
		*Upgrade.Name.ToString(), NewBase, WeaponAttributes->GetWeaponTalentPoints());
	
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
