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
struct FActionCameraStep
{
	GENERATED_BODY()

public:
	// The name under which this step will appear on the track in the timeline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Step")
	FName StepName;

	// Start time based on reference animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step", meta = (ClampMin = "0.0"))
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step", meta = (ClampMin = "0.01"))
	float Duration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	EActionCameraViewMode TargetMode = EActionCameraViewMode::ThirdPerson;

	// The camera rig to activate in this step. 
	// Transitions/blends follow the logic defined within the Rig itself 
	// (or in a BlendStack/Transition node within the Rig graph).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TSoftObjectPtr<UCameraRigAsset> CameraRig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TSoftObjectPtr<UCameraVariableCollection> VariableCollection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
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
	TSoftObjectPtr<UAnimSequence> ReferenceAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
	TSoftObjectPtr<USkeletalMesh> PreviewMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TArray<FActionCameraStep> CameraSteps;
};