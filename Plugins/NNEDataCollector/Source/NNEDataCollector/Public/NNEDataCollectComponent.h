// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NNEDataAsset.h"
#include "NNEDataCollectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NNEDATACOLLECTOR_API UNNEDataCollectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNNEDataCollectComponent();

	UPROPERTY(EditAnywhere, Category = "NNE|DataCollect")
	TObjectPtr<UNNEDataAsset> NNEDataAsset;

	UFUNCTION(BlueprintCallable, Category = "NNE|DataCollect")
	void SetValue(const FString& Name, float Value);

	/*UFUNCTION(BlueprintCallable, Category = "NNE|DataCollect")
	void Commit();*/
};
