#include "BenchmarkTraceController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "OOPBattleAgent.h"
#include "ECSBattleAgentVisual.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFile.h"

ABenchmarkTraceController::ABenchmarkTraceController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABenchmarkTraceController::BeginPlay()
{
    Super::BeginPlay();

    if (!bBenchmarkTracingEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Controller is disabled."));
        return;
    }

    // Start polling to detect when spawning is complete
    GetWorld()->GetTimerManager().SetTimer(PollSpawnTimerHandle, this, &ABenchmarkTraceController::PollForSpawnCompletion, SpawnPollInterval, true);
}

void ABenchmarkTraceController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearTimer(PollSpawnTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(PollFightTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(StopTraceDelayHandle);
    GetWorld()->GetTimerManager().ClearTimer(FinalizeTraceTimerHandle);

    if (bTraceRunning)
    {
        StopTraceCapture();
    }

    Super::EndPlay(EndPlayReason);
}

void ABenchmarkTraceController::SetBenchmarkTracingEnabled(const bool bEnabled)
{
    if (bBenchmarkTracingEnabled == bEnabled)
    {
        return;
    }

    bBenchmarkTracingEnabled = bEnabled;

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FTimerManager& TimerManager = World->GetTimerManager();
    if (!bBenchmarkTracingEnabled)
    {
        TimerManager.ClearTimer(PollSpawnTimerHandle);
        TimerManager.ClearTimer(PollFightTimerHandle);
        TimerManager.ClearTimer(StopTraceDelayHandle);
        TimerManager.ClearTimer(FinalizeTraceTimerHandle);

        if (bTraceRunning)
        {
            StopTraceCapture();
        }

        UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Controller disabled."));
        return;
    }

    LastSpawnCount = -1;
    StableSpawnTicks = 0;

    if (!bTraceRunning)
    {
        TimerManager.SetTimer(PollSpawnTimerHandle, this, &ABenchmarkTraceController::PollForSpawnCompletion, SpawnPollInterval, true);
    }

    UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Controller enabled."));
}

void ABenchmarkTraceController::PollForSpawnCompletion()
{
    if (!bBenchmarkTracingEnabled)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(PollSpawnTimerHandle);
        }
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 OOPCount = 0;
    for (TActorIterator<AOOPBattleAgent> It(World); It; ++It)
    {
        ++OOPCount;
    }

    int32 ECSVisualCount = 0;
    for (TActorIterator<AECSBattleAgentVisual> It(World); It; ++It)
    {
        ++ECSVisualCount;
    }

    const int32 TotalCount = OOPCount + ECSVisualCount;

    // simple heuristic: wait until spawn count stabilizes for a few polls
    if (TotalCount == LastSpawnCount)
    {
        ++StableSpawnTicks;
    }
    else
    {
        StableSpawnTicks = 0;
        LastSpawnCount = TotalCount;
    }

    // if stable for 3 polls and > 0, consider spawn complete
    if (StableSpawnTicks >= 3 && TotalCount > 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(PollSpawnTimerHandle);
        StartTraceCapture();

        // start polling for fight end
        GetWorld()->GetTimerManager().SetTimer(PollFightTimerHandle, this, &ABenchmarkTraceController::PollForFightEnd, FightPollInterval, true);
    }
}

void ABenchmarkTraceController::StartTraceCapture()
{
    if (!bBenchmarkTracingEnabled || bTraceRunning)
    {
        return;
    }

    // create a timestamped filename under Saved/Profiling
    const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
    const FString ProfilingDir = FPaths::ProjectSavedDir() / TEXT("Profiling");
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*ProfilingDir))
    {
        PlatformFile.CreateDirectoryTree(*ProfilingDir);
    }

    GeneratedTraceFilePath = ProfilingDir / FString::Printf(TEXT("AutoBattler_%s.utrace"), *Timestamp);
    // default temp start file the tracer may use
    TempTraceStartFilePath = ProfilingDir / TEXT("Start.utrace");

    // If a previous Temp Start file exists, try to move it aside or delete it.
    // If we can't remove it, fall back to a unique Start_<timestamp>.utrace name.
    if (PlatformFile.FileExists(*TempTraceStartFilePath))
    {
        const FString BackupPath = ProfilingDir / FString::Printf(TEXT("Start_old_%s.utrace"), *Timestamp);
        if (PlatformFile.FileExists(*BackupPath))
        {
            PlatformFile.DeleteFile(*BackupPath);
        }

        bool bHandled = false;
        if (PlatformFile.MoveFile(*BackupPath, *TempTraceStartFilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Moved existing '%s' to '%s' before starting new trace."), *TempTraceStartFilePath, *BackupPath);
            bHandled = true;
        }
        else if (PlatformFile.DeleteFile(*TempTraceStartFilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Deleted existing temp trace file '%s' before starting."), *TempTraceStartFilePath);
            bHandled = true;
        }

        if (!bHandled)
        {
            // Fall back to a unique temp filename so tracer won't collide with an existing locked Start.utrace
            TempTraceStartFilePath = ProfilingDir / FString::Printf(TEXT("Start_%s.utrace"), *Timestamp);
            UE_LOG(LogTemp, Warning, TEXT("BenchmarkTrace: Could not move or delete existing temp trace file. Using fallback temp trace file '%s'"), *TempTraceStartFilePath);
        }
    }

    // issue Trace.File with explicit output path
    const FString Command = FString::Printf(TEXT("%s %s"), *TraceStartCommand, *TempTraceStartFilePath);

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->ConsoleCommand(Command, true);
        bTraceRunning = true;
        TraceStartTime = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Started trace capture at %f, saving to '%s'"), TraceStartTime, *GeneratedTraceFilePath);
    }
}

void ABenchmarkTraceController::PollForFightEnd()
{
    if (!bBenchmarkTracingEnabled)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(PollFightTimerHandle);
        }
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Check if either team has zero alive units
    int32 OOPAliveRed = 0, OOPAliveBlue = 0;
    for (TActorIterator<AOOPBattleAgent> It(World); It; ++It)
    {
        AOOPBattleAgent* Agent = *It;
        if (!IsValid(Agent)) continue;
        if (!Agent->IsAlive()) continue;
        if (Agent->GetTeam() == EBattleAgentTeam::Red) ++OOPAliveRed; else ++OOPAliveBlue;
    }

    int32 ECSAliveRed = 0, ECSAliveBlue = 0;
    for (TActorIterator<AECSBattleAgentVisual> It(World); It; ++It)
    {
        AECSBattleAgentVisual* Visual = *It;
        if (!IsValid(Visual)) continue;
        // we don't have direct health on visuals; assume visuals exist for alive entities
        // In ECS variant visuals are removed when entity destroyed, so counting visuals is sufficient
        // Determine team via getter
        if (Visual->HasAssignedTeam())
        {
            if (Visual->GetAssignedTeam() == EECSBattleAgentTeam::Red) ++ECSAliveRed; else ++ECSAliveBlue;
        }
    }

    const int32 TotalRed = OOPAliveRed + ECSAliveRed;
    const int32 TotalBlue = OOPAliveBlue + ECSAliveBlue;

    if (TotalRed == 0 || TotalBlue == 0)
    {
        // fight ended; stop polling and schedule stopping trace after delay
        GetWorld()->GetTimerManager().ClearTimer(PollFightTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(StopTraceDelayHandle, this, &ABenchmarkTraceController::StopTraceCapture, EndTraceDelaySeconds, false);
        UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Detected fight end. Scheduling trace stop in %f seconds."), EndTraceDelaySeconds);
    }
}

void ABenchmarkTraceController::StopTraceCapture()
{
    if (!bTraceRunning)
    {
        return;
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->ConsoleCommand(TraceStopCommand, true);
        bTraceRunning = false;
        const double EndTime = FPlatformTime::Seconds();
        const double Duration = EndTime - TraceStartTime;
        UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Stopped trace capture at %f, duration=%.3f seconds. File: %s"), EndTime, Duration, *GeneratedTraceFilePath);

        // Schedule finalize to move the on-disk file (Start.utrace) to our generated filename.
        // Try multiple times because the tracing system may still hold the file for a short while.
        FinalizeAttemptsLeft = FinalizeMaxAttempts;
        GetWorld()->GetTimerManager().SetTimer(FinalizeTraceTimerHandle, this, &ABenchmarkTraceController::FinalizeTraceAttempt, FinalizeRetryInterval, true);
    }
}

void ABenchmarkTraceController::FinalizeTraceAttempt()
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (PlatformFile.FileExists(*TempTraceStartFilePath))
    {
        // remove existing target if present
        if (PlatformFile.FileExists(*GeneratedTraceFilePath))
        {
            PlatformFile.DeleteFile(*GeneratedTraceFilePath);
        }
        const bool bMoved = PlatformFile.MoveFile(*GeneratedTraceFilePath, *TempTraceStartFilePath);
        if (bMoved)
        {
            UE_LOG(LogTemp, Log, TEXT("BenchmarkTrace: Finalized trace file to '%s'"), *GeneratedTraceFilePath);
            GetWorld()->GetTimerManager().ClearTimer(FinalizeTraceTimerHandle);
        }
        else
        {
            --FinalizeAttemptsLeft;
            if (FinalizeAttemptsLeft <= 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("BenchmarkTrace: Failed to move trace file from '%s' to '%s' after multiple attempts"), *TempTraceStartFilePath, *GeneratedTraceFilePath);
                GetWorld()->GetTimerManager().ClearTimer(FinalizeTraceTimerHandle);
            }
            else
            {
                UE_LOG(LogTemp, Verbose, TEXT("BenchmarkTrace: Move failed, retrying... attempts left: %d"), FinalizeAttemptsLeft);
            }
        }
    }
    else
    {
        --FinalizeAttemptsLeft;
        if (FinalizeAttemptsLeft <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("BenchmarkTrace: Temp trace file '%s' not found when finalizing after multiple attempts."), *TempTraceStartFilePath);
            GetWorld()->GetTimerManager().ClearTimer(FinalizeTraceTimerHandle);
        }
    }
}
