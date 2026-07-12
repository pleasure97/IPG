// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPGAnimEnum.generated.h"

UENUM(BlueprintType)
enum class EIPGMovementMode : uint8
{
	OnGround	UMETA(DisplayName = "OnGround"),
	InAir		UMETA(DisplayName = "InAir"),
	Sliding		UMETA(DisplayName = "Sliding"),
	Traversing	UMETA(DisplayName = "Traversing")
};

UENUM(BlueprintType)
enum class EIPGAnimStance : uint8
{
	Stand	UMETA(DisplayName = "Stand"),
	Crouch	UMETA(DisplayName = "Crouch")
};

UENUM(BlueprintType)
enum class EIPGMovementState : uint8
{
	Idle	UMETA(DisplayName = "Idle"),
	Moving	UMETA(DisplayName = "Moving")
};

UENUM(BlueprintType)
enum class EIPGGait : uint8
{
	Walk	UMETA(DisplayName = "Walk"),
	Run		UMETA(DisplayName = "Run"),
	Sprint	UMETA(DisplayName = "Sprint")
};

UENUM(BlueprintType)
enum class EIPGRotationMode : uint8
{
	OrientationToMovement	UMETA(DisplayName = "OrientationToMovement"),
	Strafe					UMETA(DisplayName = "Strafe"),
	Aim						UMETA(DisplayName = "Aim")
};

UENUM(BlueprintType)
enum class EIPGMovementDirection : uint8
{
	Front					UMETA(DisplayName = "F"),
	Back					UMETA(DisplayName = "B"),
	LeftLeft				UMETA(DisplayName = "LL"),
	LeftRight				UMETA(DisplayName = "LR"),
	RightLeft				UMETA(DisplayName = "RL"),
	RightRight				UMETA(DisplayName = "RR")
};