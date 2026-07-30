// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionCameraPreviewCharacter.h"
#include "Animation/DebugSkelMeshComponent.h"

AActionCameraPreviewCharacter::AActionCameraPreviewCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDebugSkelMeshComponent>(ACharacter::MeshComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
}