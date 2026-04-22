#include "OOPBattleAgentSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

AOOPBattleAgentSpawner::AOOPBattleAgentSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	SpawnDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirection"));
	SpawnDirection->SetupAttachment(RootComponent);
}

void AOOPBattleAgentSpawner::BeginPlay()
{
	Super::BeginPlay();

    bool bShouldSpawn = bSpawnOnBeginPlay;
    if (UWorld* World = GetWorld())
	{
        const FString BatchFlag = World->URL.GetOption(TEXT("BenchmarkBatch="), TEXT(""));
		if (BatchFlag == TEXT("1"))
		{
           const FString ModelOption = World->URL.GetOption(TEXT("BenchmarkModel="), TEXT(""));
			const FString CountOption = World->URL.GetOption(TEXT("BenchmarkCount="), TEXT("0"));
			const int32 TotalAgentCount = FMath::Max(0, FCString::Atoi(*CountOption));
			SpawnCount = TotalAgentCount / 2;

			bShouldSpawn = ModelOption.Equals(TEXT("OOP"), ESearchCase::IgnoreCase);
           UE_LOG(LogTemp, Log, TEXT("OOP Spawner '%s': batch=1 model='%s' total=%d perTeam=%d spawn=%d"), *GetName(), *ModelOption, TotalAgentCount, SpawnCount, bShouldSpawn ? 1 : 0);
		}
	}

	if (bShouldSpawn)
	{
		SpawnAgents();
	}
}

void AOOPBattleAgentSpawner::SpawnAgents()
{
	if (!GetWorld() || !IsValid(AgentClass) || SpawnCount <= 0)
	{
		return;
	}

	const FVector StartLocation = SpawnDirection->GetComponentLocation();
	const FVector RightVector = SpawnDirection->GetRightVector();
  const FVector ForwardVector = SpawnDirection->GetForwardVector();
	const FRotator SpawnRotation = SpawnDirection->GetComponentRotation();
	const int32 Rows = FMath::Max(1, SpawnRows);
	const int32 Columns = FMath::CeilToInt(static_cast<float>(SpawnCount) / static_cast<float>(Rows));

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
        const int32 RowIndex = Index / Columns;
		const int32 ColumnIndex = Index % Columns;
		const int32 RemainingAgents = SpawnCount - (RowIndex * Columns);
		const int32 AgentsInThisRow = FMath::Min(Columns, RemainingAgents);

		const float SideOffset = (ColumnIndex - ((AgentsInThisRow - 1) * 0.5f)) * SpawnSpacing;
		const float ForwardOffset = RowIndex * RowSpacing;
		const FVector SpawnLocation = StartLocation + (RightVector * SideOffset) - (ForwardVector * ForwardOffset);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AOOPBattleAgent* SpawnedAgent = GetWorld()->SpawnActor<AOOPBattleAgent>(AgentClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (IsValid(SpawnedAgent))
		{
			SpawnedAgent->SetTeam(Team);
		}
	}
}
