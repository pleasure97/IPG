// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Composite/LeafImageWidget.h"
#include "CommonLazyImage.h"
#include "Components/SizeBox.h"

void ULeafImageWidget::SetImage(UTexture2D* Texture) const
{
	Image_Icon->SetBrushFromTexture(Texture);
}

void ULeafImageWidget::SetBoxSize(const FVector2D& Size) const
{
	SizeBox_Icon->SetWidthOverride(Size.X);
	SizeBox_Icon->SetHeightOverride(Size.Y);
}

void ULeafImageWidget::SetImageSize(const FVector2D& Size) const
{
	Image_Icon->SetDesiredSizeOverride(Size);
}

FVector2D ULeafImageWidget::GetImageSize() const
{
	return Image_Icon->GetDesiredSize(); 
}

