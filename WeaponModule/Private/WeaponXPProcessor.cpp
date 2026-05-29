#include "WeaponXPProcessor.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "WeaponComponent.h"
#include "MassWeaponFragment.h"
#include "Mass/UnitMassTag.h"
#include "Characters/Unit/LevelUnit.h"
#include "WeaponAttributeSet.h"
#include "MassActorSubsystem.h" // For FMassActorFragment

UWeaponXPProcessor::UWeaponXPProcessor()
{
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
    
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true; // Required for accessing Actor components and LevelUnit logic
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
}

void UWeaponXPProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	UE_LOG(LogTemp, Warning, TEXT("[WeaponXPProcessor] Configuring Queries"));
	NewlyDeadQuery.Initialize(EntityManager);
	NewlyDeadQuery.AddTagRequirement<FMassStateDeadTag>(EMassFragmentPresence::All);
	NewlyDeadQuery.AddTagRequirement<FMassXPProcessedTag>(EMassFragmentPresence::None);
	NewlyDeadQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	NewlyDeadQuery.AddRequirement<FMassCombatStatsFragment>(EMassFragmentAccess::ReadOnly);
	NewlyDeadQuery.RegisterWithProcessor(*this);
	
	EarnerQuery.Initialize(EntityManager);
	EarnerQuery.AddTagRequirement<FMassStateDeadTag>(EMassFragmentPresence::None);
	EarnerQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EarnerQuery.AddRequirement<FMassCombatStatsFragment>(EMassFragmentAccess::ReadOnly);
	EarnerQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadWrite);
	EarnerQuery.AddRequirement<FMassWeaponFragment>(EMassFragmentAccess::ReadOnly); // Only units with WeaponComponent/Fragment
	EarnerQuery.RegisterWithProcessor(*this);
}

void UWeaponXPProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FVector> DeadLocations;
	TArray<int32> DeadTeams;
	TArray<FMassEntityHandle> DeadEntities;

	// 1. Collect all newly dead entities
	NewlyDeadQuery.ForEachEntityChunk(EntityManager, Context, ([&](FMassExecutionContext& ChunkContext)
	{
		const auto Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
		const auto Stats = ChunkContext.GetFragmentView<FMassCombatStatsFragment>();
		
		UE_LOG(LogTemp, Log, TEXT("[WeaponXPProcessor] Found chunk with %d dead entities"), ChunkContext.GetNumEntities());

		for (int i = 0; i < ChunkContext.GetNumEntities(); ++i)
		{
			DeadLocations.Add(Transforms[i].GetTransform().GetLocation());
			DeadTeams.Add(Stats[i].TeamId);
			DeadEntities.Add(ChunkContext.GetEntity(i));
		}
	}));

	if (DeadEntities.Num() == 0) return;

	UE_LOG(LogTemp, Warning, TEXT("[WeaponXPProcessor] Processing %d dead entities in this frame"), DeadEntities.Num());

	// 2. Distribute XP to nearby earner units (only those with WeaponComponent)
	EarnerQuery.ForEachEntityChunk(EntityManager, Context, ([&](FMassExecutionContext& ChunkContext)
	{
		const auto Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
		const auto Stats = ChunkContext.GetFragmentView<FMassCombatStatsFragment>();
		TArrayView<FMassActorFragment> Actors = ChunkContext.GetMutableFragmentView<FMassActorFragment>();

		UE_LOG(LogTemp, Log, TEXT("[WeaponXPProcessor] Checking chunk with %d potential earner units"), ChunkContext.GetNumEntities());

		for (int i = 0; i < ChunkContext.GetNumEntities(); ++i)
		{
			FVector EarnerLoc = Transforms[i].GetTransform().GetLocation();
			int32 EarnerTeam = Stats[i].TeamId;
			float SightRadius = Stats[i].SightRadius;

			for (int j = 0; j < DeadLocations.Num(); ++j)
			{
				// Check if enemy and within sight range
				if (EarnerTeam != DeadTeams[j] && FVector::DistSquared(EarnerLoc, DeadLocations[j]) <= FMath::Square(SightRadius))
				{
					if (AActor* EarnerActor = const_cast<AActor*>(Actors[i].Get()))
					{
						UE_LOG(LogTemp, Log, TEXT("[WeaponXPProcessor] Unit %s identified dead enemy in range. Granting XP."), *EarnerActor->GetName());

						if (ALevelUnit* LevelUnit = Cast<ALevelUnit>(EarnerActor))
						{
							int32 OldLevel = LevelUnit->LevelData.CharacterLevel;
							
							// Increase Experience
							LevelUnit->LevelData.Experience++;
							UE_LOG(LogTemp, Log, TEXT("[WeaponXPProcessor] Unit %s XP increased to %d"), *EarnerActor->GetName(), LevelUnit->LevelData.Experience);
							
							// Check for Level-Up
							if(LevelUnit->LevelData.Experience >= LevelUnit->LevelUpData.ExperiencePerLevel * LevelUnit->LevelData.CharacterLevel)
							{
								UE_LOG(LogTemp, Warning, TEXT("[WeaponXPProcessor] Unit %s is leveling up!"), *EarnerActor->GetName());
								LevelUnit->LevelUp();
								
								// If level increased, award additional Weapon Talent Points
								if (LevelUnit->LevelData.CharacterLevel > OldLevel)
								{
									UE_LOG(LogTemp, Warning, TEXT("[WeaponXPProcessor] Unit %s reached level %d. Awarding Weapon Talent Points."), *EarnerActor->GetName(), LevelUnit->LevelData.CharacterLevel);
									if (UWeaponComponent* WeaponComp = LevelUnit->FindComponentByClass<UWeaponComponent>())
									{
										if (WeaponComp->WeaponAttributes)
										{
											float NewPoints = WeaponComp->WeaponAttributes->GetWeaponTalentPoints() + WeaponComp->PointsPerLevel;
											WeaponComp->WeaponAttributes->SetAttributeWeaponTalentPoints(NewPoints);
											UE_LOG(LogTemp, Warning, TEXT("[WeaponXPProcessor] Unit %s now has %f Weapon Talent Points."), *EarnerActor->GetName(), NewPoints);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}));

	// 3. Mark dead entities as processed so they don't give XP again
	for (FMassEntityHandle Entity : DeadEntities)
	{
		Context.Defer().AddTag<FMassXPProcessedTag>(Entity);
	}
}
