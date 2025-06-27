// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WorldGridStreamEditor : ModuleRules
{
	public WorldGridStreamEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"CoreUObject",
				"Projects",
				"Engine",
				// ... add other public dependencies that you statically link with here ...
			}
		);
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"Landscape",
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
