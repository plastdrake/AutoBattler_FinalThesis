// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoBattler.h"
#include "MassEntitySettings.h"
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
        Settings->BuildProcessorListAndPhases();

        UE_LOG(LogAutoBattler, Log, TEXT("Registered ECS battle processors in Mass settings"));
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FAutoBattlerModule, AutoBattler, "AutoBattler");

DEFINE_LOG_CATEGORY(LogAutoBattler)