#include "BenchmarkMetricsBatchController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

ABenchmarkMetricsBatchController::ABenchmarkMetricsBatchController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABenchmarkMetricsBatchController::BeginPlay()
{
	Super::BeginPlay();

	if (!bAutoStartBatch)
	{
		UE_LOG(LogTemp, Log, TEXT("BenchmarkBatch: Controller disabled."));
		return;
	}

	int32 ParsedModelIndex = 0;
	int32 ParsedCountIndex = 0;
	int32 ParsedRunIndex = 1;
	if (!ParseRunOptions(ParsedModelIndex, ParsedCountIndex, ParsedRunIndex))
	{
		StartBatchByReloadingLevel();
		return;
	}

	CurrentModelIndex = ParsedModelIndex;
	CurrentCountIndex = ParsedCountIndex;
	CurrentRunIndex = ParsedRunIndex;
	StartConfiguredRunFromOptions();
}

void ABenchmarkMetricsBatchController::StartBatchByReloadingLevel()
{
	if (AgentCounts.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BenchmarkBatch: AgentCounts is empty."));
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString BenchmarkDir = GetBenchmarkDir();
	if (!PlatformFile.DirectoryExists(*BenchmarkDir))
	{
		PlatformFile.CreateDirectoryTree(*BenchmarkDir);
	}

	PlatformFile.DeleteFile(*GetRawCsvPath());
	SaveRunState(0, 0, 1, true);

	UE_LOG(LogTemp, Log, TEXT("BenchmarkBatch: Starting fresh benchmark run."));
	OpenLevelForRun(0, 0, 1);
}

void ABenchmarkMetricsBatchController::StartConfiguredRunFromOptions()
{
	if (!GetWorld())
	{
		return;
	}

	bRunRecorded = false;
	CachedMetricsWidget = nullptr;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("BenchmarkBatch: Running model=%s, agentCount=%d, run=%d/%d"),
     *ModelToString(GetModelForIndex(CurrentModelIndex)),
		AgentCounts.IsValidIndex(CurrentCountIndex) ? AgentCounts[CurrentCountIndex] : -1,
		CurrentRunIndex,
		RunsPerAgentCount);

	GetWorld()->GetTimerManager().SetTimer(
		CompletionPollTimerHandle,
		this,
		&ABenchmarkMetricsBatchController::PollForRunCompletion,
		FMath::Max(0.05f, CompletionPollInterval),
		true);
}

void ABenchmarkMetricsBatchController::PollForRunCompletion()
{
	if (bRunRecorded)
	{
		return;
	}

	UBattlePerformanceMetricsWidget* MetricsWidget = FindMetricsWidget();
	if (!MetricsWidget)
	{
		return;
	}

	FBattlePerformanceMetricsSnapshot Snapshot;
	if (!MetricsWidget->GetFinalizedMetrics(Snapshot))
	{
		return;
	}

	bRunRecorded = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CompletionPollTimerHandle);
	}

	RecordCompletedRunAndAdvance(Snapshot);
}

void ABenchmarkMetricsBatchController::RecordCompletedRunAndAdvance(const FBattlePerformanceMetricsSnapshot& Metrics)
{
	if (!AgentCounts.IsValidIndex(CurrentCountIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("BenchmarkBatch: Invalid count index while recording."));
		return;
	}

    const EBenchmarkBatchModel Model = GetModelForIndex(CurrentModelIndex);

	FBenchmarkRawRunRecord Record;
	Record.Model = Model;
	Record.AgentCount = AgentCounts[CurrentCountIndex];
	Record.RunIndex = CurrentRunIndex;
	Record.Metrics = Metrics;

	if (!AppendRawRecord(Record))
	{
		UE_LOG(LogTemp, Warning, TEXT("BenchmarkBatch: Failed to append raw CSV record."));
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("BenchmarkBatch: Recorded model=%s count=%d run=%d/%d"),
		*ModelToString(Model),
		Record.AgentCount,
		CurrentRunIndex,
		RunsPerAgentCount);

	int32 NextModelIndex = CurrentModelIndex;
	int32 NextCountIndex = CurrentCountIndex;
	int32 NextRunIndex = CurrentRunIndex + 1;

	if (NextRunIndex > RunsPerAgentCount)
	{
		NextRunIndex = 1;
		++NextCountIndex;
		if (NextCountIndex >= AgentCounts.Num())
		{
			NextCountIndex = 0;
			++NextModelIndex;
		}
	}

    if (NextModelIndex >= GetModelRunCount())
	{
		FinalizeAndWriteAggregateCsvs();
      SaveRunState(0, 0, 1, false);
		UE_LOG(LogTemp, Log, TEXT("BenchmarkBatch: Completed all runs."));
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("BenchmarkBatch: Advancing to modelIndex=%d countIndex=%d run=%d"),
		NextModelIndex,
		NextCountIndex,
		NextRunIndex);

	OpenLevelForRun(NextModelIndex, NextCountIndex, NextRunIndex);
}

void ABenchmarkMetricsBatchController::OpenLevelForRun(const int32 InModelIndex, const int32 InCountIndex, const int32 InRunIndex) const
{
	UWorld* World = GetWorld();
	if (!World || !AgentCounts.IsValidIndex(InCountIndex))
	{
		return;
	}

    const EBenchmarkBatchModel Model = GetModelForIndex(InModelIndex);
	const int32 TotalAgentCount = AgentCounts[InCountIndex];
	const FString Options = FString::Printf(
		TEXT("BenchmarkBatch=1?BenchmarkModel=%s?BenchmarkCount=%d?BenchmarkModelIndex=%d?BenchmarkCountIndex=%d?BenchmarkRunIndex=%d"),
		*ModelToString(Model),
		TotalAgentCount,
		InModelIndex,
		InCountIndex,
		InRunIndex);

	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true, Options);
}

void ABenchmarkMetricsBatchController::FinalizeAndWriteAggregateCsvs()
{
	TArray<FBenchmarkRawRunRecord> Records;
	if (!TryReadRawRecords(Records) || Records.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BenchmarkBatch: No raw records found for aggregation."));
		return;
	}

	FString AverageCsv = TEXT("Model,Agents_nr,Captured_s,Samples,AvgFPS,MinFPS,AvgFrame_ms,P95_ms,P99_ms,WorstFrame_ms,Hitches_gt_16_6ms,Hitches_gt_33_3ms\n");
	FString MedianCsv = AverageCsv;

    for (int32 ModelIndex = 0; ModelIndex < GetModelRunCount(); ++ModelIndex)
	{
       const EBenchmarkBatchModel Model = GetModelForIndex(ModelIndex);
		for (const int32 AgentCount : AgentCounts)
		{
			TArray<float> CapturedSeconds;
			TArray<int32> Samples;
			TArray<float> AvgFPS;
			TArray<float> MinFPS;
			TArray<float> AvgFrameMs;
			TArray<float> P95Ms;
			TArray<float> P99Ms;
			TArray<float> WorstFrameMs;
			TArray<int32> Hitches16;
			TArray<int32> Hitches33;

			for (const FBenchmarkRawRunRecord& Record : Records)
			{
				if (Record.Model != Model || Record.AgentCount != AgentCount)
				{
					continue;
				}

				CapturedSeconds.Add(Record.Metrics.CapturedSeconds);
				Samples.Add(Record.Metrics.Samples);
				AvgFPS.Add(Record.Metrics.AvgFPS);
				MinFPS.Add(Record.Metrics.MinFPS);
				AvgFrameMs.Add(Record.Metrics.AvgFrameMs);
				P95Ms.Add(Record.Metrics.P95Ms);
				P99Ms.Add(Record.Metrics.P99Ms);
				WorstFrameMs.Add(Record.Metrics.WorstFrameMs);
				Hitches16.Add(Record.Metrics.HitchesOver16_6Ms);
				Hitches33.Add(Record.Metrics.HitchesOver33_3Ms);
			}

			if (CapturedSeconds.Num() == 0)
			{
				continue;
			}

			AverageCsv += FString::Printf(
				TEXT("%s,%d,%.4f,%.2f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.2f,%.2f\n"),
				*ModelToString(Model),
				AgentCount,
				ComputeAverage(CapturedSeconds),
             ComputeAverageFromInts(Samples),
				ComputeAverage(AvgFPS),
				ComputeAverage(MinFPS),
				ComputeAverage(AvgFrameMs),
				ComputeAverage(P95Ms),
				ComputeAverage(P99Ms),
				ComputeAverage(WorstFrameMs),
               ComputeAverageFromInts(Hitches16),
				ComputeAverageFromInts(Hitches33));

			MedianCsv += FString::Printf(
				TEXT("%s,%d,%.4f,%.2f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.2f,%.2f\n"),
				*ModelToString(Model),
				AgentCount,
				ComputeMedian(CapturedSeconds),
				ComputeMedianFromInts(Samples),
				ComputeMedian(AvgFPS),
				ComputeMedian(MinFPS),
				ComputeMedian(AvgFrameMs),
				ComputeMedian(P95Ms),
				ComputeMedian(P99Ms),
				ComputeMedian(WorstFrameMs),
				ComputeMedianFromInts(Hitches16),
				ComputeMedianFromInts(Hitches33));
		}
	}

 FString AverageOutputName = AverageCsvFileName;
	FString MedianOutputName = MedianCsvFileName;
	if (!bRunBothModels)
	{
		const FString ModelName = ModelToString(SingleMapModel);
		const FString AverageBase = FPaths::GetBaseFilename(AverageCsvFileName);
		const FString AverageExt = FPaths::GetExtension(AverageCsvFileName, true);
		AverageOutputName = FString::Printf(TEXT("%s_%s%s"), *AverageBase, *ModelName, *AverageExt);

		const FString MedianBase = FPaths::GetBaseFilename(MedianCsvFileName);
		const FString MedianExt = FPaths::GetExtension(MedianCsvFileName, true);
		MedianOutputName = FString::Printf(TEXT("%s_%s%s"), *MedianBase, *ModelName, *MedianExt);
	}

	const FString AveragePath = GetBenchmarkDir() / AverageOutputName;
	const FString MedianPath = GetBenchmarkDir() / MedianOutputName;
	FFileHelper::SaveStringToFile(AverageCsv, *AveragePath);
	FFileHelper::SaveStringToFile(MedianCsv, *MedianPath);

	UE_LOG(LogTemp, Log, TEXT("BenchmarkBatch: Wrote aggregate CSV files:\n  %s\n  %s"), *AveragePath, *MedianPath);
}

UBattlePerformanceMetricsWidget* ABenchmarkMetricsBatchController::FindMetricsWidget() const
{
	if (CachedMetricsWidget.IsValid())
	{
		return CachedMetricsWidget.Get();
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UBattlePerformanceMetricsWidget::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (UBattlePerformanceMetricsWidget* Metrics = Cast<UBattlePerformanceMetricsWidget>(Widget))
		{
			CachedMetricsWidget = Metrics;
			return Metrics;
		}
	}

	return nullptr;
}

bool ABenchmarkMetricsBatchController::ParseRunOptions(int32& OutModelIndex, int32& OutCountIndex, int32& OutRunIndex) const
{
 const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

  const FString BatchFlag = World->URL.GetOption(TEXT("BenchmarkBatch="), TEXT(""));
	if (BatchFlag != TEXT("1"))
	{
		return false;
	}

  OutModelIndex = FCString::Atoi(World->URL.GetOption(TEXT("BenchmarkModelIndex="), TEXT("0")));
	OutCountIndex = FCString::Atoi(World->URL.GetOption(TEXT("BenchmarkCountIndex="), TEXT("0")));
	OutRunIndex = FCString::Atoi(World->URL.GetOption(TEXT("BenchmarkRunIndex="), TEXT("1")));

	if (OutRunIndex <= 0)
	{
		OutRunIndex = 1;
	}

    if (OutModelIndex < 0 || OutModelIndex >= GetModelRunCount())
	{
		OutModelIndex = 0;
	}

	if (OutCountIndex < 0 || OutCountIndex >= AgentCounts.Num())
	{
		OutCountIndex = 0;
	}

	return true;
}

bool ABenchmarkMetricsBatchController::SaveRunState(const int32 InModelIndex, const int32 InCountIndex, const int32 InRunIndex, const bool bEnabled) const
{
  (void)InModelIndex;
	(void)InCountIndex;
	(void)InRunIndex;
	(void)bEnabled;
	return true;
}

FString ABenchmarkMetricsBatchController::GetBenchmarkDir() const
{
	return FPaths::ProjectSavedDir() / TEXT("Benchmark");
}

FString ABenchmarkMetricsBatchController::GetRawCsvPath() const
{
	return GetBenchmarkDir() / RawCsvFileName;
}

FString ABenchmarkMetricsBatchController::GetRunStatePath() const
{
	return GetBenchmarkDir() / RunStateFileName;
}

bool ABenchmarkMetricsBatchController::AppendRawRecord(const FBenchmarkRawRunRecord& Record) const
{
	const FString Header = TEXT("Model,Agents_nr,Run,Captured_s,Samples,AvgFPS,MinFPS,AvgFrame_ms,P95_ms,P99_ms,WorstFrame_ms,Hitches_gt_16_6ms,Hitches_gt_33_3ms\n");
	const FString Row = FString::Printf(
		TEXT("%s,%d,%d,%.6f,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%d\n"),
		*ModelToString(Record.Model),
		Record.AgentCount,
		Record.RunIndex,
		Record.Metrics.CapturedSeconds,
		Record.Metrics.Samples,
		Record.Metrics.AvgFPS,
		Record.Metrics.MinFPS,
		Record.Metrics.AvgFrameMs,
		Record.Metrics.P95Ms,
		Record.Metrics.P99Ms,
		Record.Metrics.WorstFrameMs,
		Record.Metrics.HitchesOver16_6Ms,
		Record.Metrics.HitchesOver33_3Ms);

	const FString RawPath = GetRawCsvPath();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*RawPath))
	{
		if (!FFileHelper::SaveStringToFile(Header, *RawPath))
		{
			return false;
		}
	}

	return FFileHelper::SaveStringToFile(Row, *RawPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

bool ABenchmarkMetricsBatchController::TryReadRawRecords(TArray<FBenchmarkRawRunRecord>& OutRecords) const
{
	OutRecords.Reset();

	FString RawText;
	if (!FFileHelper::LoadFileToString(RawText, *GetRawCsvPath()))
	{
		return false;
	}

	TArray<FString> Lines;
	RawText.ParseIntoArrayLines(Lines, true);
	for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
	{
		const FString& Line = Lines[LineIndex];
		TArray<FString> Cells;
		Line.ParseIntoArray(Cells, TEXT(","), true);
		if (Cells.Num() < 13)
		{
			continue;
		}

		FBenchmarkRawRunRecord Record;
		if (!TryParseModel(Cells[0], Record.Model))
		{
			continue;
		}

		Record.AgentCount = FCString::Atoi(*Cells[1]);
		Record.RunIndex = FCString::Atoi(*Cells[2]);
		Record.Metrics.CapturedSeconds = FCString::Atof(*Cells[3]);
		Record.Metrics.Samples = FCString::Atoi(*Cells[4]);
		Record.Metrics.AvgFPS = FCString::Atof(*Cells[5]);
		Record.Metrics.MinFPS = FCString::Atof(*Cells[6]);
		Record.Metrics.AvgFrameMs = FCString::Atof(*Cells[7]);
		Record.Metrics.P95Ms = FCString::Atof(*Cells[8]);
		Record.Metrics.P99Ms = FCString::Atof(*Cells[9]);
		Record.Metrics.WorstFrameMs = FCString::Atof(*Cells[10]);
		Record.Metrics.HitchesOver16_6Ms = FCString::Atoi(*Cells[11]);
		Record.Metrics.HitchesOver33_3Ms = FCString::Atoi(*Cells[12]);
		OutRecords.Add(Record);
	}

	return OutRecords.Num() > 0;
}

FString ABenchmarkMetricsBatchController::ModelToString(const EBenchmarkBatchModel Model)
{
	return (Model == EBenchmarkBatchModel::ECS) ? TEXT("ECS") : TEXT("OOP");
}

bool ABenchmarkMetricsBatchController::TryParseModel(const FString& ModelString, EBenchmarkBatchModel& OutModel)
{
	if (ModelString.Equals(TEXT("ECS"), ESearchCase::IgnoreCase))
	{
		OutModel = EBenchmarkBatchModel::ECS;
		return true;
	}
	if (ModelString.Equals(TEXT("OOP"), ESearchCase::IgnoreCase))
	{
		OutModel = EBenchmarkBatchModel::OOP;
		return true;
	}

	return false;
}

float ABenchmarkMetricsBatchController::ComputeAverage(const TArray<float>& Values)
{
	if (Values.Num() == 0)
	{
		return 0.0f;
	}

	double Sum = 0.0;
	for (const float Value : Values)
	{
		Sum += static_cast<double>(Value);
	}

	return static_cast<float>(Sum / static_cast<double>(Values.Num()));
}

float ABenchmarkMetricsBatchController::ComputeAverageFromInts(const TArray<int32>& Values)
{
	if (Values.Num() == 0)
	{
		return 0.0f;
	}

	double Sum = 0.0;
	for (const int32 Value : Values)
	{
		Sum += static_cast<double>(Value);
	}

	return static_cast<float>(Sum / static_cast<double>(Values.Num()));
}

float ABenchmarkMetricsBatchController::ComputeMedian(TArray<float> Values)
{
	if (Values.Num() == 0)
	{
		return 0.0f;
	}

	Values.Sort();
	const int32 MidIndex = Values.Num() / 2;
	if ((Values.Num() % 2) == 1)
	{
		return Values[MidIndex];
	}

	return 0.5f * (Values[MidIndex - 1] + Values[MidIndex]);
}

float ABenchmarkMetricsBatchController::ComputeMedianFromInts(const TArray<int32>& Values)
{
	TArray<float> AsFloats;
	AsFloats.Reserve(Values.Num());
	for (const int32 Value : Values)
	{
		AsFloats.Add(static_cast<float>(Value));
	}

	return ComputeMedian(MoveTemp(AsFloats));
}

int32 ABenchmarkMetricsBatchController::GetModelRunCount() const
{
	return bRunBothModels ? 2 : 1;
}

EBenchmarkBatchModel ABenchmarkMetricsBatchController::GetModelForIndex(const int32 ModelIndex) const
{
	if (!bRunBothModels)
	{
		return SingleMapModel;
	}

	return (ModelIndex == 0) ? EBenchmarkBatchModel::OOP : EBenchmarkBatchModel::ECS;
}
