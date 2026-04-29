// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "IPGAssetManager.generated.h"

struct FIPGBundles
{
	static const FName Equipped;
};

/**
 *	Game implementation of the asset manager that overrides functionality and stores game-specific types.
 *	It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic.
 *	This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.
 */
UCLASS(Config=Game)
class IPG_API UIPGAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	UIPGAssetManager(); 

	// Returns the AssetManager singleton object.
	static UIPGAssetManager& Get();
};
