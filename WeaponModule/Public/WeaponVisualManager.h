// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "WeaponVisualManager.generated.h"

struct FMassEntityHandle;

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UWeaponVisualManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UWeaponVisualManager();

	UInstancedStaticMeshComponent* GetOrCreateISMComponent(UStaticMesh* Mesh);

	void AssignWeapon(FMassEntityHandle Entity, UStaticMesh* Mesh);
	void RemoveWeapon(FMassEntityHandle Entity);

protected:
	UPROPERTY()
	TMap<UStaticMesh*, UInstancedStaticMeshComponent*> MeshToISMMap;

	TMap<UInstancedStaticMeshComponent*, TArray<int32>> FreeIndexPool;
};
