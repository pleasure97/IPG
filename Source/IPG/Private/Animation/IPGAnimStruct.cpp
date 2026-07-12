// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/IPGAnimStruct.h"

FIPGCharacterPropertiesForAnimation::FIPGCharacterPropertiesForAnimation()
	:MovementMode(EIPGMovementMode::OnGround) 
	, Stance(EIPGAnimStance::Stand)
	, RotationMode(EIPGRotationMode::OrientationToMovement)
	, Gait(EIPGGait::Walk)
	, MovementDirection(EIPGMovementDirection::Front)
	, ActorTransform(FTransform::Identity)
	, Velocity(FVector::ZeroVector)
	, InputAcceleration(FVector::ZeroVector)
	, CurrentMaxAcceleration(0.0f)
	, CurrentMaxDeceleration(0.0f)
	, OrientationIntent(FRotator::ZeroRotator)
	, AimingRotation(FRotator::ZeroRotator)
	, bJustLanded(false)
	, LandVelocity(FVector::ZeroVector)
	, SteeringTime(0.0f)
	, GroundNormal(FVector::UpVector)
	, GroundLocation(FVector::ZeroVector)
{
}
