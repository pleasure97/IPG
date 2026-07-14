// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IPGPreCMCTickComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FIPGPreCMCTickSignature);

class ACharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPG_API UIPGPreCMCTickComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UIPGPreCMCTickComponent();

	FIPGPreCMCTickSignature OnIPGPreCMCTickSignature;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
