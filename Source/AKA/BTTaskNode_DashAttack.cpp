// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTaskNode_DashAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossMonster.h"
#include "AkaPlayer.h"
#include "BossMonsterAnim.h"

UBTTaskNode_DashAttack::UBTTaskNode_DashAttack()
{
	NodeName = TEXT("DashAttack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTaskNode_DashAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	
	if (CachedBoss->bIsDeath || CachedAnimInstance->isAttack || CachedBoss->DashState != EDashState::None)
	{
		return EBTNodeResult::Failed;
	}

	// BossMonster will now handle playing the montage and state transitions internally
	CachedBoss->BeginDashAttack();

	// Set cooldown
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsFloat(DashAttackRemainingCooldownKey.SelectedKeyName, CachedBoss->DashAttackCooldown);
	}

	return EBTNodeResult::InProgress;
}

void UBTTaskNode_DashAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Ensure all cached pointers are valid before proceeding
	if (!IsValid(CachedAIController) || !IsValid(CachedBoss)) // No need for AnimInstance check here
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// The task is complete when the BossMonster is no longer in a dash state
	if (CachedBoss->DashState == EDashState::None)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	// The rotational logic was moved to a separate BT task/service if needed, or handled within the BossMonster.
	// This BT task is now purely for triggering and waiting for the dash sequence to complete.
}

EBTNodeResult::Type UBTTaskNode_DashAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Ensure all cached pointers are valid before proceeding
	if (!IsValid(CachedAIController) || !IsValid(CachedBoss))
	{
		return EBTNodeResult::Aborted;
	}

	// Force stop the dash attack if the task is aborted
	CachedBoss->StopDashAttack();

	return EBTNodeResult::Aborted;
}
