// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "IPGInventoryBPLibrary.generated.h"

class APlayerController;
class UInventoryComponent; 
class UInventoryItem;
class UHoverItemWidget;
class UInventoryBaseWidget;

UCLASS()
class UIPGInventoryBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* Inventory */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryComponent* GetInventoryComponent(const APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	static EItemCategory GetItemContegoryFromItemComponent(UItemComponent* ItemComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UHoverItemWidget* GetHoverItem(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void ItemHovered(APlayerController* PC, UInventoryItem* Item); 

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void ItemUnhovered(APlayerController* PC);

	static UInventoryBaseWidget* GetInventoryWidget(APlayerController* PC);

	template<typename T, typename FuncT>
	static void ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function);

	/* Widget Util */
	UFUNCTION(BlueprintCallable, Category = "Widget Util")
	static FVector2D GetWidgetPosition(UWidget* Widget);
	UFUNCTION(BlueprintCallable, Category = "Widget Util")
	static FVector2D GetWidgetSize(UWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "Widget Util")
	static bool IsWithinBounds(const FVector2D& BoundaryPosition, const FVector2D& WidgetSize, const FVector2D& MousePosition); 

	static int32 GetIndexFromPosition(const FIntPoint& Position, const int32 Columns);

	static FIntPoint GetPositionFromIndex(const int32 Index, const int32 Columns);

	static FVector2D GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize, const FVector2D& MousePosition);
};

template<typename T, typename FuncT>
void UIPGInventoryBPLibrary::ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function)
{
	for (int32 j = 0; j < Range2D.Y; ++j)
	{
		for (int32 i = 0; i < Range2D.X; ++i)
		{
			const FIntPoint Coordinates = GetPositionFromIndex(Index, GridColumns) + FIntPoint(i, j); 
			const int32 TileIndex = GetIndexFromPosition(Coordinates, GridColumns); 
			if (Array.IsValidIndex(TileIndex))
			{
				Function(Array[TileIndex]);
			}
		}
	}
}