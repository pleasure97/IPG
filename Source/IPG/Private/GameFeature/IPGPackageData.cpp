// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGPackageData.h"
#include "GameFeature/IPGBundleData.h"

TArray<FString> UIPGPackageData::GetAllPluginNames(bool bIncludeEditorOnly) const
{
	TArray<FString> AllPluginNames;

	// Iterate bundle entry
	for (const FIPGPackageBundleEntry& BundleEntry : Bundles)
	{
		// Check if the bundle is "Editor-Only"
		if (BundleEntry.bEditorOnly && !bIncludeEditorOnly)
		{
			continue;
		}
		
		// Add bundle plugin names
		if (const UIPGBundleData* BundleAsset = BundleEntry.Bundle.LoadSynchronous())
		{
			AllPluginNames.Append(BundleAsset->GetPluginNames());
		}
	}

	TArray<FString> UniquePluginNames;
	for (const FString& PluginName : AllPluginNames)
	{
		UniquePluginNames.AddUnique(PluginName);
	}
	return UniquePluginNames;
}

TArray<FString> UIPGPackageData::GetValidChunkNames() const
{
	TArray<FString> ValidChunkNames;

	ValidChunkNames.Reserve(Chunks.Num()); 

	for (const FName& ChunkNameEntry : Chunks)
	{
		ValidChunkNames.Add(ChunkNameEntry.ToString());
	}

	return ValidChunkNames;
}

void UIPGPackageData::ExportManifest() const
{
	if (PackageName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[IPG Package] PackageName is empty, skipping manifest export"));
		return;
	}

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("PackageName"), PackageName);

	TArray<TSharedPtr<FJsonValue>> PluginArray;
	// Exclude EditorOnly bundle
	TArray<FString> PluginNames = GetAllPluginNames(/*bIncludeEditorOnly=*/false);
	// Sort alphabetically
	PluginNames.Sort(); 
	for (const FString& PluginName : PluginNames)
	{
		PluginArray.Add(MakeShared<FJsonValueString>(PluginName));
	}
	Root->SetArrayField(TEXT("Plugins"), PluginArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	const FString OutputDir = FPaths::ProjectConfigDir() / TEXT("Patches");
	IFileManager::Get().MakeDirectory(*OutputDir, /*Tree=*/true);

	const FString FilePath = OutputDir / (PackageName + TEXT(".json"));
	if (FFileHelper::SaveStringToFile(OutputString, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("[IPG Package] Exported manifest: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[IPG Package] Failed to write manifest: %s"), *FilePath);
	}
}

FPrimaryAssetId UIPGPackageData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("IPGPackageData"), GetFName());
}
