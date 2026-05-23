// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Type/InventoryGridType.h"
#include "GameplayTagContainer.h"
#include "ItemManifest.generated.h"

class UInventoryItem;
struct FItemFragment;

/**
 * 
 */
USTRUCT(BlueprintType)
struct IPGINVENTORY_API FItemManifest
{
	GENERATED_BODY()

public:
	TArray<TInstancedStruct<FItemFragment>>& GetFragmentsMutable() { return Fragments; }
	EItemCategory GetItemCategory() const { return ItemCategory; }
	FGameplayTag GetItemType() const { return ItemType; }

	template<typename T> requires std::derived_from<T, FItemFragment>
	const T* GetFragmentOfTypeWithTag(const FGameplayTag& FragmentTag) const;

	template<typename T> requires std::derived_from<T, FItemFragment>
	const T* GetFragmentOfType() const; 

	template<typename T> requires std::derived_from<T, FItemFragment>
	T* GetFragmentOfTypeMutable(); 

	template<typename T> requires std::derived_from<T, FItemFragment>
	TArray<const T*> GetAllFragmentsOfType() const; 

	UInventoryItem* Manifest(UObject* NewOuter);

	void SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation);

private:
	void ClearFragments(); 

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FItemFragment>> Fragments; 

	UPROPERTY(EditAnywhere, Category = "Inventory")
	EItemCategory ItemCategory = EItemCategory::None;

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "GameItems"))
	FGameplayTag ItemType;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<AActor> PickupActorClass;
};

template<typename T> requires std::derived_from<T, FItemFragment>
inline const T* FItemManifest::GetFragmentOfTypeWithTag(const FGameplayTag& FragmentTag) const
{
	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			if (!FragmentPtr->GetFragmentTag().MatchesTagExact(FragmentTag))
			{
				continue;
			}
			return FragmentPtr;
		}
	}
	return nullptr;
}

template<typename T> requires std::derived_from<T, FItemFragment>
inline const T* FItemManifest::GetFragmentOfType() const
{
	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	return nullptr;
}

template<typename T> requires std::derived_from<T, FItemFragment>
inline T* FItemManifest::GetFragmentOfTypeMutable()
{
	for (TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			return FragmentPtr;
		}
	}

	return nullptr;
}

template<typename T> requires std::derived_from<T, FItemFragment>
inline TArray<const T*> FItemManifest::GetAllFragmentsOfType() const
{
	TArray<const T*> Result;

	for (const TInstancedStruct<FItemFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			Result.Add(FragmentPtr);
		}
	}
	return Result;
}
