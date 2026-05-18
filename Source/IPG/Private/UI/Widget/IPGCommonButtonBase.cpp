// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/IPGCommonButtonBase.h"
#include "CommonTextBlock.h"
#include "CommonLazyImage.h"
#include "UI/IPGUISubsystem.h"

void UIPGCommonButtonBase::SetButtonDisplayText(FText InText)
{
	if (CommonTextBlock_ButtonText && !InText.IsEmpty())
	{
		CommonTextBlock_ButtonText->SetText(bUseUpperCaseForButtonText ? InText.ToUpper() : InText);
	}
}

FText UIPGCommonButtonBase::GetButtonDisplayText() const
{
	if (CommonTextBlock_ButtonText)
	{
		return CommonTextBlock_ButtonText->GetText();
	}
	return FText();
}

void UIPGCommonButtonBase::SetButtonDescriptionText(FText InText)
{
	if (CommonTextBlock_ButtonDescriptionText && !InText.IsEmpty())
	{
		CommonTextBlock_ButtonDescriptionText->SetText(bUseUpperCaseForButtonText ? InText.ToUpper() : InText);
	}
}

void UIPGCommonButtonBase::SetButtonDisplayImage(const FSlateBrush& InBrush)
{
	if (CommonLazyImage_ButtonImage)
	{
		CommonLazyImage_ButtonImage->SetBrush(InBrush);
	}
}

void UIPGCommonButtonBase::SetButtonDisplayMode(bool bShowImage)
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

void UIPGCommonButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetButtonDisplayText(ButtonDisplayText);

	SetButtonDescriptionText(ButtonDescriptionText);
}

void UIPGCommonButtonBase::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	if (CommonTextBlock_ButtonText && GetCurrentTextStyleClass())
	{
		CommonTextBlock_ButtonText->SetStyle(GetCurrentTextStyleClass());
	}
}

void UIPGCommonButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();

	if (!ButtonDescriptionTextWhenHovered.IsEmpty())
	{
		UIPGUISubsystem::Get(this)->OnButtonDescriptionTextUpdated.Broadcast(this, ButtonDescriptionTextWhenHovered);
	}
}

void UIPGCommonButtonBase::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();

	if (!ButtonDescriptionTextWhenHovered.IsEmpty())
	{
		UIPGUISubsystem::Get(this)->OnButtonDescriptionTextUpdated.Broadcast(this, FText::GetEmpty());
	}
}
