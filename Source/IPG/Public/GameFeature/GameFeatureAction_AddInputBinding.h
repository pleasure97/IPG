// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeature/GameFeatureAction_WorldActionBase.h"
#include "GameFeatureAction_AddInputBinding.generated.h"

class UIPGInputConfig;
struct FComponentRequestHandle;

/**
 * 
 */
UCLASS( meta = (DisplayName = "Add Input Binding"))
class IPG_API UGameFeatureAction_AddInputBinding final: public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()
		
public:
	/* UGameFeatureAction interface */
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	/* UGameFeatureAction interface End */

	/* UObject Interface */
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	/* UObject Interface End */

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AssetBundles = "Client,Server"))
	TArray<TSoftObjectPtr<const UIPGInputConfig>> InputConfigs;

private:
	struct FInputBindingContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		TArray<TWeakObjectPtr<APawn>> PawnsAddedTo;
	};

	TMap<FGameFeatureStateChangeContext, FInputBindingContextData> InputBindingContextDataMap;

	/* UGameFeatureAction_WorldActionBase interface */
	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	/* UGameFeatureAction_WorldActionBase interface End */

	void Reset(FInputBindingContextData& ActiveData);
	void HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);

	/* Input Binding Util */
	void AddInputMappingForPlayer(APawn* Pawn, FInputBindingContextData& ActiveData);
	void RemoveInputMappingForPlayer(APawn* Pawn, FInputBindingContextData& ActiveData);
};
