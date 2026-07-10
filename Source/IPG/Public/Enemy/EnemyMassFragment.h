// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityElementTypes.h"
#include "EnemyMassFragment.generated.h"

/*
 * As fragment is a value each entity has, the number of entity you use is huge, 
   then memory usage is determined by the fragment
 */

// Fragment that enemy search for surroundings
USTRUCT()
struct FEnemyWanderFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	// Register UPROPERTY to be detected by 'Mass Debugger'
	UPROPERTY(EditAnywhere)
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	float TimeUntilNewTarget = 0.f;

	UPROPERTY(EditAnywhere)
	float Radius = 1000.f;

	UPROPERTY(EditAnywhere)
	float Speed = 300.f;
};

// Through visualization, MassEntity can spawn an actor when it is close and destroy it when it's far away
// This approach is good for optimization as LOD, but it's not suitable for the method we will use 
// That is because the actor state's not preserved 
// If you half-kill an enemy and come back, and its HP is 100%, that's definitely wrong 
// So state data like this should be stored in a fragment that shares the same lifecycle as the entity 
USTRUCT()
struct FEnemyStatusFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	// TODO 
	UPROPERTY(EditAnywhere)
	float HealthPercent = 1.f;
};

// The reason for separating it from the status fragment is that 
// only dead entities will be split into a separate archetype and handled by a processor that performs dedicated logic
// An archetype is a unit that groups entities with the same fragment combination 
USTRUCT()
struct FEnemyDeathFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float TimeToLive = 5.f;
};