// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_JumpAttack_P2.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UBTTaskNode_JumpAttack_P2 : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTaskNode_JumpAttack_P2();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
private:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector JumpAttack_P2CooldownKey;

	UPROPERTY()
	class AAIController* CachedAIController;

	UPROPERTY()
	class ABossMonster* CachedBoss;

	UPROPERTY()
	class UBossMonsterAnim* CachedAnimInstance;
};
