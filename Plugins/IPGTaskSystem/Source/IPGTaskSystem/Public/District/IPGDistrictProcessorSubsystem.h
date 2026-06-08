// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "District/IPGDistrictProcessor.h"
#include "IPGDistrictProcessorSubsystem.generated.h"

class FIPGGameSectionProcessor : public IIPGSectionProcessor
{
public:
	virtual void ProcessSection(const FDistrictSection& Section, float DeltaTime) override;
};

/**
 * 
 */
UCLASS()
class IPGTASKSYSTEM_API UIPGDistrictProcessorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void Tick(float DeltaTime);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
private:
	FIntPoint GetNumDistricts() const;

	TUniquePtr<FIPGDistrictProcessor> DistrictProcessor;
};
