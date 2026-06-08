// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * FDistrictSection
 * Individual section which makes up distriction
 * Each section is a sqaure area with a side length equal to the line of sight
 */
struct FDistrictSection
{
public:
	FDistrictSection(int32 InRow, int32 InCol)
		: Row(InRow), Col(InCol) 
	{}

	int32 Row;
	int32 Col;
};

/**
 * IIPGSectionProcessor
 * Interface for the actual game logic to be executed in each section
 * You can inherit from it and implement ProcessSection()
 */
class IIPGSectionProcessor
{
public:
	virtual ~IIPGSectionProcessor() = default;

	virtual void ProcessSection(const FDistrictSection& Section, float DeltaTime) = 0;
};

/**
 * FIPGDistrictProcessor
 * District processor based on static task graph for seamless world
 * 
 * [ Design Principles ]
 * - District consists of N * M sections
 * - Maximum interaction distance between actor does not exceed the line of sight distance
 * - Sections with a horizontal and vertical spacing of 2 or more do not affect each other and can be executed simultaneously
 * - This principle is statically pre-calculated to re-execute the same task graph every tick
 * - so that achieve thread-safe parallel processing without separate mutex or critical sections
 * 
 * [ How to Use ]
 * 1. Create a class which implements ISectionProcessor
 * 2. Create FDistrictProcessor and call Initialize()
 * 3. Call Execute() every tick
 * 
 * [ Example ]
 *		MyProcessor = MakeUnique<FDistrictProcessor>(4, 4, MakeShared<FMyGameSectionProcessor>()); 
 *		MyProcessor->Initialize();
 *		MyProcessor->Execute(DeltaTime);
 */
class IPGTASKSYSTEM_API FIPGDistrictProcessor
{
public:
	FIPGDistrictProcessor(int32 InNumRows, int32 InNumCols, TSharedPtr<IIPGSectionProcessor> InSectionProcessor); 
	~FIPGDistrictProcessor(); 

	// Pre-calculate static task graph when game starts or the size of district is changed
	void Initialize(); 

	// Execute precalculated task graph and wait for completion 
	// Called every tick
	void Execute(float DeltaTime); 

	int32 GetNumRows(); 
	int32 GetNumCols(); 

private:
	class FSectionTask
	{
	public:
		FSectionTask(FIPGDistrictProcessor* InOwner, int32 InRow, int32 InCol, float InDeltaTime)
			: Owner(InOwner), Row(InRow), Col(InCol), DeltaTime(InDeltaTime)
		{}

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FSectionTask, STATGROUP_TaskGraphTasks);
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::AnyBackgroundThreadNormalTask;
		}

		static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::TrackSubsequents;
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& CompletionGraphEvent);

	private:
		FIPGDistrictProcessor* Owner;
		int32 Row;
		int32 Col;
		float DeltaTime;
	};

	// Return previous section coordinates of the section
	TArray<FIntPoint> GetPrerequisites(int32 Row, int32 Col) const;

	// Check if section is on the district boundary
	bool IsEdgeSection(int32 Row, int32 Col) const;

	// Transform 2D index to 1d array index
	int32 Index(int32 Row, int32 Col) const;

	/* Member Variables */
	int32 NumRows;
	int32 NumCols;

	TSharedPtr<IIPGSectionProcessor> SectionProcessor;

	// Array of graph event about each section 
	// Recreate every time when Execute() is called
	TArray<FGraphEventRef> SectionEvents;

	// Array of execution order 
	// Calculated once at Initialize()
	TArray<FIntPoint> ExecutionOrders;
};
