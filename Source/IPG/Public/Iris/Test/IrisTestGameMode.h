// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPGGameMode.h"
#include "IrisTestGameMode.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API AIrisTestGameMode : public AIPGGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
