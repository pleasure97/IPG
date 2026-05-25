// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Composite/CompositeBaseWidget.h"
#include "CompositeWidget.generated.h"

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UCompositeWidget : public UCompositeBaseWidget
{
	GENERATED_BODY()
	
public:
	virtual void Collapse() override;
	virtual void ApplyFunction(FuncType Function) override;

	TArray<UCompositeBaseWidget*> GetChildren(); 

protected:
	virtual void NativeOnInitialized() override;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UCompositeBaseWidget>> Children;
};
