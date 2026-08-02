// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPGGameMode.h"
#include "IrisTestGameMode.generated.h"

class UIPGFoVNetPrioritizerDataAsset;

/**
 * 
 */
UCLASS()
class IPG_API AIrisTestGameMode : public AIPGGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void BeginPlay() override;

	/* Network Prioritizer Data Asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iris|Presets")
	TArray<TObjectPtr<UIPGFoVNetPrioritizerDataAsset>> FoVPrioritizerPresets;

private:
	/* Number of Players */
	UPROPERTY(EditAnywhere)
	int32 MaxPlayers = 100; 

	UPROPERTY(EditAnywhere)
	int32 RequiredPlayersToStart = 80;

	FTimerHandle InsightsTimerHandle;
	FTimerHandle StatusLogTimerHandle;
	
	bool bGameStarted = false;

	void TriggerGameStart(); 

	void StopProfilingAndShutdown();

	void LogServerStatus();
};
