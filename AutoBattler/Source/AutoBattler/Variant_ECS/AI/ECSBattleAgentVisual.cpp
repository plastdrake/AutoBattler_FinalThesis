#include "ECSBattleAgentVisual.h"

#include "GameFramework/CharacterMovementComponent.h"

AECSBattleAgentVisual::AECSBattleAgentVisual()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::Disabled;
	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}
}

void AECSBattleAgentVisual::PlayAttackMontage()
{
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}
