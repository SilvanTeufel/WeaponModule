// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "WeaponVisualManager.generated.h"

struct FMassEntityHandle;

USTRUCT()
struct FWeaponMeshKey {
	GENERATED_BODY()
	UPROPERTY()
	UStaticMesh* Mesh = nullptr;
	UPROPERTY()
	bool bCastShadow = false;
	UPROPERTY()
	bool bReceivesDecals = false;

	bool operator==(const FWeaponMeshKey& Other) const {
		return Mesh == Other.Mesh && bCastShadow == Other.bCastShadow && bReceivesDecals == Other.bReceivesDecals;
	}

	friend uint32 GetTypeHash(const FWeaponMeshKey& Key) {
		return HashCombine(HashCombine(GetTypeHash(Key.Mesh), GetTypeHash(Key.bCastShadow)), GetTypeHash(Key.bReceivesDecals));
	}
};

/**
 * 
 */
UCLASS()
class WEAPONMODULE_API UWeaponVisualManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UWeaponVisualManager();

	UInstancedStaticMeshComponent* GetOrCreateISMComponent(UStaticMesh* Mesh, bool bCastShadow, bool bReceivesDecals);

	void AssignWeapon(FMassEntityHandle Entity, UStaticMesh* Mesh, bool bCastShadow, bool bReceivesDecals);
	void RemoveWeapon(FMassEntityHandle Entity);

protected:
	UPROPERTY()
	TMap<FWeaponMeshKey, UInstancedStaticMeshComponent*> MeshToISMMap;

	TMap<UInstancedStaticMeshComponent*, TArray<int32>> FreeIndexPool;
};
