// Fill out your copyright notice in the Description page of Project Settings.


#include "Iris/DynamicFoVNetObjectPrioritizer.h"

const FFoVPrioritizerPreset* UDynamicFoVNetObjectPrioritizer::GetConnectionPreset(uint32 ConnectionId) const
{
	return PerConnectionPresets.Find(ConnectionId);
}

void UDynamicFoVNetObjectPrioritizer::SetConnectionPreset(uint32 ConnectionId, const FFoVPrioritizerPreset& FoVPrioritizerPreset)
{
	PerConnectionPresets.Add(ConnectionId, FoVPrioritizerPreset);
	ConnectionsWithZonePreset.Add(ConnectionId);
}

void UDynamicFoVNetObjectPrioritizer::ClearConnectionPreset(uint32 ConnectionId)
{
	ConnectionsWithZonePreset.Remove(ConnectionId);

	if (bHasGlobalFoVPrioritizerPreset)
	{
		PerConnectionPresets.Add(ConnectionId, GlobalFoVPrioritizerPreset);
	}
	else
	{
		PerConnectionPresets.Remove(ConnectionId);
	}
}

const FFoVPrioritizerPreset UDynamicFoVNetObjectPrioritizer::GetGlobalPreset() const
{
	return GlobalFoVPrioritizerPreset;
}

void UDynamicFoVNetObjectPrioritizer::SetGlobalPreset(const FFoVPrioritizerPreset& FoVPrioritizerPreset)
{
	GlobalFoVPrioritizerPreset = FoVPrioritizerPreset;
	bHasGlobalFoVPrioritizerPreset = true;

	for (auto& Pair : PerConnectionPresets)
	{
		if (!ConnectionsWithZonePreset.Contains(Pair.Key))
		{
			Pair.Value = FoVPrioritizerPreset;
		}
	}
}

void UDynamicFoVNetObjectPrioritizer::Init(FNetObjectPrioritizerInitParams& Params)
{
	Super::Init(Params);

	ExposedConfig = CastChecked<UFieldOfViewNetObjectPrioritizerConfig>(Params.Config);
}

void UDynamicFoVNetObjectPrioritizer::AddConnection(uint32 ConnectionId)
{
	Super::AddConnection(ConnectionId); 

	if (bHasGlobalFoVPrioritizerPreset)
	{
		PerConnectionPresets.Add(ConnectionId, GlobalFoVPrioritizerPreset);
	}
}

void UDynamicFoVNetObjectPrioritizer::RemoveConnection(uint32 ConnectionId)
{
	Super::RemoveConnection(ConnectionId); 

	PerConnectionPresets.Remove(ConnectionId);
	ConnectionsWithZonePreset.Remove(ConnectionId);
}

void UDynamicFoVNetObjectPrioritizer::Prioritize(FNetObjectPrioritizationParams& Params)
{
	if (const FFoVPrioritizerPreset* Preset = PerConnectionPresets.Find(Params.ConnectionId))
	{
		ApplyPresetToConfig(*Preset);
	}
	else if (bHasGlobalFoVPrioritizerPreset)
	{
		ApplyPresetToConfig(GlobalFoVPrioritizerPreset);
	}

	Super::Prioritize(Params);
}

void UDynamicFoVNetObjectPrioritizer::ApplyPresetToConfig(const FFoVPrioritizerPreset& FoVPrioritizerPreset)
{
	/*if (!ExposedConfig)
	{
		return;
	}

	ExposedConfig->InnerSphereRadius = FoVPrioritizerPreset.InnerSphereRadius;
	ExposedConfig->OuterSphereRadius = FoVPrioritizerPreset.OuterSphereRadius;
	ExposedConfig->InnerSpherePriority = FoVPrioritizerPreset.InnerSpherePriority;
	ExposedConfig->OuterSpherePriority = FoVPrioritizerPreset.OuterSpherePriority;
	ExposedConfig->OutsidePriority = FoVPrioritizerPreset.OutsidePriority;
	ExposedConfig->ConeFieldOfViewDegrees = FoVPrioritizerPreset.ConeFieldOfViewDegrees;
	ExposedConfig->ConeLength = FoVPrioritizerPreset.ConeLength;
	ExposedConfig->InnerConeLength = FoVPrioritizerPreset.InnerConeLength;
	ExposedConfig->MaxConePriority = FoVPrioritizerPreset.MaxConePriority;
	ExposedConfig->MinConePriority = FoVPrioritizerPreset.MinConePriority;
	ExposedConfig->LineOfSightWidth = FoVPrioritizerPreset.LineOfSightWidth;
	ExposedConfig->LineOfSightPriority = FoVPrioritizerPreset.LineOfSightPriority;*/

	Config->InnerSphereRadius = FoVPrioritizerPreset.InnerSphereRadius;
	Config->OuterSphereRadius = FoVPrioritizerPreset.OuterSphereRadius;
	Config->InnerSpherePriority = FoVPrioritizerPreset.InnerSpherePriority;
	Config->OuterSpherePriority = FoVPrioritizerPreset.OuterSpherePriority;
	Config->OutsidePriority = FoVPrioritizerPreset.OutsidePriority;
	Config->ConeFieldOfViewDegrees = FoVPrioritizerPreset.ConeFieldOfViewDegrees;
	Config->ConeLength = FoVPrioritizerPreset.ConeLength;
	Config->InnerConeLength = FoVPrioritizerPreset.InnerConeLength;
	Config->MaxConePriority = FoVPrioritizerPreset.MaxConePriority;
	Config->MinConePriority = FoVPrioritizerPreset.MinConePriority;
	Config->LineOfSightWidth = FoVPrioritizerPreset.LineOfSightWidth;
	Config->LineOfSightPriority = FoVPrioritizerPreset.LineOfSightPriority;
}
