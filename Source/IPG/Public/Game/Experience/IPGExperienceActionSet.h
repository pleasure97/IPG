// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IPGExperienceActionSet.generated.h"

class UGameFeatureAction;

/**
 * 
 */
UCLASS()
class IPG_API UIPGExperienceActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UIPGExperienceActionSet(); 

	/* UObject Interface */
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif		
	/* UObject Interface End */

	/* UPrimaryDataAsset Interface */
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	/* UPrimaryDataAsset Interface End */

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditAnywhere, Instanced, Category="Actions to Perform")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditAnywhere, Category = "Feature Dependencies")
	TArray<FString> GameFeaturesToEnable;
};
