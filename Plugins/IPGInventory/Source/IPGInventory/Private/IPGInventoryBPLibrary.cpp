// Copyright Epic Games, Inc. All Rights Reserved.

#include "IPGInventoryBPLibrary.h"
#include "Component/InventoryComponent.h"

UInventoryComponent* UIPGInventoryBPLibrary::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	UInventoryComponent* InventoryComponent = PlayerController->FindComponentByClass<UInventoryComponent>();
	return InventoryComponent;
}