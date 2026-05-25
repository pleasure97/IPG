// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "IPGInventoryBPLibrary.generated.h"

class APlayerController;
class UInventoryComponent; 
class UInventoryItem;

UCLASS()
class UIPGInventoryBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryComponent* GetInventoryComponent(const APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void ItemHovered(APlayerController* PC, UInventoryItem* Item); 

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void ItemUnhovered(APlayerController* PC);
};
