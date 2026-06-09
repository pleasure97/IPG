// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Manifest/ItemManifest.h"
#include "InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UInventoryItem : public UObject
{
	GENERATED_BODY()
	
public:
	/* Item Manifest */
	void SetItemManifest(const FItemManifest& Manifest); 
	const FItemManifest& GetItemManifest() const { return ItemManifest.Get<FItemManifest>(); }
	FItemManifest& GetItemManifestMutable() { return ItemManifest.GetMutable<FItemManifest>(); }

	/* Item Stack */
	bool IsStackable() const; 
	int32 GetTotalStackCount() const { return TotalStackCount; }
	void SetTotalStackCount(int32 Count);

	bool IsConsumable() const; 

#if UE_WITH_IRIS
	void ApplyIrisOwnerFilter(AActor* OwnerActor);
#endif

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	
private:
	UPROPERTY(VisibleAnywhere, meta = (BaseStruct = "/Script/Inventory.ItemManifest"), Replicated)
	FInstancedStruct ItemManifest;

	UPROPERTY(Replicated)
	int32 TotalStackCount = 0;
};

template<typename FragmentType>
const FragmentType* GetFragment(const UInventoryItem* Item, const FGameplayTag& Tag)
{
	if (!IsValid(Item))
	{
		return nullptr;
	}

	const FItemManifest& Manifest = Item->GetItemManifest(); 
	return Manifest.GetFragmentOfTypeWithTag<FragmentType>(Tag);
}
