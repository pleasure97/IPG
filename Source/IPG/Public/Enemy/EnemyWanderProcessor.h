// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "EnemyWanderProcessor.generated.h"

/**
 * A processor is a class that queries fragments and tags all entities and performs specific logic on matching entities. 
 */
UCLASS()
class IPG_API UEnemyWanderProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UEnemyWanderProcessor(); 

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery MassEntityQuery;
};
