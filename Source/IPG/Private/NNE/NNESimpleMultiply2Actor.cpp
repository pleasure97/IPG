// Fill out your copyright notice in the Description page of Project Settings.


#include "NNE/NNESimpleMultiply2Actor.h"

// Sets default values
ANNESimpleMultiply2Actor::ANNESimpleMultiply2Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

float ANNESimpleMultiply2Actor::PredictDouble(float InputValue)
{
	if (!ModelInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Modle instance is not valid in ANNESimpleMultiply2Actor::PredictDouble()"));
		return -1.f;
	}
	
	// Prepare data 
	TArray<float> InputData = { InputValue }; 
	TArray<float> OutputData = { 0.f }; 

	UE::NNE::FTensorBindingCPU InputBinding{ InputData.GetData(), (uint64)InputData.Num() * sizeof(float) }; 
	UE::NNE::FTensorBindingCPU OutputBinding{ OutputData.GetData(), (uint64)OutputData.Num() * sizeof(float) }; 

	// Execute synchronous inference (RunSync)
	if ((int32)ModelInstance->RunSync({ InputBinding }, { OutputBinding }) != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid RunSync error in ANNESimpleMultiply2Actor::PredictDouble()"));
		return -1.f;
	}

	return OutputData[0];
}

// Called when the game starts or when spawned
void ANNESimpleMultiply2Actor::BeginPlay()
{
	if (InitializeNNE())
	{
		OnModelReady();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Initializing NNE was failed in ANNESimpleMultiply2Actor::BeginPlay()"));
	}

	Super::BeginPlay();
}

bool ANNESimpleMultiply2Actor::InitializeNNE()
{
	if (!IsValid(ModelData))
	{
		return false;
	}

	// Get runtime CPU Interface (NNERuntimeORTCpu)
	TWeakInterfacePtr<INNERuntimeCPU> NNERuntimeCPUPtr = UE::NNE::GetRuntime<INNERuntimeCPU>(TEXT("NNERuntimeORTCpu")); 
	if (!NNERuntimeCPUPtr.IsValid())
	{
		return false;
	}

	// Create model
	TSharedPtr<UE::NNE::IModelCPU> Model = NNERuntimeCPUPtr->CreateModelCPU(ModelData); 
	if (!Model.IsValid())
	{
		return false;
	}

	// Create model instance
	ModelInstance = Model->CreateModelInstanceCPU(); 
	if (!ModelInstance.IsValid())
	{
		return false;
	}

	// Setup (1, 1) tensor shape
	ModelInstance->SetInputTensorShapes({ UE::NNE::FTensorShape::Make({1, 1}) });

	return true;
}
