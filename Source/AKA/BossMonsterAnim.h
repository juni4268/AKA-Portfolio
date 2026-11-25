// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BossAicontroller.h"
#include "BossMonsterAnim.generated.h"

UENUM(BlueprintType)
enum class MonsterState :uint8
{
	IdleState,
	MoveState,
};

UCLASS()
class AKA_API UBossMonsterAnim : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	MonsterState aiState;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	bool isAttack = false;
	//Phase 1

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* DashStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* DashAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* JumpStartMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* JumpEndMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* StunMontage;

	//Phase 2
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RangeAttackMontage1;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RangeAttackMontage2;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AttackMontage_P2_1;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AttackMontage_P2_2;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* SpawnMontage;
	
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayAttackMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayDashAttackMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayJumpStartMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayStunMontage();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlaySpawnMontage();
	
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void AnimNotify_Jump_Launch();

	// New AnimNotify functions for Dash Attack
	// Changed to BlueprintCallable for easier direct use from AnimNotifies in Blueprint
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void HandleDashPrepareFinished();
	
	// Changed to BlueprintCallable for easier direct use from AnimNotifies in Blueprint
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void HandleSpawnFinished();

	//Phase 2

	//State
	UFUNCTION(BlueprintImplementableEvent, Category = "MonsterAnim")
	void PlayDeathMontage(FName sectionName);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void AttackMontageEnded();

};

