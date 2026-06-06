// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/PawnInputComponent.h"

UPawnInputComponent::UPawnInputComponent()
{

}

UPawnInputComponent* UPawnInputComponent::FindPawnInputComponent(const AActor* Actor)
{
	return (Actor ? Actor->FindComponentByClass<UPawnInputComponent>() : nullptr);
}

void UPawnInputComponent::AddAdditionalInputConfig(const UIPGInputConfig* InputConfig)
{
}

void UPawnInputComponent::RemoveAdditionalInputConfig(const UIPGInputConfig* InputConfig)
{
}

void UPawnInputComponent::BeginPlay()
{
	Super::BeginPlay();
}
