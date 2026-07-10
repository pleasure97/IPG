// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyWanderProcessor.h"
#include "Enemy/EnemyMassFragment.h"
#include "MassCommonFragments.h"
#include "MassLODFragments.h"
#include "NavigationSystem.h"

UEnemyWanderProcessor::UEnemyWanderProcessor()
	: MassEntityQuery(*this)
{
	// Set the flag that determines where the processor will run 
	// Because this flag uses a bitmask, writing like this makes it run everywhere except the client
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::All & ~EProcessorExecutionFlags::Client); 
}

void UEnemyWanderProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Set the query conditions
	// For the entity to wander, it needs to modify its transform, so it should have a transform fragment
	MassEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	MassEntityQuery.AddRequirement<FEnemyWanderFragment>(EMassFragmentAccess::ReadWrite);

	// We won't read or modify any death fragment, avoiding access when possible has no performance overhead
	// If we set presence to None, it will search only for entities that do not have a death fragment
	MassEntityQuery.AddRequirement<FEnemyDeathFragment>(EMassFragmentAccess::None, EMassFragmentPresence::None);

	// A processor can also query by tag
	// When the entity is in the high LOD state, close to the player,
	// we want the actor to take control and the entity not to move, so we'll filter out that tag
	MassEntityQuery.AddTagRequirement<FMassHighLODTag>(EMassFragmentPresence::None);

	// We also want enemies to use only valid navigation paths 
	// But if the entity is too far away, world parition unloads, and navigation becomes invalid 
	// For this reason, we'll also filter out very distant OFF LOD entities 
	MassEntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
}

void UEnemyWanderProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Implement what to execute for entities that meet conditions
	// 'ForEachEntityChunk' iterates through the searched entities by archetype 
	// It will invoke the callback once per archetype, 
	// but in the end, it still runs once for every entity 
	MassEntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Ctx)
		{
			// Access the fragment data we need from the entity 
			const auto Transforms = Ctx.GetMutableFragmentView<FTransformFragment>();
			const auto Wanders = Ctx.GetMutableFragmentView<FEnemyWanderFragment>(); 

			// Iterate through the chunk and access each entity's fragment 
			for (int32 i = 0; i < Ctx.GetNumEntities(); ++i)
			{
				FTransform& Transform = Transforms[i].GetMutableTransform();
				FEnemyWanderFragment& EnemyWanderFragment = Wanders[i];

				// Calculate the time using the context's delta time
				EnemyWanderFragment.TimeUntilNewTarget -= Ctx.GetDeltaTimeSeconds(); 

				// If the time is up, we'll find a new reachable location through the navigation system 
				if (auto* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Ctx.GetWorld()))
				{
					if (EnemyWanderFragment.TimeUntilNewTarget <= 0.f)
					{
						// TODO - Literal
						EnemyWanderFragment.TimeUntilNewTarget = FMath::FRandRange(3.f, 6.f); 

						FNavLocation NavResult;
						if (NavigationSystem->GetRandomReachablePointInRadius(EnemyWanderFragment.Origin, EnemyWanderFragment.Radius, NavResult))
						{
							EnemyWanderFragment.TargetLocation = NavResult.Location;
						}
					}
				}

				// Move to the target location 
				const FVector CurrentLocation = Transform.GetLocation();
				const FVector Delta2D(
					EnemyWanderFragment.TargetLocation.X - CurrentLocation.X,
					EnemyWanderFragment.TargetLocation.Y - CurrentLocation.Y,
					0.f);

				// Calculate the squared distance and step size to avoid the square root operation
				const float DistSquared = Delta2D.SizeSquared(); 
				const float StepSize = EnemyWanderFragment.Speed * Ctx.GetDeltaTimeSeconds(); 

				// If the remaining distance is smaller than the step size, the entity doesn't need to move
				if (DistSquared < FMath::Square(StepSize))
				{
					continue;
				}

				// We'll get the normal through inverse square root
				// This lets us get the direction without performing a square root operation
				const float InverseDist = FMath::InvSqrt(DistSquared);
				const FVector Direction = Delta2D * InverseDist;

				// Calculate the next position and modify the transform fragment values
				FVector NewLocation = CurrentLocation + Direction * StepSize;
				Transform.SetLocation(NewLocation); 
				Transform.SetRotation(Direction.ToOrientationQuat()); 

			}

		});
}
