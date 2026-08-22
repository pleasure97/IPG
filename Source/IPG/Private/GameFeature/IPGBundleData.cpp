// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGBundleData.h"
#include "GameFeature/IPGPackageData.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Interfaces/IPluginManager.h"
#include "UObject/ObjectSaveContext.h"
#include "AssetRegistry/AssetRegistryModule.h"

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

#if WITH_EDITOR
void UIPGBundleData::PreSave(FObjectPreSaveContext SaveContext)
{
	// This function called immediately before the asset is saved
	Super::PreSave(SaveContext);

	// No need to save the build recipe when cooking or procedural save
	if (SaveContext.IsCooking() || SaveContext.IsProceduralSave())
	{
		return;
	}

	// Search for packages in the asset registry
	const FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssetsByClass(
		UIPGPackageData::StaticClass()->GetClassPathName(), Assets, true);

	// Check and automatically export packages referencing the bundle
	for (const FAssetData& Asset : Assets)
	{
		if (const UIPGPackageData* IPGPackage = Cast<UIPGPackageData>(Asset.GetAsset()))
		{
			// Reexport only the packages that reference this bundle
			for (const FIPGPackageBundleEntry& BundleEntry : IPGPackage->Bundles)
			{
				if (BundleEntry.Bundle.Get() == this)
				{
					IPGPackage->ExportBuildRecipe();
					break;
				}
			}
		}
	}
}
#endif // WITH_EDITOR

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
