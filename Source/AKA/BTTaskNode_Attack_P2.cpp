// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_Attack_P2.h"
#include "GameFramework/Actor.h"
#include "BossMonster.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossMonsterAnim.h"
#include "AkaPlayer.h"



UBTTaskNode_Attack_P2::UBTTaskNode_Attack_P2()
{
	NodeName = "Attack_P2_Ranged"; // Changed name for clarity
	bNotifyTick = true;
}

uint16 UBTTaskNode_Attack_P2::GetInstanceMemorySize() const
{
	return sizeof(FAttackP2Memory);
}

EBTNodeResult::Type UBTTaskNode_Attack_P2::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
		UE_LOG(LogTemp, Error, TEXT("UBTTaskNode_Attack_P2::ExecuteTask - Missing dependencies. Controller: %s, Boss: %s, Anim: %s"), 
			CachedAIController ? TEXT("Valid") : TEXT("Null"),
			CachedBoss ? TEXT("Valid") : TEXT("Null"),
			CachedAnimInstance ? TEXT("Valid") : TEXT("Null"));
		return EBTNodeResult::Failed;
	}
	// --- End of Caching Logic ---

	FAttackP2Memory* MyMemory = reinterpret_cast<FAttackP2Memory*>(NodeMemory);
	
	MyMemory->CurrentAttackIndex = 0;
	
    if(CachedAnimInstance->isAttack || CachedBoss->bIsDeath)
    {
        return EBTNodeResult::Failed;
    }

	bool bStarted = PlayNextAttack(OwnerComp, MyMemory);
    if (!bStarted)
    {
        return EBTNodeResult::Failed;
    }

    if (CachedBoss)
    {
        UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
        if (BlackboardComp)
        {
        	UE_LOG(LogTemp, Warning, TEXT("UBTTaskNode_Attack_P2::ExecuteTask - Setting RangeAttackCooldown: %f"), CachedBoss->RangeAttackCooldown);
            BlackboardComp->SetValueAsFloat(RangeAttackCooldownKey.SelectedKeyName, CachedBoss->RangeAttackCooldown);
        }
    }

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTaskNode_Attack_P2::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!IsValid(CachedAnimInstance))
	{
		return EBTNodeResult::Aborted;
	}

	UE_LOG(LogTemp, Warning, TEXT("BTTaskNode_Attack_P2 AbortTask called. Resetting via AttackMontageEnded."));
	CachedAnimInstance->AttackMontageEnded();
	
	// Stop any active P2 attack montages
	if (CachedAnimInstance && CachedAnimInstance->AttackMontage_P2_1 && CachedAnimInstance->Montage_IsPlaying(CachedAnimInstance->AttackMontage_P2_1))
	{
		CachedAnimInstance->Montage_Stop(0.1f, CachedAnimInstance->AttackMontage_P2_1);
	}
	if (CachedAnimInstance && CachedAnimInstance->AttackMontage_P2_2 && CachedAnimInstance->Montage_IsPlaying(CachedAnimInstance->AttackMontage_P2_2))
	{
		CachedAnimInstance->Montage_Stop(0.1f, CachedAnimInstance->AttackMontage_P2_2);
	}

	return EBTNodeResult::Aborted;
}

void UBTTaskNode_Attack_P2::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
		UE_LOG(LogTemp, Warning, TEXT("Montage finished, but isAttack was still true. Calling AttackMontageEnded() manually. (Missing AnimNotify?)"));
		CachedAnimInstance->AttackMontageEnded();
	}

	FAttackP2Memory* MyMemory = reinterpret_cast<FAttackP2Memory*>(NodeMemory);

	if (CachedAnimInstance->isAttack == false)
	{
		MyMemory->CurrentAttackIndex++;
		if (MyMemory->CurrentAttackIndex < 2) // Max 2 attacks
		{
			bool bStarted = PlayNextAttack(OwnerComp, MyMemory);
            if (!bStarted)
            {
                 FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            }
		}
		else
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

bool UBTTaskNode_Attack_P2::PlayNextAttack(UBehaviorTreeComponent& OwnerComp, FAttackP2Memory* NodeMemory)
{
    if (!IsValid(CachedAnimInstance))
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("UBTTaskNode_Attack_P2::PlayNextAttack - Index: %d"), NodeMemory->CurrentAttackIndex);

    UAnimMontage* MontageToPlay = (NodeMemory->CurrentAttackIndex % 2 == 0) ? AttackMontage_P2_1 : AttackMontage_P2_2;

    if (MontageToPlay)
    {
    	UE_LOG(LogTemp, Warning, TEXT("UBTTaskNode_Attack_P2::PlayNextAttack - Playing Montage: %s"), *MontageToPlay->GetName());
        float Duration = CachedAnimInstance->Montage_Play(MontageToPlay);
        if (Duration > 0.0f)
        {
            CachedAnimInstance->isAttack = true;
            return true;
        }
        else
        {
             UE_LOG(LogTemp, Error, TEXT("UBTTaskNode_Attack_P2::PlayNextAttack - Montage_Play returned 0 duration (Failed)."));
             return false;
        }
    }
    else
    {
    	UE_LOG(LogTemp, Error, TEXT("UBTTaskNode_Attack_P2::PlayNextAttack - MontageToPlay is NULL for Index: %d"), NodeMemory->CurrentAttackIndex);
        //FinishLatentTask(OwnerComp, EBTNodeResult::Failed); // Don't call FinishLatentTask here, let caller handle failure
        return false;
    }
}
