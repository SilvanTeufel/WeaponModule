// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Visuals/WeaponVisualManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "Mass/MassWeaponFragment.h"

UWeaponVisualManager::UWeaponVisualManager()
{
}

UInstancedStaticMeshComponent* UWeaponVisualManager::GetOrCreateISMComponent(UStaticMesh* Mesh, bool bCastShadow, bool bReceivesDecals)
{
	if (!Mesh) return nullptr;

	FWeaponMeshKey Key;
	Key.Mesh = Mesh;
	Key.bCastShadow = bCastShadow;
	Key.bReceivesDecals = bReceivesDecals;

	if (UInstancedStaticMeshComponent** FoundISM = MeshToISMMap.Find(Key))
	{
		return *FoundISM;
	}

	AActor* OwnerActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass());
	if (OwnerActor)
	{
#if WITH_EDITOR
		OwnerActor->SetActorLabel(FString::Printf(TEXT("WeaponISM_%s_%s_%s"), *Mesh->GetName(),
			bCastShadow ? TEXT("Shadow") : TEXT("NoShadow"), bReceivesDecals ? TEXT("Decals") : TEXT("NoDecals")));
#endif
		UInstancedStaticMeshComponent* NewISM = NewObject<UInstancedStaticMeshComponent>(OwnerActor);
		NewISM->SetStaticMesh(Mesh);
		NewISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		NewISM->SetCastShadow(bCastShadow);
		NewISM->SetReceivesDecals(bReceivesDecals);
		NewISM->RegisterComponent();
		OwnerActor->SetRootComponent(NewISM);
		
		MeshToISMMap.Add(Key, NewISM);
		return NewISM;
	}

	return nullptr;
}

void UWeaponVisualManager::AssignWeapon(FMassEntityHandle Entity, UStaticMesh* Mesh, bool bCastShadow, bool bReceivesDecals)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem) return;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FMassWeaponFragment* WeaponFrag = EntityManager.GetFragmentDataPtr<FMassWeaponFragment>(Entity);

	if (!WeaponFrag || !Mesh) return;

	// Cleanup old if exists
	if (WeaponFrag->TargetISM.IsValid())
	{
		RemoveWeapon(Entity);
	}

	UInstancedStaticMeshComponent* ISM = GetOrCreateISMComponent(Mesh, bCastShadow, bReceivesDecals);
	if (!ISM) return;

	int32 NewIndex = INDEX_NONE;
	if (FreeIndexPool.Contains(ISM) && FreeIndexPool[ISM].Num() > 0)
	{
		NewIndex = FreeIndexPool[ISM].Pop();
	}
	else
	{
		NewIndex = ISM->AddInstance(FTransform::Identity);
	}

	WeaponFrag->InstanceIndex = NewIndex;
	WeaponFrag->TargetISM = ISM;
}

void UWeaponVisualManager::RemoveWeapon(FMassEntityHandle Entity)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem) return;

	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	FMassWeaponFragment* WeaponFrag = EntityManager.GetFragmentDataPtr<FMassWeaponFragment>(Entity);

	if (WeaponFrag && WeaponFrag->TargetISM.IsValid())
	{
		// Hide instance
		WeaponFrag->TargetISM->UpdateInstanceTransform(WeaponFrag->InstanceIndex, FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector), true, true, true);
		
		// Pool index
		FreeIndexPool.FindOrAdd(WeaponFrag->TargetISM.Get()).Add(WeaponFrag->InstanceIndex);

		WeaponFrag->InstanceIndex = INDEX_NONE;
		WeaponFrag->TargetISM = nullptr;
	}
}
