// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Composite/CompositeBaseWidget.h"

FGameplayTag UCompositeBaseWidget::GetFragmentTag() const
{
	return FragmentTag;
}

void UCompositeBaseWidget::SetFragmentTag(const FGameplayTag& Tag)
{
	FragmentTag = Tag;
}

void UCompositeBaseWidget::Collapse()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCompositeBaseWidget::Expand()
{
	SetVisibility(ESlateVisibility::Visible);
}