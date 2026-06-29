// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/IPGGameInstance.h"
#include "IrisTestGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API UIrisTestGameInstance : public UIPGGameInstance
{
	GENERATED_BODY()
	
protected:
	virtual void Init() override;
};
