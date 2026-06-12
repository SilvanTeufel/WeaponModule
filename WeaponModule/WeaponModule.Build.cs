// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

using UnrealBuildTool;

public class WeaponModule : ModuleRules
{
	public WeaponModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"RTSUnitTemplate",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"MassEntity",
				"MassCommon",
				"MassSignals",
				"MassRepresentation",
				"MassSpawner",
				"NetCore",
				"UMG",
				"MassActors",
				"Json",
				"JsonUtilities",
				"DeveloperSettings"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"StructUtils"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
