// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IPGExperienceManagerComponent.generated.h"

namespace UE::GameFeatures { struct FResult; }

class UIPGExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnIPGExperienceLoaded, const UIPGExperienceDefinition* /* Experience */)

enum class EIPGExperienceLoadState : uint8
{
	Unloaded, 
	Loading, 
	LoadingGameFeatures, 
	ExecutingActions, 
	Loaded, 
	Deactivating
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPG_API UIPGExperienceManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UIPGExperienceManagerComponent();

	// Ensure the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegisterExperienceLoadedCallback(FOnIPGExperienceLoaded::FDelegate&& Delegate);

	// Try to set the current experience, either a UI or gameplay one
	void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	bool IsExperienceLoaded() const;

	// This returns the current experience if it is fully loaded, asserting otherwise
	const UIPGExperienceDefinition* GetCurrentExperienceChecked() const;

protected:
	/* UObject Interface */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/* Game Feature Plugins */
	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);

	/* Experience Load */
	EIPGExperienceLoadState LoadState = EIPGExperienceLoadState::Unloaded;

	FOnIPGExperienceLoaded OnExperienceLoaded;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperience)
	TObjectPtr<const UIPGExperienceDefinition> CurrentExperience;

	UFUNCTION()
	void OnRep_CurrentExperience();

	void StartExperienceLoad();

	void OnExperienceLoadComplete();

	void OnExperienceFullLoadCompleted();
};
