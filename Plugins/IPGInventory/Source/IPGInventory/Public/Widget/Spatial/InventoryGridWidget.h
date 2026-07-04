// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Type/InventoryGridType.h"
#include "Manifest/ItemManifest.h"
#include "GameplayTagContainer.h"
#include "Widget/Grid/GridSlotWidget.h"
#include "InventoryGridWidget.generated.h"

class UItemComponent;
class UCanvasPanel;
class UHoverItemWidget;
class UInventoryItem;
class UInventoryComponent;
class USlotItemWidget;
class UPopUpItemWidget;
struct FGridFragment;
struct FImageFragment;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UInventoryGridWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	/* Getters */
	EItemCategory GetItemCategory() const;
	FSlotAvailabilityResult HasRoomForItem(const UItemComponent* ItemComponent);
	float GetTileSize() const;

	/* Cursor */
	void ShowCursor(); 
	void HideCursor(); 

	void OnHide();

	void SetOwningCanvas(UCanvasPanel* OwningCanvas);

	/* Hover Item */
	bool HasHoveredItem() const;
	UHoverItemWidget* GetHoverItem() const; 
	void ClearHoverItem();
	void AssignHoverItem(UInventoryItem* InventoryItem);
	
	/* Item */
	UFUNCTION()
	void AddItem(UInventoryItem* Item); 
	void DropItem();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/* Grid */
	void ConstructGrid();
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const; 
	void RemoveItemFromGrid(UInventoryItem* Item, const int32 GridIndex);

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TArray<TObjectPtr<UGridSlotWidget>> GridSlots;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UGridSlotWidget> GridSlotClass;

	/* Has Room */
	FSlotAvailabilityResult HasRoomForItem(const UInventoryItem* Item, const int32 StackAmountOverride = -1); 
	FSlotAvailabilityResult HasRoomForItem(const FItemManifest& Manifest, const int32 StackAmountOverride = -1); 
	bool HasRoomAtIndex(const UGridSlotWidget* GridSlot, const FIntPoint& Dimensions, const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed, const FGameplayTag& ItemType, const int32 MaxStackSize);

	/* Add Item */
	void AddItemAtIndex(UInventoryItem* Item, const int32 Index, const bool bIsStackable, const int32 StackAmount); 
	void AddItemToIndices(const FSlotAvailabilityResult& Result, UInventoryItem* NewItem);
	void AddSlotItemToCanvas(const int32 Index, const FGridFragment* GridFragment, USlotItemWidget* SlotItem) const;

	/* Category */
	bool MatchesCategory(const UInventoryItem* Item); 
	bool DoesItemTypeMatch(const UInventoryItem* SubItem, const FGameplayTag& ItemType) const; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	EItemCategory ItemCategory;

	/* Slot */
	USlotItemWidget* CreateSlotItem(UInventoryItem* Item, const bool bIsStackable, const int32 StackAmount, const FGridFragment* GridFragment, const FImageFragment* ImageFragment, const int32 Index); 
	void SetSlotItemImage(const USlotItemWidget* SlotItem, const FGridFragment* GridFragment, const FImageFragment* ImageFragment) const;
	void UpdateGridSlots(UInventoryItem* NewItem, const int32 Index, bool bIsStackable, const int32 StackAmount); 
	bool CheckSlotConstraints(const UGridSlotWidget* GridSlot, const UGridSlotWidget* SubGridSlot, const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed, const FGameplayTag& ItemType, const int32 MaxStackSize); 
	bool IsUpperLeftSlot(const UGridSlotWidget* GridSlot, const UGridSlotWidget* SubGridSlot) const;
	int32 DetermineFillAmountForSlot(const bool bIsStackable, const int32 MaxStackSize, const int32 AmountToFill, const UGridSlotWidget* GridSlot) const;
	void HighlightSlots(const int32 Index, const FIntPoint& Dimensions); 
	void UnhighlightSlots(const int32 Index, const FIntPoint& Dimensions); 

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<USlotItemWidget> SlotItemClass;

	UPROPERTY()
	TMap<int32, TObjectPtr<USlotItemWidget>> SlotItems;

	/* Stack */
	int32 GetStackAmount(const UGridSlotWidget* GridSlot) const; 
	bool IsSameStackable(const UInventoryItem* ClickedInventoryItem) const;
	bool ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount, const int32 MaxStackSize) const;
	void SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index);
	bool ShouldConsumeHoverItemStacks(const int32 HoveredStackCount, const int32 RoomInClickedSlot) const;
	bool ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const;
	void FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index);
	void ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index);

	UFUNCTION()
	void AddStacks(const FSlotAvailabilityResult& SlotAvailabilityResult);

	/* Cilck */
	bool IsRightClick(const FPointerEvent& MouseEvent) const;
	bool IsLeftClick(const FPointerEvent& MouseEvent) const;
	void PickUp(UInventoryItem* ClickedItem, const int32 GridIndex);

	UFUNCTION()
	void OnSlotItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent); 

	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

	/* Util */
	FVector2D GetDrawSize(const FGridFragment* GridFragment) const;
	FIntPoint GetItemDimensions(const FItemManifest& Manifest) const; 
	bool HasValidItem(const UGridSlotWidget* GridSlot) const; 
	FIntPoint CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions, const ETileQuadrant TileQuadrant) const;
	void PutDownOnIndex(const int32 Index);
	bool IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const;

	/* Hover Item */
	void AssignHoverItem(UInventoryItem* Item, const int32 GridIndex, const int32 PreviousGridIndex); 
	FIntPoint CalculateHoveredCoordinates(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const; 
	FSpaceQueryResult CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions); 
	void ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EGridSlotState InGridSlotState); 
	void SwapWithHoverItem(UInventoryItem* ClickedItem, const int32 GridIndex);
	void PutHoverItemBack();

	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent); 

	UFUNCTION()
	void OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent);

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UHoverItemWidget> HoverItemClass;

	UPROPERTY()
	TObjectPtr<UHoverItemWidget> HoverItem;

	/* Tile */
	void UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition); 
	ETileQuadrant CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const;
	void OnTileParametersUpdated(const FTileParameters& InTileParameters);

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Rows;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 Columns;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float TileSize;

	FTileParameters TileParameters;
	FTileParameters LastTileParameters;

	/* Cursor */
	bool CursorExitedCanvas(const FVector2D& BoundaryPosition, const FVector2D& BoundarySize, const FVector2D& Location); 

	/* Pop Up */
	void CreateItemPopUp(const int32 GridIndex);

	UFUNCTION()
	void OnPopUpMenuSplit(int32 SplitAmount, int32 Index);

	UFUNCTION()
	void OnPopUpMenuDrop(int32 Index); 

	UFUNCTION()
	void OnPopUpMenuConsume(int32 Index);

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UPopUpItemWidget> PopUpItemClass;

	UPROPERTY()
	TObjectPtr<UPopUpItemWidget> PopUpItem;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D ItemPopUpOffset;

	/* Toggle */
	UFUNCTION()
	void OnInventoryMenuToggled(bool bOpen); 

	/* Canvas Panel */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	TWeakObjectPtr<UCanvasPanel> OwningCanvasPanel;

	bool bMouseWithinCanvas;
	bool bLastMouseWithinCanvas;

	int32 ItemDropIndex = INDEX_NONE;
	FSpaceQueryResult CurrentQueryResult;
	int32 LastHighlightedIndex;
	FIntPoint LastHighlightedDimensions;
	TWeakObjectPtr<UInventoryComponent> InventoryComponent;
};
