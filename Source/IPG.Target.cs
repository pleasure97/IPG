// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;
using Microsoft.Extensions.Logging;
using System;
using System.Text.Json;
using EpicGames.Core;

public class IPGTarget : TargetRules
{
	public IPGTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("IPG");

		ApplyBuildRecipe();
	}

	// Applied only	during packaging build
	private void ApplyBuildRecipe()
	{
		if (Type == TargetType.Editor)
		{
			return;
		}

		string PackageName = GetPackageName();
		if (string.IsNullOrEmpty(PackageName))
		{
			Logger.LogInformation("[IPG BuildRecipe] Package not specified - Build all game feature plugins.");
			return;
		}

        // Check if the JSON recipe file for the specified package exists
        string BuildRecipePath = Path.Combine(ProjectFile.Directory.FullName, 
												"Build", "BuildRecipe", PackageName + ".json");

        // Abort the build if the JSON recipe does not exist
        if (!File.Exists(BuildRecipePath))
		{
			throw new BuildException("[IPG BuildRecipe] Cannot find the build recipe : {0}", BuildRecipePath); 
		}

		JsonObject BuildRecipe = JsonObject.Read(new FileReference(BuildRecipePath));
		var AllowedPlugins = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

        // Scan all game feature plugins (Plugins/GameFeatures/) within the project
        foreach (JsonObject PluginObject in BuildRecipe.GetObjectArrayField("Plugins"))
		{
			bool bEditorOnly;
			if (!PluginObject.TryGetBoolField("EditorOnly", out bEditorOnly))
			{
				bEditorOnly = false;
			}
			if (bEditorOnly)
			{
				continue;
			}
			AllowedPlugins.Add(PluginObject.GetStringField("Name"));
		}
		
		int NumDisabledPlugins = 0;
        // Completely exclude the plugin from compilation and packaging during the build process
        foreach (string GFPName in DiscoverAllGameFeaturePlugins())
		{
			if (!AllowedPlugins.Contains(GFPName))
			{
				DisablePlugins.Add(GFPName);
				++NumDisabledPlugins;
			}
		}
		Logger.LogInformation("[IPG BuildRecipe] Package '{0}' - Allowed {1}, Disabled {2}", 
			PackageName, AllowedPlugins.Count, NumDisabledPlugins);
	}

	private IEnumerable<string> DiscoverAllGameFeaturePlugins()
    {
        string GFPRoot = Path.Combine(ProjectFile.Directory.FullName, "Plugins", "GameFeatures");
        if (!Directory.Exists(GFPRoot))
        {
            yield break;
        }

        foreach (string Dir in Directory.GetDirectories(GFPRoot))
        {
            string Name = Path.GetFileName(Dir); 
            if (File.Exists(Path.Combine(Dir, Name + ".uplugin")))
            {
                yield return Name;
            }
        }
    }
	
	private static string GetPackageName()
	{
		const string PrefixString = "-BuildRecipePackage=";
        // [Priority 1] Parse the `-BuildRecipePackage=PackageName` argument included in the build command line
        foreach (string CommandLineArg in Environment.GetCommandLineArgs())
		{
			if (CommandLineArg.StartsWith(PrefixString, StringComparison.OrdinalIgnoreCase))
			{
				return CommandLineArg.Substring(PrefixString.Length).Trim('"');
			}
		}
        // [Priority 2] If no argument is provided, the value of the system environment variable (ACTIVE_IPG_PACKAGE) is read
        return Environment.GetEnvironmentVariable("ACTIVE_IPG_PACKAGE") ?? string.Empty;
	}
}
