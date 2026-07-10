// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MassEntityEnemyType.h"

bool FEnemySpawnRequest::IsValid() const
{
	return (Remaining > 0) && (!SpawnLocations.IsEmpty());
}