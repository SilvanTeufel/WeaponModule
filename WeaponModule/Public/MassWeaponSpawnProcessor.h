// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "Core/UnitData.h"
#include "MassWeaponSpawnProcessor.generated.h"

class ARTSGameModeBase;
class AWaypoint;
class AUnitBase;

USTRUCT()
struct FMassUnitInfo
{
	GENERATED_BODY()

	FVector Location;
	int32 TeamId;
};

USTRUCT()
struct FWeaponSpawnPointData
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AWaypoint> Waypoint;

	UPROPERTY()
	FUnitSpawnParameter SpawnParams;

	UPROPERTY()
	float LastSpawnTime = 0.f;

	UPROPERTY()
	bool bIsActive = false;

	UPROPERTY()
	TArray<TWeakObjectPtr<AUnitBase>> SpawnedUnits;

	UPROPERTY()
	float ActivationRadius = 2500.f;
};

UCLASS()
class WEAPONMODULE_API UMassWeaponSpawnProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassWeaponSpawnProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	void ScanWaypoints(UWorld* World);
	bool CheckForEnemiesInRadius(const FVector& Location, float Radius, int32 TeamId, const TArray<FMassUnitInfo>& UnitsInfo, float* OutMinDist = nullptr) const;
	void ExecuteSpawn(FWeaponSpawnPointData& SpawnData, ARTSGameModeBase* GameMode, const FVector& Location, UWorld* World);
	void ExecuteDespawn(FWeaponSpawnPointData& SpawnData);
	bool AreUnitsDead(const TArray<TWeakObjectPtr<AUnitBase>>& Units) const;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float ExecutionInterval = 1.0f;

	float TimeSinceLastRun = 0.0f;

	UPROPERTY()
	TArray<FWeaponSpawnPointData> ManagedSpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float DefaultActivationRadius = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float DespawnThreshold = 200.f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float WaypointScanInterval = 5.0f;

	float LastWaypointScanTime = -5.0f; // Ensure immediate scan on start

	FMassEntityQuery UnitQuery;
};
