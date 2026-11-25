// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateCombat.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UBTService_UpdateCombat : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateCombat();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	float IdealDistance = 800.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float StrafeDistance = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;
};
