#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BenchmarkTraceController.generated.h"

/** @brief Controller that drives trace-based benchmark captures. */
UCLASS()
class AUTOBATTLER_API ABenchmarkTraceController : public AActor
{
    GENERATED_BODY()

public:
    /** @brief Default constructor. */
    ABenchmarkTraceController();

    /**
     * @brief Enables or disables benchmark tracing.
     * @param bEnabled True to enable tracing.
     */
    UFUNCTION(BlueprintCallable, Category = "Benchmark")
    void SetBenchmarkTracingEnabled(bool bEnabled);

    /**
     * @brief Returns whether benchmark tracing is enabled.
     * @return True if tracing is enabled.
     */
    UFUNCTION(BlueprintPure, Category = "Benchmark")
    bool IsBenchmarkTracingEnabled() const { return bBenchmarkTracingEnabled; }

protected:
    /** @brief Initializes trace polling. */
    virtual void BeginPlay() override;
    /**
     * @brief Cleanup on end play.
     * @param EndPlayReason Reason for ending play.
     */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /** @brief Polls until all agents have spawned. */
    void PollForSpawnCompletion();
    /** @brief Starts a trace capture session. */
    void StartTraceCapture();
    /** @brief Polls until the fight has ended. */
    void PollForFightEnd();
    /** @brief Stops the trace capture session. */
    void StopTraceCapture();
    /** @brief Finalizes the trace file and handles retries. */
    void FinalizeTraceAttempt();

    /** @brief Timer for spawn polling. */
    FTimerHandle PollSpawnTimerHandle;
    /** @brief Timer for fight-end polling. */
    FTimerHandle PollFightTimerHandle;
    /** @brief Timer for delayed trace stop. */
    FTimerHandle StopTraceDelayHandle;

    /** @brief True while a trace is running. */
    bool bTraceRunning = false;
    /** @brief Real-time seconds when the trace started. */
    double TraceStartTime = 0.0;

    /** @brief Last observed spawn count. */
    int32 LastSpawnCount = -1;
    /** @brief Number of consecutive ticks with stable spawn count. */
    int32 StableSpawnTicks = 0;

    /** @brief Spawn poll interval in seconds. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float SpawnPollInterval = 0.25f;

    /** @brief Fight poll interval in seconds. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float FightPollInterval = 0.1f;

    /** @brief Delay before ending the trace after fight completion. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float EndTraceDelaySeconds = 1.0f;

    /** @brief True when tracing is enabled. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    bool bBenchmarkTracingEnabled = true;

    /** @brief Console command used to start tracing. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    FString TraceStartCommand = TEXT("Trace.File");

    /** @brief Console command used to stop tracing. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    FString TraceStopCommand = TEXT("Trace.Stop");

    /** @brief Generated trace output path (set when starting). */
    FString GeneratedTraceFilePath;
    /** @brief Temporary Start.utrace path written during capture. */
    FString TempTraceStartFilePath;

    /** @brief Timer used to finalize the trace after stop. */
    FTimerHandle FinalizeTraceTimerHandle;
    /** @brief Remaining finalize attempts before giving up. */
    int32 FinalizeAttemptsLeft = 0;

    /** @brief Maximum number of finalize attempts. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    int32 FinalizeMaxAttempts = 20;

    /** @brief Retry interval between finalize attempts. */
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float FinalizeRetryInterval = 0.25f;
};
