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
			"Slate", 
			"NNE", 
			"NetCore",
            "GameplayTags",
            "GameplayTasks",
            "GameplayAbilities",            
			"AnimGraphRuntime",
            "ModularGameplay",
			"IPGTaskSystem", 
			"IPGInventory"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
            "AutomationController", 
			"Slate", 
			"SlateCore",
        });

		PublicIncludePaths.AddRange(new string[] {
			"IPG",
			"IPG/Public/Variant_Platforming",
            "IPG/Public/Variant_Platforming/Animation",
            "IPG/Public/Variant_Combat",
            "IPG/Public/Variant_Combat/AI",
            "IPG/Public/Variant_Combat/Animation",
            "IPG/Public/Variant_Combat/Gameplay",
            "IPG/Public/Variant_Combat/Interfaces",
            "IPG/Public/Variant_Combat/UI",
            "IPG/Public/Variant_SideScrolling",
            "IPG/Public/Variant_SideScrolling/AI",
            "IPG/Public/Variant_SideScrolling/Gameplay",
            "IPG/Public/Variant_SideScrolling/Interfaces",
            "IPG/Public/Variant_SideScrolling/UI"
        });

		SetupIrisSupport(Target);
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
