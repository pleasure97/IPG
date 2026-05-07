// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NNEDataCollectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NNEDATACOLLECTOR_API UNNEDataCollectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNNEDataCollectComponent();
};
