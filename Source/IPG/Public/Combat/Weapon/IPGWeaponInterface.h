// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Combat/Weapon/WeaponHitDetectionComponent.h"
#include "IPGWeaponInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIPGWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IPG_API IIPGWeaponInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UMeshComponent* GetWeaponMesh() const = 0;
	virtual UShapeComponent* GetWeaponCollision() const = 0;
	virtual UWeaponHitDetectionComponent* GetWeaponHitDetection() const = 0;
};
