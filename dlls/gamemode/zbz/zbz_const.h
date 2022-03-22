
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
	Skill_HuntierInstinct, // Target targets within range periodically. Your combat ability against your target opponent is greatly increased.
	Skill_MoneyGrubber, // Earn extra dollars at the start of the round. It is provided separately from the dollar rewards resulting from the combat.
	Skill_SkillEvolution, // Zombie skill reuse time is reduced.
	Skill_LifePlunder, // When you infected a human, you and your allies will recover HP.
	Skill_Craftsmanship, // When you use the same firearm continuously, the damage of the firearm increases. (Based on weapons when zombies appeared)	
	Skill_PurchasingPower, // The more dollars you have, the more damage you do.
	Skill_Elite, // As the number of humans decreases, the damageand movement rate increases.
	Skill_PenetratingRounds, // The penetration rate of the shot is increased. The effect can not be applied to firearms that can not penetrate.
	Skill_Revenge, // Automatically throw a zombie bomb in the direction you look before dying as a zombie.
	Skill_ZombieBombGiveaway, // Receive a zombie bomb in every round.
	Skill_Intellectual, // Gain experience when killing zombies or infecting humans.
	Skill_BombDefense, // Damage and knockback received from zombie bombs is reduced. The reduced damage and knockback is applied to all zombie bombs.
	Skill_SupplyContamination, // Allows you to place Zombie Grenade on human Supply Boxes by pressing E. When a human approaches a bombed supply box, the Zombie Grenade will explode.
	Skill_PainfulMemories, // For one round, marks the last player who killed you. You'll take less damage from the marked player.
	Skill_Mutant,	// Zombie gains another common zombie skill. Skills have a random chance.	No
	Skill_FuriouslyFast,	// When a zombie's health is below a certain level, movement speed is increased, stiffness and knockback are reduced.	No
	Skill_Liberator,	// Creates a stack each time it infects a human. Each stack increases resistance to stiffness, knockback attacks, movement speed, and restores health.	No
	Skill_AggressiveInvestment,	// Gets a bonus dollar from the infected target. This will be paid in addition to the existing reward.	No
	Skill_PoisonousCloud,	// When a zombie dies, a deadly gas is created. Humans who come into contact with the gas take damage. Zombies can regenerate health when they come into contact with the gas.	No
	Skill_HookBomb,	// Pull targets damaged by hitting or detonating a zombie bomb a certain distance.	No
	Skill_Fetch,	// If you deal damage with a zombie bomb, 1 zombie dog is created.	Yes
	Skill_Darkness,	// You can be reborn as a Darkness Zombie. Darkness Zombies have limited performance.	No
	Skill_QuickChange,	// Increases weapon swap speed.	Yes
	Skill_NetRound,	// Your attack has a low chance to fire a net round. Enemies hit by the grenade are limited to movement for a short time.	Yes
	Skill_EmergencyEscape,	// Escape from the brink of deathand escape into the air.Upon escaping, you become temporarily invincible.No
	Skill_GlassCannon,	// Instead of just increasing your damage, you also take increased damage.	No
	Skill_DoubleImpact,	// Low chance to apply damage once more.	Yes
	Skill_BusyLife,	// Attack power is increased when skills cannot be used. Accelerated mutation and skills during cooldown do not apply.	No
	Skill_EarlyAdopter,	// All unlock levels are reduced by 1.	No
	Skill_DiscountCoupon,	// All purchases using dollars are discounted.
	Skill_RapidReloaderII, //the shot is not fired but fires instantly from the reserved ammo.
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