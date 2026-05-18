// Copyright Epic Games, Inc. All Rights Reserved.


#include "AutoBattlerPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "AutoBattler.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AAutoBattlerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	/** Only spawn touch controls on local player controllers. */
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		/** Spawn the mobile controls widget. */
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			/** Add the controls to the player screen. */
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogAutoBattler, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AAutoBattlerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	/** Only add input mapping contexts for local player controllers. */
	if (IsLocalPlayerController())
	{
		/** Add input mapping contexts. */
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			/** Only add these input mapping contexts when touch input is disabled. */
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AAutoBattlerPlayerController::ShouldUseTouchControls() const
{
	/** Use touch controls on mobile platforms or when forced by config. */
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
