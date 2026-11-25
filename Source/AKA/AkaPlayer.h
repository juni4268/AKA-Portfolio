// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AkaPlayer.generated.h"

UCLASS()
class AKA_API AAkaPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAkaPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Input
	void DoMove(float Right, float Forward);
	void InputActionMove(const struct FInputActionValue& Value);
	void InputActionAttack(const struct FInputActionValue& Value);
	void OnAttackCompleted(const struct FInputActionValue& Value);
	void CheckAttack(const struct FInputActionValue& Value);
	void InputActionPause(const struct FInputActionValue& Value);
	void InputActionLapTime(const struct FInputActionValue& Value);
	UFUNCTION(BlueprintCallable, Category = "GamePause")
	void SetPause(bool bSet);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	class UInputMappingContext* inputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* moveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* attackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* pauseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* lapTimeAction;

	// Attack

	bool bIsAttacking;
	int NumAttackKeysDown;
	FVector AttackDirection;
	void Attack();

	UPROPERTY(VisibleAnywhere, Category = "WeaponMesh")
	class UStaticMeshComponent* BusterMesh;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ALightningBolt> WeaponClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UNiagaraComponent* WeaponMuzzleComp;

	UFUNCTION(BlueprintImplementableEvent)
	void Fire();

	float AttackCooldown;
	float LastAttackTime;

	// HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	int32 BossLevel = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HP")
	float maxHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HP")
	float currentHP;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	float BossLocationX;
	float BossLocationY;
	float LaunchPowerX;
	float LaunchPowerY;

	UFUNCTION(BlueprintNativeEvent, Category = "Damage")
	void OnTakeDamage_Event();

	UFUNCTION()
	void Death();
	void GameClear();
	bool bIsDeath = false;

	// UI
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UPlayerWidget> PlayerWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UPlayerWidget* PlayerWidgetInstance;

	// Called by triggers to show/hide the gameplay UI
	void ActivateGameplayHUD(bool bIsActive);

	void ShowLapTimeUI(bool bShow);

	// Timer for recording laps
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
	float TimerElapsedTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
	bool bIsTimerActive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timer")
	TArray<float> LapTimes;

	UFUNCTION(BlueprintCallable, Category = "Timer")
	void StartTimer();

	UFUNCTION(BlueprintCallable, Category = "Timer")
	void StopTimer();

	UFUNCTION(BlueprintCallable, Category = "Timer")
	void ResetTimer();

	UFUNCTION(BlueprintCallable, Category = "Timer")
	void RecordLapTime();
	bool bHasRecordedLap = false;
private:
	bool bWasTimerActiveBeforePause;

	class UPlayerAnimInstance* PlayerAnimInstance;
};