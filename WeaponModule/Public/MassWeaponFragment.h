// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MassWeaponFragment.generated.h"

USTRUCT()
struct WEAPONMODULE_API FMassWeaponFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 InstanceIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TWeakObjectPtr<UInstancedStaticMeshComponent> TargetISM;
	
	UPROPERTY(Transient)
	int32 CurrentWeaponIndex = INDEX_NONE;
	
	UPROPERTY(Transient)
	FName SocketName = NAME_None;
};
