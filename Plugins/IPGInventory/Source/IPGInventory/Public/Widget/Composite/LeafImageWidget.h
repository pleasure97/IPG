// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Composite/LeafWidget.h"
#include "LeafImageWidget.generated.h"

class UCommonLazyImage;
class USizeBox;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API ULeafImageWidget : public ULeafWidget
{
	GENERATED_BODY()

public:
	void SetImage(UTexture2D* Texture) const;
	void SetBoxSize(const FVector2D& Size) const;
	void SetImageSize(const FVector2D& Size) const;
	FVector2D GetImageSize() const;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonLazyImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Icon;
};
