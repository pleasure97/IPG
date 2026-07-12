// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/IPGAnimEnum.h"
#include "IPGAnimStruct.generated.h"

USTRUCT(BlueprintType)
struct FIPGPlayerInputState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWantsToSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWantsToWalk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWantsToStrafe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWantsToAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWantsToCrouch;
};

USTRUCT(BlueprintType)
struct FIPGCharacterPropertiesForAnimation
{
	GENERATED_BODY()

public:
	FIPGCharacterPropertiesForAnimation(); 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIPGPlayerInputState InputState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIPGMovementMode MovementMode; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIPGAnimStance Stance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIPGRotationMode RotationMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIPGGait Gait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EIPGMovementDirection MovementDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ActorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector InputAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMaxAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMaxDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator OrientationIntent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator AimingRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJustLanded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LandVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector GroundNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector GroundLocation;
};