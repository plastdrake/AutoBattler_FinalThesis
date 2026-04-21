#include "BattlePerformanceMetricsWidget.h"

#include "Components/TextBlock.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Variant_BattleSim/AI/OOPBattleAgent.h"
#include "Variant_ECS/AI/ECSBattleAgentVisual.h"

void UBattlePerformanceMetricsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshSummaryText();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoRefreshTimerHandle,
			this,
			&UBattlePerformanceMetricsWidget::UpdateRoundStateFromWorld,
			FMath::Max(0.05f, AutoRefreshInterval),
			true);
	}
}

void UBattlePerformanceMetricsWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoRefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UBattlePerformanceMetricsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	SampleFrame(InDeltaTime);
}

void UBattlePerformanceMetricsWidget::ResetAndStartCapture()
{
	ResetRuntimeStats();
	StartCapture();
	RefreshSummaryText();
}

void UBattlePerformanceMetricsWidget::StopCaptureAndFinalize()
{
	if (!bCaptureActive && !bCaptureFinalized)
	{
		ResetRuntimeStats();
	}

	FinalizeCapture();
	RefreshSummaryText();
}

void UBattlePerformanceMetricsWidget::UpdateRoundStateFromWorld()
{
	int32 RedCount = 0;
	int32 BlueCount = 0;
	GetCountsFromWorld(RedCount, BlueCount);

	LastRedCount = RedCount;
	LastBlueCount = BlueCount;
	PeakAliveAgents = FMath::Max(PeakAliveAgents, RedCount + BlueCount);

	const bool bRoundLive = (RedCount > 0 && BlueCount > 0);

	if (!bCaptureActive && !bCaptureFinalized && bAutoStartWhenRoundDetected && bRoundLive)
	{
		StartCapture();
	}

	if (bCaptureActive && bAutoFinalizeWhenRoundEnds && !bRoundLive)
	{
		FinalizeCapture();
	}

	RefreshSummaryText();
}

void UBattlePerformanceMetricsWidget::StartCapture()
{
	bCaptureActive = true;
	bCaptureFinalized = false;
	RoundElapsedSeconds = 0.0f;
	CapturedElapsedSeconds = 0.0f;
}

void UBattlePerformanceMetricsWidget::FinalizeCapture()
{
	if (bCaptureFinalized)
	{
		return;
	}

	bCaptureActive = false;
	bCaptureFinalized = true;

	FinalAvgFPS = (SampleCount > 0) ? static_cast<float>(SumFPS / static_cast<double>(SampleCount)) : 0.0f;
	FinalAvgFrameTimeMs = (SampleCount > 0) ? static_cast<float>(SumFrameTimeMs / static_cast<double>(SampleCount)) : 0.0f;
	FinalP95FrameTimeMs = ComputePercentile(CapturedFrameTimesMs, 95.0f);
	FinalP99FrameTimeMs = ComputePercentile(CapturedFrameTimesMs, 99.0f);
	FinalMinFPS = (SampleCount > 0) ? MinFPS : 0.0f;
	FinalMaxFrameTimeMs = MaxFrameTimeMs;
}

void UBattlePerformanceMetricsWidget::SampleFrame(const float InDeltaTime)
{
	if (!bCaptureActive)
	{
		return;
	}

	RoundElapsedSeconds += InDeltaTime;
	if (RoundElapsedSeconds < WarmupDurationSeconds)
	{
		return;
	}

	const float SafeDelta = FMath::Max(InDeltaTime, KINDA_SMALL_NUMBER);
	const float FrameTimeMs = SafeDelta * 1000.0f;
	const float FPS = 1.0f / SafeDelta;

	CapturedElapsedSeconds += InDeltaTime;
	++SampleCount;
	SumFPS += FPS;
	SumFrameTimeMs += FrameTimeMs;
	MinFPS = FMath::Min(MinFPS, FPS);
	MaxFrameTimeMs = FMath::Max(MaxFrameTimeMs, FrameTimeMs);
	CapturedFrameTimesMs.Add(FrameTimeMs);

	if (FrameTimeMs > 16.6f)
	{
		++HitchCountOver16_6;
	}
	if (FrameTimeMs > 33.3f)
	{
		++HitchCountOver33_3;
	}
}

void UBattlePerformanceMetricsWidget::RefreshSummaryText()
{
	if (!metricssummarytext)
	{
		return;
	}

	if (!bCaptureActive && !bCaptureFinalized)
	{
		metricssummarytext->SetText(FText::FromString(TEXT("Metrics: waiting for round start...")));
		return;
	}

	const bool bShowFinal = bCaptureFinalized;
	const float DisplayAvgFPS = bShowFinal
		? FinalAvgFPS
		: ((SampleCount > 0) ? static_cast<float>(SumFPS / static_cast<double>(SampleCount)) : 0.0f);
	const float DisplayAvgFrameMs = bShowFinal
		? FinalAvgFrameTimeMs
		: ((SampleCount > 0) ? static_cast<float>(SumFrameTimeMs / static_cast<double>(SampleCount)) : 0.0f);
	const float DisplayP95 = bShowFinal ? FinalP95FrameTimeMs : ComputePercentile(CapturedFrameTimesMs, 95.0f);
	const float DisplayP99 = bShowFinal ? FinalP99FrameTimeMs : ComputePercentile(CapturedFrameTimesMs, 99.0f);
	const float DisplayMinFPS = bShowFinal ? FinalMinFPS : ((SampleCount > 0) ? MinFPS : 0.0f);
	const float DisplayMaxFrameMs = bShowFinal ? FinalMaxFrameTimeMs : MaxFrameTimeMs;

	const FString Summary = FString::Printf(
		TEXT("Captured: %.2fs | Samples: %d | AvgFPS: %.1f | MinFPS: %.1f | AvgFrame: %.2fms | P95: %.2fms | P99: %.2fms | WorstFrame: %.2fms | Hitches>16.6: %d | Hitches>33.3: %d"),
		CapturedElapsedSeconds,
		SampleCount,
		DisplayAvgFPS,
		DisplayMinFPS,
		DisplayAvgFrameMs,
		DisplayP95,
		DisplayP99,
		DisplayMaxFrameMs,
		HitchCountOver16_6,
		HitchCountOver33_3);

	metricssummarytext->SetText(FText::FromString(Summary));
}

void UBattlePerformanceMetricsWidget::ResetRuntimeStats()
{
	bCaptureActive = false;
	bCaptureFinalized = false;

	RoundElapsedSeconds = 0.0f;
	CapturedElapsedSeconds = 0.0f;
	SampleCount = 0;
	SumFPS = 0.0;
	SumFrameTimeMs = 0.0;
	MinFPS = FLT_MAX;
	MaxFrameTimeMs = 0.0f;
	HitchCountOver16_6 = 0;
	HitchCountOver33_3 = 0;
	PeakAliveAgents = 0;
	LastRedCount = 0;
	LastBlueCount = 0;
	CapturedFrameTimesMs.Reset();

	FinalAvgFPS = 0.0f;
	FinalAvgFrameTimeMs = 0.0f;
	FinalP95FrameTimeMs = 0.0f;
	FinalP99FrameTimeMs = 0.0f;
	FinalMinFPS = 0.0f;
	FinalMaxFrameTimeMs = 0.0f;
}

void UBattlePerformanceMetricsWidget::GetCountsFromWorld(int32& OutRedCount, int32& OutBlueCount) const
{
	switch (CountSourceMode)
	{
	case EBattleMetricsCountSourceMode::OOPAgents:
		GetOOPCounts(OutRedCount, OutBlueCount);
		break;

	case EBattleMetricsCountSourceMode::ECSVisuals:
		if (!GetECSCounts(OutRedCount, OutBlueCount))
		{
			GetOOPCounts(OutRedCount, OutBlueCount);
		}
		break;

	case EBattleMetricsCountSourceMode::AutoDetect:
	default:
		GetOOPCounts(OutRedCount, OutBlueCount);
		if (OutRedCount == 0 && OutBlueCount == 0)
		{
			GetECSCounts(OutRedCount, OutBlueCount);
		}
		break;
	}
}

void UBattlePerformanceMetricsWidget::GetOOPCounts(int32& OutRedCount, int32& OutBlueCount) const
{
	OutRedCount = AOOPBattleAgent::GetAliveTeamCount(EBattleAgentTeam::Red);
	OutBlueCount = AOOPBattleAgent::GetAliveTeamCount(EBattleAgentTeam::Blue);
}

bool UBattlePerformanceMetricsWidget::GetECSCounts(int32& OutRedCount, int32& OutBlueCount) const
{
	OutRedCount = 0;
	OutBlueCount = 0;

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bool bFoundAnyAssignedECSVisual = false;
	for (TActorIterator<AECSBattleAgentVisual> It(World); It; ++It)
	{
		const AECSBattleAgentVisual* Visual = *It;
		if (!IsValid(Visual) || !Visual->HasAssignedTeam())
		{
			continue;
		}

		bFoundAnyAssignedECSVisual = true;
		if (Visual->GetAssignedTeam() == EECSBattleAgentTeam::Red)
		{
			++OutRedCount;
		}
		else
		{
			++OutBlueCount;
		}
	}

	return bFoundAnyAssignedECSVisual;
}

float UBattlePerformanceMetricsWidget::ComputePercentile(TArray<float> Values, const float Percentile)
{
	if (Values.Num() == 0)
	{
		return 0.0f;
	}

	Values.Sort();
	const float ClampedPercentile = FMath::Clamp(Percentile, 0.0f, 100.0f);
	const int32 Index = FMath::Clamp(
		FMath::CeilToInt((ClampedPercentile / 100.0f) * static_cast<float>(Values.Num() - 1)),
		0,
		Values.Num() - 1);

	return Values[Index];
}
