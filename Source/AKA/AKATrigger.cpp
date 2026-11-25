// Fill out your copyright notice in the Description page of Project Settings.

#include "AKATrigger.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "AkaPlayer.h"
#include "BossMonster.h"
#include "PlayerWidget.h" // Include the widget header
#include "BossAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"

// Sets default values
AAKATrigger::AAKATrigger()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));

	TextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRenderComp"));
	TextRenderComp->SetupAttachment(RootComponent);
	TextRenderComp->SetText(FText::FromString(TEXT("Trigger")));
	TextRenderComp->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextRenderComp->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
	TextRenderComp->SetWorldSize(50.0f);
	TextRenderComp->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Position above the trigger volume
}

// Called when the game starts or when spawned
void AAKATrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAKATrigger::OnOverlapBegin);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AAKATrigger::OnOverlapEnd);
}

void AAKATrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAkaPlayer* Player = Cast<AAkaPlayer>(OtherActor);
	if (Player == nullptr)
	{
		return;
	}

	// Prevent re-entry from same-frame overlaps by checking and setting the flag at the top.
	// This ensures the trigger logic executes only once per activation, even if multiple components overlap simultaneously.
	if (bHasBeenTriggered)
	{
		return;
	}
	bHasBeenTriggered = true; // Set the flag immediately after passing the check


	// Broadcast the event that the portal has been triggered
	// OnPortalTriggered.Broadcast(); // This was requested to be removed previously.


	switch (TriggerType)
	{
		case EAKATriggerType::Play:
		{
			// 1. Move Player
			Player->SetActorLocation(PlayerPlayLocation);

			// 2. Reset Existing Boss for "Play" mode
			ABossMonster* ExistingBoss = Cast<ABossMonster>(UGameplayStatics::GetActorOfClass(GetWorld(), ABossMonster::StaticClass()));
			
			if (ExistingBoss)
			{
				UE_LOG(LogTemp, Warning, TEXT("AKATrigger: Found existing boss. Resetting..."));
				ExistingBoss->ResetBoss();

				// 3. Bind the UI to the existing boss
				if (Player->PlayerWidgetInstance)
				{
					Player->PlayerWidgetInstance->BindToBoss(ExistingBoss);
				}
			}
			else
			{
				// Fallback: Spawn if no boss exists (e.g. first time loading level if not placed in map)
				UE_LOG(LogTemp, Warning, TEXT("AKATrigger: No boss found. Spawning new one..."));
				if (Play_BossToSpawn)
				{
					ABossMonster* SpawnedBoss = GetWorld()->SpawnActor<ABossMonster>(Play_BossToSpawn, BossSpawnTransform);
					if (SpawnedBoss && Player->PlayerWidgetInstance)
					{
						Player->PlayerWidgetInstance->BindToBoss(SpawnedBoss);
					}
				}
			}

			// 4. Activate UI
			Player->ActivateGameplayHUD(true);
			Player->bHasRecordedLap = false;
			GetWorld()->GetTimerManager().SetTimer(EndOverlapTimer, this, &AAKATrigger::EndOverlapActions, 1.0f, false, 1.f);
			break;
		}

		case EAKATriggerType::Tutorial:
		{
			// Tutorial logic completely removed as requested
			UE_LOG(LogTemp, Warning, TEXT("AKATrigger: Tutorial trigger activated, but logic is empty."));
			bHasBeenTriggered = false; // Allow re-triggering if no specific action
			break;
		}

		case EAKATriggerType::Back:
		{
			// 1. Move Player
			Player->SetActorLocation(PlayerBackLocation);

			// 2. Kill Boss
			ABossMonster* ExistingBoss = Cast<ABossMonster>(UGameplayStatics::GetActorOfClass(GetWorld(), ABossMonster::StaticClass()));
			if (ExistingBoss)
			{
				ExistingBoss->Death(); // Call the existing death function
			}

			// 3. Deactivate UI
			Player->ActivateGameplayHUD(false);
			GetWorld()->GetTimerManager().SetTimer(EndOverlapTimer, this, &AAKATrigger::EndOverlapActions, 3.0f, false, 1.f);
			break;	
		}

		case EAKATriggerType::LapTime:
		{
			Player->ShowLapTimeUI(true);
			// LapTime does not use the shared camera switch logic, so we return early.
			// This trigger type needs to be re-enterable, so reset the flag immediately.
			bHasBeenTriggered = false; 
			// No timer is set for LapTime, so bHasBeenTriggered needs to be reset here.
			break;
		}
		default:
			// For unhandled or temporary types, reset the flag.
			bHasBeenTriggered = false; 
			break;
	}

	// --- After handling the specific trigger type, handle the camera switch ---
	if (!TargetCameraTag.IsNone())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), TargetCameraTag, FoundActors);

		if (FoundActors.Num() > 0)
		{
			ACameraActor* CameraToFind = Cast<ACameraActor>(FoundActors[0]);
			if (CameraToFind)
			{
				APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
				if (PlayerController)
				{
					PlayerController->SetViewTargetWithBlend(CameraToFind, CameraBlendTime, EViewTargetBlendFunction::VTBlend_Cubic);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AKATrigger: Found actor with tag '%s' but it was not a CameraActor."), *TargetCameraTag.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AKATrigger: Could not find any actor with tag '%s'."), *TargetCameraTag.ToString());
		}
	}
}

void AAKATrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AAkaPlayer* Player = Cast<AAkaPlayer>(OtherActor);
	if (Player == nullptr)
	{
		return;
	}

	// Use a switch for consistency.
	switch (TriggerType)
	{
		case EAKATriggerType::LapTime:
		{
			// Hide the UI when the player leaves the volume
			Player->ShowLapTimeUI(false);
			break;
		}
		default:
			// Other trigger types don't have an OnOverlapEnd behavior.
			break;
	}
}

void AAKATrigger::EndOverlapActions()
{
	bHasBeenTriggered = false;
}
