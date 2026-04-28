// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IPGExperienceManagerComponent.generated.h"

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

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegisterExperienceLoadedCallback(FOnIPGExperienceLoaded::FDelegate&& Delegate);

	bool IsExperienceLoaded() const;

protected:
	/* UObject Interface */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	EIPGExperienceLoadState LoadState = EIPGExperienceLoadState::Unloaded;

	FOnIPGExperienceLoaded OnExperienceLoaded;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperience)
	TObjectPtr<const UIPGExperienceDefinition> CurrentExperience;

	UFUNCTION()
	void OnRep_CurrentExperience();

	void StartExperienceLoad();
};
