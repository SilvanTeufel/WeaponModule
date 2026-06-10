// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "MassWeaponPlacementProcessor.h"
#include "MassWeaponFragment.h"
#include "WeaponComponent.h"
#include "WeaponVisualManager.h"
#include "MassCommonFragments.h"
#include "MassActorSubsystem.h"
#include "MassExecutionContext.h"
#include "Characters/Unit/UnitBase.h"
#include "Mass/UnitMassTag.h"

struct FWeaponISMUpdate
{
	int32 InstanceIndex;
	FTransform NewTransform;
};

UMassWeaponPlacementProcessor::UMassWeaponPlacementProcessor()
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
	ExecutionOrder.ExecuteAfter.Add(TEXT("ActorTransformSyncProcessor"));
}

void UMassWeaponPlacementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassWeaponFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassVisibilityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FMassStateDeadTag>(EMassFragmentPresence::Optional);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMassWeaponPlacementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TMap<UInstancedStaticMeshComponent*, TArray<FWeaponISMUpdate>> BatchedUpdates;
	UWeaponVisualManager* VisualManager = Context.GetWorld()->GetSubsystem<UWeaponVisualManager>();

	EntityQuery.ForEachEntityChunk(Context, ([this, &BatchedUpdates, VisualManager](FMassExecutionContext& Context) {
		TArrayView<FMassActorFragment> ActorList = Context.GetMutableFragmentView<FMassActorFragment>();
		TArrayView<FMassWeaponFragment> WeaponList = Context.GetMutableFragmentView<FMassWeaponFragment>();
		TConstArrayView<FMassVisibilityFragment> VisibilityList = Context.GetFragmentView<FMassVisibilityFragment>();
		bool bIsDead = Context.DoesArchetypeHaveTag<FMassStateDeadTag>();

		for (int i = 0; i < Context.GetNumEntities(); ++i)
		{
			FMassActorFragment& ActorFrag = ActorList[i];
			FMassWeaponFragment& WeaponFrag = WeaponList[i];
			const FMassVisibilityFragment& Vis = VisibilityList[i];

			AActor* OwnerActor = ActorFrag.GetMutable();
			if (!OwnerActor) continue;

			if (WeaponFrag.TargetISM.IsValid() && WeaponFrag.InstanceIndex != INDEX_NONE)
			{
				bool bVisible = Vis.bIsVisibleEnemy && Vis.bIsOnViewport;
				FTransform WeaponTransform;

				if (bVisible && !OwnerActor->IsHidden() && !bIsDead)
				{
					AUnitBase* Unit = Cast<AUnitBase>(OwnerActor);
					FTransform ParentTransform;
					if (Unit && Unit->bUseSkeletalMovement && Unit->GetMesh() && WeaponFrag.SocketName != NAME_None)
					{
						ParentTransform = Unit->GetMesh()->GetSocketTransform(WeaponFrag.SocketName);
					}
					else
					{
						ParentTransform = OwnerActor->GetActorTransform();
					}

					// Apply the offset: Offset * ParentTransform
					WeaponTransform = WeaponFrag.Offset * ParentTransform;
				}
				else
				{
					WeaponTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector);
				}

				BatchedUpdates.FindOrAdd(WeaponFrag.TargetISM.Get()).Add({WeaponFrag.InstanceIndex, WeaponTransform});
			}
		}
	}));

	for (auto& [ISM, Updates] : BatchedUpdates)
	{
		if (!ISM) continue;

		for (const FWeaponISMUpdate& U : Updates)
		{
			ISM->UpdateInstanceTransform(U.InstanceIndex, U.NewTransform, true, false, false);
		}
		ISM->MarkRenderDynamicDataDirty();
	}
}
