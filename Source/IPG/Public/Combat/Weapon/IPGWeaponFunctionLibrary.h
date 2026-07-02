// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IPGWeaponFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API UIPGWeaponFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static FVector BezierInterpolate(const FVector& Start, const FVector& Control, const FVector& End, float Alpha);
};
