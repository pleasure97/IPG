// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Composite/CompositeWidget.h"
#include "Blueprint/WidgetTree.h"

TArray<UCompositeBaseWidget*> UCompositeWidget::GetChildren()
{
	return Children;
}

void UCompositeWidget::Collapse()
{
	for (UCompositeBaseWidget* Child : Children)
	{
		Child->Collapse();
	}
}

void UCompositeWidget::ApplyFunction(FuncType Function)
{
	for (UCompositeBaseWidget* Child : Children)
	{
		Child->ApplyFunction(Function);
	}
}

void UCompositeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized(); 

	WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (UCompositeBaseWidget* Composite = Cast<UCompositeBaseWidget>(Widget))
			{
				Children.Add(Composite);
				Composite->Collapse();
			}
		});
}
