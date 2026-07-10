// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyDeathProcessor.h"
#include "Enemy/EnemyMassFragment.h"
#include "MassRepresentationFragments.h"

UEnemyDeathProcessor::UEnemyDeathProcessor()
	: MassEntityQuery(*this)
{
	// Set the flag that determines where the processor will run 
	// Because this flag uses a bitmask, writing like this makes it run everywhere except the client
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::All & ~EProcessorExecutionFlags::Client);
}

void UEnemyDeathProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	// Death processor must be able to read and write the death fragment
	MassEntityQuery.AddRequirement<FEnemyDeathFragment>(EMassFragmentAccess::ReadWrite); 

	// As long as the entity is alive, the visualization will keep spawning an actor when it's close to the player 
	// If you kill an enemy and an entity that should disapper after 10 seconds come back, 
	// it might spawn a perfectly normal actor again
	MassEntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly); 
}

void UEnemyDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	MassEntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Ctx)
		{
			// Because 'ReadOnly' fragment cannot be modified, you have to access it as an array instead of mutable
			const auto Deaths = Ctx.GetMutableFragmentView<FEnemyDeathFragment>(); 
			// Representation fragment indicates how the entity is currently being represented
			const auto Representations = Ctx.GetFragmentView<FMassRepresentationFragment>();

			// Iterate through the fragments
			for (int32 i = 0; i < Ctx.GetNumEntities(); ++i)
			{
				// Check whether the entity is currently representing an actor
				const bool bAlreadySpawnedActor = 
					(Representations[i].CurrentRepresentation == EMassRepresentationType::HighResSpawnedActor) ||
					(Representations[i].CurrentRepresentation == EMassRepresentationType::LowResSpawnedActor); 

				// If an already dead entity no longer represents an actor, 
				// we'll request that the entity be destroyed
				if (!bAlreadySpawnedActor)
				{
					// These kinds of requests can be made as deferred requests, 
					// allowing mass to execute them asynchronously at a safe time 
					Ctx.Defer().DestroyEntity(Ctx.GetEntity(i)); 
					continue;
				}

				// We'll make the entity destroy itself when the specified time to live expires 
				Deaths[i].TimeToLive -= Ctx.GetDeltaTimeSeconds(); 
				if (Deaths[i].TimeToLive <= 0.f)
				{
					Ctx.Defer().DestroyEntity(Ctx.GetEntity(i)); 
				}
			}
		});
}


