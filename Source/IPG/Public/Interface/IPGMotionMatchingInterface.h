// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Animation/IPGAnimStruct.h"
#include "IPGMotionMatchingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIPGMotionMatchingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class IPG_API IIPGMotionMatchingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FIPGCharacterPropertiesForAnimation GetPropertiesForAnimation() const = 0;
	
};
