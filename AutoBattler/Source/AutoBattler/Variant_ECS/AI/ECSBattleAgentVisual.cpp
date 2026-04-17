#include "ECSBattleAgentVisual.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AECSBattleAgentVisual::AECSBattleAgentVisual()
{
	PrimaryActorTick.bCanEverTick = false;
    AutoPossessAI = EAutoPossessAI::Disabled;
	bUseControllerRotationYaw = false;
	SetActorEnableCollision(false);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

   if (USkeletalMeshComponent* SkeletalMeshComponent = GetMesh())
	{
      SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkeletalMeshComponent->SetGenerateOverlapEvents(false);
		SkeletalMeshComponent->KinematicBonesUpdateType = EKinematicBonesUpdateToPhysics::SkipAllBones;
		SkeletalMeshComponent->bEnableUpdateRateOptimizations = true;
		SkeletalMeshComponent->bComponentUseFixedSkelBounds = true;
	}

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
     Movement->StopMovementImmediately();
		Movement->SetMovementMode(EMovementMode::MOVE_None);
		Movement->Deactivate();
		Movement->SetComponentTickEnabled(false);
	}
}

FVector AECSBattleAgentVisual::GetVelocity() const
{
	return CachedVelocity;
}

void AECSBattleAgentVisual::SetAssignedTeam(const EECSBattleAgentTeam Team)
{
	AssignedTeam = Team;
	bHasAssignedTeam = true;
	OnTeamAssigned(Team);
}

void AECSBattleAgentVisual::SyncFromMassTransform(const FTransform& WorldTransform, float DeltaTimeSeconds)
{
    const FVector PreviousLocation = GetActorLocation();
	SetActorLocationAndRotation(WorldTransform.GetLocation(), WorldTransform.GetRotation(), false, nullptr, ETeleportType::TeleportPhysics);

	const float SafeDelta = FMath::Max(DeltaTimeSeconds, KINDA_SMALL_NUMBER);
	CachedVelocity = (GetActorLocation() - PreviousLocation) / SafeDelta;
}

void AECSBattleAgentVisual::PlayAttackMontage()
{
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}
