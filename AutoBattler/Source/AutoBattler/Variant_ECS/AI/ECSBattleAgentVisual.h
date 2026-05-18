#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ECSBattleAgentFragments.h"
#include "ECSBattleAgentVisual.generated.h"

class UAnimMontage;

/** @brief Visual character used to represent an ECS battle agent. */
UCLASS()
class AUTOBATTLER_API AECSBattleAgentVisual : public ACharacter
{
	GENERATED_BODY()

public:
	/** @brief Default constructor. */
	AECSBattleAgentVisual();
	/**
	 * @brief Returns the cached velocity for the visual.
	 * @return Cached velocity.
	 */
	virtual FVector GetVelocity() const override;

	/** @brief Attack montage to play when this visual performs an attack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	/**
	 * @brief Blueprint event fired when the team assignment changes.
	 * @param Team Newly assigned team.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Battle Agent|Visual")
	void OnTeamAssigned(EECSBattleAgentTeam Team);

	/**
	 * @brief Assigns this visual to a team.
	 * @param Team Team to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Battle Agent|Visual")
	void SetAssignedTeam(EECSBattleAgentTeam Team);

	/**
	 * @brief Returns the assigned team for this visual.
	 * @return Assigned team.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Agent|Visual")
	EECSBattleAgentTeam GetAssignedTeam() const { return AssignedTeam; }

	/**
	 * @brief Returns true if a team has been assigned.
	 * @return True when a team assignment is valid.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Agent|Visual")
	bool HasAssignedTeam() const { return bHasAssignedTeam; }

	/** @brief Plays the attack montage if one is assigned. */
	UFUNCTION(BlueprintCallable, Category = "Battle Agent|Combat|Animation")
	void PlayAttackMontage();

	/**
	 * @brief Syncs this visual with the Mass transform state.
	 * @param WorldTransform Source transform from Mass.
	 * @param DeltaTimeSeconds Time step used to compute velocity.
	 */
	void SyncFromMassTransform(const FTransform& WorldTransform, float DeltaTimeSeconds);
	/**
	 * @brief Returns the cached velocity.
	 * @return Cached velocity reference.
	 */
	const FVector& GetCachedVelocity() const { return CachedVelocity; }

private:
	/** @brief Cached velocity computed from transform deltas. */
	FVector CachedVelocity = FVector::ZeroVector;
	/** @brief Assigned team for this visual. */
	EECSBattleAgentTeam AssignedTeam = EECSBattleAgentTeam::Red;
	/** @brief True once a team assignment has been made. */
	bool bHasAssignedTeam = false;
};
