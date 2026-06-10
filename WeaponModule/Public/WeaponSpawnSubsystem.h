// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "WeaponSpawnSubsystem.generated.h"

UCLASS()
class WEAPONMODULE_API UWeaponSpawnSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<TSoftObjectPtr<UDataTable>> LevelTables;

    void RegisterTables(const TArray<TSoftObjectPtr<UDataTable>>& InTables)
    {
        LevelTables.Append(InTables);
    }

    virtual void OnWorldBeginPlay(UWorld& InWorld) override
    {
        Super::OnWorldBeginPlay(InWorld);
        LevelTables.Empty();
    }
};
