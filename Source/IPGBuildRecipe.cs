// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.Collections.Generic;
using System.IO;
using EpicGames.Core;      
using UnrealBuildTool;

public static class IPGBuildRecipe
{
	private const string ArgPrefix = "-BuildRecipePackage=";
	private const string EnvVarName = "IPG_BUILD_RECIPE_PACKAGE";

	private static readonly string[] GameFeatureRoots = { "Plugins/GameFeatures" };

	public static void Apply(TargetRules Rules, TargetInfo Target)
	{
		string PackageName = ResolvePackageName();

		if (string.IsNullOrEmpty(PackageName))
		{
			Console.WriteLine("[IPG BuildRecipe] 패키지가 지정되지 않아 모든 게임 피처 플러그인을 빌드합니다.");
			return;
		}

		if (Target.ProjectFile == null)
		{
			throw new BuildException("[IPG BuildRecipe] 프로젝트 파일 없이 패키지를 지정할 수 없습니다.");
		}

		DirectoryReference ProjectDir = Target.ProjectFile.Directory;
		// Verify and load the manifest JSON file path
		FileReference ManifestFile = FileReference.Combine(
			ProjectDir, "Build", "BuildRecipe", PackageName + ".json");

		if (!FileReference.Exists(ManifestFile))
		{
			throw new BuildException(
				"[IPG BuildRecipe] 매니페스트를 찾을 수 없습니다: {0}\n" +
				"패키지 데이터 에셋을 저장해 JSON 을 생성하고 소스 컨트롤에 체크인했는지 확인하세요.",
				ManifestFile.FullName);
		}

		// Check if it is an editor build
		bool bIncludeEditorOnly = (Target.Type == TargetType.Editor);

		HashSet<string> AllowedPlugins = ReadAllowedPlugins(ManifestFile, bIncludeEditorOnly);

		int DisabledCount = 0;
		foreach (string PluginName in EnumerateGameFeaturePlugins(ProjectDir))
		{
			if (!AllowedPlugins.Contains(PluginName))
			{
				Rules.DisablePlugins.Add(PluginName);
				DisabledCount++;
			}
		}

		Console.WriteLine(
			"[IPG BuildRecipe] 패키지 '{0}' ({1}) — 허용 {2}개, 제외 {3}개",
			PackageName, Target.Type, AllowedPlugins.Count, DisabledCount);
	}

	private static string ResolvePackageName()
	{
		foreach (string Arg in Environment.GetCommandLineArgs())
		{
			if (Arg.StartsWith(ArgPrefix, StringComparison.OrdinalIgnoreCase))
			{
				return Arg.Substring(ArgPrefix.Length).Trim('"');
			}
		}

		return Environment.GetEnvironmentVariable(EnvVarName) ?? string.Empty;
	}

	private static HashSet<string> ReadAllowedPlugins(FileReference ManifestFile, bool bIncludeEditorOnly)
	{
		HashSet<string> Allowed = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

		JsonObject Manifest = JsonObject.Read(ManifestFile);

		JsonObject[] PluginObjects;
		if (!Manifest.TryGetObjectArrayField("Plugins", out PluginObjects))
		{
			throw new BuildException("[IPG BuildRecipe] The manifest is missing the 'Plugins' array: {0}",
				ManifestFile.FullName);
		}

		// Iterate through the JSON array and collect plugin names.
		foreach (JsonObject PluginObject in PluginObjects)
		{
			string Name = PluginObject.GetStringField("Name");

			bool bEditorOnly;
			if (!PluginObject.TryGetBoolField("EditorOnly", out bEditorOnly))
			{
				bEditorOnly = false;
			}

			if (bEditorOnly && !bIncludeEditorOnly)
			{
				continue;
			}

			Allowed.Add(Name);
		}

		return Allowed;
	}

	private static IEnumerable<string> EnumerateGameFeaturePlugins(DirectoryReference ProjectDir)
	{
		foreach (string RelativeRoot in GameFeatureRoots)
		{
			DirectoryReference Root = DirectoryReference.Combine(ProjectDir, RelativeRoot);
			if (!DirectoryReference.Exists(Root))
			{
				continue;
			}
			
			// Search the `Plugins/GameFeatures` folder 
			foreach (string UpluginPath in Directory.EnumerateFiles(
				Root.FullName, "*.uplugin", SearchOption.AllDirectories))
			{
				yield return Path.GetFileNameWithoutExtension(UpluginPath);
			}
		}
	}
}