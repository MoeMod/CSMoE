
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
};

#endif