// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTaskNode_JumpAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossMonster.h"
#include "BossMonsterAnim.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTaskNode_JumpAttack::UBTTaskNode_JumpAttack()
{
	NodeName = TEXT("JumpAttack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTaskNode_JumpAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	CachedBoss->StartJumpAttack(false, -1.0f);

	// Set cooldown
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsFloat(JumpAttackCooldownKey.SelectedKeyName, CachedBoss->JumpAttackCooldown);
	}

	return EBTNodeResult::InProgress;
}

void UBTTaskNode_JumpAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

EBTNodeResult::Type UBTTaskNode_JumpAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!IsValid(CachedAnimInstance))
	{
		return EBTNodeResult::Aborted;
	}

	UE_LOG(LogTemp, Warning, TEXT("BTTaskNode_JumpAttack AbortTask called. Resetting isAttack."));
	CachedAnimInstance->isAttack = false;
	
	// ABossMonster::StopDashAttack() handles stopping any active montages for dash.
	// For jump, the ABossMonster class manages the animation states. Simply resetting isAttack is sufficient.
	if (CachedBoss)
	{
		// Force end the jump sequence if it's active
		// This might need more specific handling within ABossMonster if jump is interruptible at various stages.
		// For now, resetting the flag and allowing the BT to proceed should be okay.
        CachedBoss->GetCharacterMovement()->SetMovementMode(MOVE_Walking); // Ensure character is not flying
        CachedBoss->SetActorEnableCollision(true); // Ensure collision is re-enabled if it was disabled
        CachedBoss->SetActorHiddenInGame(false); // Ensure boss is visible
	}

	return EBTNodeResult::Aborted;
}
