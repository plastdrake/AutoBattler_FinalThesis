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

	if (bSpawnOnBeginPlay)
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
	const FRotator SpawnRotation = SpawnDirection->GetComponentRotation();

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		const float Offset = (Index - ((SpawnCount - 1) * 0.5f)) * SpawnSpacing;
		const FVector SpawnLocation = StartLocation + (RightVector * Offset);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AOOPBattleAgent* SpawnedAgent = GetWorld()->SpawnActor<AOOPBattleAgent>(AgentClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (IsValid(SpawnedAgent))
		{
			SpawnedAgent->SetTeam(Team);
		}
	}
}
