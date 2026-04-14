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
};
