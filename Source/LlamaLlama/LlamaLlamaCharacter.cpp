// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LlamaLlamaCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Net/UnrealNetwork.h"

#include "Public/BaseItem.h"
#include "Components/SphereComponent.h"

#include "Engine.h"

//////////////////////////////////////////////////////////////////////////
// ALlamaLlamaCharacter

ALlamaLlamaCharacter::ALlamaLlamaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 1.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	leftHandPushSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Left Hand Sphere"));
	leftHandPushSphere->SetSphereRadius(5.f);
	leftHandPushSphere->SetGenerateOverlapEvents(true);
	leftHandPushSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	leftHandPushSphere->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "push_socket_L");

	rightHandPushSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Right Hand Sphere"));
	rightHandPushSphere->SetSphereRadius(5.f);
	rightHandPushSphere->SetGenerateOverlapEvents(true);
	rightHandPushSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	rightHandPushSphere->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "push_socket_R");

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ALlamaLlamaCharacter::BeginPlay()
{
	Super::BeginPlay();

	leftHandPushSphere->OnComponentBeginOverlap.AddDynamic(this, &ALlamaLlamaCharacter::OnHandPushHit);
	rightHandPushSphere->OnComponentBeginOverlap.AddDynamic(this, &ALlamaLlamaCharacter::OnHandPushHit);
	//GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ALlamasAndRobotsCharacter::OnOverlapBegin);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ALlamaLlamaCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ALlamaLlamaCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ALlamaLlamaCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ALlamaLlamaCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ALlamaLlamaCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ALlamaLlamaCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ALlamaLlamaCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ALlamaLlamaCharacter::OnResetVR);

	PlayerInputComponent->BindAction("PickUp", IE_Pressed, this, &ALlamaLlamaCharacter::PickUp);
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &ALlamaLlamaCharacter::PrimaryAction);
	PlayerInputComponent->BindAction("SecondaryAction", IE_Pressed, this, &ALlamaLlamaCharacter::SecondaryAction);

}

void ALlamaLlamaCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ALlamaLlamaCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ALlamaLlamaCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ALlamaLlamaCharacter::OnHandPushHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != this)
	{
		if (ALlamaLlamaCharacter * llama = Cast<ALlamaLlamaCharacter>(OtherActor))
		{
			//llama->bStunned = true;
			StunOtherLlama(llama);
			//TODO add a stun function with a server_ function to set the value so it gets replicated to all 
			GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::White, "should stun the other llama");
		}
	}
}

bool ALlamaLlamaCharacter::Server_StunOtherLlama_Validate(ALlamaLlamaCharacter* otherLlama)
{
	return true;
}

void ALlamaLlamaCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ALlamaLlamaCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ALlamaLlamaCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ALlamaLlamaCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

bool ALlamaLlamaCharacter::Server_StunLlama_Validate()
{
	return true;
}

void ALlamaLlamaCharacter::Server_StunLlama_Implementation()
{
	StunLlama();
}

void ALlamaLlamaCharacter::StunLlama()
{
	if (Role < ROLE_Authority)
	{
		Server_StunLlama();
	}
	else if (Role == ROLE_Authority)
	{
		bStunned = true;
		if (item)
		{
			item->meshComp->SetSimulatePhysics(true);
			item->carrier = nullptr;
			item = nullptr;
		}
	}
}

void ALlamaLlamaCharacter::Server_StunOtherLlama_Implementation(ALlamaLlamaCharacter* otherLlama)
{
	StunOtherLlama(otherLlama);
}

void ALlamaLlamaCharacter::StunOtherLlama(ALlamaLlamaCharacter* otherLlama)
{
	if (Role < ROLE_Authority)
	{
		Server_StunOtherLlama(otherLlama);
	}
	else if (Role == ROLE_Authority)
	{
		//bStunned = true;
		if (bPushing)
		{
			//otherLlama->bStunned = true;
			otherLlama->StunLlama();
		}
	}
}

bool ALlamaLlamaCharacter::Server_OnPickUp_Validate()
{
	return true;
}

void ALlamaLlamaCharacter::Server_OnPickUp_Implementation()
{
	PickUp();
}

void ALlamaLlamaCharacter::PickUp()
{
	TArray<AActor*> actors;
	GetOverlappingActors(actors);

	if (this->item == nullptr)
	{
		//ABaseItem* closestItem;
		for (auto& actor : actors)
		{
			//if cast, it's an item you can pickup
			if (ABaseItem * overlappingItem = Cast<ABaseItem>(actor))
			{
				//FVector

				if (overlappingItem->carrier == nullptr)
				{
					if (Role < ROLE_Authority)
					{
						Server_OnPickUp();
					}
					else
					{
						this->item = overlappingItem;
						this->item->OnPickUp(this);
						OnRep_item();
						break;
					}
				}
			}
		}
	}
	else if (item)
	{
		if (Role < ROLE_Authority)
		{
			Server_OnPickUp();
		}
		else
		{
			//drop item
			item->meshComp->SetSimulatePhysics(true);
			//item->meshComp->AddImpulse(GetActorForwardVector() * 500);
			item->carrier = nullptr;
			item = nullptr;
		}
	}
}

void ALlamaLlamaCharacter::OnRep_item()
{
	if (item)
	{
		//there is an item being held
		//add move ignore 
		MoveIgnoreActorAdd(item);
	}
	else
	{
		//the item is null
		//remove all move ignore
		ClearComponentOverlaps();
		GetCapsuleComponent()->ClearMoveIgnoreActors();
	}
}

bool ALlamaLlamaCharacter::Server_PrimaryAction_Validate()
{
	return true;
}

void ALlamaLlamaCharacter::Server_PrimaryAction_Implementation()
{
	PrimaryAction();
}

void ALlamaLlamaCharacter::PrimaryAction()
{
	if (Role < ROLE_Authority)
	{
		Server_PrimaryAction();
		if (item)
		{

		}
		else
		{
			//bPushing = true;
		}
	}
	else
	{
		if (item)
		{
			item->OnPrimaryAction();
		}
		else
		{
			bPushing = true;
		}
	}
}

bool ALlamaLlamaCharacter::Server_SecondaryAction_Validate()
{
	return true;
}

void ALlamaLlamaCharacter::Server_SecondaryAction_Implementation()
{
	SecondaryAction();
}

void ALlamaLlamaCharacter::SecondaryAction()
{
	if (Role < ROLE_Authority)
	{
		Server_SecondaryAction();
	}
	else
	{
		if (item)
		{
			item->OnSecondaryAction();
		}
		else
		{

		}
	}
}

void ALlamaLlamaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALlamaLlamaCharacter, item);
	DOREPLIFETIME(ALlamaLlamaCharacter, bStunned);
	DOREPLIFETIME(ALlamaLlamaCharacter, bPushing);
}