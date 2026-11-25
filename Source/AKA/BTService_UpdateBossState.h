// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateBossState.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UBTService_UpdateBossState : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateBossState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PlayerDistanceKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DashAttackRemainingCooldownKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector RangeAttackRemainingCooldownKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector JumpAttackRemainingCooldownKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsAttackKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BossPhaseKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector JumpAttack_P2RemainingCooldownKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsStunnedKey;

private:
	UPROPERTY()
	class AAIController* CachedAIController;

	UPROPERTY()
	class ABossMonster* CachedBoss;

	UPROPERTY()
	class UBossMonsterAnim* CachedBossAnim;
};