// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Composite/CompositeBaseWidget.h"
#include "LeafWidget.generated.h"

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API ULeafWidget : public UCompositeBaseWidget
{
	GENERATED_BODY()
	
public:
	virtual void ApplyFunction(FuncType Function) override;
};
