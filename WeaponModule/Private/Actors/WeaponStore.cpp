// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Actors/WeaponStore.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/WeaponHUDComponent.h"
#include "UI/WeaponSelectionHUDWidget.h"
#include "Characters/Unit/UnitBase.h"
#include "Controller/PlayerController/ControllerBase.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AWeaponStore::AWeaponStore()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	RootComponent = TriggerSphere;
	TriggerSphere->InitSphereRadius(TriggerRadius);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetGenerateOverlapEvents(true);

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(RootComponent);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeaponStore::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponStore, TeamId);
	DOREPLIFETIME(AWeaponStore, StoreItems);
}

void AWeaponStore::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (TriggerSphere)
	{
		TriggerSphere->SetSphereRadius(TriggerRadius);
	}
}

void AWeaponStore::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerSphere)
	{
		TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponStore::OnOverlapBegin);
		TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponStore::OnOverlapEnd);
	}
}

AUnitBase* AWeaponStore::ResolveCustomer(AActor* OtherActor) const
{
	AUnitBase* Unit = Cast<AUnitBase>(OtherActor);
	if (!Unit) return nullptr;
	if (Unit->TeamId != TeamId) return nullptr;                          // only friendly-team units may shop
	if (!Unit->FindComponentByClass<UWeaponComponent>()) return nullptr; // must carry a weapon component
	return Unit;
}

void AWeaponStore::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AUnitBase* Unit = ResolveCustomer(OtherActor);
	if (!Unit) return;

	// Server: register this store as the unit's active store -> the trusted gate for BuyStoreItem.
	if (HasAuthority())
	{
		if (UWeaponComponent* WC = Unit->FindComponentByClass<UWeaponComponent>())
		{
			WC->SetCurrentStore(this);
		}
	}

	// Any machine with a local player opens the widget locally (the unit/team match is done above).
	TryOpenStoreUI(Unit);
}

void AWeaponStore::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AUnitBase* Unit = Cast<AUnitBase>(OtherActor);
	if (!Unit) return;

	if (HasAuthority())
	{
		if (UWeaponComponent* WC = Unit->FindComponentByClass<UWeaponComponent>())
		{
			WC->ClearCurrentStore(this);
		}
	}

	TryCloseStoreUI(Unit);
}

void AWeaponStore::TryOpenStoreUI(AUnitBase* Unit)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Open on the LOCAL player's HUD. The customer gate (Unit->TeamId == store TeamId) is already applied
	// in ResolveCustomer, so we do NOT additionally compare the PlayerController's SelectableTeamId here.
	// A dedicated server has no local player / widget instance, so this safely no-ops there.
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(World, 0);
	if (!LocalPC) return;

	if (UWeaponHUDComponent* HUDComp = LocalPC->FindComponentByClass<UWeaponHUDComponent>())
	{
		if (HUDComp->WeaponSelectionWidgetInstance)
		{
			HUDComp->WeaponSelectionWidgetInstance->OpenStoreForUnit(Unit, this);
		}
	}
}

void AWeaponStore::TryCloseStoreUI(AUnitBase* Unit)
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(World, 0);
	if (!LocalPC) return;

	if (UWeaponHUDComponent* HUDComp = LocalPC->FindComponentByClass<UWeaponHUDComponent>())
	{
		if (HUDComp->WeaponSelectionWidgetInstance)
		{
			HUDComp->WeaponSelectionWidgetInstance->CloseStoreForUnit(Unit, this);
		}
	}
}
