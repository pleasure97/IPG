// Fill out your copyright notice in the Description page of Project Settings.


#include "System/IPGAssetManager.h"

const FName FIPGBundles::Equipped("Equipped");

UIPGAssetManager::UIPGAssetManager()
{
}

UIPGAssetManager& UIPGAssetManager::Get()
{
	check(GEngine);

	if (UIPGAssetManager* IPGAssetManager = Cast<UIPGAssetManager>(GEngine->AssetManager))
	{
		return *IPGAssetManager;
	}

	// Fatal error above prevents this from being called.
	return *NewObject<UIPGAssetManager>();
}

UObject* UIPGAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}

		// Use LoadObject if asset manager isn't ready yet
		return AssetPath.TryLoad();
	}
	return nullptr;
}

void UIPGAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetLock(&LoadedAssetsCriticalSection); 
		LoadedAssets.Add(Asset);
	}
}
