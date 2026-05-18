#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OOPBattleAgent.h"
#include "OOPBattleAgentSpawner.generated.h"

class UArrowComponent;

/** @brief Spawns OOP battle agents for a battle simulation team. */
UCLASS()
class AUTOBATTLER_API AOOPBattleAgentSpawner : public AActor
{
	GENERATED_BODY()

public:
	/** @brief Default constructor. */
	AOOPBattleAgentSpawner();

	/** @brief Spawns agents based on the current spawner settings. */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnAgents();

protected:
	/** @brief Handles spawn-on-begin-play behavior. */
	virtual void BeginPlay() override;

	/** @brief Direction indicator used to place agent rows. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> SpawnDirection;

	/** @brief Agent class to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AOOPBattleAgent> AgentClass;

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
	EBattleAgentTeam Team = EBattleAgentTeam::Red;

	/** @brief If true, spawns agents on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;
};
