#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BenchmarkTraceController.generated.h"

UCLASS()
class AUTOBATTLER_API ABenchmarkTraceController : public AActor
{
    GENERATED_BODY()

public:
    ABenchmarkTraceController();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void PollForSpawnCompletion();
    void StartTraceCapture();
    void PollForFightEnd();
    void StopTraceCapture();
    void FinalizeTraceAttempt();

    FTimerHandle PollSpawnTimerHandle;
    FTimerHandle PollFightTimerHandle;
    FTimerHandle StopTraceDelayHandle;

    bool bTraceRunning = false;
    double TraceStartTime = 0.0;

    int32 LastSpawnCount = -1;
    int32 StableSpawnTicks = 0;

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float SpawnPollInterval = 0.25f;

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float FightPollInterval = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float EndTraceDelaySeconds = 1.0f;

    // Console commands used to control tracing
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    FString TraceStartCommand = TEXT("Trace.File");

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    FString TraceStopCommand = TEXT("Trace.Stop");

    /** Generated trace output path (set when starting) */
    FString GeneratedTraceFilePath;
    /** The temporary Start.utrace path that the trace system writes to while capturing */
    FString TempTraceStartFilePath;

    FTimerHandle FinalizeTraceTimerHandle;
    // How many times to attempt finalizing (moving) the temp trace file before giving up
    int32 FinalizeAttemptsLeft = 0;

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    int32 FinalizeMaxAttempts = 20;

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float FinalizeRetryInterval = 0.25f;
};
