// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Components/WeaponComponent.h"
#include "Abilities/WeaponAttributeSet.h"
#include "Abilities/ShootAbility.h"
#include "Characters/Unit/LevelUnit.h"
#include "Characters/Unit/AbilityUnit.h"
#include "Characters/Unit/PerformanceUnit.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"
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
	DOREPLIFETIME(UWeaponComponent, CrowdControl);
	DOREPLIFETIME(UWeaponComponent, TalentTreeNodes);
	DOREPLIFETIME(UWeaponComponent, TalentTreeSpentPoints);
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

	// Crowd-control points: Start + (CharacterLevel / Divisor) - Spent. Server-computed, replicated to the UI.
	WeaponAttributes->SetAttributeCrowdControlTalentPoints(GetAvailableCrowdControlPoints());

	if (AAbilityUnit* Unit = Cast<AAbilityUnit>(GetOwner())) {
		if (ALevelUnit* LevelUnit = Cast<ALevelUnit>(Unit)) {
			int32 LevelPoints = LevelUnit->GetCharacterLevel() / LevelDivisor;
			float TotalPoints = EffectAreaTalentPoints + (float)LevelPoints;
			WeaponAttributes->SetAttributeEffectAreaTalentPoints(TotalPoints);
		}
		else
		{
			WeaponAttributes->SetAttributeEffectAreaTalentPoints(EffectAreaTalentPoints);
		}
		Unit->ContinuousAttackDuration = Data.FireRate * Data.FireRateMultiplier;

		// Sync the unit's autoattack projectile to the CURRENT weapon's projectile.
		// The RTSUnitTemplate autoattack fallback (UnitStateProcessor -> AUnitBase::SpawnProjectileWithEntities)
		// fires APerformanceUnit::ProjectileBaseClass, NOT FWeaponData.ProjectileClass. Without this sync the
		// autoattack always uses the unit's default projectile regardless of which weapon is equipped.
		// ProjectileBaseClass is declared on APerformanceUnit (a subclass of AAbilityUnit), so cast down to it.
		// Guarded so melee / projectile-less weapons don't wipe a designer-set default.
		if (Data.ProjectileClass)
		{
			if (APerformanceUnit* PerfUnit = Cast<APerformanceUnit>(Unit))
			{
				PerfUnit->ProjectileBaseClass = Data.ProjectileClass;
			}
		}
	}

	// ShootAbility Instanzen finden und synchronisieren
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		if (UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent())
		{
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
			{
				if (Spec.Ability && Spec.Ability->IsA(UShootAbility::StaticClass()))
				{
					for (UGameplayAbility* Inst : Spec.GetAbilityInstances())
					{
						if (UShootAbility* ShootInst = Cast<UShootAbility>(Inst))
						{
							ShootInst->SynchronizeContinuousCooldown();
						}
					}
				}
			}
		}
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

	// Punkte abziehen
	WeaponAttributes->SetAttributeWeaponTalentPoints(AvailablePoints - Upgrade.Cost);

	return ApplyWeaponUpgradeEffect(Upgrade);
}

bool UWeaponComponent::ApplyWeaponUpgradeEffect(const FWeaponUpgrade& Upgrade)
{
	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC)
	{
		return false;
	}

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
        // Formula: Benefit = ModifierValue^(Tier - 1)
        NewValue = (EffectiveTier > 0) ? FMath::Pow(Upgrade.ModifierValue, EffectiveTier - 1.0f) : 0.0f;
    }
   	else if (Upgrade.Attribute == UWeaponAttributeSet::GetFireRateMultiplierAttribute())
   	{
   		NewValue = FMath::Pow(Upgrade.ModifierValue, EffectiveTier);
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
	SyncAttributesFromWeapon(CurrentWeaponIndex);

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

	float Cost = ((float)Index + 1.0f) * ProjectileEffectCostMultiplier;
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
		AreaData.SpentPoints -= AreaEffectToggleCost;
		EffectAreaTalentPoints += AreaEffectToggleCost;
	}
	else
	{
		if (AreaData.SelectedTalentIndices.Num() < MaxAreaEffects && EffectAreaTalentPoints >= AreaEffectToggleCost)
		{
			AreaData.SelectedTalentIndices.Add(TalentIndex);
			AreaData.SpentPoints += AreaEffectToggleCost;
			EffectAreaTalentPoints -= AreaEffectToggleCost;
		}
	}

	EffectAreas[AreaIndex] = AreaData; // Force replication
	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

void UWeaponComponent::Server_InvestInEffectAreaRadius_Implementation(int32 AreaIndex)
{
	if (!EffectAreas.IsValidIndex(AreaIndex)) return;
	if (EffectAreaTalentPoints >= AreaRadiusUpgradeCost)
	{
		EffectAreas[AreaIndex].RadiusInvestments++;
		EffectAreas[AreaIndex].SpentPoints += AreaRadiusUpgradeCost;
		EffectAreaTalentPoints -= AreaRadiusUpgradeCost;
		SyncAttributesFromWeapon(CurrentWeaponIndex);
	}
}

void UWeaponComponent::Server_InvestInEffectAreaDamage_Implementation(int32 AreaIndex)
{
	if (!EffectAreas.IsValidIndex(AreaIndex)) return;
	if (EffectAreaTalentPoints >= AreaDamageUpgradeCost)
	{
		EffectAreas[AreaIndex].DamageInvestments++;
		EffectAreas[AreaIndex].SpentPoints += AreaDamageUpgradeCost;
		EffectAreaTalentPoints -= AreaDamageUpgradeCost;
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
	SyncAttributesFromWeapon(CurrentWeaponIndex);

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

void UWeaponComponent::Server_InvestInCrowdControlTalent_Implementation(ECrowdControlTalent Talent)
{
	if (GetAvailableCrowdControlPoints() < CrowdControlTalentCost)
	{
		return;
	}

	if (!ApplyCrowdControlTalentLevel(Talent))
	{
		return;
	}

	CrowdControl.SpentPoints += CrowdControlTalentCost;
	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

bool UWeaponComponent::ApplyCrowdControlTalentLevel(ECrowdControlTalent Talent)
{
	switch (Talent)
	{
	case ECrowdControlTalent::Range:      CrowdControl.RangeLevel++;      break;
	case ECrowdControlTalent::Duration:   CrowdControl.DurationLevel++;   break;
	case ECrowdControlTalent::Angle:      CrowdControl.AngleLevel++;      break;
	case ECrowdControlTalent::MaxTargets: CrowdControl.MaxTargetsLevel++; break;
	case ECrowdControlTalent::Height:     CrowdControl.HeightLevel++;     break;
	case ECrowdControlTalent::Cooldown:   CrowdControl.CooldownLevel++;   break;
	default: return false;
	}
	return true;
}

void UWeaponComponent::Server_ResetCrowdControlTalents_Implementation()
{
	CrowdControl.SpentPoints = 0.f;

	CrowdControl.RangeLevel = 0;
	CrowdControl.DurationLevel = 0;
	CrowdControl.AngleLevel = 0;
	CrowdControl.MaxTargetsLevel = 0;
	CrowdControl.HeightLevel = 0;
	CrowdControl.CooldownLevel = 0;

	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

float UWeaponComponent::GetAvailableCrowdControlPoints() const
{
	int32 Level = 0;
	if (ALevelUnit* LevelUnit = Cast<ALevelUnit>(GetOwner()))
	{
		Level = LevelUnit->GetCharacterLevel();
	}
	const int32 Divisor = FMath::Max(1, CrowdControlLevelDivisor);
	const float Granted = StartCrowdControlTalentPoints + (float)(Level / Divisor);
	return FMath::Max(0.f, Granted - CrowdControl.SpentPoints);
}

float UWeaponComponent::GetCrowdControlRadius() const     { return CrowdControl.GetEffectiveRadius(); }
float UWeaponComponent::GetCrowdControlDuration() const   { return CrowdControl.GetEffectiveDuration(); }
float UWeaponComponent::GetCrowdControlHalfAngleDeg() const { return CrowdControl.GetEffectiveHalfAngleDeg(); }
int32 UWeaponComponent::GetCrowdControlMaxTargets() const { return CrowdControl.GetEffectiveMaxTargets(); }
float UWeaponComponent::GetCrowdControlHalfHeight() const { return CrowdControl.GetEffectiveHalfHeight(); }
float UWeaponComponent::GetCrowdControlCooldown() const   { return CrowdControl.GetEffectiveCooldown(); }

// ---------------------------------------------------------------------------
//  Radial Talent Tree
// ---------------------------------------------------------------------------

float UWeaponComponent::GetAvailableTalentTreePoints() const
{
	int32 Level = 0;
	if (ALevelUnit* LevelUnit = Cast<ALevelUnit>(GetOwner()))
	{
		Level = LevelUnit->GetCharacterLevel();
	}
	const int32 Divisor = FMath::Max(1, TalentTreeLevelDivisor);
	const float Granted = StartTalentTreePoints + (float)(Level / Divisor);
	return FMath::Max(0.f, Granted - TalentTreeSpentPoints);
}

int32 UWeaponComponent::GetTalentTreeNodePoints(FName NodeId) const
{
	for (const FTalentTreeNodeState& State : TalentTreeNodes)
	{
		if (State.NodeId == NodeId)
		{
			return State.Points;
		}
	}
	return 0;
}

const FTalentTreeNodeRow* UWeaponComponent::FindTalentTreeRow(FName NodeId) const
{
	if (!TalentTreeDataTable || NodeId.IsNone())
	{
		return nullptr;
	}
	return TalentTreeDataTable->FindRow<FTalentTreeNodeRow>(NodeId, TEXT("TalentTree"), /*bWarnIfMissing=*/false);
}

bool UWeaponComponent::IsTalentTreeNodeUnlocked(FName NodeId) const
{
	const FTalentTreeNodeRow* Row = FindTalentTreeRow(NodeId);
	if (!Row)
	{
		return false;
	}
	if (Row->PrevId.IsNone())
	{
		return true; // root node
	}
	const FTalentTreeNodeRow* Parent = FindTalentTreeRow(Row->PrevId);
	if (!Parent)
	{
		return true; // dangling parent reference -> don't hard-block the node
	}
	return GetTalentTreeNodePoints(Row->PrevId) >= FMath::Max(1, Parent->MaxPoints);
}

bool UWeaponComponent::CanInvestInTalentTreeNode(FName NodeId) const
{
	const FTalentTreeNodeRow* Row = FindTalentTreeRow(NodeId);
	if (!Row)
	{
		return false;
	}
	if (GetTalentTreeNodePoints(NodeId) >= FMath::Max(1, Row->MaxPoints))
	{
		return false;
	}
	if (!IsTalentTreeNodeUnlocked(NodeId))
	{
		return false;
	}
	const float Cost = Row->PointCost > 0 ? (float)Row->PointCost : TalentTreePointCost;
	return GetAvailableTalentTreePoints() >= Cost;
}

FWeaponUpgrade UWeaponComponent::MakeWeaponUpgrade(EWeaponTalentType Type) const
{
	FWeaponUpgrade U;
	U.Cost = 1;

	switch (Type)
	{
	case EWeaponTalentType::Damage:
		U.Name          = FText::FromString(TEXT("Damage"));
		U.Attribute     = UWeaponAttributeSet::GetDamageMultiplierAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetDamageTalentLevelAttribute();
		U.ModifierValue = TreeDamageModifierValue;
		U.ModifierOp    = EGameplayModOp::Additive;
		break;
	case EWeaponTalentType::Cooldown:
		U.Name          = FText::FromString(TEXT("Cooldown"));
		U.Attribute     = UWeaponAttributeSet::GetCooldownMultiplierAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetCooldownTalentLevelAttribute();
		U.ModifierValue = TreeCooldownModifierValue;
		U.ModifierOp    = EGameplayModOp::Multiplicitive;
		break;
	case EWeaponTalentType::FireRate:
		U.Name          = FText::FromString(TEXT("Fire Rate"));
		U.Attribute     = UWeaponAttributeSet::GetFireRateMultiplierAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetFireRateTalentLevelAttribute();
		U.ModifierValue = TreeFireRateModifierValue;
		U.ModifierOp    = EGameplayModOp::Multiplicitive;
		break;
	case EWeaponTalentType::Reload:
		U.Name          = FText::FromString(TEXT("Reload Speed"));
		U.Attribute     = UWeaponAttributeSet::GetReloadSpeedMultiplierAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetReloadSpeedTalentLevelAttribute();
		U.ModifierValue = TreeReloadModifierValue;
		U.ModifierOp    = EGameplayModOp::Multiplicitive;
		break;
	case EWeaponTalentType::Pierce:
		U.Name          = FText::FromString(TEXT("Pierce"));
		U.Attribute     = UWeaponAttributeSet::GetPierceExtraCountAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetPierceTalentLevelAttribute();
		U.ModifierValue = TreePierceModifierValue;
		U.ModifierOp    = EGameplayModOp::Additive;
		break;
	case EWeaponTalentType::Projectile:
		U.Name          = FText::FromString(TEXT("Multi-Shot"));
		U.Attribute     = UWeaponAttributeSet::GetProjectileExtraCountAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetProjectileTalentLevelAttribute();
		U.ModifierValue = TreeProjectileModifierValue;
		U.ModifierOp    = EGameplayModOp::Additive;
		break;
	case EWeaponTalentType::MaxAmmo:
		U.Name          = FText::FromString(TEXT("Max Ammo"));
		U.Attribute     = UWeaponAttributeSet::GetMaxAmmoAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetMaxAmmoTalentLevelAttribute();
		U.ModifierValue = TreeAmmoModifierValue;
		U.ModifierOp    = EGameplayModOp::Additive;
		break;
	case EWeaponTalentType::Magazines:
		U.Name          = FText::FromString(TEXT("Magazines"));
		U.Attribute     = UWeaponAttributeSet::GetMaxMagazinesAttribute();
		U.LevelAttribute= UWeaponAttributeSet::GetAmountMagazinesTalentLevelAttribute();
		U.ModifierValue = TreeMagazinesModifierValue;
		U.ModifierOp    = EGameplayModOp::Additive;
		break;
	default:
		break;
	}

	return U;
}

bool UWeaponComponent::ApplyTalentTreeNodeEffect(const FTalentTreeNodeRow& Row)
{
	// NOTE ON SCOPE: CrowdControl and EffectArea effects are unit-wide (stored on the component).
	// WeaponTalent and ProjectileEffect effects are written into the CURRENTLY equipped weapon's
	// FWeaponData, so on multi-weapon units those tree bonuses follow the equipped weapon. The tree
	// node state + points are unit-wide. For single-weapon units (the common case) this is invisible.
	switch (Row.TalentType)
	{
	case ETalentTreeCategory::WeaponTalent:
	{
		// Stacking talent: always applies (unless there is no ASC / level attribute).
		return ApplyWeaponUpgradeEffect(MakeWeaponUpgrade(Row.WeaponTalent));
	}
	case ETalentTreeCategory::CrowdControlTalent:
	{
		if (ApplyCrowdControlTalentLevel(Row.CrowdControlTalent))
		{
			SyncAttributesFromWeapon(CurrentWeaponIndex);
			return true;
		}
		return false;
	}
	case ETalentTreeCategory::EffectAreaTalent:
	{
		if (!EffectAreas.IsValidIndex(Row.EffectAreaIndex))
		{
			return false;
		}
		FEffectAreaData& Area = EffectAreas[Row.EffectAreaIndex];
		bool bApplied = false;
		switch (Row.EffectAreaAction)
		{
		case EEffectAreaTalentAction::InvestRadius:
			Area.RadiusInvestments++;
			bApplied = true;
			break;
		case EEffectAreaTalentAction::InvestDamage:
			Area.DamageInvestments++;
			bApplied = true;
			break;
		case EEffectAreaTalentAction::ToggleEffect:
			if (Area.PossibleEffects.IsValidIndex(Row.EffectAreaTalentIndex)
				&& !Area.SelectedTalentIndices.Contains(Row.EffectAreaTalentIndex)
				&& Area.SelectedTalentIndices.Num() < MaxAreaEffects)
			{
				Area.SelectedTalentIndices.Add(Row.EffectAreaTalentIndex);
				bApplied = true;
			}
			break;
		default:
			break;
		}
		if (!bApplied)
		{
			return false; // nothing changed -> caller must not charge a point
		}
		EffectAreas[Row.EffectAreaIndex] = Area; // force replication of the array element
		SyncAttributesFromWeapon(CurrentWeaponIndex);
		return true;
	}
	case ETalentTreeCategory::ProjectileEffect:
	{
		if (!WeaponAttributes || !AvailableWeapons.IsValidIndex(CurrentWeaponIndex))
		{
			return false;
		}
		FWeaponData& Data = AvailableWeapons[CurrentWeaponIndex];
		const int32 Index = Row.EffectAreaIndex;
		if (!Data.EffectTalents.IsValidIndex(Index))
		{
			return false;
		}
		if (Data.SelectedEffectIndex1 == Index || Data.SelectedEffectIndex2 == Index || Data.SelectedEffectIndex3 == Index)
		{
			return false; // already selected
		}
		if (Data.SelectedEffectIndex1 == -1)      WeaponAttributes->SetAttributeSelectedEffectIndex((float)Index);
		else if (Data.SelectedEffectIndex2 == -1) WeaponAttributes->SetAttributeSelectedEffectIndex2((float)Index);
		else if (Data.SelectedEffectIndex3 == -1) WeaponAttributes->SetAttributeSelectedEffectIndex3((float)Index);
		else return false; // all three effect slots occupied
		SaveAttributesToWeapon(CurrentWeaponIndex);
		return true;
	}
	default:
		break;
	}
	return false;
}

void UWeaponComponent::Server_InvestInTalentTreeNode_Implementation(FName NodeId)
{
	const FTalentTreeNodeRow* Row = FindTalentTreeRow(NodeId);
	if (!Row)
	{
		return;
	}

	const int32 MaxPts = FMath::Max(1, Row->MaxPoints);
	if (GetTalentTreeNodePoints(NodeId) >= MaxPts)
	{
		return; // node already full
	}
	if (!IsTalentTreeNodeUnlocked(NodeId))
	{
		return; // prerequisite not satisfied
	}

	const float Cost = Row->PointCost > 0 ? (float)Row->PointCost : TalentTreePointCost;
	if (GetAvailableTalentTreePoints() < Cost)
	{
		return; // not enough tree points
	}

	// Apply the concrete underlying talent (no per-system point cost).
	// If nothing actually changed (e.g. all projectile slots occupied, effect already
	// selected), do NOT charge the player or bump the node.
	if (!ApplyTalentTreeNodeEffect(*Row))
	{
		return;
	}

	// Book-keeping: bump the node's invested count and spend the tree points.
	bool bFound = false;
	for (FTalentTreeNodeState& State : TalentTreeNodes)
	{
		if (State.NodeId == NodeId)
		{
			State.Points++;
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		FTalentTreeNodeState NewState;
		NewState.NodeId = NodeId;
		NewState.Points = 1;
		TalentTreeNodes.Add(NewState);
	}

	TalentTreeSpentPoints += Cost;
	SyncAttributesFromWeapon(CurrentWeaponIndex);
}

void UWeaponComponent::Server_ResetTalentTree_Implementation()
{
	// Refund all tree points and clear per-node state.
	TalentTreeNodes.Empty();
	TalentTreeSpentPoints = 0.f;

	// The tree drives the underlying talent EFFECTS but pays only from its own pool, so it never
	// debited the weapon / effect / crowd-control / effect-area point pools. The per-system resets
	// below zero the effect state, but they ALSO credit those pools based on the invested levels,
	// which would mint free points the tree never charged. Snapshot the spendable pools first and
	// restore them afterwards so ONLY the effects are cleared, not the currencies.
	const float SavedWeaponTalentPoints  = WeaponAttributes ? WeaponAttributes->GetWeaponTalentPoints()  : 0.f;
	const float SavedEffectTalentPoints  = WeaponAttributes ? WeaponAttributes->GetEffectTalentPoints()  : 0.f;
	const float SavedCrowdControlSpent   = CrowdControl.SpentPoints;
	const float SavedEffectAreaTalentPts = EffectAreaTalentPoints;

	Server_ResetCurrentWeaponTalents_Implementation();
	Server_ResetCrowdControlTalents_Implementation();
	for (int32 i = 0; i < EffectAreas.Num(); ++i)
	{
		Server_ResetEffectAreaTalents_Implementation(i);
	}

	// Restore the currencies (no minting, no loss for a pure-tree build).
	if (WeaponAttributes)
	{
		WeaponAttributes->SetAttributeWeaponTalentPoints(SavedWeaponTalentPoints);
		WeaponAttributes->SetAttributeEffectTalentPoints(SavedEffectTalentPoints);
	}
	CrowdControl.SpentPoints = SavedCrowdControlSpent;
	EffectAreaTalentPoints   = SavedEffectAreaTalentPts;

	SaveAttributesToWeapon(CurrentWeaponIndex);
	SyncAttributesFromWeapon(CurrentWeaponIndex);
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

	// Radial talent tree: only the tree's own bookkeeping is persisted here. The concrete stat
	// effects each node produced are already baked into AvailableWeapons / EffectAreas / CrowdControl
	// above (via SaveAttributesToWeapon and the invest handlers), so on load we restore these two as
	// RAW STATE and must NOT re-apply the node effects (that would double-apply the bonuses).
	SerializeArray(TalentTreeNodes, TEXT("TalentTreeNodes"));

	SaveData.SerializedModuleData.Add(TEXT("CurrentWeaponIndex"), FString::FromInt(CurrentWeaponIndex));
	SaveData.SerializedModuleData.Add(TEXT("EffectAreaTalentPoints"), FString::SanitizeFloat(EffectAreaTalentPoints));
	SaveData.SerializedModuleData.Add(TEXT("TalentTreeSpentPoints"), FString::SanitizeFloat(TalentTreeSpentPoints));

	FString CrowdControlJson;
	if (FJsonObjectConverter::UStructToJsonObjectString(CrowdControl, CrowdControlJson))
	{
		SaveData.SerializedModuleData.Add(TEXT("CrowdControl"), CrowdControlJson);
	}
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

	if (FString* CrowdControlJson = SaveData.SerializedModuleData.Find(TEXT("CrowdControl")))
	{
		FJsonObjectConverter::JsonObjectStringToUStruct<FCrowdControlData>(*CrowdControlJson, &CrowdControl, 0, 0);
	}

	// Radial talent tree: restore the tree's bookkeeping as raw state (see OnUnitSave). We deliberately
	// do NOT call ApplyTalentTreeNodeEffect / Server_InvestInTalentTreeNode here — the resulting weapon,
	// effect-area and crowd-control stats were already restored above, so replaying node effects would
	// stack a second bonus. TalentTreeSpentPoints must come back too, or GetAvailableTalentTreePoints()
	// would regrant the whole pool (free re-spend) and the tree UI would render every node empty.
	if (FString* TalentTreeNodesJson = SaveData.SerializedModuleData.Find(TEXT("TalentTreeNodes")))
	{
		FJsonObjectConverter::JsonArrayStringToUStruct<FTalentTreeNodeState>(*TalentTreeNodesJson, &TalentTreeNodes);
	}

	if (FString* SpentStr = SaveData.SerializedModuleData.Find(TEXT("TalentTreeSpentPoints")))
	{
		TalentTreeSpentPoints = FCString::Atof(**SpentStr);
	}

	// Restore assets from DataTables since JsonObjectConverter doesn't handle pointers/classes
	if (WeaponDataTable)
	{
		TArray<FWeaponData*> AllRows;
		static const FString ContextString(TEXT("WeaponRestore"));
		WeaponDataTable->GetAllRows<FWeaponData>(ContextString, AllRows);
		for (FWeaponData& Weapon : AvailableWeapons)
		{
			for (const FWeaponData* Row : AllRows)
			{
				if (Row && Row->WeaponTag == Weapon.WeaponTag)
				{
					Weapon.WeaponMesh = Row->WeaponMesh;
					Weapon.bCastShadow = Row->bCastShadow;
					Weapon.WeaponIcon = Row->WeaponIcon;
					Weapon.ProjectileClass = Row->ProjectileClass;
					Weapon.EffectTalents = Row->EffectTalents;
					Weapon.SocketName = Row->SocketName;
					Weapon.Offset = Row->Offset;
					break;
				}
			}
		}
	}

	if (EffectAreaDataTable)
	{
		TArray<FEffectAreaData*> AllAreaRows;
		static const FString ContextString(TEXT("AreaRestore"));
		EffectAreaDataTable->GetAllRows<FEffectAreaData>(ContextString, AllAreaRows);
		for (FEffectAreaData& Area : EffectAreas)
		{
			for (const FEffectAreaData* Row : AllAreaRows)
			{
				if (Row && Row->Name == Area.Name)
				{
					Area.PossibleEffects = Row->PossibleEffects;
					Area.BaseRadius = Row->BaseRadius;
					Area.BaseDamage = Row->BaseDamage;
					break;
				}
			}
		}
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
	WeaponPreview->SetCastShadow(Data.bCastShadow);

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
