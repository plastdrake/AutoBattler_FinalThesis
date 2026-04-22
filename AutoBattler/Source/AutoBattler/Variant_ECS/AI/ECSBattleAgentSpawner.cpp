#include "ECSBattleAgentSpawner.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Avoidance/MassAvoidanceFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Steering/MassSteeringFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "StructUtils/StructView.h"

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
			else if (Line.StartsWith(TEXT("BenchmarkCount=")))
			{
				CountOption = Line.RightChop(15);
			}
		}

		if (bBatchEnabled)
		{
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

			bShouldSpawn = ModelOption.Equals(TEXT("ECS"), ESearchCase::IgnoreCase);
		}
	}

	if (bShouldSpawn)
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

	FMassMovementParameters MovementParameters;
	MovementParameters.MaxSpeed = MoveSpeed;
	MovementParameters.DefaultDesiredSpeed = MoveSpeed;
	MovementParameters.MaxAcceleration = MoveSpeed * 8.0f;
	MovementParameters.bIsCodeDrivenMovement = true;
	const FConstSharedStruct MovementParamsShared = EntityManager.GetOrCreateConstSharedFragment<FMassMovementParameters>(FConstStructView::Make(MovementParameters), MovementParameters);

	FMassMovingAvoidanceParameters AvoidanceParameters;
	const FConstSharedStruct AvoidanceParamsShared = EntityManager.GetOrCreateConstSharedFragment<FMassMovingAvoidanceParameters>(FConstStructView::Make(AvoidanceParameters), AvoidanceParameters);

	FMassStandingAvoidanceParameters StandingAvoidanceParameters;
	const FConstSharedStruct StandingAvoidanceParamsShared = EntityManager.GetOrCreateConstSharedFragment<FMassStandingAvoidanceParameters>(FConstStructView::Make(StandingAvoidanceParameters), StandingAvoidanceParameters);

	FMassMovingSteeringParameters MovingSteeringParameters;
	const FConstSharedStruct MovingSteeringParamsShared = EntityManager.GetOrCreateConstSharedFragment<FMassMovingSteeringParameters>(FConstStructView::Make(MovingSteeringParameters), MovingSteeringParameters);

	FMassStandingSteeringParameters StandingSteeringParameters;
	const FConstSharedStruct StandingSteeringParamsShared = EntityManager.GetOrCreateConstSharedFragment<FMassStandingSteeringParameters>(FConstStructView::Make(StandingSteeringParameters), StandingSteeringParameters);

	FMassArchetypeSharedFragmentValues SharedFragmentValues;
	SharedFragmentValues.Add(MovementParamsShared);
	SharedFragmentValues.Add(AvoidanceParamsShared);
    SharedFragmentValues.Add(StandingAvoidanceParamsShared);
    SharedFragmentValues.Add(MovingSteeringParamsShared);
	SharedFragmentValues.Add(StandingSteeringParamsShared);
	SharedFragmentValues.Sort();

	EntityManager.BatchCreateEntities(Archetype, SharedFragmentValues, SpawnCount, SpawnedEntities);
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

		// Assign a stable slot angle so formation remains consistent (spread around target)
		const float SlotHash = FMath::Frac(static_cast<float>(Index) * 0.61803398875f);
		AgentData.SlotAngleDegrees = (SlotHash * 2.0f - 1.0f) * 70.0f;
		AgentData.bTriggerAttackMontage = false;

		FMassVelocityFragment& VelocityData = EntityManager.GetFragmentDataChecked<FMassVelocityFragment>(SpawnedEntities[Index]);
		VelocityData.Value = FVector::ZeroVector;

		FMassDesiredMovementFragment& DesiredMovementData = EntityManager.GetFragmentDataChecked<FMassDesiredMovementFragment>(SpawnedEntities[Index]);
		DesiredMovementData.DesiredVelocity = FVector::ZeroVector;
		DesiredMovementData.DesiredFacing = SpawnRotation.Quaternion();

		FMassForceFragment& ForceData = EntityManager.GetFragmentDataChecked<FMassForceFragment>(SpawnedEntities[Index]);
		ForceData.Value = FVector::ZeroVector;

		FMassMoveTargetFragment& MoveTargetData = EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(SpawnedEntities[Index]);
		MoveTargetData.Center = SpawnLocation;
		MoveTargetData.Forward = SpawnRotation.Vector();
		MoveTargetData.DistanceToGoal = 0.0f;
		MoveTargetData.SlackRadius = 0.0f;
		MoveTargetData.DesiredSpeed = FMassInt16Real(MoveSpeed);
		MoveTargetData.IntentAtGoal = EMassMovementAction::Stand;

		FAgentRadiusFragment& AgentRadiusData = EntityManager.GetFragmentDataChecked<FAgentRadiusFragment>(SpawnedEntities[Index]);
		AgentRadiusData.Radius = AgentRadius;

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
   Composition.GetFragments().Add<FMassVelocityFragment>();
	Composition.GetFragments().Add<FMassDesiredMovementFragment>();
	Composition.GetFragments().Add<FMassForceFragment>();
	Composition.GetFragments().Add<FMassMoveTargetFragment>();
  Composition.GetFragments().Add<FMassSteeringFragment>();
	Composition.GetFragments().Add<FMassStandingSteeringFragment>();
 Composition.GetFragments().Add<FMassGhostLocationFragment>();
	Composition.GetFragments().Add<FMassNavigationEdgesFragment>();
	Composition.GetFragments().Add<FMassNavigationObstacleGridCellLocationFragment>();
	Composition.GetFragments().Add<FAgentRadiusFragment>();
	Composition.GetTags().Add<FMassCodeDrivenMovementTag>();
	Composition.GetFragments().Add<FTransformFragment>();
	return EntityManager.CreateArchetype(Composition);
}
