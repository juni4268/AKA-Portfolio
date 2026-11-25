// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_RangeAttack.h"
#include "AIController.h"
#include "BossMonster.h"
#include "BossMonsterAnim.h"
#include "BehaviorTree/BlackboardComponent.h"

// Constructor
UBTTaskNode_RangeAttack::UBTTaskNode_RangeAttack()
{
	NodeName = "Ranged Attack Sequence";
	bNotifyTick = true;
}

// GetInstanceMemorySize override
uint16 UBTTaskNode_RangeAttack::GetInstanceMemorySize() const
{
	return sizeof(FAttackSequenceMemory);
}

// ExecuteTask
EBTNodeResult::Type UBTTaskNode_RangeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	FAttackSequenceMemory* MyMemory = reinterpret_cast<FAttackSequenceMemory*>(NodeMemory);
	
	MyMemory->TotalAttacks = FMath::RandRange(2 , 4);
	MyMemory->CurrentAttackIndex = 0;
	
	UE_LOG(LogTemp, Warning, TEXT("Starting ranged attack sequence of %d attacks."), MyMemory->TotalAttacks);

    if(CachedAnimInstance->isAttack)
    {
        return EBTNodeResult::Failed; 
    }

	PlayNextAttack(OwnerComp, MyMemory);

    if (CachedBoss)
    {
        UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsFloat(RangeAttackCooldownKey.SelectedKeyName, CachedBoss->RangeAttackCooldown);
        }
    }

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTaskNode_RangeAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!IsValid(CachedAnimInstance))
	{
		return EBTNodeResult::Aborted;
	}

	UE_LOG(LogTemp, Warning, TEXT("BTTaskNode_RangeAttack AbortTask called. Resetting isAttack."));
	CachedAnimInstance->isAttack = false;
	
	// Stop any active range attack montages
	if (CachedAnimInstance && CachedAnimInstance->RangeAttackMontage1 && CachedAnimInstance->Montage_IsPlaying(CachedAnimInstance->RangeAttackMontage1))
	{
		CachedAnimInstance->Montage_Stop(0.1f, CachedAnimInstance->RangeAttackMontage1);
	}
	if (CachedAnimInstance && CachedAnimInstance->RangeAttackMontage2 && CachedAnimInstance->Montage_IsPlaying(CachedAnimInstance->RangeAttackMontage2))
	{
		CachedAnimInstance->Montage_Stop(0.1f, CachedAnimInstance->RangeAttackMontage2);
	}

	return EBTNodeResult::Aborted;
}

void UBTTaskNode_RangeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!IsValid(CachedAnimInstance))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// If isAttack is true, check if the montage has finished. If so, manually reset isAttack.
	// This makes the task resilient to missing AnimNotifies.
	if (CachedAnimInstance->isAttack && !CachedAnimInstance->Montage_IsPlaying(nullptr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Montage finished, manually resetting isAttack flag."));
		CachedAnimInstance->isAttack = false;
	}

	FAttackSequenceMemory* MyMemory = reinterpret_cast<FAttackSequenceMemory*>(NodeMemory);

	if (CachedAnimInstance->isAttack == false)
	{
		MyMemory->CurrentAttackIndex++;
		if (MyMemory->CurrentAttackIndex < MyMemory->TotalAttacks)
		{
			PlayNextAttack(OwnerComp, MyMemory);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Ranged attack sequence finished."));
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

void UBTTaskNode_RangeAttack::PlayNextAttack(UBehaviorTreeComponent& OwnerComp, FAttackSequenceMemory* NodeMemory)
{
    if (!IsValid(CachedAnimInstance)) 
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

    UAnimMontage* MontageToPlay = (NodeMemory->CurrentAttackIndex % 2 == 0) ? RangeAttackMontage1 : RangeAttackMontage2;

    if (MontageToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("Playing attack #%d of %d."), NodeMemory->CurrentAttackIndex + 1, NodeMemory->TotalAttacks);
        CachedAnimInstance->Montage_Play(MontageToPlay);
        CachedAnimInstance->isAttack = true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Montage not set for ranged attack sequence!"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}