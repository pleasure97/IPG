// Fill out your copyright notice in the Description page of Project Settings.


#include "District/IPGDistrictProcessorSubsystem.h"

void FIPGGameSectionProcessor::ProcessSection(const FDistrictSection& Section, float DeltaTime)
{
	const float SectionSize = 5'000.f;	// default line of sight distance
	FVector SectionOrigin(Section.Col * SectionSize, Section.Row * SectionSize, 0.f); 
	FVector SectionExtent(SectionSize * 0.5f, SectionSize * 0.5f, 10'000.f);
	FBox SectionBounds(SectionOrigin - SectionExtent, SectionOrigin + SectionExtent);

	// TODO - Update NPC, physics object in section bounds
	// This function will be called in worker thread, so do not use this in game thread
	UE_LOG(LogTemp, Log, TEXT("[Section (%d,%d)] Processing on thread %d"),
		Section.Row, Section.Col,
		FPlatformTLS::GetCurrentThreadId());
}

void UIPGDistrictProcessorSubsystem::Tick(float DeltaTime)
{
	if (DistrictProcessor)
	{
		DistrictProcessor->Execute(DeltaTime);
	}
}

void UIPGDistrictProcessorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get the number of districts and create district processor
	FIntPoint NumDistricts = GetNumDistricts();
	DistrictProcessor = MakeUnique<FIPGDistrictProcessor>(NumDistricts.X, NumDistricts.Y, MakeShared<FIPGGameSectionProcessor>());
}

void UIPGDistrictProcessorSubsystem::Deinitialize()
{
	DistrictProcessor.Reset(); 

	Super::Deinitialize();
}

FIntPoint UIPGDistrictProcessorSubsystem::GetNumDistricts() const
{
	return FIntPoint(4, 4);
}