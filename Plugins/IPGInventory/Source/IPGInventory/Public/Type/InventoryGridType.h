// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryGridType.generated.h"

class UInventoryItem;

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Equippable, 
	Consumable,
	Craftable,
	None
};

USTRUCT()
struct FSlotAvailability
{
	GENERATED_BODY()

public:
	FSlotAvailability() {}
	FSlotAvailability(int32 ItemIndex, int32 Room, bool bHasItem)
		: Index(ItemIndex), AmountToFill(Room), bItemAtIndex(bHasItem) {}

	int32 Index = INDEX_NONE;
	int32 AmountToFill = 0; 
	bool bItemAtIndex = false;
};

USTRUCT()
struct FSlotAvailabilityResult
{
	GENERATED_BODY()

public:
	FSlotAvailabilityResult() {}

	TWeakObjectPtr<UInventoryItem> Item;
	int32 TotalRoomToFill = 0; 
	int32 Remainder = 0; 
	bool bStackable = false; 
	TArray<FSlotAvailability> SlotAvailabilities;
};

UENUM(BlueprintType)
enum class ETileQuadrant : uint8
{
	TopLeft, 
	TopRight, 
	BottomLeft, 
	BottomRight,
	None,
};

USTRUCT(BlueprintType)
struct FTileParameters
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	FIntPoint TileCoordinates = FIntPoint();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	int32 TileIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	ETileQuadrant TileQuadrant = ETileQuadrant::None;
};

inline bool operator==(const FTileParameters& A, const FTileParameters& B)
{
	return A.TileCoordinates == B.TileCoordinates && A.TileIndex == B.TileIndex && A.TileQuadrant == B.TileQuadrant;
}

USTRUCT()
struct FSpaceQueryResult
{
	GENERATED_BODY()

	// True if the space queried has no items in it
	bool bHasSpace = false;

	// Valid if there's a single item we can swap with
	TWeakObjectPtr<UInventoryItem> ValidItem = nullptr;

	// Upper left index of the valid item, if there is one
	int32 UpperLeftIndex = INDEX_NONE;
};