// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UTitleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (GameStartButton)
    {
        GameStartButton->OnClicked.AddDynamic(this, &UTitleWidget::OnGameStartClicked);
    }

    if (SettingButton)
    {
        SettingButton->OnClicked.AddDynamic(this, &UTitleWidget::OnSettingClicked);
    }

    if (GameEndButton)
    {
        GameEndButton->OnClicked.AddDynamic(this, &UTitleWidget::OnGameEndClicked);
    }

	if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UTitleWidget::OnBackClicked);
    }
}

void UTitleWidget::OnGameStartClicked()
{
    // Implement Game Start Logic (e.g., Open Level)
    UGameplayStatics::OpenLevel(this, FName("DungeonMap")); 
}

void UTitleWidget::OnSettingClicked()
{
    // Switch to Setting Menu
	if (MenuSwitcher)
	{
		MenuSwitcher->SetActiveWidgetIndex(1);
	}
}

void UTitleWidget::OnGameEndClicked()
{
    // Implement Game End Logic
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}

void UTitleWidget::OnBackClicked()
{
	// Switch back to Main Menu
	if (MenuSwitcher)
	{
		MenuSwitcher->SetActiveWidgetIndex(0);
	}
}
