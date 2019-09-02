// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LlamaLlamaCharacter.generated.h"

class ABaseItem;
class USphereComponent;

UCLASS(config=Game)
class ALlamaLlamaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ALlamaLlamaCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	virtual void BeginPlay() override;

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	UFUNCTION(Server, reliable, WithValidation)
	void Server_OnPickUp();

	UFUNCTION()
	void PickUp();

	UPROPERTY(ReplicatedUsing=OnRep_item, VisibleAnywhere, BlueprintReadWrite)
	ABaseItem* item;

	UFUNCTION(BlueprintCallable)
	void OnRep_item();

	UFUNCTION(Server, reliable, WithValidation)
	void Server_PrimaryAction();

	UFUNCTION()
	void PrimaryAction();

	UFUNCTION(Server, reliable, WithValidation)
	void Server_SecondaryAction();

	UFUNCTION()
	void SecondaryAction();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool bStunned;

	UFUNCTION()
	void StunOtherLlama(ALlamaLlamaCharacter* otherLlama);

	UFUNCTION(Server, reliable, WithValidation)
	void Server_StunOtherLlama(ALlamaLlamaCharacter* otherLlama);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool bPushing;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
	USphereComponent* leftHandPushSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
	USphereComponent* rightHandPushSphere;

	UFUNCTION()
	void OnHandPushHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

