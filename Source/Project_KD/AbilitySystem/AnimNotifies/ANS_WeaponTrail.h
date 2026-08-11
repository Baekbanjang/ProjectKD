#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WeaponTrail.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// 몽타주 타임라인에 배치 — 무기 나이아가라 트레일 제어
// Begin = SocketName에 시스템 부착 + 등록된 유저 파라미터 전달
// End = Deactivate로 리본 자연 소멸
UCLASS(meta = (DisplayName = "Weapon Trail"))
class PROJECT_KD_API UANS_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_WeaponTrail();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Weapon Trail", meta = (DisplayName = "나이아가라 이펙트"))
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	// 트레일을 붙일 무기 메시의 ComponentTag. 비우면 캐릭터 본체 메시(맨손/발차기용)
	UPROPERTY(EditAnywhere, Category = "Weapon Trail",
		meta = (DisplayName = "붙일 무기 태그 (비우면 캐릭터 몸)",
		ToolTip = "무기 컴포넌트의 ComponentTag. Sword / Gun. 비우면 본체 메시(맨손·발차기)"))
	FName WeaponMeshComponentTag = TEXT("Sword");
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail",
		meta = (DisplayName = "붙일 소켓", ToolTip = "무기 메시의 소켓명. 검은 Sword_Bottom(손잡이)"))
	FName SocketName = TEXT("Sword_Bottom");

	// 나이아가라 유저 파라미터 - float / Color / Vector
	UPROPERTY(EditAnywhere, Category = "Weapon Trail",
		meta = (DisplayName = "float 파라미터", ToolTip = "키 = NS 유저 파라미터 이름. NS마다 다름"))
	TMap<FName, float> FloatParams;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail",
		meta = (DisplayName = "Color 파라미터", ToolTip = "키 = NS 유저 파라미터 이름"))
	TMap<FName, FLinearColor> ColorParams;

	UPROPERTY(EditAnywhere, Category = "Weapon Trail",
		meta = (DisplayName = "Vector 파라미터", ToolTip = "키 = NS 유저 파라미터 이름"))
	TMap<FName, FVector> VectorParams;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail", meta = (DisplayName = "위치 보정"))
	FVector LocationOffset = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail", meta = (DisplayName = "회전 보정"))
	FRotator RotationOffset = FRotator::ZeroRotator;

private:
	// 노티 객체 = 몽타주 에셋 소속 1개, 모든 액터가 공유 (액터별 복사 없음 = 인스턴싱 X)
	//  맵 | 키 = MeshComp (= 액터 식별) / 값 = 그 액터의 트레일
	//   TWeakObjectPtr 사용: 액터 파괴 시 GC 방해 x + 죽은 키 감지 가능
	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> SpawnedTrails;
};
