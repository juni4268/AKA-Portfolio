// Fill out your copyright notice in the Description page of Project Settings.


#include "BossMonsterAnim.h"
#include "BossMonster.h"

// Phase 1
void UBossMonsterAnim::PlayAttackMontage()
{
	if (!Montage_IsPlaying(AttackMontage))
	{
		Montage_Play(AttackMontage, 1.0f);
	}
}

void UBossMonsterAnim::PlayDashAttackMontage()
{
	if (!Montage_IsPlaying(DashAttackMontage))
	{
		Montage_Play(DashAttackMontage, 1.0f);
	}
}

void UBossMonsterAnim::PlayJumpStartMontage()
{
	if (JumpStartMontage == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("JumpStartMontage is not set!"));
		return;
	}
	if (!Montage_IsPlaying(JumpStartMontage))
	{
		Montage_Play(JumpStartMontage, 1.0f);
	}
}

void UBossMonsterAnim::PlayStunMontage()
{
	if (StunMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("StunMontage is not set!"));
		return;
	}
	if (!Montage_IsPlaying(StunMontage))
	{
		Montage_Play(StunMontage, 1.0f);
	}
}

void UBossMonsterAnim::AnimNotify_Jump_Launch()
{
	ABossMonster* Boss = Cast<ABossMonster>(TryGetPawnOwner());
	if (Boss)
	{
		Boss->StartJumpingUp();
	}
}

void UBossMonsterAnim::HandleDashPrepareFinished()
{
	ABossMonster* Boss = Cast<ABossMonster>(TryGetPawnOwner());
	if (Boss)
	{
		Boss->OnDashPrepareFinished();
	}
}

void UBossMonsterAnim::AttackMontageEnded()
{
	aiState = MonsterState::IdleState;
	isAttack = false;
}

void UBossMonsterAnim::PlaySpawnMontage()
{
	if (SpawnMontage == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnMontage is not set on BossMonsterAnim!"));
		return;
	}
	if (!Montage_IsPlaying(SpawnMontage))
	{
		Montage_Play(SpawnMontage, 1.0f);
	}
}

void UBossMonsterAnim::HandleSpawnFinished()
{
	ABossMonster* Boss = Cast<ABossMonster>(TryGetPawnOwner());
	if (Boss)
	{
		Boss->OnSpawnFinished();
	}
}