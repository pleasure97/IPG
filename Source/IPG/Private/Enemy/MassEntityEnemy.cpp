// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MassEntityEnemy.h"
#include "Enemy/EnemyMassFragment.h"
#include "MassAgentComponent.h"
#include "MassAgentSubsystem.h"
#include "MassEntitySubsystem.h"

AMassEntityEnemy::AMassEntityEnemy()
{
	MassAgentComponent = CreateDefaultSubobject<UMassAgentComponent>(TEXT("MassAgentComponent")); 
}

void AMassEntityEnemy::KillEnemy(float TimeToLive)
{
	// Get mass entity subsystem
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>(); 
	if (!IsValid(EntitySubsystem))
	{
		return;
	}

	// Get mass entity manager
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager(); 

	// Add the death fragment to the entity when an enemy dies
	// As entities are grouped by archetype, and that's what makes ECS efficient
	// Because of this mechanism, changing an entity's fragment leads to an archetype change
	// Then the entity moves to a different archetype and causes a copy 
	// In a case like death, which doesn't happen often, that's fine, 
	// but, in general, you need to be careful when changing fragments
	FEnemyDeathFragment EnemyDeathFragment; 
	EnemyDeathFragment.TimeToLive = TimeToLive;

	// Add the fragment using deferred command approach
	EntityManager.Defer().PushCommand<FMassCommandAddFragmentInstances>(
		MassAgentComponent->GetEntityHandle(), EnemyDeathFragment
	);
}

void AMassEntityEnemy::SetHealthPercent_Implementation(float Percent)
{
}

float AMassEntityEnemy::GetHealthPercent_Implementation() const
{
	return 1.f;
}

void AMassEntityEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Using a mass agent lets us register delegates for when an entity represents or hides an actor
	if (MassAgentComponent)
	{
		if (UMassAgentSubsystem* MassAgentSubsystem = GetWorld()->GetSubsystem<UMassAgentSubsystem>())
		{
			// Often, the activation callback can be called eariler than BeginPlay()
			// To prevent missing the callback, we'll call the callback directly 
			if (MassAgentComponent->GetEntityHandle().IsValid())
			{
				OnEntityAssociated(*MassAgentComponent);
			}

			MassAgentSubsystem->GetOnMassAgentComponentEntityAssociated().AddUObject(
				this, &AMassEntityEnemy::OnEntityAssociated); 

			MassAgentSubsystem->GetOnMassAgentComponentEntityDetaching().AddUObject(
				this, &AMassEntityEnemy::OnEntityDetaching);
		}
	}
	
}

void AMassEntityEnemy::OnEntityAssociated(const UMassAgentComponent& InMassAgentComponent)
{
	// The callback triggered here is broadcasted from mass entity subsystem, 
	// so it always fires for every character 
	// So we need to filter whether the parameter is this character's mass agent component 
	if (!IsValid(MassAgentComponent) || (&InMassAgentComponent != MassAgentComponent))
	{
		return;
	}

	SyncMassToActor(); 
	OnMassActorActivated();
}

void AMassEntityEnemy::OnEntityDetaching(const UMassAgentComponent& InMassAgentComponent)
{
	// The callback triggered here is broadcasted from mass entity subsystem, 
	// so it always fires for every character 
	// So we need to filter whether the parameter is this character's mass agent component 
	if (!IsValid(MassAgentComponent) || (&InMassAgentComponent != MassAgentComponent))
	{
		return;
	}

	SyncActorToMass();
	OnMassActorDeactivated();
}

// TODO - Could initialize the attribute set or trigger animations or effects
void AMassEntityEnemy::SyncMassToActor()
{
	// Get mass entity handle
	const FMassEntityHandle EntityHandle = MassAgentComponent->GetEntityHandle(); 
	if (!EntityHandle.IsValid())
	{
		return;
	}

	// Get mass entity subsytsem
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>(); 
	if (!IsValid(EntitySubsystem))
	{
		return;
	}

	const FMassEntityManager& EntityManger = EntitySubsystem->GetEntityManager();

	// Access the status fragment through the mass entity manager
	// The target is entity linked to the mass agent
	if (FEnemyStatusFragment* EnemyStatusFragment = EntityManger.GetFragmentDataPtr<FEnemyStatusFragment>(EntityHandle))
	{
		// TODO 
	}
}

void AMassEntityEnemy::SyncActorToMass()
{
	// Get mass entity handle
	const FMassEntityHandle EntityHandle = MassAgentComponent->GetEntityHandle();
	if (!EntityHandle.IsValid())
	{
		return;
	}

	// Get mass entity subsytsem
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!IsValid(EntitySubsystem))
	{
		return;
	}

	const FMassEntityManager& EntityManger = EntitySubsystem->GetEntityManager();

	// Access the status fragment through the mass entity manager
	// The target is entity linked to the mass agent
	if (FEnemyStatusFragment* EnemyStatusFragment = EntityManger.GetFragmentDataPtr<FEnemyStatusFragment>(EntityHandle))
	{
		EnemyStatusFragment->HealthPercent = GetHealthPercent(); 
	}
}
