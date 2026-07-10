// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MassEntityEnemy.generated.h"

UCLASS()
class IPG_API AMassEntityEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AMassEntityEnemy();

protected:
	virtual void BeginPlay() override;

};
