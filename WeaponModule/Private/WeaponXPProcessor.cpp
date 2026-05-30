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
	EarnerQuery.AddRequirement<FMassAllianceFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::All);
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
		
		for (int i = 0; i < ChunkContext.GetNumEntities(); ++i)
		{
			DeadLocations.Add(Transforms[i].GetTransform().GetLocation());
			DeadTeams.Add(Stats[i].TeamId);
			DeadEntities.Add(ChunkContext.GetEntity(i));
		}
	}));

	if (DeadEntities.Num() == 0) return;

	// 2. Distribute XP to nearby earner units (only those with WeaponComponent)
	EarnerQuery.ForEachEntityChunk(EntityManager, Context, ([&](FMassExecutionContext& ChunkContext)
	{
		const auto Transforms = ChunkContext.GetFragmentView<FTransformFragment>();
		const auto Stats = ChunkContext.GetFragmentView<FMassCombatStatsFragment>();
		const auto Alliances = ChunkContext.GetFragmentView<FMassAllianceFragment>();
		TArrayView<FMassActorFragment> Actors = ChunkContext.GetMutableFragmentView<FMassActorFragment>();

		for (int i = 0; i < ChunkContext.GetNumEntities(); ++i)
		{
			FVector EarnerLoc = Transforms[i].GetTransform().GetLocation();
			int32 EarnerTeam = Stats[i].TeamId;
			float SightRadius = Stats[i].SightRadius;

			for (int j = 0; j < DeadLocations.Num(); ++j)
			{
				// Check if enemy, not allied, and within sight range
				bool bIsAllied = (Alliances.Num() > 0 && (Alliances[i].AlliedTeamsMask & (1LL << DeadTeams[j])));
				if (EarnerTeam != DeadTeams[j] && !bIsAllied && FVector::DistSquared(EarnerLoc, DeadLocations[j]) <= FMath::Square(SightRadius))
				{
					if (AActor* EarnerActor = const_cast<AActor*>(Actors[i].Get()))
					{
						if (ALevelUnit* LevelUnit = Cast<ALevelUnit>(EarnerActor))
						{
							int32 OldLevel = LevelUnit->LevelData.CharacterLevel;
							
							// Increase Experience
							LevelUnit->LevelData.Experience++;
							
							// Check for Level-Up
							if(LevelUnit->LevelData.Experience >= LevelUnit->LevelUpData.ExperiencePerLevel * LevelUnit->LevelData.CharacterLevel)
							{
								LevelUnit->LevelUp();
								
								// If level increased, award additional Weapon Talent Points
								if (LevelUnit->LevelData.CharacterLevel > OldLevel)
								{
 								if (UWeaponComponent* WeaponComp = LevelUnit->FindComponentByClass<UWeaponComponent>())
 								{
 									// Alle verfügbaren Waffen erhalten Talentpunkte
 									for (FWeaponData& Weapon : WeaponComp->AvailableWeapons)
 									{
 										Weapon.WeaponTalentPoints += WeaponComp->PointsPerLevel;

										if (LevelUnit->LevelData.CharacterLevel % 5 == 0)
										{
											Weapon.EffectTalentPoints += 1.0f;
										}
 									}
									
 									// Attribute für die aktuell ausgerüstete Waffe synchronisieren
 									WeaponComp->SyncAttributesFromWeapon(WeaponComp->CurrentWeaponIndex);
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
