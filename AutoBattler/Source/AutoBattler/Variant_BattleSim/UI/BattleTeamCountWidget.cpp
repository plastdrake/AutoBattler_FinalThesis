#include "BattleTeamCountWidget.h"

#include "EngineUtils.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "Variant_BattleSim/AI/OOPBattleAgent.h"
#include "Variant_ECS/AI/ECSBattleAgentVisual.h"

void UBattleTeamCountWidget::NativeConstruct()
{
	Super::NativeConstruct();
    UpdateCountsFromWorld();
	RefreshTeamCountTexts();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoRefreshTimerHandle,
			this,
			&UBattleTeamCountWidget::UpdateCountsFromWorld,
			FMath::Max(0.05f, AutoRefreshInterval),
			true);
	}
}

void UBattleTeamCountWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoRefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UBattleTeamCountWidget::SetTeamCounts(const int32 RedCount, const int32 BlueCount)
{
	CachedRedTeamCount = RedCount;
	CachedBlueTeamCount = BlueCount;
	RefreshTeamCountTexts();
}

void UBattleTeamCountWidget::SetRedTeamCount(const int32 RedCount)
{
	CachedRedTeamCount = RedCount;
	RefreshTeamCountTexts();
}

void UBattleTeamCountWidget::SetBlueTeamCount(const int32 BlueCount)
{
	CachedBlueTeamCount = BlueCount;
	RefreshTeamCountTexts();
}

void UBattleTeamCountWidget::RefreshTeamCountTexts() const
{
	if (redteamcounttext)
	{
		redteamcounttext->SetText(FText::AsNumber(CachedRedTeamCount));
	}

	if (blueteamcounttext)
	{
		blueteamcounttext->SetText(FText::AsNumber(CachedBlueTeamCount));
	}
}

void UBattleTeamCountWidget::UpdateCountsFromWorld()
{
 int32 RedCount = 0;
	int32 BlueCount = 0;

	switch (CountSourceMode)
	{
	case EBattleTeamCountSourceMode::OOPAgents:
		GetOOPCounts(RedCount, BlueCount);
		break;

	case EBattleTeamCountSourceMode::ECSVisuals:
		if (!GetECSCounts(RedCount, BlueCount))
		{
			GetOOPCounts(RedCount, BlueCount);
		}
		break;

	case EBattleTeamCountSourceMode::AutoDetect:
	default:
	{
		GetOOPCounts(RedCount, BlueCount);
		if (RedCount == 0 && BlueCount == 0)
		{
			GetECSCounts(RedCount, BlueCount);
		}
		break;
	}
	}

	SetTeamCounts(RedCount, BlueCount);
}

void UBattleTeamCountWidget::GetOOPCounts(int32& OutRedCount, int32& OutBlueCount) const
{
	OutRedCount = AOOPBattleAgent::GetAliveTeamCount(EBattleAgentTeam::Red);
	OutBlueCount = AOOPBattleAgent::GetAliveTeamCount(EBattleAgentTeam::Blue);
}

bool UBattleTeamCountWidget::GetECSCounts(int32& OutRedCount, int32& OutBlueCount) const
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
