// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class IPG : ModuleRules
{
	public IPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"IPG",
			"IPG/Variant_Platforming",
			"IPG/Variant_Platforming/Animation",
			"IPG/Variant_Combat",
			"IPG/Variant_Combat/AI",
			"IPG/Variant_Combat/Animation",
			"IPG/Variant_Combat/Gameplay",
			"IPG/Variant_Combat/Interfaces",
			"IPG/Variant_Combat/UI",
			"IPG/Variant_SideScrolling",
			"IPG/Variant_SideScrolling/AI",
			"IPG/Variant_SideScrolling/Gameplay",
			"IPG/Variant_SideScrolling/Interfaces",
			"IPG/Variant_SideScrolling/UI"
		});

		SetupIrisSupport(Target);
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
