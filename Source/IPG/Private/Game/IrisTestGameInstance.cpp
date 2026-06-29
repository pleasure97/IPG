// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/IrisTestGameInstance.h"
#include "Engine/Engine.h"

void UIrisTestGameInstance::Init()
{
	Super::Init(); 

    FString ServerIP;
    // Early return if the game instance is in server
    if (!FParse::Value(FCommandLine::Get(), TEXT("ServerIP="), ServerIP))
    {
        return; 
    }

    FTimerHandle Handle;
    GetTimerManager().SetTimer(Handle, [this, ServerIP]()
        {
            if (!GEngine) return;

            FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
            if (!WorldContext) return;

            FURL URL;
            URL.Host = ServerIP;
            URL.Port = 7777;

            FString BrowseError;
            GEngine->Browse(*WorldContext, URL, BrowseError);

            if (!BrowseError.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("Browse error: %s"), *BrowseError);
            }
        }, 2.0f, false);
}
