// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
/* NNE */
#include "NNE.h"			/* NNE default interface */
#include "NNETypes.h"		/* Define types such as FTensorBindingCPU */
#include "NNERuntimeCPU.h"	/* Interface for runtime CPU */
#include "NNEModelData.h"	/* Model asset data types */
/* NNE End */
#include "NNESimpleMultiply2Actor.generated.h"

UCLASS()
class IPG_API ANNESimpleMultiply2Actor : public AActor
{
	GENERATED_BODY()
	
public:	
	ANNESimpleMultiply2Actor();

	UFUNCTION(BlueprintCallable, Category = "NNE")
	float PredictDouble(float InputValue); 

protected:
	virtual void BeginPlay() override;

	// Model data to be setup in editor (.onnx)
	UPROPERTY(EditAnywhere, Category = "NNE")
	TObjectPtr<UNNEModelData> ModelData; 

	// Instance for performing actual calcualtions
	TSharedPtr<UE::NNE::IModelInstanceCPU> ModelInstance;

	UFUNCTION(BlueprintImplementableEvent, Category = "NNE")
	void OnModelReady(); 

private:
	bool InitializeNNE(); 
};
