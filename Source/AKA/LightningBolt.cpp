// Fill out your copyright notice in the Description page of Project Settings.


#include "LightningBolt.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AkaPlayer.h"
#include "BossLightningBolt.h"


// Sets default values
ALightningBolt::ALightningBolt()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetBoxExtent(FVector(30.0f, 30.0f, 30.0f)); // Length 120, Width 30, Height 30
	
	RootComponent = BoxComp;
	LightningBoltMesh = CreateDefaultSubobject< UStaticMeshComponent>(TEXT("LightningMeshComp"));
	LightningBoltMesh->SetupAttachment(BoxComp);

	movementComp = CreateDefaultSubobject< UProjectileMovementComponent>(TEXT("MovementComp"));
	movementComp->SetUpdatedComponent(BoxComp);
	movementComp->InitialSpeed = 800;
	movementComp->MaxSpeed = 800;
	movementComp->bShouldBounce = true;
	movementComp->Bounciness = 0.3f;
	movementComp->ProjectileGravityScale = 0.0f;
}

// Called when the game starts or when spawned
void ALightningBolt::BeginPlay()
{
	Super::BeginPlay();

	if (LightningBoltMesh)
	{
		LightningBoltMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ALightningBolt::OnOverlap);
	FTimerHandle deathTimer;
	GetWorld()->GetTimerManager().SetTimer(deathTimer, this, &ALightningBolt::Die, 2.0f, false);
	if (tailEffect)
	{
		// 2. ���̾ư��� �ý����� �����ϰ� ������Ʈ�� ���ɴϴ�.
		UNiagaraComponent* NiagaraComponent =
			UNiagaraFunctionLibrary::SpawnSystemAttached(
			tailEffect,          // UNiagaraSystem ����
			LightningBoltMesh,   // ������ �θ� ������Ʈ
			TEXT("LB_Socket"),   // ������ ���� �̸�
			FVector(0),          // ����� ��ġ
			FRotator(0),         // ����� ȸ��
			EAttachLocation::SnapToTarget, // ���� ��Ģ
			true                 // ���� �� �ڵ� Ȱ��ȭ (true�� �⺻��)
		);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Tail Effect is not assigned in the LightningBolt actor."));
	}
}

// Called every frame
void ALightningBolt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ALightningBolt::Die()
{
	Destroy();
}
// ALightningBolt.cpp

void ALightningBolt::OnOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this->GetOwner()) return;
	if (OtherActor == this) return;
	if (OtherActor->IsA<AAkaPlayer>()) return;
	if (OtherActor->IsA<ABossLightningBolt>()) return;


	UGameplayStatics::ApplyDamage(OtherActor, 10.0f, nullptr, this, UDamageType::StaticClass());

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		impactEffect, // UNiagaraSystem* Ÿ�� ����
		SweepResult.ImpactPoint
	);
	//UGameplayStatics::PlaySoundAtLocation(
	//	GetWorld(), bomSound, SweepResult.ImpactPoint, 1.0f);
	Destroy();
}
void ALightningBolt::SetVelocity(FVector startPos, FVector targetPos)
{
	FVector direction = targetPos - startPos;
	direction.Normalize();
	movementComp->Velocity = direction * movementComp->InitialSpeed;
}