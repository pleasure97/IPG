// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NNEDataType.h"
#include "NNEDataCollectSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class NNEDATACOLLECTOR_API UNNEDataCollectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void SaveRecord(const INNEDataRecord& DataRecord);
	
	UFUNCTION(BlueprintCallable, Category="NNE|DataCollect")
	void FlushAll(); 

	virtual void Deinitialize() override;
};
