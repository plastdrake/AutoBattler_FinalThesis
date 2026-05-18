#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "ECSBattleAgentProcessor.generated.h"

struct FECSBattleAgentFragment;
struct FTransformFragment;

/** @brief Mass processor that drives ECS battle agent behavior. */
UCLASS()
class AUTOBATTLER_API UECSBattleAgentProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
    /** @brief Default constructor. */
    UECSBattleAgentProcessor();

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
    /** @brief Query that selects agent entities for behavior updates. */
	FMassEntityQuery AgentQuery;

    /** @brief Aggregated performance metrics for logging and benchmarking. */
    int64 TotalSeparationChecks = 0;
    /** @brief Total attacks processed. */
    int64 TotalAttacks = 0;
    /** @brief Total deaths processed. */
    int64 TotalDeaths = 0;
    /** @brief Total frames processed. */
    int64 FramesProcessed = 0;
    /** @brief Total processor time in seconds. */
    double TotalProcessorTimeSeconds = 0.0;
    /** @brief True once the final summary has been logged. */
    bool bFinalSummaryLogged = false;
};
