// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "EquipmentActor.generated.h"

UCLASS()
class IPGINVENTORY_API AEquipmentActor : public AActor
{
	GENERATED_BODY()
	
public:
	AEquipmentActor(); 

	FGameplayTag GetEquipmentType() const; 
	void SetEquipmentType(FGameplayTag Tag); 

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentTypeTag;
};
