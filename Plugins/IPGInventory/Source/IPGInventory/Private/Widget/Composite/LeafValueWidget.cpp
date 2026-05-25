// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Composite/LeafValueWidget.h"
#include "CommonTextBlock.h"

void ULeafValueWidget::SetLabelText(const FText& Text, bool bCollapse) const
{
	if (bCollapse)
	{
		TextBlock_Label->SetVisibility(ESlateVisibility::Collapsed); 
		return;
	}
	TextBlock_Label->SetText(Text); 
}

void ULeafValueWidget::SetValueText(const FText& Text, bool bCollapse) const
{
	if (bCollapse)
	{
		TextBlock_Value->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	TextBlock_Value->SetText(Text);
}

void ULeafValueWidget::NativePreConstruct()
{
	Super::NativePreConstruct(); 

	FSlateFontInfo LabelFontInfo = TextBlock_Label->GetFont(); 
	LabelFontInfo.Size = FontSize_Label;
	TextBlock_Label->SetFont(LabelFontInfo); 

	FSlateFontInfo ValueFontInfo = TextBlock_Value->GetFont(); 
	ValueFontInfo.Size = FontSize_Value;
	TextBlock_Value->SetFont(ValueFontInfo);
}
