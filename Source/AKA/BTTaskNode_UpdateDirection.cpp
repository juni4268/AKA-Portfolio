// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_UpdateDirection.h"
#include "GameFramework/Actor.h"
#include "BossMonster.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "BossMonsterAnim.h"

UBTTaskNode_UpdateDirection::UBTTaskNode_UpdateDirection()
{
	NodeName = "Update Direction";
	bNotifyTick = true;
}
EBTNodeResult::Type UBTTaskNode_UpdateDirection::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	return EBTNodeResult::InProgress;
}
void UBTTaskNode_UpdateDirection::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {

	// Initialize cached variables if they are not valid yet
	if (!IsValid(CachedAIController))
	{
		CachedAIController = OwnerComp.GetAIOwner();
	}
	if (!IsValid(CachedBoss))
	{
		if (CachedAIController)
		{
			CachedBoss = Cast<ABossMonster>(CachedAIController->GetPawn());
		}
	}

	if (!IsValid(CachedAIController) || !IsValid(CachedBoss))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* TargetActor = Cast<AActor>(
		CachedAIController->GetBlackboardComponent()->GetValueAsObject("TargetActor"));
	if (TargetActor == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	UpdateRotation(OwnerComp, TargetActor, DeltaSeconds);
}
void UBTTaskNode_UpdateDirection::UpdateRotation(UBehaviorTreeComponent& OwnerComp, AActor* TargetActor, float DeltaSeconds) {
	if (CachedBoss == nullptr || CachedAIController == nullptr)
		return;
	FVector DirectionToTarget =
		(TargetActor->GetActorLocation() - CachedBoss->GetActorLocation()).GetSafeNormal(); FRotator TargetRotation = DirectionToTarget.Rotation();
	FRotator CurrentRotation = CachedAIController->GetControlRotation(); float InterpSpeed = 10.0f; // ȸ  ӵ
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, InterpSpeed);
	CachedBoss->SetActorRotation(NewRotation);
	const float Tolerance = 1.1f;
	if (FMath::IsNearlyEqual(CurrentRotation.Yaw, TargetRotation.Yaw, Tolerance)) {
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}