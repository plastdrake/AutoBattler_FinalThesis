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

	struct FECSQueuedDamage
	{
		float Damage = 0.0f;
		FMassEntityHandle LastCauser;
	};

	int32 FindNearestEnemyIndex(const FECSAgentSnapshot& SourceSnapshot, const FECSBattleAgentFragment& SourceAgent, const TArray<FECSAgentSnapshot>& Snapshots)
	{
		const float SearchRadiusSquared = FMath::Square(SourceAgent.TargetSearchRadius);
        float BestScore = MAX_flt;
		int32 BestIndex = INDEX_NONE;

		for (int32 CandidateIndex = 0; CandidateIndex < Snapshots.Num(); ++CandidateIndex)
		{
			const FECSAgentSnapshot& Candidate = Snapshots[CandidateIndex];
			if (!Candidate.bAlive || Candidate.Entity == SourceSnapshot.Entity || Candidate.Team == SourceAgent.Team)
			{
				continue;
			}

			const float DistanceSquared = FVector::DistSquared(SourceSnapshot.Location, Candidate.Location);
            if (DistanceSquared > SearchRadiusSquared)
			{
				continue;
			}

			int32 FriendlyCongestion = 0;
			const float CongestionRadiusSquared = FMath::Square(SourceAgent.AttackRange * 2.0f);
			for (const FECSAgentSnapshot& Other : Snapshots)
			{
				if (!Other.bAlive || Other.Team != SourceAgent.Team || Other.Entity == SourceSnapshot.Entity)
				{
					continue;
				}

				if (FVector::DistSquared(Other.Location, Candidate.Location) <= CongestionRadiusSquared)
				{
					++FriendlyCongestion;
				}
			}

			const float Score = DistanceSquared + (FriendlyCongestion * FMath::Square(SourceAgent.AttackRange * 1.5f));
			if (Score >= BestScore)
			{
				continue;
			}

          BestScore = Score;
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
	TMap<FMassEntityHandle, int32> SnapshotIndexByEntity;

	AgentQuery.ForEachEntityChunk(Context, [&Snapshots, &SnapshotIndexByEntity](FMassExecutionContext& QueryContext)
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
			SnapshotIndexByEntity.Add(Entity, Snapshots.Num() - 1);
		}
	});

	TMap<FMassEntityHandle, FECSQueuedDamage> PendingDamage;

	AgentQuery.ForEachEntityChunk(Context, [&Snapshots, &SnapshotIndexByEntity, &PendingDamage, DeltaTime](FMassExecutionContext& QueryContext)
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

				MoveTarget.Center = Transform.GetLocation();
				MoveTarget.Forward = Transform.GetRotation().Vector();
				MoveTarget.DistanceToGoal = 0.0f;
             MoveTarget.DesiredSpeed = FMassInt16Real(Agent.MoveSpeed);
               if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}
				continue;
			}

			Agent.TimeUntilNextAttack = FMath::Max(0.0f, Agent.TimeUntilNextAttack - DeltaTime);
			Agent.TimeUntilTargetRefresh -= DeltaTime;

			const int32* SelfSnapshotIndex = SnapshotIndexByEntity.Find(Entity);
			if (!SelfSnapshotIndex)
			{
				continue;
			}
			const FECSAgentSnapshot& SelfSnapshot = Snapshots[*SelfSnapshotIndex];

			bool bCurrentTargetValid = false;
			if (Agent.CurrentTarget.IsSet())
			{
				if (const int32* CurrentTargetIndex = SnapshotIndexByEntity.Find(Agent.CurrentTarget))
				{
					const FECSAgentSnapshot& Candidate = Snapshots[*CurrentTargetIndex];
					bCurrentTargetValid = Candidate.bAlive && Candidate.Team != Agent.Team;
				}
			}

			if (Agent.TimeUntilTargetRefresh <= 0.0f || !bCurrentTargetValid)
			{
				const int32 NewTargetIndex = FindNearestEnemyIndex(SelfSnapshot, Agent, Snapshots);
				Agent.CurrentTarget = (NewTargetIndex != INDEX_NONE) ? Snapshots[NewTargetIndex].Entity : FMassEntityHandle();
				Agent.TimeUntilTargetRefresh = Agent.TargetRefreshInterval;
			}

			const int32* TargetSnapshotIndex = SnapshotIndexByEntity.Find(Agent.CurrentTarget);
			if (!TargetSnapshotIndex)
			{
               MoveTarget.Center = Transform.GetLocation();
				MoveTarget.Forward = Transform.GetRotation().Vector();
				MoveTarget.DistanceToGoal = 0.0f;
				MoveTarget.DesiredSpeed = FMassInt16Real(0.0f);
               if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}
				continue;
			}

			const FECSAgentSnapshot& TargetSnapshot = Snapshots[*TargetSnapshotIndex];
			if (!TargetSnapshot.bAlive)
			{
               MoveTarget.Center = Transform.GetLocation();
				MoveTarget.Forward = Transform.GetRotation().Vector();
				MoveTarget.DistanceToGoal = 0.0f;
				MoveTarget.DesiredSpeed = FMassInt16Real(0.0f);
               if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}
				continue;
			}

           const FVector ToTarget = TargetSnapshot.Location - SelfSnapshot.Location;
			const FVector MoveDirection = ToTarget.GetSafeNormal2D();
			const float DistanceToTarget = ToTarget.Size2D();
			const float AgentRadius = RadiusFragments[EntityIndex].Radius;
           const float StandoffDistance = FMath::Max(AgentRadius, Agent.AttackRange - AgentRadius);

			const float SlotHash = FMath::Frac(static_cast<float>(Entity.Index) * 0.61803398875f);
			const float SlotAngleDegrees = (SlotHash * 2.0f - 1.0f) * 70.0f;
			FVector SlotDirection = MoveDirection.IsNearlyZero() ? Transform.GetRotation().GetForwardVector().GetSafeNormal2D() : MoveDirection;
			SlotDirection = SlotDirection.RotateAngleAxis(SlotAngleDegrees, FVector::UpVector).GetSafeNormal2D();
			const FVector StandoffCenter = TargetSnapshot.Location - (SlotDirection * StandoffDistance);
			const FVector ToStandoff = StandoffCenter - SelfSnapshot.Location;
			float DistanceToStandoff = ToStandoff.Size2D();

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

             MoveTarget.DesiredSpeed = FMassInt16Real(Agent.MoveSpeed);
				if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
				}

				if (Agent.TimeUntilNextAttack <= 0.0f)
				{
					FECSQueuedDamage& QueuedDamage = PendingDamage.FindOrAdd(TargetSnapshot.Entity);
					QueuedDamage.Damage += Agent.AttackDamage;
					QueuedDamage.LastCauser = Entity;
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
				const float FriendlyAvoidanceRadiusSquared = FMath::Square(FriendlyAvoidanceRadius);
				for (const FECSAgentSnapshot& Friendly : Snapshots)
				{
					if (!Friendly.bAlive || Friendly.Team != Agent.Team || Friendly.Entity == Entity)
					{
						continue;
					}

					const FVector Offset = SelfSnapshot.Location - Friendly.Location;
					const float DistSquared = Offset.SizeSquared2D();
					if (DistSquared <= KINDA_SMALL_NUMBER || DistSquared > FriendlyAvoidanceRadiusSquared)
					{
						continue;
					}

					const float Dist = FMath::Sqrt(DistSquared);
					const float Weight = 1.0f - (Dist / FriendlyAvoidanceRadius);
					LocalSeparation += Offset.GetSafeNormal2D() * Weight;
				}

				if (!LocalSeparation.IsNearlyZero())
				{
					DesiredDirection = (DesiredDirection + LocalSeparation * 1.6f).GetSafeNormal2D();
				}

				DesiredMovement.DesiredVelocity = DesiredDirection * Agent.MoveSpeed;
				DesiredMovement.DesiredFacing = DesiredDirection.ToOrientationQuat();
				Transform.SetRotation(DesiredMovement.DesiredFacing);
               if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Move)
				{
					MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
				}
			}
           else if (World && MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
			{
				MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
			}
		}
	});

	for (TPair<FMassEntityHandle, FECSQueuedDamage>& PendingHit : PendingDamage)
	{
		if (!EntityManager.IsEntityValid(PendingHit.Key))
		{
			continue;
		}

		FECSBattleAgentFragment* TargetAgent = EntityManager.GetFragmentDataPtr<FECSBattleAgentFragment>(PendingHit.Key);
		if (!TargetAgent || TargetAgent->bDying || TargetAgent->CurrentHealth <= 0.0f)
		{
			continue;
		}

		TargetAgent->CurrentHealth = FMath::Max(0.0f, TargetAgent->CurrentHealth - PendingHit.Value.Damage);
		if (TargetAgent->CurrentHealth <= 0.0f)
		{
			TargetAgent->bDying = true;
			TargetAgent->LifeTimeRemaining = TargetAgent->DestroyDelay;
			TargetAgent->CurrentTarget = FMassEntityHandle();
           TargetAgent->bPendingEntityDestroy = false;
		}
		else if (PendingHit.Value.LastCauser.IsSet() && PendingHit.Value.LastCauser != PendingHit.Key)
		{
			TargetAgent->CurrentTarget = PendingHit.Value.LastCauser;
		}
	}
}
