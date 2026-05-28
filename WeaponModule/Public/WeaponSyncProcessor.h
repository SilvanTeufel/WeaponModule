#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "WeaponSyncProcessor.generated.h"

/**
 * Syncs data from WeaponComponent (Actor) to FMassWeaponFragment (Mass)
 */
UCLASS()
class WEAPONMODULE_API UWeaponSyncProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UWeaponSyncProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
