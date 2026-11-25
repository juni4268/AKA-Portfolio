// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AKAGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FLapTimeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BossLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime;
};

/**
 * 
 */
UCLASS()
class AKA_API UAKAGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UAKAGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundWave* MainBGM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	class USoundWave* BattleBGM;

	UPROPERTY()
	class UAudioComponent* BGM_AudioComponent;

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayBGM(USoundWave* NewBGM, float FadeInDuration = 1.0f, float StartTime = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopBGM(float FadeOutDuration = 1.0f);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 CurrentBossLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	TArray<FLapTimeData> LapTimeRecords;

	UFUNCTION(BlueprintCallable, Category = "GameData")
	void AddLapTime(int32 Level, float Time);

	UFUNCTION(BlueprintCallable, Category = "GameData")
	float GetTotalPlayTime() const;
};
