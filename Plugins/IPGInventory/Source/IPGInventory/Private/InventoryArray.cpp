// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryArray.h"
#include "InventoryItem.h"
#include "Component/InventoryComponent.h"
#include "Component/ItemComponent.h"

TArray<UInventoryItem*> FInventoryArray::GetAllItems() const
{
	TArray<UInventoryItem*> Results;
	Results.Reserve(Entries.Num()); 

	for (const FInventoryEntry& Entry : Entries)
	{
		if (!IsValid(Entry.Item))
		{
			continue;
		}
		Results.Add(Entry.Item);
	}

	return Results;
}

void FInventoryArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(OwnerComponent); 
	if (!IsValid(InventoryComponent))
	{
		return;
	}
	for (int32 Index : RemovedIndices)
	{
		InventoryComponent->OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FInventoryArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(OwnerComponent); 
	if (!IsValid(InventoryComponent))
	{
		return;
	}
	for (int32 Index : AddedIndices)
	{
		InventoryComponent->OnItemAdded.Broadcast(Entries[Index].Item);
	}
}

UInventoryItem* FInventoryArray::AddEntry(UItemComponent* ItemComponent)
{
	check(OwnerComponent); 

	AActor* OwningActor = OwnerComponent->GetOwner(); 
	check(OwningActor->HasAuthority()); 

	UInventoryComponent* InventoryComponent = Cast<UInventoryComponent>(OwnerComponent); 
	if (!IsValid(InventoryComponent))
	{
		return nullptr;
	}

	FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef(); 
	NewEntry.Item = ItemComponent->GetItemManifest().Manifest(OwningActor);

	if (InventoryComponent->IsUsingRegisteredSubObjectList() && InventoryComponent->IsReadyForReplication() && IsValid(NewEntry.Item))
	{
		InventoryComponent->AddReplicatedSubObject(NewEntry.Item);
	}
	MarkItemDirty(NewEntry);

	return NewEntry.Item;
}

UInventoryItem* FInventoryArray::AddEntry(UInventoryItem* Item)
{
	check(OwnerComponent); 

	AActor* OwningActor = OwnerComponent->GetOwner(); 
	check(OwningActor->HasAuthority()); 

	FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef(); 
	NewEntry.Item = Item;

	MarkItemDirty(NewEntry);

	return Item;
}

void FInventoryArray::RemoveEntry(UInventoryItem* Item)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

UInventoryItem* FInventoryArray::FindFirstItemByType(const FGameplayTag& ItemType)
{
	FInventoryEntry* FoundEntry = Entries.FindByPredicate([ItemType = ItemType](const FInventoryEntry& Entry)
		{
			return IsValid(Entry.Item) && Entry.Item->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
		});
	return FoundEntry ? FoundEntry->Item : nullptr;
}
