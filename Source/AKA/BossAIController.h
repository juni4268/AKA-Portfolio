// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossAIController.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API ABossAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABossAIController();
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
public:
	// Behavior Tree asset to run
	UPROPERTY(EditDefaultsOnly, Category = "BehaviorTree")
	class UBehaviorTree* BehaviorTreeAsset;

	// Blackboard component for this controller
	class UBlackboardComponent* BlackboardComp;		
};
