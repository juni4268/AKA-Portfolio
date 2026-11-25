// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_UpdateAnimation.h"
#include "BossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "BossMonster.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "BossMonsterAnim.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"


UBTService_UpdateAnimation::UBTService_UpdateAnimation()
{
	NodeName = "Update Animation State";
	Interval = 0.2f; // ������Ʈ �ֱ� (��)
	RandomDeviation = 0.05f; // ���� ����
}
void UBTService_UpdateAnimation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// --- Robust Caching Logic ---
	if (!IsValid(CachedAIController))
	{
		CachedAIController = Cast<ABossAIController>(OwnerComp.GetAIOwner());
	}
	if (IsValid(CachedAIController) && !IsValid(CachedAIPawn))
	{
		CachedAIPawn = Cast<ABossMonster>(CachedAIController->GetPawn());
	}
	if (IsValid(CachedAIPawn) && !IsValid(CachedAnimInstance))
	{
		CachedAnimInstance = Cast<UBossMonsterAnim>(CachedAIPawn->GetMesh()->GetAnimInstance());
	}

	// Ensure all cached pointers are valid before proceeding
	if (!IsValid(CachedAIController) || !IsValid(CachedAIPawn) || !IsValid(CachedAnimInstance))
	{
		return;
	}
	// --- End of Caching Logic ---

	bool bIsMoving = CachedAIController->GetPathFollowingComponent()->HasValidPath();
	if (bIsMoving)
	{
		CachedAnimInstance->aiState = MonsterState::MoveState;
	}
	else
	{
		CachedAnimInstance->aiState = MonsterState::IdleState;
		CachedAnimInstance->isAttack = false;
	}
}
