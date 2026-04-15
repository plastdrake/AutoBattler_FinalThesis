// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AutoBattler : ModuleRules
{
	public AutoBattler(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
          "MassEntity",
			"MassCommon",
          "MassMovement",
			"MassNavigation",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AutoBattler",
          "AutoBattler/Variant_BattleSim",
			"AutoBattler/Variant_BattleSim/AI",
			"AutoBattler/Variant_ECS",
			"AutoBattler/Variant_ECS/AI",
			"AutoBattler/Variant_Platforming",
			"AutoBattler/Variant_Platforming/Animation",
			"AutoBattler/Variant_Combat",
			"AutoBattler/Variant_Combat/AI",
			"AutoBattler/Variant_Combat/Animation",
			"AutoBattler/Variant_Combat/Gameplay",
			"AutoBattler/Variant_Combat/Interfaces",
			"AutoBattler/Variant_Combat/UI",
			"AutoBattler/Variant_SideScrolling",
			"AutoBattler/Variant_SideScrolling/AI",
			"AutoBattler/Variant_SideScrolling/Gameplay",
			"AutoBattler/Variant_SideScrolling/Interfaces",
			"AutoBattler/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
