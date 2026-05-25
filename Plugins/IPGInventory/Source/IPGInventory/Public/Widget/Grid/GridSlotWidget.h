// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GridSlotWidget.generated.h"

class UInventoryItem;
class UPopUpItemWidget;
class UCommonLazyImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent, int32, GridIndex, const FPointerEvent&, MouseEvent);

UENUM(BlueprintType)
enum class EGridSlotState : uint8
{
	Unoccupied,
	Occupied, 
	Selected,
	GrayedOut,
};

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UGridSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	/* "TILE INDEX" Setter & Getter */
	void SetTileIndex(int32 Index); 
	int32 GetTileIndex() const; 

	/* "GRID SLOT STATE" Getter */
	EGridSlotState GetGridSlotState() const; 
	
	/* "INVENTORY ITEM" Setter & Getter */
	TWeakObjectPtr<UInventoryItem> GetInventoryItem() const; 
	void SetInventoryItem(UInventoryItem* Item); 

	/* "STACK COUNT" Setter & Getter */
	int32 GetStackCount() const; 
	void SetStackCount(int32 Count);

	/* "UPPER LEFT INDEX" Setter & Getter */
	int32 GetUpperLeftIndex() const; 
	void SetUpperLeftIndex(int32 Index);

	/* "AVAILABLE" Setter & Getter */
	bool IsAvailable() const; 
	void SetAvailable(bool bIsAvailable); 

	/* "ITEM POP UP" Setter & Getter */
	void SetItemPopUp(UPopUpItemWidget* PopUpItem)	; 
	UPopUpItemWidget* GetItemPopUp() const;

	/* "Texture" Setters */
	void SetUnoccupiedTexture();
	void SetOccupiedTexture();
	void SetSelectedTexture();
	void SetGrayedOutTexture();

	/* Grid Slot Delegates */
	FGridSlotEvent GridSlotClicked;
	FGridSlotEvent GridSlotHovered;
	FGridSlotEvent GridSlotUnhovered;

protected:
	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override; 
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	UFUNCTION()
	void OnItemPopUpDestruct(UUserWidget* Menu);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonLazyImage> Image_GridSlot;

	/* Slate Brush - Unoccupied, Occupied, Selected, GrayedOut */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_Unoccupied;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_Occupied;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_Selected;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_GrayedOut;

	int32 StackCount = 0;
	bool bAvailable = true;
	int32 TileIndex = INDEX_NONE;
	int32 UpperLeftIndex = INDEX_NONE;
	EGridSlotState GridSlotState;

	TWeakObjectPtr<UInventoryItem> InventoryItem;
	TWeakObjectPtr<UPopUpItemWidget> ItemPopUp;
};
