// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Spatial/InventoryGridWidget.h"
#include "Component/ItemComponent.h"
#include "Widget/Grid/GridSlotWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Component/InventoryComponent.h"
#include "IPGInventoryBPLibrary.h"
#include "Widget/SlotItem/HoverItemWidget.h"
#include "InventoryItem.h"
#include "IPGInventoryTags.h"
#include "Fragment/ItemFragment.h"
#include "Widget/SlotItem/SlotItemWidget.h"
#include "Widget/SlotItem/PopUpItemWidget.h"

/*----------------------------------------------------- Getters ----------------------------------------------------- */
EItemCategory UInventoryGridWidget::GetItemCategory() const
{
	return ItemCategory;
}

FSlotAvailabilityResult UInventoryGridWidget::HasRoomForItem(const UItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest()); 
}

float UInventoryGridWidget::GetTileSize() const
{
	return TileSize;
}

/*----------------------------------------------------- Cursor ----------------------------------------------------- */
void UInventoryGridWidget::ShowCursor()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC))
	{
		return;
	}

	PC->SetMouseCursorWidget(EMouseCursor::Default, nullptr);
	PC->CurrentMouseCursor = EMouseCursor::Default;
	PC->SetShowMouseCursor(true);
}

void UInventoryGridWidget::HideCursor()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC))
	{
		return;
	}

	PC->SetMouseCursorWidget(EMouseCursor::Default, nullptr);
	PC->CurrentMouseCursor = EMouseCursor::Default;
	PC->SetShowMouseCursor(false);
}

void UInventoryGridWidget::OnHide()
{
	PutHoverItemBack();
}

/*----------------------------------------------------- Canvas ----------------------------------------------------- */
void UInventoryGridWidget::SetOwningCanvas(UCanvasPanel* OwningCanvas)
{
	OwningCanvasPanel = OwningCanvas;
}

/*----------------------------------------------------- Hover Item ----------------------------------------------------- */
bool UInventoryGridWidget::HasHoveredItem() const
{
	return IsValid(HoverItem);
}

UHoverItemWidget* UInventoryGridWidget::GetHoverItem() const
{
	return HoverItem;
}

void UInventoryGridWidget::ClearHoverItem()
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetStackable(false);
	HoverItem->SetPreviousGridIndex(INDEX_NONE); 
	HoverItem->UpdateStackCount(0); 
	HoverItem->SetImageBrush(FSlateNoResource());

	HoverItem->RemoveFromParent(); 
	HoverItem = nullptr;

	ShowCursor();
}

void UInventoryGridWidget::AssignHoverItem(UInventoryItem* InventoryItem)
{
	// Create hover item widget if it's not valid
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UHoverItemWidget>(GetOwningPlayer(), HoverItemClass); 
	}

	// Get grid fragment and image fragment
	const FGridFragment* GridFragment = GetFragment<FGridFragment>(InventoryItem, FragmentTags::GridFragment);
	const FImageFragment* ImageFragment = GetFragment<FImageFragment>(InventoryItem, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment)
	{
		return;
	}

	const FVector2D DrawSize = GetDrawSize(GridFragment); 

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon()); 
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this); 

	HoverItem->SetImageBrush(IconBrush); 
	HoverItem->SetGridDimensions(GridFragment->GetGridSize()); 
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetStackable(InventoryItem->IsStackable()); 

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
}

/*----------------------------------------------------- Item ----------------------------------------------------- */
void UInventoryGridWidget::AddItem(UInventoryItem* Item)
{
	if (!MatchesCategory(Item))
	{
		return;
	}

	FSlotAvailabilityResult SlotAvailabilityResult = HasRoomForItem(Item); 
	AddItemToIndices(SlotAvailabilityResult, Item);
}

void UInventoryGridWidget::DropItem()
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	FSlotAvailabilityResult SlotAvailabilityResult = HasRoomForItem(HoverItem->GetInventoryItem(), HoverItem->GetStackCount()); 
	SlotAvailabilityResult.Item = HoverItem->GetInventoryItem(); 

	AddStacks(SlotAvailabilityResult);
	
	ClearHoverItem();
}

void UInventoryGridWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ConstructGrid();

	// Set inventory component and bind 'item added', 'stack changed', and 'menu toggled' delegates
	InventoryComponent = UIPGInventoryBPLibrary::GetInventoryComponent(GetOwningPlayer()); 
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->OnItemAdded.AddDynamic(this, &UInventoryGridWidget::AddItem);
		InventoryComponent->OnStackChanged.AddDynamic(this, &UInventoryGridWidget::AddStacks);
		InventoryComponent->OnInventoryMenuToggled.AddDynamic(this, &UInventoryGridWidget::OnInventoryMenuToggled);
	}
}

void UInventoryGridWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime); 

	const FVector2D CanvasPosition = UIPGInventoryBPLibrary::GetWidgetPosition(CanvasPanel); 
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()); 

	if (CursorExitedCanvas(CanvasPosition, UIPGInventoryBPLibrary::GetWidgetSize(CanvasPanel), MousePosition))
	{
		return;
	}

	UpdateTileParameters(CanvasPosition, MousePosition);
}

/*----------------------------------------------------- Grid ----------------------------------------------------- */
void UInventoryGridWidget::ConstructGrid()
{
	// Grid Slots [Rows * Columns]
	GridSlots.Reserve(Rows * Columns); 

	// Iterate rows and columns
	for (int32 j = 0; j < Rows; ++j)
	{
		for (int32 i = 0; i < Columns; ++i)
		{
			// Create grid slot and add it to canvas panel
			UGridSlotWidget* GridSlot = CreateWidget<UGridSlotWidget>(this, GridSlotClass); 
			CanvasPanel->AddChild(GridSlot); 

			// Set tile index of each grid slot
			const FIntPoint TilePosition(i, j); 
			int32 TileIndex = UIPGInventoryBPLibrary::GetIndexFromPosition(TilePosition, Columns);
			GridSlot->SetTileIndex(TileIndex);

			// Set size and position of grid slot as 'canvas panel slot'
			UCanvasPanelSlot* GridCanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCanvasPanelSlot->SetSize(FVector2D(TileSize)); 
			GridCanvasPanelSlot->SetPosition(TilePosition * TileSize);

			// Bind 'clicked', 'hovered', and 'unhovered' delegates
			GridSlot->GridSlotClicked.AddDynamic(this, &UInventoryGridWidget::OnGridSlotClicked); 
			GridSlot->GridSlotHovered.AddDynamic(this, &UInventoryGridWidget::OnGridSlotHovered); 
			GridSlot->GridSlotUnhovered.AddDynamic(this, &UInventoryGridWidget::OnGridSlotUnhovered); 

			GridSlots.Add(GridSlot);
		}
	}
}

bool UInventoryGridWidget::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num())
	{
		return false;
	}

	const int32 EndColumn = (StartIndex % Columns) + ItemDimensions.X;
	const int32 EndRow = (StartIndex / Columns) + ItemDimensions.Y;

	return EndColumn <= Columns && EndRow <= Rows;
}

void UInventoryGridWidget::RemoveItemFromGrid(UInventoryItem* Item, const int32 GridIndex)
{
	const FGridFragment* GridFramgent = GetFragment<FGridFragment>(Item, FragmentTags::GridFragment);
	if (!GridFramgent)
	{
		return;
	}

	UIPGInventoryBPLibrary::ForEach2D(GridSlots, GridIndex, GridFramgent->GetGridSize(), Columns,
		[&](UGridSlotWidget* GridSlot)
		{
			if (IsValid(GridSlot))
			{
				GridSlot->SetInventoryItem(nullptr);
				GridSlot->SetUpperLeftIndex(INDEX_NONE);
				GridSlot->SetUnoccupiedTexture(); 
				GridSlot->SetAvailable(true);
				GridSlot->SetStackCount(0);
			}
		});

	if (SlotItems.Contains(GridIndex))
	{
		TObjectPtr<USlotItemWidget> FoundSlotItem;
		SlotItems.RemoveAndCopyValue(GridIndex, FoundSlotItem);
		if (IsValid(FoundSlotItem))
		{
			FoundSlotItem->RemoveFromParent();
		}
	}
}

/*----------------------------------------------------- Has Room ----------------------------------------------------- */
FSlotAvailabilityResult UInventoryGridWidget::HasRoomForItem(const UInventoryItem* Item, const int32 StackAmountOverride)
{
	return HasRoomForItem(Item->GetItemManifest(), StackAmountOverride);
}

FSlotAvailabilityResult UInventoryGridWidget::HasRoomForItem(const FItemManifest& Manifest, const int32 StackAmountOverride)
{
	FSlotAvailabilityResult SlotAvailabilityResult;

	// Determine if the item is stackable
	const FStackableFragment* StackableFragment = Manifest.GetFragmentOfType<FStackableFragment>(); 
	SlotAvailabilityResult.bStackable = (StackableFragment != nullptr); 

	// Determine how many stacks to add
	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = StackableFragment ? StackableFragment->GetStackCount() : 1; 
	if (StackAmountOverride != -1 && SlotAvailabilityResult.bStackable)
	{
		AmountToFill = StackAmountOverride;
	}

	TSet<int32> CheckedIndices;
	// For each grid slot
	for (const auto& GridSlot : GridSlots)
	{
		// If we don't have anymore to fill, break out of the loop early
		if (AmountToFill == 0)
		{
			break;
		}

		// Is this index claimed yet?
		if (IsIndexClaimed(CheckedIndices, GridSlot->GetTileIndex()))
		{
			continue;
		}

		// Is this in grid bounds?
		if (!IsInGridBounds(GridSlot->GetTileIndex(), GetItemDimensions(Manifest)))
		{
			continue;
		}

		// Can the item fit here?
		TSet<int32> TentativelyClaimed;
		if (!HasRoomAtIndex(GridSlot, GetItemDimensions(Manifest), CheckedIndices, TentativelyClaimed, Manifest.GetItemType(), MaxStackSize))
		{
			continue;
		}

		// How much to fill?
		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(SlotAvailabilityResult.bStackable, MaxStackSize, AmountToFill, GridSlot);
		if (AmountToFillInSlot == 0)
		{
			continue;
		}
		CheckedIndices.Append(TentativelyClaimed); 

		// Update the amount left to fill
		SlotAvailabilityResult.TotalRoomToFill += AmountToFillInSlot;
		SlotAvailabilityResult.SlotAvailabilities.Emplace(
			FSlotAvailability{
				HasValidItem(GridSlot) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetTileIndex(),
				SlotAvailabilityResult.bStackable ? AmountToFillInSlot : 0,
				HasValidItem(GridSlot)
			}
		);

		AmountToFill -= AmountToFillInSlot;

		// How much is remainder?
		SlotAvailabilityResult.Remainder = AmountToFill;

		if (AmountToFill == 0)
		{
			return SlotAvailabilityResult;
		}
	}
	return SlotAvailabilityResult;
}

bool UInventoryGridWidget::HasRoomAtIndex(const UGridSlotWidget* GridSlot, const FIntPoint& Dimensions, const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed, const FGameplayTag& ItemType, const int32 MaxStackSize)
{
	// Is there room at this index? (Are there other items in the way?)
	bool bHasRoomAtIndex = true; 
	UIPGInventoryBPLibrary::ForEach2D(GridSlots, GridSlot->GetTileIndex(), Dimensions, Columns,
		[&](const UGridSlotWidget* SubGridSlot)
		{
			if (CheckSlotConstraints(GridSlot, SubGridSlot, CheckedIndices, OutTentativelyClaimed, ItemType, MaxStackSize))
			{
				OutTentativelyClaimed.Add(SubGridSlot->GetTileIndex());
			}
			else
			{
				bHasRoomAtIndex = false;
			}
		});

	return bHasRoomAtIndex;
}

/* Add Item */
void UInventoryGridWidget::AddItemAtIndex(UInventoryItem* Item, const int32 Index, const bool bIsStackable, const int32 StackAmount)
{
	// Get grid fragment and image fragment
	const FGridFragment* GridFragment = GetFragment<FGridFragment>(Item, FragmentTags::GridFragment);
	const FImageFragment* ImageFragment = GetFragment<FImageFragment>(Item, FragmentTags::IconFragment); 
	if (!GridFragment || !ImageFragment)
	{
		return;
	}

	// Create slot item widget based on item, index, and stack
	USlotItemWidget* SlotItem = CreateSlotItem(Item, bIsStackable, StackAmount, GridFragment, ImageFragment, Index);
	if (!IsValid(SlotItem))
	{
		return;
	}

	// Setup slot item information
	SlotItem->SetGridIndex(Index); 
	SlotItem->SetStackable(bIsStackable);
	const int32 StackUpdateAmount = bIsStackable ? StackAmount : 0; 
	SlotItem->UpdateStackCount(StackUpdateAmount); 
	SlotItem->OnSlotItemClicked.AddDynamic(this, &UInventoryGridWidget::OnSlotItemClicked);

	AddSlotItemToCanvas(Index, GridFragment, SlotItem);

	SlotItems.Add(Index, SlotItem);
}

void UInventoryGridWidget::AddItemToIndices(const FSlotAvailabilityResult& Result, UInventoryItem* NewItem)
{
	for (const auto& SlotAvailability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewItem, SlotAvailability.Index, Result.bStackable, SlotAvailability.AmountToFill); 
		UpdateGridSlots(NewItem, SlotAvailability.Index, Result.bStackable, SlotAvailability.AmountToFill);
	}
}

void UInventoryGridWidget::AddSlotItemToCanvas(const int32 Index, const FGridFragment* GridFragment, USlotItemWidget* SlotItem) const
{
	// Add slot item to canvas panel 
	CanvasPanel->AddChild(SlotItem); 
	UCanvasPanelSlot* CanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlotItem);
	// Set canvas panel slot according to draw size of grid fragment
	if (CanvasPanelSlot)
	{
		CanvasPanelSlot->SetSize(GetDrawSize(GridFragment));
	}
	// Set position of canvas panel slot considering padding
	const FVector2D DrawPosition = UIPGInventoryBPLibrary::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPositionWithPadding = DrawPosition + FVector2D(GridFragment->GetGridPadding()); 
	CanvasPanelSlot->SetPosition(DrawPositionWithPadding); 
}

/*----------------------------------------------------- Category ----------------------------------------------------- */
bool UInventoryGridWidget::MatchesCategory(const UInventoryItem* Item)
{
	return Item->GetItemManifest().GetItemCategory() == ItemCategory;
}

bool UInventoryGridWidget::DoesItemTypeMatch(const UInventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetItemType().MatchesTagExact(ItemType);
}

/*----------------------------------------------------- Slot ----------------------------------------------------- */
USlotItemWidget* UInventoryGridWidget::CreateSlotItem(UInventoryItem* Item, const bool bIsStackable, const int32 StackAmount, const FGridFragment* GridFragment, const FImageFragment* ImageFragment, const int32 Index)
{
	USlotItemWidget* SlotItem = CreateWidget<USlotItemWidget>(GetOwningPlayer(), SlotItemClass); 
	SlotItem->SetInventoryItem(Item); 
	SetSlotItemImage(SlotItem, GridFragment, ImageFragment);

	return SlotItem;
}

void UInventoryGridWidget::SetSlotItemImage(const USlotItemWidget* SlotItem, const FGridFragment* GridFragment, const FImageFragment* ImageFragment) const
{
	if (!IsValid(SlotItem) || !GridFragment || !ImageFragment)
	{
		return;
	}

	FSlateBrush SlateBrush;
	SlateBrush.SetResourceObject(ImageFragment->GetIcon()); 
	SlateBrush.DrawAs = ESlateBrushDrawType::Image;
	SlateBrush.ImageSize = GetDrawSize(GridFragment); 

	SlotItem->SetImageBrush(SlateBrush);
}

void UInventoryGridWidget::UpdateGridSlots(UInventoryItem* NewItem, const int32 Index, bool bIsStackable, const int32 StackAmount)
{
	// Check if index is valid in grid slots 
	check(GridSlots.IsValidIndex(Index)); 

	// Set stack count for specific index of grid slot
	if (bIsStackable)
	{
		GridSlots[Index]->SetStackCount(StackAmount);
	}

	// Get grid fragment and dimensions
	const FGridFragment* GridFragment = GetFragment<FGridFragment>(NewItem, FragmentTags::GridFragment); 
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1); 

	UIPGInventoryBPLibrary::ForEach2D(GridSlots, Index, Dimensions, Columns,
		[&](UGridSlotWidget* GridSlot)
		{
			if (IsValid(GridSlot))
			{
				GridSlot->SetInventoryItem(NewItem); 
				GridSlot->SetUpperLeftIndex(Index); 
				GridSlot->SetOccupiedTexture(); 
				GridSlot->SetAvailable(false);
			}
		});
}

bool UInventoryGridWidget::CheckSlotConstraints(const UGridSlotWidget* GridSlot, const UGridSlotWidget* SubGridSlot, const TSet<int32>& CheckedIndices, TSet<int32>& OutTentativelyClaimed, const FGameplayTag& ItemType, const int32 MaxStackSize)
{
	// Is index claimed?
	if (IsIndexClaimed(CheckedIndices, SubGridSlot->GetTileIndex()))
	{
		return false;
	}

	// Has valid item?
	if (!HasValidItem(SubGridSlot))
	{
		OutTentativelyClaimed.Add(SubGridSlot->GetTileIndex()); 
		return true;
	}

	// Is this grid slot an upper left slot?
	if (!IsUpperLeftSlot(GridSlot, SubGridSlot))
	{
		return false;
	}

	// If so, is this stackable?
	const UInventoryItem* SubItem = SubGridSlot->GetInventoryItem().Get(); 
	if (!SubItem->IsStackable())
	{
		return false;
	}

	// Is this item same type as the item we're trying to add?
	if (!DoesItemTypeMatch(SubItem, ItemType))
	{
		return false;
	}

	// If stackable, is this slot at the max stack size already?
	if (GridSlot->GetStackCount() >= MaxStackSize)
	{
		return false;
	}

	return true;
}

bool UInventoryGridWidget::IsUpperLeftSlot(const UGridSlotWidget* GridSlot, const UGridSlotWidget* SubGridSlot) const
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetTileIndex();
}

int32 UInventoryGridWidget::DetermineFillAmountForSlot(const bool bIsStackable, const int32 MaxStackSize, const int32 AmountToFill, const UGridSlotWidget* GridSlot) const
{
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot); 
	return bIsStackable ? FMath::Min(AmountToFill, RoomInSlot) : 1;
}

void UInventoryGridWidget::HighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	// Double check if mouse is within canvas
	if (!bMouseWithinCanvas)
	{
		return;
	}

	UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions); 
	UIPGInventoryBPLibrary::ForEach2D(GridSlots, Index, Dimensions, Columns,
		[&](UGridSlotWidget* GridSlot)
		{
			GridSlot->SetOccupiedTexture();
		}
	);
	LastHighlightedIndex = Index;
	LastHighlightedDimensions = Dimensions;
}

void UInventoryGridWidget::UnhighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UIPGInventoryBPLibrary::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UGridSlotWidget* GridSlot)
		{
			if (GridSlot->IsAvailable())
			{
				GridSlot->SetUnoccupiedTexture();
			}
			else
			{
				GridSlot->SetOccupiedTexture();
			}
		}
	);
}

/*----------------------------------------------------- Stack ----------------------------------------------------- */
int32 UInventoryGridWidget::GetStackAmount(const UGridSlotWidget* GridSlot) const
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount(); 
	// If we are at a slot that doesn't hold the stack count, we must get the actual stack count 
	if (const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		UGridSlotWidget* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
		CurrentSlotStackCount = UpperLeftGridSlot->GetStackCount();
	}
	return CurrentSlotStackCount;
}

bool UInventoryGridWidget::IsSameStackable(const UInventoryItem* ClickedInventoryItem) const
{
	const bool bSameItem = (ClickedInventoryItem == HoverItem->GetInventoryItem()); 
	const bool bStackable = (ClickedInventoryItem->IsStackable()); 
	return bSameItem && bStackable && HoverItem->GetItemType().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

bool UInventoryGridWidget::ShouldSwapStackCounts(const int32 RoomInClickedSlot, const int32 HoveredStackCount, const int32 MaxStackSize) const
{
	return (RoomInClickedSlot == 0) && (HoveredStackCount < MaxStackSize);
}

void UInventoryGridWidget::SwapStackCounts(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index)
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	UGridSlotWidget* GridSlot = GridSlots[Index];
	if (IsValid(GridSlot))
	{
		GridSlot->SetStackCount(HoveredStackCount); 
	}
	USlotItemWidget* ClickedSlotItem = SlotItems.FindChecked(Index); 
	ClickedSlotItem->UpdateStackCount(HoveredStackCount); 

	HoverItem->UpdateStackCount(ClickedStackCount);
}

bool UInventoryGridWidget::ShouldConsumeHoverItemStacks(const int32 HoveredStackCount, const int32 RoomInClickedSlot) const
{
	return RoomInClickedSlot >= HoveredStackCount;
}

bool UInventoryGridWidget::ShouldFillInStack(const int32 RoomInClickedSlot, const int32 HoveredStackCount) const
{
	return RoomInClickedSlot < HoveredStackCount;
}

void UInventoryGridWidget::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index)
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	UGridSlotWidget* GridSlot = GridSlots[Index];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount); 

	USlotItemWidget* ClickedSlotItem = SlotItems.FindChecked(Index); 
	if (ClickedSlotItem)
	{
		ClickedSlotItem->UpdateStackCount(NewStackCount);
	}

	HoverItem->UpdateStackCount(Remainder);
}

void UInventoryGridWidget::ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index)
{
	const int32 NewClickedStackCount = ClickedStackCount + HoveredStackCount;

	GridSlots[Index]->SetStackCount(NewClickedStackCount); 
	SlotItems.FindChecked(Index)->UpdateStackCount(NewClickedStackCount); 

	ClearHoverItem(); 

	ShowCursor();

	const FGridFragment* GridFragment = GridSlots[Index]->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FGridFragment>(); 
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1); 
	HighlightSlots(Index, Dimensions);
}

void UInventoryGridWidget::AddStacks(const FSlotAvailabilityResult& SlotAvailabilityResult)
{
	// Check item manifest's category matches inventory grid widget's category
	if (!MatchesCategory(SlotAvailabilityResult.Item.Get()))
	{
		return;
	}

	// Iterate item slot availability array of item slot result 
	for (const FSlotAvailability& SlotAvailability : SlotAvailabilityResult.SlotAvailabilities)
	{
		// If item already exists, add stacks
		if (SlotAvailability.bItemAtIndex)
		{
			const TObjectPtr<UGridSlotWidget>& GridSlotWidget = GridSlots[SlotAvailability.Index];
			const TObjectPtr<USlotItemWidget>& SlotItemWidget = SlotItems.FindChecked(SlotAvailability.Index);
			// Update slot item widget's stack count
			SlotItemWidget->UpdateStackCount(GridSlotWidget->GetStackCount() + SlotAvailability.AmountToFill);
			// Update grid slot widget's stack count
			GridSlotWidget->SetStackCount(GridSlotWidget->GetStackCount() + SlotAvailability.AmountToFill);
		}
		else
		{
			AddItemAtIndex(SlotAvailabilityResult.Item.Get(), SlotAvailability.Index, SlotAvailabilityResult.bStackable, SlotAvailability.AmountToFill); 
			UpdateGridSlots(SlotAvailabilityResult.Item.Get(), SlotAvailability.Index, SlotAvailabilityResult.bStackable, SlotAvailability.AmountToFill);
		}
	}
}

/*----------------------------------------------------- Click ----------------------------------------------------- */
bool UInventoryGridWidget::IsRightClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
}

bool UInventoryGridWidget::IsLeftClick(const FPointerEvent& MouseEvent) const
{
	return MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
}

void UInventoryGridWidget::PickUp(UInventoryItem* ClickedItem, const int32 GridIndex)
{
	AssignHoverItem(ClickedItem, GridIndex, GridIndex);
	RemoveItemFromGrid(ClickedItem, GridIndex);
}

void UInventoryGridWidget::OnSlotItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	UIPGInventoryBPLibrary::ItemUnhovered(GetOwningPlayer()); 

	check(GridSlots.IsValidIndex(GridIndex)); 

	UInventoryItem* ClickedItem = GridSlots[GridIndex]->GetInventoryItem().Get(); 

	// Unhovered, Left Click
	if (!IsValid(HoverItem) && IsLeftClick(MouseEvent))
	{
		PickUp(ClickedItem, GridIndex);
		return;
	}

	// Right Click
	if (IsRightClick(MouseEvent))
	{
		CreateItemPopUp(GridIndex);
		return;
	}

	// Do hovered item and clicked inventory share a type, and are they stackable?
	if (IsSameStackable(ClickedItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount(); 
		const FStackableFragment* StackableFragment = ClickedItem->GetItemManifest().GetFragmentOfType<FStackableFragment>(); 
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize(); 
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount;
		const int32 HoveredStackCount = HoverItem->GetStackCount(); 

		// Should we swap their stack counts?
		if (ShouldSwapStackCounts(RoomInClickedSlot, HoveredStackCount, MaxStackSize))
		{
			SwapStackCounts(ClickedStackCount, HoveredStackCount, GridIndex); 
			return;
		}

		// Should we consume stacks of hover item? 
		if (ShouldConsumeHoverItemStacks(HoveredStackCount, RoomInClickedSlot))
		{
			ConsumeHoverItemStacks(ClickedStackCount, HoveredStackCount, GridIndex); 
			return;
		}

		// Should we fill in the stacks of the clicked item?
		if (ShouldFillInStack(RoomInClickedSlot, HoveredStackCount))
		{
			FillInStack(RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot, GridIndex);
			return;
		}

		// Clicked slot is already full - TODO (Play Sound)
		if (RoomInClickedSlot == 0)
		{
			return;
		}
	}

	// Make sure we can swap with a valid item
	if (CurrentQueryResult.ValidItem.IsValid())
	{
		// Swap with hover item
		SwapWithHoverItem(ClickedItem, GridIndex);
	}
}

void UInventoryGridWidget::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	if (!GridSlots.IsValidIndex(ItemDropIndex))
	{
		return;
	}

	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		OnSlotItemClicked(CurrentQueryResult.UpperLeftIndex, MouseEvent); 
		return;
	}

	if (!IsInGridBounds(ItemDropIndex, HoverItem->GetGridDimensions()))
	{
		return;
	}

	auto GridSlot = GridSlots[ItemDropIndex];
	if (!GridSlot->GetInventoryItem().IsValid())
	{
		PutDownOnIndex(ItemDropIndex);
	}
}

/*----------------------------------------------------- Util ----------------------------------------------------- */
FVector2D UInventoryGridWidget::GetDrawSize(const FGridFragment* GridFragment) const
{
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2; 

	return GridFragment->GetGridSize() * IconTileWidth;
}

FIntPoint UInventoryGridWidget::GetItemDimensions(const FItemManifest& Manifest) const
{
	const FGridFragment* GridFragment = Manifest.GetFragmentOfType<FGridFragment>(); 
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
}

bool UInventoryGridWidget::HasValidItem(const UGridSlotWidget* GridSlot) const
{
	return GridSlot->GetInventoryItem().IsValid();
}

FIntPoint UInventoryGridWidget::CalculateStartingCoordinate(const FIntPoint& Coordinate, const FIntPoint& Dimensions, const ETileQuadrant TileQuadrant) const
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0; 
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0; 

	FIntPoint StartingCoordinate;
	switch (TileQuadrant)
	{
	case ETileQuadrant::TopLeft:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case ETileQuadrant::TopRight:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case ETileQuadrant::BottomLeft:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	case ETileQuadrant::BottomRight:
		StartingCoordinate.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoordinate.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	default:
		return FIntPoint(-1, -1);
	}
	return StartingCoordinate;
}

void UInventoryGridWidget::PutDownOnIndex(const int32 Index)
{
	AddItemAtIndex(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount()); 
	UpdateGridSlots(HoverItem->GetInventoryItem(), Index, HoverItem->IsStackable(), HoverItem->GetStackCount()); 
	ClearHoverItem();
}

bool UInventoryGridWidget::IsIndexClaimed(const TSet<int32>& CheckedIndices, const int32 Index) const
{
	return CheckedIndices.Contains(Index);
}

/*----------------------------------------------------- Hover ----------------------------------------------------- */
void UInventoryGridWidget::AssignHoverItem(UInventoryItem* Item, const int32 GridIndex, const int32 PreviousGridIndex)
{
	AssignHoverItem(Item); 

	HoverItem->SetPreviousGridIndex(PreviousGridIndex); 
	HoverItem->UpdateStackCount(Item->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
}

FIntPoint UInventoryGridWidget::CalculateHoveredCoordinates(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const
{
	return FIntPoint{
		static_cast<int32>(FMath::FloorToInt((MousePosition.X - CanvasPosition.X) / TileSize)),
		static_cast<int32>(FMath::FloorToInt((MousePosition.Y - CanvasPosition.Y) / TileSize))
	};
}

FSpaceQueryResult UInventoryGridWidget::CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions)
{
	FSpaceQueryResult Result;

	// Check if it's in grid bounds
	if (!IsInGridBounds(UIPGInventoryBPLibrary::GetIndexFromPosition(Position, Columns), Dimensions))
	{
		return Result;
	}

	Result.bHasSpace = true;

	// If more than one of the indices is occupied with the same item, we need to see if they all have the same upper left index
	TSet<int32> OccupiedUpperLeftIndices;
	UIPGInventoryBPLibrary::ForEach2D(
		GridSlots, UIPGInventoryBPLibrary::GetIndexFromPosition(Position, Columns), Dimensions, Columns,
		[&](const UGridSlotWidget* GridSlot)
		{
			if (GridSlot->GetInventoryItem().IsValid())
			{
				OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex()); 
				Result.bHasSpace = false;
			}
		});

	// If so, is there only one item in the way? Can we swap?
	if (OccupiedUpperLeftIndices.Num() == 1)	// single item at position - it's valid for swapping or combining
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator(); 
		Result.ValidItem = GridSlots[Index]->GetInventoryItem(); 
		Result.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	}

	return Result;
}

void UInventoryGridWidget::ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EGridSlotState InGridSlotState)
{
	UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions); 
	UIPGInventoryBPLibrary::ForEach2D(GridSlots, Index, Dimensions, Columns,
		[InGridSlotState](UGridSlotWidget* GridSlot)
		{
			switch (InGridSlotState)
			{
			case EGridSlotState::Unoccupied:
				GridSlot->SetUnoccupiedTexture();
				break;
			case EGridSlotState::Occupied:
				GridSlot->SetOccupiedTexture();
				break;
			case EGridSlotState::Selected:
				GridSlot->SetSelectedTexture(); 
				break;
			case EGridSlotState::GrayedOut:
				GridSlot->SetGrayedOutTexture();
				break;
			default:
				break;
			}
		});
}

void UInventoryGridWidget::SwapWithHoverItem(UInventoryItem* ClickedItem, const int32 GridIndex)
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	UInventoryItem* TempItem = HoverItem->GetInventoryItem(); 
	const int32 TempStackCount = HoverItem->GetStackCount(); 
	const bool bTempStackable = HoverItem->IsStackable(); 

	// Keep same previous grid index
	AssignHoverItem(ClickedItem, GridIndex, HoverItem->GetPreviousGridIndex()); 
	RemoveItemFromGrid(ClickedItem, GridIndex);
	AddItemAtIndex(TempItem, ItemDropIndex, bTempStackable, TempStackCount); 
	UpdateGridSlots(TempItem, ItemDropIndex, bTempStackable, TempStackCount);
}

void UInventoryGridWidget::PutHoverItemBack()
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	FSlotAvailabilityResult SlotAvailabilityResult = HasRoomForItem(HoverItem->GetInventoryItem(), HoverItem->GetStackCount()); 
	SlotAvailabilityResult.Item = HoverItem->GetInventoryItem(); 

	AddStacks(SlotAvailabilityResult); 

	ClearHoverItem(); 
}

void UInventoryGridWidget::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem))
	{
		return;
	}

	UGridSlotWidget* GridSlot = GridSlots[GridIndex]; 
	if (IsValid(GridSlot) && GridSlot->IsAvailable())
	{
		GridSlot->SetOccupiedTexture();
	}
}

void UInventoryGridWidget::OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem))
	{
		return;
	}

	UGridSlotWidget* GridSlot = GridSlots[GridIndex]; 
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetUnoccupiedTexture();
	}
}

/*----------------------------------------------------- Tile ----------------------------------------------------- */
void UInventoryGridWidget::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{
	if (!bMouseWithinCanvas)
	{
		return;
	}

	// Calculate tile quadrant, tile index, and coordinates
	const FIntPoint HoveredTileCoordinates = CalculateHoveredCoordinates(CanvasPosition, MousePosition);

	LastTileParameters = TileParameters;
	TileParameters.TileCoordinates = HoveredTileCoordinates;
	TileParameters.TileIndex = UIPGInventoryBPLibrary::GetIndexFromPosition(HoveredTileCoordinates, Columns); 
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition, MousePosition); 

	OnTileParametersUpdated(TileParameters);
}

ETileQuadrant UInventoryGridWidget::CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const
{
	// Calculate relative position within current tile
	const float TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X, TileSize); 
	const float TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y, TileSize); 

	// Determine which quadrant the mouse is in 
	const bool bTop = TileLocalY < TileSize / 2.f; // Top if Y is in the upper half
	const bool bLeft = TileLocalX < TileSize / 2.f; // Left if X is in the left half

	ETileQuadrant HoveredTileQuadrant = ETileQuadrant::None;
	if (bTop && bLeft)
	{
		HoveredTileQuadrant = ETileQuadrant::TopLeft;
	}
	else if (bTop && !bLeft)
	{
		HoveredTileQuadrant = ETileQuadrant::TopRight;
	}
	else if (!bTop && bLeft)
	{
		HoveredTileQuadrant = ETileQuadrant::BottomLeft;
	}
	else if (!bTop && !bLeft)
	{
		HoveredTileQuadrant = ETileQuadrant::BottomRight;
	}
	return HoveredTileQuadrant;
}

void UInventoryGridWidget::OnTileParametersUpdated(const FTileParameters& InTileParameters)
{
	if (!IsValid(HoverItem))
	{
		return;
	}

	// Get dimensions of hover item
	const FIntPoint Dimensions = HoverItem->GetGridDimensions(); 

	// Calculate starting coordinates for highlighting
	const FIntPoint StartingCoordinates = CalculateStartingCoordinate(InTileParameters.TileCoordinates, Dimensions, InTileParameters.TileQuadrant);
	ItemDropIndex = UIPGInventoryBPLibrary::GetIndexFromPosition(StartingCoordinates, Columns); 

	CurrentQueryResult = CheckHoverPosition(StartingCoordinates, Dimensions);

	if (CurrentQueryResult.bHasSpace)
	{
		HighlightSlots(ItemDropIndex, Dimensions);
		return;
	}
	UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions); 

	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		const FGridFragment* GridFragment = GetFragment<FGridFragment>(CurrentQueryResult.ValidItem.Get(), FragmentTags::GridFragment);
		if (!GridFragment)
		{
			return;
		}
		ChangeHoverType(CurrentQueryResult.UpperLeftIndex, GridFragment->GetGridSize(), EGridSlotState::GrayedOut); 
	}
}

/*----------------------------------------------------- Cursor ----------------------------------------------------- */
bool UInventoryGridWidget::CursorExitedCanvas(const FVector2D& BoundaryPosition, const FVector2D& BoundarySize, const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	bMouseWithinCanvas = UIPGInventoryBPLibrary::IsWithinBounds(BoundaryPosition, BoundarySize, Location);

	if (!bMouseWithinCanvas && bLastMouseWithinCanvas)
	{
		UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
		return true;
	}

	return false;
}


/*----------------------------------------------------- Pop Up ----------------------------------------------------- */
void UInventoryGridWidget::CreateItemPopUp(const int32 GridIndex)
{
	if (!OwningCanvasPanel.IsValid())
	{
		return;
	}

	UInventoryItem* RightClickedItem = GridSlots[GridIndex]->GetInventoryItem().Get(); 
	if (!IsValid(RightClickedItem))
	{
		return;
	}
	if (IsValid(GridSlots[GridIndex]->GetItemPopUp()))
	{
		return;
	}

	PopUpItem = CreateWidget<UPopUpItemWidget>(this, PopUpItemClass); 
	GridSlots[GridIndex]->SetItemPopUp(PopUpItem); 

	OwningCanvasPanel->AddChild(PopUpItem);
	UCanvasPanelSlot* CanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(PopUpItem); 
	if (!IsValid(CanvasPanelSlot))
	{
		return;
	}
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	CanvasPanelSlot->SetPosition(MousePosition - ItemPopUpOffset); 
	CanvasPanelSlot->SetSize(PopUpItem->GetBoxSize()); 

	const int32 SliderMax = GridSlots[GridIndex]->GetStackCount() - 1; 
	if (RightClickedItem->IsStackable() && SliderMax > 0)
	{
		PopUpItem->OnSplit.BindDynamic(this, &UInventoryGridWidget::OnPopUpMenuSplit); 
		PopUpItem->SetSliderParams(SliderMax, FMath::Max(1, GridSlots[GridIndex]->GetStackCount() / 2)); 
	}
	else
	{
		PopUpItem->CollapseSplitButton(); 
	}

	PopUpItem->OnDrop.BindDynamic(this, &UInventoryGridWidget::OnPopUpMenuDrop); 

	if (RightClickedItem->IsConsumable())
	{
		PopUpItem->OnConsume.BindDynamic(this, &UInventoryGridWidget::OnPopUpMenuConsume); 
	}
	else
	{
		PopUpItem->CollapseConsumeButton();
	}
}

void UInventoryGridWidget::OnPopUpMenuSplit(int32 SplitAmount, int32 Index)
{
	if (!IsValid(HoverItem))
	{
		return;
	}
	UInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem().Get(); 
	if (!IsValid(RightClickedItem))
	{
		return;
	}
	if (!RightClickedItem->IsStackable())
	{
		return;
	}

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex(); 
	UGridSlotWidget* UpperLeftGridSlot = GridSlots[UpperLeftIndex]; 
	if (!IsValid(UpperLeftGridSlot))
	{
		return;
	}
	const int32 StackCount = UpperLeftGridSlot->GetStackCount(); 
	const int32 NewStackCount = StackCount - SplitAmount;

	UpperLeftGridSlot->SetStackCount(NewStackCount); 
	SlotItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount); 

	AssignHoverItem(RightClickedItem, UpperLeftIndex, UpperLeftIndex); 
	HoverItem->UpdateStackCount(SplitAmount);
}

void UInventoryGridWidget::OnPopUpMenuDrop(int32 Index)
{
	UInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem().Get(); 
	if (!IsValid(RightClickedItem))
	{
		return;
	}

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex(); 
	UGridSlotWidget* UpperLeftGridSlot = GridSlots[UpperLeftIndex]; 
	if (!IsValid(UpperLeftGridSlot))
	{
		return;
	}
	const int32 NewStackCount = UpperLeftGridSlot->GetStackCount() - 1; 
	SlotItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount); 

	if (InventoryComponent.IsValid())
	{
		InventoryComponent->Server_ConsumeItem(RightClickedItem);
	}

	if (NewStackCount <= 0)
	{
		RemoveItemFromGrid(RightClickedItem, Index);
	}
}

void UInventoryGridWidget::OnPopUpMenuConsume(int32 Index)
{
	UInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem().Get(); 
	if (!IsValid(RightClickedItem))
	{
		return;
	}

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex(); 
	UGridSlotWidget* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	if (!IsValid(UpperLeftGridSlot))
	{
		return;
	}
	const int32 NewStackCount = UpperLeftGridSlot->GetStackCount() - 1; 

	UpperLeftGridSlot->SetStackCount(NewStackCount); 
	SlotItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount); 

	InventoryComponent->Server_ConsumeItem(RightClickedItem); 
	if (NewStackCount <= 0)
	{
		RemoveItemFromGrid(RightClickedItem, Index);
	}
}

/*----------------------------------------------------- Toggle ----------------------------------------------------- */
void UInventoryGridWidget::OnInventoryMenuToggled(bool bOpen)
{
	if (!bOpen)
	{
		PutHoverItemBack();
	}
}