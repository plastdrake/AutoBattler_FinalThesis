#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OOPBattleAgent.generated.h"

/** @brief Team affiliation used by OOP battle agents. */
UENUM(BlueprintType)
enum class EBattleAgentTeam : uint8
{
	Red UMETA(DisplayName = "Red Team"),
	Blue UMETA(DisplayName = "Blue Team")
};

class UAnimMontage;

/** @brief OOP battle agent character with combat and targeting logic. */
UCLASS()
class AUTOBATTLER_API AOOPBattleAgent : public ACharacter
{
	GENERATED_BODY()

public:
	/** @brief Default constructor. */
	AOOPBattleAgent();

	/**
	 * @brief Per-frame update.
	 * @param DeltaSeconds Time since the last tick.
	 */
	virtual void Tick(float DeltaSeconds) override;
	/**
	 * @brief Cleanup when the actor is ending play.
	 * @param EndPlayReason Reason for ending play.
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * @brief Assigns a new team to the agent.
	 * @param NewTeam Team to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Battle Agent")
	void SetTeam(EBattleAgentTeam NewTeam);

	/**
	 * @brief Returns true if the agent is alive.
	 * @return True when current health is above zero.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Agent")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	/**
	 * @brief Returns current health.
	 * @return Current health value.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Agent")
	float GetCurrentHealth() const { return CurrentHealth; }

	/**
	 * @brief Returns the assigned team.
	 * @return Team assignment.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Agent")
	EBattleAgentTeam GetTeam() const { return Team; }

	/**
	 * @brief Returns the number of alive agents on the specified team.
	 * @param InTeam Team to query.
	 * @return Alive agent count.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Agent")
	static int32 GetAliveTeamCount(EBattleAgentTeam InTeam);

	/**
	 * @brief Applies damage to this agent.
	 * @param DamageAmount Amount of damage to apply.
	 * @param DamageCauser Agent that caused the damage.
	 */
	void ReceiveDamage(float DamageAmount, AOOPBattleAgent* DamageCauser);

protected:
	/** @brief Actor initialization. */
	virtual void BeginPlay() override;

	/** @brief Current team assignment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent")
	EBattleAgentTeam Team = EBattleAgentTeam::Red;

	/** @brief Maximum health value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Stats", meta = (ClampMin = 1.0))
	float MaxHealth = 100.0f;

	/** @brief Current health value. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle Agent|Stats", meta = (ClampMin = 0.0))
	float CurrentHealth = 100.0f;

	/** @brief Movement speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Stats", meta = (ClampMin = 0.0))
	float MoveSpeed = 300.0f;

	/** @brief Attack range in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0, Units = "cm"))
	float AttackRange = 140.0f;

	/** @brief Damage dealt per attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0))
	float AttackDamage = 10.0f;

	/** @brief Time between attacks in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat", meta = (ClampMin = 0.05, Units = "s"))
	float AttackInterval = 0.6f;

	/** @brief Attack montage to play when attacking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** @brief Target search radius in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Targeting", meta = (ClampMin = 100.0, Units = "cm"))
	float TargetSearchRadius = 100000.0f;

	/** @brief Time between target refresh checks in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Targeting", meta = (ClampMin = 0.05, Units = "s"))
	float TargetRefreshInterval = 0.25f;

	/** @brief Delay before the agent is removed after death. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Death", meta = (ClampMin = 0.0, Units = "s"))
	float DestroyDelay = 2.0f;

private:
	/** @brief Current target agent. */
	UPROPERTY(Transient)
	TObjectPtr<AOOPBattleAgent> CurrentTarget;

	/** @brief Time remaining before the next attack. */
	float TimeUntilNextAttack = 0.0f;
	/** @brief Time remaining before a target refresh. */
	float TimeUntilTargetRefresh = 0.0f;

	/** @brief Registry of agents used to compute alive counts. */
	static TArray<TWeakObjectPtr<AOOPBattleAgent>> AgentRegistry;

	/** @brief Picks or refreshes the current target. */
	void RefreshTarget();
	/** @brief Moves toward or attacks the current target. */
	void MoveOrAttackTarget();
	/** @brief Handles agent death flow. */
	void Die();
};
