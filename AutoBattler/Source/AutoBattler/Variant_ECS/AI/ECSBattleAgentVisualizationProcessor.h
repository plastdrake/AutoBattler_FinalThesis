#pragma once

#include "CoreMinimal.h"
#include "MassEntityHandle.h"
#include "MassProcessor.h"
#include "ECSBattleAgentVisualizationProcessor.generated.h"

class ACharacter;

UCLASS()
class AUTOBATTLER_API UECSBattleAgentVisualizationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UECSBattleAgentVisualizationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery AgentQuery;
  TMap<FMassEntityHandle, TWeakObjectPtr<ACharacter>> VisualsByEntity;
};
