// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/EquipmentActor.h"
#if UE_WITH_IRIS
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Iris/ReplicationSystem/NetRefHandle.h"
#endif

AEquipmentActor::AEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

FGameplayTag AEquipmentActor::GetEquipmentType() const
{
	return EquipmentTypeTag;
}

void AEquipmentActor::SetEquipmentType(FGameplayTag Tag)
{
	EquipmentTypeTag = Tag;
}

void AEquipmentActor::BeginPlay()
{
	Super::BeginPlay();

#if UE_WITH_IRIS
	if (HasAuthority())
	{
		ApplyDistancePriority();
		if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
		{
			if (PC->GetNetConnection())
			{
				SuppressReplicationToOwner(PC);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Player Controller is not valid in AEquipmentActor::BeginPlay()"));
			}
		}
	}
#endif
}

void AEquipmentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if UE_WITH_IRIS
	ClearConnectionFilter();
#endif
	Super::EndPlay(EndPlayReason);
}

#if UE_WITH_IRIS
void AEquipmentActor::ApplyDistancePriority()
{
	// Check if replication system is valid
	UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(this);
	if (!IsValid(ReplicationSystem))
	{
		return;
	}

	// Check if replication bridge is valid
	UEngineReplicationBridge* ReplicationBridge = UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(this);
	if (!IsValid(ReplicationBridge))
	{
		return;
	}

	// Check if equipment actor's net handle is valid
	UE::Net::FNetRefHandle ObjectNetHandle = ReplicationBridge->GetReplicatedRefHandle(this);
	if (!ObjectNetHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Object Net Handle is Not Valid in UInventoryItem::ApplyIrisOwnerFilter()"));
		return;
	}
	
	// Get prioritizer handle
	// "Sphere with Owner Boost" 
	UE::Net::FNetObjectPrioritizerHandle PrioritizerHandle =
		ReplicationSystem->GetPrioritizerHandle(FName("SphereWithOwnerBoostNetObjectPrioritizer"));

	// Set prioritizer handle, but if it doesn't work, set static priority
	if (!ReplicationSystem->SetPrioritizer(ObjectNetHandle, PrioritizerHandle))
	{
		ReplicationSystem->SetStaticPriority(ObjectNetHandle, 2.f);
	}
}

void AEquipmentActor::SuppressReplicationToOwner(APlayerController* OwnerPC)
{
	if (!OwnerPC->GetNetConnection())
	{
		return;
	}

	// Check if replication system is valid
	UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(this);
	if (!IsValid(ReplicationSystem))
	{
		return;
	}

	// Check if replication bridge is valid
	UEngineReplicationBridge* ReplicationBridge = UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(this);
	if (!IsValid(ReplicationBridge))
	{
		return;
	}

	// Check if equipment actor's net handle is valid
	UE::Net::FNetRefHandle ObjectNetHandle = ReplicationBridge->GetReplicatedRefHandle(this);
	if (!ObjectNetHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Object Net Handle is Not Valid in AEquipmentActor::SuppressReplicationToOwner()"));
		return;
	}

	// 1. Create a group 
	FName GroupName = FName(*FString::Printf(TEXT("EquipmentOwnerFilter_%llu"), ObjectNetHandle.GetId())); 
	UE::Net::FNetObjectGroupHandle GroupHandle = ReplicationSystem->CreateGroup(GroupName);
	if (!GroupHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Group Handle is Not Valid in AEquipmentActor::SuppressReplicationToOwner()"));
		return;
	}

	// 2. Register the group to exclusion filter
	if (!ReplicationSystem->AddExclusionFilterGroup(GroupHandle))
	{
		UE_LOG(LogTemp, Error, TEXT("Exclusion Filter Group is Not Valid in AEquipmentActor::SuppressReplicationToOwner()"));
		ReplicationSystem->DestroyGroup(GroupHandle);
		return;
	}

	// 3. Add equipment actor to group
	ReplicationSystem->AddToGroup(GroupHandle, ObjectNetHandle);

	// 4. Setup 'Disallow' setting on owner connection
	uint32 ConnectionID = OwnerPC->GetNetConnection()->GetConnectionHandle().GetParentConnectionId(); 
	ReplicationSystem->SetGroupFilterStatus(GroupHandle, ConnectionID, UE::Net::ENetFilterStatus::Disallow);

	// 5. Save the handle when disconnected
	ConnectionFilterGroupHandle = GroupHandle;
}

void AEquipmentActor::ClearConnectionFilter()
{
	if (!ConnectionFilterGroupHandle.IsValid())
	{
		return;
	}

	// Check if replication system is valid
	UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(this);
	if (!IsValid(ReplicationSystem))
	{
		return;
	}

	ReplicationSystem->DestroyGroup(ConnectionFilterGroupHandle);
	ConnectionFilterGroupHandle = UE::Net::FNetObjectGroupHandle();
}
#endif
