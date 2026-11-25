// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_Attack_P2.generated.h"

// Struct to hold the state of the attack sequence
struct FAttackP2Memory
{
	int32 CurrentAttackIndex;
};

/**
 * 
 */
UCLASS()
class AKA_API UBTTaskNode_Attack_P2 : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTaskNode_Attack_P2();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** Montage to play for the 1st attack in the sequence. */
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* AttackMontage_P2_1;

	/** Montage to play for the 2nd attack in the sequence. */
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* AttackMontage_P2_2;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector RangeAttackCooldownKey;

private:
	// Helper function to start an attack
	bool PlayNextAttack(UBehaviorTreeComponent& OwnerComp, FAttackP2Memory* NodeMemory);

	UPROPERTY()
	class AAIController* CachedAIController;

	UPROPERTY()
	class ABossMonster* CachedBoss;

	UPROPERTY()
	class UBossMonsterAnim* CachedAnimInstance;
};
