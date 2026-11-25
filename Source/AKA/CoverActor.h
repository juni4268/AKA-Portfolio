// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverActor.generated.h"

UCLASS()
class AKA_API ACoverActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACoverActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
	float maxHP = 50;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float currentHP;
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	class UBoxComponent* collisionComp;

	UPROPERTY(VisibleAnywhere, Category = "BodyMesh")
	class UStaticMeshComponent* CoverActorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundWave* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundWave* BreakSound;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

};
