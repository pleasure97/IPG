// Fill out your copyright notice in the Description page of Project Settings.


#include "Manifest/ItemManifest.h"
#include "Fragment/ItemFragment.h"
#include "InventoryItem.h"
#include "Component/ItemComponent.h"

UInventoryItem* FItemManifest::Manifest(UObject* NewOuter)
{
	UInventoryItem* InventoryItem = NewObject<UInventoryItem>(NewOuter, UInventoryItem::StaticClass()); 
	InventoryItem->SetItemManifest(*this); 

	for (TInstancedStruct<FItemFragment>& Fragment : InventoryItem->GetItemManifestMutable().GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}

	ClearFragments();

	return InventoryItem;
}

void FItemManifest::SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!IsValid(PickupActorClass) || !IsValid(WorldContextObject))
	{
		return;
	}

	AActor* SpawnedActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass, SpawnLocation, SpawnRotation); 
	if (!IsValid(SpawnedActor))
	{
		return;
	}

	// Set item manifest, item category, item type, etc..
	UItemComponent* ItemComponent = SpawnedActor->FindComponentByClass<UItemComponent>();
	check(ItemComponent);

	ItemComponent->InitItemManifest(*this);
}

void FItemManifest::ClearFragments()
{
	for (auto& Fragment : Fragments)
	{
		Fragment.Reset();
	}
	Fragments.Empty();
}