// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "HoverItemWidget.generated.h"

class UCommonLazyImage;
class UCommonTextBlock;
class UInventoryItem;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UHoverItemWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	void SetImageBrush(const FSlateBrush& Brush) const; 
	FGameplayTag GetItemType() const; 

	/* Grid */
	int32 GetPreviousGridIndex() const; 
	void SetPreviousGridIndex(int32 Index);
	FIntPoint GetGridDimensions() const; 
	void SetGridDimensions(const FIntPoint& Dimensions); 

	/* Stack */
	void UpdateStackCount(const int32 Count); 
	int32 GetStackCount() const; 
	bool IsStackable() const; 
	void SetStackable(bool bIsStackable); 

	/* Inventory Item */
	UInventoryItem* GetInventoryItem() const; 
	void SetInventoryItem(UInventoryItem* Item);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonLazyImage> Image_Icon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock_StackCount;

	/* Grid */
	int32 PreviousGridIndex;
	FIntPoint GridDimensions;

	/* Stack */
	bool bStackable = false;
	int32 StackCount = 0; 

	/* Inventory Item */
	TWeakObjectPtr<UInventoryItem> InventoryItem;
};
