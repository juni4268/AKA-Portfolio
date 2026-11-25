// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_UpdateCombat.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"

UBTService_UpdateCombat::UBTService_UpdateCombat()
{
    NodeName = "Update Combat Location";
    Interval = 0.5f;
    RandomDeviation = 0.1f;
}

void UBTService_UpdateCombat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (AIController == nullptr || BlackboardComp == nullptr)
    {
        return;
    }

    APawn* AIPawn = AIController->GetPawn();
    AActor* PlayerActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));

    if (AIPawn == nullptr || PlayerActor == nullptr)
    {
        return;
    }

    FVector AIPos = AIPawn->GetActorLocation();
    FVector PlayerPos = PlayerActor->GetActorLocation();
    FVector VecToPlayer = PlayerPos - AIPos;
    VecToPlayer.Z = 0; // Ignore height difference
    
    FVector DirectionFromPlayerToAI = (AIPos - PlayerPos).GetSafeNormal();
    FVector IdealLocation = PlayerPos + (DirectionFromPlayerToAI * IdealDistance);

    FHitResult HitResult;
    bool bHitWall = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        AIPos,
        IdealLocation,
        ECC_Visibility // Trace against walls and static objects
    );

    FVector FinalTargetLocation = AIPos; // Default to current position
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation NavLocation;

    if (NavSys)
    {
        // Check if the IdealLocation is valid on the NavMesh
        // Using a small extent ensures we check the exact spot, not somewhere far away
        bool bProjectSuccess = NavSys->ProjectPointToNavigation(IdealLocation, NavLocation, FVector(100.f, 100.f, 100.f));

        if (!bHitWall && bProjectSuccess)
        {
            // Path is clear and destination is valid
            FinalTargetLocation = NavLocation.Location;
        }
        else
        {
            // Either wall blocked or destination invalid. Try to find a valid point nearby.
            // Try strafing left/right
            FVector RightVector = AIPawn->GetActorRightVector();
            float StrafeDirection = FMath::RandBool() ? 1.0f : -1.0f;
            FVector StrafeLocation = AIPos + (RightVector * StrafeDistance * StrafeDirection);

            // Ensure StrafeLocation is on NavMesh
            if (NavSys->ProjectPointToNavigation(StrafeLocation, NavLocation, FVector(200.f, 200.f, 200.f)))
            {
                FinalTargetLocation = NavLocation.Location;
            }
            else
            {
                // Fallback: Just stay put if we can't find a good spot
                FinalTargetLocation = AIPos;
            }
        }
    }

    // Update the blackboard with the final calculated location
    BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, FinalTargetLocation);
}
