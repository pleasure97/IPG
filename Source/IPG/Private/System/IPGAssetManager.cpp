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