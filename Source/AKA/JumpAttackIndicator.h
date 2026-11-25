
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JumpAttackIndicator.generated.h"

class UDecalComponent;
class AAkaPlayer;

UCLASS()
class AKA_API AJumpAttackIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	AJumpAttackIndicator();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void StopFollowing();
	void CheckForPlayerAndDamage(AController* EventInstigator, AActor* DamageCauser);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDecalComponent* DecalComponent;

	UPROPERTY()
	AAkaPlayer* TargetPlayer;

	bool bIsFollowing = true;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<class UDamageType> DamageTypeClass;
};
