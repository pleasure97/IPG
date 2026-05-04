// Fill out your copyright notice in the Description page of Project Settings.


#include "IPGExclusiveTask.h"

TSharedPtr<FIPGExclusiveTask> FIPGExclusiveResource::ExchangeTailTask(TSharedPtr<FIPGExclusiveTask> NewTask)
{
	TSharedPtr<FIPGExclusiveTask> PrevExclusiveTask;

	/* Spin Lock */
	TailTaskSpinLock.Lock();
	PrevExclusiveTask = TailTaskPtr;
	TailTaskPtr = NewTask;
	TailTaskSpinLock.Unlock();
	/* Spin Lock End */

	return PrevExclusiveTask;
}

TSharedPtr<FIPGExclusiveTask> FIPGExclusiveTask::Create(TFunction<void()> InBody)
{
	TSharedPtr<FIPGExclusiveTask> NewTask = MakeShared<FIPGExclusiveTask>();
	NewTask->Body = MoveTemp(InBody);
	return NewTask;
}

FIPGExclusiveTask& FIPGExclusiveTask::BuildTaskGraph(TArray<FIPGExclusiveResource*> Resources)
{
	TArray<TSharedPtr<FIPGExclusiveTask>> PrecedingTasks;

	// 1) Sort resource list to prevent cycle
	Resources.Sort([](const FIPGExclusiveResource& A, const FIPGExclusiveResource& B)
		{
			// Prevent deadlock by comparing address values
			return &A < &B;
		});

	for (FIPGExclusiveResource* Resource : Resources)
	{
		// Get preceding tasks required for building task graph 
		TSharedPtr<FIPGExclusiveTask> PrecedingTask = Resource->ExchangeTailTask(AsShared());

		PrecedingTasks.Add(PrecedingTask);

		// Wait to spin until preceding task completes BuildTaskGraph
		if (PrecedingTask.IsValid())
		{
			while (!FPlatformAtomics::AtomicRead(&PrecedingTask->bBuildComplete))
			{
				FPlatformProcess::YieldThread();
			}
		}
	}

	// Collect Task of preceding tasks
	TArray<UE::Tasks::FTask> Prerequisites;
	for (const TSharedPtr<FIPGExclusiveTask>& PrecedingTask : PrecedingTasks)
	{
		if (PrecedingTask.IsValid() && PrecedingTask->InnerTask.IsValid())
		{
			Prerequisites.Add(PrecedingTask->InnerTask);
		}
	}

	// Create task by passing prerequisites
	TSharedRef<FIPGExclusiveTask> Self = AsShared();
	TFunction<void()> BodyCopy = Body;

	InnerTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [Self, BodyCopy]()
		{
			if (BodyCopy)
			{
				BodyCopy();
			}
		}, Prerequisites);

	// Notify succedding task when building graph completes
	FPlatformAtomics::AtomicStore(&bBuildComplete, 1);

	return *this;
}