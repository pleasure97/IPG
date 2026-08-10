// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tasks/Pipe.h"
#include "Exclusive/IPGExclusiveTask.h"
#include "IPGTaskSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class IPGTASKSYSTEM_API UIPGTaskSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="IPGTaskSystem")
	void LaunchGraph();

	UFUNCTION(BlueprintCallable, Category = "IPGTaskSystem", meta = (DisplayName = "Run Exclusive Test"))
	void RunExclusiveTest();

	UFUNCTION(BlueprintCallable, Category="IPGTaskSystem", meta = (DisplayName = "Run Stress Test"))
	void RunStressTest(int32 NumTasks = 200, int32 NumResources = 20, float MinWorkMs = 0.1f, float MaxWorkMs = 1.5f);

	UFUNCTION(BlueprintCallable, Category = "IPGTaskSystem", meta = (DisplayName = "Run Deadlock Avoidance Test"))
	void RunDeadlockAvoidanceTest();

private:
	UE::Tasks::FPipe IPGPipe = UE::Tasks::FPipe(TEXT("IPGTaskSubsystem")); 

	/* Exclusive Task Test*/
	TSharedPtr<FIPGExclusiveResource> ResourceA;
	TSharedPtr<FIPGExclusiveResource> ResourceB;
	TSharedPtr<FIPGExclusiveResource> ResourceC;

	/* Stress Test */
	TArray<TSharedPtr<FIPGExclusiveResource>> ExclusiveResourcePool;
	int32 StressBurstCounter = 0;

	/* Deadlock Test*/
	TSharedPtr<FIPGExclusiveResource> DeadlockR1;
	TSharedPtr<FIPGExclusiveResource> DeadlockR2;
	TSharedPtr<FIPGExclusiveResource> DeadlockR3;
};
