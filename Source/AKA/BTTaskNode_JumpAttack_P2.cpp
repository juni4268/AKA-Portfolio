// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_JumpAttack_P2.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossMonster.h"
#include "BossMonsterAnim.h"

UBTTaskNode_JumpAttack_P2::UBTTaskNode_JumpAttack_P2()
{
	NodeName = TEXT("JumpAttack_P2");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTaskNode_JumpAttack_P2::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// --- Robust Caching Logic ---
	if (!IsValid(CachedAIController))
	{
		CachedAIController = OwnerComp.GetAIOwner();
	}
	if (IsValid(CachedAIController) && !IsValid(CachedBoss))
	{
		CachedBoss = Cast<ABossMonster>(CachedAIController->GetPawn());
	}
	if (IsValid(CachedBoss) && !IsValid(CachedAnimInstance))
	{
		CachedAnimInstance = Cast<UBossMonsterAnim>(CachedBoss->GetMesh()->GetAnimInstance());
	}

	// Ensure all cached pointers are valid before proceeding
	if (!IsValid(CachedAIController) || !IsValid(CachedBoss) || !IsValid(CachedAnimInstance))
	{
		return EBTNodeResult::Failed;
	}
	// --- End of Caching Logic ---
	
	if (CachedAnimInstance->isAttack)
	{
		return EBTNodeResult::Failed;
	}

	// Start the jump attack
	CachedBoss->StartJumpAttack(true, -1000.0f);

	// Set cooldown
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsFloat(JumpAttack_P2CooldownKey.SelectedKeyName, CachedBoss->JumpAttackP2Cooldown);
	}

	return EBTNodeResult::InProgress;
}

void UBTTaskNode_JumpAttack_P2::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Ensure all cached pointers are valid before proceeding
	if (!IsValid(CachedAnimInstance))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!CachedAnimInstance->isAttack)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}