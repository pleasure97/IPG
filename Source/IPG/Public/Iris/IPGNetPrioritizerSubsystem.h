// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Iris/IPGNetPrioritizerDataAsset.h"
#include "IPGNetPrioritizerSubsystem.generated.h"

class UDynamicFoVNetObjectPrioritizer;

/**
 * 
 */
UCLASS()
class IPG_API UIPGNetPrioritizerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void RegisterNetPrioritizerPreset(UIPGFoVNetPrioritizerDataAsset* NetPrioritizerPreset);
	void ApplyNetPrioritizerPreset(EIPGMapType MapType, float BlendTime = 3.f);
	void ApplyNetPrioritizerPresetForConnection(uint32 ConnectionId, EIPGMapType MapType);	
	void ClearConnectionPreset(uint32 ConnectionId);

	void SetDebugDraw(bool bEnabled);
	bool GetDebugDraw() const;

protected:
	virtual bool ShouldCreateSubsystem(UObject* Object) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	
private:
	UDynamicFoVNetObjectPrioritizer* GetDynamicFoVNetObjectPrioritizer() const;

	void ApplyToConfig(const FFoVPrioritizerPreset& Data);

	void DebugDraw();

	bool bDebugDrawEnabled = false;

	FFoVPrioritizerPreset CurrentData;
	FFoVPrioritizerPreset TargetData;

	float BlendAlpha = 1.f;
	float BlendDuration = 3.f;
	
	bool bHasBeenInitialized = false;

	UPROPERTY()
	TMap<EIPGMapType, UIPGFoVNetPrioritizerDataAsset*> MapPrioritizerMappings;
};
