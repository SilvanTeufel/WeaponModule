// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWeaponModule, Log, All);

class AActor;
class UScriptStruct;

class FWeaponModuleModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void HandleAdditionalFragments(AActor* Owner, TArray<const UScriptStruct*>& FragmentsAndTags);
};
