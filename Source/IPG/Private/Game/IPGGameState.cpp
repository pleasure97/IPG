// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/IPGGameState.h"
#include "Game/Experience/IPGExperienceManagerComponent.h"

AIPGGameState::AIPGGameState()
{
	ExperienceManagerComponent = CreateDefaultSubobject<UIPGExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
}
