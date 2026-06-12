// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.


#include "Actors/WeaponPickup.h"
#include "Characters/Unit/UnitBase.h"

AWeaponPickup::AWeaponPickup()
{
}

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();
	OnPickupImpact.AddDynamic(this, &AWeaponPickup::HandlePickupImpact);
}

void AWeaponPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SyncWeaponDataFromTable();
}

#if WITH_EDITOR
void AWeaponPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AWeaponPickup, WeaponDataTable) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(AWeaponPickup, WeaponRowName))
	{
		SyncWeaponDataFromTable();
	}
}
#endif

void AWeaponPickup::SyncWeaponDataFromTable()
{
	if (WeaponDataTable && !WeaponRowName.IsNone())
	{
		static const FString ContextString(TEXT("WeaponPickup Context"));
		FWeaponData* Row = WeaponDataTable->FindRow<FWeaponData>(WeaponRowName, ContextString);
		if (Row)
		{
			WeaponData = *Row;
		}
	}
}

void AWeaponPickup::HandlePickupImpact(APickup* Pickup, AUnitBase* Unit)
{
	if (!Unit) return;

	SyncWeaponDataFromTable(); // Ensure we have latest data
	
	UWeaponComponent* WeaponComp = Unit->FindComponentByClass<UWeaponComponent>();
	if (!WeaponComp) return;

	if (EffectAreaIndex != -1)
	{
		if (WeaponComp->EffectAreas.IsValidIndex(EffectAreaIndex))
		{
			WeaponComp->EffectAreas[EffectAreaIndex].Amount += AmountToAdd;
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponPickup: Added %.1f to EffectArea %d. New Amount: %.1f"), 
				AmountToAdd, EffectAreaIndex, WeaponComp->EffectAreas[EffectAreaIndex].Amount);
		}
	}
	else if (bIsMagazineOnly)
	{
		if (WeaponComp->AvailableWeapons.IsValidIndex(WeaponComp->CurrentWeaponIndex))
		{
			FWeaponData& CurrentData = WeaponComp->AvailableWeapons[WeaponComp->CurrentWeaponIndex];
			float MaxMags = CurrentData.MaxMagazinesSpec > 0 ? CurrentData.MaxMagazinesSpec : CurrentData.MaxMagazines;
			CurrentData.AmountMagazines = FMath::Min(CurrentData.AmountMagazines + AmountToAdd, MaxMags);
			
			WeaponComp->SyncAttributesFromWeapon(WeaponComp->CurrentWeaponIndex);
			
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponPickup: MagazineOnly - Added %.1f magazines for current weapon. New Amount: %.1f"), 
				AmountToAdd, CurrentData.AmountMagazines);
		}
	}
	else if (WeaponData.WeaponTag != FGameplayTag::EmptyTag)
	{
		bool bHasWeapon = false;
		int32 WeaponIdx = -1;
		for (int32 i = 0; i < WeaponComp->AvailableWeapons.Num(); ++i)
		{
			if (WeaponComp->AvailableWeapons[i].WeaponTag == WeaponData.WeaponTag)
			{
				bHasWeapon = true;
				WeaponIdx = i;
				break;
			}
		}

		if (bHasWeapon)
		{
			// Give magazines
			FWeaponData& FoundData = WeaponComp->AvailableWeapons[WeaponIdx];
			float MaxMags = FoundData.MaxMagazinesSpec > 0 ? FoundData.MaxMagazinesSpec : FoundData.MaxMagazines;
			FoundData.AmountMagazines = FMath::Min(FoundData.AmountMagazines + AmountToAdd, MaxMags);
			
			if (WeaponIdx == WeaponComp->CurrentWeaponIndex)
			{
				WeaponComp->SyncAttributesFromWeapon(WeaponComp->CurrentWeaponIndex);
			}
			
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponPickup: Added %.1f magazines for %s. New Amount: %.1f"), 
				AmountToAdd, *WeaponData.WeaponTag.ToString(), FoundData.AmountMagazines);
		}
		else
		{
			// Add new weapon, set ammo to 0
			FWeaponData NewWeapon = WeaponData;
			NewWeapon.Ammo = 0.0f;
			NewWeapon.AmountMagazines = AmountToAdd;

			int32 NewIdx = WeaponComp->AvailableWeapons.Add(NewWeapon);
			if (WeaponComp->CurrentWeaponIndex == -1)
			{
				WeaponComp->CurrentWeaponIndex = NewIdx;
				WeaponComp->SyncAttributesFromWeapon(NewIdx);
			}
			UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponPickup: Added new weapon: %s with 0 ammo"), *WeaponData.WeaponTag.ToString());
		}
	}
}
