// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Composite/CompositeWidget.h"
#include "ItemDescriptionWidget.generated.h"

class USizeBox;
/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UItemDescriptionWidget : public UCompositeWidget
{
	GENERATED_BODY()
	
public:
	FVector2D GetBoxSize() const;

	virtual void SetVisibility(ESlateVisibility InVisibility) override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox;
};
