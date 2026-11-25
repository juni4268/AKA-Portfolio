// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleWidget.generated.h"

class UButton;
class UWidgetSwitcher;

/**
 * 
 */
UCLASS()
class AKA_API UTitleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UButton* GameStartButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SettingButton;

    UPROPERTY(meta = (BindWidget))
    UButton* GameEndButton;
	
    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
    UWidgetSwitcher* MenuSwitcher;

	UFUNCTION()
	void OnGameStartClicked();

	UFUNCTION()
	void OnSettingClicked();

	UFUNCTION()
	void OnGameEndClicked();

	UFUNCTION()
	void OnBackClicked();
};
