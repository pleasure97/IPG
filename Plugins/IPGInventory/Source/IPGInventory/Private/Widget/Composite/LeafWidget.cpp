// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Composite/LeafWidget.h"

void ULeafWidget::ApplyFunction(FuncType Function)
{
	Function(this);
}
