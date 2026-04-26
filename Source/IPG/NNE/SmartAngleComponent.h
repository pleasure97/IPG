// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
/* NNE */
#include "NNE.h"			/* NNE default interface */
#include "NNETypes.h"		/* Define types such as FTensorBindingCPU */
#include "NNERuntimeCPU.h"	/* Interface for runtime CPU */
#include "NNEModelData.h"	/* Model asset data types */
/* NNE End */
#include "SmartAngleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPG_API USmartAngleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USmartAngleComponent();

	UFUNCTION(BlueprintCallable, Category = "SmartAngle")
	float GetSmartAngle(float Distance2D, float HeightDiff); 

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
