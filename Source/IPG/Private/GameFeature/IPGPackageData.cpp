// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGPackageData.h"
#include "GameFeature/IPGBundleData.h"
#include "UObject/ObjectSaveContext.h"

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

void UIPGPackageData::ExportBuildRecipe() const
{
	// Check if package name is valid
	if (PackageName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[IPG Package] PackageName is empty, skipping manifest export"));
		return;
	}


	// Create JSON root object
	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("PackageName"), PackageName);

	TArray<TSharedPtr<FJsonValue>> PluginArray;
	TSet<FString> AddedPluginNames;

	// Iterate bundles
	for (const FIPGPackageBundleEntry& BundleEntry : Bundles)
	{
		const UIPGBundleData* IPGBundle = BundleEntry.Bundle.LoadSynchronous();
		if (!IsValid(IPGBundle))
		{
			continue;
		}

		// Get bundle plugin names 
		TArray<FString> BundlePluginNames = IPGBundle->GetPluginNames();
		BundlePluginNames.Sort();

		// Remove dupliate plugins
		for (const FString& BundlePluginName : BundlePluginNames)
		{
			if (BundlePluginName.IsEmpty() || AddedPluginNames.Contains(BundlePluginName))
			{
				continue;
			}
			AddedPluginNames.Add(BundlePluginName);

			TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
			Obj->SetStringField(TEXT("Name"), BundlePluginName);
			Obj->SetStringField(TEXT("Bundle"), IPGBundle->BundleName);
			Obj->SetStringField(TEXT("Chunk"),
				BundleEntry.ChunkName.IsNone() ? FString() : BundleEntry.ChunkName.ToString());
			Obj->SetBoolField(TEXT("EditorOnly"), BundleEntry.bEditorOnly);
			Obj->SetBoolField(TEXT("Writable"), BundleEntry.bWritable);

			PluginArray.Add(MakeShared<FJsonValueObject>(Obj));
		}
	}

	Root->SetArrayField(TEXT("Plugins"), PluginArray);

	// Serialization
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

	// Make build recipe directory
	const FString OutputDir = FPaths::ProjectDir() / TEXT("Build") / TEXT("BuildRecipe");
	IFileManager::Get().MakeDirectory(*OutputDir, /*Tree=*/true);

	// Save the build recipe JSON file to the directory
	const FString FilePath = OutputDir / (PackageName + TEXT(".json"));
	if (FFileHelper::SaveStringToFile(OutputString, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
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

#if WITH_EDITOR
void UIPGPackageData::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	if (SaveContext.IsCooking() || SaveContext.IsProceduralSave())
	{
		return;
	}

	ExportBuildRecipe();
}
#endif