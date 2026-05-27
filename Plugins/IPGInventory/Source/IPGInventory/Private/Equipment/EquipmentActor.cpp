// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/EquipmentActor.h"

AEquipmentActor::AEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

FGameplayTag AEquipmentActor::GetEquipmentType() const
{
	return EquipmentTypeTag;
}

void AEquipmentActor::SetEquipmentType(FGameplayTag Tag)
{
	EquipmentTypeTag = Tag;
}

