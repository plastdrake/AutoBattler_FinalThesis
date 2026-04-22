#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Variant_BattleSim/UI/BattlePerformanceMetricsWidget.h"
#include "BenchmarkMetricsBatchController.generated.h"

class UBattlePerformanceMetricsWidget;

UENUM()
enum class EBenchmarkBatchModel : uint8
{
	OOP,
	ECS
};

USTRUCT()
struct FBenchmarkRawRunRecord
{
	GENERATED_BODY()

	EBenchmarkBatchModel Model = EBenchmarkBatchModel::OOP;
	int32 AgentCount = 0;
	int32 RunIndex = 0;
	FBattlePerformanceMetricsSnapshot Metrics;
};

UCLASS()
class AUTOBATTLER_API ABenchmarkMetricsBatchController : public AActor
{
	GENERATED_BODY()

public:
	ABenchmarkMetricsBatchController();

protected:
	virtual void BeginPlay() override;

private:
	void StartBatchByReloadingLevel();
	void StartConfiguredRunFromOptions();
	void PollForRunCompletion();
	void RecordCompletedRunAndAdvance(const FBattlePerformanceMetricsSnapshot& Metrics);
	void OpenLevelForRun(int32 InModelIndex, int32 InCountIndex, int32 InRunIndex) const;
	void FinalizeAndWriteAggregateCsvs();
	int32 GetModelRunCount() const;
	EBenchmarkBatchModel GetModelForIndex(int32 ModelIndex) const;

	UBattlePerformanceMetricsWidget* FindMetricsWidget() const;
	bool ParseRunOptions(int32& OutModelIndex, int32& OutCountIndex, int32& OutRunIndex) const;
	bool SaveRunState(int32 InModelIndex, int32 InCountIndex, int32 InRunIndex, bool bEnabled) const;

	FString GetBenchmarkDir() const;
	FString GetRawCsvPath() const;
   FString GetRunStatePath() const;
	bool AppendRawRecord(const FBenchmarkRawRunRecord& Record) const;
	bool TryReadRawRecords(TArray<FBenchmarkRawRunRecord>& OutRecords) const;

	static FString ModelToString(EBenchmarkBatchModel Model);
	static bool TryParseModel(const FString& ModelString, EBenchmarkBatchModel& OutModel);
	static float ComputeAverage(const TArray<float>& Values);
   static float ComputeAverageFromInts(const TArray<int32>& Values);
	static float ComputeMedian(TArray<float> Values);
	static float ComputeMedianFromInts(const TArray<int32>& Values);

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	bool bAutoStartBatch = true;

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	bool bRunBothModels = false;

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch", meta = (EditCondition = "!bRunBothModels"))
	EBenchmarkBatchModel SingleMapModel = EBenchmarkBatchModel::OOP;

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch", meta = (ClampMin = 1))
	int32 RunsPerAgentCount = 10;

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	TArray<int32> AgentCounts = { 10, 20, 40, 80, 160, 320, 640 };

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch", meta = (ClampMin = 0.05, Units = "s"))
	float CompletionPollInterval = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString RawCsvFileName = TEXT("BattleBenchmark_Raw.csv");

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString AverageCsvFileName = TEXT("BattleBenchmark_Average.csv");

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString MedianCsvFileName = TEXT("BattleBenchmark_Median.csv");

	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString RunStateFileName = TEXT("BattleBenchmark_RunState.ini");

	FTimerHandle CompletionPollTimerHandle;
	mutable TWeakObjectPtr<UBattlePerformanceMetricsWidget> CachedMetricsWidget;
	bool bRunRecorded = false;

	int32 CurrentModelIndex = 0;
	int32 CurrentCountIndex = 0;
	int32 CurrentRunIndex = 1;
};
