// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Enemy/MassEntityEnemyType.h"
#include "MassEntityEnemySettings.generated.h"

class UMassEntityConfigAsset;
class UEnvQuery;

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName = "Mass Entity Enemy Settings"))
class IPG_API UMassEntityEnemySettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Config, meta = (ClampMin="1", ClampMax="128"))
	int32 MaxSpawnPerTick = 64;
	
	UPROPERTY(EditDefaultsOnly, Config)
	TMap<EEnemyType, TSoftObjectPtr<UMassEntityConfigAsset>> EntityConfigs;
	
	UPROPERTY(EditDefaultsOnly, Config)
	TSoftObjectPtr<UEnvQuery> SpawnPointQuery;

	UPROPERTY(EditDefaultsOnly, Config)
	TArray<TSoftObjectPtr<UWorld>> SpawnableWorlds;

	UPROPERTY(EditDefaultsOnly, Config)
	float GridSpacing = 200.f;
};
