// Fill out your copyright notice in the Description page of Project Settings.


#include "CoverActor.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACoverActor::ACoverActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	collisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	RootComponent = collisionComp;

	CoverActorMesh = CreateDefaultSubobject< UStaticMeshComponent>(TEXT("LightningMeshComp"));
	CoverActorMesh->SetupAttachment(collisionComp);

	static ConstructorHelpers::FObjectFinder<USoundWave> HitSoundAsset(TEXT("/Script/Engine.SoundWave'/Game/Sound/CoverActorHit.CoverActorHit'"));
	if (HitSoundAsset.Succeeded())
	{
		HitSound = HitSoundAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundWave> BreakSoundAsset(TEXT("/Script/Engine.SoundWave'/Game/Sound/CoverActorBreak.CoverActorBreak'"));
	if (BreakSoundAsset.Succeeded())
	{
		BreakSound = BreakSoundAsset.Object;
	}
}

// Called when the game starts or when spawned
void ACoverActor::BeginPlay()
{
	Super::BeginPlay();

	if (collisionComp)
	{
		collisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	}

	currentHP = maxHP;

}

// Called every frame
void ACoverActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float ACoverActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	currentHP -= DamageAmount;
	
	if (currentHP > 0.f)
	{
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		}
	}
	else if (currentHP <= 0.f)
	{
		if (BreakSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BreakSound, GetActorLocation());
		}
		Destroy();
	}
	return 0.0f;
}

