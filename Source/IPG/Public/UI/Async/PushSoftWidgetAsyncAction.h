// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayTagContainer.h"
#include "PushSoftWidgetAsyncAction.generated.h"

class UIPGActivatbleWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPushSoftWidgetDelegate, UIPGActivatableWidget*, PushedWidget);

/**
 * 
 */
UCLASS()
class IPG_API UPushSoftWidgetAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", HidePin = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Push Soft Widget To Widget Stack"))
	static UPushSoftWidgetAsyncAction* PushSoftWidget(
		const UObject* WorldContextObject,
		APlayerController* OwningPlayerController,
		TSoftClassPtr<UIPGActivatableWidget> InSoftWidgetClass,
		UPARAM(meta = (Categories = "UI.WidgetStack")) FGameplayTag InWidgetStackTag,
		bool bFocusOnNewlyPushedWidget = true);

	/* UBlueprintAsyncActionBase Begins */
	virtual void Activate() override;
	/* UBlueprintAsyncActionBase Ends */

	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate OnWidgetCreatedBeforePush;

	UPROPERTY(BlueprintAssignable)
	FOnPushSoftWidgetDelegate AfterPush;

private:
	TWeakObjectPtr<UWorld> CachedOwningWorld;

	TWeakObjectPtr<APlayerController> CachedOwningPlayerController;

	TSoftClassPtr<UIPGActivatableWidget> CachedSoftWidgetClass;

	FGameplayTag CachedWidgetStackTag;

	bool bCachedFocusOnNewlyPushedWidget = false;
};

