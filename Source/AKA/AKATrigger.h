// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AKATrigger.generated.h"

class UBoxComponent;
class ABossMonster;
class UBehaviorTree;

	// Define the Enum for the trigger types with a unique name
UENUM(BlueprintType)
enum class EAKATriggerType : uint8
{
	Play,
	Tutorial,
	Back,
	LapTime
};



UCLASS()
class AKA_API AAKATrigger : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AAKATrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	FTimerHandle EndOverlapTimer;
	void EndOverlapActions();

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* TriggerVolume;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UTextRenderComponent* TextRenderComp;

	// The ID that defines what this trigger does. Editable in the editor.
	UPROPERTY(EditAnywhere, Category = "Trigger Settings")
	EAKATriggerType TriggerType;

	// --- Settings for all types ---
	UPROPERTY(EditAnywhere, Category = "Trigger Settings", meta = (MakeEditWidget = true))
	FVector PlayerTutorialLocation;
	UPROPERTY(EditAnywhere, Category = "Trigger Settings", meta = (MakeEditWidget = true))
	FVector PlayerPlayLocation;
	UPROPERTY(EditAnywhere, Category = "Trigger Settings", meta = (MakeEditWidget = true))
	FVector PlayerBackLocation;

	// --- Camera Settings ---
	UPROPERTY(EditAnywhere, Category = "Trigger Settings|Camera")
	FName TargetCameraTag;

	UPROPERTY(EditAnywhere, Category = "Trigger Settings|Camera", meta = (ClampMin = "0.0"))
	float CameraBlendTime = 1.0f;


	// --- Settings for Play & Tutorial ---
	UPROPERTY(EditAnywhere, Category = "Trigger Settings|Spawning")
	FTransform BossSpawnTransform;

	UPROPERTY(EditAnywhere, Category = "Trigger Settings|Spawning", meta = (EditCondition = "TriggerType == EAKATriggerType::Play"))
	TSubclassOf<ABossMonster> Play_BossToSpawn;

	UPROPERTY(EditAnywhere, Category = "Trigger Settings|Spawning", meta = (EditCondition = "TriggerType == EAKATriggerType::Tutorial"))
	TSubclassOf<ABossMonster> Tutorial_BossToSpawn;

	// To prevent the trigger from firing multiple times
	bool bHasBeenTriggered = false;
};