// Fill out your copyright notice in the Description page of Project Settings.


#include "StunnablePillar.h"
#include "Components/StaticMeshComponent.h"

AStunnablePillar::AStunnablePillar()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// Default settings for a pillar (Moved back to Constructor)
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));
	MeshComp->SetSimulatePhysics(false); // Static by default

	// Add friction/damping to stop it from sliding too much after falling
	// MeshComp->SetLinearDamping(2.0f);  // Air resistance (stops sliding)
	// MeshComp->SetAngularDamping(2.0f); // Rotational resistance (stops rolling)
	// MeshComp->SetMassOverrideInKg(NAME_None, 5000.0f, true); // Make it heavy! (5 tons)
	
	// Add the tag so the boss recognizes it
	Tags.Add(TEXT("StunnableWall"));
}

void AStunnablePillar::BeginPlay()
{
	Super::BeginPlay();

	// Moved physics settings to BeginPlay to avoid GEngine not initialized error in Constructor
	if (MeshComp)
	{
		MeshComp->SetLinearDamping(2.0f);
		MeshComp->SetAngularDamping(2.0f);
		MeshComp->SetMassOverrideInKg(NAME_None, 5000.0f, true);
	}

	CurrentHits = 0;
	bIsBroken = false;
}

void AStunnablePillar::TakeHitFromBoss(FVector BossForwardVector)
{
	if (bIsBroken) return;

	CurrentHits++;
	UE_LOG(LogTemp, Warning, TEXT("Pillar Hit! %d / %d"), CurrentHits, MaxHits);

	if (CurrentHits >= MaxHits)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pillar Broken! Physics Enabled."));
		bIsBroken = true;

		// Enable Physics to make it fall
		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetCollisionProfileName(TEXT("PhysicsActor")); // Change to physics-enabled profile

		// Apply a push in the direction the boss was dashing
		// Increased strength significantly
		FVector Impulse = BossForwardVector * (FallImpulseStrength * 5.0f); 
		
		// Apply slightly upwards so it tips over nicely
		Impulse.Z += (FallImpulseStrength * 2.0f); 

		MeshComp->AddImpulse(Impulse);
		
		// Optional: Remove the tag so the boss doesn't get stunned by the debris on the floor
		Tags.Remove(TEXT("StunnableWall"));

		// Freeze physics after a few seconds to stop sliding/jittering
		FTimerHandle FreezeTimer;
		GetWorld()->GetTimerManager().SetTimer(FreezeTimer, [this]()
		{
			if (MeshComp)
			{
				MeshComp->SetSimulatePhysics(false);
				MeshComp->SetCollisionProfileName(TEXT("BlockAll")); // Or keep as PhysicsActor but static, or BlockAll to be a solid obstacle
				UE_LOG(LogTemp, Warning, TEXT("Pillar Physics Frozen."));
			}
		}, 2.0f, false);
	}
}
