// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Composite/LeafTextWidget.h"
#include "CommonTextBlock.h"

void ULeafTextWidget::NativePreConstruct()
{
	Super::NativePreConstruct(); 

	FSlateFontInfo FontInfo = TextBlock_LeafText->GetFont(); 
	FontInfo.Size = FontSize;

	TextBlock_LeafText->SetFont(FontInfo);
}

void ULeafTextWidget::SetText(const FText& Text) const
{
	TextBlock_LeafText->SetText(Text);
}