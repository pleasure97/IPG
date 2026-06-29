// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Iris/ReplicationSystem/Prioritization/FieldOfViewNetObjectPrioritizer.h"
#include "Iris/IPGNetPrioritizerDataAsset.h"
#include "DynamicFoVNetObjectPrioritizer.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API UDynamicFoVNetObjectPrioritizer : public UFieldOfViewNetObjectPrioritizer
{
	GENERATED_BODY()

public:
	const FFoVPrioritizerPreset* GetConnectionPreset(uint32 ConnectionId) const;
	void SetConnectionPreset(uint32 ConnectionId, const FFoVPrioritizerPreset& FoVPrioritizerPreset);
	void ClearConnectionPreset(uint32 ConnectionId); 
	const FFoVPrioritizerPreset GetGlobalPreset() const;
	void SetGlobalPreset(const FFoVPrioritizerPreset& FoVPrioritizerPreset);

protected:
	virtual void Init(FNetObjectPrioritizerInitParams& Params) override;
	virtual void AddConnection(uint32 ConnectionId) override;
	virtual void RemoveConnection(uint32 ConnectionId) override;
	virtual void Prioritize(FNetObjectPrioritizationParams& Params) override;

private:
	void ApplyPresetToConfig(const FFoVPrioritizerPreset& FoVPrioritizerPreset);

	TObjectPtr<UFieldOfViewNetObjectPrioritizerConfig> ExposedConfig;

	TMap<uint32, FFoVPrioritizerPreset> PerConnectionPresets;

	TSet<uint32> ConnectionsWithZonePreset;

	FFoVPrioritizerPreset GlobalFoVPrioritizerPreset;

	bool bHasGlobalFoVPrioritizerPreset = false;
};