// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_Attack.h"
#include "GameFramework/Actor.h"
#include "BossMonster.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "BossMonsterAnim.h"
#include "Components/CapsuleComponent.h"
#include "AkaPlayer.h"

UBTTaskNode_Attack::UBTTaskNode_Attack()
{
	NodeName = "Attack";
}
EBTNodeResult::Type UBTTaskNode_Attack::ExecuteTask(
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
		return EBTNodeResult::Failed;
	}
	// --- End of Caching Logic ---

	AActor* TargetActor = Cast<AActor>(
		CachedAIController->GetBlackboardComponent()->GetValueAsObject("TargetActor"));
	if (TargetActor == nullptr)
		return EBTNodeResult::Failed;

	if (CachedBoss->bIsDeath)
		return EBTNodeResult::Failed;
	if (CachedAnimInstance->isAttack)
		return EBTNodeResult::Failed;

	FHitResult HitResult;
	FVector EyeLocation;
	FRotator EyeRotation;
	CachedAIController->GetPawn()->GetActorEyesViewPoint(EyeLocation, EyeRotation); FVector Direction =
		(TargetActor->GetActorLocation() - EyeLocation).GetSafeNormal(); FVector End = EyeLocation + Direction * CachedBoss->AttackRange;
	TArray<AActor*> ignoreActor;
	ignoreActor.Add(CachedAIController->GetPawn());
	const AAkaPlayer* TargetPlayer = Cast<AAkaPlayer>(TargetActor); 
	ETraceTypeQuery TraceQuery = UEngineTypes::ConvertToTraceType(TargetPlayer->GetCapsuleComponent()->GetCollisionObjectType());
	bool isPlayerTrace = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), EyeLocation, End, 20.0f,
		TraceQuery, false, ignoreActor,
		EDrawDebugTrace::None,
		HitResult, true, FLinearColor::Red, FLinearColor::Yellow, 1.0f); if (isPlayerTrace == false)
		return EBTNodeResult::Failed;
	CachedAIController->StopMovement();
	CachedAnimInstance->isAttack = true;
	CachedAnimInstance->PlayAttackMontage();
	return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTaskNode_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!IsValid(CachedAnimInstance))
	{
		return EBTNodeResult::Aborted;
	}

	UE_LOG(LogTemp, Warning, TEXT("BTTaskNode_Attack AbortTask called. Resetting isAttack."));
	CachedAnimInstance->isAttack = false;
	
	// Stop the attack montage if it's still playing
	if (CachedAnimInstance && CachedAnimInstance->AttackMontage && CachedAnimInstance->Montage_IsPlaying(CachedAnimInstance->AttackMontage))
	{
		CachedAnimInstance->Montage_Stop(0.1f, CachedAnimInstance->AttackMontage);
	}
	
	return EBTNodeResult::Aborted;
}
