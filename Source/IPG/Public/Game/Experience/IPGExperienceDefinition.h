// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IPGExperienceDefinition.generated.h"

class UIPGCharacterData;
class UGameFeatureAction;
class UIPGExperienceActionSet;

/**
 * 
 */
UCLASS(BlueprintType, Const)
class IPG_API UIPGExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UIPGExperienceDefinition(); 

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

	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;
	
	/** The default pawn class to spawn for players */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TObjectPtr<const UIPGCharacterData> CharacterData;
	
	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// List of additional action sets to compose into this experience
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<TObjectPtr<UIPGExperienceActionSet>> ActionSets;
};
