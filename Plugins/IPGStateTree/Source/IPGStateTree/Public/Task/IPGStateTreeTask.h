// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "IPGStateTreeTask.generated.h"

class UEnhancedInputWorldSubsystem;

USTRUCT()
struct FIPGStateTreeInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Input")
	int32 Input = 0;

	UPROPERTY(EditAnywhere, Category = "Context")
	AActor* Context = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	int32 Output = 0;

	UPROPERTY(EditAnywhere, Category = "AnythingElse")
	int32 Parameter = 0;
};

USTRUCT()
struct FIPGStateTreeExecutionRuntime
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Parameter")
	int32 Runtime = 0;
};

USTRUCT()
struct FIPGStateTreeTask : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FIPGStateTreeInstance;
	using FExecutionRuntimeDataType = FIPGStateTreeExecutionRuntime;

	virtual const UStruct* GetInstanceDataType() const override;

	virtual const UStruct* GetExecutionRuntimeDataType() const override;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	int32 Template = 0;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& ExecutionContext, const FStateTreeTransitionResult& Transition) const override;

	virtual bool Link(FStateTreeLinker& Linker) override;

	void AsynchronousExample(FStateTreeExecutionContext& ExecutionContext) const;

	TStateTreeExternalDataHandle<UEnhancedInputWorldSubsystem> ExternalInputSubsystem;
};