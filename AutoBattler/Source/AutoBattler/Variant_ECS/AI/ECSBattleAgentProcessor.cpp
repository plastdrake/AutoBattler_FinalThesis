#include "ECSBattleAgentProcessor.h"

#include "ECSBattleAgentFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"

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
		float BestDistanceSquared = MAX_flt;
		int32 BestIndex = INDEX_NONE;

		for (int32 CandidateIndex = 0; CandidateIndex < Snapshots.Num(); ++CandidateIndex)
		{
			const FECSAgentSnapshot& Candidate = Snapshots[CandidateIndex];
			if (!Candidate.bAlive || Candidate.Entity == SourceSnapshot.Entity || Candidate.Team == SourceAgent.Team)
			{
				continue;
			}

			const float DistanceSquared = FVector::DistSquared(SourceSnapshot.Location, Candidate.Location);
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
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::All);
	bAutoRegisterWithProcessingPhases = true;
}

void UECSBattleAgentProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AgentQuery.AddRequirement<FECSBattleAgentFragment>(EMassFragmentAccess::ReadWrite);
	AgentQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
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
		const TConstArrayView<FMassEntityHandle> Entities = QueryContext.GetEntities();

		for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
		{
			FECSBattleAgentFragment& Agent = AgentFragments[EntityIndex];
			FTransform& Transform = TransformFragments[EntityIndex].GetMutableTransform();
			const FMassEntityHandle Entity = Entities[EntityIndex];
			Agent.bTriggerAttackMontage = false;

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
				continue;
			}

			const FECSAgentSnapshot& TargetSnapshot = Snapshots[*TargetSnapshotIndex];
			if (!TargetSnapshot.bAlive)
			{
				continue;
			}

			const FVector ToTarget = TargetSnapshot.Location - SelfSnapshot.Location;
			const float DistanceToTarget = ToTarget.Size();

			if (DistanceToTarget <= Agent.AttackRange)
			{
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

			FVector MoveDirection = ToTarget.GetSafeNormal2D();
			constexpr float FriendlyAvoidanceRadius = 180.0f;
			const float FriendlyAvoidanceRadiusSquared = FMath::Square(FriendlyAvoidanceRadius);
			FVector AvoidanceDirection = FVector::ZeroVector;

			for (const FECSAgentSnapshot& Friendly : Snapshots)
			{
				if (!Friendly.bAlive || Friendly.Entity == Entity || Friendly.Team != Agent.Team)
				{
					continue;
				}

				const FVector Offset = SelfSnapshot.Location - Friendly.Location;
				const float DistanceSquared = Offset.SizeSquared2D();
				if (DistanceSquared <= KINDA_SMALL_NUMBER || DistanceSquared > FriendlyAvoidanceRadiusSquared)
				{
					continue;
				}

				const float Distance = FMath::Sqrt(DistanceSquared);
				const float Weight = 1.0f - (Distance / FriendlyAvoidanceRadius);
				AvoidanceDirection += Offset.GetSafeNormal2D() * Weight;
			}

			if (!AvoidanceDirection.IsNearlyZero())
			{
				MoveDirection = (MoveDirection + AvoidanceDirection * 1.5f).GetSafeNormal2D();
			}

			if (!MoveDirection.IsNearlyZero())
			{
				Transform.SetLocation(Transform.GetLocation() + (MoveDirection * Agent.MoveSpeed * DeltaTime));
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
