#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Variant_BattleSim/UI/BattlePerformanceMetricsWidget.h"
#include "BenchmarkMetricsBatchController.generated.h"

class UBattlePerformanceMetricsWidget;

/** @brief Model type used for benchmark batch runs. */
UENUM()
enum class EBenchmarkBatchModel : uint8
{
	OOP,
	ECS
};

/** @brief Raw record describing a single benchmark run. */
USTRUCT()
struct FBenchmarkRawRunRecord
{
	GENERATED_BODY()

	/** @brief Model type for the run. */
	EBenchmarkBatchModel Model = EBenchmarkBatchModel::OOP;
	/** @brief Total agent count for the run. */
	int32 AgentCount = 0;
	/** @brief Run index for the given model/count combination. */
	int32 RunIndex = 0;
	/** @brief Captured performance metrics for the run. */
	FBattlePerformanceMetricsSnapshot Metrics;
};

/** @brief Controller that runs a batch of benchmark captures and writes CSV output. */
UCLASS()
class AUTOBATTLER_API ABenchmarkMetricsBatchController : public AActor
{
	GENERATED_BODY()

public:
	/** @brief Default constructor. */
	ABenchmarkMetricsBatchController();

protected:
	/** @brief Initializes batch run state. */
	virtual void BeginPlay() override;

private:
	/** @brief Restarts the level to kick off a batch run. */
	void StartBatchByReloadingLevel();
	/** @brief Starts a configured run based on URL options. */
	void StartConfiguredRunFromOptions();
	/** @brief Polls for run completion and advances when done. */
	void PollForRunCompletion();
	/**
	 * @brief Records a completed run and advances indices.
	 * @param Metrics Finalized metrics snapshot for the run.
	 */
	void RecordCompletedRunAndAdvance(const FBattlePerformanceMetricsSnapshot& Metrics);
	/**
	 * @brief Opens the level for a specific run configuration.
	 * @param InModelIndex Model index.
	 * @param InCountIndex Agent count index.
	 * @param InRunIndex Run index.
	 */
	void OpenLevelForRun(int32 InModelIndex, int32 InCountIndex, int32 InRunIndex) const;
	/** @brief Writes aggregate CSV files after the batch completes. */
	void FinalizeAndWriteAggregateCsvs();
	/**
	 * @brief Returns the run count per model.
	 * @return Run count per model.
	 */
	int32 GetModelRunCount() const;
	/**
	 * @brief Resolves the model for a given index.
	 * @param ModelIndex Index into model list.
	 * @return Model enum value.
	 */
	EBenchmarkBatchModel GetModelForIndex(int32 ModelIndex) const;

	/**
	 * @brief Finds the metrics widget in the current world.
	 * @return Metrics widget instance if found.
	 */
	UBattlePerformanceMetricsWidget* FindMetricsWidget() const;
	/**
	 * @brief Parses run options from URL parameters.
	 * @param OutModelIndex Output model index.
	 * @param OutCountIndex Output agent count index.
	 * @param OutRunIndex Output run index.
	 * @return True if options were parsed successfully.
	 */
	bool ParseRunOptions(int32& OutModelIndex, int32& OutCountIndex, int32& OutRunIndex) const;
	/**
	 * @brief Saves the current run state to disk.
	 * @param InModelIndex Model index.
	 * @param InCountIndex Agent count index.
	 * @param InRunIndex Run index.
	 * @param bEnabled True if the run is enabled.
	 * @return True if save succeeded.
	 */
	bool SaveRunState(int32 InModelIndex, int32 InCountIndex, int32 InRunIndex, bool bEnabled) const;

	/** @brief Returns the benchmark directory path. */
	FString GetBenchmarkDir() const;
	/** @brief Returns the raw CSV file path. */
	FString GetRawCsvPath() const;
	/** @brief Returns the run state file path. */
	FString GetRunStatePath() const;
	/**
	 * @brief Appends a raw run record to the CSV file.
	 * @param Record Record to append.
	 * @return True if append succeeded.
	 */
	bool AppendRawRecord(const FBenchmarkRawRunRecord& Record) const;
	/**
	 * @brief Reads raw records from disk.
	 * @param OutRecords Output record list.
	 * @return True if records were read successfully.
	 */
	bool TryReadRawRecords(TArray<FBenchmarkRawRunRecord>& OutRecords) const;

	/**
	 * @brief Converts a model enum to a string.
	 * @param Model Model enum value.
	 * @return Model string.
	 */
	static FString ModelToString(EBenchmarkBatchModel Model);
	/**
	 * @brief Attempts to parse a model string.
	 * @param ModelString Input string.
	 * @param OutModel Output model enum.
	 * @return True if parsing succeeded.
	 */
	static bool TryParseModel(const FString& ModelString, EBenchmarkBatchModel& OutModel);
	/**
	 * @brief Computes the average of float values.
	 * @param Values Input values.
	 * @return Average value.
	 */
	static float ComputeAverage(const TArray<float>& Values);
	/**
	 * @brief Computes the average of int values.
	 * @param Values Input values.
	 * @return Average value.
	 */
	static float ComputeAverageFromInts(const TArray<int32>& Values);
	/**
	 * @brief Computes the median of float values.
	 * @param Values Input values (copied by value for sorting).
	 * @return Median value.
	 */
	static float ComputeMedian(TArray<float> Values);
	/**
	 * @brief Computes the median of int values.
	 * @param Values Input values (copied by value for sorting).
	 * @return Median value.
	 */
	static float ComputeMedianFromInts(const TArray<int32>& Values);

	/** @brief If true, starts the batch automatically. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	bool bAutoStartBatch = true;

	/** @brief If true, runs both models for each agent count. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	bool bRunBothModels = false;

	/** @brief Single model to run when bRunBothModels is false. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch", meta = (EditCondition = "!bRunBothModels"))
	EBenchmarkBatchModel SingleMapModel = EBenchmarkBatchModel::OOP;

	/** @brief Runs per agent count per model. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch", meta = (ClampMin = 1))
	int32 RunsPerAgentCount = 10;

	/** @brief Agent counts to run in the batch. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	TArray<int32> AgentCounts = { 10, 20, 40, 80, 160, 320, 640 };

	/** @brief Poll interval for run completion checks. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch", meta = (ClampMin = 0.05, Units = "s"))
	float CompletionPollInterval = 0.2f;

	/** @brief Raw CSV output file name. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString RawCsvFileName = TEXT("BattleBenchmark_Raw.csv");

	/** @brief Average CSV output file name. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString AverageCsvFileName = TEXT("BattleBenchmark_Average.csv");

	/** @brief Median CSV output file name. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString MedianCsvFileName = TEXT("BattleBenchmark_Median.csv");

	/** @brief Run state file name. */
	UPROPERTY(EditAnywhere, Category = "Benchmark Batch")
	FString RunStateFileName = TEXT("BattleBenchmark_RunState.ini");

	/** @brief Timer handle for completion polling. */
	FTimerHandle CompletionPollTimerHandle;
	/** @brief Cached metrics widget pointer. */
	mutable TWeakObjectPtr<UBattlePerformanceMetricsWidget> CachedMetricsWidget;
	/** @brief True once a run has been recorded. */
	bool bRunRecorded = false;

	/** @brief Current model index within the batch. */
	int32 CurrentModelIndex = 0;
	/** @brief Current agent count index within the batch. */
	int32 CurrentCountIndex = 0;
	/** @brief Current run index for the active model/count. */
	int32 CurrentRunIndex = 1;
};
