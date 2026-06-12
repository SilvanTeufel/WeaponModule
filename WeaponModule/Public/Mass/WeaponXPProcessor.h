// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "WeaponXPProcessor.generated.h"

UCLASS()
class WEAPONMODULE_API UWeaponXPProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UWeaponXPProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery NewlyDeadQuery;
	FMassEntityQuery EarnerQuery;
};
