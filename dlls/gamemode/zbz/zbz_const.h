
#ifndef ZBZ_CONST_H
#define ZBZ_CONST_H
#ifdef _WIN32
#pragma once
#endif

enum ZombieZSkillType : byte
{
	TYPE_GENERAL,
	TYPE_HUMAN,
	TYPE_ZOMBIE
};

enum ZombieZSkillSkillId : byte
{
	Skill_BulletDamage = 1,
	Skill_Critical,
	Skill_KnifeMaster,
	Skill_DoubleJump,
	Skill_Health,
	Skill_BombBackpack,
	Skill_ExplodeBullet,
	Skill_Speed,
	Skill_Icarus,
	Skill_Fanatic,
	Skill_HeroicPresence,
	Skill_AmmoReserves, //	Increases the amount of ammo of the weapon. When reached a certain level, it will be able to shoot the weapon's endlessly.
	Skill_RapidReloader, //	The reloading speed of the weapon decreases. If the loading speed is faster than a certain value, automatic loading is activated, which means that the shot is not fired but fires instantly from the reserved ammo.
	Skill_StealthReloader,
	Skill_FocusedBreakthrough, //	Moving an assault rifle forward with your hands. It will improve your combat ability every second. Improvement width is as follows.
	Skill_Booster, //	Movement speed increases when equipped with submachine guns, and is subject to additional acceleration periodically. The additional acceleration effect is maintained only for a short time.
	Skill_AmmoCreation,
	Skill_IncendiaryAmmo,
	Skill_Marksman,
	Skill_SixthSense, //	You can detect the appearance of a zombie host. You can detect when zombie hosts appear within a certain range. You can also tell if you yourself are a zombie host.
	Skill_Specialist,
	Skill_DropZone,
	Skill_VaccinationBlaster, //	The smoke bomb will be changed to a zombie-compatible vaccine. Also, the round starts with the vaccine shot.
	Skill_SupportBombardment, //	When a grenade explodes, there is a low probability of bombing at that location.
	Skill_RapidFireAmmo, //	When throwing a grenade, the same kind of grenade is automatically thrown.
	Skill_Fireball, //	The use of sniper rifles causes a strong explosion in close range. This explosion causes great damage to the enemy.
	Skill_FrostBlaster, //	Replace flashbang with Nitrogen Grenade that can freeze and knock zombies away in a short period of time when exploded. A new grenade will be issued at the beginning of each round.
	Skill_GeneBooster,
	Skill_EnduranceHave,
	Skill_ContactInfection,
	Skill_Resucitation, // 复活 获得复活能力。病毒被驱散后，致命击杀死亡时依旧不能复活。
	Skill_SteelBody,
	Skill_SpeedyCrouch,
	Skill_SteelHead,
	Skill_Adaptability, // 适应力 You can change the zombie class at any time by using the class change key (default [M] key). However, when you change a zombie class, your health decreases.
	Skill_ExplosiveReaction,
	Skill_ArmonedAdvance,
	Skill_BombEnhacement, // 兽颅强化 增加爆弹兽颅的攻击力和击退效果。
	Skill_ClawEnhacement,
	Skill_SteelSkin,
	Skill_Kangaroo,
};

enum ZBZMessageType : byte
{
	ZBZ_MESSAGE_LEVEL_INFO, // { byte msg_type, byte lv, byte exp, byte point }
	ZBZ_MESSAGE_SKILL_ADD, // { byte msg_type, byte id }
	ZBZ_MESSAGE_SKILL_REMOVE, // { byte msg_type, byte id }
	ZBZ_MESSAGE_BURN, // { byte msg_type, byte player, byte time(in 0.1s) }
	ZBZ_MESSAGE_CONTACT_INFECTION_RING, // { byte msg_type, byte player }
	ZBZ_MESSAGE_CLAW, // { byte msg_type, byte player }
	ZBZ_MESSAGE_WING, // { byte msg_type, byte player }
};

#endif