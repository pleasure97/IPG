// Fill out your copyright notice in the Description page of Project Settings.


#include "IPGTaskSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogIPGSubsystem, Log, All);

using namespace UE::Tasks;

void UIPGTaskSubsystem::LaunchGraph()
{
	constexpr TCHAR RegionName[] = TEXT("IPGTaskSystem"); 
	UE_LOG(LogIPGSubsystem, Log, TEXT("%s Begin"), RegionName); 
	TRACE_BEGIN_REGION(RegionName);

	// FPipe - Serialization of asynchronous tasks
	// A channel that ensures tasks executable on worker threads are executed in a FIFO manner
	IPGPipe.Launch(UE_SOURCE_LOCATION, [this]
		{

		});
}

void UIPGTaskSubsystem::RunExclusiveTest()
{
	if (!ResourceA.IsValid())
	{
		ResourceA = MakeShared<FIPGExclusiveResource>();
		ResourceB = MakeShared<FIPGExclusiveResource>();
		ResourceC = MakeShared<FIPGExclusiveResource>();
	}

	// Independent log for every call
	TSharedPtr<TArray<FString>>  RunLog = MakeShared<TArray<FString>>();
	TSharedPtr<FCriticalSection> RunLock = MakeShared<FCriticalSection>();

	auto Log = [RunLog, RunLock](const FString& Name)
		{
			FScopeLock ScopeLock(RunLock.Get());
			UE_LOG(LogTemp, Log, TEXT("[Exclusive Task Scenario] %s executed"), *Name);
			RunLog->Add(Name);
			FPlatformProcess::Sleep(0.01f);
		};

	using namespace UE::Tasks;
	FTask T1 = IPGTaskSystem::LaunchExclusive(TEXT("Task1"), { ResourceA.Get() }, [=] { Log(TEXT("Task1")); });
	FTask T2 = IPGTaskSystem::LaunchExclusive(TEXT("Task2"), { ResourceA.Get(), ResourceC.Get(), ResourceB.Get() }, [=] { Log(TEXT("Task2")); });
	FTask T3 = IPGTaskSystem::LaunchExclusive(TEXT("Task3"), { ResourceC.Get() }, [=] { Log(TEXT("Task3")); });
	FTask T4 = IPGTaskSystem::LaunchExclusive(TEXT("Task4"), { ResourceA.Get(), ResourceB.Get() }, [=] { Log(TEXT("Task4")); });
	FTask T5 = IPGTaskSystem::LaunchExclusive(TEXT("Task5"), { ResourceC.Get(), ResourceB.Get() }, [=] { Log(TEXT("Task5")); });

	Launch(UE_SOURCE_LOCATION, [RunLog]
		{
			const bool bOrderOK =
				RunLog->Num() == 5 &&
				RunLog->Find(TEXT("Task1")) < RunLog->Find(TEXT("Task2")) &&
				RunLog->Find(TEXT("Task2")) < RunLog->Find(TEXT("Task3")) &&
				RunLog->Find(TEXT("Task2")) < RunLog->Find(TEXT("Task4")) &&
				RunLog->Find(TEXT("Task3")) < RunLog->Find(TEXT("Task5")) &&
				RunLog->Find(TEXT("Task4")) < RunLog->Find(TEXT("Task5"));

			UE_LOG(LogTemp, Log, TEXT("[Exclusive Task Scenario] Verification: %s"), bOrderOK ? TEXT("PASS") : TEXT("FAIL"));
			RunLog->Reset();
		}, T5);
}

void UIPGTaskSubsystem::RunStressTest(int32 NumTasks, int32 NumResources, float MinWorkMs, float MaxWorkMs)
{
	while (ExclusiveResourcePool.Num() < NumResources)
	{
		ExclusiveResourcePool.Add(MakeShared<FIPGExclusiveResource>());
	}

	const int32 BurstID = StressBurstCounter;
	++StressBurstCounter;
	const FString RegionName = FString::Printf(TEXT("StressBurst_%d"), BurstID);
	TRACE_BEGIN_REGION(*RegionName);

	TArray<UE::Tasks::FTask> BurstTasks;
	BurstTasks.Reserve(NumTasks);

	for (int32 i = 0; i < NumTasks; ++i)
	{
		TArray<FIPGExclusiveResource*> PickedResources;
		const int32 OverlapCount = FMath::RandRange(1, 3);
		for (int32 j = 0; j < OverlapCount; ++j)
		{
			const int32 Idx = FMath::RandRange(0, NumResources - 1);
			PickedResources.AddUnique(ExclusiveResourcePool[Idx].Get());
		}

		const float WorkMs = FMath::RandRange(MinWorkMs, MaxWorkMs);
		const FString TaskName = FString::Printf(TEXT("Stress_%d_%d"), BurstID, i);

		BurstTasks.Add(IPGTaskSystem::LaunchExclusive(
			*TaskName,
			PickedResources,
			[i, WorkMs]
			{
				// Visually assign artificial work time in the Insights timeline
				if (i == 0)
				{
					FPlatformProcess::Sleep(0.05f);
				}
				else
				{
					FPlatformProcess::Sleep(WorkMs / 1000.0f);
				}
			}
		));
	}

	// Return immediately without waiting -> The game thread continues right away
	UE_LOG(LogTemp, Log, TEXT("[StressTest] Burst %d launched: %d tasks over %d resources"),
		BurstID, NumTasks, NumResources);

	TRACE_END_REGION(*RegionName);
}

void UIPGTaskSubsystem::RunDeadlockAvoidanceTest()
{
	if (!DeadlockR1.IsValid())
	{
		DeadlockR1 = MakeShared<FIPGExclusiveResource>();
		DeadlockR2 = MakeShared<FIPGExclusiveResource>();
		DeadlockR3 = MakeShared<FIPGExclusiveResource>();
	}

	using namespace UE::Tasks;

	// Request resources in "reverse order"
	// Without internal pointer-based ordering logic, this can lead to a deadlock caused by a circular wait
	FTask A = IPGTaskSystem::LaunchExclusive(TEXT("Deadlock_A"), { DeadlockR2.Get(), DeadlockR1.Get() },
		[] { UE_LOG(LogTemp, Log, TEXT("[Deadlock] A running")); FPlatformProcess::Sleep(0.02f); });
	FTask B = IPGTaskSystem::LaunchExclusive(TEXT("Deadlock_B"), { DeadlockR3.Get(), DeadlockR2.Get() },
		[] { UE_LOG(LogTemp, Log, TEXT("[Deadlock] B running")); FPlatformProcess::Sleep(0.02f); });
	FTask C = IPGTaskSystem::LaunchExclusive(TEXT("Deadlock_C"), { DeadlockR1.Get(), DeadlockR3.Get() },
		[] { UE_LOG(LogTemp, Log, TEXT("[Deadlock] C running")); FPlatformProcess::Sleep(0.02f); });

	// Prerequisites Task A, B, and C. 
	// If a deadlock occurs, this log will never be printed
	TArray<FTask> All = { A, B, C };
	Launch(UE_SOURCE_LOCATION, [] { UE_LOG(LogTemp, Log, TEXT("[Deadlock] All completed -> NO DEADLOCK")); }, All);

	// Timeout monitoring
	// Perform a blocking wait in a separate thread (not the game thread)
	Launch(UE_SOURCE_LOCATION, [All]
		{
			const bool bCompleted = Wait(All, FTimespan::FromSeconds(3.0));
			if (!bCompleted)
			{
				UE_LOG(LogTemp, Error, TEXT("[Deadlock] TIMEOUT after 3s -> POSSIBLE DEADLOCK"));
			}
		});
}
