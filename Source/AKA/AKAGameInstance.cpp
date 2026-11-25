// Fill out your copyright notice in the Description page of Project Settings.


#include "AKAGameInstance.h"
#include "Sound/SoundWave.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UAKAGameInstance::UAKAGameInstance()
{
	CurrentBossLevel = 0; // Initialize to 0 so first ResetBoss() makes it Level 1

	// BGM Load (for default values in editor, can be overridden in BP)
	static ConstructorHelpers::FObjectFinder<USoundWave> MainBGMAsset(TEXT("/Script/Engine.SoundWave'/Game/Sound/BGM_Main.BGM_Main'"));
	if (MainBGMAsset.Succeeded())
	{
		MainBGM = MainBGMAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundWave> BattleBGMAsset(TEXT("/Script/Engine.SoundWave'/Game/Sound/BGM_Battle.BGM_Battle'"));
	if (BattleBGMAsset.Succeeded())
	{
		BattleBGM = BattleBGMAsset.Object;
	}
}

void UAKAGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Warning, TEXT("AKAGameInstance::Init Called!"));
	// Audio initialization moved to explicit call (e.g. from GameMode or Level BP)
	// because GetWorld() might not be ready here.
}

void UAKAGameInstance::Shutdown()
{
	Super::Shutdown();

	if (IsValid(BGM_AudioComponent))
	{
		BGM_AudioComponent->Stop();
		// CreateSound2D components are usually managed by the audio engine, 
		// but since we keep a pointer, nulling it is good practice.
		BGM_AudioComponent = nullptr;
	}
}

void UAKAGameInstance::PlayBGM(USoundWave* NewBGM, float FadeInDuration, float StartTime)
{
	if (!IsValid(NewBGM))
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayBGM: New BGM is not valid. Cannot play."));
		return;
	}

	// If component doesn't exist (e.g. MainBGM was null at Init), create it now
	if (!IsValid(BGM_AudioComponent))
	{
		BGM_AudioComponent = UGameplayStatics::CreateSound2D(this, NewBGM, 0.8f, 1.0f, 0.0f, nullptr, true, false);
		if (BGM_AudioComponent)
		{
			BGM_AudioComponent->bShouldRemainActiveIfDropped = true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PlayBGM: Failed to create BGM_AudioComponent."));
			return;
		}
	}

	// If the new BGM is already playing, just ensure it's playing and exit.
	if (BGM_AudioComponent->Sound == NewBGM && BGM_AudioComponent->IsPlaying())
	{
		return;
	}

	// Stop current BGM if playing
	if (BGM_AudioComponent->IsPlaying())
	{
		BGM_AudioComponent->FadeOut(1.0f, 0.0f); // Fast fade out (1.0s)
	}
	
	// Set new BGM and play
	BGM_AudioComponent->SetSound(NewBGM);
	
	// FadeIn takes (Duration, TargetVolume)
	// We use 1.0f as target volume.
	BGM_AudioComponent->FadeIn(FadeInDuration, 1.0f, StartTime);
}

void UAKAGameInstance::StopBGM(float FadeOutDuration)
{
	if (IsValid(BGM_AudioComponent) && BGM_AudioComponent->IsPlaying())
	{
		BGM_AudioComponent->FadeOut(FadeOutDuration, 0.0f);
	}
}

void UAKAGameInstance::AddLapTime(int32 Level, float Time)
{
	FLapTimeData NewRecord;
	NewRecord.BossLevel = Level;
	NewRecord.LapTime = Time;

	LapTimeRecords.Add(NewRecord);

	// Sort by BossLevel (Ascending) to show progression
	LapTimeRecords.Sort([](const FLapTimeData& A, const FLapTimeData& B)
	{
		return A.BossLevel < B.BossLevel;
	});
}

float UAKAGameInstance::GetTotalPlayTime() const
{
	float TotalTime = 0.0f;
	for (const FLapTimeData& Record : LapTimeRecords)
	{
		TotalTime += Record.LapTime;
	}
	return TotalTime;
}
