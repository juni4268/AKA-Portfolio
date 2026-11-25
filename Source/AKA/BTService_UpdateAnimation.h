// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateAnimation.generated.h"

/**
 * 
 */
UCLASS()
class AKA_API UBTService_UpdateAnimation : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_UpdateAnimation();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	class ABossAIController* CachedAIController;

	UPROPERTY()
	class ABossMonster* CachedAIPawn;

	UPROPERTY()
	class UBossMonsterAnim* CachedAnimInstance;
};
