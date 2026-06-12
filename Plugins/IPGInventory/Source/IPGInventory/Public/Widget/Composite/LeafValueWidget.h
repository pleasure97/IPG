// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Composite/LeafWidget.h"
#include "LeafValueWidget.generated.h"

class UCommonTextBlock;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API ULeafValueWidget : public ULeafWidget
{
	GENERATED_BODY()
	
public:
	void SetLabelText(const FText& Text, bool bCollapse) const;
	void SetValueText(const FText& Text, bool bCollapse) const; 

protected:
	virtual void NativePreConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock_Label;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock_Value;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 FontSize_Label = 12;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 FontSize_Value = 12;
};
