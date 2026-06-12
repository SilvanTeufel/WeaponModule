// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "AWeaponSpawnTableConfig.generated.h"

UCLASS()
class WEAPONMODULE_API AWeaponSpawnTableConfig : public AActor
{
    GENERATED_BODY()

public:
    AWeaponSpawnTableConfig();

    UPROPERTY(EditAnywhere, Category = "Spawn")
    TArray<TSoftObjectPtr<UDataTable>> WeaponSpawnParameters;

protected:
    virtual void BeginPlay() override;
};
