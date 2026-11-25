// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "AkaPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	auto ownerPawn = TryGetPawnOwner();
	auto player = Cast<AAkaPlayer>(ownerPawn);
	if (player)
	{
		FVector velocity = player->GetVelocity();
		FVector rightVector = player->GetActorRightVector();
		hDirectionSpeed = FVector::DotProduct(rightVector, velocity);
		FVector forwardVector = player->GetActorForwardVector();
		vDirectionSpeed = FVector::DotProduct(forwardVector, velocity);
		auto movement = player->GetCharacterMovement();
		isFalling = movement->IsFalling();
	}
}