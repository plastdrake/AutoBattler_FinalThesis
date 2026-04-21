#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattlePerformanceMetricsWidget.generated.h"

class UTextBlock;

UENUM(BlueprintType)
enum class EBattleMetricsCountSourceMode : uint8
{
	AutoDetect UMETA(DisplayName = "Auto Detect"),
	OOPAgents UMETA(DisplayName = "OOP Agents"),
	ECSVisuals UMETA(DisplayName = "ECS Visuals")
};

UCLASS(Abstract)
class AUTOBATTLER_API UBattlePerformanceMetricsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Battle Metrics")
	void ResetAndStartCapture();

	UFUNCTION(BlueprintCallable, Category = "Battle Metrics")
	void StopCaptureAndFinalize();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics")
	EBattleMetricsCountSourceMode CountSourceMode = EBattleMetricsCountSourceMode::AutoDetect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics", meta = (ClampMin = 0.0, Units = "s"))
	float WarmupDurationSeconds = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics", meta = (ClampMin = 0.05, Units = "s"))
	float AutoRefreshInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics")
	bool bAutoStartWhenRoundDetected = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics")
	bool bAutoFinalizeWhenRoundEnds = true;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Battle Metrics")
	TObjectPtr<UTextBlock> metricssummarytext;

private:
	FTimerHandle AutoRefreshTimerHandle;

	bool bCaptureActive = false;
	bool bCaptureFinalized = false;
	double CaptureStartRealSeconds = 0.0;
	double LastSampleRealSeconds = 0.0;

	float RoundElapsedSeconds = 0.0f;
	float CapturedElapsedSeconds = 0.0f;
	int32 SampleCount = 0;
	double SumFPS = 0.0;
	double SumFrameTimeMs = 0.0;
	float MinFPS = FLT_MAX;
	float MaxFrameTimeMs = 0.0f;
	int32 HitchCountOver16_6 = 0;
	int32 HitchCountOver33_3 = 0;
	int32 PeakAliveAgents = 0;
	int32 LastRedCount = 0;
	int32 LastBlueCount = 0;
	TArray<float> CapturedFrameTimesMs;

	float FinalAvgFPS = 0.0f;
	float FinalAvgFrameTimeMs = 0.0f;
	float FinalP95FrameTimeMs = 0.0f;
	float FinalP99FrameTimeMs = 0.0f;
	float FinalMinFPS = 0.0f;
	float FinalMaxFrameTimeMs = 0.0f;

	void UpdateRoundStateFromWorld();
	void StartCapture();
	void FinalizeCapture();
	void SampleFrame(float InDeltaTime);
	void RefreshSummaryText();
	void ResetRuntimeStats();

	void GetCountsFromWorld(int32& OutRedCount, int32& OutBlueCount) const;
	void GetOOPCounts(int32& OutRedCount, int32& OutBlueCount) const;
	bool GetECSCounts(int32& OutRedCount, int32& OutBlueCount) const;

	static float ComputePercentile(TArray<float> Values, float Percentile);
};
