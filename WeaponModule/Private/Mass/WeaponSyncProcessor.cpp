// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "Mass/WeaponSyncProcessor.h"
#include "MassActorSubsystem.h"
#include "MassExecutionContext.h"
#include "Components/WeaponComponent.h"
#include "Mass/MassWeaponFragment.h"
#include "MassCommonFragments.h"
#include "Visuals/WeaponVisualManager.h"

UWeaponSyncProcessor::UWeaponSyncProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	bRequiresGameThreadExecution = true; // Required to access Actor components
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UWeaponSyncProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassWeaponFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UWeaponSyncProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWeaponVisualManager* VisualManager = Context.GetWorld()->GetSubsystem<UWeaponVisualManager>();
	if (!VisualManager) return;

	EntityQuery.ForEachEntityChunk(Context, [&, this, VisualManager](FMassExecutionContext& ChunkContext)
	{
		const TConstArrayView<FMassActorFragment> ActorList = ChunkContext.GetFragmentView<FMassActorFragment>();
		const TArrayView<FMassWeaponFragment> WeaponList = ChunkContext.GetMutableFragmentView<FMassWeaponFragment>();

		for (int32 EntityIndex = 0; EntityIndex < ChunkContext.GetNumEntities(); ++EntityIndex)
		{
			FMassWeaponFragment& WeaponFrag = WeaponList[EntityIndex];
			const AActor* Actor = ActorList[EntityIndex].Get();
			
			if (!Actor) continue;

			const UWeaponComponent* WeaponComp = Actor->FindComponentByClass<UWeaponComponent>();
			if (!WeaponComp)
			{
				if (WeaponFrag.TargetISM.IsValid())
				{
					VisualManager->RemoveWeapon(ChunkContext.GetEntity(EntityIndex));
				}
				continue;
			}

			// Sync Weapon Index and Mesh
			if (WeaponComp->CurrentWeaponIndex != WeaponFrag.CurrentWeaponIndex || !WeaponFrag.TargetISM.IsValid())
			{
				WeaponFrag.CurrentWeaponIndex = WeaponComp->CurrentWeaponIndex;
				FWeaponData WeaponData = WeaponComp->GetCurrentWeaponData();
				VisualManager->AssignWeapon(ChunkContext.GetEntity(EntityIndex), WeaponData.WeaponMesh, WeaponData.bCastShadow, WeaponData.bReceivesDecals);
				WeaponFrag.SocketName = WeaponData.SocketName;
				WeaponFrag.Offset = WeaponData.Offset;
			}
		}
	});
}
