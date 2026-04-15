#include "ECSBattleAgentVisual.h"

#include "GameFramework/CharacterMovementComponent.h"

AECSBattleAgentVisual::AECSBattleAgentVisual()
{
	PrimaryActorTick.bCanEverTick = false;
   AutoPossessAI = EAutoPossessAI::Disabled;
	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
        Movement->SetMovementMode(EMovementMode::MOVE_Walking);
		Movement->bOrientRotationToMovement = true;
		Movement->bRequestedMoveUseAcceleration = true;
		Movement->GravityScale = 0.0f;
	}
}

FVector AECSBattleAgentVisual::GetVelocity() const
{
	return CachedVelocity;
}

void AECSBattleAgentVisual::SyncFromMassTransform(const FTransform& WorldTransform, float DeltaTimeSeconds)
{
    const FVector PreviousLocation = GetActorLocation();

	FHitResult SweepHit;
	SetActorLocation(WorldTransform.GetLocation(), true, &SweepHit, ETeleportType::None);
	SetActorRotation(WorldTransform.GetRotation(), ETeleportType::None);

	const float SafeDelta = FMath::Max(DeltaTimeSeconds, KINDA_SMALL_NUMBER);
	CachedVelocity = (GetActorLocation() - PreviousLocation) / SafeDelta;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (Movement->MovementMode == EMovementMode::MOVE_None)
		{
			Movement->SetMovementMode(EMovementMode::MOVE_Walking);
		}

		Movement->Velocity = CachedVelocity;
	}
}

void AECSBattleAgentVisual::PlayAttackMontage()
{
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}
