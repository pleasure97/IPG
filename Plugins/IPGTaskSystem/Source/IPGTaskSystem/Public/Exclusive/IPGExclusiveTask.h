// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/Task.h"          
#include "Trace/Trace.h"

class FIPGExclusiveTask;

UE_TRACE_CHANNEL_EXTERN(IPGTaskSystemChannel, IPGTASKSYSTEM_API)

enum class EIPGThreadMode : uint8
{
	WorkerThread,
	GameThread
};

/**
 * Exclusive Resource
 * - Each resource atomically manages the TailTask last registered to it
 */
class IPGTASKSYSTEM_API FIPGExclusiveResource
{
public:
	FIPGExclusiveResource() = default;
	~FIPGExclusiveResource();

	// Copy/Move prohibited
	FIPGExclusiveResource(const FIPGExclusiveResource&) = delete;
	FIPGExclusiveResource& operator=(const FIPGExclusiveResource&) = delete;

	/*
	 * ExchangeTailTask
	 * - Replace this resource's tail task with NewTask atomically, 
	 * - return preceding tail task
	 */
	UE::Tasks::FTask ExchangeTailTask(const UE::Tasks::FTask& NewTask);

private:
	UE::Tasks::FTask TailTask;
};

/**
 * Exclusive Task 
 * - Atomically exchange "TailTask" for each shared resource to form an implicit precedence / sucession on relationship.
 * - "BuildTaskGraph()" sorts the resource list to prevent cycles, waits until previous TailTask for each resource is completed, 
 * - and then starts its own task.
 */

 /*
* Build Task Graph
*
* 1) Sort resource list to prevent cycle
* 2) Get preceding task from each resource using "ExchangeTailTask()"
* 3) Wait to spin until the preceding task's "bBuildComplete" flag becomes true
* 4) Start own task once all preceding tasks are completed
* 5) Atomically set "bBuildComplete to true
*/

namespace IPGTaskSystem
{
	/**
	 * Launch task that guarantees exclusive access to multiple shared resources
	 * @param DebugName       Name for debugging and profiling
	 * @param Resources       List of required resources determined at runtime
	 * @param Body            Lambda function to execute
	 * @param ThreadMode      Whether to delegate to game thread
	 * @param Prerequisites   Additional prerequisite tasks to wait for (Optional)
	 * @return                FTask indicating the completion of this task 
	 */
	IPGTASKSYSTEM_API UE::Tasks::FTask LaunchExclusive(
		const TCHAR* DebugName,
		TArray<FIPGExclusiveResource*> Resources,
		TFunction<void()> Body,
		EIPGThreadMode ThreadMode = EIPGThreadMode::WorkerThread,
		TConstArrayView<UE::Tasks::FTask> Prerequisites = {} /* { TaskA, TaskB} */
	);
}