// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core/CameraRigAsset.h"
#include "ActionCameraDirectorAsset.generated.h"

class UCameraVariableCollection;
class UAnimSequence;
class USkeletalMesh;

UENUM(BlueprintType)
enum class EActionCameraViewMode : uint8
{
	FirstPerson,
	ThirdPerson,
	OverShoulder,
	TopDown,
	LockOn
};

USTRUCT(BlueprintType)
struct FActionCameraClip
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Clip")
	FName ClipName;

	UPROPERTY(EditAnywhere, Category = "Clip")
	TSoftObjectPtr<UAnimSequenceBase> Animation;

	// Start time based on reference animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step", meta = (ClampMin = "0.0"))
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step", meta = (ClampMin = "0.01"))
	float Duration = 0.f;
};

USTRUCT(BlueprintType)
struct FActionCameraEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Event")
	FName EventName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Event", meta = (ClampMin = "0.0"))
	float Time = 0.f;

	UPROPERTY(EditAnywhere, Category = "Event", meta = (ClampMin = "0.0"))
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSoftObjectPtr<UCameraRigAsset> CameraRig;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSoftObjectPtr<UCameraVariableCollection> VariableCollection;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TMap<FName, float> VariableOverrides;
};

/**
 * An "editor-only" asset that defines the camera direction for a single skill
 * CameraSteps are positioned based on the ReferenceAnimation's timeline.
 */
UCLASS(BlueprintType, Blueprintable)
class ACTIONCAMERAEDITOR_API UActionCameraDirectorAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	TSoftObjectPtr<USkeletalMesh> PreviewMesh;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	TArray<FActionCameraClip> CameraSteps;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSoftObjectPtr<UCameraRigAsset> DefaultCameraRig;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TSoftObjectPtr<UCameraVariableCollection> DefaultVariableCollection;

	UPROPERTY(EditAnywhere, Category = "Events")
	TArray<FActionCameraEvent> CameraEvents;
};