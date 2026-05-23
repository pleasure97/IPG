// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryArray.generated.h"

struct FInventoryArray;
class UInventoryComponent;
class UItemComponent;
class UInventoryItem;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FInventoryEntry() {}

	bool operator==(const FInventoryEntry& Other) const
	{
		return Item == Other.Item;
	}
private:
	friend FInventoryArray;
	friend UInventoryComponent;

	UPROPERTY()
	TObjectPtr<UInventoryItem> Item = nullptr;
};

// List of inventory items
USTRUCT(BlueprintType)
struct FInventoryArray : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FInventoryArray()
		: OwnerComponent(nullptr) {}
	
	FInventoryArray(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent) {}

	TArray<UInventoryItem*> GetAllItems() const; 

	/* FFastArraySerializer */
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize); 
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize); 
	/* FFastArraySerializer End */

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInventoryEntry, FInventoryArray>(Entries, DeltaParams, *this);
	}

	/* Item Entry */
	UInventoryItem* AddEntry(UItemComponent* ItemComponent); 
	UInventoryItem* AddEntry(UInventoryItem* Item); 
	void RemoveEntry(UInventoryItem* Item);
	UInventoryItem* FindFirstItemByType(const FGameplayTag& ItemType);

private:
	friend UInventoryComponent;

	// Replicated list of items
	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};