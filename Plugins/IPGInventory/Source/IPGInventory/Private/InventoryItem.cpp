// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "Net/UnrealNetwork.h"
#include "Fragment/ItemFragment.h"

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
}

bool UInventoryItem::IsStackable() const
{
	const FStackableFragment* Stackable = GetItemManifest().GetFragmentOfType<FStackableFragment>();
	return false;
}

bool UInventoryItem::IsConsumable() const
{
	return GetItemManifest().GetItemCategory() == EItemCategory::Consumable;
}