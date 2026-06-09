// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "Fragment/ItemFragment.h"
#include "Net/Core/PushModel/PushModel.h" 
#if UE_WITH_IRIS
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Iris/ReplicationSystem/NetRefHandle.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#endif

void UInventoryItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Iris Push Model
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UInventoryItem, ItemManifest, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UInventoryItem, TotalStackCount, Params);
}

void UInventoryItem::SetItemManifest(const FItemManifest& Manifest)
{
	ItemManifest = FInstancedStruct::Make<FItemManifest>(Manifest);
	MARK_PROPERTY_DIRTY_FROM_NAME(UInventoryItem, ItemManifest, this);
}

bool UInventoryItem::IsStackable() const
{
	const FStackableFragment* Stackable = GetItemManifest().GetFragmentOfType<FStackableFragment>();
	return false;
}

void UInventoryItem::SetTotalStackCount(int32 Count)
{
	TotalStackCount = Count;
	MARK_PROPERTY_DIRTY_FROM_NAME(UInventoryItem, TotalStackCount, this);
}

bool UInventoryItem::IsConsumable() const
{
	return GetItemManifest().GetItemCategory() == EItemCategory::Consumable;
}

#if UE_WITH_IRIS
void UInventoryItem::ApplyIrisOwnerFilter(AActor* OwnerActor)
{
	// Check if owner actor has authority
	if (!IsValid(OwnerActor) || !OwnerActor->HasAuthority())
	{
		return;
	}

	// Check if replication system is valid
	UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(OwnerActor);
	if (!IsValid(ReplicationSystem))
	{
		return;
	}

	// Check if replication bridge is valid
	UEngineReplicationBridge* ReplicationBridge = UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(OwnerActor);
	if (!IsValid(ReplicationBridge))
	{
		return;
	}

	// Check if inventory item's net handle is valid
	UE::Net::FNetRefHandle ObjectNetHandle = ReplicationBridge->GetReplicatedRefHandle(OwnerActor);
	if (!ObjectNetHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Object Net Handle is Not Valid in UInventoryItem::ApplyIrisOwnerFilter()"));
		return;
	}
	ReplicationSystem->SetFilter(ObjectNetHandle, UE::Net::ToOwnerFilterHandle);
}
#endif