// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_CheckLineOfSight.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"

UBTDecorator_CheckLineOfSight::UBTDecorator_CheckLineOfSight()
{
    NodeName = "Check Line Of Sight";
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckLineOfSight, TargetActorKey), AActor::StaticClass());
}

bool UBTDecorator_CheckLineOfSight::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController == nullptr)
    {
        return false;
    }

    APawn* ControllingPawn = AIController->GetPawn();
    if (ControllingPawn == nullptr)
    {
        return false;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (BlackboardComp == nullptr)
    {
        return false;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (TargetActor == nullptr)
    {
        // If there is no target, we can consider LOS "clear" or "blocked" based on desired default behavior.
        // Let's say it's blocked if there's no target.
        return false;
    }

    FHitResult HitResult;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(ControllingPawn);

    // Perform a line trace from the pawn's eyes to the target's location
    FVector StartLocation = ControllingPawn->GetPawnViewLocation();
    FVector EndLocation = TargetActor->GetActorLocation();

    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        ControllingPawn->GetWorld(),
        StartLocation,
        EndLocation,
        UEngineTypes::ConvertToTraceType(ECC_Visibility), // Check for any visible obstacle
        false, // bTraceComplex
        ActorsToIgnore,
        EDrawDebugTrace::None, // You can change this to EDrawDebugTrace::None later
        HitResult,
        true,
        FLinearColor::Red,
        FLinearColor::Green,
        1.0f
    );

    // If we hit something, it means there is a wall in the way.
    // If we didn't hit anything, the line of sight is clear.
    return !bHit;
}
