#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "ECSBattleAgentFragments.generated.h"

UENUM(BlueprintType)
enum class EECSBattleAgentTeam : uint8
{
	Red UMETA(DisplayName = "Red Team"),
	Blue UMETA(DisplayName = "Blue Team")
};

USTRUCT()
struct AUTOBATTLER_API FECSBattleAgentFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Battle Agent")
	EECSBattleAgentTeam Team = EECSBattleAgentTeam::Red;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Stats")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Stats")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Combat")
	float AttackRange = 140.0f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Combat")
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Combat")
	float AttackInterval = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Targeting")
	float TargetSearchRadius = 100000.0f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Targeting")
	float TargetRefreshInterval = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Death")
	float DestroyDelay = 2.0f;

	UPROPERTY(Transient)
	FMassEntityHandle CurrentTarget = FMassEntityHandle();

	UPROPERTY(Transient)
	float TimeUntilNextAttack = 0.0f;

	UPROPERTY(Transient)
	float TimeUntilTargetRefresh = 0.0f;

	UPROPERTY(Transient)
	float LifeTimeRemaining = 0.0f;

	UPROPERTY(Transient)
	bool bDying = false;

	UPROPERTY(Transient)
	bool bPendingEntityDestroy = false;

	UPROPERTY(Transient)
	bool bTriggerAttackMontage = false;

    /** Persistent slot angle (degrees) used for stable formation positioning. Set by the spawner. */
    UPROPERTY(Transient)
    float SlotAngleDegrees = 0.0f;
};

USTRUCT()
struct AUTOBATTLER_API FECSBattleAgentRepresentationFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Battle Agent|Visual")
 uint64 VisualCharacterClassAddress = 0;
};
