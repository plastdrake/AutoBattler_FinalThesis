#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ECSBattleAgentFragments.h"
#include "ECSBattleAgentVisual.generated.h"

class UAnimMontage;

UCLASS()
class AUTOBATTLER_API AECSBattleAgentVisual : public ACharacter
{
	GENERATED_BODY()

public:
	AECSBattleAgentVisual();
	virtual FVector GetVelocity() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UFUNCTION(BlueprintImplementableEvent, Category = "Battle Agent|Visual")
	void OnTeamAssigned(EECSBattleAgentTeam Team);

	UFUNCTION(BlueprintCallable, Category = "Battle Agent|Combat|Animation")
	void PlayAttackMontage();

   void SyncFromMassTransform(const FTransform& WorldTransform, float DeltaTimeSeconds);
	const FVector& GetCachedVelocity() const { return CachedVelocity; }

private:
	FVector CachedVelocity = FVector::ZeroVector;
};
