// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoBattlerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AutoBattler.h"

AAutoBattlerCharacter::AAutoBattlerCharacter()
{
	/** Set the collision capsule size. */
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	/** Keep controller rotation from affecting the character directly. */
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	/** Configure character movement defaults. */
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	/** These movement values can be tuned in the Blueprint without recompiling. */
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	/** Create a camera boom that pulls in toward the player on collision. */
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	/** Create a follow camera. */
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	/** The mesh and animation blueprint are assigned in the derived Blueprint asset. */
}

void AAutoBattlerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	/** Set up action bindings. */
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		/** Jumping. */
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		/** Moving. */
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAutoBattlerCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAutoBattlerCharacter::Look);

		/** Looking. */
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAutoBattlerCharacter::Look);
	}
	else
	{
		UE_LOG(LogAutoBattler, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAutoBattlerCharacter::Move(const FInputActionValue& Value)
{
	/** Input is a Vector2D. */
	FVector2D MovementVector = Value.Get<FVector2D>();

	/** Route the input. */
	DoMove(MovementVector.X, MovementVector.Y);
}

void AAutoBattlerCharacter::Look(const FInputActionValue& Value)
{
	/** Input is a Vector2D. */
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	/** Route the input. */
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AAutoBattlerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		/** Find out which way is forward. */
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		/** Get the forward vector. */
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		/** Get the right vector. */
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		/** Add movement input. */
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AAutoBattlerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		/** Add yaw and pitch input to the controller. */
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AAutoBattlerCharacter::DoJumpStart()
{
	/** Signal the character to jump. */
	Jump();
}

void AAutoBattlerCharacter::DoJumpEnd()
{
	/** Signal the character to stop jumping. */
	StopJumping();
}
