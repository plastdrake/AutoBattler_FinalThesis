#include "OOPBattleAgent.h"

#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"

AOOPBattleAgent::AOOPBattleAgent()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AOOPBattleAgent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	TimeUntilTargetRefresh = 0.0f;
	TimeUntilNextAttack = 0.0f;

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
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
		AddMovementInput(GetActorForwardVector(), 1.0f);
		return;
	}

	const FVector ToTarget = CurrentTarget->GetActorLocation() - GetActorLocation();
	const float DistanceToTarget = ToTarget.Size();

	if (DistanceToTarget <= AttackRange)
	{
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

	const FVector MoveDirection = ToTarget.GetSafeNormal2D();
	if (!MoveDirection.IsNearlyZero())
	{
		AddMovementInput(MoveDirection, 1.0f);
	}

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AOOPBattleAgent::Die()
{
	CurrentTarget = nullptr;

    GetCharacterMovement()->DisableMovement();

	SetActorEnableCollision(false);

	if (DestroyDelay <= 0.0f)
	{
		Destroy();
		return;
	}

	SetLifeSpan(DestroyDelay);
}
