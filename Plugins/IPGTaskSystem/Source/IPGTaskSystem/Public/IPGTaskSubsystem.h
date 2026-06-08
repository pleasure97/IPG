// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tasks/Pipe.h"
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
	
private:
	UE::Tasks::FPipe IPGPipe = UE::Tasks::FPipe(TEXT("IPGTaskSubsystem")); 

	//void DispatchAnimFrame(TArrayView<FNPCAnimContext> NPCAnimContexts, float DeltaTime);
};
