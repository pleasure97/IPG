// Fill out your copyright notice in the Description page of Project Settings.


#include "Task/IPGStateTreeTask.h"
#include "EnhancedInputSubsystems.h"
#include "StateTreeAsyncExecutionContext.h"

const UStruct* FIPGStateTreeTask::GetInstanceDataType() const
{
	return FInstanceDataType::StaticStruct();
}

const UStruct* FIPGStateTreeTask::GetExecutionRuntimeDataType() const
{
	return FExecutionRuntimeDataType::StaticStruct();
}

EStateTreeRunStatus FIPGStateTreeTask::EnterState(FStateTreeExecutionContext& ExecutionContext, const FStateTreeTransitionResult& Transition) const
{
	// Template
	UE_LOG(LogTemp, Log, TEXT("Template value {Template}"), Template);

	// Instance (read-write)
	FInstanceDataType& InstanceData = ExecutionContext.GetInstanceData(*this);
	InstanceData.Output = InstanceData.Input + 1;

	// Runtime (read-write)
	FExecutionRuntimeDataType& ExecutionData = ExecutionContext.GetExecutionRuntimeData(*this);
	++ExecutionData.Runtime;

	// External (read)
	UEnhancedInputWorldSubsystem& InputSubsystem = ExecutionContext.GetExternalData(ExternalInputSubsystem);
	InputSubsystem.AddActorInputComponent(InstanceData.Context);

	AsynchronousExample(ExecutionContext);

	return EStateTreeRunStatus::Running;
}

bool FIPGStateTreeTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ExternalInputSubsystem);
	return true;
}

void FIPGStateTreeTask::AsynchronousExample(FStateTreeExecutionContext& ExecutionContext) const
{
	// BAD! DON'T DO THIS
	// FInstanceDataType& InstanceData = ExecutionContext.GetInstanceData(*this); 
	// ExecuteOnGameThread(TEXT(""), [this, ExecutionContext, InstanceData](){}); 
	FStateTreeWeakExecutionContext WeakExecutionContext = ExecutionContext.MakeWeakExecutionContext();
	ExecuteOnGameThread(TEXT("StateTreeTask"), [WeakExecutionContext]()
		{
			// ExecuteOnGameThread is always called but
			// * the active states may have changed
			// * the task may be completed by something else
			// * the owning actor may have stop the state tree execution
			// * the owning actor can be garbage collected 

			// We do not have the context
			// ExecutionContext.FinishTask(EStateTreeFinishTaskType::Succeeded); 

			// Shortcut
			// WeakExecutionContext.FinishTask(EStateTreeFinishTaskType::Succeeded);

			// Long form 
			FStateTreeStrongExecutionContext StrongExecutionContext = WeakExecutionContext.MakeStrongExecutionContext();
			if (StrongExecutionContext.IsValid())
			{
				StrongExecutionContext.FinishTask(EStateTreeFinishTaskType::Succeeded);

				// Access instance data
				if (FInstanceDataType* InstanceData = StrongExecutionContext.GetInstanceDataPtr<FInstanceDataType>())
				{
					InstanceData->Output = InstanceData->Input + 1;
				};
			}
		});
}
