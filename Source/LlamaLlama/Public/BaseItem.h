// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseItem.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class ALlamaLlamaCharacter;

UCLASS()
class LLAMALLAMA_API ABaseItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UStaticMeshComponent* meshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USphereComponent* sphereComp;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	ALlamaLlamaCharacter* carrier;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_OnPickUp(ACharacter* invoker);

	UFUNCTION()
	void OnPickUp(ACharacter* invoker);

	UFUNCTION(Server, reliable, WithValidation)
	void Server_OnPrimaryAction();

	UFUNCTION(BlueprintNativeEvent)
	void OnPrimaryAction();

	UFUNCTION(Server, reliable, WithValidation)
	void Server_OnSecondaryAction();

	UFUNCTION(BlueprintNativeEvent)
	void OnSecondaryAction();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
