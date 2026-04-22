#include "OOPBattleAgentSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

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
 const FString RunStatePath = FPaths::ProjectSavedDir() / TEXT("Benchmark/BattleBenchmark_RunState.ini");
	FString RunStateText;
	if (FFileHelper::LoadFileToString(RunStateText, *RunStatePath))
	{
      bool bBatchEnabled = false;
		FString ModelOption;
		FString CountOption;

		TArray<FString> Lines;
		RunStateText.ParseIntoArrayLines(Lines, true);
		for (const FString& Line : Lines)
		{
           if (Line.StartsWith(TEXT("Enabled=")))
			{
				bBatchEnabled = FCString::Atoi(*Line.RightChop(8)) != 0;
			}
          else if (Line.StartsWith(TEXT("BenchmarkModel=")))
			{
				ModelOption = Line.RightChop(15);
			}
			else if (Line.StartsWith(TEXT("ModelIndex=")))
			{
               if (ModelOption.IsEmpty())
				{
					const int32 ModelIndex = FCString::Atoi(*Line.RightChop(11));
					ModelOption = (ModelIndex == 1) ? TEXT("ECS") : TEXT("OOP");
				}
			}
			else if (Line.StartsWith(TEXT("CountIndex=")))
			{
				// not needed here
			}
			else if (Line.StartsWith(TEXT("BenchmarkCount=")))
			{
				CountOption = Line.RightChop(15);
			}
		}

		if (bBatchEnabled)
		{
			// Count is indexed by the controller, but we also support direct count value if present.
			if (CountOption.IsEmpty())
			{
               for (const FString& Line : Lines)
				{
					if (Line.StartsWith(TEXT("CountIndex=")))
					{
						const int32 CountIndex = FCString::Atoi(*Line.RightChop(11));
						static const int32 BatchCounts[] = { 10, 20, 40, 80, 160, 320, 640 };
						if (CountIndex >= 0 && CountIndex < UE_ARRAY_COUNT(BatchCounts))
						{
							SpawnCount = BatchCounts[CountIndex];
						}
						break;
					}
				}
			}
			else
			{
				SpawnCount = FMath::Max(0, FCString::Atoi(*CountOption));
			}

			bShouldSpawn = ModelOption.Equals(TEXT("OOP"), ESearchCase::IgnoreCase);
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
