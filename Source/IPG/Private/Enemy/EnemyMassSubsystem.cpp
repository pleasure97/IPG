// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyMassSubsystem.h"
#include "Enemy/EnemyMassFragment.h"
#include "System/MassEntityEnemySettings.h"
#include "Kismet/GameplayStatics.h"
#include "EnvironmentQuery/EQSTestingPawn.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "MassEntityConfigAsset.h"
#include "NavigationSystem.h"
#include "MassEntitySubsystem.h"
#include "MassEntityManager.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"

void UEnemyMassSubsystem::RequestSpawnEnemyEQS(int32 Count, EEnemyType Type)
{
	QueryAllEQS([this, Count, Type](const TArray<TSharedPtr<FEnvQueryResult>>& EnvQueryResults)
	{
		// In callback, need to collect locations from env query results
		TArray<FVector> Locations; 
		Locations.Reserve(Count);

		const int32 EQSNum = EnvQueryResults.Num(); 
		for (int32 i = 0; i < EQSNum; ++i)
		{
			// Check if env query result is valid
			if (!EnvQueryResults[i] || !EnvQueryResults[i]->IsSuccessful())
			{
				continue;
			}

			// Use spawn location evenly for each EQS point
			// Calculate batch count differently if it's last index
			const int32 BatchCount = (i == EQSNum - 1) ? Count - (Count / EQSNum) * (EQSNum - 1) : Count / EQSNum;

			// Use weighted random sampling
			Locations.Append(GetWeightedRandomLocations(*EnvQueryResults[i], BatchCount)); 
		}

		// Make request based on spawn locations
		const int32 SpawnCount = FMath::Min(Count, Locations.Num()); 
		FEnemySpawnRequest EnemySpawnRequest; 
		EnemySpawnRequest.EnemyType = Type;
		EnemySpawnRequest.Remaining = SpawnCount;
		EnemySpawnRequest.SpawnLocations = MoveTemp(Locations);

		// Add the request to the queue (queue has ownership of request, so we'll use move semantics)
		EnqueueSpawnRequest(MoveTemp(EnemySpawnRequest)); 
	});
}

void UEnemyMassSubsystem::RequestSpawnEnemyLocation(int32 Count, const FVector& Location, EEnemyType Type)
{
	TArray<FVector> Locations = GetGridLocations(Location, Count); 

	const int32 SpawnCount = FMath::Min(Count, Locations.Num()); 

	FEnemySpawnRequest EnemySpawnRequest;
	EnemySpawnRequest.EnemyType = Type;
	EnemySpawnRequest.Remaining = SpawnCount;
	EnemySpawnRequest.SpawnLocations = MoveTemp(Locations); 

	EnqueueSpawnRequest(MoveTemp(EnemySpawnRequest));
}

bool UEnemyMassSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	const UWorld* World = Outer->GetWorld(); 
	if (!IsValid(World))
	{
		return false;
	}

	// Server-Authorative
	if (World->GetNetMode() == NM_Client)
	{
		return false;
	}

	// Get mass entity enemy developer settings 
	const UMassEntityEnemySettings* MassEntityEnemySettings = GetDefault<UMassEntityEnemySettings>();
	if (!IsValid(MassEntityEnemySettings))
	{
		return false;
	}

	// Get current map name 
	const FString CurrentMapName = FPackageName::GetShortName(
		UWorld::StripPIEPrefixFromPackageName(World->GetOutermost()->GetName(), World->StreamingLevelsPrefix));

	// Find a spawnable world that matches current map name in developer settings
	for (const auto& SpawnableWorld : MassEntityEnemySettings->SpawnableWorlds)
	{
		if (SpawnableWorld.IsNull())
		{
			continue;
		}

		if (CurrentMapName.Equals(SpawnableWorld.GetAssetName(), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

void UEnemyMassSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Get mass entity enemy developer settings 
	const UMassEntityEnemySettings* MassEntityEnemySettings = GetDefault<UMassEntityEnemySettings>();
	if (!IsValid(MassEntityEnemySettings))
	{
		return;
	}

	// Find "EQSTestingPawns" in the world and store them in EQSPawns
	UGameplayStatics::GetAllActorsOfClass(&InWorld, AEQSTestingPawn::StaticClass(), EQSPawns);

	for (const auto& [Type, Config] : MassEntityEnemySettings->EntityConfigs)
	{
		if (Type == EEnemyType::Random)
		{
			continue;
		}

		if (UMassEntityConfigAsset* LoadedConfig = Config.LoadSynchronous())
		{
			EntityTemplateMap.Add(Type, LoadedConfig->GetOrCreateEntityTemplate(InWorld));
			EnemyTypes.Add(Type);
		}
	}

	EQS = MassEntityEnemySettings->SpawnPointQuery.LoadSynchronous(); 
	MassSpawnerSubsystem = InWorld.GetSubsystem<UMassSpawnerSubsystem>();
	NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(&InWorld);

	if (UMassEntitySubsystem* MassEntitySubsystem = InWorld.GetSubsystem<UMassEntitySubsystem>())
	{
		MassEntityManager = &MassEntitySubsystem->GetMutableEntityManager();
	}
}

void UEnemyMassSubsystem::EnqueueSpawnRequest(FEnemySpawnRequest&& Request)
{
	EnemySpawnQueue.Enqueue(MoveTemp(Request));

	if (const UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(EnemySpawnTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				EnemySpawnTimerHandle, this,
				&UEnemyMassSubsystem::ProcessSpawnRequest, 0.1f/*Rate*/, true/*Loop*/);
		}
	}
}

void UEnemyMassSubsystem::ProcessSpawnRequest()
{
	if (!MassEntityManager || !MassSpawnerSubsystem.IsValid())
	{
		return;
	}

	// Get mass entity enemy developer settings 
	const UMassEntityEnemySettings* MassEntityEnemySettings = GetDefault<UMassEntityEnemySettings>();
	if (!IsValid(MassEntityEnemySettings))
	{
		return;
	}

	int32 RemainingThisTick = MassEntityEnemySettings->MaxSpawnPerTick;

	FEnemySpawnRequest EnemySpawnRequest;
	while (EnemySpawnQueue.Dequeue(EnemySpawnRequest))
	{
		if (!EnemySpawnRequest.IsValid())
		{
			continue;
		}

		// Find entity template matching enemy spawn request
		FMassEntityTemplate* MassEntityTemplate = nullptr;
		// Random
		if (EnemySpawnRequest.EnemyType == EEnemyType::Random)
		{
			if (!EnemyTypes.IsEmpty())
			{
				int32 RandomIndex = FMath::RandRange(0, EnemyTypes.Num() - 1); 
				MassEntityTemplate = EntityTemplateMap.Find(EnemyTypes[RandomIndex]);
			}
		}
		// Not Random
		else
		{
			MassEntityTemplate = EntityTemplateMap.Find(EnemySpawnRequest.EnemyType);
		}

		if (!MassEntityTemplate)
		{
			continue;
		}

		// Calculate how many spawns need in this tick
		const int32 CurrentSpawn = FMath::Min(EnemySpawnRequest.Remaining, RemainingThisTick); 

		// Spawn mass entities
		TArray<FMassEntityHandle> RequestMassEntities;
		MassSpawnerSubsystem->SpawnEntities(*MassEntityTemplate, CurrentSpawn, RequestMassEntities);

		// Set actual spawned mass location considering spawning in uneven terrains
		const int32 ActualSpawned = FMath::Min(RequestMassEntities.Num(), EnemySpawnRequest.SpawnLocations.Num());

		const int32 AvailableOffset = EnemySpawnRequest.SpawnLocations.Num() - EnemySpawnRequest.Remaining;

		for (int32 i = 0; i < ActualSpawned; ++i)
		{
			SetMassLocation(RequestMassEntities[i], EnemySpawnRequest.SpawnLocations[AvailableOffset + i]);
		}

		// Update spawn request's remaining 
		EnemySpawnRequest.Remaining -= ActualSpawned;
		RemainingThisTick -= ActualSpawned;

		// If there is anything left to spawn, add it to the spawn queue
		if (EnemySpawnRequest.Remaining > 0)
		{
			EnemySpawnQueue.Enqueue(MoveTemp(EnemySpawnRequest));
		}
		if (RemainingThisTick <= 0)
		{
			break;
		}
	}

	if (EnemySpawnQueue.IsEmpty())
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(EnemySpawnTimerHandle);
		}
	}
}

void UEnemyMassSubsystem::SetMassLocation(const FMassEntityHandle& MassEntity, const FVector& Location)
{
	// Determine the correct z-axis using the navigation system 
	FVector Position = Location;
	if (NavigationSystem.IsValid())
	{
		FNavLocation ProjectedLocation; 
		if (NavigationSystem->ProjectPointToNavigation(Position, ProjectedLocation))
		{
			Position = ProjectedLocation;
		}
	}

	if (MassEntityManager)
	{
		// Modify the transform fragment via the entity manager
		if (FTransformFragment* TransformFragment = MassEntityManager->GetFragmentDataPtr<FTransformFragment>(MassEntity))
		{
			TransformFragment->GetMutableTransform().SetLocation(Position);
		}

		if (FEnemyWanderFragment* EnemyWanderFragment = MassEntityManager->GetFragmentDataPtr<FEnemyWanderFragment>(MassEntity))
		{
			EnemyWanderFragment->Origin = Position;
		}
	}
}

void UEnemyMassSubsystem::QueryAllEQS(const TFunction<void(const TArray<TSharedPtr<FEnvQueryResult>>& Results)>& Callback)
{
	if (EQSPawns.IsEmpty())
	{
		return;
	}

	UEnvQueryManager* EQSManager = UEnvQueryManager::GetCurrent(GetWorld()); 
	if (!IsValid(EQS) || !IsValid(EQSManager))
	{
		return;
	}

	// Define a shared pointer which collects query result values
	auto Pending = MakeShared<TPair<int32, TArray<TSharedPtr<FEnvQueryResult>>>>(); 
	// Key = the number of queries remaining until the callback is called
	Pending->Key = EQSPawns.Num(); 
	Pending->Value.Reserve(EQSPawns.Num()); 

	// Iterate EQS Pawn array and execute each query 
	for (auto* EQSPawn : EQSPawns)
	{
		// If EQS pawn is not valid, subtract key and skip
		if (!IsValid(EQSPawn))
		{
			--Pending->Key;
			continue;
		}

		// Register query delegate
		FQueryFinishedSignature QueryFinishedSignature;
		QueryFinishedSignature.BindWeakLambda(this,
			[Pending, Callback](const TSharedPtr<FEnvQueryResult>& Result)
			{
				Pending->Value.Add(Result);
				if (--Pending->Key == 0)
				{
					Callback(Pending->Value);
				}
			});

		// Request the query through query manager 
		FEnvQueryRequest EnvQueryRequest(EQS, EQSPawn); 
		EQSManager->RunQuery(EnvQueryRequest, EEnvQueryRunMode::AllMatching, QueryFinishedSignature); 
	}
}

TArray<FVector> UEnemyMassSubsystem::GetGridLocations(const FVector& Center, int32 Count)
{
	// Get mass entity enemy developer settings 
	const UMassEntityEnemySettings* MassEntityEnemySettings = GetDefault<UMassEntityEnemySettings>();
	if (!IsValid(MassEntityEnemySettings))
	{
		return TArray<FVector>(); 
	}

	const float Padding = MassEntityEnemySettings->GridSpacing;

	TArray<FVector> Locations;
	Locations.Reserve(Count); 

	const int32 GridCol = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Count))); 
	const int32 GridRow = FMath::CeilToInt(static_cast<float>(Count) / GridCol); 

	const float GridWidth = (GridCol - 1) * Padding;
	const float GridHeight = (GridRow - 1) * Padding;

	const FVector GridOrigin = Center - FVector(GridWidth, GridHeight, 0.f) * 5.f;

	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Row = i / GridCol;
		const int32 Col = i % GridCol;

		Locations.Add(GridOrigin + FVector(Col, Row, 0.f) * Padding); 
	}

	return Locations;
}

TArray<FVector> UEnemyMassSubsystem::GetWeightedRandomLocations(const FEnvQueryResult& EnvQueryResult, int32 Count)
{
	if (Count <= 0 || !EnvQueryResult.IsSuccessful() || EnvQueryResult.Items.IsEmpty())
	{
		return TArray<FVector>(); 
	}

	// Create an array that stores random value 
	TArray<TPair<float, int32>> Weighted;
	Weighted.Reserve(EnvQueryResult.Items.Num()); 

	for (int32 i = 0; i < EnvQueryResult.Items.Num(); ++i)
	{
		// For weighted random sampling, we need valid score and u value 
		const float Score = FMath::Max(EnvQueryResult.Items[i].Score, KINDA_SMALL_NUMBER); 
		const float U = FMath::Max(FMath::FRand(), KINDA_SMALL_NUMBER); 
		// Calculate weight using log e function and add it to the array 
		Weighted.Emplace(FMath::Loge(U) / Score, i); 
	}

	// Get actual count
	const int32 ActualCount = FMath::Min(Count, Weighted.Num()); 

	// Use 'QuickSelect' algorithm to access higher score
	std::nth_element(Weighted.GetData(), Weighted.GetData() + ActualCount, Weighted.GetData() + Weighted.Num(),
		[](const auto& A, const auto& B)
		{
			return A.Key > B.Key;
		});

	TArray<FVector> Out;
	Out.Reserve(ActualCount); 

	for (int32 i = 0; i < ActualCount; ++i)
	{
		Out.Add(EnvQueryResult.GetItemAsLocation(Weighted[i].Value)); 
	}

	return Out;
}
