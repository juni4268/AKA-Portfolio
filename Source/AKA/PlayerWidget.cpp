// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "BossMonster.h"
#include "AkaPlayer.h"
#include "Components/Button.h"


void UPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CurrentHPPercent = 1.0f; // Assume full HP at start
	TargetHPPercent = 1.0f;
	CurrentBossHPPercent = 1.0f;
	TargetBossHPPercent = 1.0f;

	// Set default visibility for individual components
	if (PlayerHP)
	{
		PlayerHP->SetVisibility(ESlateVisibility::Hidden);
	}
	if (BossHP)
	{
		BossHP->SetVisibility(ESlateVisibility::Hidden);
	}

	if (LapTime)
	{
		LapTime->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Pause)
	{
		Pause->SetVisibility(ESlateVisibility::Hidden);
	}
	if (GameOver)
	{
		GameOver->SetVisibility(ESlateVisibility::Hidden);
	}
	if (GameClear)
	{
		GameClear->SetVisibility(ESlateVisibility::Hidden);
	}
	ReturnButton->OnClicked.AddDynamic(this, &UPlayerWidget::ReturnButtonClicked);
	RetryButton->OnClicked.AddDynamic(this, &UPlayerWidget::RetryButtonClicked);
	ExitButton->OnClicked.AddDynamic(this, &UPlayerWidget::ExitButtonClicked);
}

void UPlayerWidget::ShowPauseMenu(bool bShow)
{
	const ESlateVisibility NewVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	if (Pause)
	{
		Pause->SetVisibility(NewVisibility);
	}
}

void UPlayerWidget::ShowGameClearUI(bool bShow)
{
	const ESlateVisibility NewVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	if (GameClear)
	{
		GameClear->SetVisibility(NewVisibility);
	}

	if (bShow)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			PlayerController->SetShowMouseCursor(true);
			PlayerController->SetInputMode(FInputModeUIOnly());
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
}

void UPlayerWidget::ShowGameOverUI(bool bShow)
{
	const ESlateVisibility NewVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	if (GameOver)
	{
		GameOver->SetVisibility(NewVisibility);
	}

	if (bShow)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			PlayerController->SetShowMouseCursor(true);
			PlayerController->SetInputMode(FInputModeUIOnly());
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
}

void UPlayerWidget::ReturnButtonClicked()
{

}

void UPlayerWidget::RetryButtonClicked()
{

}

void UPlayerWidget::ExitButtonClicked()
{

}
void UPlayerWidget::BindToBoss(ABossMonster* Boss)
{
	if (Boss)
	{
		// Bind our UpdateBossHP function to the boss's OnBossHPChanged event.
		Boss->OnBossHPChanged.AddDynamic(this, &UPlayerWidget::UpdateBossHP);

		// Immediately update the HP bar to the boss's current state
		UpdateBossHP(Boss->currentHP, Boss->maxHP);

		UE_LOG(LogTemp, Warning, TEXT("PlayerWidget successfully bound to spawned boss and HP updated."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerWidget failed to bind to boss (Boss was null)."));
	}
}

void UPlayerWidget::ShowGameplayUI(bool bShow)
{
	ShowPlayerUI(bShow);
	ShowBossUI(bShow);
}

void UPlayerWidget::ShowPlayerUI(bool bShow)
{
	const ESlateVisibility NewVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	if (PlayerHP)
	{
		PlayerHP->SetVisibility(NewVisibility);
	}
}

void UPlayerWidget::ShowBossUI(bool bShow)
{
	const ESlateVisibility NewVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	if (BossHP)
	{
		BossHP->SetVisibility(NewVisibility);
	}
}

void UPlayerWidget::ShowTimer(bool bShow)
{
	if (TimerText)
	{
		TimerText->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UPlayerWidget::SetLapTimeVisibility(bool bIsVisible)
{
	if (LapTime)
	{
		LapTime->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UPlayerWidget::UpdateHP(float InCurrentHP, float InMaxHP)
{
	TargetHPPercent = InCurrentHP / InMaxHP;
}

void UPlayerWidget::UpdateBossHP(float InCurrentHP, float InMaxHP)
{
	TargetBossHPPercent = InCurrentHP / InMaxHP;
}

void UPlayerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (HP_Bar)
	{
		// Smoothly interpolate CurrentHPPercent towards TargetHPPercent
		CurrentHPPercent = FMath::FInterpTo(CurrentHPPercent, TargetHPPercent, InDeltaTime, 5.0f); // 5.0f is interpolation speed
		HP_Bar->SetPercent(CurrentHPPercent);
	}
	if (BossHP_Bar)
	{
		// Smoothly interpolate CurrentBossHPPercent towards TargetBossHPPercent
		CurrentBossHPPercent = FMath::FInterpTo(CurrentBossHPPercent, TargetBossHPPercent, InDeltaTime, 5.0f); // 5.0f is interpolation speed
		BossHP_Bar->SetPercent(CurrentBossHPPercent);
	}
}