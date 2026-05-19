// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/GameFeatureAction_WorldActionBase.h"

void UGameFeatureAction_WorldActionBase::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FDelegateHandle GameInstanceStartDelegateHandle = FWorldDelegates::OnStartGameInstance.AddUObject(
		this,
		&UGameFeatureAction_WorldActionBase::HandleGameInstanceStart,
		FGameFeatureStateChangeContext(Context));

	GameInstanceStartHandles.Add({ Context, GameInstanceStartDelegateHandle }); 

	// Add to any worlds with associated game instances that have already been initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (Context.ShouldApplyToWorldContext(WorldContext))
		{
			AddToWorld(WorldContext, Context);
		}
	}
}

void UGameFeatureAction_WorldActionBase::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FDelegateHandle* FoundHandle = GameInstanceStartHandles.Find(Context);
	if (FoundHandle->IsValid())
	{
		FWorldDelegates::OnStartGameInstance.Remove(*FoundHandle);
	}
}

void UGameFeatureAction_WorldActionBase::HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext)
{
	if (FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		if (ChangeContext.ShouldApplyToWorldContext(*WorldContext))
		{
			AddToWorld(*WorldContext, ChangeContext);
		}
	}
}

