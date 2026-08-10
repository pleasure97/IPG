// Fill out your copyright notice in the Description page of Project Settings.


#include "Exclusive/IPGExclusiveTask.h"
#include "Misc/ScopeLock.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UE_TRACE_CHANNEL_DEFINE(IPGTask)

UE::Tasks::FTask FIPGExclusiveResource::ExchangeTailTask(UE::Tasks::FTask NewTask)
{
	FScopeLock Lock(&TailTaskCriticalSection);

	UE::Tasks::FTask PrevTask = TailTask;
	TailTask = NewTask;

	return PrevTask;
}

namespace IPGTaskSystem
{
	UE::Tasks::FTask LaunchExclusive(
		const TCHAR* DebugName,
		TArray<FIPGExclusiveResource*> Resources,
		TFunction<void()> Body,
		EIPGThreadMode ThreadMode,
		UE::Tasks::FTask Prerequisites)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_TEXT_ON_CHANNEL(DebugName, IPGTask);

		// 1) Sort resource list to prevent deadlock
		Resources.Sort([](const FIPGExclusiveResource& A, const FIPGExclusiveResource& B)
		{
				return &A < &B;
		});

		TArray<UE::Tasks::FTask> AllPrerequisites;
		if (Prerequisites.IsValid())
		{
			AllPrerequisites.Add(Prerequisites); 
		}

		// 2) Create completion event 
		UE::Tasks::FTaskEvent CompletionEvent(DebugName); 

		// 3) Call exchange() made safe with FScopeLock
		for (FIPGExclusiveResource* Resource : Resources)
		{
			if (Resource)
			{
				UE::Tasks::FTask PrevTask = Resource->ExchangeTailTask(CompletionEvent); 
				if (PrevTask.IsValid())
				{
					AllPrerequisites.Add(PrevTask);
				}
			}
		}

		const FString WorkScopeName = FString(DebugName) + TEXT(":Work");

		// 4) Launch after resolving dependencies, then route thread
		UE::Tasks::FTask LaunchHandle = UE::Tasks::Launch(UE_SOURCE_LOCATION, [Body = MoveTemp(Body), ThreadMode, CompletionEvent]() mutable
			{
				// Game Thread
				if (ThreadMode == EIPGThreadMode::GameThread)
				{
					AsyncTask(ENamedThreads::GameThread, [Body = MoveTemp(Body), CompletionEvent]() mutable
						{
							if (Body)
							{
								Body();
							}

							CompletionEvent.Trigger();
						});
				}
				// Worker Thread
				else
				{
					if (Body)
					{
						Body();
					}
					CompletionEvent.Trigger();
				}
			}, AllPrerequisites);

		// 5) Prevent deadlock caused by launch failure
		if (!LaunchHandle.IsValid())
		{
			// If a task launch fails due to a system error, 
			// Immediately release the event tied to the resource to prevent subsequent tasks from being blocked
			CompletionEvent.Trigger();
		}

		return CompletionEvent;
	}
}