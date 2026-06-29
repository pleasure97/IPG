// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "Iris/IPGNetPrioritizerDataAsset.h"
#include "IPGNetPrioritizerVolume.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API AIPGNetPrioritizerVolume : public AVolume
{
	GENERATED_BODY()
	
public:
	AIPGNetPrioritizerVolume(); 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iris")
	EIPGMapType ZoneType = EIPGMapType::Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Iris")
	float BlendTime = 3.0f;
	
protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

private:
	bool TryGetConnectId(AActor* OtherActor, OUT uint32& ConnectionId) const;
};
