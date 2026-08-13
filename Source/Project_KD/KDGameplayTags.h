#pragma once

#include "NativeGameplayTags.h"

// 네이티브 태그 중앙 선언 — BP 등록 금지(머지 충돌).
namespace GameplayTags
{
	// Abilities
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_Light)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_Heavy)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_Dodge)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_Parry)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_SprintAttack)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_CounterThrust)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_AirCombo)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_Execution)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_Aim)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_Shoot)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Mugong_ShotBlast)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Turn)

	// 적 공격 (BP GA child가 ability identity 태그로 설정)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Grunt_Attack_Basic)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Grunt_Attack_Heavy)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Grunt_Attack_Combo)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Grunt_Attack_Ranged)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Grunt_Attack_RangedCharged)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Grunt_Backstep)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Grunt_Parry)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Elite_Attack_Basic)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Elite_Attack_Sweep)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Elite_Attack_Smash)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Elite_Attack_Rush)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Enemy_Boss_Attack_Basic)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combat_Unblockable)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Grunt_Heavy)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Grunt_Ranged)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Grunt_RangedCharged)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Grunt_Backstep)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Grunt_SwordAttack1)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Grunt_SwordAttack2)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Grunt_Parry)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Elite_Smash)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Elite_Basic)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Elite_Sweep)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Enemy_Elite_Rush)

	// Character states
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_InCombat)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_InAction)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Attacking) 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Dodging)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Invulnerable)

	// ANS_CancelWindow이 몽타주 회복 구간에 부여하는 취소 윈도우.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_CanCancel)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_MovementCanCancel)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Parrying)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_PerfectParryReady)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_EnemyAttackHitWindow)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_CounterReady)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_AirComboLock)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Aiming)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Shooting)


	// Poise-break 경직 상태. 적 GA가 ActivationBlockedTags로 자가 차단(BP Class Defaults 설정). 적 전용(플레이어 비대칭).
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Staggered)

	// 슈퍼아머 — HitReact GA만 차단, 데미지/포이즈/넉백은 통과. 공격 GA Owned Tags로 부여.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_SuperArmor)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_PoiseRegenPaused)

	// 스태미나 소모 직후 회복 차단. StaminaComponent가 루즈 태그로 부여, GE_StaminaRegen이 OngoingTagRequirements로 inhibit.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stamina_RegenBlocked)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_LockOn)

	// Turn Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_Turning)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Movement_InAir)
	
	// HandleDeath가 루즈 태그로 설정. 이 태그가 있으면 Damage GE 거부 — 코퍼스에 히트/큐/본셰이크 방지.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead)

	// Camera Cinematic
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Camera_Cinematic)
	
	// 적 진영 태그. 적 ASC가 PossessedBy에서 자가 부여. WeaponTrace GA가 아군사격 방지에 사용.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Enemy)

	// Animation events sent from AnimNotify, consumed by GA via WaitGameplayEvent.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_TraceBegin)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Shoot)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ShotBlast)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_TraceEnd)

	// 돌진 재조준 — 런지 윈도우 시작 시 AN_SendGameplayEvent가 발화, GA_EnemyRushAttack이 현재 위치로 워프 타겟 재계산(stale-overshoot 방지).
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Rush_Warp)

	// OnWeaponHit이 피격자에게 전송하는 제네릭 히트 이벤트. InstigatorTags에 공격 태그 운반.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_Hit)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitReact)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_WeaponToggle)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_ExecutionStarted)

	// 명중 시 공격자가 자기 자신에게 발신 - EventMagnitude에 정지 시간 운반
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_HitStop)

	// Combat events
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_PerfectParryTriggered)

	// 패링 성공 시 AS_Combat이 패리어 ASC로 발화. ContextHandle에 HitResult 운반 → 클래시 큐 위치 결정.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combat_ParrySuccess)

	// SetByCaller magnitude tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_AttackPower)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Dosul)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Stamina)

	// GameplayCues
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_HitImpact_Light)

	// 적 공격 전조 큐. Parryable=퍼펙트패링 윈도우, Unblockable=회피 필수, Lethal=치명(도끼 엘리트 주).
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Enemy_Telegraph_Parryable)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Enemy_Telegraph_Unblockable)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Enemy_Telegraph_Lethal)

	// 경직 진입 큐 (BP GC가 스턴 VFX/SFX 재생).
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_Staggered)

	// 패링 성공 시 무기 충돌 큐. 버스트, trace ImpactPoint에 배치. 적 BP child가 ParryClashCueTag로 override 가능.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_ParryClash)

	// Player
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_PlayerHitConfirm)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_PerfectParry_SlowMo)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_PerfectDodge)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_HitReact)

	// 처형 큐 — BP GCN이 피니셔 VFX/SFX 재생.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_Execution)

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_CounterThrust)
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Camera_DashTrail)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Camera_Execution)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Camera_ShootRecoil);


	// Input 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Light)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Heavy)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Dodge)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Parry)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Combo_Light)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Combo_Heavy)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Action_Aim)
}
