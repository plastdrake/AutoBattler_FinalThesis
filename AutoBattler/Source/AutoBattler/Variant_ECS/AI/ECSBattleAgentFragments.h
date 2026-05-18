#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "ECSBattleAgentFragments.generated.h"

/** @brief Team affiliation used by ECS battle agents. */
UENUM(BlueprintType)
enum class EECSBattleAgentTeam : uint8
{
	Red UMETA(DisplayName = "Red Team"),
	Blue UMETA(DisplayName = "Blue Team")
};

/** @brief Mass fragment that stores combat, targeting, and lifetime state for an ECS battle agent. */
USTRUCT()
struct AUTOBATTLER_API FECSBattleAgentFragment : public FMassFragment
{
	GENERATED_BODY()

	/** @brief Current team assignment for this agent. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent")
	EECSBattleAgentTeam Team = EECSBattleAgentTeam::Red;

	/** @brief Maximum health the agent can have. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Stats")
	float MaxHealth = 100.0f;

	/** @brief Current health remaining for the agent. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Stats")
	float CurrentHealth = 100.0f;

	/** @brief Movement speed used by Mass movement systems. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Stats")
	float MoveSpeed = 300.0f;

	/** @brief Attack range in world units. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Combat")
	float AttackRange = 140.0f;

	/** @brief Damage dealt per attack. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Combat")
	float AttackDamage = 10.0f;

	/** @brief Time between attacks in seconds. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Combat")
	float AttackInterval = 0.6f;

	/** @brief Radius used when searching for enemy targets. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Targeting")
	float TargetSearchRadius = 100000.0f;

	/** @brief Time between target refresh attempts in seconds. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Targeting")
	float TargetRefreshInterval = 0.25f;

	/** @brief Delay before the entity is destroyed after death. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Death")
	float DestroyDelay = 2.0f;

	/** @brief Entity currently targeted by this agent. */
	UPROPERTY(Transient)
	FMassEntityHandle CurrentTarget = FMassEntityHandle();

	/** @brief Time remaining before the next attack can occur. */
	UPROPERTY(Transient)
	float TimeUntilNextAttack = 0.0f;

	/** @brief Time remaining before the target is refreshed. */
	UPROPERTY(Transient)
	float TimeUntilTargetRefresh = 0.0f;

	/** @brief Remaining lifetime while the agent is dying. */
	UPROPERTY(Transient)
	float LifeTimeRemaining = 0.0f;

	/** @brief True when the agent is in its death state. */
	UPROPERTY(Transient)
	bool bDying = false;

	/** @brief True when the entity should be removed by the system. */
	UPROPERTY(Transient)
	bool bPendingEntityDestroy = false;

	/** @brief True when an attack montage should be triggered. */
	UPROPERTY(Transient)
	bool bTriggerAttackMontage = false;

	/** @brief Persistent slot angle (degrees) used for stable formation positioning. Set by the spawner. */
    UPROPERTY(Transient)
    float SlotAngleDegrees = 0.0f;
};

/** @brief Mass fragment that stores the visual actor reference for an ECS battle agent. */
USTRUCT()
struct AUTOBATTLER_API FECSBattleAgentRepresentationFragment : public FMassFragment
{
	GENERATED_BODY()

	/** @brief Soft address of the visual character class used to spawn the agent representation. */
	UPROPERTY(EditAnywhere, Category = "Battle Agent|Visual")
	uint64 VisualCharacterClassAddress = 0;
};
