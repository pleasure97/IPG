// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "InventoryHUDWidget.generated.h"

class UInventoryMessageWidget;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UInventoryHUDWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	void ShowPickupMessage(const FString& Message);

	void HidePickupMessage();

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryMessageWidget> InfoMessage;
	
	UFUNCTION()
	void OnNoRoom(); 
};
