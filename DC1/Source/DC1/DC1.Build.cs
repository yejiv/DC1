// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DC1 : ModuleRules
{
	public DC1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[]
		{
            "DC1",
		});

        PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",

            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "UMG",

            "AIModule",
            "StateTreeModule",
            "GameplayStateTreeModule",
		});

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
