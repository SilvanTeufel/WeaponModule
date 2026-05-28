// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FWeaponModuleModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void HandleAdditionalFragments(AActor* Owner, TArray<const UScriptStruct*>& FragmentsAndTags);
};
