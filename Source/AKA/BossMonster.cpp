// Fill out your copyright notice in the Description page of Project Settings.


#include "BossMonster.h"
#include "AkaPlayer.h"
#include "BossAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "BossMonsterAnim.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BossLightningBolt.h"
#include "JumpAttackIndicator.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "PlayerWidget.h"
#include "AKAGameInstance.h"
#include "StunnablePillar.h"



// Sets default values
ABossMonster::ABossMonster()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CurrentIndicator = nullptr;

	// This is the crucial line: It tells the pawn to get an AI controller when spawned in the world OR at runtime.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	aiState = MonsterState::IdleState;
	DashState = EDashState::None; // Initialize DashState

	LeftHandCollisionBox = CreateDefaultSubobject<UBoxComponent>("LeftHandCollisionBox");
	LeftHandCollisionBox->SetupAttachment(GetMesh());
	LeftHandCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnBodyCollisionBoxBeginOverlap);

	RightHandCollisionBox = CreateDefaultSubobject<UBoxComponent>("RightHandCollisionBox");
	RightHandCollisionBox->SetupAttachment(GetMesh());
	RightHandCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnBodyCollisionBoxBeginOverlap);

	DashCollisionBox = CreateDefaultSubobject<UBoxComponent>("DashCollisionBox");
	DashCollisionBox->SetupAttachment(GetMesh());
	DashCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnBodyCollisionBoxBeginOverlap);
}

void ABossMonster::PlayIntroSequence()
{
	UE_LOG(LogTemp, Warning, TEXT("BossMonster: Playing Intro Sequence."));

	// Reset basic states
	currentPhase = 1;
	bIsDeath = false;
	bIsStunned = false;
	DashState = EDashState::None;
	bIsSpawning = true; // Boss is now spawning and invincible

	// Reset Location (use InitialSpawnLocation which is set in BeginPlay)
	SetActorLocation(InitialSpawnLocation);
	SetActorRotation(InitialSpawnRotation);
	
	// Ensure boss is visible and has collision for the spawn animation
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Reset Animation State and play spawn montage
	if (cachedAnim)
	{
		cachedAnim->Montage_Stop(0.0f, nullptr);
		cachedAnim->isAttack = false;
		cachedAnim->aiState = MonsterState::IdleState;
		
		// Play Spawn Montage
		cachedAnim->PlaySpawnMontage();
	}

	// Reset Overlay
	UpdateOverlayMaterial();

	// Disable AI Logic initially
	ABossAIController* AICon = Cast<ABossAIController>(GetController());
	if (AICon)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComponent();
		if (BlackboardComp)
		{
			// Reset Cooldowns
			BlackboardComp->SetValueAsFloat(FName("DashAttackRemainingCooldown"), 0.0f);
			BlackboardComp->SetValueAsFloat(FName("RangeAttackRemainingCooldown"), 0.0f);
			BlackboardComp->SetValueAsFloat(FName("JumpAttackRemainingCooldown"), 0.0f);
			BlackboardComp->SetValueAsFloat(FName("JumpAttack_P2RemainingCooldown"), 0.0f);
			
			// Disable AI Logic until spawn animation is finished
			BlackboardComp->SetValueAsBool(FName("IsSpawned"), false);
			
			// Stop Behavior Tree
			AICon->GetBrainComponent()->StopLogic("Boss Spawning Intro");
		}
	}
	
	// Update stats based on current BossLevel (should be already set from GameInstance)
	LevelUpBoss(); // This will set maxHP and currentHP based on BossLevel.
	
	// Broadcast HP update
	if (OnBossHPChanged.IsBound())
	{
		OnBossHPChanged.Broadcast(currentHP, maxHP);
	}

	// Switch to Battle BGM
	UAKAGameInstance* GI = Cast<UAKAGameInstance>(GetGameInstance());
	if (GI && GI->BattleBGM)
	{
		GI->PlayBGM(GI->BattleBGM, 1.0f); // Fade in over 1 second
	}
}

// Called when the game starts or when spawned
void ABossMonster::BeginPlay()
{
	Super::BeginPlay();

	// Initialize Collision Settings (Moved from Constructor to avoid GEngine initialization error)
	if (LeftHandCollisionBox) LeftHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (RightHandCollisionBox) RightHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (DashCollisionBox) DashCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OnActorHit.AddDynamic(this, &ABossMonster::OnHit);
	
	InitialSpawnLocation = GetActorLocation(); // Store initial location where boss is placed
	InitialSpawnRotation = GetActorRotation(); // Store initial rotation

	// Get AnimInstance early
	if (GetMesh())
	{
		cachedAnim = Cast<UBossMonsterAnim>(GetMesh()->GetAnimInstance());
		if (cachedAnim)
		{
			UE_LOG(LogTemp, Warning, TEXT("BossMonster: AnimInstance cached."));
		}
	}
	
	// Initial intro sequence call on first spawn (BossLevel will be 0, then 1 after Intro)
	PlayIntroSequence();

	// All other initializations are now handled by PlayIntroSequence
}

void ABossMonster::ResetBoss()
{
	UE_LOG(LogTemp, Warning, TEXT("BossMonster: Resetting Boss State for new phase..."));

	// 1. Increment Level
	BossLevel++;
	UAKAGameInstance* GI = Cast<UAKAGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->CurrentBossLevel = BossLevel; // Update GameInstance
	}
	
	// All other reset and intro logic is handled by PlayIntroSequence
	PlayIntroSequence();
	
	UE_LOG(LogTemp, Warning, TEXT("BossMonster: Reset Complete. Level: %d, Spawning..."), BossLevel);
}

void ABossMonster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
		World->GetTimerManager().ClearTimer(DeathTimerHandle);
	}

	if (IsValid(CurrentIndicator))
	{
		CurrentIndicator->Destroy();
		CurrentIndicator = nullptr;
	}
}

// Called every frame

void ABossMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDeath) return;

	// Look at player when preparing dash
	
	if (DashState == EDashState::Preparing)
	{
		AAkaPlayer* Player = Cast<AAkaPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		if (Player)
		{
			FVector PlayerLocation = Player->GetActorLocation();
			FVector BossLocation = GetActorLocation();
			FVector DirectionToPlayer = (PlayerLocation - BossLocation);
			DirectionToPlayer.Z = 0; // Keep the rotation in the XY plane

			FRotator TargetRotation = DirectionToPlayer.Rotation();
			FRotator CurrentRotation = GetActorRotation();

			// Smoothly interpolate to the target rotation
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f); // 5.0f is the interpolation speed, can be adjusted
			
			SetActorRotation(NewRotation);
		}
	}

	// Manual dash movement
	if (DashState == EDashState::Dashing)
	{
		FVector Displacement = DashDirection * DashSpeed * DeltaTime;
		AddActorWorldOffset(Displacement, true); // Using AddActorWorldOffset with sweep
	}


	// Dash movement is now controlled by OnDashPrepareFinished and StopDashAttack
	// No longer need bIsDashing check here.

	if (bIsJumpingUp)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = FVector(CurrentLocation.X, CurrentLocation.Y, CurrentJumpHeight);
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, JumpSpeed / 1000.0f);
		SetActorLocation(NewLocation);

		if (FMath::IsNearlyEqual(NewLocation.Z, CurrentJumpHeight, 10.0f))
		{
			bIsJumpingUp = false;
			Jump_StartInAirPhase();
		}
	}

	if (bIsJumpingDown)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, JumpLandLocation, DeltaTime, JumpSpeed / 500.0f);
		
		FHitResult HitResult;
		bool bMoved = SetActorLocation(NewLocation, true, &HitResult);

		// Check if we hit the ground OR reached the target
		if ((HitResult.bBlockingHit && HitResult.Normal.Z > 0.1f) || FVector::Dist(GetActorLocation(), JumpLandLocation) < 40.0f)
		{
			bIsJumpingDown = false;
			Jump_FinishLanding();
		}
	}
}

void ABossMonster::BeginDashAttack()
{
	if (DashState != EDashState::None) return;

	// Check for player early to avoid starting a useless attack
	AAkaPlayer* Player = Cast<AAkaPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss BeginDashAttack: Player not found, cannot start dash."));
		return; // Abort if no target
	}

	UE_LOG(LogTemp, Warning, TEXT("Boss Begin Dash Attack Sequence!"));
	DashState = EDashState::Preparing;
	cachedAnim->isAttack = true; // General attack flag

	// Direction is now calculated in OnDashPrepareFinished, after rotation

	// Play preparation montage, which will trigger OnDashPrepareFinished via AnimNotify
	if (cachedAnim && cachedAnim->DashStartMontage)
	{
		cachedAnim->Montage_Play(cachedAnim->DashStartMontage);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Boss BeginDashAttack: DashStartMontage is not set on AnimInstance."));
		OnDashPrepareFinished(); // Try to proceed directly if montage is missing
	}
}

void ABossMonster::OnDashPrepareFinished()
{
	if (DashState != EDashState::Preparing) return;

	UE_LOG(LogTemp, Warning, TEXT("Boss OnDashPrepareFinished: Starting actual dash movement."));

	// Calculate direction based on current rotation (which was set during 'Preparing' state)
	DashDirection = GetActorForwardVector();
	DashDirection.Z = 0.0f; // Ensure movement is horizontal
	DashDirection.Normalize();

	DashState = EDashState::Dashing;
	
	// Ensure capsule triggers hit events
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Pass through player
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // Hit walls
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // Hit dynamic obstacles
	
	DashCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (DashSound)
	{
		DashAudioComponent = UGameplayStatics::SpawnSoundAttached(DashSound, GetRootComponent());
	}

	// Play actual dash attack montage
	if (cachedAnim && cachedAnim->DashAttackMontage)
	{
		cachedAnim->Montage_Play(cachedAnim->DashAttackMontage);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Boss OnDashPrepareFinished: DashAttackMontage is not set on AnimInstance."));
		StopDashAttack(); // Abort if montage is missing
	}
}

void ABossMonster::StopDashAttack()
{
	if (DashState == EDashState::None) return; // Already stopped

	UE_LOG(LogTemp, Warning, TEXT("Boss Stop Dash Attack!"));
	DashState = EDashState::None;
	cachedAnim->isAttack = false; // Reset general attack flag

	if (DashAudioComponent && DashAudioComponent->IsPlaying())
	{
		DashAudioComponent->Stop();
	}

	if (DashEndSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DashEndSound, GetActorLocation());
	}

	// Stop any currently playing dash montages
	if (cachedAnim && cachedAnim->DashStartMontage && cachedAnim->Montage_IsPlaying(cachedAnim->DashStartMontage))
	{
		cachedAnim->Montage_Stop(0.1f, cachedAnim->DashStartMontage);
	}
	if (cachedAnim && cachedAnim->DashAttackMontage && cachedAnim->Montage_IsPlaying(cachedAnim->DashAttackMontage))
	{
		cachedAnim->Montage_Stop(0.1f, cachedAnim->DashAttackMontage);
	}

	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	DashCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bDashDamageApplied = false;
}

void ABossMonster::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	// Debug Log for OnHit
	if (DashState == EDashState::Dashing)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossMonster::OnHit - Hit Detected! OtherActor: %s"), *OtherActor->GetName());
	}

	if (bIsDeath) return; // Don't process hits if dead

	if (DashState == EDashState::Dashing) // Check the new DashState
	{
		if (!OtherActor) return; // Safety check

		// Check if the actor we hit is a stunnable wall
		if (OtherActor->ActorHasTag(TEXT("StunnableWall")))
		{
			UE_LOG(LogTemp, Warning, TEXT("BossMonster::OnHit - Hit StunnableWall!"));
			AStunnablePillar* Pillar = Cast<AStunnablePillar>(OtherActor);
			if (Pillar)
			{
				Pillar->TakeHitFromBoss(GetActorForwardVector());
			}
			// Stun regardless of whether it was a pillar class, as long as it has the tag
			Stun();
		}
		else // If it's any other actor that's not the player, stop the dash
		{
			AAkaPlayer* Player = Cast<AAkaPlayer>(OtherActor);
			// Stop dash if we hit something that is NOT the player (and not a stunnable wall handled above)
			if (OtherActor != this && Player == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Condition Met! -> Stopping Dash Attack. Hit: %s"), *OtherActor->GetName());
				StopDashAttack();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("BossMonster::OnHit - Hit ignored. Player: %s, Self: %s"), 
					Player ? TEXT("Yes") : TEXT("No"), 
					(OtherActor == this) ? TEXT("Yes") : TEXT("No"));
			}
		}
	}
}

void ABossMonster::Stun()
{
	if (bIsStunned) return;

	UE_LOG(LogTemp, Warning, TEXT("Boss is STUNNED!"));
	bIsStunned = true;

	// Stop any current dash attack or other actions
	StopDashAttack(); 

	// Play a stun animation if one exists
	if (cachedAnim)
	{	
		cachedAnim->PlayStunMontage();
	}

	// Set a timer to unstun the boss
	FTimerHandle StunTimerHandle;
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ABossMonster::Unstun, StunDuration, false);
}

void ABossMonster::Unstun()
{
	UE_LOG(LogTemp, Warning, TEXT("Boss is no longer stunned."));
	bIsStunned = false;
}

// Called to bind functionality to input
void ABossMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABossMonster::BossAttack()
{

	UE_LOG(LogTemp, Warning, TEXT("Boss Attack!"));
}

float ABossMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsSpawning)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss is currently spawning and invincible!"));
		return 0.0f; // No damage during spawning
	}

	currentHP -= DamageAmount;
	//HP
	UE_LOG(LogTemp, Warning, TEXT("Boss Took Damage: %f, Current HP: %f"), DamageAmount, currentHP);

	if (HitSound1)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound1, GetActorLocation(), 0.3f);
	}
	if (HitSound2)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound2, GetActorLocation(), 0.3f);
	}

	// Phase transition logic
	bool bPhaseChanged = false;
	if (currentPhase == 1 && currentHP <= (maxHP * 2.0f / 3.0f))
	{
		currentPhase = 2;
		bPhaseChanged = true;
		UE_LOG(LogTemp, Warning, TEXT("Boss Phase Change to 2!"));
	}
	if (currentPhase == 2 && currentHP <= (maxHP * 1.0f / 3.0f))
	{
		currentPhase = 3;
		bPhaseChanged = true;
		UE_LOG(LogTemp, Warning, TEXT("Boss Phase Change to 3!"));
	}

	if (bPhaseChanged)
	{
		UpdateOverlayMaterial();
	}

	if (OnBossHPChanged.IsBound())
	{
		OnBossHPChanged.Broadcast(currentHP, maxHP);
	}
	if (currentHP <= 0.f && bIsDeath == false)
	{
		Death();
	}
	return 0.0f;
}

void ABossMonster::Death()
{
	if (bIsDeath) // Safety guard against multiple Death calls
	{
		return;
	}
	
	bIsDeath = true;
	
	DashState = EDashState::None;
	
	if (DashAudioComponent && DashAudioComponent->IsPlaying())
	{
		DashAudioComponent->Stop();
	}
	if (JumpAudioComponent && JumpAudioComponent->IsPlaying())
	{
		JumpAudioComponent->Stop();
	}

	AAkaPlayer* Player = Cast<AAkaPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
	if (Player)
	{
		// Hide the boss UI, then handle other player-related actions
		if (Player->PlayerWidgetInstance)
		{
			Player->PlayerWidgetInstance->ShowBossUI(false);
		}
	}

	// Victory Condition Check
	if (BossLevel >= 3)
	{
		if (Player)
		{
			Player->GameClear(); // Trigger Victory
		}
		// Skip portal spawning and corpse hiding on victory so the player can see the defeated boss
	}
	else
	{
		// Standard Progression
		if (Player)
		{
			Player->RecordLapTime(); // Auto-record lap time on boss death
			Player->StopTimer();
		}

		// Switch back to Main BGM
		UAKAGameInstance* GI = Cast<UAKAGameInstance>(GetGameInstance());
		if (GI && GI->MainBGM)
		{
			GI->PlayBGM(GI->MainBGM, 1.0f); // Fade in over 1 second
		}

		// If a portal blueprint has been assigned in the editor
		if (DeathPortalBlueprint)
		{
			// Use the user-defined spawn location
			const FVector SpawnLocation = DeathPortalSpawnLocation;
			// Use the boss's current rotation for the portal (or a default rotation if preferred)
			const FRotator SpawnRotation = FRotator(0, 0, 0);

			GetWorld()->SpawnActor<AActor>(DeathPortalBlueprint, SpawnLocation, SpawnRotation);
		}

		// Hide Boss after a delay (to let Death Anim play)
		TWeakObjectPtr<ABossMonster> WeakSelf(this);
		GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, [WeakSelf]()
			{
				if (WeakSelf.IsValid())
				{
					WeakSelf->SetActorHiddenInGame(true);
					WeakSelf->SetActorEnableCollision(false);
				}
			}, 3.0f, false);
	}

	if (cachedAnim)
	{
		cachedAnim->PlayDeathMontage(TEXT("DeathStart"));
	}

	GetWorldTimerManager().ClearTimer(JumpFollowTimer);
	GetWorldTimerManager().ClearTimer(JumpLandTimer);
	
	if (IsValid(CurrentIndicator))
	{
		CurrentIndicator->Destroy();
		CurrentIndicator = nullptr;
	}

	// Disable AI Logic (temporarily, until reset and spawned)
	ABossAIController* AICon = Cast<ABossAIController>(GetController());
	if (AICon)
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic("Boss Died");
		}
		AICon->StopMovement();

		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(FName("IsSpawned"), false);
		}
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABossMonster::OnSpawnFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("BossMonster: OnSpawnFinished - Boss is now active!"));
	bIsSpawning = false; // Invincibility ends

	// Enable AI Logic
	ABossAIController* AICon = Cast<ABossAIController>(GetController());
	if (AICon)
	{
		UBlackboardComponent* BlackboardComp = AICon->GetBlackboardComponent();
		if (BlackboardComp)
		{
			BlackboardComp->SetValueAsBool(FName("IsSpawned"), true);
			AICon->GetBrainComponent()->RestartLogic(); // Restart BT to pick up new state
		}
	}
}

void ABossMonster::UpdateOverlayMaterial()
{
	USkeletalMeshComponent* BossMesh = GetMesh();
	if (!BossMesh)
	{
		return;
	}

	UMaterialInterface* NewOverlayMaterial = nullptr;
	switch (currentPhase)
	{
	case 2:
		NewOverlayMaterial = Phase2OverlayMaterial;
		break;
	case 3:
		NewOverlayMaterial = Phase3OverlayMaterial;
		break;
	default:
		// For phase 1, or if we want to remove the overlay, set it to null
		NewOverlayMaterial = nullptr;
		break;
	}

	// This function renders the material on top of the existing materials,
	// which is perfect for an outline or overlay effect.
	BossMesh->SetOverlayMaterial(NewOverlayMaterial);
	if (NewOverlayMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("Set overlay material for Phase %d. Material: %s"), currentPhase, *NewOverlayMaterial->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Removed overlay material for Phase %d."), currentPhase);
	}
}

#if WITH_EDITOR
void ABossMonster::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, LeftHandCollisionBoxAttachBoneName))
	{
		LeftHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, LeftHandCollisionBoxAttachBoneName);
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, RightHandCollisionBoxAttachBoneName))
	{
		RightHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, RightHandCollisionBoxAttachBoneName);
	}
	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, DashCollisionBoxAttachBoneName))
	{
		DashCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, DashCollisionBoxAttachBoneName);
	}
}
#endif

void ABossMonster::OnBodyCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsDeath == true) return;

	AAkaPlayer* Player = Cast<AAkaPlayer>(OtherActor);
	if (Player == nullptr) return;
	if (OtherActor != Player) return;
	if (OverlappedComponent == DashCollisionBox)
	{
		if (bDashDamageApplied) return;
		UE_LOG(LogTemp, Warning, TEXT("Boss Dash Attack Hit Detected"));
		UGameplayStatics::ApplyDamage(Player, DashDamage, GetController(), this, UDamageType::StaticClass());
		bDashDamageApplied = true;
		FTimerHandle ResetDashDamageHandle;
		TWeakObjectPtr<ABossMonster> WeakSelf(this);
		GetWorld()->GetTimerManager().SetTimer(ResetDashDamageHandle, [WeakSelf]()
			{
				if (WeakSelf.IsValid())
				{
					WeakSelf->bDashDamageApplied = false;
				}
			}, 0.5f, false);
	}
	else if (OverlappedComponent == LeftHandCollisionBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss Left Hand Attack Hit Detected"));
		UGameplayStatics::ApplyDamage(Player, PunchDamage, GetController(), this, UDamageType::StaticClass());
	}
	else if (OverlappedComponent == RightHandCollisionBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss Left Hand Attack Hit Detected"));
		UGameplayStatics::ApplyDamage(Player, PunchDamage, GetController(), this, UDamageType::StaticClass());
	}
	return;
}

void ABossMonster::MonsterFire()
{
	if (RangedProjectileClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("RangedProjectileClass is not set on BossMonster."));
		return;
	}

	// Get spawn location and rotation
	const FVector SpawnLocation = GetMesh()->GetSocketLocation(RangeAttackSocket);
	const FRotator CharacterRotation = GetActorRotation();

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SocketLocation = GetMesh()->GetSocketLocation(RangeAttackSocket);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	auto SpawnIfClear = [&](const FVector& SpawnLoc, const FRotator& SpawnRot)
	{
		FHitResult HitResult;
		// Trace from socket to spawn location to ensure line of sight
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, SocketLocation, SpawnLoc, ECC_WorldStatic, QueryParams);
		if (!bHit)
		{
			if (ABossLightningBolt* Bolt = GetWorld()->SpawnActor<ABossLightningBolt>(RangedProjectileClass, SpawnLoc, SpawnRot, SpawnParams))
			{
				Bolt->Initialize(currentPhase, BossLightningDamage, BossLightningSpeed);
			}
		}
	};

	if (currentPhase >= 2)
	{
		// 1. Center
		SpawnIfClear(SpawnLocation, CharacterRotation);
		
		// 2. Right
		FRotator RightRotation = CharacterRotation;
		RightRotation.Yaw += SpreadAngle*1.4;
		SpawnIfClear(SpawnLocation, RightRotation);

		// 3. Left
		FRotator LeftRotation = CharacterRotation;
		LeftRotation.Yaw -= SpreadAngle*1.4;
		SpawnIfClear(SpawnLocation, LeftRotation);
	}
	else 
	{
		// 1. Center
		SpawnIfClear(SpawnLocation, CharacterRotation);

		// 2. Right
		FRotator RightRotation = CharacterRotation;
		RightRotation.Yaw += SpreadAngle;
		SpawnIfClear(SpawnLocation, RightRotation);

		// 3. Left
		FRotator LeftRotation = CharacterRotation;
		LeftRotation.Yaw -= SpreadAngle;
		SpawnIfClear(SpawnLocation, LeftRotation);
	}
}

void ABossMonster::MonsterFire_P2()
{
	UE_LOG(LogTemp, Warning, TEXT("ABossMonster::MonsterFire_P2 Called"));
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 1; i <= 10; ++i)
	{
		FRotator ProjectileRotation = GetActorRotation();
		ProjectileRotation.Yaw += i * 36;
		
		FVector FinalSpawnLocation = GetActorLocation(); // GetActorLocation()을 기준으로 변경
		FinalSpawnLocation.Z -= 100.0f; // Z 오프셋을 20.0f로 설정

		// 보스 몸에 겹치지 않도록 발사 방향으로 약간 밀어내기 (선택 사항)
		FinalSpawnLocation += ProjectileRotation.Vector() * 50.0f; 

		FHitResult HitResult;
		// QueryParams 선언 다시 추가
		FCollisionQueryParams QueryParamsLocal;
		QueryParamsLocal.AddIgnoredActor(this);

		// 스폰 지점 자체에 막히는지 검사합니다.
		bool bSpawnPointBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, FinalSpawnLocation, FinalSpawnLocation + FVector(0,0,1), ECC_WorldStatic, QueryParamsLocal);

		if (!bSpawnPointBlocked) // 스폰 지점 자체가 막히지 않았다면
		{
			if (ABossLightningBolt* Bolt = GetWorld()->SpawnActor<ABossLightningBolt>(RangedProjectileClass, FinalSpawnLocation, ProjectileRotation, SpawnParams))
			{
				Bolt->Initialize(currentPhase, BossLightningDamage, BossLightningSpeed);
				UE_LOG(LogTemp, Warning, TEXT("MonsterFire_P2: Successfully spawned BossLightningBolt at location: %s"), *Bolt->GetActorLocation().ToString());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("MonsterFire_P2: Failed to spawn BossLightningBolt. RangedProjectileClass might be invalid."));
			}
		}
		else // 스폰 지점이 막혔을 때 로그 출력
		{
			UE_LOG(LogTemp, Warning, TEXT("MonsterFire_P2: Projectile spawn blocked at exact spawn point. Hit Actor: %s, Hit Location: %s"),
				*HitResult.GetActor()->GetName(), *HitResult.ImpactPoint.ToString());
		}
	}
}

void ABossMonster::StartJumpAttack(bool bIsP2, float CustomJumpHeight)
{	
	if (this == nullptr) return;
	if (cachedAnim == nullptr || cachedAnim->isAttack)
	{
		return;
	}
	this->bIsJumpAttackP2 = bIsP2;
	this->CurrentJumpHeight = (CustomJumpHeight != -1.0f) ? CustomJumpHeight : JumpApexHeight;
	cachedAnim->isAttack = true;
	GetController()->StopMovement();
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	// Play jump start montage, which contains the AnimNotify to call StartJumpingUp()
	cachedAnim->PlayJumpStartMontage();
}

void ABossMonster::StartJumpingUp()
{
	bIsJumpingUp = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (JumpSound)
	{
		JumpAudioComponent = UGameplayStatics::SpawnSoundAttached(JumpSound, GetRootComponent());
	}
}

void ABossMonster::LevelUpBoss()
{
	// Scale HP: InitialHP + 500 per level
	maxHP = InitialHP + (BossLevel * 150.0f);
	currentHP = maxHP;

	// Scale Damage based on BossLevel using base values defined in header as reference
	// Base Punch: 5, Dash: 20, Jump: 20, Lightning: 10
	PunchDamage = BossLevel * 5.0f;
	DashDamage = 10.0f + (BossLevel * 5.0f);
	JumpDamage = 10.0f + (BossLevel * 5.0f);
	BossLightningDamage = BossLevel * 5.0f;
	BossLightningSpeed = 850.0f + (BossLevel * 50.0f);
	
	AAkaPlayer* Player = Cast<AAkaPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (Player)
	{
		Player->BossLevel = this->BossLevel;
	}

	UE_LOG(LogTemp, Warning, TEXT("Boss Level Up! Level: %d, HP: %f, Punch: %f, Dash: %f, Jump: %f, Lightning: %f, BoltSpeed: %f"), 
		BossLevel, maxHP, PunchDamage, DashDamage, JumpDamage, BossLightningDamage, BossLightningSpeed);
}

void ABossMonster::Jump_StartInAirPhase()
{
	// Hide actor
	SetActorHiddenInGame(true);

	// Spawn indicator actor
	if (JumpIndicatorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z = 0; // Spawn on the ground
		CurrentIndicator = GetWorld()->SpawnActor<AJumpAttackIndicator>(JumpIndicatorClass, SpawnLocation, GetActorRotation(), SpawnParams);
	}

	// Start timer to stop following the player
	GetWorldTimerManager().SetTimer(JumpFollowTimer, this, &ABossMonster::Jump_LockPositionAndPrepareLand, JumpFollowTime, false);
}

void ABossMonster::Jump_LockPositionAndPrepareLand()
{
	if (CurrentIndicator)
	{
		CurrentIndicator->StopFollowing();
	}

	// Start timer to actually land
	GetWorldTimerManager().SetTimer(JumpLandTimer, this, &ABossMonster::Jump_Land, JumpLandDelay, false);
}

void ABossMonster::Jump_Land()
{
	if (JumpAudioComponent && JumpAudioComponent->IsPlaying())
	{
		JumpAudioComponent->Stop();
	}

	if (!IsValid(CurrentIndicator))
	{
		if(cachedAnim) cachedAnim->isAttack = false;
		return;
	}

	JumpLandLocation = CurrentIndicator->GetActorLocation();
	
	// Adjust Z to prevent sinking (Capsule Center = Ground Z + Half Height)
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	if (bIsJumpAttackP2)
	{
		// P2 attack: adjust by HalfHeight - 5.0f (솟아오르는 높이 미세조정)
		JumpLandLocation.Z += (CapsuleHalfHeight - 5.0f);
	}

	FVector StartLandLocation = JumpLandLocation;
	StartLandLocation.Z = CurrentJumpHeight;
	SetActorLocation(StartLandLocation);

	// Make actor visible and enable collision
	SetActorHiddenInGame(false);

	// Play land montage
	if (cachedAnim && cachedAnim->JumpEndMontage)
	{
		cachedAnim->Montage_Play(cachedAnim->JumpEndMontage);
	}

	bIsJumpingDown = true;

}

void ABossMonster::Jump_FinishLanding()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	
	if (IsValid(CurrentIndicator))
	{
		CurrentIndicator->CheckForPlayerAndDamage(GetController(), this);
		CurrentIndicator->Destroy();
		CurrentIndicator = nullptr;
	}
	// Check for damage and destroy indicator

	// End of jump
	if(cachedAnim)
	{
		cachedAnim->isAttack = false;
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	if (bIsJumpAttackP2 || currentPhase == 3)
	{
		MonsterFire_P2();
	}
}

