// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AutoBattlerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/** @brief Player-controllable third-person character with an orbit camera. */
UCLASS(abstract)
class AAutoBattlerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** @brief Camera boom positioning the camera behind the character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** @brief Follow camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** @brief Jump input action. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** @brief Move input action. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** @brief Look input action. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** @brief Mouse look input action. */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

public:

	/** @brief Default constructor. */
	AAutoBattlerCharacter();	

protected:

	/**
	 * @brief Initializes input action bindings.
	 * @param PlayerInputComponent Input component to bind actions on.
	 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/**
	 * @brief Handles raw movement input.
	 * @param Value Input action value carrying a movement vector.
	 */
	void Move(const FInputActionValue& Value);

	/**
	 * @brief Handles raw look input.
	 * @param Value Input action value carrying a look vector.
	 */
	void Look(const FInputActionValue& Value);

public:

	/**
	 * @brief Handles move input from either controls or UI interfaces.
	 * @param Right Right-axis input value.
	 * @param Forward Forward-axis input value.
	 */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/**
	 * @brief Handles look input from either controls or UI interfaces.
	 * @param Yaw Yaw delta to apply.
	 * @param Pitch Pitch delta to apply.
	 */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** @brief Handles jump-start input from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** @brief Handles jump-end input from either controls or UI interfaces. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/**
	 * @brief Returns the CameraBoom subobject.
	 * @return Camera boom component.
	 */
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/**
	 * @brief Returns the FollowCamera subobject.
	 * @return Follow camera component.
	 */
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

