// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class IIPGThreadQueue
{
public:
	using FTaskFunction = TFunction<void()>;

	static IIPGThreadQueue& Get();

	virtual ~IIPGThreadQueue() = default;
	virtual void AddTask(FTaskFunction&& Task) = 0;
};

class FIPGThreadScope
{
public:
	FIPGThreadScope();
	~FIPGThreadScope();

	bool ProcessThread(int64 WaitCycles) const;
};

template <typename FuncType>
void ExecuteJobs(int32 NumJobs, TStatId StatId, FuncType&& Job)
{
	if (NumJobs <= 0)
	{
		return;
	}

	// Thread Scope
	FIPGThreadScope OwnerThreadScope;

	// Job Index & Completed Jobs
	TAtomic<int32> JobIndex = 0;
	TAtomic<int32> CompletedJobs = 0;

	const int32 NumThreads = FMath::Max(1, GThreadPool->GetNumThreads());
	for (int32 ThreadIdx = 0; ThreadIdx < NumThreads; ++ThreadIdx)
	{
		GThreadPool->AddQueuedWork(new TAsyncQueuedWork<void>(
			[&Job, &JobIndex, &CompletedJobs, NumJobs, StatId]
			{
				FScopeCycleCounter Counter(StatId);

				int32 MyJobIdx;
				while ((MyJobIdx = JobIndex.IncrementExchange()) < NumJobs)
				{
					Job(MyJobIdx);
					++CompletedJobs;
				}
			}, TPromise<void>()), EQueuedWorkPriority::Highest);
	}

	while (OwnerThreadScope.ProcessThread(10000) || CompletedJobs.Load() < NumJobs);

	check(CompletedJobs.Load() >= NumJobs);
}