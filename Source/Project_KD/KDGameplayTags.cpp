#include "KDGameplayTags.h"

namespace GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Light, "Ability.Player.Light")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Heavy, "Ability.Player.Heavy")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Dodge, "Ability.Player.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Parry, "Ability.Player.Parry")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_SprintAttack, "Ability.Player.SprintAttack")
    UE_DEFINE_GAMEPLAY_TAG(Ability_Player_CounterThrust, "Ability.Player.CounterThrust")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_AirCombo, "Ability.Player.AirCombo")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Execution, "Ability.Player.Execution")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Aim, "Ability.Player.Aim")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Shoot, "Ability.Player.Shoot")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_ShotBlast, "Ability.Player.ShotBlast")
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Turn, "Ability.Movement.Turn")
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Grunt_Attack_Basic, "Ability.Enemy.Grunt.Attack.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Grunt_Attack_Heavy, "Ability.Enemy.Grunt.Attack.Heavy")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Grunt_Attack_Combo, "Ability.Enemy.Grunt.Attack.Combo")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Grunt_Attack_Ranged, "Ability.Enemy.Grunt.Attack.Ranged")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Grunt_Attack_RangedCharged, "Ability.Enemy.Grunt.Attack.RangedCharged")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Grunt_Backstep, "Ability.Enemy.Grunt.Backstep")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Grunt_Parry, "Ability.Enemy.Grunt.Parry")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Elite_Attack_Basic, "Ability.Enemy.Elite.Attack.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Elite_Attack_Sweep, "Ability.Enemy.Elite.Attack.Sweep")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Elite_Attack_Smash, "Ability.Enemy.Elite.Attack.Smash")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Elite_Attack_Rush, "Ability.Enemy.Elite.Attack.Rush")
	UE_DEFINE_GAMEPLAY_TAG(Ability_Enemy_Boss_Attack_Basic, "Ability.Enemy.Boss.Attack.Basic")

	UE_DEFINE_GAMEPLAY_TAG(Ability_Combat_Unblockable, "Ability.Combat.Unblockable")

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Grunt_Heavy, "Cooldown.Enemy.Grunt.Heavy")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Grunt_Ranged, "Cooldown.Enemy.Grunt.Ranged")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Grunt_RangedCharged, "Cooldown.Enemy.Grunt.RangedCharged")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Grunt_Backstep, "Cooldown.Enemy.Grunt.Backstep")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Grunt_SwordAttack1, "Cooldown.Enemy.Grunt.SwordAttack1")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Grunt_SwordAttack2, "Cooldown.Enemy.Grunt.SwordAttack2")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Grunt_Parry, "Cooldown.Enemy.Grunt.Parry")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Elite_Smash, "Cooldown.Enemy.Elite.Smash")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Elite_Basic, "Cooldown.Enemy.Elite.Basic")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Elite_Sweep, "Cooldown.Enemy.Elite.Sweep")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Enemy_Elite_Rush, "Cooldown.Enemy.Elite.Rush")

	UE_DEFINE_GAMEPLAY_TAG(State_Combat_InCombat, "State.Combat.InCombat")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_InAction, "State.Combat.InAction")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Attacking, "State.Combat.Attacking")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Dodging, "State.Combat.Dodging")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Invulnerable, "State.Combat.Invulnerable")
	
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_CanCancel, "State.Combat.CanCancel")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_MovementCanCancel, "State.Combat.MovementCanCancel")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Parrying, "State.Combat.Parrying")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_PerfectParryReady, "State.Combat.PerfectParryReady")
	
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Staggered, "State.Combat.Staggered")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_SuperArmor, "State.Combat.SuperArmor")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_PoiseRegenPaused, "State.Combat.PoiseRegenPaused")
	UE_DEFINE_GAMEPLAY_TAG(State_Stamina_RegenBlocked, "State.Stamina.RegenBlocked")
	UE_DEFINE_GAMEPLAY_TAG(State_Gun_Reloading, "State.Gun.Reloading")
	UE_DEFINE_GAMEPLAY_TAG(State_Gun_RegenBlocked, "State.Gun.RegenBlocked")
	
	UE_DEFINE_GAMEPLAY_TAG(State_Character_LockOn, "State.Character.LockOn")

	UE_DEFINE_GAMEPLAY_TAG(State_Movement_Turning, "State.Movement.Turning")
	UE_DEFINE_GAMEPLAY_TAG(State_Movement_InAir, "State.Movement.InAir")
	
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead")

	UE_DEFINE_GAMEPLAY_TAG(State_Camera_Cinematic, "State.Camera.Cinematic")
	
	UE_DEFINE_GAMEPLAY_TAG(Team_Enemy, "Team.Enemy")

	UE_DEFINE_GAMEPLAY_TAG(State_Combat_EnemyAttackHitWindow, "State.Combat.EnemyAttackHitWindow")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_CounterReady, "State.Combat.CounterReady")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_AirComboLock, "State.Combat.AirComboLock")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Aiming, "State.Combat.Aiming")
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Shooting, "State.Combat.Shooting")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_TraceBegin, "Event.Montage.TraceBegin")
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Shoot, "Event.Montage.Shoot")
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ShotBlast, "Event.Montage.ShotBlast")
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_TraceEnd, "Event.Montage.TraceEnd")

	UE_DEFINE_GAMEPLAY_TAG(Event_Rush_Warp, "Event.Rush.Warp")
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_Hit, "Event.Combat.Hit")
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitReact, "Event.Combat.HitReact")
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_WeaponToggle, "Event.Combat.WeaponToggle")
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_ExecutionStarted, "Event.Combat.ExecutionStarted")
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_HitStop, "Event.Combat.HitStop")

	
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_PerfectParryTriggered, "Event.Combat.PerfectParryTriggered")
	UE_DEFINE_GAMEPLAY_TAG(Event_Combat_ParrySuccess, "Event.Combat.ParrySuccess")

	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_AttackPower, "SetByCaller.AttackPower")
	
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Stamina, "SetByCaller.Stamina")

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_HitImpact_Light, "GameplayCue.Combat.HitImpact.Light")

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Enemy_Telegraph_Parryable, "GameplayCue.Enemy.Telegraph.Parryable")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Enemy_Telegraph_Unblockable, "GameplayCue.Enemy.Telegraph.Unblockable")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Enemy_Telegraph_Lethal, "GameplayCue.Enemy.Telegraph.Lethal")

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Staggered, "GameplayCue.Combat.Staggered")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_ParryClash, "GameplayCue.Combat.ParryClash")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_Execution, "GameplayCue.Combat.Execution")

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_PlayerHitConfirm, "GameplayCue.Combat.PlayerHitConfirm")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_PerfectParry_SlowMo, "GameplayCue.Combat.PerfectParry.SlowMo")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_PerfectDodge, "GameplayCue.Combat.PerfectDodge")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_HitReact, "GameplayCue.Combat.HitReact")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Camera_Execution, "GameplayCue.Camera.Execution")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Camera_ShootRecoil, "GameplayCue.Camera.ShootRecoil")

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Combat_CounterThrust, "GameplayCue.Combat.CounterThrust")
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Camera_DashTrail, "GameplayCue.Camera.DashTrail")

	
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Light, "Input.Action.Light")
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Heavy, "Input.Action.Heavy")
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Dodge, "Input.Action.Dodge")
	UE_DEFINE_GAMEPLAY_TAG(Input_Action_Parry, "Input.Action.Parry")
	UE_DEFINE_GAMEPLAY_TAG(Input_Combo_Light, "Input.Combo.Light")
	UE_DEFINE_GAMEPLAY_TAG(Input_Combo_Heavy, "Input.Combo.Heavy")
}
 