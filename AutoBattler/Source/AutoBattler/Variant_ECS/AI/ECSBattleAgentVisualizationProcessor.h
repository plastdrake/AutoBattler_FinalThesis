#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "MassProcessor.h"
#include "ECSBattleAgentVisualizationProcessor.generated.h"

class ACharacter;

/** @brief Mass processor that updates ECS battle agent visuals. */
UCLASS()
class AUTOBATTLER_API UECSBattleAgentVisualizationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	/** @brief Default constructor. */
	UECSBattleAgentVisualizationProcessor();

protected:
	/**
	 * @brief Configures entity queries for this processor.
	 * @param EntityManager Entity manager used to build queries.
	 */
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	/**
	 * @brief Executes the processor for each tick.
	 * @param EntityManager Entity manager used to access entities.
	 * @param Context Mass execution context for this tick.
	 */
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	/** @brief Query that selects agent entities for visualization updates. */
	FMassEntityQuery AgentQuery;
	/** @brief Cache from entity handles to their spawned visual characters. */
	TMap<FMassEntityHandle, TWeakObjectPtr<ACharacter>> VisualsByEntity;
};
