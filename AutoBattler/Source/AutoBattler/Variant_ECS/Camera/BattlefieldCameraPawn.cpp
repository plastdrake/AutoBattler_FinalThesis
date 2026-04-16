// Copyright Epic Games, Inc. All Rights Reserved.

#include "BattlefieldCameraPawn.h"

#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ABattlefieldCameraPawn::ABattlefieldCameraPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(RootScene);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootScene);
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->TargetArmLength = Height;
    CameraBoom->SetRelativeRotation(FRotator(PitchAngle, 0.0f, 0.0f));

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ABattlefieldCameraPawn::BeginPlay()
{
    Super::BeginPlay();

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = Height;
        CameraBoom->SetRelativeRotation(FRotator(PitchAngle, 0.0f, 0.0f));
    }

    if (bAutoSetViewTarget && GetWorld())
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->SetViewTarget(this);
        }
    }
}

void ABattlefieldCameraPawn::SnapTo(const FVector& WorldLocation)
{
    SetActorLocation(WorldLocation);
}
