// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_Dodge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Combat/KDProjectile.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
// 개발용 회피 판정 표시 스위치 — 콘솔 KD.ShowDodge 1
static TAutoConsoleVariable<int32> CVarShowDodge(
	TEXT("KD.ShowDodge"), 0,
	TEXT("퍼펙트 회피 판정 온스크린 표시 유무"), ECVF_Cheat);
#endif

UGA_Dodge::UGA_Dodge()
{
	bRetriggerInstancedAbility = true;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // InstancedPerActor 잔류 방지 — 매 활성화 시 명시 리셋.
    ActiveInvincibleHandle = FActiveGameplayEffectHandle();

    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    if (!IsValid(ASC))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Perfect 윈도우 판정 — 근접 적 ASC에 EnemyAttackHitWindow 태그 있나.
    const bool bPerfect = IsInPerfectDodgeWindow(ActorInfo);

    
#if !UE_BUILD_SHIPPING
    if (GEngine && CVarShowDodge.GetValueOnGameThread() > 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f,
            bPerfect ? FColor::Green : FColor::Silver,
            bPerfect ? TEXT("PERFECT DODGE") : TEXT("Normal Dodge"));
    }
#endif


    // GE 적용: Perfect는 강한 i-frame + CounterReady, Normal은 짧은 i-frame
    if (bPerfect)
    {
        ActiveInvincibleHandle = ApplySelfEffect(PerfectInvincibleGE);
        ApplySelfEffect(CounterWindowGE);

        // 퍼펙트 성공 보상 연출(슬로우+FX)
        if (UAbilitySystemComponent* RewardASC = GetAbilitySystemComponentFromActorInfo())
        {
            RewardASC->ExecuteGameplayCue(GameplayTags::GameplayCue_Combat_PerfectDodge);
        }
    }
    else
    {
        ActiveInvincibleHandle = ApplySelfEffect(NormalInvincibleGE);
    }

    // 방향 -> Montage 인덱스.
    EDodgeDirection Direction = ResolveDodgeDirection();
    
    ACharacter* DodgeChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn))
    {
        Direction = ResolveDodgeDirection(); // 락온: 현행 유지(타겟 기준 strafe)
    }
    else if (DodgeChar && DodgeChar->GetCharacterMovement())
    {
        // 프리: 카메라 기준 입력 방향으로 캐릭터를 돌린 뒤 전진 회피 
        const FVector InputVec = DodgeChar->GetCharacterMovement()->GetLastInputVector();
        if (!InputVec.IsNearlyZero())
        {
            DodgeChar->SetActorRotation(FRotator(0.f, InputVec.Rotation().Yaw, 0.f));
            Direction = EDodgeDirection::Forward;
        }
        // 입력 없으면 Backward(백스텝) 유지 — 회전 없음
    }
    
    const int32 Idx = static_cast<int32>(Direction);
    UAnimMontage* SelectedMontage = DodgeMontages.IsValidIndex(Idx) ? DodgeMontages[Idx].Get() : nullptr;

    if (!IsValid(SelectedMontage))
    {
        // Montage 없으면 안전하게 종료. GE는 OnCleanup에서 자동 정리.
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // PlayMontageAndWait — 종료 콜백 4종 묶기.
    UAbilityTask_PlayMontageAndWait* PlayTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, SelectedMontage, MontagePlayRate, NAME_None, true, 1.0f);

    PlayTask->OnCompleted.AddDynamic(this, &UGA_Dodge::OnMontageCompleted);
    PlayTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::OnMontageInterrupted);
    PlayTask->OnCancelled.AddDynamic(this, &UGA_Dodge::OnMontageCancelled);
    PlayTask->OnBlendOut.AddDynamic(this, &UGA_Dodge::OnMontageBlendOut);
    PlayTask->ReadyForActivation();

    // 베이스의 무한 GA 방지 타이머.
    StartSafetyTimer(SelectedMontage->GetPlayLength(), MontagePlayRate);

    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Dodge::OnCleanup(bool bWasCancelled)
{
    // 무적 GE 명시 제거 — Duration 끝나기 전에 GA 종료되면 leak 방지.
    if (ActiveInvincibleHandle.IsValid())
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        {
            ASC->RemoveActiveGameplayEffect(ActiveInvincibleHandle);
        }
        ActiveInvincibleHandle = FActiveGameplayEffectHandle();
    }
}

EDodgeDirection UGA_Dodge::ResolveDodgeDirection() const
{
    const AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!IsValid(Avatar)) return EDodgeDirection::Backward;

    const ACharacter* Character = Cast<ACharacter>(Avatar);
    if (!IsValid(Character) || !Character->GetCharacterMovement()) return EDodgeDirection::Backward;

    // 캐릭터 이동 입력 벡터 (월드 좌표).
    const FVector InputVec = Character->GetCharacterMovement()->GetLastInputVector();
    if (InputVec.IsNearlyZero()) return EDodgeDirection::Backward; // 입력 없으면 뒤로

    // 캐릭터 정면/우측 기준 dot 계산.
    const FVector Forward = Character->GetActorForwardVector();
    const FVector Right = Character->GetActorRightVector();
    const float DotForward = FVector::DotProduct(InputVec, Forward);
    const float DotRight = FVector::DotProduct(InputVec, Right);

    // 절댓값 큰 축이 주방향.
    if (FMath::Abs(DotForward) >= FMath::Abs(DotRight))
    {
        return DotForward > 0 ? EDodgeDirection::Forward : EDodgeDirection::Backward;
    }
    return DotRight > 0 ? EDodgeDirection::Right : EDodgeDirection::Left;
}


bool UGA_Dodge::IsInPerfectDodgeWindow(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if (!ActorInfo) return false;
    const AActor* Avatar = ActorInfo->AvatarActor.Get();
    if (!IsValid(Avatar)) return false;

    UWorld* World = Avatar->GetWorld();
    if (!World) return false;
    
    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);               // 적
    ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);  // 발사체 = Projectile 

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Avatar);                        // 본인 제외

    TArray<FOverlapResult> Overlaps;
    World->OverlapMultiByObjectType(Overlaps, Avatar->GetActorLocation(), FQuat::Identity,
        ObjectParams, FCollisionShape::MakeSphere(PerfectDodgeCheckRadius), QueryParams);

    // 컴포넌트 단위 결과라 같은 액터가 중복 가능
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Other = Overlap.GetActor();
        if (!Other) continue;

        // 발사체 경로 — 플래그 검사
        if (const AKDProjectile* Proj = Cast<AKDProjectile>(Other))
        {
            if (Proj->IsPerfectDodgeable() && Proj->GetInstigator() != Avatar)
            {
                return true;
            }
            continue; // 발사체 = ASC 없음 -> 아래 태그 경로 생략
        }

        // 적 경로 — 태그 검사
        UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other);
        if (!OtherASC) continue;

        if (OtherASC->HasMatchingGameplayTag(GameplayTags::State_Combat_EnemyAttackHitWindow))
        {
            return true;
        }
    }
    return false;
}

void UGA_Dodge::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Dodge::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Dodge::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Dodge::OnMontageBlendOut()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

