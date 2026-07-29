#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WeaponTrail.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// Drag onto Montage timeline to drive the weapon's Niagara trail VFX.
// Begin → spawns the system attached to SocketName and pushes blade dimensions.
// End → deactivates so the ribbon fades out naturally instead of cutting.
UCLASS(meta = (DisplayName = "Weapon Trail"))
class PROJECT_KD_API UANS_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
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

	UPROPERTY(EditAnywhere, Category = "Weapon Trail",
		meta = (DisplayName = "칼 길이 (NS 변수)", ToolTip = "나이아가라의 SwordLength 변수로 전달"))
	float SwordLength = 120.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail",
		meta = (DisplayName = "트레일 폭 (NS 변수)", ToolTip = "나이아가라의 TrailWidth 변수로 전달"))
	float TrailWidth = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail", meta = (DisplayName = "위치 보정"))
	FVector LocationOffset = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Trail", meta = (DisplayName = "회전 보정"))
	FRotator RotationOffset = FRotator::ZeroRotator;

private:
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> SpawnedComponent;
};
