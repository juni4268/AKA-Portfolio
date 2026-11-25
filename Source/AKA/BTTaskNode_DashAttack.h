// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_DashAttack.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UBTTaskNode_DashAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskNode_DashAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DashAttackRemainingCooldownKey;

private:
	UPROPERTY()
	class AAIController* CachedAIController;

	UPROPERTY()
	class ABossMonster* CachedBoss;

	UPROPERTY()
	class UBossMonsterAnim* CachedAnimInstance;
};