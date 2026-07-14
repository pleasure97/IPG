// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IPGPreCMCTickComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/IPGMotionMatchingInterface.h"

UIPGPreCMCTickComponent::UIPGPreCMCTickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UIPGPreCMCTickComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		// Get character movement coponent from owner character
		if (UCharacterMovementComponent* CMC = OwnerCharacter->GetCharacterMovement())
		{
			// Ensure that PrimaryComponentTick() completes before the CMC's tick function executes.
			CMC->PrimaryComponentTick.AddPrerequisite(this, PrimaryComponentTick);
		}
	}
}

void UIPGPreCMCTickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	OnIPGPreCMCTickSignature.Broadcast();
}

/*
 * This function is called every tick, 
 * and is used to update the character movement components rotation mode using the Wants to Strafe input condition.
 * When the character is on the ground, this function sets the Rotation Rate to -1, 
 * which causes the character to rotate instantly. 
 * This technique allows us to treat the actor as the "target rotation", 
 * while we independently control the rotation of the root bone within the Animation Blueprint. 
 * This allows us to do things not currently supported in the character movement component, such as stick flicks 
 * (completely re-orienting the character when only tapping movement input), 
 * and gives us more control over the rotation behavior during actions like turn starts, pivots, and turning in place. 
 */
