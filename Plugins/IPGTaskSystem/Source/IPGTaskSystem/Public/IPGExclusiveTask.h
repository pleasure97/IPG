// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"          
#include "HAL/PlatformAtomics.h"
#include "Misc/SpinLock.h"

class FIPGExclusiveTask;

/**
 * Exclusive Resource
 * - Each resource atomically manages the TailTask last registered to it
 */
class IPGTASKSYSTEM_API FIPGExclusiveResource
{
public:
	FIPGExclusiveResource() = default;
	~FIPGExclusiveResource() = default;

	// Copy/Move prohibited because this is used as a pointer identifier
	FIPGExclusiveResource(const FIPGExclusiveResource&) = delete;
	FIPGExclusiveResource& operator=(const FIPGExclusiveResource&) = delete;

	/*
	 * ExchangeTailTask
	 * - Replace this resource's tail task with NewTask atomically, 
	 * - return preceding tail task
	 */
	TSharedPtr<FIPGExclusiveTask> ExchangeTailTask(TSharedPtr<FIPGExclusiveTask> NewTask);

private:
	TSharedPtr<FIPGExclusiveTask> TailTaskPtr;
	UE::FSpinLock TailTaskSpinLock;
};

/**
 * Exclusive Task 
 * - Atomically exchange "TailTask" for each shared resource to form an implicit precedence / sucession on relationship.
 * - "BuildTaskGraph()" sorts the resource list to prevent cycles, waits until previous TailTask for each resource is completed, 
 * - and then starts its own task.
 */
class IPGTASKSYSTEM_API FIPGExclusiveTask : public TSharedFromThis<FIPGExclusiveTask>
{
public:
	// Create Exclusive Task Instance Receving Lambda Function
	static TSharedPtr<FIPGExclusiveTask> Create(TFunction<void()> InBody);

	/*
	 * Build Task Graph 
	 * 
	 * 1) Sort resource list to prevent cycle
	 * 2) Get preceding task from each resource using "ExchangeTailTask()"
	 * 3) Wait to spin until the preceding task's "bBuildComplete" flag becomes true
	 * 4) Start own task once all preceding tasks are completed
	 * 5) Atomically set "bBuildComplete to true
	 */
	FIPGExclusiveTask& BuildTaskGraph(TArray<FIPGExclusiveResource*> Resources);

	UE::Tasks::FTask InnerTask;
private:
	TFunction<void()> Body;
	volatile int32 bBuildComplete = 0; 
};