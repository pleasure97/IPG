// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MerchantComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPGINVENTORY_API UMerchantComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMerchantComponent();

protected:
	virtual void BeginPlay() override;
	
};
