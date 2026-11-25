// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_CoolTime.h"
#include "GameFramework/Actor.h"
#include "BossMonster.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

UBTTaskNode_CoolTime::UBTTaskNode_CoolTime()
{
	NodeName = "Wait for CoolTime";
	bNotifyTick = true;
	WaitTime = 1.0f; // 기본 대기 시간 (초), 필요에 따라 에디터에서 수정 가능
}
EBTNodeResult::Type UBTTaskNode_CoolTime::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (AIController == nullptr)
		return EBTNodeResult::Failed;
	FCoolTimeNodeMemory* Memory = (FCoolTimeNodeMemory*)NodeMemory;
	Memory->bIsWaiting = true;
	Memory->TimeElapsed = 0.0f;
	return EBTNodeResult::InProgress;
}
void UBTTaskNode_CoolTime::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FCoolTimeNodeMemory* Memory = (FCoolTimeNodeMemory*)NodeMemory;
	if (Memory->bIsWaiting)
	{
		Memory->TimeElapsed += DeltaSeconds;
		if (Memory->TimeElapsed >= WaitTime)
		{
			Memory->bIsWaiting = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}
