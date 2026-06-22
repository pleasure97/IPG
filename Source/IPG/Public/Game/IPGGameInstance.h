// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IPGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API UIPGGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UIPGGameInstance(); 

protected:
	virtual void Init() override;
};
