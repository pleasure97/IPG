// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Composite/LeafWidget.h"
#include "LeafTextWidget.generated.h"

class UCommonTextBlock;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API ULeafTextWidget : public ULeafWidget
{
	GENERATED_BODY()
	
public:
	void SetText(const FText& Text) const;
	
protected:
	virtual void NativePreConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock_LeafText;

	UPROPERTY(meta = (BindWidget))
	int32 FontSize = 12;
};
