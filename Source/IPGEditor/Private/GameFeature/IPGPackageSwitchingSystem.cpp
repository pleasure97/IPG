// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGPackageSwitchingSystem.h"
#include "GameFeature/IPGPackageData.h"
#include "GameFeature/IPGGameFeaturePolicy.h"
#include "System/IPGPackageSettings.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameFeaturePluginOperationResult.h"
#include "Interfaces/IPluginManager.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Misc/MessageDialog.h"
#include "Misc/ScopedSlowTask.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "IPGPackageSwitcher"

DEFINE_LOG_CATEGORY_STATIC(LogIPGPackageSwitcher, Log, All);

namespace
{
	TArray<TSharedRef<IPlugin>> GetAllInstalledGameFeaturePlugins()
	{
		TArray<TSharedRef<IPlugin>> Result;

		const UGameFeaturesSubsystemSettings* Settings = GetDefault<UGameFeaturesSubsystemSettings>();
		for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetDiscoveredPlugins())
		{
			const FString DescriptorFilename = FPaths::ConvertRelativePathToFull(Plugin->GetDescriptorFileName());
			if (Settings->IsValidGameFeaturePlugin(DescriptorFilename))
			{
				Result.Add(Plugin);
			}
		}
		return Result;
	}
}

void UIPGPackageSwitchingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UIPGPackageSettings& Settings = UIPGPackageSettings::Get();

	// 시작 시점의 마운트 상태를 기준선으로 잡는다.
	// (정책이 InitGameFeatureManager 에서 이미 이 집합대로 로드해 두었다)
	const UIPGPackageData* ActivePackage = Settings.GetDeterminedActivePackage();
	CurrentPlugins = ComputeDesiredPluginSet(ActivePackage);
	LastAppliedPackage = Settings.ActivePackage;

	ActivePackageChangedHandle = Settings.OnActivePackageChanged.AddUObject(
		this, &UIPGPackageSwitchingSystem::UpdateActivePackageChanged);
}

void UIPGPackageSwitchingSystem::Deinitialize()
{
	if (ActivePackageChangedHandle.IsValid())
	{
		UIPGPackageSettings::Get().OnActivePackageChanged.Remove(ActivePackageChangedHandle);
		ActivePackageChangedHandle.Reset();
	}

	Super::Deinitialize();
}

void UIPGPackageSwitchingSystem::UpdateActivePackageChanged(const UIPGPackageData* NewPackage)
{
	// 롤백으로 인해 다시 들어온 경우
	if (bSwitchInProgress)
	{
		return;
	}

	if (UpdateSwitch(NewPackage))
	{
		LastAppliedPackage = UIPGPackageSettings::Get().ActivePackage;
		return;
	}

	// 전환 실패 — 설정값만 바뀌고 마운트 상태는 그대로인 불일치를 없앤다.
	TGuardValue<bool> ReentryGuard(bSwitchInProgress, true);

	UIPGPackageSettings& Settings = UIPGPackageSettings::Get();
	Settings.ActivePackage = LastAppliedPackage;
	Settings.InvalidateDeterminedPackage();
	Settings.SaveConfig();

	UE_LOG(LogTemp, Warning, TEXT("패키지 전환이 취소되어 이전 값으로 되돌렸습니다."));
}

bool UIPGPackageSwitchingSystem::UpdateSwitch(const UIPGPackageData* NewPackage)
{
	if (bSwitchInProgress)
	{
		return false;
	}

	if (GEditor && GEditor->PlayWorld != nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("NoSwitchDuringPIE", "플레이 중에는 패키지를 전환할 수 없습니다. PIE를 종료한 뒤 다시 시도하세요."));
		return false;
	}

	const TSet<FString> Desired = ComputeDesiredPluginSet(NewPackage);
	const TSet<FString> ToUnmount = CurrentPlugins.Difference(Desired);
	const TSet<FString> ToMount = Desired.Difference(CurrentPlugins);

	if (ToUnmount.IsEmpty() && ToMount.IsEmpty())
	{
		CurrentPlugins = Desired;
		return true;
	}

	TGuardValue<bool> Guard(bSwitchInProgress, true);

	if (!PrepareEditorForSwitch(ToUnmount))
	{
		return false;
	}

	FScopedSlowTask SlowTask(4.0f, LOCTEXT("SwitchingPackage", "패키지를 전환하는 중..."));
	SlowTask.MakeDialog();

	SlowTask.EnterProgressFrame(1.0f, LOCTEXT("Unmounting", "플러그인 언마운트 중..."));
	UnmountPlugins(ToUnmount);

	// GC 를 돌리지 않으면 남은 참조 때문에 마운트 해제가 조용히 실패한다.
	SlowTask.EnterProgressFrame(1.0f, LOCTEXT("CollectingGarbage", "가비지 컬렉션 중..."));
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	// 새 플러그인을 로드하기 전에 반드시 갱신할 것.
	// LoadBuiltInGameFeaturePlugin 이 내부에서 IsPluginAllowed 를 다시 물어본다.
	SlowTask.EnterProgressFrame(1.0f, LOCTEXT("RefreshingPolicy", "정책 갱신 중..."));
	UGameFeaturesSubsystem::Get().GetPolicy<UIPGGameFeaturePolicy>().RebuildAllowedPluginCache();

	SlowTask.EnterProgressFrame(1.0f, LOCTEXT("Mounting", "플러그인 마운트 중..."));
	MountPlugins(ToMount);

	CurrentPlugins = Desired;

	UE_LOG(LogIPGPackageSwitcher, Log,
		TEXT("패키지 전환 완료 — 언마운트 %d개, 마운트 %d개, 현재 %d개"),
		ToUnmount.Num(), ToMount.Num(), CurrentPlugins.Num());

	OnPackageSwitchCompleted.Broadcast();
	return true;
}

TSet<FString> UIPGPackageSwitchingSystem::ComputeDesiredPluginSet(const UIPGPackageData* Package)
{
	TSet<FString> Desired;

	const TArray<TSharedRef<IPlugin>> Installed = GetAllInstalledGameFeaturePlugins();

	// 활성 패키지가 없으면 필터링하지 않는다 (정책과 동일한 규칙).
	if (!IsValid(Package))
	{
		for (const TSharedRef<IPlugin>& Plugin : Installed)
		{
			Desired.Add(Plugin->GetName());
		}
		return Desired;
	}

	const TSet<FString> Allowed(Package->GetAllPluginNames());
	for (const TSharedRef<IPlugin>& Plugin : Installed)
	{
		if (Allowed.Contains(Plugin->GetName()))
		{
			Desired.Add(Plugin->GetName());
		}
	}

	UE_LOG(LogIPGPackageSwitcher, Log, TEXT("패키지 허용 %d개, 설치됨 %d개, 교집합 %d개"),
		Allowed.Num(), Installed.Num(), Desired.Num());

	for (const FString& Name : Allowed)
	{
		UE_LOG(LogIPGPackageSwitcher, Log, TEXT("  허용 목록: '%s' (길이 %d)"), *Name, Name.Len());
	}

	return Desired;
}

bool UIPGPackageSwitchingSystem::PrepareEditorForSwitch(const TSet<FString>& PluginsToUnmount)
{
	if (!GEditor)
	{
		return false;
	}

	// 1. 편집 중인 레벨이 언마운트 대상 안에 있으면 먼저 빠져나온다.
	if (const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
	{
		const FString WorldPackageName = EditorWorld->GetPackage()->GetName();
		for (const FString& PluginName : PluginsToUnmount)
		{
			if (WorldPackageName.StartsWith(FString::Printf(TEXT("/%s/"), *PluginName)))
			{
				UE_LOG(LogIPGPackageSwitcher, Log,
					TEXT("현재 레벨(%s)이 언마운트 대상이므로 새 맵으로 전환합니다."), *WorldPackageName);
				GEditor->CreateNewMapForEditing();
				break;
			}
		}
	}

	// 2. 저장하지 않은 변경 사항. 사용자가 취소하면 전환을 포기한다.
	const bool bSaved = FEditorFileUtils::SaveDirtyPackages(
		/*bPromptUserToSave=*/true,
		/*bSaveMapPackages=*/true,
		/*bSaveContentPackages=*/true);

	if (!bSaved)
	{
		return false;
	}

	// 3. 열린 에셋 에디터가 언마운트될 오브젝트를 붙잡고 있으면 좀비가 된다.
	//    대상만 골라 닫는 것보다 전부 닫는 편이 안전하다.
	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->CloseAllAssetEditors();
	}

	return true;
}

void UIPGPackageSwitchingSystem::UnmountPlugins(const TSet<FString>& PluginNames)
{
	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();

	for (const FString& PluginName : PluginNames)
	{
		FString PluginURL;
		if (!Subsystem.GetPluginURLByName(PluginName, PluginURL))
		{
			UE_LOG(LogIPGPackageSwitcher, Warning,
				TEXT("URL을 찾지 못해 언마운트를 건너뜁니다: %s"), *PluginName);
			continue;
		}

		// Deactivate/Unload 로는 마운트 포인트가 남는다. 콘텐츠를 에디터에서
		// 완전히 내리려면 Terminal 상태까지 보내야 한다.
		Subsystem.TerminateGameFeaturePlugin(PluginURL,
			FGameFeaturePluginTerminateComplete::CreateLambda(
				[PluginName](const UE::GameFeatures::FResult& Result)
				{
					if (!Result.HasValue())
					{
						UE_LOG(LogIPGPackageSwitcher, Error,
							TEXT("언마운트 실패: %s"), *PluginName);
					}
				}));
	}
}

void UIPGPackageSwitchingSystem::MountPlugins(const TSet<FString>& PluginNames)
{
	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();

	for (const FString& PluginName : PluginNames)
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
		if (!Plugin.IsValid())
		{
			UE_LOG(LogIPGPackageSwitcher, Warning,
				TEXT("설치되어 있지 않아 마운트를 건너뜁니다: %s"), *PluginName);
			continue;
		}

		// 정책 캐시가 이미 갱신되었으므로 추가 필터는 통과시키기만 하면 된다.
		auto AdditionalFilter = [](const FString& PluginFilename,
			const FGameFeaturePluginDetails& Details,
			FBuiltInGameFeaturePluginBehaviorOptions& OutOptions) -> bool
		{
			return true;
		};

		Subsystem.LoadBuiltInGameFeaturePlugin(Plugin.ToSharedRef(), AdditionalFilter,
			FGameFeaturePluginLoadComplete::CreateLambda(
				[PluginName](const UE::GameFeatures::FResult& Result)
				{
					if (!Result.HasValue())
					{
						UE_LOG(LogIPGPackageSwitcher, Error,
							TEXT("마운트 실패: %s"), *PluginName);
					}
				}));
	}
}

#undef LOCTEXT_NAMESPACE
