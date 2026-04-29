// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IPGCharacterData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Const)
class IPG_API UIPGCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// Class to instantiate for this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IPG|Character")
	TSubclassOf<APawn> PawnClass;
};
