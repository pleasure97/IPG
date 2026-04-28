// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataCollectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPG_API UDataCollectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDataCollectComponent();

protected:
	virtual void BeginPlay() override;

	// Function to save data as CSV file
	// Distance2D - Horizontal distance
	// HeightDiff - Difference between Target and Start
	// Angle - Launch angle 
	UFUNCTION(BlueprintCallable, Category="NNE|Data")
	void SaveProjectileData(float Distance2D, float HeightDiff, float Angle);
};
