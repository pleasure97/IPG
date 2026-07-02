// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponHitDetectionDataAsset.generated.h"

UENUM(BlueprintType)
enum class EHitDetectionMethod : uint8
{
	SweepTrace,				// Wide weapon
	TriangleIntersection,	// Thin weapon or weapon requiring precise hit detection
	RawAnimationData,
};

UENUM(BlueprintType)
enum class EHitInterpolationMethod : uint8
{
	Linear,		// Linear interpolation
	Bezier,		// Bezier interpolation
};

USTRUCT(BlueprintType)
struct FWeaponSocketInfo
{
	GENERATED_BODY()

public:
	// Socket name should match the name registered in the mesh
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SocketName;

	// Whether it refers to the 'tip' or the 'root' of the weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bWeaponTip = false;
};

/**
 * 
 */
UCLASS(BlueprintType)
class IPG_API UWeaponHitDetectionDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    // Hit detection method
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
    EHitDetectionMethod DetectionMethod = EHitDetectionMethod::SweepTrace;

    // Hit interpolation method
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation")
    EHitInterpolationMethod InterpolationMethod = EHitInterpolationMethod::Linear;

    // The number of substeps for interpolation (Greater numbers yield higher accuracy but increase costs)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation", meta = (ClampMin = 1, ClampMax = 16))
    int32 InterpolationSubSteps = 4;

    // Interpolation rate 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interpolation", meta = (ClampMin = 1, ClampMax = 16))
    float InterpolationRate = 30.f;

    // Socket lists for triangle intersection (At least one tip and one root are required)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets", meta = (EditCondition = "DetectionMethod != EHitDetectionMethod::SweepTrace"))
    TArray<FWeaponSocketInfo> Sockets;
};
