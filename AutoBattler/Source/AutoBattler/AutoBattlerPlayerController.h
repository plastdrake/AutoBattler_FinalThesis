// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AutoBattlerPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/** @brief Basic player controller for the third-person template. */
UCLASS(abstract)
class AAutoBattlerPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** @brief Default input mapping contexts. */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** @brief Input mapping contexts excluded when touch controls are active. */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** @brief Mobile controls widget to spawn. */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** @brief Pointer to the mobile controls widget. */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** @brief If true, the player uses UMG touch controls even outside mobile platforms. */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** @brief Gameplay initialization. */
	virtual void BeginPlay() override;

	/** @brief Input mapping context setup. */
	virtual void SetupInputComponent() override;

	/**
	 * @brief Returns true if the player should use UMG touch controls.
	 * @return True when touch controls should be active.
	 */
	bool ShouldUseTouchControls() const;

};
