#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleTeamCountWidget.generated.h"

class UTextBlock;

UENUM(BlueprintType)
enum class EBattleTeamCountSourceMode : uint8
{
	AutoDetect UMETA(DisplayName = "Auto Detect"),
	OOPAgents UMETA(DisplayName = "OOP Agents"),
	ECSVisuals UMETA(DisplayName = "ECS Visuals")
};

UCLASS(Abstract)
class AUTOBATTLER_API UBattleTeamCountWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Battle UI")
	void SetTeamCounts(int32 RedCount, int32 BlueCount);

	UFUNCTION(BlueprintCallable, Category = "Battle UI")
	void SetRedTeamCount(int32 RedCount);

	UFUNCTION(BlueprintCallable, Category = "Battle UI")
	void SetBlueTeamCount(int32 BlueCount);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

   UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle UI|Auto Update")
	EBattleTeamCountSourceMode CountSourceMode = EBattleTeamCountSourceMode::AutoDetect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle UI|Auto Update", meta = (ClampMin = 0.05, Units = "s"))
	float AutoRefreshInterval = 0.2f;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Battle UI")
	TObjectPtr<UTextBlock> redteamcounttext;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Battle UI")
	TObjectPtr<UTextBlock> blueteamcounttext;

private:
	int32 CachedRedTeamCount = 0;
	int32 CachedBlueTeamCount = 0;
	FTimerHandle AutoRefreshTimerHandle;

	void RefreshTeamCountTexts() const;
	void UpdateCountsFromWorld();
	void GetOOPCounts(int32& OutRedCount, int32& OutBlueCount) const;
	bool GetECSCounts(int32& OutRedCount, int32& OutBlueCount) const;
};
