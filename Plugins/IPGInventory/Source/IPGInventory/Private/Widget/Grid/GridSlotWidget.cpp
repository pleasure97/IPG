// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Grid/GridSlotWidget.h"
#include "CommonLazyImage.h"

/* "TILE INDEX" Setter & Getter */
void UGridSlotWidget::SetTileIndex(int32 Index)
{
	TileIndex = Index;
}

int32 UGridSlotWidget::GetTileIndex() const
{
	return TileIndex;
}

/* "STACK COUNT" Setter & Getter */
int32 UGridSlotWidget::GetStackCount() const
{
	return StackCount;
}

void UGridSlotWidget::SetStackCount(int32 Count)
{
	StackCount = Count;
}

/* "UPPER LEFT INDEX" Setter & Getter */
int32 UGridSlotWidget::GetUpperLeftIndex() const
{
	return UpperLeftIndex;
}

void UGridSlotWidget::SetUpperLeftIndex(int32 Index)
{
	UpperLeftIndex = Index;
}

/* "GRID SLOT STATE" Getter */
EGridSlotState UGridSlotWidget::GetGridSlotState() const
{
	return GridSlotState;
}

/* "INVENTORY ITEM" Setter & Getter */
TWeakObjectPtr<UInventoryItem> UGridSlotWidget::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UGridSlotWidget::SetInventoryItem(UInventoryItem* Item)
{
	InventoryItem = Item;
}

/* "AVAILABLE" Setter & Getter */
bool UGridSlotWidget::IsAvailable() const
{
	return bAvailable;
}

void UGridSlotWidget::SetAvailable(bool bIsAvailable)
{
	bAvailable = bIsAvailable;
}

/* "ITEM POP UP" Setter & Getter */
void UGridSlotWidget::SetItemPopUp(UPopUpItemWidget* PopUpItem)
{
	ItemPopUp = PopUpItem;
}

UPopUpItemWidget* UGridSlotWidget::GetItemPopUp() const
{
	return ItemPopUp.Get();
}

/* "Texture" Setters */
void UGridSlotWidget::SetUnoccupiedTexture()
{
	GridSlotState = EGridSlotState::Unoccupied;
	Image_GridSlot->SetBrush(Brush_Unoccupied);
}

void UGridSlotWidget::SetOccupiedTexture()
{
	GridSlotState = EGridSlotState::Occupied;
	Image_GridSlot->SetBrush(Brush_Occupied);
}

void UGridSlotWidget::SetSelectedTexture()
{
	GridSlotState = EGridSlotState::Selected;
	Image_GridSlot->SetBrush(Brush_Selected);
}

void UGridSlotWidget::SetGrayedOutTexture()
{
	GridSlotState = EGridSlotState::GrayedOut;
	Image_GridSlot->SetBrush(Brush_GrayedOut);
}

void UGridSlotWidget::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseEnter(MyGeometry, MouseEvent); 

	GridSlotHovered.Broadcast(TileIndex, MouseEvent);
}

void UGridSlotWidget::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseLeave(MouseEvent); 
	
	GridSlotUnhovered.Broadcast(TileIndex, MouseEvent); 
}

FReply UGridSlotWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	GridSlotClicked.Broadcast(TileIndex, MouseEvent);

    return FReply::Handled();
}

void UGridSlotWidget::OnItemPopUpDestruct(UUserWidget* Menu)
{
	ItemPopUp.Reset();
}
