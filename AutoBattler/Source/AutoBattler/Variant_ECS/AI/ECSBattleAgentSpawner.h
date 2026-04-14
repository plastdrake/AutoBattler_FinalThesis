#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "ECSBattleAgentFragments.h"
#include "ECSBattleAgentSpawner.generated.h"

struct FMassArchetypeHandle;
struct FMassEntityManager;
class UArrowComponent;

UCLASS()
class AUTOBATTLER_API AECSBattleAgentSpawner : public AActor
{
	GENERATED_BODY()

public:
	AECSBattleAgentSpawner();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnAgents();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> SpawnDirection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0, ClampMax = 500))
	int32 SpawnCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0.0, Units = "cm"))
	float SpawnSpacing = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 1, ClampMax = 100))
	int32 SpawnRows = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = 0.0, Units = "cm"))
	float RowSpacing = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	EECSBattleAgentTeam Team = EECSBattleAgentTeam::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Stats", meta = (ClampMin = 1.0))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Stats", meta = (ClampMin = 0.0))
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0, Units = "cm"))
	float AttackRange = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0))
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Combat", meta = (ClampMin = 0.05, Units = "s"))
	float AttackInterval = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Targeting", meta = (ClampMin = 100.0, Units = "cm"))
	float TargetSearchRadius = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Targeting", meta = (ClampMin = 0.05, Units = "s"))
	float TargetRefreshInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Death", meta = (ClampMin = 0.0, Units = "s"))
	float DestroyDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Agent|Visual")
	TSubclassOf<ACharacter> VisualCharacterClass;

private:
  FMassArchetypeHandle CreateOrGetAgentArchetype(FMassEntityManager& EntityManager) const;
};
