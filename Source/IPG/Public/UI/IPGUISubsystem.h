// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/IPGUIEnumTypes.h"
#include "IPGUISubsystem.generated.h"

class UIPGActivatableWidget;
class UIPGPrimaryLayoutWidget;
class UIPGCommonButtonBase;
struct FGameplayTag;

enum class EAsyncPushWidgetState : uint8
{
	OnCreatedBeforePush,
	AfterPush
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnButtonDescriptionTextUpdatedDelegate, UIPGCommonButtonBase*, BroadcastingButton, FText, DescriptionText);

/**
 * 
 */
UCLASS()
class IPG_API UIPGUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	static UIPGUISubsystem* Get(const UObject* WorldContextObject);

	/* USubsystem Interface Begins */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	/* USubsystem Interface Ends */

	UFUNCTION(BlueprintCallable)
	void RegisterCreatedPrimaryLayoutWidget(UIPGPrimaryLayoutWidget* InCreatedWidget);

	void PushSoftWidgetToStackAsync(
		const FGameplayTag& InWidgetStackTag,
		TSoftClassPtr<UIPGActivatableWidget> InSoftWidgetClass,
		TFunction<void(EAsyncPushWidgetState, UIPGActivatableWidget*)> AsyncPushStateCallback);

	UPROPERTY(BlueprintAssignable)
	FOnButtonDescriptionTextUpdatedDelegate OnButtonDescriptionTextUpdated;

private:
	UPROPERTY(Transient)
	UIPGPrimaryLayoutWidget* CreatedPrimaryLayoutWidget;
};
