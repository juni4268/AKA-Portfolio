// Fill out your copyright notice in the Description page of Project Settings.


#include "BossAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "UObject/ConstructorHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossMonster.h"


ABossAIController::ABossAIController()
{
	// Blackboard Component initialization
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}
void ABossAIController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BossAIController BeginPlay() has been called."));
	// The logic will be moved to OnPossess.
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Reset the initialization flag on the blackboard to allow the BT's Initialize task to run again.
	if (BlackboardComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Resetting 'isInitialized' flag on blackboard."));
		BlackboardComp->SetValueAsBool("isInitialized", false);
	}

	ABossMonster* BossPawn = Cast<ABossMonster>(InPawn);
	if (BossPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossAIController OnPossess called for Pawn: %s"), *BossPawn->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BossAIController OnPossess called for a non-BossMonster pawn."));
		return;
	}

	if (BehaviorTreeAsset)
	{
		if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp))
		{
			RunBehaviorTree(BehaviorTreeAsset);
			UE_LOG(LogTemp, Log, TEXT("Behavior Tree started successfully on Possess for: %s"), *InPawn->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to use blackboard on Possess for: %s"), *InPawn->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BehaviorTreeAsset is not set on Possess for: %s"), *InPawn->GetName());
	}
}
