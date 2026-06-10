// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "AWeaponSpawnTableConfig.h"
#include "WeaponSpawnSubsystem.h"
#include "Engine/World.h"

AWeaponSpawnTableConfig::AWeaponSpawnTableConfig()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWeaponSpawnTableConfig::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (UWeaponSpawnSubsystem* Subsystem = World->GetSubsystem<UWeaponSpawnSubsystem>())
        {
            Subsystem->RegisterTables(WeaponSpawnParameters);
        }
    }
}
