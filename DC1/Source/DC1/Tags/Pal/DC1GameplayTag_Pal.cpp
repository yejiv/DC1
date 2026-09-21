// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Pal/DC1GameplayTag_Pal.h"

namespace DC1PalTags
{
#pragma region Species
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Alpaca, "Pal.Species.Alpaca");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Anubis, "Pal.Species.Anubis");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Boar, "Pal.Species.Boar");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_CaptainPenguin, "Pal.Species.CaptainPenguin");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_ChickenPal, "Pal.Species.ChickenPal");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_CowPal, "Pal.Species.CowPal");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_CuteFox, "Pal.Species.CuteFox");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Deer, "Pal.Species.Deer");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_DreamDemon, "Pal.Species.DreamDemon");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_ElecPanda, "Pal.Species.ElecPanda");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_ElecPomeranian, "Pal.Species.ElecPomeranian");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Garm, "Pal.Species.Garm");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_GhostBlackCat, "Pal.Species.GhostBlackCat");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_GrassMammoth, "Pal.Species.GrassMammoth");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_HadesBird, "Pal.Species.HadesBird");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_HawkBird, "Pal.Species.HawkBird");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Kitsunebi, "Pal.Species.Kitsunebi");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_LeafMomonga, "Pal.Species.LeafMomonga");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Monkey, "Pal.Species.Monkey");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Penguin, "Pal.Species.Penguin");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_PinkCat, "Pal.Species.PinkCat");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_SheepBall, "Pal.Species.SheepBall");
#pragma endregion

#pragma region Passive
	// 공격력만
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Ferocity, "Pal.Passive.Ferocity");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Courageous, "Pal.Passive.Courageous");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Coward, "Pal.Passive.Coward");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Pacifist, "Pal.Passive.Pacifist");

	// 방어력만
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_ThickHide, "Pal.Passive.ThickHide");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Frail, "Pal.Passive.Frail");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_BrittleBones, "Pal.Passive.BrittleBones");

	// 작업 속도만
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Artisan, "Pal.Passive.Artisan");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Diligence, "Pal.Passive.Diligence");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Clumsiness, "Pal.Passive.Clumsiness");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_SlackerHabit, "Pal.Passive.SlackerHabit");

	// 공격력 <-> 방어력
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_DemonGod, "Pal.Passive.DemonGod");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Sadist, "Pal.Passive.Sadist");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Masochist, "Pal.Passive.Masochist");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Delinquent, "Pal.Passive.Delinquent");

	// 공격력 <-> 작업 속도
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Musclehead, "Pal.Passive.Musclehead");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Ferocious, "Pal.Passive.Ferocious");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_WorkSlave, "Pal.Passive.WorkSlave");

	// 방어력 <-> 작업 속도
	UE_DEFINE_GAMEPLAY_TAG(Pal_Passive_Conceited, "Pal.Passive.Conceited");
#pragma endregion

#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Idle, "Ability.Pal.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Hit, "Ability.Pal.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Tossin, "Ability.Pal.Tossin");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_FarSkill, "Ability.Pal.FarSkill");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Encount, "Ability.Pal.Encount");
#pragma endregion

#pragma region State
	UE_DEFINE_GAMEPLAY_TAG(State_Pal_Attacking, "State.Pal.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Pal_Idling, "State.Pal.Idling");
	UE_DEFINE_GAMEPLAY_TAG(State_Pal_HitReacting, "State.Pal.HitReacting");
	UE_DEFINE_GAMEPLAY_TAG(State_Pal_Encounting, "State.Pal.Encounting");
#pragma endregion

#pragma region Anim
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Hit, "Anim.Pal.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Tossin, "Anim.Pal.Tossin");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_FarSkill, "Anim.Pal.FarSkill");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Encount, "Anim.Pal.Encount");

	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_LookAround, "Anim.Pal.Idle.LookAround");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_Eat, "Anim.Pal.Idle.Eat");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_Rest, "Anim.Pal.Idle.Rest");
#pragma endregion

#pragma region Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Pal_Attack_Hit, "Event.Pal.Attack.Hit");
#pragma endregion
}
