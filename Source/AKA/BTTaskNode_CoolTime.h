// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_CoolTime.generated.h"

struct FCoolTimeNodeMemory
{
	float TimeElapsed;
	bool bIsWaiting;
};

UCLASS()
class AKA_API UBTTaskNode_CoolTime : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskNode_CoolTime();
	virtual uint16 GetInstanceMemorySize() const override
	{
		return sizeof(FCoolTimeNodeMemory);
	}
	virtual EBTNodeResult::Type ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override; virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CoolTime")
	float WaitTime;

};
