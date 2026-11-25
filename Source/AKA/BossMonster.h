// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossMonster.generated.h"

enum class AttackType :uint8
{
	Melee, 
	Ranged,
};
enum class MonsterState :uint8;

// Enum to manage the dash attack state machine
UENUM(BlueprintType)
enum class EDashState : uint8
{
	None,
	Preparing,
	Dashing,
};


class UBoxComponent;
class AkaPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossHPChanged, float, CurrentHP, float, MaxHP);

UCLASS()
class AKA_API ABossMonster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABossMonster();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float DashSpeed = 750.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float DashAttackCooldown = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float RangeAttackCooldown = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float JumpAttackCooldown = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float JumpAttackP2Cooldown = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float JumpFollowTime = 1.f; // Time the indicator follows the player

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float JumpLandDelay = 1.f; // Time between indicator stopping and boss landing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float PunchDamage = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float DashDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float JumpDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float BossLightningDamage = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float BossLightningSpeed = 950.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundWave* DashSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundWave* DashEndSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundWave* JumpSound;

	UPROPERTY()
	UAudioComponent* DashAudioComponent;

	UPROPERTY()
	UAudioComponent* JumpAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundWave* HitSound1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundWave* HitSound2;

	// Refactored Dash Attack Logic
	void BeginDashAttack();
	void StopDashAttack();
	
	// Functions to be called by AnimNotifies
	UFUNCTION()
	void OnDashPrepareFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EDashState DashState;

	void StartJumpAttack(bool bIsP2 = false, float CustomJumpHeight = -1.0f);
	void StartJumpingUp();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	int BossLevel = 1;
	void LevelUpBoss();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

	// Jump Attack sequence functions
	void Jump_StartInAirPhase();
	void Jump_LockPositionAndPrepareLand();
	void Jump_Land();
	void Jump_FinishLanding();

private:
	FVector DashDirection;

	bool bIsJumpingUp = false;
	bool bIsJumpingDown = false;

	bool bIsJumpAttackP2 = false;

	UPROPERTY(EditAnywhere, Category = "Stat")
	float JumpApexHeight = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Stat")
	float JumpSpeed = 3000.0f;

	FVector JumpLandLocation;

	float CurrentJumpHeight;

	FTimerHandle JumpFollowTimer;
	FTimerHandle JumpLandTimer;
	FTimerHandle DeathTimerHandle;

	UPROPERTY()
	class AJumpAttackIndicator* CurrentIndicator;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class ABossLightningBolt> RangedProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AJumpAttackIndicator> JumpIndicatorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<AActor> DeathPortalBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning", meta = (MakeEditWidget = true))
	FVector DeathPortalSpawnLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FName RangeAttackSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SpreadAngle = 20.0f;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void MonsterFire();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void MonsterFire_P2();

	bool IsInJumpSequence() const { return bIsJumpingUp || bIsJumpingDown || CurrentIndicator != nullptr; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void BossAttack();
	//HP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
	float maxHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
	float InitialHP = 500;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HP")
	float currentHP;
	int DamageLevels;
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, BlueprintCallable, Category = "HP")
	FOnBossHPChanged OnBossHPChanged;


	bool bIsDeath = false;
	int32 currentPhase = 1;
	bool bIsStunned = false;
	UPROPERTY(EditDefaultsOnly, Category = Attack)
	float AttackRange = 150.f;
	UPROPERTY(EditDefaultsOnly, Category = Attack)
	float TraceRange = 1000.f;
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	AttackType attackType;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Death();
	MonsterState aiState;
	class UBossMonsterAnim* cachedAnim;


#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UBoxComponent> LeftHandCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UBoxComponent> RightHandCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UBoxComponent> DashCollisionBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName LeftHandCollisionBoxAttachBoneName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName RightHandCollisionBoxAttachBoneName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName DashCollisionBoxAttachBoneName;

	UFUNCTION()
	void OnBodyCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	class UMaterialInterface* Phase2OverlayMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	class UMaterialInterface* Phase3OverlayMaterial;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void ResetBoss();

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void PlayIntroSequence();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsSpawning = false; // Is true when playing spawn animation and boss is invincible

	UFUNCTION()
	void OnSpawnFinished(); // Called by AnimInstance via AnimNotify

protected:
	void UpdateOverlayMaterial();

	bool bDashDamageApplied = false;

	bool bIsinJumpAttackSequence = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float StunDuration = 3.0f;

	void Stun();
	void Unstun();

private:
	FVector InitialSpawnLocation;
	FRotator InitialSpawnRotation;
};
