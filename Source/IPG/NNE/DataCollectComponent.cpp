// Fill out your copyright notice in the Description page of Project Settings.


#include "NNE/DataCollectComponent.h"

UDataCollectComponent::UDataCollectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDataCollectComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UDataCollectComponent::SaveProjectileData(float Distance2D, float HeightDiff, float Angle)
{
	// 1. Setup path to save CSV file (/Saved/ProjectileTrainingData.csv)
	FString FilePath = FPaths::ProjectSavedDir() + TEXT("ProjectileTrainingData.csv"); 

	// 2. Initialize a line of data 
	FString DataLine = FString::Printf(TEXT("%f,%f,%f,%d\n"), Distance2D, HeightDiff, Angle, Angle > 30 ? 1 : 0); 

	// 3. Write to the file (FILEWRITE_Append option)
	bool bSuccess = FFileHelper::SaveStringToFile(
		DataLine,
		*FilePath,
		FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(),
		FILEWRITE_Append); 

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("NNE Data Saved : %s"), *DataLine);
	}
}
