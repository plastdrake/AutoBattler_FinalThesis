#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OOPBattleAgent.generated.h"

UENUM(BlueprintType)
enum class EBattleAgentTeam : uint8
{
	Red UMETA(DisplayName = "Red Team"),
	Blue UMETA(DisplayName = "Blue Team")
};

class UAnimMontage;

UCLASS()
class AUTOBATTLER_API AOOPBattleAgent : public ACharacter
{
	GENERATED_BODY()

public:
	AOOPBattleAgent();

	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Battle Agent")
	void SetTeam(EBattleAgentTeam NewTeam);

	UFUNCTION(BlueprintPure, Category = "Battle Agent")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Battle Agent")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Battle Agent")
	EBattleAgentTeam GetTeam() const { return Team; }

	void ReceiveDamage(float DamageAmount, AOOPBattleAgent* DamageCauser);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent")
	EBattleAgentTeam Team = EBattleAgentTeam::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Stats", meta = (ClampMin = 1.0))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Battle Agent|Stats", meta = (ClampMin = 0.0))
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Stats", meta = (ClampMin = 0.0))
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0, Units = "cm"))
	float AttackRange = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat", meta = (ClampMin = 0.0))
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat", meta = (ClampMin = 0.05, Units = "s"))
	float AttackInterval = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Combat|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Targeting", meta = (ClampMin = 100.0, Units = "cm"))
	float TargetSearchRadius = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Targeting", meta = (ClampMin = 0.05, Units = "s"))
	float TargetRefreshInterval = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle Agent|Death", meta = (ClampMin = 0.0, Units = "s"))
	float DestroyDelay = 2.0f;

private:
	UPROPERTY(Transient)
	TObjectPtr<AOOPBattleAgent> CurrentTarget;

	float TimeUntilNextAttack = 0.0f;
	float TimeUntilTargetRefresh = 0.0f;

	static TArray<TWeakObjectPtr<AOOPBattleAgent>> AgentRegistry;

	void RefreshTarget();
    void MoveOrAttackTarget();
	void Die();
};
