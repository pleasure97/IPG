// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enemy/MassEntityEnemyType.h"
#include "MassEntityTemplate.h"
#include "EnemyMassSubsystem.generated.h"

class UEnvQuery;
class UMassSpawnerSubsystem;
class UNavigationSystemV1;
struct FEnvQueryResult;
struct FMassEntityManager;

/**
 * 
 */
UCLASS()
class IPG_API UEnemyMassSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void RequestSpawnEnemyEQS(int32 Count, EEnemyType Type = EEnemyType::Random);

	UFUNCTION(BlueprintCallable)
	void RequestSpawnEnemyLocation(int32 Count, const FVector& Location, EEnemyType Type = EEnemyType::Random);

protected:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override; 
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void EnqueueSpawnRequest(FEnemySpawnRequest&& Request); 
	void ProcessSpawnRequest();

	void SetMassLocation(const FMassEntityHandle& MassEntity, const FVector& Location);

	void QueryAllEQS(const TFunction<void(const TArray<TSharedPtr<FEnvQueryResult>>& Results)>& Callback);
	TArray<FVector> GetGridLocations(const FVector& Center, int32 Count);

	TArray<FVector> GetWeightedRandomLocations(const FEnvQueryResult& EnvQueryResult, int32 Count);

	UPROPERTY()
	TArray<AActor*> EQSPawns;

	UPROPERTY()
	TObjectPtr<UEnvQuery> EQS;

	UPROPERTY()
	TWeakObjectPtr<UMassSpawnerSubsystem> MassSpawnerSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UNavigationSystemV1> NavigationSystem;

	FMassEntityManager* MassEntityManager = nullptr;

	TMap<EEnemyType, FMassEntityTemplate> EntityTemplateMap;

	TArray<EEnemyType> EnemyTypes;

	// TODO - Thread
	TQueue<FEnemySpawnRequest> EnemySpawnQueue; 

	FTimerHandle EnemySpawnTimerHandle;
};
