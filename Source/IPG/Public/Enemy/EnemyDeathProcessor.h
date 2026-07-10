// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "EnemyDeathProcessor.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API UEnemyDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UEnemyDeathProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery MassEntityQuery;
};
