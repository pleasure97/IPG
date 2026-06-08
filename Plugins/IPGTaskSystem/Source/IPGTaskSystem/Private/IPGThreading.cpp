// Fill out your copyright notice in the Description page of Project Settings.


#include "IPGThreading.h"

class FIPGThreadQueue final : public IIPGThreadQueue
{
public:
	FIPGThreadQueue()
		: ThreadID(FPlatformTLS::GetCurrentThreadId())
		, Event(FPlatformProcess::GetSynchEventFromPool(true))
	{}

	virtual ~FIPGThreadQueue() override
	{
		check(ThreadID == FPlatformTLS::GetCurrentThreadId());
		check(IsComplete());

		FPlatformProcess::ReturnSynchEventToPool(Event);
		Event = nullptr;
	}

	virtual void AddTask(TFunction<void()>&& Task) override
	{
		WorkQueue.Push(new FTaskFunction(Task));
		Event->Trigger();
	}

	bool ProcessThread(int64 WaitCycles)
	{
		check(ThreadID == FPlatformTLS::GetCurrentThreadId());
		bool bDidWork = false;
		bool bTriggered;

		do
		{
			{
				QUICK_SCOPE_CYCLE_COUNTER(STAT_IPG_ProcessThreadQueue);

				while (const FTaskFunction* Function = WorkQueue.Pop())
				{
					(*Function)();
					delete Function;
					bDidWork = true;
				}
			}

			QUICK_SCOPE_CYCLE_COUNTER(STAT_IPG_WaitThreadQueue);
			Event->Reset();
			bTriggered = Event->Wait(FTimespan(WaitCycles));
		} while (bTriggered || !IsComplete());

		return bDidWork;
	}

	bool IsComplete() const { return WorkQueue.IsEmpty(); }

private:
	const uint32 ThreadID;
	TLockFreePointerListFIFO<FTaskFunction, PLATFORM_CACHE_LINE_SIZE> WorkQueue;
	FEvent* Event;
};

static TSharedPtr<FIPGThreadQueue> GIPGThreadQueue;

IIPGThreadQueue& IIPGThreadQueue::Get()
{
	checkf(GIPGThreadQueue.IsValid(), TEXT("Use FIPGThreadScope to set up a Thread Queue!"));
	return *GIPGThreadQueue;
}

FIPGThreadScope::FIPGThreadScope()
{
	check(!GIPGThreadQueue.IsValid());
	GIPGThreadQueue = MakeShared<FIPGThreadQueue>();
}

FIPGThreadScope::~FIPGThreadScope()
{
	check(GIPGThreadQueue.IsValid());
	GIPGThreadQueue.Reset();
}

bool FIPGThreadScope::ProcessThread(int64 WaitCycles) const
{
	return GIPGThreadQueue->ProcessThread(WaitCycles);
}
