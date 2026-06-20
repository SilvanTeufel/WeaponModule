// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "WeaponModule.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY(LogWeaponModule);

#include "Mass/MassActorBindingComponent.h"
#include "Components/WeaponComponent.h"
#include "Mass/MassWeaponFragment.h"

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