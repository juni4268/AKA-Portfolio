// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	void NativeUpdateAnimation(float DeltaSeconds) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PlayerMoveState")
	float hDirectionSpeed = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PlayerMoveState")
	float vDirectionSpeed = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "PlayerMoveState")
	bool isFalling = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerMontage")
	void PlayerDeathMontage(FName sessionName);
	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerMontage")
	void PlayerHitMontage();

	UPROPERTY(VisibleANywhere, BlueprintReadWrite, Category = "Attack")
	bool bAttackRotation;
};
