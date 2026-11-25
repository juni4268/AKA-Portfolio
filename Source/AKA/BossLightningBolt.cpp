// Fill out your copyright notice in the Description page of Project Settings.


#include "BossLightningBolt.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "BossMonster.h"
#include "AkaPlayer.h"
#include "LightningBolt.h"
#include "CoverActor.h"


// Sets default values
ABossLightningBolt::ABossLightningBolt()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	collisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	RootComponent = collisionComp;
	LightningBoltMesh = CreateDefaultSubobject< UStaticMeshComponent>(TEXT("LightningMeshComp"));
	LightningBoltMesh->SetupAttachment(collisionComp);

	movementComp = CreateDefaultSubobject< UProjectileMovementComponent>(TEXT("MovementComp"));
	movementComp->SetUpdatedComponent(collisionComp);
	movementComp->InitialSpeed = 950;
	movementComp->MaxSpeed = 950;
	movementComp->bShouldBounce = false;
	movementComp->Bounciness = 0.3f;
	movementComp->ProjectileGravityScale = 0.0f;
}

// Called when the game starts or when spawned
void ABossLightningBolt::BeginPlay()
{
	Super::BeginPlay();

	if (LightningBoltMesh)
	{
		LightningBoltMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}

	// Initially ignore walls to prevent immediate destruction if spawned inside/near a wall
	if (collisionComp)
	{
		collisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	}

	// Re-enable wall collision after a short delay
	FTimerHandle CollisionActivationTimer;
	GetWorld()->GetTimerManager().SetTimer(CollisionActivationTimer, this, &ABossLightningBolt::EnableCollision, 0.2f, false);

	collisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABossLightningBolt::OnOverlap);
	//FTimerHandle deathTimer;
	//GetWorld()->GetTimerManager().SetTimer(deathTimer, this, &ABossLightningBolt::Die, 3.0f, false);

	SetLifeSpan(3.f);
}

void ABossLightningBolt::EnableCollision()
{
	if (collisionComp)
	{
		collisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	}
}

void ABossLightningBolt::Initialize(int32 Phase, float NewDamage, float NewSpeed)
{
	if (IsActorBeingDestroyed())
	{
		return;
	}

	Damage = NewDamage;
	
	if (movementComp)
	{
		movementComp->InitialSpeed = NewSpeed;
		movementComp->MaxSpeed = NewSpeed;
		// Update velocity if already moving to reflect the new speed immediately
		if (!movementComp->Velocity.IsNearlyZero())
		{
			movementComp->Velocity = movementComp->Velocity.GetSafeNormal() * NewSpeed;
		}
	}

	FVector EffectScale = FVector(1.0f); // Default scale for Phase 1
	FVector BoxExtent = FVector(35.f, 40.f, 10.f); // Default extent for Phase 1

	if (Phase >= 2)
	{
		// Set properties for Phase 2
		BoxExtent = FVector(40.f, 60.f, 10.f); // Phase 2 extent
		// Speed is now handled by NewSpeed parameter, removed hardcoded 750 override.
		EffectScale = FVector(1.7f); // Set scale for Phase 2
	}
	collisionComp->SetBoxExtent(BoxExtent, true);

	if (tailEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			tailEffect,
			LightningBoltMesh,
			TEXT(""),
			FVector(0),      // Location
			FRotator(0),     // Rotation
			EffectScale,     // Scale
			EAttachLocation::SnapToTarget, // LocationType
			true,            // bAutoDestroy
			ENCPoolMethod::None, // PoolingMethod
			true,            // bAutoActivate
			true             // bPreCullCheck
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Tail Effect is not assigned in the LightningBolt actor."));
	}
}

// Called every frame
void ABossLightningBolt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABossLightningBolt::Die()
{
	Destroy();
}

void ABossLightningBolt::OnOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ABossMonster* Boss = Cast<ABossMonster>(OtherActor);
	AAkaPlayer* Player = Cast<AAkaPlayer>(OtherActor);
	ACoverActor* Cover = Cast<ACoverActor>(OtherActor);
	if (OtherActor == Boss) return;
	if (OtherActor == this->GetOwner()) return;
	if (OtherActor == this) return;
	if (OtherActor->IsA<ALightningBolt>()) return;
	if (OtherActor->IsA <ABossLightningBolt>()) return;	
	if (Player)
	{
		UGameplayStatics::ApplyDamage(Player, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			impactEffect,
			SweepResult.ImpactPoint
		);
		Destroy();
		return;
	}

	if (Cover)
	{
		UGameplayStatics::ApplyDamage(Cover, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			impactEffect,
			SweepResult.ImpactPoint
		);
		Destroy();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Lightning Bolt Hit: %s"), *OtherActor->GetName());
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		impactEffect,
		SweepResult.ImpactPoint
	);

	Destroy();
}
void ABossLightningBolt::SetVelocity(FVector startPos, FVector targetPos)
{
	FVector direction = targetPos - startPos;
	direction.Normalize();
	movementComp->Velocity = direction * movementComp->InitialSpeed;
}