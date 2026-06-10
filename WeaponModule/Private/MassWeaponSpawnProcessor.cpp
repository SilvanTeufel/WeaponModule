// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "MassWeaponSpawnProcessor.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "WeaponModule.h"
#include "WeaponSpawnSubsystem.h"
#include "GameModes/RTSGameModeBase.h"
#include "Characters/Unit/UnitBase.h"
#include "Actors/Waypoint.h"
#include "Mass/UnitMassTag.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UMassWeaponSpawnProcessor::UMassWeaponSpawnProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UMassWeaponSpawnProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	UnitQuery.Initialize(EntityManager);
	UnitQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UnitQuery.AddRequirement<FMassCombatStatsFragment>(EMassFragmentAccess::ReadOnly);
	UnitQuery.AddTagRequirement<FMassStateDeadTag>(EMassFragmentPresence::None);
	UnitQuery.RegisterWithProcessor(*this);
}

void UMassWeaponSpawnProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TimeSinceLastRun += Context.GetDeltaTimeSeconds();
	if (TimeSinceLastRun < ExecutionInterval)
	{
		return;
	}
	TimeSinceLastRun = 0.0f;

	UWorld* World = GetWorld();
	if (!World) return;

	ARTSGameModeBase* GameMode = World->GetAuthGameMode<ARTSGameModeBase>();
	if (!GameMode) return;

	// 1. Periodic Waypoint Scan
	float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastWaypointScanTime >= WaypointScanInterval)
	{
		ScanWaypoints(World);
		LastWaypointScanTime = CurrentTime;
	}

	if (ManagedSpawnPoints.Num() == 0) return;

	// 2. Collect all unit info from Mass
	TArray<FMassUnitInfo> UnitsInfo;
	UnitQuery.ForEachEntityChunk(Context, ([&](FMassExecutionContext& ChunkContext)
	{
		const auto Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
		const auto Stats = ChunkContext.GetFragmentView<FMassCombatStatsFragment>();
		
		for (int i = 0; i < ChunkContext.GetNumEntities(); ++i)
		{
			FMassUnitInfo Info;
			Info.Location = Transforms[i].GetTransform().GetLocation();
			Info.TeamId = Stats[i].TeamId;
			UnitsInfo.Add(Info);
		}
	}));

	// 3. Determine Enemy presence and Spawn/Despawn Logic
	for (int32 i = 0; i < ManagedSpawnPoints.Num(); ++i)
	{
		FWeaponSpawnPointData& SpawnData = ManagedSpawnPoints[i];
		if (!SpawnData.Waypoint.IsValid()) continue;

		AWaypoint* Waypoint = SpawnData.Waypoint.Get();
		FVector Loc = Waypoint->GetActorLocation();
		int32 MyTeamId = SpawnData.SpawnParams.TeamId;
		
		float MinDist = -1.0f;
		bool bEnemyInSpawnRadius = CheckForEnemiesInRadius(Loc, SpawnData.ActivationRadius, MyTeamId, UnitsInfo, &MinDist);

		if (bEnemyInSpawnRadius)
		{
			if (!SpawnData.bIsActive)
			{
				ExecuteSpawn(SpawnData, GameMode, Loc, World);
			}
			else
			{
				if (AreUnitsDead(SpawnData.SpawnedUnits))
				{
					if (CurrentTime - SpawnData.LastSpawnTime > SpawnData.SpawnParams.LoopTime)
					{
						ExecuteSpawn(SpawnData, GameMode, Loc, World);
					}
				}
			}
		}
		else if (SpawnData.bIsActive)
		{
			float DespawnRadius = SpawnData.ActivationRadius + DespawnThreshold;
			bool bEnemyInDespawnRadius = CheckForEnemiesInRadius(Loc, DespawnRadius, MyTeamId, UnitsInfo, &MinDist);
			
			if (!bEnemyInDespawnRadius)
			{
				ExecuteDespawn(SpawnData);
			}
		}
	}
}

void UMassWeaponSpawnProcessor::ScanWaypoints(UWorld* World)
{
	UWeaponSpawnSubsystem* SpawnSubsystem = World->GetSubsystem<UWeaponSpawnSubsystem>();
	if (!SpawnSubsystem) return;

	// Collect all relevant parameters from DataTables
	TArray<FUnitSpawnParameter> AllParams;
	for (const TSoftObjectPtr<UDataTable>& TablePtr : SpawnSubsystem->LevelTables)
	{
		UDataTable* Table = TablePtr.LoadSynchronous();
		if (Table)
		{
			static const FString ContextString(TEXT("ScanWaypoints"));
			TArray<FUnitSpawnParameter*> Rows;
			Table->GetAllRows<FUnitSpawnParameter>(ContextString, Rows);
			for (FUnitSpawnParameter* Row : Rows)
			{
				if (Row) AllParams.Add(*Row);
			}
		}
	}

	if (AllParams.Num() == 0)
	{
		if (ManagedSpawnPoints.Num() > 0)
		{
			ManagedSpawnPoints.Empty();
		}
		return;
	}

	int32 FoundWaypoints = 0;
	// Find Waypoints matching tags
	for (TActorIterator<AWaypoint> It(World); It; ++It)
	{
		AWaypoint* Waypoint = *It;
		if (!Waypoint) continue;

		if (Waypoint->Tag.IsEmpty())
		{
			// UE_LOG(LogWeaponModule, Verbose, TEXT("UMassWeaponSpawnProcessor: Waypoint %s has no Tag."), *Waypoint->GetName());
			continue;
		}

		for (const FUnitSpawnParameter& Param : AllParams)
		{
			if (Waypoint->Tag == Param.WaypointTag)
			{
				FoundWaypoints++;
				// Check if already managed
				FWeaponSpawnPointData* ExistingData = ManagedSpawnPoints.FindByPredicate([Waypoint, &Param](const FWeaponSpawnPointData& Existing)
				{
					return Existing.Waypoint == Waypoint && Existing.SpawnParams.Id == Param.Id;
				});

				if (!ExistingData)
				{
					FWeaponSpawnPointData NewData;
					NewData.Waypoint = Waypoint;
					NewData.SpawnParams = Param;
					NewData.ActivationRadius = DefaultActivationRadius;
					ManagedSpawnPoints.Add(NewData);
				}
				else
				{
					// Update params and radius in case they changed in DataTable or Settings
					ExistingData->SpawnParams = Param;
					ExistingData->ActivationRadius = DefaultActivationRadius;
				}
			}
		}
	}

	// Cleanup invalid waypoints
	ManagedSpawnPoints.RemoveAll([](const FWeaponSpawnPointData& Data) { return !Data.Waypoint.IsValid(); });
}

bool UMassWeaponSpawnProcessor::CheckForEnemiesInRadius(const FVector& Location, float Radius, int32 TeamId, const TArray<FMassUnitInfo>& UnitsInfo, float* OutMinDist) const
{
	float RadiusSq = Radius * Radius;
	float MinDistSq = -1.0f;
	bool bFound = false;

	for (const FMassUnitInfo& Info : UnitsInfo)
	{
		if (Info.TeamId != TeamId)
		{
			float DistSq = FVector::DistSquared2D(Location, Info.Location);
			if (DistSq <= RadiusSq)
			{
				bFound = true;
			}
			
			if (MinDistSq < 0 || DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
			}
		}
	}

	if (OutMinDist && MinDistSq >= 0)
	{
		*OutMinDist = FMath::Sqrt(MinDistSq);
	}

	return bFound;
}

void UMassWeaponSpawnProcessor::ExecuteSpawn(FWeaponSpawnPointData& SpawnData, ARTSGameModeBase* GameMode, const FVector& Location, UWorld* World)
{
	SpawnData.SpawnedUnits.Empty();
	for (int32 i = 0; i < SpawnData.SpawnParams.UnitCount; ++i)
	{
		AUnitBase* NewUnit = GameMode->SpawnSingleUnit(SpawnData.SpawnParams, Location, nullptr, SpawnData.SpawnParams.TeamId, SpawnData.Waypoint.Get());
		if (NewUnit)
		{
			SpawnData.SpawnedUnits.Add(NewUnit);
		}
	}
	SpawnData.bIsActive = true;
	SpawnData.LastSpawnTime = World->GetTimeSeconds();
}

void UMassWeaponSpawnProcessor::ExecuteDespawn(FWeaponSpawnPointData& SpawnData)
{
	for (auto& UnitPtr : SpawnData.SpawnedUnits)
	{
		if (UnitPtr.IsValid())
		{
			UnitPtr->Destroy();
		}
	}
	SpawnData.SpawnedUnits.Empty();
	SpawnData.bIsActive = false;
}

bool UMassWeaponSpawnProcessor::AreUnitsDead(const TArray<TWeakObjectPtr<AUnitBase>>& Units) const
{
	if (Units.Num() == 0) return true;
	for (const auto& UnitPtr : Units)
	{
		if (UnitPtr.IsValid() && UnitPtr->GetUnitState() != UnitData::Dead)
		{
			return false;
		}
	}
	return true;
}
