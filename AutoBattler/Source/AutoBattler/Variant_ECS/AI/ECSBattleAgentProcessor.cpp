#include "ECSBattleAgentProcessor.h"
#include "ECSBattleAgentFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"

namespace
{
	struct FECSAgentSnapshot
	{
		FMassEntityHandle Entity;
		FVector Location = FVector::ZeroVector;
		EECSBattleAgentTeam Team = EECSBattleAgentTeam::Red;
		bool bAlive = false;
	};

 struct FECSSpatialHash
	{
        float CellSize = 200.0f;
		TMap<int64, TArray<int32>> CellToSnapshotIndices;
		TArray<int32> AliveByTeam[2];

		static int32 TeamToIndex(const EECSBattleAgentTeam Team)
		{
			return (Team == EECSBattleAgentTeam::Blue) ? 1 : 0;
		}

		static FIntPoint ToCell(const FVector& Location, const float InCellSize)
		{
			return FIntPoint(
				FMath::FloorToInt(Location.X / InCellSize),
				FMath::FloorToInt(Location.Y / InCellSize));
		}

		static int64 MakeCellKey(const FIntPoint& Cell)
		{
			return (static_cast<int64>(Cell.X) << 32) | static_cast<uint32>(Cell.Y);
		}

		void Build(const TArray<FECSAgentSnapshot>& Snapshots, const float InCellSize)
		{
			CellSize = FMath::Max(80.0f, InCellSize);
			CellToSnapshotIndices.Reset();
			AliveByTeam[0].Reset();
			AliveByTeam[1].Reset();

			for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
			{
				const FECSAgentSnapshot& Snapshot = Snapshots[SnapshotIndex];
				if (!Snapshot.bAlive)
				{
					continue;
				}

				const FIntPoint Cell = ToCell(Snapshot.Location, CellSize);
				CellToSnapshotIndices.FindOrAdd(MakeCellKey(Cell)).Add(SnapshotIndex);
				AliveByTeam[TeamToIndex(Snapshot.Team)].Add(SnapshotIndex);
			}
		}

		template <typename VisitorType>
		void ForEachSnapshotInRadius(const FVector& Center, const float Radius, const TArray<FECSAgentSnapshot>& Snapshots, VisitorType&& Visitor) const
		{
			if (Radius <= 0.0f || Snapshots.Num() == 0)
			{
				return;
			}

			const FVector MinBounds(Center.X - Radius, Center.Y - Radius, 0.0f);
			const FVector MaxBounds(Center.X + Radius, Center.Y + Radius, 0.0f);
			const FIntPoint MinCell = ToCell(MinBounds, CellSize);
			const FIntPoint MaxCell = ToCell(MaxBounds, CellSize);
			const float RadiusSquared = FMath::Square(Radius);

			for (int32 CellY = MinCell.Y; CellY <= MaxCell.Y; ++CellY)
			{
				for (int32 CellX = MinCell.X; CellX <= MaxCell.X; ++CellX)
				{
					const int64 CellKey = MakeCellKey(FIntPoint(CellX, CellY));
					const TArray<int32>* Bucket = CellToSnapshotIndices.Find(CellKey);
					if (!Bucket)
					{
						continue;
					}

					for (const int32 SnapshotIndex : *Bucket)
					{
						if (FVector::DistSquared2D(Center, Snapshots[SnapshotIndex].Location) <= RadiusSquared)
						{
							Visitor(SnapshotIndex);
						}
					}
				}
			}
		}
	};

    int32 FindSnapshotIndex(const TArray<int32>& SnapshotIndexByEntityIndex, const FMassEntityHandle Entity)
	{
		if (!Entity.IsSet() || Entity.Index < 0 || Entity.Index >= SnapshotIndexByEntityIndex.Num())
		{
			return INDEX_NONE;
		}

		return SnapshotIndexByEntityIndex[Entity.Index];
	}

int32 FindNearestEnemyIndex(const FECSAgentSnapshot& SourceSnapshot, const FECSBattleAgentFragment& SourceAgent, const TArray<FECSAgentSnapshot>& Snapshots, const FECSSpatialHash& SpatialHash)
{
    const float SearchRadiusSquared = FMath::Square(SourceAgent.TargetSearchRadius);
    float BestDistanceSquared = MAX_flt;
    int32 BestIndex = INDEX_NONE;

    for (int32 CandidateIndex = 0; CandidateIndex < Snapshots.Num(); ++CandidateIndex)
    {
        const FECSAgentSnapshot& Candidate = Snapshots[CandidateIndex];
        if (!Candidate.bAlive || Candidate.Entity == SourceSnapshot.Entity || Candidate.Team == SourceAgent.Team)
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared2D(SourceSnapshot.Location, Candidate.Location);
        if (DistanceSquared > SearchRadiusSquared || DistanceSquared >= BestDistanceSquared)
        {
            continue;
        }

        BestDistanceSquared = DistanceSquared;
        BestIndex = CandidateIndex;
    }

    return BestIndex;
}
}

UECSBattleAgentProcessor::UECSBattleAgentProcessor()
	: AgentQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::All);
	bAutoRegisterWithProcessingPhases = true;
}

void UECSBattleAgentProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AgentQuery.AddRequirement<FECSBattleAgentFragment>(EMassFragmentAccess::ReadWrite);
	AgentQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    AgentQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
    AgentQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	AgentQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	AgentQuery.RegisterWithProcessor(*this);
}

void UECSBattleAgentProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = Context.GetDeltaTimeSeconds();
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	TArray<FECSAgentSnapshot> Snapshots;
	Snapshots.Reserve(AgentQuery.GetNumMatchingEntities());
	int32 MaxEntityIndex = INDEX_NONE;

    AgentQuery.ForEachEntityChunk(Context, [&Snapshots, &MaxEntityIndex](FMassExecutionContext& QueryContext)
	{
		const TConstArrayView<FECSBattleAgentFragment> AgentFragments = QueryContext.GetFragmentView<FECSBattleAgentFragment>();
		const TConstArrayView<FTransformFragment> TransformFragments = QueryContext.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassEntityHandle> Entities = QueryContext.GetEntities();

		for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
		{
			const FECSBattleAgentFragment& Agent = AgentFragments[EntityIndex];
			const FMassEntityHandle Entity = Entities[EntityIndex];

			FECSAgentSnapshot& Snapshot = Snapshots.AddDefaulted_GetRef();
			Snapshot.Entity = Entity;
			Snapshot.Location = TransformFragments[EntityIndex].GetTransform().GetLocation();
			Snapshot.Team = Agent.Team;
			Snapshot.bAlive = !Agent.bDying && Agent.CurrentHealth > 0.0f;
			MaxEntityIndex = FMath::Max(MaxEntityIndex, Entity.Index);
		}
	});

    if (Snapshots.Num() == 0)
	{
		return;
	}

	TArray<int32> SnapshotIndexByEntityIndex;
	if (MaxEntityIndex >= 0)
	{
		SnapshotIndexByEntityIndex.Init(INDEX_NONE, MaxEntityIndex + 1);
		for (int32 SnapshotIndex = 0; SnapshotIndex < Snapshots.Num(); ++SnapshotIndex)
		{
			const FMassEntityHandle Entity = Snapshots[SnapshotIndex].Entity;
			if (Entity.Index >= 0 && Entity.Index < SnapshotIndexByEntityIndex.Num())
			{
				SnapshotIndexByEntityIndex[Entity.Index] = SnapshotIndex;
			}
		}
	}

	FECSSpatialHash SpatialHash;
	SpatialHash.Build(Snapshots, 220.0f);

	TArray<float> PendingDamageBySnapshot;
	PendingDamageBySnapshot.Init(0.0f, Snapshots.Num());
	TArray<FMassEntityHandle> LastDamageCauserBySnapshot;
	LastDamageCauserBySnapshot.Init(FMassEntityHandle(), Snapshots.Num());

    AgentQuery.ForEachEntityChunk(Context, [&Snapshots, &SnapshotIndexByEntityIndex, &SpatialHash, &PendingDamageBySnapshot, &LastDamageCauserBySnapshot, DeltaTime](FMassExecutionContext& QueryContext)
	{
		TArrayView<FECSBattleAgentFragment> AgentFragments = QueryContext.GetMutableFragmentView<FECSBattleAgentFragment>();
		TArrayView<FTransformFragment> TransformFragments = QueryContext.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> RadiusFragments = QueryContext.GetFragmentView<FAgentRadiusFragment>();
		TArrayView<FMassDesiredMovementFragment> DesiredMovementFragments = QueryContext.GetMutableFragmentView<FMassDesiredMovementFragment>();
		TArrayView<FMassMoveTargetFragment> MoveTargetFragments = QueryContext.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FMassEntityHandle> Entities = QueryContext.GetEntities();
		UWorld* World = QueryContext.GetWorld();

		for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
		{
			FECSBattleAgentFragment& Agent = AgentFragments[EntityIndex];
			FTransform& Transform = TransformFragments[EntityIndex].GetMutableTransform();
			FMassDesiredMovementFragment& DesiredMovement = DesiredMovementFragments[EntityIndex];
			FMassMoveTargetFragment& MoveTarget = MoveTargetFragments[EntityIndex];
			const FMassEntityHandle Entity = Entities[EntityIndex];
            const FVector SelfLocation = Transform.GetLocation();
			Agent.bTriggerAttackMontage = false;
			DesiredMovement.DesiredVelocity = FVector::ZeroVector;

			if (Agent.bDying || Agent.CurrentHealth <= 0.0f)
			{
				if (!Agent.bDying)
				{
					Agent.bDying = true;
					Agent.LifeTimeRemaining = Agent.DestroyDelay;
					Agent.CurrentTarget = FMassEntityHandle();
				}

				Agent.LifeTimeRemaining -= DeltaTime;
				if (Agent.LifeTimeRemaining <= 0.0f)
				{
                    Agent.bPendingEntityDestroy = true;
				}

                MoveTarget.Center = SelfLocation;
				MoveTarget.Forward = Transform.GetRotation().Vector();
				MoveTarget.DistanceToGoal = 0.0f;
				MoveTarget.DesiredSpeed = FMassInt16Real(0.0f);
				if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}
				continue;
			}

			Agent.TimeUntilNextAttack = FMath::Max(0.0f, Agent.TimeUntilNextAttack - DeltaTime);
			Agent.TimeUntilTargetRefresh -= DeltaTime;
			const FECSAgentSnapshot SelfSnapshot{ Entity, SelfLocation, Agent.Team, true };

			bool bCurrentTargetValid = false;
			if (Agent.CurrentTarget.IsSet())
			{
              const int32 CurrentTargetIndex = FindSnapshotIndex(SnapshotIndexByEntityIndex, Agent.CurrentTarget);
				if (CurrentTargetIndex != INDEX_NONE)
				{
                    const FECSAgentSnapshot& Candidate = Snapshots[CurrentTargetIndex];
					bCurrentTargetValid = Candidate.bAlive && Candidate.Team != Agent.Team;
				}
			}

			if (Agent.TimeUntilTargetRefresh <= 0.0f || !bCurrentTargetValid)
			{
             const int32 NewTargetIndex = FindNearestEnemyIndex(SelfSnapshot, Agent, Snapshots, SpatialHash);
				Agent.CurrentTarget = (NewTargetIndex != INDEX_NONE) ? Snapshots[NewTargetIndex].Entity : FMassEntityHandle();
				Agent.TimeUntilTargetRefresh = Agent.TargetRefreshInterval;
			}

			const int32 TargetSnapshotIndex = FindSnapshotIndex(SnapshotIndexByEntityIndex, Agent.CurrentTarget);
			if (TargetSnapshotIndex == INDEX_NONE)
			{
             MoveTarget.Center = SelfLocation;
				MoveTarget.Forward = Transform.GetRotation().Vector();
				MoveTarget.DistanceToGoal = 0.0f;
				MoveTarget.DesiredSpeed = FMassInt16Real(0.0f);
                if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}
				continue;
			}

          const FECSAgentSnapshot& TargetSnapshot = Snapshots[TargetSnapshotIndex];
			if (!TargetSnapshot.bAlive)
			{
             MoveTarget.Center = SelfLocation;
				MoveTarget.Forward = Transform.GetRotation().Vector();
				MoveTarget.DistanceToGoal = 0.0f;
				MoveTarget.DesiredSpeed = FMassInt16Real(0.0f);
                if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}
				continue;
			}

            const FVector ToTarget = TargetSnapshot.Location - SelfLocation;
            const FVector MoveDirection = ToTarget.GetSafeNormal2D();
            const int32 SelfSnapshotIndex = FindSnapshotIndex(SnapshotIndexByEntityIndex, Entity);
			const float DistanceToTarget = ToTarget.Size2D();
			const float AgentRadius = RadiusFragments[EntityIndex].Radius;
			const float StandoffDistance = FMath::Max(AgentRadius, Agent.AttackRange - AgentRadius);

            // Use persistent slot angle assigned at spawn to keep formation stable
            const float SlotAngleDegrees = Agent.SlotAngleDegrees;
            FVector SlotDirection = MoveDirection.IsNearlyZero() ? Transform.GetRotation().GetForwardVector().GetSafeNormal2D() : MoveDirection;
            SlotDirection = SlotDirection.RotateAngleAxis(SlotAngleDegrees, FVector::UpVector).GetSafeNormal2D();
			const FVector StandoffCenter = TargetSnapshot.Location - (SlotDirection * StandoffDistance);
			const FVector ToStandoff = StandoffCenter - SelfLocation;
            const float DistanceToStandoff = ToStandoff.Size2D();
            // Small hysteresis tolerance to avoid rapid Move/Stand toggles when near goal
            const float GoalTolerance = 30.0f;

            MoveTarget.Center = MoveDirection.IsNearlyZero() ? TargetSnapshot.Location : StandoffCenter;
			MoveTarget.Forward = SlotDirection.IsNearlyZero() ? Transform.GetRotation().Vector() : SlotDirection;
			MoveTarget.DistanceToGoal = DistanceToStandoff;
			MoveTarget.DesiredSpeed = FMassInt16Real(Agent.MoveSpeed);

			if (DistanceToTarget <= Agent.AttackRange)
			{
             if (!ToTarget.IsNearlyZero())
				{
					Transform.SetRotation(ToTarget.ToOrientationQuat());
				}

             MoveTarget.DesiredSpeed = FMassInt16Real(0.0f);
				if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}

				if (Agent.TimeUntilNextAttack <= 0.0f)
				{
                    PendingDamageBySnapshot[TargetSnapshotIndex] += Agent.AttackDamage;
					LastDamageCauserBySnapshot[TargetSnapshotIndex] = Entity;
					Agent.TimeUntilNextAttack = Agent.AttackInterval;
					Agent.bTriggerAttackMontage = true;
				}
				continue;
			}

			if (!MoveDirection.IsNearlyZero())
			{
              FVector DesiredDirection = ToStandoff.GetSafeNormal2D();

				FVector LocalSeparation = FVector::ZeroVector;
				constexpr float FriendlyAvoidanceRadiusScale = 3.0f;
				const float FriendlyAvoidanceRadius = FMath::Max(AgentRadius * FriendlyAvoidanceRadiusScale, 120.0f);
                SpatialHash.ForEachSnapshotInRadius(SelfLocation, FriendlyAvoidanceRadius, Snapshots, [&](const int32 FriendlyIndex)
				{
                   const FECSAgentSnapshot& Friendly = Snapshots[FriendlyIndex];
					if (!Friendly.bAlive || Friendly.Team != Agent.Team || Friendly.Entity == Entity)
					{
                       return;
					}

                   const FVector Offset = SelfLocation - Friendly.Location;
					const float DistSquared = Offset.SizeSquared2D();
                  if (DistSquared <= KINDA_SMALL_NUMBER)
					{
                       return;
					}

					const float Dist = FMath::Sqrt(DistSquared);
					const float Weight = 1.0f - (Dist / FriendlyAvoidanceRadius);
					LocalSeparation += Offset.GetSafeNormal2D() * Weight;
               });

                if (!LocalSeparation.IsNearlyZero())
                {
                    DesiredDirection = (DesiredDirection + LocalSeparation * 1.6f).GetSafeNormal2D();
                }

                DesiredMovement.DesiredVelocity = DesiredDirection * Agent.MoveSpeed;
                const FQuat TargetFacing = DesiredDirection.ToOrientationQuat();
                // Smooth facing changes to avoid 180-degree twitch when path is blocked or vectors flip
                const FQuat CurrentFacing = Transform.GetRotation();
                const float LerpAlpha = FMath::Clamp(DeltaTime * 8.0f, 0.05f, 1.0f);
                const FQuat SmoothedFacing = FQuat::Slerp(CurrentFacing, TargetFacing, LerpAlpha).GetNormalized();
                DesiredMovement.DesiredFacing = SmoothedFacing;
                // Only update the transform rotation when we have meaningful movement
                if (!DesiredMovement.DesiredVelocity.IsNearlyZero())
                {
                    Transform.SetRotation(DesiredMovement.DesiredFacing);
                }

                if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Move)
                {
                    MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
                }
			}
           else if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
            {
                MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
            }
            else if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
            {
                MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
            }
		}
	});

    for (int32 SnapshotIndex = 0; SnapshotIndex < PendingDamageBySnapshot.Num(); ++SnapshotIndex)
	{
       const float QueuedDamage = PendingDamageBySnapshot[SnapshotIndex];
		if (QueuedDamage <= 0.0f)
		{
			continue;
		}

		const FMassEntityHandle TargetEntity = Snapshots[SnapshotIndex].Entity;
		if (!EntityManager.IsEntityValid(TargetEntity))
		{
			continue;
		}

       FECSBattleAgentFragment* TargetAgent = EntityManager.GetFragmentDataPtr<FECSBattleAgentFragment>(TargetEntity);
		if (!TargetAgent || TargetAgent->bDying || TargetAgent->CurrentHealth <= 0.0f)
		{
			continue;
		}

        TargetAgent->CurrentHealth = FMath::Max(0.0f, TargetAgent->CurrentHealth - QueuedDamage);
		if (TargetAgent->CurrentHealth <= 0.0f)
		{
			TargetAgent->bDying = true;
			TargetAgent->LifeTimeRemaining = TargetAgent->DestroyDelay;
			TargetAgent->CurrentTarget = FMassEntityHandle();
			TargetAgent->bPendingEntityDestroy = false;
		}
      else if (LastDamageCauserBySnapshot[SnapshotIndex].IsSet() && LastDamageCauserBySnapshot[SnapshotIndex] != TargetEntity)
		{
           TargetAgent->CurrentTarget = LastDamageCauserBySnapshot[SnapshotIndex];
		}
	}
}
