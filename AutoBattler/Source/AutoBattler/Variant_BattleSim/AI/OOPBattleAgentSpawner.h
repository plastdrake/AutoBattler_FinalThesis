#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OOPBattleAgent.h"
#include "OOPBattleAgentSpawner.generated.h"

class UArrowComponent;

UCLASS()
class AUTOBATTLER_API AOOPBattleAgentSpawner : public AActor
{
	GENERATED_BODY()

public:
	AOOPBattleAgentSpawner();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnAgents();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> SpawnDirection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AOOPBattleAgent> AgentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0, ClampMax = 500))
	int32 SpawnCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0.0, Units = "cm"))
	float SpawnSpacing = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	EBattleAgentTeam Team = EBattleAgentTeam::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;
};
