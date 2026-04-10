#include "OOPBattleAgent.h"

#include "AIController.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

AOOPBattleAgent::AOOPBattleAgent()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

   GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
	GetCharacterMovement()->bUseRVOAvoidance = true;
}

void AOOPBattleAgent::BeginPlay()
{
	Super::BeginPlay();

	if (!Controller)
	{
		SpawnDefaultController();
	}

	CurrentHealth = MaxHealth;
	TimeUntilTargetRefresh = 0.0f;
	TimeUntilNextAttack = 0.0f;

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AOOPBattleAgent::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Controller)
	{
		SpawnDefaultController();
	}

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
	if (!GetWorld())
	{
		CurrentTarget = nullptr;
		return;
	}

	const FVector MyLocation = GetActorLocation();
	const float SearchRadiusSquared = FMath::Square(TargetSearchRadius);

	AOOPBattleAgent* BestTarget = nullptr;
	float BestDistanceSquared = MAX_flt;

	for (TActorIterator<AOOPBattleAgent> It(GetWorld()); It; ++It)
	{
		AOOPBattleAgent* Candidate = *It;
		if (!IsValid(Candidate) || Candidate == this || !Candidate->IsAlive() || Candidate->GetTeam() == Team)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(MyLocation, Candidate->GetActorLocation());
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
	if (!IsValid(CurrentTarget) || !CurrentTarget->IsAlive())
	{
        RefreshTarget();
		AAIController* AIController = Cast<AAIController>(GetController());

		if (!IsValid(CurrentTarget) || !CurrentTarget->IsAlive())
		{
          if (AIController)
			{
				AIController->StopMovement();
			}
			GetCharacterMovement()->StopMovementImmediately();
			return;
		}
	}

    AAIController* AIController = Cast<AAIController>(GetController());
	const FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	const float DistanceToTarget = ToTarget.Size();

	if (DistanceToTarget <= AttackRange)
	{
     if (AIController)
		{
			AIController->StopMovement();
		}

		if (Controller)
		{
			Controller->SetControlRotation(ToTarget.Rotation());
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

	const FVector MyLocation = GetActorLocation();
	constexpr float FriendlyAvoidanceRadius = 180.0f;
	const float FriendlyAvoidanceRadiusSquared = FMath::Square(FriendlyAvoidanceRadius);
	FVector AvoidanceDirection = FVector::ZeroVector;

	for (TActorIterator<AOOPBattleAgent> It(GetWorld()); It; ++It)
	{
		AOOPBattleAgent* Friendly = *It;
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

	if (!MoveDirection.IsNearlyZero())
	{
		AddMovementInput(MoveDirection, 1.0f);
	}

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AOOPBattleAgent::Die()
{
	CurrentTarget = nullptr;
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

    GetCharacterMovement()->DisableMovement();

	SetActorEnableCollision(false);

	if (DestroyDelay <= 0.0f)
	{
		Destroy();
		return;
	}

	SetLifeSpan(DestroyDelay);
}
