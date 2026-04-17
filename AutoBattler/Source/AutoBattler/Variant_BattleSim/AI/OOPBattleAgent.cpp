#include "OOPBattleAgent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

TArray<TWeakObjectPtr<AOOPBattleAgent>> AOOPBattleAgent::AgentRegistry;

AOOPBattleAgent::AOOPBattleAgent()
{
	PrimaryActorTick.bCanEverTick = true;
  AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeed;
		Movement->bOrientRotationToMovement = true;
		Movement->bRequestedMoveUseAcceleration = true;
		Movement->bUseRVOAvoidance = true;
	}

	if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
	{
		SkeletalMeshComponent->SetGenerateOverlapEvents(false);
		SkeletalMeshComponent->bEnableUpdateRateOptimizations = true;
		SkeletalMeshComponent->bComponentUseFixedSkelBounds = true;
	}
}

void AOOPBattleAgent::BeginPlay()
{
	Super::BeginPlay();

    AgentRegistry.Add(this);

	if (!Controller)
	{
		SpawnDefaultController();
	}

	CurrentHealth = MaxHealth;
	TimeUntilTargetRefresh = 0.0f;
	TimeUntilNextAttack = 0.0f;

   if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeed;
	}
}

void AOOPBattleAgent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AgentRegistry.Remove(this);
	Super::EndPlay(EndPlayReason);
}

int32 AOOPBattleAgent::GetAliveTeamCount(const EBattleAgentTeam InTeam)
{
	int32 TeamCount = 0;
	for (const TWeakObjectPtr<AOOPBattleAgent>& WeakAgent : AgentRegistry)
	{
		const AOOPBattleAgent* Agent = WeakAgent.Get();
		if (!IsValid(Agent) || !Agent->IsAlive() || Agent->GetTeam() != InTeam)
		{
			continue;
		}

		++TeamCount;
	}

	return TeamCount;
}

void AOOPBattleAgent::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsAlive())
	{
		return;
	}

	TimeUntilTargetRefresh -= DeltaSeconds;
	if (TimeUntilTargetRefresh <= 0.0f)
	{
		RefreshTarget();
		TimeUntilTargetRefresh = TargetRefreshInterval;
	}

	TimeUntilNextAttack = FMath::Max(0.0f, TimeUntilNextAttack - DeltaSeconds);
   MoveOrAttackTarget();
}

void AOOPBattleAgent::SetTeam(EBattleAgentTeam NewTeam)
{
	Team = NewTeam;
}

void AOOPBattleAgent::ReceiveDamage(float DamageAmount, AOOPBattleAgent* DamageCauser)
{
	if (!IsAlive() || DamageAmount <= 0.0f)
	{
		return;
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
	else if (DamageCauser && DamageCauser != this)
	{
		CurrentTarget = DamageCauser;
	}
}

void AOOPBattleAgent::RefreshTarget()
{
    if (AgentRegistry.Num() == 0)
	{
		CurrentTarget = nullptr;
		return;
	}

	const FVector MyLocation = GetActorLocation();
	const float SearchRadiusSquared = FMath::Square(TargetSearchRadius);

	AOOPBattleAgent* BestTarget = nullptr;
	float BestDistanceSquared = MAX_flt;

  for (const TWeakObjectPtr<AOOPBattleAgent>& WeakAgent : AgentRegistry)
	{
       AOOPBattleAgent* Candidate = WeakAgent.Get();
		if (!IsValid(Candidate) || Candidate == this || !Candidate->IsAlive() || Candidate->GetTeam() == Team)
		{
			continue;
		}

      const float DistanceSquared = FVector::DistSquared2D(MyLocation, Candidate->GetActorLocation());
		if (DistanceSquared > SearchRadiusSquared)
		{
			continue;
		}

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Candidate;
		}
	}

	CurrentTarget = BestTarget;
}

void AOOPBattleAgent::MoveOrAttackTarget()
{
   UCharacterMovementComponent* Movement = GetCharacterMovement();

	if (!IsValid(CurrentTarget) || !CurrentTarget->IsAlive())
	{
		if (Movement && !Movement->Velocity.IsNearlyZero())
		{
			Movement->StopMovementImmediately();
		}
       return;
	}

	const FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
 const float DistanceToTargetSquared = ToTarget.SizeSquared2D();
	const float AttackRangeSquared = FMath::Square(AttackRange);

    if (DistanceToTargetSquared <= AttackRangeSquared)
	{
     if (Movement)
		{
			Movement->StopMovementImmediately();
		}

     if (!ToTarget.IsNearlyZero())
		{
            const FRotator DesiredRotation(0.0f, ToTarget.Rotation().Yaw, 0.0f);
			SetActorRotation(DesiredRotation);
		}

		if (TimeUntilNextAttack <= 0.0f)
		{
           if (AttackMontage)
			{
				PlayAnimMontage(AttackMontage);
			}

			CurrentTarget->ReceiveDamage(AttackDamage, this);
			TimeUntilNextAttack = AttackInterval;
		}
		return;
	}

 FVector MoveDirection = ToTarget.GetSafeNormal2D();

  if (Movement && Movement->bUseRVOAvoidance == false)
	{
      const FVector MyLocation = GetActorLocation();
		constexpr float FriendlyAvoidanceRadius = 180.0f;
		const float FriendlyAvoidanceRadiusSquared = FMath::Square(FriendlyAvoidanceRadius);
		FVector AvoidanceDirection = FVector::ZeroVector;

       for (const TWeakObjectPtr<AOOPBattleAgent>& WeakAgent : AgentRegistry)
		{
			AOOPBattleAgent* Friendly = WeakAgent.Get();
			if (!IsValid(Friendly) || Friendly == this || !Friendly->IsAlive() || Friendly->GetTeam() != Team)
			{
				continue;
			}

			const FVector Offset = MyLocation - Friendly->GetActorLocation();
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
	}

	if (!MoveDirection.IsNearlyZero())
	{
      AddMovementInput(MoveDirection, 1.0f, true);
	}
}

void AOOPBattleAgent::Die()
{
	CurrentTarget = nullptr;

  if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}

	SetActorEnableCollision(false);

	if (DestroyDelay <= 0.0f)
	{
		Destroy();
		return;
	}

	SetLifeSpan(DestroyDelay);
}
