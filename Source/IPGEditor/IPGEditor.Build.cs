// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class IPGEditor : ModuleRules
{
	public IPGEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"IPG",                  
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"InputCore",
			"UnrealEd",
			"ToolMenus",            
			"EditorSubsystem",      
			"LevelEditor",
			"Projects",             
			"AssetRegistry",
			"GameFeatures",
			"DeveloperSettings",
			"SourceControl",
		});
	}
}