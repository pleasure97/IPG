// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IPGNetPrioritizerDataAsset.generated.h"

UENUM(BlueprintType)
enum class EIPGMapType : uint8
{
	Default,
	DenseCity,    
	BattleField, 
};

USTRUCT(BlueprintType)
struct FFoVPrioritizerPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sphere")
    float InnerSphereRadius = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sphere")
    float InnerSpherePriority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sphere")
    float OuterSphereRadius = 10000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sphere")
    float OuterSpherePriority = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cone")
    float ConeFieldOfViewDegrees = 45.f; 

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cone")
    float InnerConeLength = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cone")
    float ConeLength = 20000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cone")
    float MaxConePriority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cone")
    float MinConePriority = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LineOfSight")
    float LineOfSightWidth = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LineOfSight")
    float LineOfSightPriority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Outside")
    float OutsidePriority = 0.1f;
};

/**
 * 
 */
UCLASS(abstract)
class IPG_API UIPGNetPrioritizerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
};

UCLASS(meta = (DisplayName = "IPG FoV Net Prioritizer Data Asset"))
class IPG_API UIPGFoVNetPrioritizerDataAsset : public UIPGNetPrioritizerDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    EIPGMapType MapType = EIPGMapType::Default;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (DisplayName = "Field of View Net Prioritizer"))
    FFoVPrioritizerPreset FoVPrioritizerPresetData;
};