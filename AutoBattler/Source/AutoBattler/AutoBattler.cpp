// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoBattler.h"
#include "MassEntitySettings.h"
#include "Avoidance/MassAvoidanceProcessors.h"
#include "MassNavigationProcessors.h"
#include "Movement/MassMovementProcessors.h"
#include "Steering/MassSteeringProcessors.h"
#include "Modules/ModuleManager.h"
#include "Variant_ECS/AI/ECSBattleAgentProcessor.h"
#include "Variant_ECS/AI/ECSBattleAgentVisualizationProcessor.h"

class FAutoBattlerModule : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();

        UMassEntitySettings::GetOnInitializedEvent().AddRaw(this, &FAutoBattlerModule::RegisterBattleSimMassProcessors);
        RegisterBattleSimMassProcessors();
    }

    virtual void ShutdownModule() override
    {
       UMassEntitySettings::GetOnInitializedEvent().RemoveAll(this);

        FDefaultGameModuleImpl::ShutdownModule();
    }

private:
    void RegisterBattleSimMassProcessors()
    {
        UMassEntitySettings* Settings = GetMutableDefault<UMassEntitySettings>();
        if (!Settings)
        {
            return;
        }

        Settings->AddToActiveProcessorsList(UECSBattleAgentProcessor::StaticClass());
        Settings->AddToActiveProcessorsList(UECSBattleAgentVisualizationProcessor::StaticClass());
        Settings->AddToActiveProcessorsList(UMassMoveTargetFragmentInitializer::StaticClass());
        Settings->AddToActiveProcessorsList(UMassNavigationObstacleGridProcessor::StaticClass());
      Settings->AddToActiveProcessorsList(UMassSteerToMoveTargetProcessor::StaticClass());
        Settings->AddToActiveProcessorsList(UMassMovingAvoidanceProcessor::StaticClass());
       Settings->AddToActiveProcessorsList(UMassStandingAvoidanceProcessor::StaticClass());
        Settings->AddToActiveProcessorsList(UMassApplyForceProcessor::StaticClass());
        Settings->AddToActiveProcessorsList(UMassApplyMovementProcessor::StaticClass());
        Settings->BuildProcessorListAndPhases();

        UE_LOG(LogAutoBattler, Log, TEXT("Registered ECS battle processors in Mass settings"));
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FAutoBattlerModule, AutoBattler, "AutoBattler");

DEFINE_LOG_CATEGORY(LogAutoBattler)