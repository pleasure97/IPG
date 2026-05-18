// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/IPGUISubsystem.h"
#include "Engine/AssetManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "UI/Widget/IPGPrimaryLayoutWidget.h"
#include "UI/Widget/IPGActivatableWidget.h"
#include "IPGGameplayTags.h"

UIPGUISubsystem* UIPGUISubsystem::Get(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);

		return UGameInstance::GetSubsystem<UIPGUISubsystem>(World->GetGameInstance());
	}

	return nullptr;
}

bool UIPGUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> FoundClasses;
		GetDerivedClasses(GetClass(), FoundClasses);

		return FoundClasses.IsEmpty();
	}

	return false;
}

void UIPGUISubsystem::RegisterCreatedPrimaryLayoutWidget(UIPGPrimaryLayoutWidget* InCreatedWidget)
{
	check(InCreatedWidget);

	CreatedPrimaryLayoutWidget = InCreatedWidget;
}

void UIPGUISubsystem::PushSoftWidgetToStackAsync(const FGameplayTag& InWidgetStackTag, TSoftClassPtr<UIPGActivatableWidget> InSoftWidgetClass, TFunction<void(EAsyncPushWidgetState, UIPGActivatableWidget*)> AsyncPushStateCallback)
{
	check(!InSoftWidgetClass.IsNull());

	// Request Async Load 
	// Command to load widget assets from the hard disk into memory via "AssetManager"
	// Since this task runs on a separate thread or in the background, load on GameThread is reduced.
	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		InSoftWidgetClass.ToSoftObjectPath(),
		// After Asset Loading is Complete, Call Lambda Function 
		FStreamableDelegate::CreateLambda(
			[this, InSoftWidgetClass, InWidgetStackTag, AsyncPushStateCallback]()
			{
				UClass* LoadedWidgetClass = InSoftWidgetClass.Get();
				check(LoadedWidgetClass && CreatedPrimaryLayoutWidget);
				// Found Widget Stack from Primary Layout Widget 
				UCommonActivatableWidgetContainerBase* FoundWidgetStack = CreatedPrimaryLayoutWidget->FindWidgetStackByTag(InWidgetStackTag);
				UIPGActivatableWidget* CreatedWidget = FoundWidgetStack->AddWidget<UIPGActivatableWidget>(
					LoadedWidgetClass,
					// State just before being fully 'pushed' onto the widget stack
					// At this point, the initial data required for the widget can be pre-set
					[AsyncPushStateCallback](UIPGActivatableWidget& CreatedWidgetInstance)
					{
						AsyncPushStateCallback(EAsyncPushWidgetState::OnCreatedBeforePush, &CreatedWidgetInstance);
					}
				);
				// After the widget is successfully added to the stack
				AsyncPushStateCallback(EAsyncPushWidgetState::AfterPush, CreatedWidget);
			}
		)
	);
}