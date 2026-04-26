// Fill out your copyright notice in the Description page of Project Settings.


#include "NNE/SmartAngleComponent.h"

USmartAngleComponent::USmartAngleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//==================================================
//[Normalized value]
//Input(X) Mean : [1305.2756889724026, 170.41463305032468, 0.5162337662337663]
//Input(X) Std : [848.3461958417342, 162.59363649649316, 0.49973639534645403]
//Output(Y) Mean : [29.595924501623376]
//Output(Y) Std : [16.19202720795021]
//==================================================
float USmartAngleComponent::GetSmartAngle(float Distance2D, float HeightDiff)
{
	if (!ModelInstance.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Model Instance is not valid in USmartAngleComponent::GetSmartAngle()"));
		return -1.f;
	}

	// Initialize variable to low angle 
	float ArcMode = 0.f;

	// 1. Normalize input data 
	float ScaledDistance = (Distance2D - 1305.2756889724026f) / 848.3461958417342f;
	float ScaledHeight = (HeightDiff - 170.41463305032468f) / 162.59363649649316f;
	float ScaledArcMode = (ArcMode - 0.5162337662337663f) / 0.49973639534645403f;

	// 2. Setup NNE Input/Ouput Bindings 
	TArray<float> InputData = { ScaledDistance, ScaledHeight, ScaledArcMode };
	TArray<float> OutputData = { 0.f }; 

	UE::NNE::FTensorBindingCPU InputBinding{ InputData.GetData(), (uint64)InputData.Num() * sizeof(float) }; 
	UE::NNE::FTensorBindingCPU OutputBinding{ OutputData.GetData(), (uint64)OutputData.Num() * sizeof(float) }; 

	// 3. Execute AI Inference
	auto Status = ModelInstance->RunSync({ InputBinding }, { OutputBinding });

	if ((int32)Status != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Inference is failed in USmartAngleComponent::GetSmartAngle()")); 
		return -1.f;
	}

	// 4. Denormalize output data 
	// (output * standard deviation) + mean
	float PredictedAngle = (OutputData[0] * 16.19202720795021f) + 29.595924501623376f;

	return PredictedAngle;
}

void USmartAngleComponent::BeginPlay()
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

bool USmartAngleComponent::InitializeNNE()
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
	ModelInstance->SetInputTensorShapes({ UE::NNE::FTensorShape::Make({1, 3}) });

	return true;
}
