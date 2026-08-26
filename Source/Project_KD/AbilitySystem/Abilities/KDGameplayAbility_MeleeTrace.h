#pragma once

#include "CoreMinimal.h"
#include "KDGameplayAbility.h"
#include "AbilitySystem/Tasks/AT_MeleeTrace.h"
#include "KDGameplayAbility_MeleeTrace.generated.h"

class UANS_MeleeTrace;
class UAnimMontage;
class UAT_MeleeTrace;
class UGameplayEffect;

// 근접 무기 판정 베이스 — 플레이어 공격 / 적 공격
// 몽타주 재생 -> TraceBegin·TraceEnd 노티 -> AT_MeleeTrace -> DamageEffectClass 적용
// 훅 = OnActivated / OnTargetHit / OnCleanup
UCLASS(Abstract)
class PROJECT_KD_API UKDGameplayAbility_MeleeTrace : public UKDGameplayAbility
{
	GENERATED_BODY()

public:
	UKDGameplayAbility_MeleeTrace();

protected:
	// ASC 가 베이스 포인터로 호출 — protected 유지
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Montage", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float MontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 콤보 노드 없을 시 사용할 해당 공격의 데미지, 넉백, Poise 배수
	float DamageMultiplier = 1.f;
	float KnockbackMultiplier = 1.f;
	float PoiseMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName WeaponMeshComponentTag = TEXT("Sword");
	
	// 판정 출처 기본값
	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	ETraceMeshSource MeshSource = ETraceMeshSource::Weapon;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName StartSocket = TEXT("Sword_Bottom");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	FName EndSocket = TEXT("Sword_Tip");

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	ETraceMode TraceMode = ETraceMode::TipLine;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon", meta = (ClampMin = "0.1"))
	float CapsuleRadius = 3.0f;

	// 판정창 1개당 액터 1히트 - 검이 닿아 있는 동안 매 프레임 반복 타격 차단
	UPROPERTY(EditDefaultsOnly, Category = "Action|Weapon")
	bool bOncePerActor = true;

	UPROPERTY(EditDefaultsOnly, Category = "Action|Debug")
	bool bDrawDebug = false;

	// 몽타주·판정 배선 후 훅
	virtual void OnActivated();
	virtual float GetEffectiveMontagePlayRate() const { return MontagePlayRate;}

	// 액터당 1회 명중 훅 — Hit = 접점 큐용 충돌 정보
	virtual void OnTargetHit(AActor* HitActor, UAbilitySystemComponent* TargetASC, const FHitResult& Hit) {}

	// 오버라이드 시 Super 호출 필수 — 트레이스 정리가 여기에 연결됨
	virtual void OnCleanup(bool bWasCancelled) override;

	const UANS_MeleeTrace* GetActiveWindow() const { return ActiveWindow.Get(); }

private:
	UFUNCTION() void OnTraceBeginEvent(FGameplayEventData Payload);
	UFUNCTION() void OnTraceEndEvent(FGameplayEventData Payload);
	UFUNCTION() void OnWeaponHit(const FHitResult& Hit);
	UFUNCTION() void OnMontageCompleted();
	UFUNCTION() void OnMontageInterrupted();

	UPROPERTY()
	TObjectPtr<UAT_MeleeTrace> ActiveTraceTask;

	UPROPERTY()
	TSet<TObjectPtr<AActor>> AlreadyHitActors;
	
	TWeakObjectPtr<const UANS_MeleeTrace> ActiveWindow;
};
