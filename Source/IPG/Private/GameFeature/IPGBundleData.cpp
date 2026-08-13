// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGBundleData.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Interfaces/IPluginManager.h"

TArray<FString> UIPGBundleData::GetPluginNames() const
{
	TArray<FString> PluginNames = BundlePluginNames;

	if (!PluginNameFilter.IsEmpty())
	{
		const FRegexPattern RegexPattern(PluginNameFilter);

		for (const FString& PluginName : GetAllInstalledGameFeaturePluginNames())
		{
			FRegexMatcher RegexMatcher(RegexPattern, PluginName);
			if (RegexMatcher.FindNext())
			{
				PluginNames.AddUnique(PluginName);
			}
		}
	}

	return PluginNames;
}

FPrimaryAssetId UIPGBundleData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("IPGBundleData"), GetFName());
}

TArray<FString> UIPGBundleData::GetAllInstalledGameFeaturePluginNames()
{
	TArray<FString> AllGameFeaturePluginNames;

	const UGameFeaturesSubsystemSettings* Settings = GetDefault<UGameFeaturesSubsystemSettings>();

	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
	{
		const FString DescriptorFilename = FPaths::ConvertRelativePathToFull(Plugin->GetDescriptorFileName());
		if (Settings->IsValidGameFeaturePlugin(DescriptorFilename))
		{
			AllGameFeaturePluginNames.Add(Plugin->GetName());
		}
	}
	
	return AllGameFeaturePluginNames;
}
