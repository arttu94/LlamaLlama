// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/BaseItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "Net/UnrealNetwork.h"

#include "../LlamaLlamaCharacter.h"

// Sets default values
ABaseItem::ABaseItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	RootComponent = meshComp;

	sphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Component"));
	sphereComp->SetupAttachment(meshComp);
	sphereComp->SetGenerateOverlapEvents(true);

	meshComp->SetSimulatePhysics(true);
	meshComp->CanCharacterStepUp(false);

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ABaseItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ABaseItem::Server_OnPickUp_Validate(ACharacter* invoker)
{
	return true;
}

void ABaseItem::Server_OnPickUp_Implementation(ACharacter* invoker)
{
	OnPickUp(invoker);
}

void ABaseItem::OnPickUp(ACharacter* invoker)
{
	if (Role < ROLE_Authority)
	{
		//is client
		Server_OnPickUp(invoker);
	}
	else if (Role == ROLE_Authority)
	{
		//is server
		meshComp->SetSimulatePhysics(false);
		AttachToComponent(invoker->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("item_socket_R"));
		carrier = Cast<ALlamaLlamaCharacter>(invoker);
	}
}

bool ABaseItem::Server_OnPrimaryAction_Validate()
{
	return true;
}

void ABaseItem::Server_OnPrimaryAction_Implementation()
{
	OnPrimaryAction();
}

void ABaseItem::OnPrimaryAction_Implementation()
{
	if (Role < ROLE_Authority)
	{
		Server_OnPrimaryAction();
	}
	else if (Role == ROLE_Authority)
	{
		//
	}
}

bool ABaseItem::Server_OnSecondaryAction_Validate()
{
	return true;
}

void ABaseItem::Server_OnSecondaryAction_Implementation()
{
	OnSecondaryAction();
}

void ABaseItem::OnSecondaryAction_Implementation()
{
	if (Role < ROLE_Authority)
	{
		Server_OnSecondaryAction();
	}
	else if (Role == ROLE_Authority)
	{
		//
	}
}

void ABaseItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseItem, carrier);
}