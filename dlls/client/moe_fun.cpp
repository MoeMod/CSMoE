
#include "extdll.h"
#include "cbase.h"
#include "util.h"
#include "gamerules.h"
#include "player/player_mod_strategy.h"
#include "util/u_range.hpp"
#include "player/player_const.h"
#include "gamemode/zb2//zb2_const.h"
#include "moe_fun.h"
#include "player/player_emotion.h"
#include "wdnmd.h"
#include "immenu.h"

namespace sv {
    namespace moefunmenu {
        
    std::map<HandTattooName, const char*> g_TattoName =
    {
        { TATTOO_NONE, "清除纹身" },
        { TATTOO_CHAOS, "火花纹身" },
        { TATTOO_DRAGON, "青龙纹身" },
        { TATTOO_TIGER, "猛虎纹身" },
        { TATTOO_BAMBOU, "翠竹纹身" },
        { TATTOO_CSOWC, "CSOWC 纪念纹身" },
        { TATTOO_TTCUSTOM, "蚩尤纹身" },
        { TATTOO_DIONE, "狄欧斯纹身" },
        { TATTOO_GLUTTONY, "狂暴饕餮纹身" },
        { TATTOO_MADMECHANIC, "破面重甲纹身" },
        { TATTOO_PHOBOS, "异形斗兽纹身" },
        { TATTOO_POISONFIRE, "鬼火纹身" },
        { TATTOO_RUSTYSCORPION, "铁甲巨蝎纹身" },
        { TATTOO_SNAKE, "巨蛇纹身" },
        { TATTOO_SNOW, "冰晶纹身" },
        { TATTOO_ZAVISTNEID, "恸哭双子纹身" },
        { TATTOO_LETTERCHN1, "纹身设计师活动获奖作品1(文字纹身)" },
        { TATTOO_LETTERCHN2, "纹身设计师活动获奖作品2(文字纹身)" },
        { TATTOO_LETTERCHN3, "纹身设计师活动获奖作品3(文字纹身)" },
        { TATTOO_TIGER22, "黑虎纹身" },
    };

    struct ZombieSkinMenu
    {
        ZombieClassType ClassType;
        const char* SkinName;
    };

    std::map<ZombieSkin, ZombieSkinMenu> g_ZombieSkinName =
    {
        { SKIN_CHNNORMAL, { ZOMBIE_CLASS_TANK ,"国服(普通僵尸装饰)" } },
        { SKIN_CHNSPEED, { ZOMBIE_CLASS_SPEED ,"国服(暗影芭比装饰)" } },
        { SKIN_CHNPC, { ZOMBIE_CLASS_PC ,"国服(迷雾鬼影装饰)" } },
        { SKIN_COSSPEED1, { ZOMBIE_CLASS_SPEED , "血腥玛丽(暗影芭比装饰)" } },
        { SKIN_SANTASKINLIGHT, { ZOMBIE_CLASS_SPEED ,"圣诞派对(暗影芭比装饰)" } },
        { SKIN_SANTASKINHEAVY, { ZOMBIE_CLASS_HEAVY ,"圣诞派对(憎恶屠夫装饰)"} },
        { SKIN_CHEFSKINHEAVY, { ZOMBIE_CLASS_HEAVY ,"特级厨师(憎恶屠夫装饰)"} },
        { SKIN_NURSESKINRESI, { ZOMBIE_CLASS_RESIDENT ,"死亡天使(痛苦女王装饰)"} },
        { SKIN_TWSKINHEAVY, { ZOMBIE_CLASS_HEAVY ,"电音三太子(憎恶屠夫装饰)"} },
        { SKIN_DEVILSKINLIGHT, { ZOMBIE_CLASS_SPEED ,"黑夜魔女(暗影芭比装饰)" } },
        { SKIN_STONEDEIMOS2, { ZOMBIE_CLASS_DEIMOS2 ,"晶石魔像(恶魔猎手装饰)" } },
        { SKIN_MUMMYSTAMPER, { ZOMBIE_CLASS_STAMPER ,"木乃伊(送葬者装饰)" } },
        { SKIN_STUDENTLIGHT, { ZOMBIE_CLASS_SPEED ,"黑暗萝莉(暗影芭比装饰)" } },
        { SKIN_STONEBOOMER, { ZOMBIE_CLASS_BOOMER ,"晶石巨像(腐化暴君装饰)" } },
        { SKIN_Y19S2BLOTERSKIN, { ZOMBIE_CLASS_PASS ,"稻草人(爆弹狂魔装饰)" } },
        { SKIN_Y19S2LIGHTSKIN, { ZOMBIE_CLASS_SPEED ,"西部牛仔(暗影芭比装饰)" } },
        { SKIN_Y19S4PSYCHO, { ZOMBIE_CLASS_PC ,"海盗船长(迷雾鬼影装饰)" } },
        { SKIN_Y19S4RUSTYWING, { ZOMBIE_CLASS_FLYING ,"堕落天使(断翼恶灵装饰)" } },
        { SKIN_Y20S1DOCTORSKIN, { ZOMBIE_CLASS_HEAL ,"祸乱朝堂(巫蛊术尸装饰)" } },
        { SKIN_Y20S1WITCHSKIN, { ZOMBIE_CLASS_WITCH ,"村野民妇(嗜血女妖装饰)" } },
        { SKIN_Y20S2RESISKIN, { ZOMBIE_CLASS_RESIDENT ,"南方女巫(痛苦女王装饰)" } },
        { SKIN_Y20S2BOOTSKIN, { ZOMBIE_CLASS_BOOSTER ,"西方女巫(追猎傀儡装饰)" } },
        { SKIN_Y20S3SPEEDSKIN, { ZOMBIE_CLASS_SPEED ,"人工智能(暗影芭比装饰)" } },
        { SKIN_Y20S3STAMPERSKIN, { ZOMBIE_CLASS_STAMPER ,"街头派对(送葬者装饰)" } },
        { SKIN_Y21S1AKSHA, { ZOMBIE_CLASS_AKSHA ,"冰霜巨魔(赤炎恶鬼装饰)" } },
        { SKIN_Y21S1FLYINGZOMBIE, { ZOMBIE_CLASS_FLYING ,"冰雪妖姬(断翼恶灵装饰)" } },
        { SKIN_Y21S2DEIMOSSKIN, { ZOMBIE_CLASS_DEIMOS ,"半兽人(恶魔之子装饰)" } },
        { SKIN_Y21S2WITCHSKIN, { ZOMBIE_CLASS_WITCH ,"黑暗精灵(嗜血女妖装饰)" } },
        { SKIN_Y21S3DEIMOS2SKIN, { ZOMBIE_CLASS_DEIMOS2 ,"拘束装甲(恶魔猎手装饰)" } },
        { SKIN_Y21S3BOOSTERZOMBISKIN, { ZOMBIE_CLASS_BOOSTER ,"自动人偶(追猎傀儡装饰)" } },
        { SKIN_Y21S4BLOTERSKIN, { ZOMBIE_CLASS_PASS ,"双脊龙(爆弹狂魔装饰)" } },
        { SKIN_Y21S4BOOMERSKIN, { ZOMBIE_CLASS_BOOMER ,"盾甲龙(腐化禁卫装饰)" } },
        { SKIN_Y23S1NORMAL, { ZOMBIE_CLASS_TANK ,"机械殖装(普通僵尸装饰)" } },
        { SKIN_Y23S2WITCHSKIN, { ZOMBIE_CLASS_WITCH ,"红心王后(嗜血女妖装饰)" } },
        { SKIN_Y23S3NORMAL, { ZOMBIE_CLASS_TANK ,"沙漠守卫(普通僵尸装饰)" } },
        
    };

    struct PaintMenu
    {
        WeaponPaintType paintType;
        const char* paintName;
    };

    std::map<WeaponPaintType, PaintMenu> g_PaintName =
    {
        { WEAPONPAINTCUSTOM, { WEAPONPAINTCUSTOM, "玩家自制彩漆" } },
        { WEAPONPAINT1, { WEAPONPAINT1, "红色闪电彩漆" } },
        { WEAPONPAINT2, { WEAPONPAINT2, "蓝色闪电彩漆" } },
        { WEAPONPAINT3, { WEAPONPAINT3, "熏的幻想彩漆" } },
        { WEAPONPAINTEAGLE, { WEAPONPAINTEAGLE, "呼啸狂鹰彩漆" } },
        { WEAPONPAINTGALAXY, { WEAPONPAINTGALAXY, "绚烂银河彩漆" } },
        { WEAPONPAINTCHOIJIYOON, { WEAPONPAINTCHOIJIYOON, "智云蝶舞彩漆" } },
        { WEAPONPAINTBLOOD, { WEAPONPAINTBLOOD, "腥红骷髅彩漆" } },
        { WEAPONPAINTPOISON, { WEAPONPAINTPOISON, "剧毒魔蛛彩漆" } },
        { WEAPONPAINTPLASMAGUNONLY, { WEAPONPAINTPLASMAGUNONLY, "水瓶座彩漆" } },
        { WEAPONPAINTSFSNIPERONLY, { WEAPONPAINTSFSNIPERONLY, "辐射彩漆" } },
        { WEAPONPAINTSPRIFLEONLY, { WEAPONPAINTSPRIFLEONLY, "狂想曲彩漆" } },
        { WEAPONPAINTBOWONLY, { WEAPONPAINTBOWONLY, "糖果幻境彩漆" } },
        { WEAPONPAINTINFINITYEX2ONLY, { WEAPONPAINTINFINITYEX2ONLY, "生死时速彩漆" } },
        { WEAPONPAINTM950SEONLY, { WEAPONPAINTM950SEONLY, "蜂语蜜意彩漆" } },
        { WEAPONPAINTLASERMINIGUNONLY, { WEAPONPAINTLASERMINIGUNONLY, "灵息魔蛇彩漆" } },
        { WEAPONPAINTDESTROYERONLY, { WEAPONPAINTDESTROYERONLY, "机动战士彩漆" } },
        { WEAPONPAINTNATAKNIFEKR, { WEAPONPAINTNATAKNIFEKR, "血焰彩漆" } },
        { WEAPONPAINTAIRBURSTERCN, { WEAPONPAINTAIRBURSTERCN, "东风彩漆" } },
        { WEAPONPAINTCHAINSAWMTW, { WEAPONPAINTCHAINSAWMTW, "光晕彩漆" } },
        { WEAPONPAINTPETROLBOOMERJP, { WEAPONPAINTPETROLBOOMERJP, "迷彩彩漆" } },
        { WEAPONPAINTSFPISTOLNA, { WEAPONPAINTSFPISTOLNA, "矩阵彩漆" } },
        { WEAPONPAINTDESERTEAGLEID, { WEAPONPAINTDESERTEAGLEID, "雷龙彩漆" } },
        { WEAPONPAINTGS18, { WEAPONPAINTGS18, "幻彩秘银彩漆" } },
        { WEAPONPAINT19S2CT, { WEAPONPAINT19S2CT, "天界守护彩漆" } },
        { WEAPONPAINTCHN11THBALROG11, { WEAPONPAINTCHN11THBALROG11, "Balrog-Ⅺ周年庆典彩漆" } },
        { WEAPONPAINTCHN11THCROW11, { WEAPONPAINTCHN11THCROW11, "CROW-11周年庆典彩漆" } },
        { WEAPONPAINTCHN11THJANUS11, { WEAPONPAINTCHN11THJANUS11, "Janus-11周年庆典彩漆" } },
        { WEAPONPAINTCHN11THTHANATOS11, { WEAPONPAINTCHN11THTHANATOS11, "Thanatos-11周年庆典彩漆" } },
        { WEAPONPAINTCHN11THTURBULENT11, { WEAPONPAINTCHN11THTURBULENT11, "Turbulent-11周年庆典彩漆" } },
        { WEAPONPAINTCHN11THSKULL11, { WEAPONPAINTCHN11THSKULL11, "SKULL-11周年庆典彩漆" } },
        { WEAPONPAINTIRON, { WEAPONPAINTIRON, "铁皮人彩漆" } },
        { WEAPONPAINTWOOD, { WEAPONPAINTWOOD, "稻草人彩漆" } },
        { WEAPONPAINTNEON, { WEAPONPAINTNEON, "霓虹彩漆" } },
        { WEAPONPAINTPSYCHEDELIC, { WEAPONPAINTPSYCHEDELIC, "涂鸦彩漆" } },
        { WEAPONPAINTREAPER, { WEAPONPAINTREAPER, "格式化彩漆" } },
        { WEAPONPAINTBNW, { WEAPONPAINTBNW, "昼夜彩漆" } },
        { WEAPONPAINTSIGMA, { WEAPONPAINTSIGMA, "核心转换彩漆" } },
        { WEAPONPAINTCHN12TH, { WEAPONPAINTCHN12TH, "12周年庆典彩漆" } },
        { WEAPONPAINT20CN, { WEAPONPAINT20CN, "寒冰凌眼彩漆" } },
        { WEAPONPAINT20KR, { WEAPONPAINT20KR, "粉色点点彩漆" } },
        { WEAPONPAINT20TW, { WEAPONPAINT20TW, "机械蜂巢彩漆" } },
        { WEAPONPAINT20NA, { WEAPONPAINT20NA, "黄金符文彩漆" } },
        { WEAPONPAINTFLAKE, { WEAPONPAINTFLAKE, "雪花彩漆" } },
        { WEAPONPAINTCRACKED, { WEAPONPAINTCRACKED, "深寒彩漆" } },
        { WEAPONPAINT21RB, { WEAPONPAINT21RB, "MK7彩漆" } },
        { WEAPONPAINTDKNIGHT, { WEAPONPAINTDKNIGHT, "龙骑士彩漆" } },
        { WEAPONPAINTHIGHELF, { WEAPONPAINTHIGHELF, "精灵彩漆" } },
        { WEAPONPAINTCLOCK, { WEAPONPAINTCLOCK, "齿轮彩漆" } },
        { WEAPONPAINTCOPPER, { WEAPONPAINTCOPPER, "黄铜彩漆" } },
        { WEAPONPAINTTYRANNO, { WEAPONPAINTTYRANNO, "暴龙彩漆" } },
        { WEAPONPAINTTRICERA, { WEAPONPAINTTRICERA, "角龙彩漆" } },
        { WEAPONPAINTCHN13TH, { WEAPONPAINTCHN13TH, "2021周年庆典彩漆" } },
        { WEAPONPAINTPARK, { WEAPONPAINTPARK, "游乐园彩漆" } },
        { WEAPONPAINTLUNAR, { WEAPONPAINTLUNAR, "星球彩漆" } },
        { WEAPONPAINTLIZARD, { WEAPONPAINTLIZARD, "蜥蜴彩漆" } },
        { WEAPONPAINTCHN14TH, { WEAPONPAINTCHN14TH, "2022周年彩漆" } },
        { WEAPONPAINTCYBORG, { WEAPONPAINTCYBORG, "赛博格彩漆" } },
        { WEAPONPAINTTRUMP, { WEAPONPAINTTRUMP, "扑克兵彩漆" } },
        { WEAPONPAINTY23CHN77, { WEAPONPAINTY23CHN77, "浪漫七夕彩漆" } },
    };

    struct WeaponPaintMenu
    {
        WeaponIdType WeaponID;
        KnifeIdType KnifeID;
        WeaponPaintType itemID;
        const char* weaponpaintName;
    };

    std::vector<WeaponPaintMenu> g_WeaponPaintName =
    {
        { WEAPON_AK47, KNIFE_NORMAL, WEAPONPAINT1, "AK47(红色闪电)" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINT1, "DesertEagle(红色闪电)" },
        { WEAPON_M4A1, KNIFE_NORMAL, WEAPONPAINT1, "M4A1(红色闪电)" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINT1, "AWP(红色闪电)"},
        { WEAPON_AK47, KNIFE_NORMAL, WEAPONPAINT2, "AK47(蓝色闪电)" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINT2, "DesertEagle(蓝色闪电)" },
        { WEAPON_M4A1, KNIFE_NORMAL, WEAPONPAINT2, "M4A1(蓝色闪电)" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINT2, "AWP(蓝色闪电)"},
        { WEAPON_AK47, KNIFE_NORMAL, WEAPONPAINT3, "AK47(熏的幻想)" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINT3, "DesertEagle(熏的幻想)" },
        { WEAPON_M4A1, KNIFE_NORMAL, WEAPONPAINT3, "M4A1(熏的幻想)" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINT3, "AWP(熏的幻想)"},
        { WEAPON_AK47, KNIFE_NORMAL, WEAPONPAINTEAGLE, "AK47(呼啸狂鹰)" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINTEAGLE, "DesertEagle(呼啸狂鹰)" },
        { WEAPON_M4A1, KNIFE_NORMAL, WEAPONPAINTEAGLE, "M4A1(呼啸狂鹰)" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINTEAGLE, "AWP(呼啸狂鹰)"},
        { WEAPON_AK47, KNIFE_NORMAL, WEAPONPAINTGALAXY, "AK47(绚烂银河)" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINTGALAXY, "DesertEagle(绚烂银河)" },
        { WEAPON_M4A1, KNIFE_NORMAL, WEAPONPAINTGALAXY, "M4A1(绚烂银河)" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINTGALAXY, "AWP(绚烂银河)"},
        { WEAPON_AK47, KNIFE_NORMAL, WEAPONPAINTCHOIJIYOON, "AK47(智云蝶舞)" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINTCHOIJIYOON, "DesertEagle(智云蝶舞)" },
        { WEAPON_M4A1, KNIFE_NORMAL, WEAPONPAINTCHOIJIYOON, "M4A1(智云蝶舞)" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINTCHOIJIYOON, "AWP(智云蝶舞)"},
        { WEAPON_SKULL1, KNIFE_NORMAL, WEAPONPAINTBLOOD, "腥红噬魂SKULL-1" },
        { WEAPON_SKULL3, KNIFE_NORMAL, WEAPONPAINTBLOOD, "腥红喋血SKULL-3" },
        { WEAPON_SKULL5, KNIFE_NORMAL, WEAPONPAINTBLOOD, "腥红死骑SKULL-5" },
        { WEAPON_M249EX, KNIFE_NORMAL, WEAPONPAINTBLOOD, "腥红碎魂者SKULL-7"},
        { WEAPON_KNIFE, KNIFE_SKULLAXE, WEAPONPAINTBLOOD, "腥红旋风SKULL-9"},
        { WEAPON_SKULL11, KNIFE_NORMAL, WEAPONPAINTBLOOD, "腥红雷霆SKULL-11"},
        { WEAPON_BALROG1, KNIFE_NORMAL, WEAPONPAINTPOISON, "剧毒红莲Balrog-Ⅰ"},
        { WEAPON_BALROG3, KNIFE_NORMAL, WEAPONPAINTPOISON, "剧毒灼魂Balrog-Ⅲ"},
        { WEAPON_BALROG5, KNIFE_NORMAL, WEAPONPAINTPOISON, "剧毒地狱犬Balrog-Ⅴ"},
        { WEAPON_BALROG7, KNIFE_NORMAL, WEAPONPAINTPOISON, "剧毒炎魔Balrog-Ⅶ"},
        { WEAPON_KNIFE, KNIFE_BALROG9, WEAPONPAINTPOISON, "剧毒爆裂Balrog-Ⅸ"},
        { WEAPON_BALROG11, KNIFE_NORMAL, WEAPONPAINTPOISON, "剧毒龙炎Balrog-XI" },
        { WEAPON_PLASMAGUN, KNIFE_NORMAL, WEAPONPAINTPLASMAGUNONLY, "破晓黎明-水瓶座" },
        { WEAPON_SFSNIPER, KNIFE_NORMAL, WEAPONPAINTSFSNIPERONLY, "雷神-辐射" },
        { WEAPON_SPRIFLE, KNIFE_NORMAL, WEAPONPAINTSPRIFLEONLY, "狂暴蒸汽-狂想曲" },
        { WEAPON_BOW, KNIFE_NORMAL, WEAPONPAINTBOWONLY, "鹰眼-糖果幻境" },
        { WEAPON_INFINITYEX2, KNIFE_NORMAL, WEAPONPAINTINFINITYEX2ONLY, "金红双蝎-生死时速" },
        { WEAPON_M950SE, KNIFE_NORMAL, WEAPONPAINTM950SEONLY, "黯夜流光-蜂语蜜意" },
        { WEAPON_LASERMINIGUN, KNIFE_NORMAL, WEAPONPAINTLASERMINIGUNONLY, "爆能终结者-灵息魔蛇" },
        { WEAPON_DESTROYER, KNIFE_NORMAL, WEAPONPAINTDESTROYERONLY, "虚空破碎-机动战士" },
        { WEAPON_KNIFE, KNIFE_STRONGKNIFE, WEAPONPAINTNATAKNIFEKR, "背刺银刃-血焰" },
        { WEAPON_AIRBURSTER, KNIFE_NORMAL, WEAPONPAINTAIRBURSTERCN, "风魔啸天破-东风" },
        { WEAPON_CHAINSAWM, KNIFE_NORMAL, WEAPONPAINTCHAINSAWMTW, "死神使者-光晕" },
        { WEAPON_PETROLBOOMER, KNIFE_NORMAL, WEAPONPAINTPETROLBOOMERJP, "暴风烈焰-迷彩" },
        { WEAPON_SFPISTOL, KNIFE_NORMAL, WEAPONPAINTSFPISTOLNA, "死亡射线-矩阵" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINTDESERTEAGLEID, "DesertEagle-雷龙" },
        { WEAPON_SGMISSILE, KNIFE_NORMAL, WEAPONPAINTGS18, "撼宇碎星-幻彩秘银" },
        { WEAPON_GUNKATA, KNIFE_NORMAL, WEAPONPAINTGS18, "【深渊】Elites-幻彩秘银" },
        { WEAPON_KNIFE, KNIFE_DUALSWORD, WEAPONPAINTGS18, "【弑神】双剑-幻彩秘银" },
        { WEAPON_KNIFE, KNIFE_HOLYSWORD, WEAPONPAINT19S2CT, "【辉煌】圣剑-天界守护" },
        { WEAPON_BALROG11, KNIFE_NORMAL, WEAPONPAINTCHN11THBALROG11, "龙炎Balrog-Ⅺ周年纪念" },
        { WEAPON_CROW11, KNIFE_NORMAL, WEAPONPAINTCHN11THCROW11, "飞瀑Turbulent-11周年纪念" },
        { WEAPON_JANUS11, KNIFE_NORMAL, WEAPONPAINTCHN11THJANUS11, "极光Janus-11周年纪念" },
        { WEAPON_THANATOS11, KNIFE_NORMAL, WEAPONPAINTCHN11THTHANATOS11, "魄灭Thanatos-11周年纪念" },
        { WEAPON_TURBULENT11, KNIFE_NORMAL, WEAPONPAINTCHN11THTURBULENT11, "飞瀑Turbulent-11周年纪念" },
        { WEAPON_SKULL11, KNIFE_NORMAL, WEAPONPAINTCHN11THSKULL11, "雷霆SKULL-11周年纪念" },
        { WEAPON_AK47, KNIFE_NORMAL, WEAPONPAINTIRON, "AK47-铁皮人" },
        { WEAPON_LOCKONGUN, KNIFE_NORMAL, WEAPONPAINTIRON, "【暮光】F2000-铁皮人" },
        { WEAPON_M95TIGER, KNIFE_NORMAL, WEAPONPAINTIRON, "【虎王】M95-铁皮人" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINTIRON, "AWP-铁皮人" },
        { WEAPON_BUFFM249, KNIFE_NORMAL, WEAPONPAINTIRON, "【朱雀】M249-铁皮人" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINTWOOD, "D.Eagle-稻草人" },
        { WEAPON_THUNDERPISTOL, KNIFE_NORMAL, WEAPONPAINTWOOD, "【幽影】Thunder.50-稻草人" },
        { WEAPON_M3DRAGON, KNIFE_NORMAL, WEAPONPAINTWOOD, "【天龙】M3-稻草人" },
        { WEAPON_MP5N, KNIFE_NORMAL, WEAPONPAINTWOOD, "MP5-稻草人" },
        { WEAPON_CANNONEX, KNIFE_NORMAL, WEAPONPAINTNEON, "赤焰魔龙-霓虹" },
        { WEAPON_SGDRILL, KNIFE_NORMAL, WEAPONPAINTNEON, "怒海狂鲨-霓虹" },
        { WEAPON_SVD, KNIFE_NORMAL, WEAPONPAINTNEON, "SVD-霓虹" },
        { WEAPON_BROAD, KNIFE_NORMAL, WEAPONPAINTNEON, "冰瀑毁灭者-霓虹" },
        { WEAPON_M950, KNIFE_NORMAL, WEAPONPAINTNEON, "卡利柯M950-霓虹" },
        { WEAPON_KRONOS3, KNIFE_NORMAL, WEAPONPAINTPSYCHEDELIC, "追踪者X-90 涂鸦" },
        { WEAPON_M32VENOM, KNIFE_NORMAL, WEAPONPAINTPSYCHEDELIC, "腐化重炮M32-涂鸦" },
        { WEAPON_SG550, KNIFE_NORMAL, WEAPONPAINTPSYCHEDELIC, "SG550-涂鸦" },
        { WEAPON_XM1014, KNIFE_NORMAL, WEAPONPAINTPSYCHEDELIC, "XM1014-涂鸦" },
        { WEAPON_KNIFE, KNIFE_STRONGKNIFE, WEAPONPAINTPSYCHEDELIC, "背刺银刃-涂鸦" },
        { WEAPON_KRONOS3, KNIFE_NORMAL, WEAPONPAINTREAPER, "追踪者X-90 格式化" },
        { WEAPON_KNIFE, KNIFE_DUALSWORDM, WEAPONPAINTBNW, "炼狱双刃-昼夜" },
        { WEAPON_GUNGNIR, KNIFE_NORMAL, WEAPONPAINTSIGMA, "断域雷魂-核心转换" },
        { WEAPON_USAS12, KNIFE_NORMAL, WEAPONPAINTCHN12TH, "USAS-12周年庆典" },
        { WEAPON_KSG12, KNIFE_NORMAL, WEAPONPAINTCHN12TH, "灼热剃刀KSG-12周年庆典" },
        { WEAPON_SPAS12, KNIFE_NORMAL, WEAPONPAINTCHN12TH, "爆裂炙炎SPAS-12周年庆典" },
        { WEAPON_KRONOS12, KNIFE_NORMAL, WEAPONPAINTCHN12TH, "粉碎者X-12周年庆典" },
        { WEAPON_GUNKATA, KNIFE_NORMAL, WEAPONPAINT20CN, "【深渊】Elites-寒冰凌眼" },
        { WEAPON_GUNKATA, KNIFE_NORMAL, WEAPONPAINT20KR, "【深渊】Elites-粉色点点" },
        { WEAPON_GUNGNIR, KNIFE_NORMAL, WEAPONPAINT20TW, "断域雷魂-机械蜂巢" },
        { WEAPON_KNIFE, KNIFE_SUMMONKNIFE, WEAPONPAINT20NA, "星际仲裁-黄金符文" },
        { WEAPON_BUFFAUG, KNIFE_NORMAL, WEAPONPAINTFLAKE, "【无双】AUG-雪花" },
        { WEAPON_FAMAS, KNIFE_NORMAL, WEAPONPAINTFLAKE, "Famas-雪花" },
        { WEAPON_BOUNCER, KNIFE_NORMAL, WEAPONPAINTFLAKE, "破灭光雷-雪花" },
        { WEAPON_PLASMAGUN, KNIFE_NORMAL, WEAPONPAINTFLAKE, "破晓黎明-雪花" },
        { WEAPON_KNIFE, KNIFE_DUALSWORD, WEAPONPAINTFLAKE, "【弑神】双剑-雪花" },
        { WEAPON_KRONOS3, KNIFE_NORMAL, WEAPONPAINTCRACKED, "追踪者X-90 深寒" },
        { WEAPON_KNIFE, KNIFE_HOLYSWORD, WEAPONPAINTCRACKED, "【辉煌】圣剑-深寒" },
        { WEAPON_MP5N, KNIFE_NORMAL, WEAPONPAINTCRACKED, "MP5-深寒" },
        { WEAPON_MK3A1, KNIFE_NORMAL, WEAPONPAINTCRACKED, "汽锤MK3A1-深寒" },
        { WEAPON_DESPERADO, KNIFE_NORMAL, WEAPONPAINTCRACKED, "【永恒】Python-深寒" },
        { WEAPON_BUFFAUG, KNIFE_NORMAL, WEAPONPAINT21RB, "【无双】AUG-MK7" },
        { WEAPON_RAILCANNON, KNIFE_NORMAL, WEAPONPAINTDKNIGHT, "电浆轨道炮-龙骑士" },
        { WEAPON_SG550, KNIFE_NORMAL, WEAPONPAINTDKNIGHT, "SG550-龙骑士" },
        { WEAPON_KNIFE, KNIFE_STORMGIANT, WEAPONPAINTDKNIGHT, "【星陨】巨锤-龙骑士" },
        { WEAPON_AIRBURSTER, KNIFE_NORMAL, WEAPONPAINTDKNIGHT, "风魔啸天破-龙骑士" },
        { WEAPON_BLOODHUNTER, KNIFE_NORMAL, WEAPONPAINTDKNIGHT, "【血契】D.Eagle-龙骑士" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINTHIGHELF, "D.Eagle-精灵" },
        { WEAPON_BROAD, KNIFE_NORMAL, WEAPONPAINTHIGHELF, "冰瀑毁灭者-精灵" },
        { WEAPON_KRONOS12, KNIFE_NORMAL, WEAPONPAINTHIGHELF, "粉碎者X-12 精灵" },
        { WEAPON_BENDITA, KNIFE_NORMAL, WEAPONPAINTHIGHELF, "冰封圣骑-精灵" },
        { WEAPON_KNIFE, KNIFE_LANCE, WEAPONPAINTHIGHELF, "【天罚】龙枪-精灵" },
        { WEAPON_GLOCK18, KNIFE_NORMAL, WEAPONPAINTCLOCK, "Glock18-齿轮" },
        { WEAPON_KNIFE, KNIFE_SKULLAXE, WEAPONPAINTCLOCK, "旋风SKULL-9 齿轮" },
        { WEAPON_MAGICMG, KNIFE_NORMAL, WEAPONPAINTCLOCK, "红宝石之心-齿轮" },
        { WEAPON_CHAINSAW, KNIFE_NORMAL, WEAPONPAINTCLOCK, "生命收割者-齿轮" },
        { WEAPON_JETGUN, KNIFE_LANCE, WEAPONPAINTCLOCK, "【天驱】突击套装-齿轮" },
        { WEAPON_K3, KNIFE_NORMAL, WEAPONPAINTCOPPER, "K3-黄铜" },
        { WEAPON_BALROG11, KNIFE_NORMAL, WEAPONPAINTCOPPER, "龙炎Balrog-XI 黄铜" },
        { WEAPON_M950SE, KNIFE_NORMAL, WEAPONPAINTCOPPER, "卡利柯M950-黄铜" },
        { WEAPON_BUFFSG552, KNIFE_NORMAL, WEAPONPAINTCOPPER, "【狼魂】SG552-黄铜" },
        { WEAPON_KNIFE, KNIFE_RUNEBLADE, WEAPONPAINTCOPPER, "【魔魂】重剑-黄铜" },
        { WEAPON_SGMISSILE, KNIFE_NORMAL, WEAPONPAINTTYRANNO, "撼宇碎星-暴龙" },
        { WEAPON_BUFFAUG, KNIFE_SKULLAXE, WEAPONPAINTTYRANNO, "【无双】AUG-暴龙" },
        { WEAPON_KRONOS12, KNIFE_NORMAL, WEAPONPAINTTYRANNO, "粉碎者X-12 暴龙" },
        { WEAPON_CROW1, KNIFE_NORMAL, WEAPONPAINTTYRANNO, "幽凰CROW-1 暴龙" },
        { WEAPON_KNIFE, KNIFE_STRONGKNIFE, WEAPONPAINTTYRANNO, "背刺银刃-暴龙" },
        { WEAPON_BUFFM249, KNIFE_NORMAL, WEAPONPAINTTRICERA, "【朱雀】M249-角龙" },
        { WEAPON_SPEARGUN, KNIFE_NORMAL, WEAPONPAINTTRICERA, "海皇之怒-角龙" },
        { WEAPON_KNIFE, KNIFE_MAGICKNIFE, WEAPONPAINTTRICERA, "幻镜！光棱剑-角龙" },
        { WEAPON_CHARGER5, KNIFE_NORMAL, WEAPONPAINTTRICERA, "追电Charger-5 角龙" },
        { WEAPON_ELITE, KNIFE_NORMAL, WEAPONPAINTTRICERA, "Elites-角龙" },
        { WEAPON_SFSNIPER, KNIFE_NORMAL, WEAPONPAINTCHN13TH, "雷神-周年庆典" },
        { WEAPON_MG3, KNIFE_NORMAL, WEAPONPAINTCHN13TH, "毁灭者MG3-周年庆典" },
        { WEAPON_CANNON, KNIFE_NORMAL, WEAPONPAINTCHN13TH, "黑龙炮-周年庆典" },
        { WEAPON_M3DRAGON, KNIFE_NORMAL, WEAPONPAINTCHN13TH, "【天龙】M3-周年庆典" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINTPARK, "AWP-游乐园" },
        { WEAPON_KNIFE, KNIFE_VULCANUS9, WEAPONPAINTPARK, "炽天断Vul-9 游乐园" },
        { WEAPON_INFINITYEX2, KNIFE_NORMAL, WEAPONPAINTPARK, "金红双蝎-游乐园" },
        { WEAPON_KNIFE, KNIFE_ARMTORCH, WEAPONPAINTPARK, "爆焰火拳-游乐园" },
        { WEAPON_HUNTBOW, KNIFE_NORMAL, WEAPONPAINTPARK, "【魂狩】猎弓-游乐园" },
        { WEAPON_DEAGLE, KNIFE_NORMAL, WEAPONPAINTLUNAR, "D.Eagle-星球" },
        { WEAPON_PETROLBOOMER, KNIFE_NORMAL, WEAPONPAINTLUNAR, "暴风烈焰-星球" },
        { WEAPON_KNIFE, KNIFE_THANATOS9, WEAPONPAINTLUNAR, "殒裂Th-9 星球" },
        { WEAPON_KNIFE, KNIFE_SPKNIFE, WEAPONPAINTLUNAR, "淬火蒸汽-星球" },
        { WEAPON_MGSM, KNIFE_NORMAL, WEAPONPAINTLUNAR, "【合金】M2-HB 星球" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINTLIZARD, "AWP-蜥蜴" },
        { WEAPON_ROCKBREAKER, KNIFE_NORMAL, WEAPONPAINTLIZARD, "震地者-蜥蜴" },
        { WEAPON_SKULL1, KNIFE_NORMAL, WEAPONPAINTLIZARD, "噬魂SKULL-1 蜥蜴" },
        { WEAPON_KRONOS12, KNIFE_NORMAL, WEAPONPAINTLIZARD, "粉碎者X-12 蜥蜴" },
        { WEAPON_KNIFE, KNIFE_STORMGIANT, WEAPONPAINTLIZARD, "【星陨】巨锤-蜥蜴" },
        { WEAPON_M2, KNIFE_NORMAL, WEAPONPAINTCHN14TH, "守护者M2-2022周年" },
        { WEAPON_SPEARGUN, KNIFE_NORMAL, WEAPONPAINTCHN14TH, "海皇之怒-2022周年" },
        { WEAPON_KRONOS7, KNIFE_NORMAL, WEAPONPAINTCHN14TH, "捍卫者X-7 2022周年" },
        { WEAPON_BUFFM4, KNIFE_NORMAL, WEAPONPAINTCHN14TH, "【邪皇】M4A1-2022周年" },
        { WEAPON_XM1014, KNIFE_NORMAL, WEAPONPAINTCYBORG, "XM1014-赛博格" },
        { WEAPON_WATERCANNON, KNIFE_NORMAL, WEAPONPAINTCYBORG, "极冻水加农-赛博格" },
        { WEAPON_KNIFE, KNIFE_TURBULENT9, WEAPONPAINTCYBORG, "惊涛Tu-9 赛博格" },
        { WEAPON_PLASMAGUN, KNIFE_NORMAL, WEAPONPAINTCYBORG, "破晓黎明-赛博格" },
        { WEAPON_GUNKATA, KNIFE_NORMAL, WEAPONPAINTCYBORG, "【深渊】Elites-赛博格" },
        { WEAPON_OICW, KNIFE_NORMAL, WEAPONPAINTTRUMP, "尖端勇士OICW-扑克兵" },
        { WEAPON_AUGEX, KNIFE_NORMAL, WEAPONPAINTTRUMP, "强袭AUG-扑克兵" },
        { WEAPON_KNIFE, KNIFE_VULCANUS9, WEAPONPAINTTRUMP, "炽天断Vul-9 扑克兵" },
        { WEAPON_FGLAUNCHER, KNIFE_NORMAL, WEAPONPAINTTRUMP, "礼花发射器-扑克兵" },
        { WEAPON_RESTRICTIONPISTOL, KNIFE_NORMAL, WEAPONPAINTTRUMP, "【光轮】电击枪-扑克兵" },
        { WEAPON_KNIFE, KNIFE_DVHAMMER, WEAPONPAINTY23CHN77, "火箭重锤-浪漫七夕" },
        { WEAPON_AWP, KNIFE_NORMAL, WEAPONPAINTCUSTOM, "AWP-青葱少女" },
    };

    void PrintToChat(CBasePlayer* target, const std::string& what)
    {
        MESSAGE_BEGIN(MSG_ONE, gmsgSayText, {}, target->edict());
        WRITE_BYTE(0);
        WRITE_STRING(what.c_str());
        MESSAGE_END();
    }

    void PrintToChatAll(const std::string& what)
    {
        MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
        WRITE_BYTE(0);
        WRITE_STRING(what.c_str());
        MESSAGE_END();
    }

    std::string GetPlayerName(CBasePlayer* player)
    {
        return STRING(player->pev->netname);
    }

    Vector vecSize[70] =
    {
    {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}, {0.0, 1.0, 0.0}, {0.0, -1.0, 0.0}, {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}, {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0}, {1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, -1.0},
    {0.0, 0.0, 2.0}, {0.0, 0.0, -2.0}, {0.0, 2.0, 0.0}, {0.0, -2.0, 0.0}, {2.0, 0.0, 0.0}, {-2.0, 0.0, 0.0}, {-2.0, 2.0, 2.0}, {2.0, 2.0, 2.0}, {2.0, -2.0, 2.0}, {2.0, 2.0, -2.0}, {-2.0, -2.0, 2.0}, {2.0, -2.0, -2.0}, {-2.0, 2.0, -2.0}, {-2.0, -2.0, -2.0},
    {0.0, 0.0, 3.0}, {0.0, 0.0, -3.0}, {0.0, 3.0, 0.0}, {0.0, -3.0, 0.0}, {3.0, 0.0, 0.0}, {-3.0, 0.0, 0.0}, {-3.0, 3.0, 3.0}, {3.0, 3.0, 3.0}, {3.0, -3.0, 3.0}, {3.0, 3.0, -3.0}, {-3.0, -3.0, 3.0}, {3.0, -3.0, -3.0}, {-3.0, 3.0, -3.0}, {-3.0, -3.0, -3.0},
    {0.0, 0.0, 4.0}, {0.0, 0.0, -4.0}, {0.0, 4.0, 0.0}, {0.0, -4.0, 0.0}, {4.0, 0.0, 0.0}, {-4.0, 0.0, 0.0}, {-4.0, 4.0, 4.0}, {4.0, 4.0, 4.0}, {4.0, -4.0, 4.0}, {4.0, 4.0, -4.0}, {-4.0, -4.0, 4.0}, {4.0, -4.0, -4.0}, {-4.0, 4.0, -4.0}, {-4.0, -4.0, -4.0},
    {0.0, 0.0, 5.0}, {0.0, 0.0, -5.0}, {0.0, 5.0, 0.0}, {0.0, -5.0, 0.0}, {5.0, 0.0, 0.0}, {-5.0, 0.0, 0.0}, {-5.0, 5.0, 5.0}, {5.0, 5.0, 5.0}, {5.0, -5.0, 5.0}, {5.0, 5.0, -5.0}, {-5.0, -5.0, 5.0}, {5.0, -5.0, -5.0}, {-5.0, 5.0, -5.0}, {-5.0, -5.0, -5.0}
    };

    bool Is_Hull_Vacant(CBasePlayer* playerid, Vector Origin, int Hull)
    {
        TraceResult tr;
        UTIL_TraceHull(Origin, Origin, dont_ignore_monsters, Hull, playerid->edict(), &tr);

        if (!tr.fStartSolid || !tr.fAllSolid) //if (!get_tr2(tr, TR_StartSolid) || !get_tr2(tr, TR_AllSolid)) get_tr2(tr, TR_InOpen))
            return true;

        return false;
    }

    void CheckStuck(CBasePlayer* playerid)
    {
        int hull;
        hull = playerid->pev->flags & FL_DUCKING ? head_hull : human_hull;

        if (!Is_Hull_Vacant(playerid, playerid->pev->origin, hull) && !(playerid->pev->solid & SOLID_NOT))
        {
            //if (!is_hull_vacant(origin, hull, player) && !get_user_noclip(player) && !(pev(player, pev_solid) & SOLID_NOT))

            Vector min, vec, vecOrigin;

            min = playerid->pev->mins;
            vecOrigin = playerid->pev->origin;
            vec[2] = vecOrigin[2];

            for (int i = 0; i < (sizeof(vecSize) / sizeof(Vector)); ++i)
            {
                vec[0] = vecOrigin[0] - min[0] * vecSize[i][0];
                vec[1] = vecOrigin[1] - min[1] * vecSize[i][1];
                vec[2] = vecOrigin[2] - min[2] * vecSize[i][2];

                if (Is_Hull_Vacant(playerid, vec, hull))
                {
                    playerid->pev->origin = vec;
                    PrintToChat(playerid, "解除卡住");
                    break;
                }
            }
        }
        else
            PrintToChat(playerid, "你没有被卡住");
    }

    void ShowTattoMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("纹身管理");
            for (int i = TATTOO_NONE; i < NUM_HANDTATTOO_TYPES; i++)
            {   
                if (context.item(std::to_string(i), g_TattoName[HandTattooName(i)]))
                {
                    MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, adminid->pev);
                    WRITE_BYTE(46);
                    WRITE_BYTE(i);
                    MESSAGE_END();

                    if (i == TATTOO_NONE)
                        PrintToChat(adminid, "已清除纹身");
                    else
                        PrintToChat(adminid, "已应用纹身");
                }

            }
            context.end(adminid);
            });
    }

    void ShowKillConfirmMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("确认自杀吗");
           
            if (context.item("Yes", "是"))
            {
                return ClientKill(adminid->edict());
            }

            context.end(adminid);
            });
    }

    void ShowZombieSkinMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("僵尸皮肤管理");

            g_ZombieSkinName.size();

            for (auto it = g_ZombieSkinName.begin(); it != g_ZombieSkinName.end(); it++) {

                char szMenuSkinName[64];

                if (adminid->m_iZombieClassSkin[it->second.ClassType] == it->first)
                    Q_snprintf(szMenuSkinName, sizeof(szMenuSkinName), "%s[使用中]", it->second.SkinName);
                else
                    Q_snprintf(szMenuSkinName, sizeof(szMenuSkinName), "%s", it->second.SkinName);

                if (context.item(it->second.SkinName, szMenuSkinName))
                {
                    if (adminid->m_iZombieClassSkin[it->second.ClassType] == it->first)
                    {
                        adminid->m_iZombieClassSkin[it->second.ClassType] = 0;

                        PrintToChat(adminid, "已清除皮肤");
                    }
                    else
                    {
                        adminid->m_iZombieClassSkin[it->second.ClassType] = it->first;

                        PrintToChat(adminid, "已应用皮肤");
                    }
                }
            }

            context.end(adminid);
            });
    }

    template<class CallbackFn = void(*)(CBasePlayer* target), class FilterFn = bool(*)(CBasePlayer* target)>
    void ShowWeaponPaintMenu(CBasePlayer* adminid, const std::string& szAction, int iItemID, CallbackFn&& fnAction, FilterFn&& fnFilter = [](CBasePlayer*) { return true;  })
    {

        ImMenu(adminid, [szAction, iItemID](auto&& context, CBasePlayer* adminid) {
            context.begin(szAction);

            for (const WeaponPaintMenu& it : g_WeaponPaintName)
            {
                if (it.itemID != iItemID)
                    continue;

                char szMenuPaintName[64];

                if (adminid->m_uiWeaponPaintType[it.WeaponID] == it.itemID || adminid->m_uiKnifePaintType[it.KnifeID] == it.itemID)
                    Q_snprintf(szMenuPaintName, sizeof(szMenuPaintName), "%s[使用中]", it.weaponpaintName);
                else
                    Q_snprintf(szMenuPaintName, sizeof(szMenuPaintName), "%s", it.weaponpaintName);

                if (context.item(it.weaponpaintName, szMenuPaintName))
                {
                    if (adminid->m_uiWeaponPaintType[it.WeaponID] == it.itemID || adminid->m_uiKnifePaintType[it.KnifeID] == it.itemID)
                    {
                        adminid->m_uiWeaponPaintType[it.WeaponID] = 0;
                        adminid->m_uiKnifePaintType[it.KnifeID] = 0;

                        PrintToChat(adminid, "已清除皮肤");
                    }
                    else
                    {
                        if (it.WeaponID == WEAPON_KNIFE)
                            adminid->m_uiKnifePaintType[it.KnifeID] = it.itemID;
                        else
                            adminid->m_uiWeaponPaintType[it.WeaponID] = it.itemID;

                        PrintToChat(adminid, "已应用皮肤");
                    }
                }
            }

            context.end(adminid);
            });
    }

    void ShowPaintMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("武器涂装系统");

            g_PaintName.size();

            for (auto it = g_PaintName.begin(); it != g_PaintName.end(); it++) {

                if (context.item(it->second.paintName, it->second.paintName))
                {
                    ShowWeaponPaintMenu(adminid, it->second.paintName, it->second.paintType, [](CBasePlayer* player) {  return true;  });
                }
            }

            context.end(adminid);
        });
    }

    void ShowEmotionMenu(CBasePlayer* adminid)
    {
        if (!adminid->IsAlive())
            return;

        std::string strText = PlayerEmotionManager().GetEmotionByModelID(adminid->m_iModelName).radioText;

        int iCount = PlayerEmotionManager().GetEmotionByModelID(adminid->m_iModelName).emotionCount;

        ShowMenu(adminid, (((1 << iCount) - 1) | MENU_KEY_0), -1, FALSE, strText.c_str());
        adminid->m_iMenu = Menu_Emotion;
    }

    void ShowCameraMenu(CBasePlayer* adminid)
    {
        ImMenu(adminid, [](auto context, CBasePlayer* adminid) {
            context.begin("摄影机菜单");

            if (context.item("firstperson", "第一人称视角"))
            {
                adminid->ClientCommand("firstperson");
            }

            if (context.item("thirdperson", "第三人称视角"))
            {
                adminid->ClientCommand("thirdperson");
            }
            
            context.end(adminid);
            });
    }
    
    void ShowMoeFunMenu(CBasePlayer *playerid)
    {
        ImMenu(playerid, [](auto context, CBasePlayer * playerid){         
   
            context.begin("Moe_Fun");

            if (context.item("CheckStuck", "解除卡住"))
                CheckStuck(playerid);

            if (context.item("suicide", "自杀"))
                ShowKillConfirmMenu(playerid);

            if (context.item("cameramenu", "摄影机菜单"))
                ShowCameraMenu(playerid);

            if (context.item("wdnmd", "全明星语音菜单"))
                wdnmdmenu::ShowWdnmdMenu(playerid);

            if (context.item("tattomenu", "纹身管理"))
                ShowTattoMenu(playerid);

            if (context.item("zombieskinmenu", "僵尸皮肤管理"))
                ShowZombieSkinMenu(playerid);

            if (context.item("weaponpaintmenu", "武器涂装系统"))
                ShowPaintMenu(playerid);

            if (context.item("emotionmenu", "表情系统"))
                ShowEmotionMenu(playerid);

            context.end(playerid);
        });
    }
    }
}