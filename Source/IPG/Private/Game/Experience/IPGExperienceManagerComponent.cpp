// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Experience/IPGExperienceManagerComponent.h"
#include "Net/UnrealNetwork.h"

UIPGExperienceManagerComponent::UIPGExperienceManagerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UIPGExperienceManagerComponent::CallOrRegisterExperienceLoadedCallback(FOnIPGExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{

	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate)); 
	}
}

bool UIPGExperienceManagerComponent::IsExperienceLoaded() const
{
	return (LoadState == EIPGExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

void UIPGExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
}

void UIPGExperienceManagerComponent::OnRep_CurrentExperience()
{
}

void UIPGExperienceManagerComponent::StartExperienceLoad()
{
}
