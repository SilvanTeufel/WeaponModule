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
	
	// Increment Level Attribute if valid
	if (Upgrade.LevelAttribute.IsValid())
	{
		float CurrentLevel = ASC->GetNumericAttributeBase(Upgrade.LevelAttribute);
		ASC->SetNumericAttributeBase(Upgrade.LevelAttribute, CurrentLevel + 1.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("[WeaponModule] WeaponComponent: Purchased upgrade %s. New Base: %.2f. Points remaining: %.1f"), 
		*Upgrade.Name.ToString(), NewBase, WeaponAttributes->GetWeaponTalentPoints());
	
	return true;
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
