// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

namespace IPGCameraGameplayTags
{
	/* Camera View */
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_FirstPerson);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_OverShoulder);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_ThirdPerson);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_XRayVision);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_TopDown);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_Crouch);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_SideScroller);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_LookAtClosestTarget);
}