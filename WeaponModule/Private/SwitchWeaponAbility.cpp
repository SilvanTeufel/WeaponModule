// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "SwitchWeaponAbility.h"
#include "WeaponComponent.h"

USwitchWeaponAbility::USwitchWeaponAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void USwitchWeaponAbility::PerformSwitchWeapon()
{
	UWeaponComponent* WeaponComp = GetWeaponComponent();

	if (WeaponComp && WeaponComp->AvailableWeapons.Num() > 1)
	{
		int32 NextIndex = (WeaponComp->CurrentWeaponIndex + 1) % WeaponComp->AvailableWeapons.Num();
		UE_LOG(LogTemp, Log, TEXT("[WeaponModule] SwitchWeaponAbility: Switching to weapon index %d"), NextIndex);
		WeaponComp->Server_SwitchWeapon(NextIndex);
	}
	else
	{
		if (!WeaponComp)
		{
			UE_LOG(LogTemp, Warning, TEXT("[WeaponModule] SwitchWeaponAbility: WeaponComponent not found!"));
		}
		else if (WeaponComp->AvailableWeapons.Num() <= 1)
		{
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] SwitchWeaponAbility: Not enough weapons to switch (%d)"), WeaponComp->AvailableWeapons.Num());
		}
	}
}
