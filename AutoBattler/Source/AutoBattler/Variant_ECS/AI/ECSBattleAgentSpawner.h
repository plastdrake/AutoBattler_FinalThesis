#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "ECSBattleAgentFragments.h"
#include "ECSBattleAgentSpawner.generated.h"

struct FMassArchetypeHandle;
struct FMassEntityManager;
class UArrowComponent;

/** @brief Spawns ECS battle agents and initializes their Mass fragments. */
UCLASS()
class AUTOBATTLER_API AECSBattleAgentSpawner : public AActor
{
	GENERATED_BODY()

public:
	/** @brief Default constructor. */
	AECSBattleAgentSpawner();

	/** @brief Spawns agents based on the current spawner settings. */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnAgents();

protected:
	/** @brief Handles spawn-on-begin-play behavior. */
	virtual void BeginPlay() override;

	/** @brief Direction indicator used to place agent rows. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> SpawnDirection;

	/** @brief Number of agents to spawn for this team. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0, ClampMax = 500))
	int32 SpawnCount = 10;

	/** @brief Spacing between agents in a row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0.0, Units = "cm"))
	float SpawnSpacing = 120.0f;

	/** @brief Number of rows to distribute agents across. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 1, ClampMax = 100))
	int32 SpawnRows = 1;

	/** @brief Spacing between rows. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0.0, Units = "cm"))
	float RowSpacing = 120.0f;

	/** @brief Team assignment for spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	EECSBattleAgentTeam Team = EECSBattleAgentTeam::Red;

	/** @brief If true, spawns agents on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;

	/** @brief Maximum health value assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Stats", meta = (ClampMin = 1.0))
	float MaxHealth = 100.0f;

	/** @brief Movement speed assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Stats", meta = (ClampMin = 0.0))
	float MoveSpeed = 300.0f;

	/** @brief Agent radius used for avoidance and spacing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Stats", meta = (ClampMin = 1.0, Units = "cm"))
	float AgentRadius = 42.0f;

	/** @brief Attack range assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0, Units = "cm"))
	float AttackRange = 140.0f;

	/** @brief Attack damage assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0))
	float AttackDamage = 10.0f;

	/** @brief Attack interval assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Combat", meta = (ClampMin = 0.05, Units = "s"))
	float AttackInterval = 0.6f;

	/** @brief Target search radius assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Targeting", meta = (ClampMin = 100.0, Units = "cm"))
	float TargetSearchRadius = 100000.0f;

	/** @brief Target refresh interval assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Targeting", meta = (ClampMin = 0.05, Units = "s"))
	float TargetRefreshInterval = 0.25f;

	/** @brief Destroy delay assigned to spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Death", meta = (ClampMin = 0.0, Units = "s"))
	float DestroyDelay = 2.0f;

	/** @brief Visual character class used to represent spawned agents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Visual")
	TSubclassOf<ACharacter> VisualCharacterClass;

private:
	/**
	 * @brief Creates or retrieves the Mass archetype used for battle agents.
	 * @param EntityManager Entity manager used to create the archetype.
	 * @return Archetype handle for spawned agents.
	 */
	FMassArchetypeHandle CreateOrGetAgentArchetype(FMassEntityManager& EntityManager) const;
};
