
// Fill out your copyright notice in the Description page of Project Settings.

#include "JumpAttackIndicator.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AkaPlayer.h"
#include "Engine/DamageEvents.h"
#include "BossMonster.h"

AJumpAttackIndicator::AJumpAttackIndicator()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	DecalComponent->SetupAttachment(RootComponent);
	DecalComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComponent->DecalSize = FVector(200.0f, 350.0f, 350.0f);
}

void AJumpAttackIndicator::BeginPlay()
{
	Super::BeginPlay();
	
	TargetPlayer = Cast<AAkaPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void AJumpAttackIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFollowing && TargetPlayer)
	{
		FVector PlayerLocation = TargetPlayer->GetActorLocation();
		SetActorLocation(PlayerLocation);
	}
}

void AJumpAttackIndicator::StopFollowing()
{
	bIsFollowing = false;
}

void AJumpAttackIndicator::CheckForPlayerAndDamage(AController* EventInstigator, AActor* DamageCauser)
{
	if (TargetPlayer == nullptr) return;
	ABossMonster* Boss = Cast<ABossMonster>(DamageCauser);
	float Distance = FVector::Dist(TargetPlayer->GetActorLocation(), GetActorLocation());
	float DecalRadius = DecalComponent->DecalSize.Y; // Assuming Y and Z are the radius

	if (Distance <= DecalRadius)
	{
		TSubclassOf<UDamageType> DmgType;
		if (DamageTypeClass)
		{
			DmgType = DamageTypeClass;
		}
		else
		{
			DmgType = UDamageType::StaticClass();
		}
		UGameplayStatics::ApplyDamage(TargetPlayer, Boss->JumpDamage, EventInstigator, DamageCauser, DmgType);
		UE_LOG(LogTemp, Warning, TEXT("Jump Attack Hit Player!"));
	}
}
