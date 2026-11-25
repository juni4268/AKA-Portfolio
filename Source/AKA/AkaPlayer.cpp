#include "AkaPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LightningBolt.h"
#include "BossMonster.h"
#include "PlayerAnimInstance.h"
#include "BossMonsterAnim.h"
#include "PlayerWidget.h"
#include "Components/Button.h"
#include "Misc/Timespan.h"
#include "AKAGameInstance.h"

AAkaPlayer::AAkaPlayer()
{
	bIsAttacking = false;
	AttackCooldown = 0.5f;
	LastAttackTime = 0.f;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	TimerElapsedTime = 0.0f;
	bIsTimerActive = false;
	bWasTimerActiveBeforePause = false;

	BusterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BusterMesh"));
	BusterMesh->SetupAttachment(GetMesh(), TEXT("Buster_RSocket"));
}

void AAkaPlayer::BeginPlay()
{
	Super::BeginPlay();
	currentHP = maxHP;

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (inputMappingContext)
			{
				Subsystem->AddMappingContext(inputMappingContext, 0);
			}
		}

		AActor* MainCamera = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
		if (MainCamera)
		{
			PlayerController->SetViewTarget(MainCamera);
		}

	}

	// Create the widget and add it to the viewport. 
	// It will be hidden by default from its own NativeConstruct.
	if (PlayerWidgetClass)
	{
		PlayerWidgetInstance = CreateWidget<UPlayerWidget>(GetWorld(), PlayerWidgetClass);
		if (PlayerWidgetInstance)
		{
			PlayerWidgetInstance->AddToViewport();
			PlayerWidgetInstance->UpdateHP(currentHP, maxHP);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create PlayerWidgetInstance."));
		}
	}

	PlayerAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
}

void AAkaPlayer::ActivateGameplayHUD(bool bIsActive)
{
	if (PlayerWidgetInstance)
	{
		PlayerWidgetInstance->ShowGameplayUI(bIsActive);
		PlayerWidgetInstance->ShowTimer(bIsActive);
	}
	if (bIsActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Gameplay HUD Activated"));
		StartTimer();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Gameplay HUD Deactivated"));	
		StopTimer();
	}
}

void AAkaPlayer::ShowLapTimeUI(bool bShow)
{
	if (PlayerWidgetInstance)
	{
		PlayerWidgetInstance->SetLapTimeVisibility(bShow);
	}
}


void AAkaPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDeath) return;

	if (bIsAttacking)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;

		if (!AttackDirection.IsNearlyZero())
		{
			//SetActorRotation(AttackDirection.Rotation());
		}

		if (GetWorld()->GetTimeSeconds() >= LastAttackTime + AttackCooldown)
		{
			Fire();
			SetActorRotation(AttackDirection.Rotation());
			LastAttackTime = GetWorld()->GetTimeSeconds();
		}
	}
	else if (PlayerAnimInstance->bAttackRotation)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
	}

	// Update timer logic
	if (bIsTimerActive)
	{
		TimerElapsedTime += DeltaTime;
		if (PlayerWidgetInstance)
		{
			// 시간을 MM:SS.ms 형식으로 포맷합니다.
			FTimespan Timespan = FTimespan::FromSeconds(TimerElapsedTime);
			FText FormattedTime = FText::FromString(FString::Printf(TEXT("%02d:%02d.%03d"),
				Timespan.GetMinutes(), Timespan.GetSeconds(), Timespan.GetFractionMilli()));
			
			// 포맷된 텍스트를 위젯으로 전달합니다.
			PlayerWidgetInstance->UpdateTimer(FormattedTime);
		}
	}
}

void AAkaPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(moveAction, ETriggerEvent::Triggered, this, &AAkaPlayer::InputActionMove);
		EnhancedInputComponent->BindAction(attackAction, ETriggerEvent::Triggered, this, &AAkaPlayer::InputActionAttack);
		EnhancedInputComponent->BindAction(attackAction, ETriggerEvent::Started, this, &AAkaPlayer::CheckAttack);
		EnhancedInputComponent->BindAction(attackAction, ETriggerEvent::Completed, this, &AAkaPlayer::OnAttackCompleted);
		EnhancedInputComponent->BindAction(pauseAction, ETriggerEvent::Started, this, &AAkaPlayer::InputActionPause);
		EnhancedInputComponent->BindAction(lapTimeAction, ETriggerEvent::Started, this, &AAkaPlayer::InputActionLapTime);
	}
}

void AAkaPlayer::InputActionMove(const FInputActionValue& Value)
{
	if (bIsDeath) return;
	if (Controller != nullptr)
	{
		FVector2D MovementVector = Value.Get<FVector2D>();
		DoMove(MovementVector.X, MovementVector.Y);
	}
}

void AAkaPlayer::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AAkaPlayer::InputActionAttack(const FInputActionValue& Value)
{
	if (bIsDeath) return;
	bIsAttacking = true;
	const FVector2D AimAxis = Value.Get<FVector2D>();
	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	//if (FMath::Abs(AimAxis.X) > FMath::Abs(AimAxis.Y))
	//{
	//	AttackDirection = (AimAxis.X > 0) ? RightDirection : -RightDirection;
	//}
	//else
	//{
	//	AttackDirection = (AimAxis.Y > 0) ? ForwardDirection : -ForwardDirection;
	//}
	//밢  
	AttackDirection = (ForwardDirection * AimAxis.Y + RightDirection * AimAxis.X).GetSafeNormal();
}

void AAkaPlayer::CheckAttack(const FInputActionValue& Value)
{
	if (bIsDeath) return;
	NumAttackKeysDown++;
}

void AAkaPlayer::OnAttackCompleted(const FInputActionValue& Value)
{
	if (bIsDeath) return;
	NumAttackKeysDown--;

	if (NumAttackKeysDown <= 0)
	{
		bIsAttacking = false;
		NumAttackKeysDown = 0;
	}
}

void AAkaPlayer::InputActionPause(const FInputActionValue& Value)
{
	bool bIsCurrentlyPaused = UGameplayStatics::IsGamePaused(GetWorld());
	UE_LOG(LogTemp, Warning, TEXT("InputActionPause: Current Game Paused State (before SetPause) = %s"), bIsCurrentlyPaused ? TEXT("TRUE") : TEXT("FALSE"));
	SetPause(!bIsCurrentlyPaused);
}

void AAkaPlayer::SetPause(bool bToPause)
{
	UE_LOG(LogTemp, Warning, TEXT("SetPause called with bToPause = %s"), bToPause ? TEXT("TRUE") : TEXT("FALSE"));
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (bToPause) // This block is for PAUSING the game
		{
			if (PlayerWidgetInstance)
			{
				PlayerWidgetInstance->ShowPauseMenu(true);
			}
			UGameplayStatics::SetGamePaused(GetWorld(), true);
			PlayerController->SetInputMode(FInputModeGameAndUI()); // Keep this as it's the right input mode.
			PlayerController->SetShowMouseCursor(true);
			NumAttackKeysDown = 0;
			bWasTimerActiveBeforePause = bIsTimerActive;
			StopTimer();

			// Set initial keyboard focus using the simpler method.
			if (PlayerWidgetInstance && PlayerWidgetInstance->ReturnButton)
			{
				PlayerWidgetInstance->ReturnButton->SetKeyboardFocus();
			}
		}
		else // This block is for UNPAUSING the game
		{
			if (PlayerWidgetInstance)
			{
				PlayerWidgetInstance->ShowPauseMenu(false);
			}
			UGameplayStatics::SetGamePaused(GetWorld(), false);
			UE_LOG(LogTemp, Warning, TEXT("Game Unpaused. New Game Paused State = %s"), UGameplayStatics::IsGamePaused(GetWorld()) ? TEXT("TRUE") : TEXT("FALSE"));
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->SetShowMouseCursor(false);
			
			if (bWasTimerActiveBeforePause)
			{
				StartTimer();
			}

		}
	}
}
void AAkaPlayer::Death()
{
	bIsDeath = true;
	bIsAttacking = false;
	NumAttackKeysDown = 0;
	currentHP = 0.0f;
	if (PlayerAnimInstance)
	{
		PlayerAnimInstance->PlayerDeathMontage(TEXT("DeathStart"));
	}
	UE_LOG(LogTemp, Warning, TEXT("Player has died."));
	StopTimer();

	// Delay the Game Over UI to allow death animation to play
	FTimerHandle DeathUITimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DeathUITimerHandle, [this]()
	{
		if (PlayerWidgetInstance)
		{
			PlayerWidgetInstance->ShowGameOverUI(true);
		}
	}, 3.0f, false);
}

void AAkaPlayer::GameClear()
{
	UE_LOG(LogTemp, Warning, TEXT("Game Cleared!"));
	StopTimer();
	RecordLapTime();

	// Delay the Game Clear UI to allow boss death animation/victory pose
	FTimerHandle VictoryUITimerHandle;
	GetWorld()->GetTimerManager().SetTimer(VictoryUITimerHandle, [this]()
	{
		if (PlayerWidgetInstance)
		{
			PlayerWidgetInstance->ShowGameClearUI(true);
		}
	}, 4.0f, false);
}


float AAkaPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (bIsDeath) return DamageAmount;
	currentHP -= DamageAmount;

	if (PlayerWidgetInstance)
	{
		PlayerWidgetInstance->UpdateHP(currentHP, maxHP);
	}
	if (currentHP <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player has died."));
		Death();
		return DamageAmount;
	}

	OnTakeDamage_Event();
	UE_LOG(LogTemp, Warning, TEXT("Player HP: %f"), currentHP);
	if (PlayerAnimInstance)
	{
		PlayerAnimInstance->PlayerHitMontage();
	}
	ABossMonster* Boss = Cast<ABossMonster>(DamageCauser);
	if (Boss)
	{
		// It's a direct hit from the boss, apply knockback
		UE_LOG(LogTemp, Warning, TEXT("Player hit by Boss, applying knockback."));
		BossLocationX = Boss->GetActorLocation().X;
		BossLocationY = Boss->GetActorLocation().Y;
		if (GetActorLocation().X - BossLocationX > 0)
		{
			LaunchPowerX += 200.0f;
		}
		else
		{
			LaunchPowerX -= 200.0f;
		}

		if (GetActorLocation().Y - BossLocationY > 0)
		{
			LaunchPowerY += 200.0f;
		}
		else
		{
			LaunchPowerY -= 200.0f;
		}
		LaunchCharacter(FVector(LaunchPowerX, LaunchPowerY, 50), true, true);
	}
	else
	{
		// Damage from other sources (like projectiles), no knockback
		UE_LOG(LogTemp, Warning, TEXT("Player hit by projectile, no knockback."));
	}

	return DamageAmount;
}

void AAkaPlayer::OnTakeDamage_Event_Implementation()
{
	// This function can be left empty.
	// The Blueprint event will be called, and you can override it there.
	// You can also add a base C++ implementation here if needed in the future.
}

void AAkaPlayer::StartTimer() { bIsTimerActive = true; }

void AAkaPlayer::StopTimer() { bIsTimerActive = false; }

void AAkaPlayer::ResetTimer()
{
	bIsTimerActive = false;
	TimerElapsedTime = 0.0f;
	LapTimes.Empty();
	if (PlayerWidgetInstance)
	{
		// 포맷된 빈 텍스트 배열을 전달합니다.
		TArray<FText> FormattedLapTimes;
		PlayerWidgetInstance->UpdateLapTimes(FormattedLapTimes);

		// 타이머 디스플레이도 0으로 리셋합니다.
		PlayerWidgetInstance->UpdateTimer(FText::FromString(TEXT("00:00.000")));
	}
}
void AAkaPlayer::RecordLapTime()
{
	UE_LOG(LogTemp, Warning, TEXT("RecordLapTime Called. TimerElapsedTime: %f"), TimerElapsedTime);

	if (TimerElapsedTime > 0.0f)
	{
		// Save to GameInstance
		UAKAGameInstance* GI = Cast<UAKAGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->AddLapTime(BossLevel, TimerElapsedTime);

			if (PlayerWidgetInstance)
			{
				UE_LOG(LogTemp, Warning, TEXT("Updating UI with %d records from GameInstance."), GI->LapTimeRecords.Num());
				TArray<FText> FormattedLapTimes;
				
				for (const FLapTimeData& Record : GI->LapTimeRecords)
				{
					FTimespan Timespan = FTimespan::FromSeconds(Record.LapTime);
					// Format: "Lv.5 - 01:23.456"
					FText FormattedTime = FText::FromString(FString::Printf(TEXT("Lv.%d - %02d:%02d.%03d"),
						Record.BossLevel, Timespan.GetMinutes(), Timespan.GetSeconds(), Timespan.GetFractionMilli()));
					FormattedLapTimes.Add(FormattedTime);
				}
				PlayerWidgetInstance->UpdateLapTimes(FormattedLapTimes);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameInstance is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TimerElapsedTime is 0. Cannot record lap time."));
	}
}

void AAkaPlayer::InputActionLapTime(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("LapTime Key Pressed!"));
	AActor* Boss = UGameplayStatics::GetActorOfClass(GetWorld(), ABossMonster::StaticClass());
	if (Boss && !Cast<ABossMonster>(Boss)->bIsDeath)
	{
		UE_LOG(LogTemp, Warning, TEXT("LapTime Rejected: Boss is still alive!"));
		// Boss is alive, do not record lap time
		return;
	}
	if (bHasRecordedLap)
	{
		UE_LOG(LogTemp, Warning, TEXT("LapTime Rejected: Lap time already recorded for this run!"));
		return;
	}
	RecordLapTime();
	bHasRecordedLap = true;
}