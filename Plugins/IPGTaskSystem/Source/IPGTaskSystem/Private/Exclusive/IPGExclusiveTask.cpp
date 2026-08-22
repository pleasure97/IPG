// Fill out your copyright notice in the Description page of Project Settings.

#include "Exclusive/IPGExclusiveTask.h"
#include "Misc/ScopeLock.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Algo/Unique.h"

UE_TRACE_CHANNEL_DEFINE(IPGTaskSystemChannel)

namespace IPGTaskSystem::Private
{
	// Critical section waits for the build of the preceding task to complete, 
	// not for the actual execution of the preceding task to finish, 
	// so it does not significantly impact overall performance
	static FCriticalSection GBuildLock;

#if DO_CHECK
	static thread_local bool bBuildInScope = false;
#endif //DO_CHECK
}

FIPGExclusiveResource::~FIPGExclusiveResource()
{
	// If the object is destroyed while a task that has not yet executed is referencing this resource,
	// The body of the task accesses a dangling resource.
	checkf(!TailTask.IsValid() || TailTask.IsCompleted(),
		TEXT("FIPGExclusiveResource destroyed while an exclusive task is still pending."));
}

UE::Tasks::FTask FIPGExclusiveResource::ExchangeTailTask(const UE::Tasks::FTask& NewTask)
{
	checkf(IPGTaskSystem::Private::bBuildInScope, TEXT("ExchangeTailTask must be called under the exclusive build lock."));

	UE::Tasks::FTask PrevTask = MoveTemp(TailTask); 
	TailTask = NewTask;

	// Exclude already completed preceding task
	if (PrevTask.IsValid() && PrevTask.IsCompleted())
	{
		return {};
	}
	
	return PrevTask;
}

namespace IPGTaskSystem
{
	UE::Tasks::FTask LaunchExclusive(
		const TCHAR* DebugName,
		TArray<FIPGExclusiveResource*> Resources,
		TFunction<void()> Body,
		EIPGThreadMode ThreadMode,
		TConstArrayView<UE::Tasks::FTask> Prerequisites)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_TEXT_ON_CHANNEL(DebugName, IPGTaskSystemChannel);

		// 1) Remove null values from the resource array to prevent sorting errors
		Resources.RemoveAll([](const FIPGExclusiveResource* Resource) { return Resource == nullptr; });

		// 2) Sort resource list to prevent deadlock
		Algo::Sort(Resources, [](const FIPGExclusiveResource* ResourceA, const FIPGExclusiveResource* ResourceB)
			{
				return reinterpret_cast<UPTRINT>(ResourceA) < reinterpret_cast<UPTRINT>(ResourceB);
			});


		Resources.SetNum(Algo::Unique(Resources), EAllowShrinking::No);

		// 3) Create completion event 
		UE::Tasks::FTaskEvent CompletionEvent(DebugName);

		TArray<UE::Tasks::FTask> AllPrerequisites;
		AllPrerequisites.Reserve(Resources.Num() + Prerequisites.Num());

		for (const UE::Tasks::FTask& Prerequisite : Prerequisites)
		{
			if (Prerequisite.IsValid() && !Prerequisite.IsCompleted())
			{
				AllPrerequisites.Add(Prerequisite);
			}
		}

		// 4) Get prerequisite tasks needed to generate the task graph
		// Set to release the lock when the scope ends
		{
			FScopeLock BuildLock(&Private::GBuildLock); 

#if DO_CHECK
			Private::bBuildInScope = true;
			ON_SCOPE_EXIT{ Private::bBuildInScope = false; };
#endif
			for (FIPGExclusiveResource* Resource : Resources)
			{
				UE::Tasks::FTask PrevTask = Resource->ExchangeTailTask(CompletionEvent); 
				if (PrevTask.IsValid())
				{
					AllPrerequisites.Add(MoveTemp(PrevTask));
				}
			}
		}

		const UE::Tasks::EExtendedTaskPriority ExtendedTaskPriority = (ThreadMode == EIPGThreadMode::GameThread)
			? UE::Tasks::EExtendedTaskPriority::GameThreadNormalPri
			: UE::Tasks::EExtendedTaskPriority::None;

		// 5) Launch after resolving dependencies, then route thread
		// Scheduled only when all tasks in the array are completed, and execution is guaranteed upon completion
		UE::Tasks::Launch(
			DebugName, 
			[DebugName, Body = MoveTemp(Body), ThreadMode, CompletionEvent]() mutable
			{
				ON_SCOPE_EXIT{ CompletionEvent.Trigger(); };

				TRACE_CPUPROFILER_EVENT_SCOPE_TEXT_ON_CHANNEL(DebugName, IPGTaskSystemChannel);

				// Game Thread
				if (Body)
				{
					Body();
				}
			}, 
			MoveTemp(AllPrerequisites),
			UE::Tasks::ETaskPriority::Default,
			ExtendedTaskPriority);

		return CompletionEvent;
	}
}