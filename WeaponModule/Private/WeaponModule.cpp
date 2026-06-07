// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponModule.h"
#include "GameFramework/Actor.h"
#include "Mass/MassActorBindingComponent.h"
#include "WeaponComponent.h"
#include "MassWeaponFragment.h"

#define LOCTEXT_NAMESPACE "FWeaponModuleModule"

void FWeaponModuleModule::StartupModule()
{
	UMassActorBindingComponent::OnMassArchetypeBuilding.AddStatic(&FWeaponModuleModule::HandleAdditionalFragments);
}

void FWeaponModuleModule::ShutdownModule()
{
}

void FWeaponModuleModule::HandleAdditionalFragments(AActor* Owner, TArray<const UScriptStruct*>& FragmentsAndTags)
{
	if (Owner && Owner->GetComponentByClass(UWeaponComponent::StaticClass()))
	{
		FragmentsAndTags.Add(FMassWeaponFragment::StaticStruct());
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWeaponModuleModule, WeaponModule)