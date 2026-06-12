// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "WeaponModuleSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Weapon Module Settings"))
class WEAPONMODULE_API UWeaponModuleSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
};
