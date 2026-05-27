// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/InventoryCommonButton.h"
#include "CommonTextBlock.h"
#include "CommonLazyImage.h"
#include "InventorySubsystem.h"

void UInventoryCommonButton::SetButtonDisplayText(FText InText)
{
	if (CommonTextBlock_ButtonText && !InText.IsEmpty())
	{
		CommonTextBlock_ButtonText->SetText(bUseUpperCaseForButtonText ? InText.ToUpper() : InText);
	}
}

FText UInventoryCommonButton::GetButtonDisplayText() const
{
	if (CommonTextBlock_ButtonText)
	{
		return CommonTextBlock_ButtonText->GetText();
	}
	return FText();
}

void UInventoryCommonButton::SetButtonDescriptionText(FText InText)
{
	if (CommonTextBlock_ButtonDescriptionText && !InText.IsEmpty())
	{
		CommonTextBlock_ButtonDescriptionText->SetText(bUseUpperCaseForButtonText ? InText.ToUpper() : InText);
	}
}

void UInventoryCommonButton::SetButtonDisplayImage(const FSlateBrush& InBrush)
{
	if (CommonLazyImage_ButtonImage)
	{
		CommonLazyImage_ButtonImage->SetBrush(InBrush);
	}
}

void UInventoryCommonButton::SetButtonDisplayMode(bool bShowImage)
{
	if (!CommonLazyImage_ButtonImage || !CommonTextBlock_ButtonText)
	{
		return;
	}

	if (bShowImage)
	{
		CommonLazyImage_ButtonImage->SetVisibility(ESlateVisibility::Visible);
		CommonTextBlock_ButtonText->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		CommonLazyImage_ButtonImage->SetVisibility(ESlateVisibility::Collapsed);
		CommonTextBlock_ButtonText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInventoryCommonButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetButtonDisplayText(ButtonDisplayText);

	SetButtonDescriptionText(ButtonDescriptionText);
}

void UInventoryCommonButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	if (CommonTextBlock_ButtonText && GetCurrentTextStyleClass())
	{
		CommonTextBlock_ButtonText->SetStyle(GetCurrentTextStyleClass());
	}
}

void UInventoryCommonButton::NativeOnHovered()
{
	Super::NativeOnHovered();

	if (!ButtonDescriptionTextWhenHovered.IsEmpty())
	{
		UInventorySubsystem::Get(this)->OnButtonDescriptionTextUpdated.Broadcast(this, ButtonDescriptionTextWhenHovered);
	}
}

void UInventoryCommonButton::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();

	if (!ButtonDescriptionTextWhenHovered.IsEmpty())
	{
		UInventorySubsystem::Get(this)->OnButtonDescriptionTextUpdated.Broadcast(this, FText::GetEmpty());
	}
}




