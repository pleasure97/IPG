// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/ItemComponent.h"
#include "Net/UnrealNetwork.h"

UItemComponent::UItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PickupMessage = FString("Press E to Pickup!"); 
	SetIsReplicatedByDefault(true);
}

void UItemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); 

	// Iris Push Model
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UItemComponent, ItemManifest, Params);
}

void UItemComponent::InitItemManifest(FItemManifest CopiedManifest)
{
	ItemManifest = CopiedManifest;
}

void UItemComponent::PickedUp()
{
	OnPickedUp();
	GetOwner()->Destroy();
}
