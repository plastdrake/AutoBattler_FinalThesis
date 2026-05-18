#pragma once
	/** @brief Timer handle used for auto-refresh. */
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
	/** @brief True while capture is actively running. */
	bool bCaptureActive = false;
	/** @brief True when capture has been finalized. */
	bool bCaptureFinalized = false;
	/** @brief Real-time seconds at the capture start. */
	double CaptureStartRealSeconds = 0.0;
	/** @brief Real-time seconds at the last sample. */
	double LastSampleRealSeconds = 0.0;
/** @brief Snapshot of finalized performance metrics for a battle round. */
	/** @brief Elapsed seconds since the round began. */
	float RoundElapsedSeconds = 0.0f;
	/** @brief Elapsed seconds captured after warmup. */
	float CapturedElapsedSeconds = 0.0f;
	/** @brief Number of samples recorded. */
	int32 SampleCount = 0;
	/** @brief Sum of FPS samples. */
	double SumFPS = 0.0;
	/** @brief Sum of frame time samples in milliseconds. */
	double SumFrameTimeMs = 0.0f;
	/** @brief Minimum FPS observed. */
	float MinFPS = FLT_MAX;
	/** @brief Maximum frame time observed in milliseconds. */
	float MaxFrameTimeMs = 0.0f;
	/** @brief Number of frames slower than 16.6 ms. */
	int32 HitchCountOver16_6 = 0;
	/** @brief Number of frames slower than 33.3 ms. */
	int32 HitchCountOver33_3 = 0;
	/** @brief Peak number of alive agents seen in the round. */
	int32 PeakAliveAgents = 0;
	/** @brief Last observed red team count. */
	int32 LastRedCount = 0;
	/** @brief Last observed blue team count. */
	int32 LastBlueCount = 0;
	/** @brief Captured frame time samples in milliseconds. */
	TArray<float> CapturedFrameTimesMs;
	/** @brief Average frames per second. */
	/** @brief Final average FPS value. */
	float FinalAvgFPS = 0.0f;
	/** @brief Final average frame time in milliseconds. */
	float FinalAvgFrameTimeMs = 0.0f;
	/** @brief Final 95th percentile frame time in milliseconds. */
	float FinalP95FrameTimeMs = 0.0f;
	/** @brief Final 99th percentile frame time in milliseconds. */
	float FinalP99FrameTimeMs = 0.0f;
	/** @brief Final minimum FPS value. */
	float FinalMinFPS = 0.0f;
	/** @brief Final maximum frame time in milliseconds. */
	float FinalMaxFrameTimeMs = 0.0f;

	/** @brief Average frame time in milliseconds. */
	UPROPERTY(BlueprintReadOnly, Category = "Battle Metrics")
	float AvgFrameMs = 0.0f;

	/** @brief 95th percentile frame time in milliseconds. */
	UPROPERTY(BlueprintReadOnly, Category = "Battle Metrics")
	float P95Ms = 0.0f;

	/** @brief 99th percentile frame time in milliseconds. */
	UPROPERTY(BlueprintReadOnly, Category = "Battle Metrics")
	float P99Ms = 0.0f;

	/** @brief Worst frame time in milliseconds. */
	UPROPERTY(BlueprintReadOnly, Category = "Battle Metrics")
	float WorstFrameMs = 0.0f;

	/** @brief Number of hitches above 16.6 ms. */
	UPROPERTY(BlueprintReadOnly, Category = "Battle Metrics")
	int32 HitchesOver16_6Ms = 0;

	/** @brief Number of hitches above 33.3 ms. */
	UPROPERTY(BlueprintReadOnly, Category = "Battle Metrics")
	int32 HitchesOver33_3Ms = 0;
};

/** @brief Source mode used when counting alive agents for metrics. */
UENUM(BlueprintType)
enum class EBattleMetricsCountSourceMode : uint8
{
	AutoDetect UMETA(DisplayName = "Auto Detect"),
	OOPAgents UMETA(DisplayName = "OOP Agents"),
	ECSVisuals UMETA(DisplayName = "ECS Visuals")
};

/** @brief UI widget that captures and displays performance metrics. */
UCLASS(Abstract)
class AUTOBATTLER_API UBattlePerformanceMetricsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief Resets internal state and starts capturing metrics. */
	UFUNCTION(BlueprintCallable, Category = "Battle Metrics")
	void ResetAndStartCapture();

	/** @brief Stops capturing metrics and finalizes results. */
	UFUNCTION(BlueprintCallable, Category = "Battle Metrics")
	void StopCaptureAndFinalize();

	/**
	 * @brief Returns true if capture is finalized.
	 * @return True when finalized metrics are available.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Metrics")
	bool IsCaptureFinalized() const { return bCaptureFinalized; }

	/**
	 * @brief Retrieves finalized metrics if available.
	 * @param OutSnapshot Output snapshot to populate.
	 * @return True when finalized metrics were returned.
	 */
	UFUNCTION(BlueprintPure, Category = "Battle Metrics")
	bool GetFinalizedMetrics(FBattlePerformanceMetricsSnapshot& OutSnapshot) const;

protected:
	/** @brief Initializes the widget and refresh timer. */
	virtual void NativeConstruct() override;
	/** @brief Clears timers before the widget is destroyed. */
	virtual void NativeDestruct() override;
	/**
	 * @brief Per-frame tick used to sample metrics.
	 * @param MyGeometry Widget geometry data.
	 * @param InDeltaTime Tick delta time in seconds.
	 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** @brief Count source used when checking round state. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics")
	EBattleMetricsCountSourceMode CountSourceMode = EBattleMetricsCountSourceMode::AutoDetect;

	/** @brief Warmup time before samples are recorded. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics", meta = (ClampMin = 0.0, Units = "s"))
	float WarmupDurationSeconds = 3.0f;

	/** @brief Auto-refresh interval for round state checks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics", meta = (ClampMin = 0.05, Units = "s"))
	float AutoRefreshInterval = 0.2f;

	/** @brief If true, starts capture when a round is detected. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics")
	bool bAutoStartWhenRoundDetected = true;

	/** @brief If true, finalizes capture when the round ends. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle Metrics")
	bool bAutoFinalizeWhenRoundEnds = true;

	/** @brief Text block used to display the summary string. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Battle Metrics")
	TObjectPtr<UTextBlock> metricssummarytext;

private:
	/** @brief Timer handle used for auto-refresh. */
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

	/** @brief Updates round state and decides capture transitions. */
	void UpdateRoundStateFromWorld();
	/** @brief Starts capture and resets timers. */
	void StartCapture();
	/** @brief Finalizes capture and computes summary stats. */
	void FinalizeCapture();
	/**
	 * @brief Samples a single frame for performance metrics.
	 * @param InDeltaTime Engine tick delta time in seconds.
	 */
	void SampleFrame(float InDeltaTime);
	/** @brief Refreshes the summary text widget. */
	void RefreshSummaryText();
	/** @brief Resets runtime counters and cached samples. */
	void ResetRuntimeStats();

	/**
	 * @brief Retrieves team counts based on the selected source mode.
	 * @param OutRedCount Output red count.
	 * @param OutBlueCount Output blue count.
	 */
	void GetCountsFromWorld(int32& OutRedCount, int32& OutBlueCount) const;
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

	/**
	 * @brief Computes the percentile for the provided sample set.
	 * @param Values Sample values (copied by value for sorting).
	 * @param Percentile Percentile to compute (0-100).
	 * @return Percentile value.
	 */
	static float ComputePercentile(TArray<float> Values, float Percentile);
};
