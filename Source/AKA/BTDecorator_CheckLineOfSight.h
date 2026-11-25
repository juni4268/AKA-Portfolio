// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckLineOfSight.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UBTDecorator_CheckLineOfSight : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckLineOfSight();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
