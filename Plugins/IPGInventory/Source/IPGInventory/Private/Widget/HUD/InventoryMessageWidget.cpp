// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/InventoryMessageWidget.h"
#include "CommonTextBlock.h"

void UInventoryMessageWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TextBlock_Message->SetText(FText::GetEmpty());
	HideMessage();
}

void UInventoryMessageWidget::SetMessage(const FText& Message)
{
	TextBlock_Message->SetText(Message); 

	if (!bIsMessageActive)
	{
		ShowMessage(); 
	}
	bIsMessageActive = true;
	GetWorld()->GetTimerManager().SetTimer(MessageTimer, [this]()
		{
			HideMessage(); 
			bIsMessageActive = false;

		}, MessageLifetime, false);
}
