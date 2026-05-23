// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/InventoryHUDWidget.h"
#include "Component/InventoryComponent.h"
#include "IPGInventoryBPLibrary.h"
#include "Widget/HUD/InventoryMessageWidget.h"

#define LOCTEXT_NAMESPACE "Inventory"
void UInventoryHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized(); 

	UInventoryComponent* InventoryComponent = UIPGInventoryBPLibrary::GetInventoryComponent(GetOwningPlayer()); 
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->NoRoomInInventory.AddDynamic(this, &UInventoryHUDWidget::OnNoRoom);
	}
}

void UInventoryHUDWidget::OnNoRoom()
{
	if (!IsValid(InfoMessage))
	{
		return;
	}
	InfoMessage->SetMessage(LOCTEXT("No Room in Inventory", "No Room in Inventory"));
}
#undef LOCTEXT_NAMESPACE // Inventory