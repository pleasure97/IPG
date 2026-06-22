// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NNEDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class NNEDATACOLLECTOR_API UNNEDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="NNE|DataCollect")
	FFilePath NNEDataPath;
	
	UPROPERTY(EditAnywhere, Category = "NNE|DataCollect")
	TArray<FString> InputNames;

	UPROPERTY(EditAnywhere, Category = "NNE|DataCollect")
	TArray<FString> OutputNames;
	
	UPROPERTY(EditAnywhere, Category = "NNE|DataCollect")
	FString CSVFileName;

	UPROPERTY(EditAnywhere, Category = "NNE|DataCollect")
	float CollectInterval;

	void LoadFromNNEData();
};
