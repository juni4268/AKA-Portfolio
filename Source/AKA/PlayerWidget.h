// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidget.generated.h"

class UProgressBar;
class ABossMonster;
class UOverlay;
class UTextBlock;
class UButton;
UCLASS()
class AKA_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// This is based on the .cpp file. These properties need to be bound in the UMG editor.
	UPROPERTY(meta = (BindWidget))
	UOverlay* PlayerHP;

	UPROPERTY(meta = (BindWidget))
	UOverlay* BossHP;

	UPROPERTY(meta = (BindWidget))
	UOverlay* Pause;

	UPROPERTY(meta = (BindWidget))
	UOverlay* GameOver;

	UPROPERTY(meta = (BindWidget))
	UOverlay* GameClear;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TimerText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HP_Bar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* BossHP_Bar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UOverlay* LapTime;

	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;
	UPROPERTY(meta = (BindWidget))
	UButton* RetryButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	// This is a C++ function called by AkaPlayer
	void UpdateHP(float current, float max);

	// Functions to control visibility of specific HUD elements
	void ShowGameplayUI(bool bShow);
	void ShowPlayerUI(bool bShow);
	void ShowBossUI(bool bShow);
	void ShowTimer(bool bShow);
	void SetLapTimeVisibility(bool bIsVisible);

	// New function to bind this widget to a specific boss instance
	void BindToBoss(ABossMonster* Boss);

	UFUNCTION(BlueprintImplementableEvent, Category = "Timer")
	void UpdateTimer(const FText& FormattedTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "Timer")
	void UpdateLapTimes(const TArray<FText>& FormattedLapTimes);
	void ShowPauseMenu(bool bShow);
	void ShowGameOverUI(bool bShow);
	void ShowGameClearUI(bool bShow);
	void ReturnButtonClicked();
	void RetryButtonClicked();
	void ExitButtonClicked();
protected:
	// These functions are called from the engine.
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	// These are the internal state variables for smoothing the progress bars.
	float CurrentHPPercent;
	float TargetHPPercent;
	float CurrentBossHPPercent;
	float TargetBossHPPercent;

	// This function is bound to the boss's delegate.
	UFUNCTION()
	void UpdateBossHP(float InCurrentHP, float InMaxHP);
};
