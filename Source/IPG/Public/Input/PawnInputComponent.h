// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnInputComponent.generated.h"

class UIPGInputConfig;
struct FPriorityInputMappingContext;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPG_API UPawnInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPawnInputComponent();

	// Returns the hero component if one exists on the specified actor. 
	UFUNCTION(BlueprintPure, Category = "IPG|Input")
	static UPawnInputComponent* FindPawnInputComponent(const AActor* Actor);

	//  Adds mode-specific input config
	void AddAdditionalInputConfig(const UIPGInputConfig* InputConfig);

	//  Removes a mode-specific input config if it has been added 
	void RemoveAdditionalInputConfig(const UIPGInputConfig* InputConfig);


protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<FPriorityInputMappingContext> DefaultInputMappingContexts;
};
