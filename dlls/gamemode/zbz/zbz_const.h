#include <string>

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
	Skill_ZombieS, // become z5 zombie
	Skill_GhostHunter, // get dualkukri
	Skill_ZombiBombGiveaway, //Zombie Bombs given every cycle
	Skill_SwordMaster,	//enable accshoot to knife
	Skill_Knife2X, //Knife2X
	Skill_GoldClip, //GoldClip
	Skill_Penetration, //go through teamates
	Skill_StableGrip, //avoid deimos zombie
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
	ZBZ_MESSAGE_GHOSTHUNTER, // { byte msg_type, byte player }
};
const int MAX_SKILL = { 80 };
/*
std::string Skill_Info[MAX_SKILL][3] =

{

	{"mode/zbz/zombie3z_mutation_id_000","金融家","回合开始后，获得更多$。"},
	{"mode/zbz/zombie3z_mutation_id_001","格斗大师","使用近身武器时，攻击力提升。"},
	{"mode/zbz/zombie3z_mutation_id_002","弹夹扩充","填装时一定概率装填更多的子弹。"},
	{"mode/zbz/zombie3z_mutation_id_003","高速填装","加快武器的装填速度。"},
	{"mode/zbz/zombie3z_mutation_id_004","透明换弹","装弹时身体变透明。"},
	{"mode/zbz/zombie3z_mutation_id_005","猎豹","增加移动速度。追加的移动速度不受重量影响。"},
	{"mode/zbz/zombie3z_mutation_id_006","袋鼠","增加跳跃力。追加跳跃力不受重量影响。"},
	{"mode/zbz/zombie3z_mutation_id_007","恢复强化","提高恢复能力。"},
	{"mode/zbz/zombie3z_mutation_id_008","正面突击","装备突击步枪时，定期增加移动速度和攻击力。"},
	{"mode/zbz/zombie3z_mutation_id_009","冲锋推进","装备冲锋枪时增加移动速度，定期受到追加加速效果。"},
	{"mode/zbz/zombie3z_mutation_id_010","备弹补充","增加武器的备弹。"},
	{"mode/zbz/zombie3z_mutation_id_011","狩猎本能","定期将周围敌人指定为目标。"},
	{"mode/zbz/zombie3z_mutation_id_012","二段跳","在空中可以再跳一次。"},
	{"mode/zbz/zombie3z_mutation_id_013","爆炸弹头","攻击几率爆炸"},
	{"mode/zbz/zombie3z_mutation_id_014","强化基因","被感染时进化为母体僵尸。"},
	{"mode/zbz/zombie3z_mutation_id_015","坚持不懈","受到攻击时低概率忽略定身和击退。"},
	{"mode/zbz/zombie3z_mutation_id_016","点燃","狙击枪命中时，使敌人燃烧。"},
	{"mode/zbz/zombie3z_mutation_id_017","生命补液","增加生命值。"},
	{"mode/zbz/zombie3z_mutation_id_018","接触感染","身体触发时触发感染。"},
	{"mode/zbz/zombie3z_mutation_id_019","复活","取得复活能力。"},
	{"mode/zbz/zombie3z_mutation_id_020","钢铁铠甲","提升防御能力。"},
	{"mode/zbz/zombie3z_mutation_id_021","脚掌","蹲下移动时增加移速。"},
	{"mode/zbz/zombie3z_mutation_id_022","钢铁头盔","降低致命打击的伤害。"},
	{"mode/zbz/zombie3z_mutation_id_023","神枪手","大幅提升狙击步枪的命中率。"},
	{"mode/zbz/zombie3z_mutation_id_024","第六感","可以感知母体僵尸的出现，亦能感知到自己即将变为母体僵尸。"},
	{"mode/zbz/zombie3z_mutation_id_025","英雄出现","拥有被选为英雄的资格。"},
	{"mode/zbz/zombie3z_mutation_id_026","狂热","通过战斗与合作变得更强大。"},
	{"mode/zbz/zombie3z_mutation_id_027","持之以恒","激活人类特殊技能。"},
	{"mode/zbz/zombie3z_mutation_id_028","强制坠落","让梯子上的敌人摔落。"},
	{"mode/zbz/zombie3z_mutation_id_029","适应力","随时改变将是种类。"},
	{"mode/zbz/zombie3z_mutation_id_030","疫苗手雷","生成一个装有疫苗的烟雾弹，处于烟雾中的僵尸会受到持续伤害。"},
	{"mode/zbz/zombie3z_mutation_id_031","炸弹背包","一定周期自动生成手雷。"},
	{"mode/zbz/zombie3z_mutation_id_032","支援轰炸","手雷爆炸时低概率进行轰炸。"},
	{"mode/zbz/zombie3z_mutation_id_033","致命一击","低概率触发致命一击。"},
	{"mode/zbz/zombie3z_mutation_id_034","破甲弹头","增加武器的射击威力。"},
	{"mode/zbz/zombie3z_mutation_id_035","连发手雷","使用投掷装备时，自动追加使用次数。"},
	{"mode/zbz/zombie3z_mutation_id_036","爆弹兽颅","受到伤害后产生爆弹兽颅。"},
	{"mode/zbz/zombie3z_mutation_id_037","火焰球","使用狙击枪射击时发射火焰球。"},
	{"mode/zbz/zombie3z_mutation_id_038","猛犸","受到伤害后产生爆弹兽颅。"},
	{"mode/zbz/zombie3z_mutation_id_039","冷冻手雷","生成一个可击退僵尸的冷冻手雷。"},
	{"mode/zbz/zombie3z_mutation_id_040","伊卡洛斯","短时间内可以滑翔。"},
	{"mode/zbz/zombie3z_mutation_id_041","贫铀弹头","增加子弹的穿透效果。"},
	{"mode/zbz/zombie3z_mutation_id_042","少数精锐","人类数量减少时，增加攻击力和移动速度。"},
	{"mode/zbz/zombie3z_mutation_id_043","金钱之力","拥有的$越多，武器的攻击力越高。"},
	{"mode/zbz/zombie3z_mutation_id_044","武器熟练","连续购买同一武器，增加攻击力。"},
	{"mode/zbz/zombie3z_mutation_id_045","感染恢复","感染时恢复周围僵尸的生命值。"},
	{"mode/zbz/zombie3z_mutation_id_046","兽颅强化","强化爆弹兽颅。"},
	{"mode/zbz/zombie3z_mutation_id_047","合金利爪","增加僵尸的攻击距离。"},
	{"mode/zbz/zombie3z_mutation_id_048","技能冷却","减少僵尸技能的冷却时间。"},
	{"mode/zbz/zombie3z_mutation_id_049","倾斜装甲","受到攻击时，一定概率不受伤害。"},
	{"mode/zbz/zombie3z_mutation_id_050","进化学者","通过击败僵尸及感染人类也可以提升进化等级。"},
	{"mode/zbz/zombie3z_mutation_id_051","兽颅增殖","降低兽颅造成的伤害及击退效果。"},
	{"mode/zbz/zombie3z_mutation_id_052","复仇投掷","作为僵尸死亡时，向前方投掷爆弹兽颅。"},
	{"mode/zbz/zombie3z_mutation_id_053","兽颅护盾","受到攻击时，一定概率不受伤害。"},
	{"mode/zbz/zombie3z_mutation_id_054","痛苦记忆","降低击败过我的敌人再次对我造成的伤害。"},
	{"mode/zbz/zombie3z_mutation_id_055","兽颅寄生","在生化补给箱中放置1个爆弹兽颅。"},
	{"mode/zbz/zombie3z_mutation_id_056","融合","获得额外的僵尸技能。"},
	{"mode/zbz/zombie3z_mutation_id_057","愤怒狂奔","生命值较低时提升移动速度。"},
	{"mode/zbz/zombie3z_mutation_id_058","优惠券","使用资金$时享受折扣。"},
	{"mode/zbz/zombie3z_mutation_id_059","快速切换","提升武器切换速度。"},
	{"mode/zbz/zombie3z_mutation_id_060","狂战士","成功击败人类目标后强化自身。"},
	{"mode/zbz/zombie3z_mutation_id_061","先行者","所有解锁等级要求降低1级。"},
	{"mode/zbz/zombie3z_mutation_id_062","伤害复制","攻击命中时较低概率重复造成1次相同的伤害。"},
	{"mode/zbz/zombie3z_mutation_id_063","能量网","攻击时较低概率自动发射能量网。"},
	{"mode/zbz/zombie3z_mutation_id_064","风险投资","击败人类目标时获得奖励资金$。"},
	{"mode/zbz/zombie3z_mutation_id_065","腐化之地","僵尸被击败时生成腐化之地。"},
	{"mode/zbz/zombie3z_mutation_id_066","僵尸猎犬","使用爆弹兽颅造成伤害后召唤1只僵尸猎犬。"},
	{"mode/zbz/zombie3z_mutation_id_067","弹射座椅","即将被僵尸击败时，逃离死亡危机。"},
	{"mode/zbz/zombie3z_mutation_id_068","钩子爆弹","在提升攻击力的同时，提升受到的伤害。"},
	{"mode/zbz/zombie3z_mutation_id_069","玻璃大炮","使用爆弹兽颅造成伤害后，拉扯命中的目标。"},
	{"mode/zbz/zombie3z_mutation_id_070","忙碌","无法使用技能时提升攻击力。"},
	{"mode/zbz/zombie3z_mutation_id_071","变异","有一定概率成为变异僵尸。"}


};
*/


#endif