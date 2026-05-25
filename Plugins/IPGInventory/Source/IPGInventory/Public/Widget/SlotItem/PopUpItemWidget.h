// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PopUpItemWidget.generated.h"

class UCommonButtonBase;
class UCommonTextBlock;
class USlider;
class USizeBox;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPopUpMenuSplit, int32, SplitAmount, int32, Index); 
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuDrop, int32, Index); 
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopUpMenuConsume, int32, Index);

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UPopUpItemWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	FPopUpMenuSplit OnSplit;
	FPopUpMenuDrop OnDrop;
	FPopUpMenuConsume OnConsume;

	/* "SPLIT AMOUNT" Getter */
	int32 GetSplitAmount() const;

	/* "SLIDER PARAMS" Setter */
	void SetSliderParams(const float Max, const float Value) const; 

	/* "BOX SIZE" Getter */
	FVector2D GetBoxSize() const; 

	/* "GRID INDEX" Getter & Setter */
	void SetGridIndex(int32 Index); 
	int32 GetGridIndex() const; 

	/* Collapse Button */
	void CollapseSplitButton() const;
	void CollapseConsumeButton() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	
private:
	UFUNCTION()
	void SplitButtonClicked(); 

	UFUNCTION()
	void DropButtonClicked(); 

	UFUNCTION()
	void ConsumeButtonClicked(); 

	UFUNCTION()
	void SliderValueChanged(float Value);

	int32 GridIndex = INDEX_NONE;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Split;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Drop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Consume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock_SplitAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
};
