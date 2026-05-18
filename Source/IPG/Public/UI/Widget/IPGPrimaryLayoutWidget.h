// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "IPGPrimaryLayoutWidget.generated.h"

class UCommonActivatableWidgetContainerBase;
/**
 * 
 */
UCLASS()
class IPG_API UIPGPrimaryLayoutWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UCommonActivatableWidgetContainerBase* FindWidgetStackByTag(const FGameplayTag& InTag) const;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable)
	void RegisterWidgetStack(
		UPARAM(meta = (Categories = "UI.WidgetStack")) FGameplayTag InStackTag,
		UCommonActivatableWidgetContainerBase* InStack);

	UPROPERTY(meta = (Bindwidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> WidgetStack_Frontend;

	UPROPERTY(meta = (Bindwidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> WidgetStack_GameHUD;

	UPROPERTY(meta = (Bindwidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> WidgetStack_GameMenu;

	UPROPERTY(meta = (Bindwidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> WidgetStack_Modal;

private:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*> RegisterWidgetStackMap;
};