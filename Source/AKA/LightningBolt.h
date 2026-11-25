// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightningBolt.generated.h"

UCLASS()
class AKA_API ALightningBolt : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALightningBolt();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UProjectileMovementComponent* movementComp;
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	class UBoxComponent* BoxComp;
	UPROPERTY(VisibleAnywhere, Category = "BodyMesh")
	class UStaticMeshComponent* LightningBoltMesh;
	UPROPERTY(EditAnywhere, Category = "Effect")
	class UNiagaraSystem* impactEffect;
	UPROPERTY(EditAnywhere, Category = "Effect")
	class UNiagaraSystem* tailEffect;
	void Die();
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult);
	void SetVelocity(FVector startPos, FVector targetPos);
};
