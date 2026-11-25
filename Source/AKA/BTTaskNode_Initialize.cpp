	// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNode_Initialize.h"
#include "GameFramework/Actor.h"
#include "BossMonster.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

UBTTaskNode_Initialize::UBTTaskNode_Initialize()
{
	NodeName = "BT Initialize";
}
EBTNodeResult::Type UBTTaskNode_Initialize::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner()); if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent(); if (BlackboardComp)
		{
			AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0); BlackboardComp->SetValueAsObject("TargetActor", Player); BlackboardComp->SetValueAsVector("TargetLocation", Player->GetActorLocation());  BlackboardComp->SetValueAsObject(
				"SelfActor", AIController->GetPawn()); BlackboardComp->SetValueAsBool("isInitialized", true); return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
