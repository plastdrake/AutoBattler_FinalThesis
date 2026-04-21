#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "ECSBattleAgentProcessor.generated.h"

struct FECSBattleAgentFragment;
struct FTransformFragment;

UCLASS()
class AUTOBATTLER_API UECSBattleAgentProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UECSBattleAgentProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery AgentQuery;

    // Simple aggregated performance metrics
    int64 TotalSeparationChecks = 0; // Resetting to zero
    int64 TotalAttacks = 0; // Resetting to zero
    int64 TotalDeaths = 0; // Resetting to zero
    int64 FramesProcessed = 0; // Resetting to zero
    double TotalProcessorTimeSeconds = 0.0; // Resetting to zero
    bool bFinalSummaryLogged = false; // Resetting to false
};
