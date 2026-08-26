#include "Enemy/KDEnemyAnimInstance.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Combat/KDHitFeedbackComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

void UKDEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}

	// Locomotion — BlendSpace(Speed×Direction) 구동. DetourCrowd 회피가 velocity 노이즈를 일으켜
	// FInterpTo로 흡수(snap 금지 — 순간 0 떨굼을 오히려 악화시킴).
	const FVector Velocity = Pawn->GetVelocity();
	const float RawSpeed = Velocity.Size2D();
	Speed = FMath::FInterpTo(Speed, RawSpeed, DeltaSeconds, 8.f);

	// Direction — velocity vs facing의 부호 있는 각도(strafe BlendSpace X축).
	// 저속이면 노이즈라 마지막 값 유지. ±180 wrap은 delta-angle 보간으로 처리.
	float TargetDirection = Direction;
	if (RawSpeed > 20.f)
	{
		const FVector VelDir = Velocity.GetSafeNormal2D();
		const float Fwd = FVector::DotProduct(Pawn->GetActorForwardVector(), VelDir);
		const float Rgt = FVector::DotProduct(Pawn->GetActorRightVector(), VelDir);
		TargetDirection = FMath::RadiansToDegrees(FMath::Atan2(Rgt, Fwd));
	}
	const float DeltaAngle = FMath::FindDeltaAngleDegrees(Direction, TargetDirection);
	Direction = FMath::UnwindDegrees(Direction + DeltaAngle * FMath::Clamp(DeltaSeconds * 10.f, 0.f, 1.f));

	// Combat state — ABP State Machine Travel↔Combat 전환용.
	// 진입/이탈 임계값 분리(히스테리시스) — 경계 1개면 경계선에서 매 틱 깜빡(검 뽑기/넣기 무한 발동).
	bHasTarget = false;
	if (const AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
	{
		if (const UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			if (const AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor"))))
			{
				bHasTarget = true;
				const float DistToTarget = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
				if (!bInCombat && DistToTarget <= CombatEnterRange)
				{
					bInCombat = true;
				}
				else if (bInCombat && DistToTarget > CombatExitRange)
				{
					bInCombat = false;
				}
			}
			else
			{
				bInCombat = false; // 타겟 소실 → 검 내림
			}
		}
	}

	// 자세에 이동 속도를 맞춰 foot sliding 방지 — Combat(검 듦 strafe)=느림, Travel(추격)=빠름, 타겟 없음(패트롤)=걷기.
	// 매 틱 set(토글 감지 불필요, 초기값도 일관). BS 최대속도와 일치시켜야 발이 안 미끄러짐.
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			Move->MaxWalkSpeed = !bHasTarget ? PatrolWalkSpeed : (bInCombat ? CombatWalkSpeed : TravelWalkSpeed);
		}
	}

	// Hit feedback bone shake.
	if (!CachedHitFeedback)
	{
		CachedHitFeedback = Pawn->FindComponentByClass<UKDHitFeedbackComponent>();
		if (!CachedHitFeedback)
		{
			return;
		}
	}

	const float Alpha = CachedHitFeedback->CurrentShakeAlpha;
	const float Magnitude = Alpha * CachedHitFeedback->ShakeIntensity;

	if (Magnitude <= KINDA_SMALL_NUMBER)
	{
		PelvisOffset = FVector::ZeroVector;
		Spine01Offset = FVector::ZeroVector;
		Spine02Offset = FVector::ZeroVector;
		return;
	}

	auto MakeRandomOffset = [Magnitude](float Weight) -> FVector
	{
		const FVector Raw(
			FMath::FRandRange(-1.0f, 1.0f),
			FMath::FRandRange(-1.0f, 1.0f),
			FMath::FRandRange(-1.0f, 1.0f));
		return Raw.GetSafeNormal() * (Magnitude * Weight);
	};

	PelvisOffset  = MakeRandomOffset(PelvisWeight);
	Spine01Offset = MakeRandomOffset(Spine01Weight);
	Spine02Offset = MakeRandomOffset(Spine02Weight);
}
