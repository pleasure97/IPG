// Copyright Epic Games, Inc. All Rights Reserved.

#include "IPGInventoryBPLibrary.h"
#include "Component/InventoryComponent.h"
#include "Component/ItemComponent.h"
#include "Widget/InventoryBaseWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"

/*----------------------------------------------------- Inventory ----------------------------------------------------- */
UInventoryComponent* UIPGInventoryBPLibrary::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	UInventoryComponent* InventoryComponent = PlayerController->FindComponentByClass<UInventoryComponent>();
	return InventoryComponent;
}

EItemCategory UIPGInventoryBPLibrary::GetItemContegoryFromItemComponent(UItemComponent* ItemComponent)
{
	if (!IsValid(ItemComponent))
	{
		return EItemCategory::None;
	}
	return ItemComponent->GetItemManifest().GetItemCategory();
}

UHoverItemWidget* UIPGInventoryBPLibrary::GetHoverItem(APlayerController* PC)
{
	UInventoryComponent* InventoryComponent = GetInventoryComponent(PC); 
	if (!IsValid(InventoryComponent))
	{
		return nullptr;
	}

	UInventoryBaseWidget* InventoryBase = InventoryComponent->GetInventoryMenu(); 
	if (!IsValid(InventoryBase))
	{
		return nullptr;
	}

	return InventoryBase->GetHoverItem();
}

void UIPGInventoryBPLibrary::ItemHovered(APlayerController* PC, UInventoryItem* Item)
{
	// Get inventory component from player controller
	UInventoryComponent* InventoryComponent = GetInventoryComponent(PC); 
	if (!IsValid(InventoryComponent))
	{
		return;
	}

	// Get inventory menu from inventory component 
	UInventoryBaseWidget* InventoryMenu = InventoryComponent->GetInventoryMenu();
	if (!IsValid(InventoryMenu))
	{
		return;
	}

	InventoryMenu->OnItemHovered(Item);
}

void UIPGInventoryBPLibrary::ItemUnhovered(APlayerController* PC)
{
	// Get inventory component from player controller
	UInventoryComponent* InventoryComponent = GetInventoryComponent(PC);
	if (!IsValid(InventoryComponent))
	{
		return;
	}

	// Get inventory menu from inventory component 
	UInventoryBaseWidget* InventoryMenu = InventoryComponent->GetInventoryMenu();
	if (!IsValid(InventoryMenu))
	{
		return;
	}
	InventoryMenu->OnItemUnhovered();
}

UInventoryBaseWidget* UIPGInventoryBPLibrary::GetInventoryWidget(APlayerController* PC)
{
	UInventoryComponent* InventoryComponent = GetInventoryComponent(PC); 
	if (!IsValid(InventoryComponent))
	{
		return nullptr;
	}

	return InventoryComponent->GetInventoryMenu();
}

/*----------------------------------------------------- Widget Util ----------------------------------------------------- */
FVector2D UIPGInventoryBPLibrary::GetWidgetPosition(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry(); 
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(Widget, Geometry, USlateBlueprintLibrary::GetLocalTopLeft(Geometry), PixelPosition, ViewportPosition);
	return ViewportPosition;
}

FVector2D UIPGInventoryBPLibrary::GetWidgetSize(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	return Geometry.GetLocalSize();
}

bool UIPGInventoryBPLibrary::IsWithinBounds(const FVector2D& BoundaryPosition, const FVector2D& WidgetSize, const FVector2D& MousePosition)
{
	// BoundaryPosition <= MousePosition <= BoundaryPosition + WidgetSize
	return (MousePosition.X >= BoundaryPosition.X) && (MousePosition.X <= (BoundaryPosition.X + WidgetSize.X)) &&
		(MousePosition.Y >= BoundaryPosition.Y) && (MousePosition.Y <= (BoundaryPosition.Y + WidgetSize.Y));
}

int32 UIPGInventoryBPLibrary::GetIndexFromPosition(const FIntPoint& Position, const int32 Columns)
{
	return Position.X + Position.Y * Columns;
}

FIntPoint UIPGInventoryBPLibrary::GetPositionFromIndex(const int32 Index, const int32 Columns)
{
	return FIntPoint(Index % Columns, Index / Columns);
}

FVector2D UIPGInventoryBPLibrary::GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize, const FVector2D& MousePosition)
{
	FVector2D ClampedPosition = MousePosition;

	// Adjust horizontal position to ensure that the widget stays within the boundary
	if (MousePosition.X + WidgetSize.X > Boundary.X) // Widget exceeds the right edge
	{
		ClampedPosition.X = Boundary.X - WidgetSize.X;
	}
	if (MousePosition.X < 0.f) // Widget exceeds the left edge
	{
		ClampedPosition.X = 0.f;
	}

	// Adjust vertical position to ensure that the widget stays within the boundary
	if (MousePosition.Y + WidgetSize.Y > Boundary.Y) // Widget exceeds the bottom edge
	{
		ClampedPosition.Y = Boundary.Y - WidgetSize.Y;
	}
	if (MousePosition.Y < 0.f) // Widget exceeds the top edge
	{
		ClampedPosition.Y = 0.f;
	}

	return ClampedPosition;
}
