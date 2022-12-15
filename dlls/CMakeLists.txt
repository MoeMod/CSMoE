#
# Copyright (c) 2016 Alibek Omarov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHE
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.10.2)

project (server)

set (SERVER_LIBRARY server)

#--------------
# USER DEFINES \
################\
set(GAMEDIR "csmoe" CACHE STRING "Gamedir path")
set(SERVER_INSTALL_DIR "dlls" CACHE STRING "Where put server dll")
if(ANDROID OR IOS)
	set(SERVER_LIBRARY_NAME "server" CACHE STRING "Library name for Android/iOS")
elseif(NOT WIN32)
	set(SERVER_LIBRARY_NAME "cs" CACHE STRING "Library name for Linux/MacOS")
else()
	set(SERVER_LIBRARY_NAME "mp" CACHE STRING "Library name for Windows")
endif()

#-----------------
# MAIN BUILD CODE \
###################\

set(HLDLL_SRCS
	./airtank.cpp
	./ammo.cpp
	./animating.cpp
	./animation.cpp
	./bmodels.cpp
	./buttons.cpp
	./career_tasks.cpp
	./cbase.cpp
	./client.cpp
	./combat.cpp
	./debug.cpp
	./doors.cpp
	./effects.cpp
	./explode.cpp
	./func_break.cpp
	./func_tank.cpp
	./game.cpp
	./gamerules.cpp
	./ggrenade.cpp
	./globals.cpp
	./h_ai.cpp
	./h_battery.cpp
	./h_cycler.cpp
	./healthkit.cpp
	./h_export.cpp
	./player/csdm_randomspawn.cpp
	./player/hintmessage.cpp
	./player/player_msg.cpp
	./player/player_spawnpoint.cpp
	./player/player_model.cpp
	./player/player_zombie.cpp
	./player/player_spawnprotection.cpp
	./player/player_knockback.cpp
	./player/player_human_level.cpp
	./items.cpp
	./lights.cpp
	./maprules.cpp
	./mortar.cpp
	./mpstubb.cpp
	./multiplay_gamerules.cpp
	./observer.cpp
	./pathcorner.cpp
	./plane.cpp
	./plats.cpp
	./player.cpp
	./singleplay_gamerules.cpp
	./skill.cpp
	./sound.cpp
	./soundent.cpp
	./spectator.cpp
	./subs.cpp
	./training_gamerules.cpp
	./triggers.cpp
	./tutor_base_states.cpp
	./tutor_base_tutor.cpp
	./tutor.cpp
	./tutor_cs_states.cpp
	./tutor_cs_tutor.cpp
	./util.cpp
	./vehicle.cpp
	./weapons.cpp
	./weapons_buy.cpp
	./world.cpp
	./bot/cs_bot_chatter.cpp
	./bot/cs_bot.cpp
	./bot/cs_bot_event.cpp
	./bot/cs_bot_init.cpp
	./bot/cs_bot_learn.cpp
	./bot/cs_bot_listen.cpp
	./bot/cs_bot_manager.cpp
	./bot/cs_bot_nav.cpp
	./bot/cs_bot_pathfind.cpp
	./bot/cs_bot_radio.cpp
	./bot/cs_bot_statemachine.cpp
	./bot/cs_bot_update.cpp
	./bot/cs_bot_vision.cpp
	./bot/cs_bot_weapon.cpp
	./bot/cs_gamestate.cpp
	./bot/states/cs_bot_attack.cpp
	./bot/states/cs_bot_buy.cpp
	./bot/states/cs_bot_defuse_bomb.cpp
	./bot/states/cs_bot_escape_from_bomb.cpp
	./bot/states/cs_bot_fetch_bomb.cpp
	./bot/states/cs_bot_follow.cpp
	./bot/states/cs_bot_hide.cpp
	./bot/states/cs_bot_hunt.cpp
	./bot/states/cs_bot_idle.cpp
	./bot/states/cs_bot_investigate_noise.cpp
	./bot/states/cs_bot_move_to.cpp
	./bot/states/cs_bot_plant_bomb.cpp
	./bot/states/cs_bot_use_entity.cpp
	./hostage/hostage.cpp
	./hostage/hostage_improv.cpp
	./hostage/hostage_localnav.cpp
	./hostage/states/hostage_animate.cpp
	./hostage/states/hostage_escape.cpp
	./hostage/states/hostage_follow.cpp
	./hostage/states/hostage_idle.cpp
	./hostage/states/hostage_retreat.cpp
	./gamemode/mods.cpp
	./gamemode/mod_base.cpp
	./gamemode/mod_none.cpp
	./gamemode/mod_tdm.cpp
	./gamemode/mod_dm.cpp
	./gamemode/mod_zb1.cpp
	./gamemode/mod_zbs.cpp
	./gamemode/zbs/zs_subs.cpp
	./gamemode/zbs/monster_entity.cpp
	./gamemode/mod_zb2.cpp
	./gamemode/zb2/supplybox.cpp
	./bot/cs_bot_bhopjump.cpp
	./weapons_data.cpp
	./weapons_msg.cpp
	./player/player_mod_strategy.cpp
	./gamemode/zbs/monster_manager.cpp
	./player/player_account.cpp
	./cbase/cbase_memory.cpp
	./gamemode/mod_zb3.cpp
	./gamemode/mod_gd.cpp
	./gamemode/zb2/zb2_skill_human.cpp
	./gamemode/zb2/zb2_skill_zombie.cpp
	./gamemode/zb2/zb2_zclass.cpp
	./gamemode/zb2/zb2_zclass_tank.cpp
	./gamemode/zb2/zb2_zclass_speed.cpp
	./gamemode/zb2/zb2_zclass_deimos2.cpp
	./gamemode/zb2/zb2_zclass_deimos.cpp
	./gamemode/zb2/zb2_zclass_pc.cpp
	./gamemode/zb2/zb2_zclass_heal.cpp
	./gamemode/zb2/zb2_zclass_heavy.cpp
	./weapons_precache.cpp
	./gamemode/zbb/zbb_basebuilder.cpp
	./gamemode/zbb/zbb_ghost.cpp
	./gamemode/zb1/zb1_countdown.cpp
	./cbase/cbase_entity_factory.cpp
	./cbase/cbase_physint.cpp
	./gamemode/zbb/zbb_zombie.cpp
	./refl/cbase_export.cpp
	./bot/states/cs_bot_supplybox_rush.cpp
	./gamemode/zb2/zb2_zclass_teleport.cpp
	./gamemode/mod_zbb.cpp
	./gamemode/zb2/deimostail.cpp
	./gamemode/zb2/teleportentity.cpp
	./gamemode/zb2/zombiebomb.cpp
	./gamemode/zb2/zb2_zclass_booster.cpp
	./gamemode/zb2/zb2_zclass_china.cpp
	./gamemode/zb2/zb2_zclass_pass.cpp
	./gamemode/zb2/zb2_zclass_flying.cpp
	./gamemode/zb2/zb2_zclass_nemesis.cpp
	./balrog11cannon.cpp
	./bunkerbuster.cpp
	./gamemode/mod_zbz.cpp
	./gamemode/zbz/zbz_skill.cpp
	./map_manager.cpp
	./bot/states/cs_bot_defend.cpp
	./gamemode/zb2/zb2_zclass_revival.cpp
	./gamemode/zb2/zombietrap.cpp
	./gamemode/zb2/zombiepile.cpp
	./gamemode/zb2/zb2_zclass_stamper.cpp
	./gamemode/mod_z4.cpp
	./gamemode/zb2/zb2_zclass_resident.cpp
	./gamemode/zb2/zb2_zclass_witch.cpp
	./gamemode/zb2/zombiehook.cpp
	./gamemode/zb2/zb2_zclass_zbs_deimos2.cpp
	./gamemode/zb2/zb2_zclass_aksha.cpp
	./gamemode/zb2/fireball.cpp
	./gamemode/zb2/zb2_zclass_boomer.cpp
	./holyswordcannon.cpp
	./player/player_fuck.cpp
	./gamemode/zb2/zb2_zclass_spider.cpp
	./gamemode/zb2/zb2_zclass_deathknight.cpp
	./gamemode/zb2/webbomb.cpp
	./gamemode/zb2/zb2_zclass_meatwall.cpp
	./gamemode/zb2/zombidogegg.cpp
	./cannonexdragon.cpp
	./spearammo.cpp
	./summonknifecannon.cpp
	./gamemode/mod_z4e.cpp
	./sgmissilecannon.cpp
	./newmenus.cpp
	./client/admin.cpp
	./gamemode/zb2/zb2_zclass_siren.cpp
	./m95tigernet.cpp
	./client/ghosthuntermenu.cpp
	./gamemode/mod_zsh.cpp
	./gamemode/zsh/zsh_building.cpp
	./m3dragoncannon.cpp
	./m3dragoncannon_dragon.cpp
	./wondercannonchain.cpp
	./wondercannonmine.cpp
	./y22s2sfpistolfield.cpp
	./pianogunwave.cpp
	./client/wdnmd.cpp
	./database/db_config.cpp
	./database/db_main.cpp
	./gamemode/mod_zb3zp.cpp
	./lockongun_missile.cpp
	./gamemode/zb3zp/zp_zclass_alien_boss.cpp
	./gamemode/zb3zp/zp_zclass_source.cpp
	./gamemode/zb3zp/zp_zclass_tank.cpp
	./gamemode/zb3zp/zp_zclass_alien_infected.cpp
	./gamemode/zb3zp/zp_zclass_ap.cpp
	./gamemode/zb3zp/zp_zclass_hostex.cpp
	./gamemode/zb3zp/zp_zclass_licker.cpp
	./gamemode/zb3zp/zp_zclass_nohead.cpp
	./gamemode/zb3zp/zp_zclass_pyrammid.cpp
	./gamemode/zb3zp/zp_zclass_tf.cpp
	./gamemode/zb3zp/zp_zclass_human.cpp
	./guillotineammo.cpp
	./guillotineexammo.cpp
	./voidpistolblackhole.cpp
	./throwingknifeentity.cpp
	./magicsgcannon.cpp
	)

set (HLWPN_SRCS
	../dlls/wpn_shared/wpn_ak47.cpp
	../dlls/wpn_shared/wpn_aug.cpp
	../dlls/wpn_shared/wpn_awp.cpp
	../dlls/wpn_shared/wpn_c4.cpp
	../dlls/wpn_shared/wpn_deagle.cpp
	../dlls/wpn_shared/wpn_elite.cpp
	../dlls/wpn_shared/wpn_famas.cpp
	../dlls/wpn_shared/wpn_fiveseven.cpp
	../dlls/wpn_shared/wpn_flashbang.cpp
	../dlls/wpn_shared/wpn_g3sg1.cpp
	../dlls/wpn_shared/wpn_galil.cpp
	../dlls/wpn_shared/wpn_glock18.cpp
	../dlls/wpn_shared/wpn_hegrenade.cpp
	../dlls/wpn_shared/wpn_knife.cpp
	../dlls/wpn_shared/wpn_m249.cpp
	../dlls/wpn_shared/csgo_mag7.cpp
	../dlls/wpn_shared/csgo_negev.cpp
	../dlls/wpn_shared/csgo_bizon.cpp
	../dlls/wpn_shared/csgo_cz75.cpp
	../dlls/wpn_shared/csgo_tec9.cpp
	../dlls/wpn_shared/csgo_zeus.cpp
	../dlls/wpn_shared/csgo_r8.cpp
	../dlls/wpn_shared/csgo_sawedoff.cpp
	../dlls/wpn_shared/wpn_stg44.cpp
	../dlls/wpn_shared/wpn_voidpistol.cpp
	../dlls/wpn_shared/wpn_gatlingex.cpp
	../dlls/wpn_shared/wpn_m3.cpp
	../dlls/wpn_shared/wpn_m4a1.cpp
	../dlls/wpn_shared/wpn_mac10.cpp
	../dlls/wpn_shared/wpn_mp5navy.cpp
	../dlls/wpn_shared/wpn_p228.cpp
	../dlls/wpn_shared/wpn_p90.cpp
	../dlls/wpn_shared/wpn_scout.cpp
	../dlls/wpn_shared/wpn_sg550.cpp
	../dlls/wpn_shared/wpn_sg552.cpp
	../dlls/wpn_shared/wpn_smokegrenade.cpp
	../dlls/wpn_shared/wpn_molotov.cpp
	../dlls/wpn_shared/wpn_tmp.cpp
	../dlls/wpn_shared/wpn_ump45.cpp
	../dlls/wpn_shared/wpn_usp.cpp
	../dlls/wpn_shared/wpn_xm1014.cpp
	../dlls/wpn_shared/knife_zombi.cpp
	../dlls/wpn_shared/wpn_zombiebomb.cpp
	../dlls/wpn_shared/wpn_ak47l.cpp
	../dlls/wpn_shared/wpn_mp7a1d.cpp
	../dlls/wpn_shared/wpn_deagled.cpp
	../dlls/wpn_shared/wpn_as50.cpp
	../dlls/wpn_shared/wpn_cannon.cpp
	../dlls/wpn_shared/wpn_sgdrill.cpp
	../dlls/wpn_shared/z4b_cannontiger.cpp
	../dlls/wpn_shared/z4b_cannonplasma.cpp
	../dlls/wpn_shared/wpn_infinity.cpp
	../dlls/wpn_shared/wpn_k1a.cpp
	../dlls/wpn_shared/wpn_kriss.cpp
	../dlls/wpn_shared/wpn_m95.cpp
	../dlls/wpn_shared/wpn_buffsg552ex.cpp
	../dlls/wpn_shared/wpn_m1887.cpp
	../dlls/wpn_shared/wpn_scarh.cpp
	../dlls/wpn_shared/wpn_scarl.cpp
	../dlls/wpn_shared/wpn_tar21.cpp
	../dlls/wpn_shared/wpn_thompson.cpp
	../dlls/wpn_shared/wpn_wa2000.cpp
	../dlls/wpn_shared/wpn_xm8c.cpp
	../dlls/wpn_shared/wpn_xm8s.cpp
	../dlls/wpn_shared/knife_skullaxe.cpp
	../dlls/wpn_shared/knife_katana.cpp
	../dlls/wpn_shared/knife_dragonsword.cpp
	../dlls/wpn_shared/knife_dgaxe.cpp
	../dlls/wpn_shared/wpn_gungnir.cpp
	../dlls/wpn_shared/wpn_mg3.cpp
	../dlls/wpn_shared/wpn_desperado.cpp
	../dlls/wpn_shared/wpn_svdex.cpp
	../dlls/wpn_shared/wpn_mp7a1c.cpp
	../dlls/wpn_shared/wpn_mp7a1p.cpp
	../dlls/wpn_shared/wpn_infinitys.cpp
	../dlls/wpn_shared/wpn_m14ebr.cpp
	../dlls/wpn_shared/wpn_infinityex1.cpp
	../dlls/wpn_shared/wpn_infinityex2.cpp
	../dlls/wpn_shared/knife_dualsword.cpp
	../dlls/wpn_shared/wpn_anaconda.cpp
	../dlls/wpn_shared/wpn_tbarrel.cpp
	../dlls/wpn_shared/wpn_mg36.cpp
	../dlls/wpn_shared/wpn_balrog7.cpp
	../dlls/wpn_shared/wpn_gatling.cpp
	../dlls/wpn_shared/wpn_bow.cpp
	../dlls/wpn_shared/wpn_huntbow.cpp
	../dlls/wpn_shared/wpn_m2.cpp
	../dlls/wpn_shared/wpn_janus7xmas.cpp
	../dlls/wpn_shared/wpn_chainsaw.cpp
	../dlls/wpn_shared/wpn_m134.cpp
	../dlls/wpn_shared/wpn_m134xmas.cpp
	../dlls/wpn_shared/wpn_mg3xmas.cpp
	../dlls/wpn_shared/wpn_m95xmas.cpp
	../dlls/wpn_shared/wpn_m4a1dragon.cpp
	../dlls/wpn_shared/wpn_ak47dragon.cpp
	../dlls/wpn_shared/wpn_knifedragon.cpp
	../dlls/wpn_shared/wpn_m134ex.cpp
	../dlls/wpn_shared/wpn_starchaserar.cpp
	../dlls/wpn_shared/wpn_gunkata.cpp
	../dlls/wpn_shared/z4b_m37se.cpp
	../dlls/wpn_shared/z4b_Frequency9.cpp
	../dlls/wpn_shared/z4b_stormgiantx.cpp
	../dlls/wpn_shared/z4b_lc_sword.cpp
	../dlls/wpn_shared/z4b_oppovivo.cpp
	../dlls/wpn_shared/z4b_strongknifex.cpp
	../dlls/wpn_shared/z4b_infinityx.cpp
	../dlls/wpn_shared/z4b_nataknifedx.cpp
	../dlls/wpn_shared/z4b_facelessvoid.cpp
	../dlls/wpn_shared/z4b_m134heroi.cpp
	../dlls/wpn_shared/z4b_deagleb.cpp
	../dlls/wpn_shared/z4b_xm2010pc.cpp
	../dlls/wpn_shared/z4b_dmp7a1x.cpp
	../dlls/wpn_shared/z4b_m4a1razer.cpp
	../dlls/wpn_shared/z4b_awpnvidia.cpp
	../dlls/wpn_shared/z4b_m4a1x.cpp
	../dlls/wpn_shared/z4b_m4a1mw.cpp
	../dlls/wpn_shared/z4b_ak47x.cpp
	../dlls/wpn_shared/wpn_zombiebomb2.cpp
	../dlls/wpn_shared/wpn_bendita.cpp
	../dlls/wpn_shared/wpn_skull5.cpp
	../dlls/wpn_shared/z4b_k1ases.cpp
	../dlls/wpn_shared/z4b_m1887se.cpp
	../dlls/wpn_shared/z4b_frequency1.cpp
	../dlls/wpn_shared/wpn_balrog11.cpp
	../dlls/wpn_shared/wpn_balrog11b.cpp
	../dlls/wpn_shared/wpn_balrog11wc.cpp
	../dlls/wpn_shared/knife_lance.cpp
	../dlls/wpn_shared/wpn_m249ex.cpp
	../dlls/wpn_shared/wpn_sfsmg.cpp
	../dlls/wpn_shared/wpn_sfgun.cpp
	../dlls/wpn_shared/wpn_sfmg.cpp
	../dlls/wpn_shared/wpn_qbarrel.cpp
	../dlls/wpn_shared/wpn_buffm249.cpp
	../dlls/wpn_shared/knife_zombi_z4.cpp
	../dlls/wpn_shared/wpn_pumpkin.cpp
	../dlls/wpn_shared/wpn_buffm4a1.cpp
	../dlls/wpn_shared/knife_stormgiant.cpp
	../dlls/wpn_shared/wpn_buffak47.cpp
	../dlls/wpn_shared/wpn_bison.cpp
	../dlls/wpn_shared/knife_strongknife.cpp
	../dlls/wpn_shared/knife_nataknifed.cpp
	../dlls/wpn_shared/knife_thanatos9.cpp
	../dlls/wpn_shared/wpn_y21s1jetgunmd.cpp
	../dlls/wpn_shared/knife_skullt9.cpp
	../dlls/wpn_shared/z4b_barrettd.cpp
	../dlls/wpn_shared/wpn_sfsniper.cpp
	../dlls/wpn_shared/knife_runeblade.cpp
	../dlls/wpn_shared/wpn_plasmagun.cpp
	../dlls/wpn_shared/knife_janus9.cpp
	../dlls/wpn_shared/knife_balrog9.cpp
	../dlls/wpn_shared/knife_summonknife.cpp
	../dlls/wpn_shared/wpn_revivegun.cpp
	../dlls/wpn_shared/knife_holysword.cpp
	../dlls/wpn_shared/wpn_y20s2scard.cpp
	../dlls/wpn_shared/wpn_zombielaw.cpp
	../dlls/wpn_shared/wpn_m32venom.cpp
	../dlls/wpn_shared/wpn_m95tiger.cpp
	../dlls/wpn_shared/knife_y22s1holyswordmb.cpp
	../dlls/wpn_shared/knife_magicknife.cpp
	../dlls/wpn_shared/knife_whipsword.cpp
	../dlls/wpn_shared/knife_zombi_z5.cpp
	../dlls/wpn_shared/z4b_dualkukri.cpp
	../dlls/wpn_shared/wpn_cannonex.cpp
	../dlls/wpn_shared/wpn_speargun.cpp
	../dlls/wpn_shared/z4b_dualslayer.cpp
	../dlls/wpn_shared/wpn_hk23.cpp
	../dlls/wpn_shared/wpn_m60.cpp
	../dlls/wpn_shared/knife_jknife.cpp
	../dlls/wpn_shared/knife_combat.cpp
	../dlls/wpn_shared/knife_mastercombat.cpp
	../dlls/wpn_shared/knife_hdagger.cpp
	../dlls/wpn_shared/knife_axe.cpp
	../dlls/wpn_shared/knife_hammer.cpp
	../dlls/wpn_shared/wpn_mk48.cpp
	../dlls/wpn_shared/wpn_lightzg.cpp
	../dlls/wpn_shared/wpn_heavyzg.cpp
	../dlls/wpn_shared/wpn_bpgm.cpp
	../dlls/wpn_shared/wpn_m950.cpp
	../dlls/wpn_shared/wpn_tmpdragon.cpp
	../dlls/wpn_shared/wpn_zgun.cpp
	../dlls/wpn_shared/wpn_k3.cpp
	../dlls/wpn_shared/wpn_as50g.cpp
	../dlls/wpn_shared/wpn_ak47g.cpp
	../dlls/wpn_shared/wpn_sgmissile.cpp
	../dlls/wpn_shared/wpn_sgmissilem.cpp
	../dlls/wpn_shared/wpn_sgmissileex.cpp
	../dlls/wpn_shared/wpn_cheytaclrrs.cpp
	../dlls/wpn_shared/wpn_m400.cpp
	../dlls/wpn_shared/wpn_m82.cpp
	../dlls/wpn_shared/wpn_m24.cpp
	../dlls/wpn_shared/wpn_trg42.cpp
	../dlls/wpn_shared/wpn_aw50.cpp
	../dlls/wpn_shared/wpn_sl8.cpp
	../dlls/wpn_shared/wpn_sl8g.cpp
	../dlls/wpn_shared/wpn_sl8ex.cpp
	../dlls/wpn_shared/wpn_trg42g.cpp
	../dlls/wpn_shared/wpn_sprifle.cpp
	../dlls/wpn_shared/wpn_spsmg.cpp
	../dlls/wpn_shared/z4b_holyfist.cpp
	../dlls/wpn_shared/z4b_dualstinger.cpp
	../dlls/wpn_shared/z4b_freedom.cpp
	../dlls/wpn_shared/z4b_csg12ss.cpp
	../dlls/wpn_shared/wpn_skull8.cpp
	../dlls/wpn_shared/wpn_skull4.cpp
	../dlls/wpn_shared/wpn_skull6.cpp
	../dlls/wpn_shared/wpn_m249ep.cpp
	../dlls/wpn_shared/z4b_heartstimulater.cpp
	../dlls/wpn_shared/z4b_syringe.cpp
	../dlls/wpn_shared/wpn_skull1.cpp
	../dlls/wpn_shared/wpn_m3dragon.cpp
	../dlls/wpn_shared/wpn_m3dragonm.cpp
	../dlls/wpn_shared/wpn_herochainsaw.cpp
	../dlls/wpn_shared/wpn_chainsawm.cpp
	../dlls/wpn_shared/wpn_ethereal.cpp
	../dlls/wpn_shared/wpn_railgun.cpp
	../dlls/wpn_shared/wpn_thanatos7.cpp
	../dlls/wpn_shared/wpn_crow7.cpp
	../dlls/wpn_shared/wpn_winggun.cpp
	../dlls/wpn_shared/wpn_m950se.cpp
	../dlls/wpn_shared/wpn_chainsawex.cpp
	../dlls/wpn_shared/wpn_lasersg.cpp
	../dlls/wpn_shared/wpn_wondercannon.cpp
	../dlls/wpn_shared/knife_y22s2sfsword.cpp
	../dlls/wpn_shared/wpn_sfpistol.cpp
	../dlls/wpn_shared/wpn_y22s2sfpistol.cpp
	../dlls/wpn_shared/wpn_pianogun.cpp
	../dlls/wpn_shared/wpn_pianogunex.cpp
	../dlls/wpn_shared/wpn_stunrifle.cpp
	../dlls/wpn_shared/z4b_ragingbull.cpp
	../dlls/wpn_shared/wpn_kronos12.cpp
	../dlls/wpn_shared/wpn_crossbowex21.cpp
	../dlls/wpn_shared/wpn_buffawp.cpp
	../dlls/wpn_shared/wpn_divinetitan.cpp
	../dlls/wpn_shared/wpn_railcannon.cpp
	../dlls/wpn_shared/wpn_skull11.cpp
	../dlls/wpn_shared/wpn_janus11.cpp
	../dlls/wpn_shared/wpn_crow5.cpp
	../dlls/wpn_shared/wpn_janusmk5.cpp
	../dlls/wpn_shared/wpn_janus3.cpp
	../dlls/wpn_shared/wpn_crow3.cpp
	../dlls/wpn_shared/wpn_crow11.cpp
	../dlls/wpn_shared/z4b_m60amethyst.cpp
	../dlls/wpn_shared/z4b_ar57amethyst.cpp
	../dlls/wpn_shared/z4b_acramethyst.cpp
	../dlls/wpn_shared/z4b_dbarrelamethyst.cpp
	../dlls/wpn_shared/z4b_deagleamethyst.cpp
	../dlls/wpn_shared/wpn_crossbow.cpp
	../dlls/wpn_shared/wpn_broad.cpp
	../dlls/wpn_shared/wpn_sgdrillex.cpp
	../dlls/wpn_shared/wpn_balrog1.cpp
	../dlls/wpn_shared/wpn_sapientia.cpp
	../dlls/wpn_shared/wpn_balrog3.cpp
	../dlls/wpn_shared/wpn_coilmg.cpp
	../dlls/wpn_shared/wpn_charger7.cpp
	../dlls/wpn_shared/wpn_charger5.cpp
	../dlls/wpn_shared/wpn_airburster.cpp
	../dlls/wpn_shared/wpn_poisongun.cpp
	../dlls/wpn_shared/wpn_flamethrower.cpp
	../dlls/wpn_shared/wpn_watercannon.cpp
	../dlls/wpn_shared/wpn_spmg.cpp
	../dlls/wpn_shared/wpn_lockongun.cpp
	../dlls/wpn_shared/wpn_patroldrone.cpp
	../dlls/wpn_shared/z4b_aquariusplasmagun.cpp
	../dlls/wpn_shared/wpn_fglauncher.cpp
	../dlls/wpn_shared/wpn_bloodhunter.cpp
	../dlls/wpn_shared/wpn_buffaug.cpp
	../dlls/wpn_shared/wpn_buffsg552.cpp
	../dlls/wpn_shared/wpn_m1887g.cpp
	../dlls/wpn_shared/wpn_cameragun.cpp
	../dlls/wpn_shared/wpn_mgsm.cpp
	../dlls/wpn_shared/wpn_bunkerbuster.cpp
	../dlls/wpn_shared/wpn_thunderpistol.cpp
	../dlls/wpn_shared/wpn_balrog5.cpp
	../dlls/wpn_shared/wpn_thanatos11.cpp
	../dlls/wpn_shared/wpn_m1887xmas.cpp
	../dlls/wpn_shared/wpn_kronos5.cpp
	../dlls/wpn_shared/wpn_an94.cpp
	../dlls/wpn_shared/wpn_m16a4.cpp
	../dlls/wpn_shared/wpn_kronos7.cpp
	../dlls/wpn_shared/wpn_kronos3.cpp
	../dlls/wpn_shared/wpn_kronos1.cpp
	../dlls/wpn_shared/wpn_laserfist.cpp
	../dlls/wpn_shared/wpn_laserfistex.cpp
	../dlls/wpn_shared/wpn_rainbowgun.cpp
	../dlls/wpn_shared/wpn_guillotine.cpp
	../dlls/wpn_shared/wpn_guillotineex.cpp
	../dlls/wpn_shared/wpn_speargunm.cpp
	../dlls/wpn_shared/wpn_kingcobra.cpp
	../dlls/wpn_shared/wpn_spsg.cpp
	../dlls/wpn_shared/wpn_destroyer.cpp
	../dlls/wpn_shared/wpn_blockar.cpp
	../dlls/wpn_shared/wpn_blockmg.cpp
	../dlls/wpn_shared/wpn_blockas.cpp
	../dlls/wpn_shared/wpn_coilgun.cpp
	../dlls/wpn_shared/wpn_bouncer.cpp
	../dlls/wpn_shared/wpn_thanatos5.cpp
	../dlls/wpn_shared/wpn_oicw.cpp
	../dlls/wpn_shared/wpn_starchasersr.cpp
	../dlls/wpn_shared/wpn_m1garand.cpp
	../dlls/wpn_shared/wpn_dualkrisshero.cpp
	../dlls/wpn_shared/wpn_skull2.cpp
	../dlls/wpn_shared/wpn_dualuzi.cpp
	../dlls/wpn_shared/wpn_spas12.cpp
	../dlls/wpn_shared/wpn_spas12ex.cpp
	../dlls/wpn_shared/wpn_spas12ex2.cpp
	../dlls/wpn_shared/wpn_gilboaex.cpp
	../dlls/wpn_shared/wpn_musket.cpp
	../dlls/wpn_shared/wpn_buffng7.cpp
	../dlls/wpn_shared/wpn_bufffiveseven.cpp
	../dlls/wpn_shared/wpn_guitar.cpp
	../dlls/wpn_shared/wpn_violingun.cpp
	../dlls/wpn_shared/wpn_holybomb.cpp
	../dlls/wpn_shared/wpn_wondercannonex.cpp
	../dlls/wpn_shared/wpn_cake.cpp
	../dlls/wpn_shared/wpn_mooncake.cpp
	../dlls/wpn_shared/wpn_heartbomb.cpp
	../dlls/wpn_shared/wpn_fgrenade.cpp
	../dlls/wpn_shared/wpn_fgrenade2.cpp
	../dlls/wpn_shared/wpn_m24grenade.cpp
	../dlls/wpn_shared/wpn_cartfrag.cpp
	../dlls/wpn_shared/wpn_sfgrenade.cpp
	../dlls/wpn_shared/wpn_chaingren.cpp
	../dlls/wpn_shared/z4b_tiga.cpp
	../dlls/wpn_shared/wpn_restrictionpistol.cpp
	../dlls/wpn_shared/knife_y22s3dragonsword.cpp
	../dlls/wpn_shared/wpn_janus7.cpp
	../dlls/wpn_shared/wpn_y22s3janus7.cpp
	../dlls/wpn_shared/wpn_voidpistolex.cpp
	../dlls/wpn_shared/wpn_waterpistol.cpp
	../dlls/wpn_shared/knife_monkeywpnset3.cpp
	../dlls/wpn_shared/knife_ozwpnset3.cpp
	../dlls/wpn_shared/wpn_monkeywpnset2.cpp
	../dlls/wpn_shared/wpn_dualkriss.cpp
	../dlls/wpn_shared/wpn_mosin.cpp
	../dlls/wpn_shared/z4b_steyrtmpd.cpp
	../dlls/wpn_shared/wpn_tknife.cpp
	../dlls/wpn_shared/wpn_tknifeex.cpp
	../dlls/wpn_shared/wpn_tknifeex2.cpp
	../dlls/wpn_shared/wpn_laserminigun.cpp
	../dlls/wpn_shared/wpn_mk3a1.cpp
	../dlls/wpn_shared/wpn_mk3a1se.cpp
	../dlls/wpn_shared/wpn_drillgun.cpp
	../dlls/wpn_shared/wpn_vulcanus3.cpp
	../dlls/wpn_shared/wpn_vulcanus11.cpp
	../dlls/wpn_shared/wpn_vulcanus7.cpp
	../dlls/wpn_shared/wpn_skull3.cpp
	../dlls/wpn_shared/wpn_skull3d.cpp
	../dlls/wpn_shared/z4b_deathray.cpp
	../dlls/wpn_shared/wpn_cartredl.cpp
	../dlls/wpn_shared/wpn_cartredh.cpp
	../dlls/wpn_shared/wpn_cartbluec.cpp
	../dlls/wpn_shared/wpn_cartblues.cpp
	../dlls/wpn_shared/knife_tomahawk.cpp
	../dlls/wpn_shared/knife_crowbarcraft.cpp
	../dlls/wpn_shared/knife_vulcanus9.cpp
	../dlls/wpn_shared/wpn_magicsg.cpp
	../dlls/wpn_shared/z4b_malorian3516.cpp
	../dlls/wpn_shared/knife_sfsword.cpp
	../dlls/wpn_shared/knife_dragontail.cpp
	../dlls/wpn_shared/knife_katanad.cpp
	../dlls/wpn_shared/knife_crow9.cpp
	../dlls/wpn_shared/knife_turbulent9.cpp
	../dlls/wpn_shared/knife_spknife.cpp
	../dlls/wpn_shared/knife_swordbombard.cpp
	)

set(PM_SRCS
	../pm_shared/pm_shared.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
    )

set(GAME_SHARED_SRCS
    ../game_shared/voice_gamemgr.cpp
	../game_shared/shared_util.cpp
	../game_shared/bot/bot.cpp
	../game_shared/bot/bot_manager.cpp
	../game_shared/bot/bot_profile.cpp
	../game_shared/bot/bot_util.cpp
	../game_shared/bot/nav_area.cpp
	../game_shared/bot/nav_file.cpp
	../game_shared/bot/nav_node.cpp
	../game_shared/bot/nav_path.cpp
    )

set(LUASV_SRCS
	luash_sv/luash_sv.cpp
	luash_sv/luash_sv_filesystem.cpp
	luash_sv/luash_sv_engfunc.cpp
	luash_sv/luash_sv_dllfunc.cpp
	luash_sv/luash_sv_glb.cpp
	luash_sv/luash_sv_luaclass.cpp
	luash_sv/luash_sv_cppclass.cpp
	luash_sv/luash_sv_entity.cpp
	luash_sv/luash_sv_util.cpp
	luash_sv/luash_sv_extern.cpp
)

set_source_files_properties(
    ${LUASV_SRCS} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
)

set(PUBLIC_SRCS
	../public/MemPool.cpp
    )

set (SERVER_SOURCES
	${HLWPN_SRCS}
	${HLDLL_SRCS}
	${PM_SRCS}
	${GAME_SHARED_SRCS}
	${PUBLIC_SRCS}
	${LUASV_SRCS}
)

include_directories (
	../
	../common/
	../engine/
	../pm_shared/
	../public/
	../SourceSDK/public/
	../dlls/
	../game_shared/)

if(XASH_STATIC_GAMELIB)
	add_library (${SERVER_LIBRARY} STATIC ${SERVER_SOURCES})
else()
	add_library (${SERVER_LIBRARY} SHARED ${SERVER_SOURCES})
endif()

if(APPLE OR ANDROID)
	add_definitions(
		-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf
		-D_alloca=alloca -D_strdup=strdup -D_write=write -DNO_MALLOC_OVERRIDE
		-D_close=close -D_write=write
	)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(
		-fpermissive -fno-strict-aliasing -fexpensive-optimizations
		-DLINUX -D_LINUX -DGNUC
		-Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf
		-D_alloca=alloca -D_strdup=strdup -D_write=write -DNO_MALLOC_OVERRIDE
		-D_close=close -D_write=write
	)
endif()

if(MINGW)
add_definitions(
	-fpermissive -fno-strict-aliasing -fexpensive-optimizations
	-D_EMULATE_GLIBC -DGNUC
	-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
	-D_alloca=alloca -D_vsnprintf=vsnprintf -DNO_MALLOC_OVERRIDE
		)
endif()

if(MSVC)
add_definitions(
	-DWIN32 -DDEDICATED -D_USING_V110_SDK71_ -D_WINDLL -D_MBCS
	-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE
)
endif()

add_definitions(
	-DCLIENT_WEAPONS
	-DQUIVER -DQUAKE2 -DVALVE_DLL -DSERVER_DLL
	)

# fix tier1/strtools
add_definitions(
	-DVSTDLIB_BACKWARD_COMPAT
)

target_link_libraries( ${SERVER_LIBRARY} ${CMAKE_DL_LIBS} )
target_link_libraries( ${SERVER_LIBRARY} luash cqmiao )
target_link_libraries( ${SERVER_LIBRARY} platform_config tier1)

if(ANDROID OR IOS)
	set_target_properties(${SERVER_LIBRARY} PROPERTIES
			OUTPUT_NAME ${SERVER_LIBRARY_NAME})
else()
	set_target_properties(${SERVER_LIBRARY} PROPERTIES
			PREFIX "" OUTPUT_NAME ${SERVER_LIBRARY_NAME})
endif()

if(NOT XASH_STATIC_GAMELIB)
	install( TARGETS ${SERVER_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GAMEDIR}/${SERVER_INSTALL_DIR})
endif()

if(XASH_PCH)
	target_precompile_headers(${SERVER_LIBRARY} PRIVATE stdafx.h)
endif()

if(XASH_UNITY_BUILD)
	set_target_properties(${SERVER_LIBRARY} PROPERTIES UNITY_BUILD ON)
endif()

if(XASH_DEDICATED)
	option(XASH_MYSQL "MYSQL support for CSMoE." ON)
else()
	option(XASH_MYSQL "MYSQL support for CSMoE." OFF)
endif()

if(XASH_MYSQL)
	add_library (mysql SHARED IMPORTED)
	set_property(TARGET mysql PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/3rdparty/mysql/lib/libmysql.dll)
	set_property(TARGET mysql PROPERTY IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/3rdparty/mysql/lib/libmysql.lib)
	include_directories(${CMAKE_SOURCE_DIR}/3rdparty/mysql/include)
	target_link_libraries(${SERVER_LIBRARY} mysql)
	install(FILES ${CMAKE_SOURCE_DIR}/3rdparty/mysql/lib/libmysql.dll DESTINATION ".")
	add_definitions(-DXASH_MYSQL=1)
endif()