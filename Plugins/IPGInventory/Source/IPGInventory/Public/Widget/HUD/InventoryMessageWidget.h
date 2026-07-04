// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "InventoryMessageWidget.generated.h"

class UCommonTextBlock;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UInventoryMessageWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	void ShowMessage();

	void HideMessage();

	void SetMessage(const FText& Message);

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock_Message;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float MessageLifetime = 3.f;

	FTimerHandle MessageTimer;

	bool bIsMessageActive = false;
};
