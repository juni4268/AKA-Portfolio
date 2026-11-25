#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_RangeAttack.generated.h"

// Struct to hold the state of the attack sequence
struct FAttackSequenceMemory
{
	int32 TotalAttacks;
	int32 CurrentAttackIndex;
};

UCLASS()
class AKA_API UBTTaskNode_RangeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskNode_RangeAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** Montage to play for the 1st, 3rd, 5th... attack in the sequence. */
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* RangeAttackMontage1;

	/** Montage to play for the 2nd, 4th, 6th... attack in the sequence. */
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* RangeAttackMontage2;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector RangeAttackCooldownKey;

private:
	// Helper function to start an attack
	void PlayNextAttack(UBehaviorTreeComponent& OwnerComp, FAttackSequenceMemory* NodeMemory);

	UPROPERTY()
	class AAIController* CachedAIController;

	UPROPERTY()
	class ABossMonster* CachedBoss;

	UPROPERTY()
	class UBossMonsterAnim* CachedAnimInstance;
};
