// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponComponent.h"
#include "WeaponAttributeSet.h"
#include "Characters/Unit/LevelUnit.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "System/GameSaveSubsystem.h"
#include "JsonObjectConverter.h"

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
		LoadDataFromTables();
		for (int32 i = 0; i < AvailableWeapons.Num(); ++i)
		{
			if (AvailableWeapons[i].Ammo == 0)
			{
				AvailableWeapons[i].Ammo = AvailableWeapons[i].MaxAmmoSpec > 0 ? AvailableWeapons[i].MaxAmmoSpec : AvailableWeapons[i].MaxAmmo;
			}
			AvailableWeapons[i].WeaponTalentPoints += StartTalentPoints;
			AvailableWeapons[i].EffectTalentPoints += StartEffectTalentPoints;
		}
		EffectAreaTalentPoints += StartEffectAreaTalentPoints;
	}

	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
		if (ASC)
		{
			WeaponAttributes = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>());
			
			if (GetOwner()->HasAuthority())
			{
				if (!WeaponAttributes)
				{
					WeaponAttributes = NewObject<UWeaponAttributeSet>(ASC->GetOwnerActor(), UWeaponAttributeSet::StaticClass());
					ASC->AddAttributeSetSubobject(WeaponAttributes);
				}
			}

			// On Server this initializes the AttributeSet.
			// On Client this tries to find the AttributeSet and sync if it already replicated.
			if (AvailableWeapons.IsValidIndex(CurrentWeaponIndex))
			{
				SyncAttributesFromWeapon(CurrentWeaponIndex);
			}
		}
	}

	if (UGameInstance* GI = GetOwner()->GetGameInstance())
	{
		if (UGameSaveSubsystem* SaveSub = GI->GetSubsystem<UGameSaveSubsystem>())
		{
			SaveSub->OnUnitSave.AddUObject(this, &UWeaponComponent::OnUnitSave);
			SaveSub->OnUnitLoad.AddUObject(this, &UWeaponComponent::OnUnitLoad);
		}
	}
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UWeaponComponent, CurrentWeaponIndex);
	DOREPLIFETIME(UWeaponComponent, AvailableWeapons);
	DOREPLIFETIME(UWeaponComponent, EffectAreas);
	DOREPLIFETIME(UWeaponComponent, EffectAreaTalentPoints);
}

void UWeaponComponent::OnRep_CurrentWeaponIndex()
{
	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

void UWeaponComponent::OnRep_AvailableWeapons()
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
	if (!WeaponAttributes)
	{
		if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
			if (ASC)
			{
				WeaponAttributes = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>());
			}
		}
	}
	
	if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(Index)) return;
	FWeaponData& Data = AvailableWeapons[Index];

	WeaponAttributes->SetAttributeMaxAmmo(Data.MaxAmmoSpec > 0 ? Data.MaxAmmoSpec : Data.MaxAmmo);
	WeaponAttributes->SetAttributeAmmo(Data.Ammo);
	WeaponAttributes->SetAttributeMaxMagazines(Data.MaxMagazinesSpec > 0 ? Data.MaxMagazinesSpec : Data.MaxMagazines);
	WeaponAttributes->SetAttributeAmountMagazines(Data.AmountMagazines);
	
	WeaponAttributes->SetAttributeWeaponTalentPoints(Data.WeaponTalentPoints);
	WeaponAttributes->SetDamageMultiplier(Data.DamageMultiplier);
	WeaponAttributes->SetCooldownMultiplier(Data.CooldownMultiplier);
	WeaponAttributes->SetFireRateMultiplier(Data.FireRateMultiplier);
	WeaponAttributes->SetReloadSpeedMultiplier(Data.ReloadSpeedMultiplier);
	WeaponAttributes->SetPierceExtraCount(Data.PierceExtraCount);
	WeaponAttributes->SetProjectileExtraCount(Data.ProjectileExtraCount);

	WeaponAttributes->SetDamageTalentLevel(Data.DamageTalentLevel);
	WeaponAttributes->SetCooldownTalentLevel(Data.CooldownTalentLevel);
	WeaponAttributes->SetFireRateTalentLevel(Data.FireRateTalentLevel);
	WeaponAttributes->SetReloadSpeedTalentLevel(Data.ReloadSpeedTalentLevel);
	WeaponAttributes->SetPierceTalentLevel(Data.PierceTalentLevel);
	WeaponAttributes->SetProjectileTalentLevel(Data.ProjectileTalentLevel);
	WeaponAttributes->SetMaxAmmoTalentLevel(Data.MaxAmmoTalentLevel);
	WeaponAttributes->SetAmountMagazinesTalentLevel(Data.AmountMagazinesTalentLevel);

	WeaponAttributes->SetAttributeEffectTalentPoints(Data.EffectTalentPoints);
	WeaponAttributes->SetAttributeSelectedEffectIndex(Data.SelectedEffectIndex1);
	WeaponAttributes->SetAttributeSelectedEffectIndex2(Data.SelectedEffectIndex2);
	WeaponAttributes->SetAttributeSelectedEffectIndex3(Data.SelectedEffectIndex3);

	if (ALevelUnit* LevelUnit = Cast<ALevelUnit>(GetOwner())) {
		int32 LevelPoints = LevelUnit->GetCharacterLevel() / LevelDivisor;
		float TotalPoints = EffectAreaTalentPoints + (float)LevelPoints;
		WeaponAttributes->SetAttributeEffectAreaTalentPoints(TotalPoints);
	}
	else
	{
		WeaponAttributes->SetAttributeEffectAreaTalentPoints(EffectAreaTalentPoints);
	}
}

void UWeaponComponent::SaveAttributesToWeapon(int32 Index)
{
	if (!WeaponAttributes)
	{
		if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
			if (ASC)
			{
				WeaponAttributes = const_cast<UWeaponAttributeSet*>(ASC->GetSet<UWeaponAttributeSet>());
			}
		}
	}
	
	if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(Index)) return;
	FWeaponData& Data = AvailableWeapons[Index];

	Data.Ammo = WeaponAttributes->GetAmmo();
	Data.MaxAmmoSpec = WeaponAttributes->GetMaxAmmo();
	Data.AmountMagazines = WeaponAttributes->GetAmountMagazines();
	Data.MaxMagazinesSpec = WeaponAttributes->GetMaxMagazines();
	Data.WeaponTalentPoints = WeaponAttributes->GetWeaponTalentPoints();
	Data.DamageMultiplier = WeaponAttributes->GetDamageMultiplier();
	Data.CooldownMultiplier = WeaponAttributes->GetCooldownMultiplier();
	Data.FireRateMultiplier = WeaponAttributes->GetFireRateMultiplier();
	Data.ReloadSpeedMultiplier = WeaponAttributes->GetReloadSpeedMultiplier();
	Data.PierceExtraCount = WeaponAttributes->GetPierceExtraCount();
	Data.ProjectileExtraCount = WeaponAttributes->GetProjectileExtraCount();

	Data.DamageTalentLevel = WeaponAttributes->GetDamageTalentLevel();
	Data.CooldownTalentLevel = WeaponAttributes->GetCooldownTalentLevel();
	Data.FireRateTalentLevel = WeaponAttributes->GetFireRateTalentLevel();
	Data.ReloadSpeedTalentLevel = WeaponAttributes->GetReloadSpeedTalentLevel();
	Data.PierceTalentLevel = WeaponAttributes->GetPierceTalentLevel();
	Data.ProjectileTalentLevel = WeaponAttributes->GetProjectileTalentLevel();
	Data.MaxAmmoTalentLevel = WeaponAttributes->GetMaxAmmoTalentLevel();
	Data.AmountMagazinesTalentLevel = WeaponAttributes->GetAmountMagazinesTalentLevel();

	Data.EffectTalentPoints = WeaponAttributes->GetEffectTalentPoints();
	Data.SelectedEffectIndex1 = WeaponAttributes->GetSelectedEffectIndex();
	Data.SelectedEffectIndex2 = WeaponAttributes->GetSelectedEffectIndex2();
	Data.SelectedEffectIndex3 = WeaponAttributes->GetSelectedEffectIndex3();
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
	if (!GetOwner()->HasAuthority())
	{
		Server_PurchaseUpgrade(Upgrade);
		return true;
	}

	if (!WeaponAttributes) return false;

	float AvailablePoints = WeaponAttributes->GetWeaponTalentPoints();

	if (AvailablePoints < Upgrade.Cost)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return false;
	}

	// Punkte abziehen
	WeaponAttributes->SetAttributeWeaponTalentPoints(AvailablePoints - Upgrade.Cost);

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
   	else if (Upgrade.Attribute == UWeaponAttributeSet::GetFireRateMultiplierAttribute())
   	{
   		NewValue = FMath::Pow(0.8f, NewLevel);
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

void UWeaponComponent::Server_PurchaseUpgrade_Implementation(FWeaponUpgrade Upgrade)
{
	PurchaseUpgrade(Upgrade);
}

void UWeaponComponent::Server_SelectEffectTalent_Implementation(int32 Index)
{
	if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(CurrentWeaponIndex)) return;
	FWeaponData& Data = AvailableWeapons[CurrentWeaponIndex];

	if (!Data.EffectTalents.IsValidIndex(Index)) return;

	// Check if already selected
	if (Data.SelectedEffectIndex1 == Index || Data.SelectedEffectIndex2 == Index || Data.SelectedEffectIndex3 == Index) return;

	float Cost = (float)Index + 1.0f;
	float AvailablePoints = WeaponAttributes->GetEffectTalentPoints();

	if (AvailablePoints >= Cost)
	{
		// 1. Update Attributes first
		WeaponAttributes->SetAttributeEffectTalentPoints(AvailablePoints - Cost);
		
		if (Data.SelectedEffectIndex1 == -1) WeaponAttributes->SetAttributeSelectedEffectIndex((float)Index);
		else if (Data.SelectedEffectIndex2 == -1) WeaponAttributes->SetAttributeSelectedEffectIndex2((float)Index);
		else if (Data.SelectedEffectIndex3 == -1) WeaponAttributes->SetAttributeSelectedEffectIndex3((float)Index);

		// 2. Save Attributes back to Data struct
		SaveAttributesToWeapon(CurrentWeaponIndex);
		
		UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponComponent: Selected Effect Talent %d. Cost: %.1f"), Index, Cost);
	}
}

void UWeaponComponent::Server_SelectEffectAreaIndex_Implementation(int32 Index)
{
	if (EffectAreas.IsValidIndex(Index))
	{
		if (WeaponAttributes)
		{
			WeaponAttributes->SetAttributeSelectedEffectAreaIndex(Index);
		}
	}
}

void UWeaponComponent::Server_ToggleEffectAreaTalent_Implementation(int32 AreaIndex, int32 TalentIndex)
{
	if (!EffectAreas.IsValidIndex(AreaIndex)) return;
	FEffectAreaData& AreaData = EffectAreas[AreaIndex];

	if (!AreaData.PossibleEffects.IsValidIndex(TalentIndex)) return;

	if (AreaData.SelectedTalentIndices.Contains(TalentIndex))
	{
		AreaData.SelectedTalentIndices.Remove(TalentIndex);
		AreaData.SpentPoints -= 1.0f;
		EffectAreaTalentPoints += 1.0f;
	}
	else
	{
		if (AreaData.SelectedTalentIndices.Num() < 3 && EffectAreaTalentPoints >= 1.0f)
		{
			AreaData.SelectedTalentIndices.Add(TalentIndex);
			AreaData.SpentPoints += 1.0f;
			EffectAreaTalentPoints -= 1.0f;
		}
	}

	EffectAreas[AreaIndex] = AreaData; // Force replication
	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

void UWeaponComponent::Server_InvestInEffectAreaRadius_Implementation(int32 AreaIndex)
{
	if (!EffectAreas.IsValidIndex(AreaIndex)) return;
	if (EffectAreaTalentPoints >= 1.0f)
	{
		EffectAreas[AreaIndex].RadiusInvestments++;
		EffectAreas[AreaIndex].SpentPoints += 1.0f;
		EffectAreaTalentPoints -= 1.0f;
		SyncAttributesFromWeapon(CurrentWeaponIndex);
	}
}

void UWeaponComponent::Server_InvestInEffectAreaDamage_Implementation(int32 AreaIndex)
{
	if (!EffectAreas.IsValidIndex(AreaIndex)) return;
	if (EffectAreaTalentPoints >= 1.0f)
	{
		EffectAreas[AreaIndex].DamageInvestments++;
		EffectAreas[AreaIndex].SpentPoints += 1.0f;
		EffectAreaTalentPoints -= 1.0f;
		SyncAttributesFromWeapon(CurrentWeaponIndex);
	}
}

void UWeaponComponent::LoadDataFromTables()
{
	if (WeaponDataTable)
	{
		AvailableWeapons.Empty();
		static const FString ContextString(TEXT("WeaponData"));
		TArray<FWeaponData*> Rows;
		WeaponDataTable->GetAllRows<FWeaponData>(ContextString, Rows);
		for (const auto* Row : Rows)
		{
			if (Row)
			{
				AvailableWeapons.Add(*Row);
			}
		}
	}

	if (EffectAreaDataTable)
	{
		EffectAreas.Empty();
		static const FString ContextString(TEXT("EffectAreaData"));
		TArray<FEffectAreaData*> Rows;
		EffectAreaDataTable->GetAllRows<FEffectAreaData>(ContextString, Rows);
		for (const auto* Row : Rows)
		{
			if (Row)
			{
				EffectAreas.Add(*Row);
			}
		}
	}
}

void UWeaponComponent::Server_ResetCurrentWeaponTalents_Implementation()
{
	if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(CurrentWeaponIndex)) return;

	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return;

	// --- 1. Reset Normal Talents ---
	float InvestedPoints = 0.0f;
	InvestedPoints += WeaponAttributes->GetDamageTalentLevel();
	InvestedPoints += WeaponAttributes->GetCooldownTalentLevel();
	InvestedPoints += WeaponAttributes->GetFireRateTalentLevel();
	InvestedPoints += WeaponAttributes->GetReloadSpeedTalentLevel();
	InvestedPoints += WeaponAttributes->GetPierceTalentLevel();
	InvestedPoints += WeaponAttributes->GetProjectileTalentLevel();
	InvestedPoints += WeaponAttributes->GetMaxAmmoTalentLevel();
	InvestedPoints += WeaponAttributes->GetAmountMagazinesTalentLevel();

	// Refund normal points
	float CurrentPoints = WeaponAttributes->GetWeaponTalentPoints();
	WeaponAttributes->SetAttributeWeaponTalentPoints(CurrentPoints + InvestedPoints);

	// Reset Normal Multipliers / Extra Counts to default values
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetDamageMultiplierAttribute(), 1.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetCooldownMultiplierAttribute(), 1.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetFireRateMultiplierAttribute(), 1.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetReloadSpeedMultiplierAttribute(), 1.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetPierceExtraCountAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetProjectileExtraCountAttribute(), 0.0f);

	// Reset Ammo attributes to weapon base values
	FWeaponData& Data = AvailableWeapons[CurrentWeaponIndex];
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetMaxAmmoAttribute(), Data.MaxAmmo);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesAttribute(), Data.AmountMagazines);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetMaxMagazinesAttribute(), Data.MaxMagazines);

	// Reset Normal Levels to zero
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetDamageTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetCooldownTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetFireRateTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetReloadSpeedTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetPierceTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetProjectileTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetMaxAmmoTalentLevelAttribute(), 0.0f);
	ASC->SetNumericAttributeBase(UWeaponAttributeSet::GetAmountMagazinesTalentLevelAttribute(), 0.0f);

	// --- 2. Reset Effect Talents ---
	float RefundPoints = 0.0f;
	int32 Idx1 = FMath::FloorToInt(WeaponAttributes->GetSelectedEffectIndex());
	int32 Idx2 = FMath::FloorToInt(WeaponAttributes->GetSelectedEffectIndex2());
	int32 Idx3 = FMath::FloorToInt(WeaponAttributes->GetSelectedEffectIndex3());
	
	if (Idx1 != -1) RefundPoints += (float)Idx1 + 1.0f;
	if (Idx2 != -1) RefundPoints += (float)Idx2 + 1.0f;
	if (Idx3 != -1) RefundPoints += (float)Idx3 + 1.0f;

	if (RefundPoints > 0.0f)
	{
		float CurrentEffectPoints = WeaponAttributes->GetEffectTalentPoints();
		WeaponAttributes->SetAttributeEffectTalentPoints(CurrentEffectPoints + RefundPoints);
		WeaponAttributes->SetAttributeSelectedEffectIndex(-1.0f);
		WeaponAttributes->SetAttributeSelectedEffectIndex2(-1.0f);
		WeaponAttributes->SetAttributeSelectedEffectIndex3(-1.0f);
	}

	SaveAttributesToWeapon(CurrentWeaponIndex);

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponComponent: Reset talents for current weapon. Refunded %.1f normal points."), InvestedPoints);
}

void UWeaponComponent::Server_ResetEffectAreaTalents_Implementation(int32 AreaIndex)
{
	if (EffectAreas.IsValidIndex(AreaIndex))
	{
		FEffectAreaData& AreaData = EffectAreas[AreaIndex];
		EffectAreaTalentPoints += AreaData.SpentPoints;
		AreaData.SpentPoints = 0.0f;
		AreaData.SelectedTalentIndices.Empty();
		AreaData.RadiusInvestments = 0;
		AreaData.DamageInvestments = 0;

		SyncAttributesFromWeapon(CurrentWeaponIndex);
	}
}

void UWeaponComponent::OnUnitSave(AUnitBase* Unit, FUnitSaveData& SaveData)
{
	if (Unit != GetOwner()) return;

	SaveAttributesToWeapon(CurrentWeaponIndex);

	auto SerializeArray = [&](const auto& Array, const FString& Key)
	{
		TArray<TSharedPtr<FJsonValue>> JsonArray;
		for (const auto& Item : Array)
		{
			TSharedPtr<FJsonObject> JsonObj = FJsonObjectConverter::UStructToJsonObject(Item);
			JsonArray.Add(MakeShared<FJsonValueObject>(JsonObj));
		}
		FString JsonString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
		FJsonSerializer::Serialize(JsonArray, Writer);
		SaveData.SerializedModuleData.Add(Key, JsonString);
	};

	SerializeArray(AvailableWeapons, TEXT("AvailableWeapons"));
	SerializeArray(EffectAreas, TEXT("EffectAreas"));

	SaveData.SerializedModuleData.Add(TEXT("CurrentWeaponIndex"), FString::FromInt(CurrentWeaponIndex));
	SaveData.SerializedModuleData.Add(TEXT("EffectAreaTalentPoints"), FString::SanitizeFloat(EffectAreaTalentPoints));
}

void UWeaponComponent::OnUnitLoad(AUnitBase* Unit, FUnitSaveData& SaveData)
{
	if (Unit != GetOwner()) return;

	if (FString* WeaponsJson = SaveData.SerializedModuleData.Find(TEXT("AvailableWeapons")))
	{
		FJsonObjectConverter::JsonArrayStringToUStruct<FWeaponData>(*WeaponsJson, &AvailableWeapons);
	}

	if (FString* EffectAreasJson = SaveData.SerializedModuleData.Find(TEXT("EffectAreas")))
	{
		FJsonObjectConverter::JsonArrayStringToUStruct<FEffectAreaData>(*EffectAreasJson, &EffectAreas);
	}

	if (FString* IndexStr = SaveData.SerializedModuleData.Find(TEXT("CurrentWeaponIndex")))
	{
		CurrentWeaponIndex = FCString::Atoi(**IndexStr);
	}

	if (FString* PointsStr = SaveData.SerializedModuleData.Find(TEXT("EffectAreaTalentPoints")))
	{
		EffectAreaTalentPoints = FCString::Atof(**PointsStr);
	}

	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

#if WITH_EDITOR
void UWeaponComponent::OnRegister()
{
	Super::OnRegister();
	LoadDataFromTables();
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
			MemberName == GET_MEMBER_NAME_CHECKED(UWeaponComponent, AvailableWeapons) ||
			MemberName == GET_MEMBER_NAME_CHECKED(UWeaponComponent, WeaponDataTable) ||
			MemberName == GET_MEMBER_NAME_CHECKED(UWeaponComponent, EffectAreaDataTable))
		{
			LoadDataFromTables();
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
