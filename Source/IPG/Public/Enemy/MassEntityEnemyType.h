// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityEnemyType.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Humanoid UMETA(DisplayName = "Humanoid"),
	Animal	UMETA(DisplayName = "Animal"),
	Random UMETA(DisplayName = "Random")
};

UENUM(BlueprintType)
enum class EEnemySpawnType : uint8
{
	Location UMETA(DisplayName = "Location"),
	EnvironmentQuerySystem	UMETA(DisplayName = "EQS")
};

USTRUCT()
struct FEnemySpawnRequest
{
	GENERATED_BODY()

public:
	EEnemyType EnemyType;
	int32 Remaining = 0; 
	TArray<FVector> SpawnLocations; 

	bool IsValid() const;
};