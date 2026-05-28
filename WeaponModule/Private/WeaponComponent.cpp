// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponComponent.h"
#include "WeaponAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

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
				WeaponAttributes->SetAttributeMaxAmmo(AvailableWeapons[CurrentWeaponIndex].MaxAmmo);
				WeaponAttributes->SetAttributeAmmo(AvailableWeapons[CurrentWeaponIndex].MaxAmmo);
				WeaponAttributes->SetAttributeAmountMagazines(AvailableWeapons[CurrentWeaponIndex].AmountMagazines);
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
	// Hier könnte man Events triggern, um das Visual zu aktualisieren
	// Der MassProcessor wird das aber wahrscheinlich über das Fragment erledigen
}

void UWeaponComponent::Server_SwitchWeapon_Implementation(int32 NewIndex)
{
	if (AvailableWeapons.IsValidIndex(NewIndex))
	{
		CurrentWeaponIndex = NewIndex;
		
		if (WeaponAttributes)
		{
			WeaponAttributes->SetAttributeMaxAmmo(AvailableWeapons[CurrentWeaponIndex].MaxAmmo);
			WeaponAttributes->SetAttributeAmmo(AvailableWeapons[CurrentWeaponIndex].MaxAmmo);
			WeaponAttributes->SetAttributeAmountMagazines(AvailableWeapons[CurrentWeaponIndex].AmountMagazines);
		}
		
		OnRep_CurrentWeaponIndex(); // Manuell aufrufen auf Server, falls nötig
	}
}

FWeaponData UWeaponComponent::GetCurrentWeaponData() const
{
	if (AvailableWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		return AvailableWeapons[CurrentWeaponIndex];
	}
	return FWeaponData();
}
