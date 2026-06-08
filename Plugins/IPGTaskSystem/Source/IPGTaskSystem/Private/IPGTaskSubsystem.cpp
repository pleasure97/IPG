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
			// FTask PreviousTask = Something->DoOperation();

			/*AddNested(Launch(UE_SOURCE_LOCATION, [Something]() mutable))
			{
				Something.Reset();
				TRACE_END_REGION(RegionName);
				UE_LOG(LogIPGSubsystem, Log, TEXT("%s : End"), RegionName);
			}, PreviousTask));*/
		});
}

void UIPGTaskSubsystem::DispatchAnimFrame(TArrayView<FNPCAnimContext> NPCAnimContexts, float DeltaTime)
{
	IPGPipe.Launch(UE_SOURCE_LOCATION, [this, NPCAnimContexts, DeltaTime]
		{
			// Step 1 - Animation Update
			FTask AnimationUpdateTask = Launch(UE_SOURCE_LOCATION, [NPCAnimContexts, DeltaTime] {
				for (FNPCAnimContext& NPCAnimContext : NPCAnimContexts)
				{
					//if (NPCAnimContext.should)
				}
				});
		});
}
