#include "ECSBattleAgentVisualizationProcessor.h"

#include "ECSBattleAgentFragments.h"
#include "ECSBattleAgentVisual.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassExecutionContext.h"

UECSBattleAgentVisualizationProcessor::UECSBattleAgentVisualizationProcessor()
	: AgentQuery(*this)
{
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::All);
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
}

void UECSBattleAgentVisualizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AgentQuery.AddRequirement<FECSBattleAgentFragment>(EMassFragmentAccess::ReadWrite);
	AgentQuery.AddRequirement<FECSBattleAgentRepresentationFragment>(EMassFragmentAccess::ReadWrite);
   AgentQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	AgentQuery.RequireMutatingWorldAccess();
	AgentQuery.RegisterWithProcessor(*this);
}

void UECSBattleAgentVisualizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = FMath::Max(Context.GetDeltaTimeSeconds(), KINDA_SMALL_NUMBER);

  AgentQuery.ForEachEntityChunk(Context, [this, &EntityManager, DeltaTime](FMassExecutionContext& QueryContext)
	{
		UWorld* World = QueryContext.GetWorld();
		if (!World)
		{
			return;
		}

		TArrayView<FECSBattleAgentFragment> AgentFragments = QueryContext.GetMutableFragmentView<FECSBattleAgentFragment>();
		TArrayView<FECSBattleAgentRepresentationFragment> RepresentationFragments = QueryContext.GetMutableFragmentView<FECSBattleAgentRepresentationFragment>();
      TArrayView<FTransformFragment> TransformFragments = QueryContext.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FMassEntityHandle> Entities = QueryContext.GetEntities();

		for (int32 EntityIndex = 0; EntityIndex < QueryContext.GetNumEntities(); ++EntityIndex)
		{
			FECSBattleAgentFragment& AgentData = AgentFragments[EntityIndex];
			FECSBattleAgentRepresentationFragment& RepresentationData = RepresentationFragments[EntityIndex];
			const FTransform& EntityTransform = TransformFragments[EntityIndex].GetTransform();
			const FMassEntityHandle Entity = Entities[EntityIndex];
			ACharacter* VisualCharacter = nullptr;

			if (const TWeakObjectPtr<ACharacter>* FoundVisual = VisualsByEntity.Find(Entity))
			{
				VisualCharacter = FoundVisual->Get();
			}

			if (AgentData.bPendingEntityDestroy)
			{
                if (IsValid(VisualCharacter))
				{
                  VisualCharacter->Destroy();
				}

				VisualsByEntity.Remove(Entity);

              QueryContext.Defer().DestroyEntity(Entity);
				AgentData.bPendingEntityDestroy = false;
				continue;
			}

            if (!IsValid(VisualCharacter) && RepresentationData.VisualCharacterClassAddress != 0)
			{
              UClass* VisualCharacterClass = reinterpret_cast<UClass*>(RepresentationData.VisualCharacterClassAddress);
				if (!VisualCharacterClass || !VisualCharacterClass->IsChildOf(ACharacter::StaticClass()))
				{
					continue;
				}

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                VisualCharacter = World->SpawnActor<ACharacter>(VisualCharacterClass, EntityTransform, SpawnParams);
				VisualsByEntity.FindOrAdd(Entity) = VisualCharacter;

             if (AECSBattleAgentVisual* TypedVisual = Cast<AECSBattleAgentVisual>(VisualCharacter))
				{
					TypedVisual->OnTeamAssigned(AgentData.Team);
				}
			}

            if (IsValid(VisualCharacter))
			{
				const float HalfHeight = VisualCharacter->GetCapsuleComponent() ? VisualCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.0f;
				FTransform AdjustedTransform = EntityTransform;
				AdjustedTransform.AddToTranslation(FVector(0.0f, 0.0f, HalfHeight));

				if (AECSBattleAgentVisual* TypedVisual = Cast<AECSBattleAgentVisual>(VisualCharacter))
				{
					TypedVisual->SyncFromMassTransform(AdjustedTransform, DeltaTime);
				}
				else
				{
                  VisualCharacter->SetActorLocation(AdjustedTransform.GetLocation(), true, nullptr, ETeleportType::None);
					VisualCharacter->SetActorRotation(AdjustedTransform.GetRotation(), ETeleportType::None);
				}

				FTransform SyncedTransform = VisualCharacter->GetActorTransform();
				SyncedTransform.AddToTranslation(FVector(0.0f, 0.0f, -HalfHeight));
				TransformFragments[EntityIndex].SetTransform(SyncedTransform);

				if (AgentData.bTriggerAttackMontage)
				{
                   if (AECSBattleAgentVisual* TypedVisual = Cast<AECSBattleAgentVisual>(VisualCharacter))
					{
						TypedVisual->PlayAttackMontage();
					}
				}
			}
		}
	});
}
