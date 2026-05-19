// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureAction_WorldActionBase.generated.h"

class UGameInstance;

/**
 * Base Class for GameFeatureActions that wish to do something world specific
 */
UCLASS()
class IPG_API UGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
	GENERATED_BODY()
	
public:
	/* UGameFeatureAction Interface */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	/* UGameFeatureAction Interface End */

private:
	void HandleGameInstanceStart(UGameInstance* GameInstance, FGameFeatureStateChangeContext ChangeContext); 

	// Override with the action-specific logic
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) PURE_VIRTUAL(UGameFeatureAction_WorldActionBase::AddToWorld, ); 

	TMap<FGameFeatureStateChangeContext, FDelegateHandle> GameInstanceStartHandles;
};
