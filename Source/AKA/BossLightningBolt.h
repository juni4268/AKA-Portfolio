// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossLightningBolt.generated.h"

UCLASS()
class AKA_API ABossLightningBolt : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABossLightningBolt();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UProjectileMovementComponent* movementComp;
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	class UBoxComponent* collisionComp;
	UPROPERTY(VisibleAnywhere, Category = "BodyMesh")
	class UStaticMeshComponent* LightningBoltMesh;
	UPROPERTY(EditAnywhere, Category = "Effect")
	class UNiagaraSystem* impactEffect;
	UPROPERTY(EditAnywhere, Category = "Effect")
	class UNiagaraSystem* tailEffect;
	
	float Damage = 10.0f;

	void Die();
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void SetVelocity(FVector startPos, FVector targetPos);
	void Initialize(int32 Phase, float NewDamage, float NewSpeed);

	void EnableCollision();
};
