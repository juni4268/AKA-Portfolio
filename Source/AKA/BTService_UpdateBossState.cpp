// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_UpdateBossState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BossMonster.h"
#include "Kismet/GameplayStatics.h"
#include "AkaPlayer.h"
#include "BossMonsterAnim.h"

UBTService_UpdateBossState::UBTService_UpdateBossState()
{
	NodeName = TEXT("Update Boss State");
	// Tick 간격 설정 (반응성을 위해 0.1초로 단축)
	Interval = 0.1f;
	RandomDeviation = 0.05f;
}
void UBTService_UpdateBossState::TickNode(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Retrieve fresh pointers every tick to avoid stale references after respawn
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController)) return;

	ABossMonster* Boss = Cast<ABossMonster>(AIController->GetPawn());
	if (!IsValid(Boss)) return;

	UBossMonsterAnim* BossAnim = Cast<UBossMonsterAnim>(Boss->GetMesh()->GetAnimInstance());
	if (!IsValid(BossAnim)) return;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp == nullptr) return;

    // --- Watchdog: Force Reset isAttack if no montage is playing ---
    // Skip this check if we are in a Jump Sequence (as there might be no montage playing mid-air)
    if (BossAnim->isAttack && !BossAnim->Montage_IsPlaying(nullptr) && !Boss->IsInJumpSequence())
    {
        UE_LOG(LogTemp, Warning, TEXT("Service Watchdog: isAttack is TRUE but no Montage playing. Resetting to FALSE."));
        BossAnim->isAttack = false;
    }
    // -------------------------------------------------------------

	BlackboardComp->SetValueAsBool(IsAttackKey.SelectedKeyName, BossAnim->isAttack);
	BlackboardComp->SetValueAsBool(IsStunnedKey.SelectedKeyName, Boss->bIsStunned);
	BlackboardComp->SetValueAsInt(BossPhaseKey.SelectedKeyName, Boss->currentPhase);
 
    AActor * TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));
    if (TargetActor == nullptr)
    {
        AAkaPlayer * Player = Cast<AAkaPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
        if (Player)
        {
            BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Player);
            TargetActor = Player; 
        }
    }

    if (IsValid(TargetActor))
    {
        float Distance = FVector::Dist(Boss->GetActorLocation(), TargetActor->GetActorLocation());
        BlackboardComp->SetValueAsFloat(PlayerDistanceKey.SelectedKeyName, Distance);
    }
    else
    {
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), nullptr);
        BlackboardComp->SetValueAsFloat(PlayerDistanceKey.SelectedKeyName, 99999.0f);
    }
    
    float DashCurrentCooldown = BlackboardComp->GetValueAsFloat(DashAttackRemainingCooldownKey.SelectedKeyName);
    float RangeCurrentCooldown = BlackboardComp->GetValueAsFloat(RangeAttackRemainingCooldownKey.SelectedKeyName);
    float JumpCurrentCooldown = BlackboardComp->GetValueAsFloat(JumpAttackRemainingCooldownKey.SelectedKeyName);
    float JumpP2CurrentCooldown = BlackboardComp->GetValueAsFloat(JumpAttack_P2RemainingCooldownKey.SelectedKeyName);

    if (DashCurrentCooldown > 0.0f)
    {
        DashCurrentCooldown -= DeltaSeconds;
        BlackboardComp->SetValueAsFloat(DashAttackRemainingCooldownKey.SelectedKeyName, FMath::Max(0.0f, DashCurrentCooldown));
    }
    if (RangeCurrentCooldown > 0.0f)
    {
        RangeCurrentCooldown -= DeltaSeconds;
        BlackboardComp->SetValueAsFloat(RangeAttackRemainingCooldownKey.SelectedKeyName, FMath::Max(0.0f, RangeCurrentCooldown));
	}
    if (JumpCurrentCooldown > 0.0f)
    {
        JumpCurrentCooldown -= DeltaSeconds;
        BlackboardComp->SetValueAsFloat(JumpAttackRemainingCooldownKey.SelectedKeyName, FMath::Max(0.0f, JumpCurrentCooldown));
    }
	if (JumpP2CurrentCooldown > 0.0f)
	{
		JumpP2CurrentCooldown -= DeltaSeconds;
		BlackboardComp->SetValueAsFloat(JumpAttack_P2RemainingCooldownKey.SelectedKeyName, FMath::Max(0.0f, JumpP2CurrentCooldown));
	}
}
