// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_Initialize.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UBTTaskNode_Initialize : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTaskNode_Initialize();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
