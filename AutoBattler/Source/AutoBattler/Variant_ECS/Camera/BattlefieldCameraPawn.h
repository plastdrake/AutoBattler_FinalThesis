// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BattlefieldCameraPawn.generated.h"

class USceneComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class AUTOBATTLER_API ABattlefieldCameraPawn : public APawn
{
    GENERATED_BODY()

public:
    ABattlefieldCameraPawn();

protected:
    virtual void BeginPlay() override;

public:
    /** Root component */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootScene;

    /** Boom that positions camera above the battlefield */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* CameraBoom;

    /** Camera used for the static overhead view */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* Camera;

    /** Height of the camera above the pivot (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (Units = "cm"))
    float Height = 1500.0f;

    /** Pitch angle of the camera (degrees, negative looks down) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (Units = "degrees"))
    float PitchAngle = -60.0f;

    /** If true, the first player controller will be set to view this camera on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    bool bAutoSetViewTarget = true;

    /** Move the camera pawn to a new world location (keeps overhead orientation) */
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SnapTo(const FVector& WorldLocation);
};
