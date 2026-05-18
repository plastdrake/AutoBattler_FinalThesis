#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleTeamCountWidget.generated.h"

class UTextBlock;

/** @brief Source mode for team count updates. */
UENUM(BlueprintType)
enum class EBattleTeamCountSourceMode : uint8
{
	AutoDetect UMETA(DisplayName = "Auto Detect"),
	OOPAgents UMETA(DisplayName = "OOP Agents"),
	ECSVisuals UMETA(DisplayName = "ECS Visuals")
};

/** @brief UI widget that displays live team counts for the battle simulation. */
UCLASS(Abstract)
class AUTOBATTLER_API UBattleTeamCountWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Sets both team counts at once.
	 * @param RedCount Red team count.
	 * @param BlueCount Blue team count.
	 */
	UFUNCTION(BlueprintCallable, Category = "Battle UI")
	void SetTeamCounts(int32 RedCount, int32 BlueCount);

	/**
	 * @brief Sets the red team count.
	 * @param RedCount Red team count.
	 */
	UFUNCTION(BlueprintCallable, Category = "Battle UI")
	void SetRedTeamCount(int32 RedCount);

	/**
	 * @brief Sets the blue team count.
	 * @param BlueCount Blue team count.
	 */
	UFUNCTION(BlueprintCallable, Category = "Battle UI")
	void SetBlueTeamCount(int32 BlueCount);

protected:
	/** @brief Initializes the widget and starts auto-refresh. */
	virtual void NativeConstruct() override;
	/** @brief Clears timers before the widget is destroyed. */
	virtual void NativeDestruct() override;

	/** @brief Source mode used when auto-refreshing team counts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle UI|Auto Update")
	EBattleTeamCountSourceMode CountSourceMode = EBattleTeamCountSourceMode::AutoDetect;

	/** @brief Auto-refresh interval in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle UI|Auto Update", meta = (ClampMin = 0.05, Units = "s"))
	float AutoRefreshInterval = 0.2f;

	/** @brief Text block that displays the red team count. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Battle UI")
	TObjectPtr<UTextBlock> redteamcounttext;

	/** @brief Text block that displays the blue team count. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Battle UI")
	TObjectPtr<UTextBlock> blueteamcounttext;

private:
	/** @brief Cached red team count. */
	int32 CachedRedTeamCount = 0;
	/** @brief Cached blue team count. */
	int32 CachedBlueTeamCount = 0;
	/** @brief Timer handle used for auto-refresh. */
	FTimerHandle AutoRefreshTimerHandle;

	/** @brief Applies cached counts to the widget text. */
	void RefreshTeamCountTexts() const;
	/** @brief Updates cached counts from the world state. */
	void UpdateCountsFromWorld();
	/**
	 * @brief Reads counts from the OOP agent registry.
	 * @param OutRedCount Output red count.
	 * @param OutBlueCount Output blue count.
	 */
	void GetOOPCounts(int32& OutRedCount, int32& OutBlueCount) const;
	/**
	 * @brief Reads counts from ECS visuals if available.
	 * @param OutRedCount Output red count.
	 * @param OutBlueCount Output blue count.
	 * @return True if ECS visuals were found and counted.
	 */
	bool GetECSCounts(int32& OutRedCount, int32& OutBlueCount) const;
};
