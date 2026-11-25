// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StunnablePillar.generated.h"

class UStaticMeshComponent;

UCLASS()
class AKA_API AStunnablePillar : public AActor
{
	GENERATED_BODY()
	
public:	
	AStunnablePillar();

protected:
	virtual void BeginPlay() override;

public:	
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Settings")
	int32 MaxHits = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pillar Settings")
	float FallImpulseStrength = 500000.0f;

	// State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pillar Settings")
	int32 CurrentHits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pillar Settings")
	bool bIsBroken = false;

	// Function called by Boss
	UFUNCTION(BlueprintCallable, Category = "Pillar Logic")
	void TakeHitFromBoss(FVector BossForwardVector);
};
