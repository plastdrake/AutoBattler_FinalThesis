#include "ECSBattleAgentSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"

AECSBattleAgentSpawner::AECSBattleAgentSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	SpawnDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnDirection"));
	SpawnDirection->SetupAttachment(RootComponent);
}

void AECSBattleAgentSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnBeginPlay)
	{
		SpawnAgents();
	}
}

void AECSBattleAgentSpawner::SpawnAgents()
{
	UWorld* World = GetWorld();
	if (!World || SpawnCount <= 0)
	{
     UE_LOG(LogTemp, Warning, TEXT("ECS Spawner '%s': invalid world or SpawnCount <= 0"), *GetName());
		return;
	}

	UMassEntitySubsystem* MassSubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (!MassSubsystem)
	{
     UE_LOG(LogTemp, Error, TEXT("ECS Spawner '%s': MassEntitySubsystem not found"), *GetName());
		return;
	}

	FMassEntityManager& EntityManager = MassSubsystem->GetMutableEntityManager();
	const FMassArchetypeHandle Archetype = CreateOrGetAgentArchetype(EntityManager);
	if (!Archetype.IsValid())
	{
     UE_LOG(LogTemp, Error, TEXT("ECS Spawner '%s': Failed to create archetype"), *GetName());
		return;
	}

	const FVector StartLocation = SpawnDirection->GetComponentLocation();
	const FVector RightVector = SpawnDirection->GetRightVector();
	const FVector ForwardVector = SpawnDirection->GetForwardVector();
	const FRotator SpawnRotation = SpawnDirection->GetComponentRotation();
	const int32 Rows = FMath::Max(1, SpawnRows);
	const int32 Columns = FMath::CeilToInt(static_cast<float>(SpawnCount) / static_cast<float>(Rows));

	TArray<FMassEntityHandle> SpawnedEntities;
	SpawnedEntities.Reserve(SpawnCount);
	EntityManager.BatchCreateEntities(Archetype, SpawnCount, SpawnedEntities);
	UE_LOG(LogTemp, Warning, TEXT("ECS Spawner '%s': requested %d, created %d entities"), *GetName(), SpawnCount, SpawnedEntities.Num());

	for (int32 Index = 0; Index < SpawnedEntities.Num(); ++Index)
	{
		const int32 RowIndex = Index / Columns;
		const int32 ColumnIndex = Index % Columns;
		const int32 RemainingAgents = SpawnCount - (RowIndex * Columns);
		const int32 AgentsInThisRow = FMath::Min(Columns, RemainingAgents);

		const float SideOffset = (ColumnIndex - ((AgentsInThisRow - 1) * 0.5f)) * SpawnSpacing;
		const float ForwardOffset = RowIndex * RowSpacing;
		const FVector SpawnLocation = StartLocation + (RightVector * SideOffset) - (ForwardVector * ForwardOffset);

		FECSBattleAgentFragment& AgentData = EntityManager.GetFragmentDataChecked<FECSBattleAgentFragment>(SpawnedEntities[Index]);
		AgentData.Team = Team;
		AgentData.MaxHealth = MaxHealth;
		AgentData.CurrentHealth = MaxHealth;
		AgentData.MoveSpeed = MoveSpeed;
		AgentData.AttackRange = AttackRange;
		AgentData.AttackDamage = AttackDamage;
		AgentData.AttackInterval = AttackInterval;
		AgentData.TargetSearchRadius = TargetSearchRadius;
		AgentData.TargetRefreshInterval = TargetRefreshInterval;
		AgentData.DestroyDelay = DestroyDelay;
		AgentData.TimeUntilNextAttack = 0.0f;
		AgentData.TimeUntilTargetRefresh = 0.0f;
		AgentData.LifeTimeRemaining = 0.0f;
		AgentData.CurrentTarget = FMassEntityHandle();
		AgentData.bDying = false;
		AgentData.bPendingEntityDestroy = false;

		FECSBattleAgentRepresentationFragment& RepresentationData = EntityManager.GetFragmentDataChecked<FECSBattleAgentRepresentationFragment>(SpawnedEntities[Index]);
     RepresentationData.VisualCharacterClassAddress = reinterpret_cast<uint64>(VisualCharacterClass.Get());

		FTransformFragment& TransformData = EntityManager.GetFragmentDataChecked<FTransformFragment>(SpawnedEntities[Index]);
		TransformData.SetTransform(FTransform(SpawnRotation, SpawnLocation));
	}
}

FMassArchetypeHandle AECSBattleAgentSpawner::CreateOrGetAgentArchetype(FMassEntityManager& EntityManager) const
{
	FMassArchetypeCompositionDescriptor Composition;
   Composition.GetFragments().Add<FECSBattleAgentFragment>();
   Composition.GetFragments().Add<FECSBattleAgentRepresentationFragment>();
	Composition.GetFragments().Add<FTransformFragment>();
	return EntityManager.CreateArchetype(Composition);
}
