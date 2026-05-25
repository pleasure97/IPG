// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotItem/PopUpItemWidget.h"
#include "Components/Slider.h"
#include "Components/SizeBox.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"

/* "SPLIT AMOUNT" Getter */
int32 UPopUpItemWidget::GetSplitAmount() const
{
	return FMath::Floor(Slider_Split->GetValue()); 
}

/* "SLIDER PARAMS" Setter */
void UPopUpItemWidget::SetSliderParams(const float Max, const float Value) const
{
	Slider_Split->SetMaxValue(Max);
	Slider_Split->SetMinValue(1);
	Slider_Split->SetValue(Value);

	TextBlock_SplitAmount->SetText(FText::AsNumber(FMath::Floor(Value)));
}

/* "BOX SIZE" Getter */
FVector2D UPopUpItemWidget::GetBoxSize() const
{
	return FVector2D(SizeBox_Root->GetWidthOverride(), SizeBox_Root->GetHeightOverride()); 
}

/* "GRID INDEX" Getter & Setter */
void UPopUpItemWidget::SetGridIndex(int32 Index)
{
	GridIndex = Index;
}

int32 UPopUpItemWidget::GetGridIndex() const
{
	return GridIndex;
}

/* Collapse Button */
void UPopUpItemWidget::CollapseSplitButton() const
{
	Button_Split->SetVisibility(ESlateVisibility::Collapsed);
	Slider_Split->SetVisibility(ESlateVisibility::Collapsed);
	TextBlock_SplitAmount->SetVisibility(ESlateVisibility::Collapsed);
}

void UPopUpItemWidget::CollapseConsumeButton() const
{
	Button_Consume->SetVisibility(ESlateVisibility::Collapsed);
}

void UPopUpItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized(); 

	Button_Split->OnClicked().AddUObject(this, &UPopUpItemWidget::SplitButtonClicked);
	Button_Drop->OnClicked().AddUObject(this, &UPopUpItemWidget::DropButtonClicked);
	Button_Consume->OnClicked().AddUObject(this, &UPopUpItemWidget::ConsumeButtonClicked);
	Slider_Split->OnValueChanged.AddDynamic(this, &UPopUpItemWidget::SliderValueChanged);
}

void UPopUpItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	RemoveFromParent();
}

void UPopUpItemWidget::SplitButtonClicked()
{
	if (OnSplit.ExecuteIfBound(GetSplitAmount(), GridIndex))
	{
		RemoveFromParent();
	}
}

void UPopUpItemWidget::DropButtonClicked()
{
	if (OnDrop.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UPopUpItemWidget::ConsumeButtonClicked()
{
	if (OnConsume.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UPopUpItemWidget::SliderValueChanged(float Value)
{
	TextBlock_SplitAmount->SetText(FText::AsNumber(FMath::Floor(Value)));
}
