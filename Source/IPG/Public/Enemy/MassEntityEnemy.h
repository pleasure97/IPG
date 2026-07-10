// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Combat/AI/CombatEnemy.h"
#include "MassEntityEnemy.generated.h"

class UMassAgentComponent;

UCLASS()
class IPG_API AMassEntityEnemy : public ACombatEnemy
{
	GENERATED_BODY()

public:
	AMassEntityEnemy();

	UFUNCTION(BlueprintCallable)
	void KillEnemy(float TimeToLive); 

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnMassActorActivated();

	UFUNCTION(BlueprintImplementableEvent)
	void OnMassActorDeactivated();

	UFUNCTION(BlueprintNativeEvent)
	void SetHealthPercent(float Percent);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	float GetHealthPercent() const;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMassAgentComponent> MassAgentComponent;

private:
	void OnEntityAssociated(const UMassAgentComponent& InMassAgentComponent);
	void OnEntityDetaching(const UMassAgentComponent& InMassAgentComponent);

	// Syncs the state between the entity and the actor
	void SyncMassToActor(); 
	void SyncActorToMass(); 
};
