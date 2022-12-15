#
# Copyright (c) 2015 Pavlo Lavrenenko
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
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.10.2)

project (client)

set(CMAKE_CXX_STANDARD 20)

if (APPLE OR ANDROID)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -stdlib=libc++")
elseif(MSVC)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
else()
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -static-libstdc++")
endif()
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ")

#find_package(SDL2 REQUIRED)

set (CLIENT_LIBRARY client)

#--------------
# USER DEFINES \
################\
set(GAMEDIR "csmoe" CACHE STRING "Gamedir path")
set(CLIENT_INSTALL_DIR "cl_dlls" CACHE STRING "Where put client dll")
set(CLIENT_LIBRARY_NAME "client" CACHE STRING "Library name for client")

#-----------------
# MAIN BUILD CODE \
###################\

file (GLOB WEAPONEVENT_SRCS
	./ev_common.cpp
	./events/ev_cs16.cpp
	./events/hl_events.cpp
	./events/event_*.cpp
)

set (CLWEAPON_SRCS
	./com_weapons.cpp
	./cs_wpn/cs_baseentity.cpp
	./cs_wpn/cs_weapons.cpp
	./cs_wpn/bte_weapons.cpp
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
	../dlls/wpn_shared/knife_dragonsword.cpp
	../dlls/wpn_shared/knife_dgaxe.cpp
	../dlls/wpn_shared/wpn_gungnir.cpp
	../dlls/wpn_shared/wpn_mg3.cpp
	../dlls/wpn_shared/wpn_mp7a1c.cpp
	../dlls/wpn_shared/wpn_mp7a1p.cpp
	../dlls/wpn_shared/wpn_infinitys.cpp
	../dlls/wpn_shared/wpn_m14ebr.cpp
	../dlls/wpn_shared/wpn_balrog7.cpp
	../dlls/wpn_shared/wpn_gatling.cpp
	../dlls/wpn_shared/wpn_m2.cpp
	../dlls/wpn_shared/wpn_bow.cpp
	../dlls/wpn_shared/wpn_huntbow.cpp
	../dlls/wpn_shared/wpn_desperado.cpp
	../dlls/wpn_shared/wpn_m134.cpp
	../dlls/wpn_shared/wpn_m134ex.cpp
	../dlls/wpn_shared/wpn_m134xmas.cpp
	../dlls/wpn_shared/wpn_mg3xmas.cpp
	../dlls/wpn_shared/wpn_sgdrill.cpp
	../dlls/wpn_shared/wpn_m95xmas.cpp
	../dlls/wpn_shared/wpn_m4a1dragon.cpp
	../dlls/wpn_shared/wpn_ak47dragon.cpp
	../dlls/wpn_shared/wpn_knifedragon.cpp
	../dlls/wpn_shared/wpn_molotov.cpp
	../dlls/wpn_shared/wpn_janus7xmas.cpp
	../dlls/wpn_shared/wpn_chainsaw.cpp
	../dlls/wpn_shared/wpn_tbarrel.cpp
	../dlls/wpn_shared/wpn_starchaserar.cpp
	../dlls/wpn_shared/wpn_svdex.cpp
	../dlls/wpn_shared/knife_katana.cpp
	../dlls/wpn_shared/wpn_infinityex1.cpp
	../dlls/wpn_shared/wpn_infinityex2.cpp
	../dlls/wpn_shared/z4b_infinityx.cpp
	../dlls/wpn_shared/knife_dualsword.cpp
	../dlls/wpn_shared/wpn_anaconda.cpp
	../dlls/wpn_shared/wpn_mg36.cpp
	../dlls/wpn_shared/wpn_gunkata.cpp
	../dlls/wpn_shared/z4b_m37se.cpp
	../dlls/wpn_shared/z4b_Frequency9.cpp
	../dlls/wpn_shared/z4b_stormgiantx.cpp
	../dlls/wpn_shared/z4b_lc_sword.cpp
	../dlls/wpn_shared/z4b_oppovivo.cpp
	../dlls/wpn_shared/z4b_strongknifex.cpp
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

set (HUD_SRCS
	./hud/ammo.cpp
	./hud/ammo_secondary.cpp
	./hud/ammohistory.cpp
	./hud/battery.cpp
	./hud/geiger.cpp
	./hud/health.cpp
	./hud/hud.cpp
	./hud/hud_msg.cpp
	./hud/hud_redraw.cpp
	./hud/hud_spectator.cpp
	./hud/hud_update.cpp
	./hud/sniperscope.cpp
	./hud/nvg.cpp
	./hud/hud_menu.cpp
	./hud/message.cpp
	./hud/status_icons.cpp
	./hud/statusbar.cpp
	./hud/text_message.cpp
	./hud/train.cpp
	./hud/scoreboard.cpp
	./hud/MOTD.cpp
	./hud/radio.cpp
	./hud/radar.cpp
	./hud/timer.cpp
	./hud/money.cpp
	./hud/flashlight.cpp
	./hud/death.cpp
	./hud/showwin.cpp
	./hud/hud_headicon.cpp
	./hud/specialcrosshair.cpp
	./hud/hit_indicator.cpp
	./hud/saytext.cpp
	./hud/spectator_gui.cpp
	./hud/followicon.cpp
	./hud/headname.cpp
	./hud/zbs/zbs.cpp
	./hud/zbs/zbs_level.cpp
	./hud/zbs/zbs_scoreboard.cpp
	./hud/zbs/zbs_kill.cpp
	./hud/zbs/zbs_roundclear.cpp
	./hud/retina.cpp
	./hud/scenariostatus.cpp
	./hud/zb2/zb2.cpp
	./hud/zb2/zb2_skill.cpp
	./hud/legacy/hud_radar_legacy.cpp
	./hud/legacy/hud_scoreboard_legacy.cpp
	./hud/modern/hud_radar_modern.cpp
	./hud/zb3/zb3.cpp
	./hud/zb3/zb3_morale.cpp
	./hud/zb3/zb3_rage.cpp
	./hud/moe/moe_touch.cpp
	./hud/z4/z4.cpp
	./hud/z4/z4_energy.cpp
	./hud/z4/z4_scoreboard.cpp
	./hud/z4/z4_tips.cpp
	./hud/z4/z4_status.cpp
	./hud/zbz/zbz.cpp
	./hud/zbz/zbz_skill_info.cpp
	./hud/hud_sifiammo.cpp
	./hud/hud_mvp.cpp
	./hud/server.cpp
	./hud/newhud/newhud.cpp
	./hud/hud_centertips.cpp
	./hud/zbz/zbz_ghosthunter.cpp
)

set (STUDIORENDER_SRCS
	./studio/GameStudioModelRenderer.cpp
	./studio/StudioModelRenderer.cpp
	./studio/studio_util.cpp

)

set(PM_SRCS
	../pm_shared/pm_shared.cpp
	../pm_shared/pm_debug.cpp
	../pm_shared/pm_math.cpp
)

if(XASH_IMGUI)
	set(IMGUI_SRCS
			imgui_cl/imgui_cl_main.cpp
			imgui_cl/imgui_cl_buy.cpp
			imgui_cl/imgui_cl_menu.cpp
	)
	add_definitions(-DXASH_IMGUI)
else()
	set(IMGUI_SRCS )
endif()

if(XASH_LUASH)
	set(LUASH_SRCS
		luash_cl/lua_cl.cpp
		luash_cl/ev_lua.cpp
		luash_cl/luash_cl_engfunc.cpp
		luash_cl/luash_cl_filesystem.cpp
		luash_cl/luash_cl_mobile.cpp
		luash_cl/luash_cl_render.cpp
		../dlls/luash_sv/luash_sv_cppclass.cpp
		../dlls/luash_sv/luash_sv_luaclass.cpp
		../dlls/luash_sv/luash_sv_engfunc.cpp
		../dlls/luash_sv/luash_sv_glb.cpp
		../dlls/luash_sv/luash_sv_entity.cpp
		../dlls/luash_sv/luash_sv_util.cpp
		../dlls/luash_sv/luash_sv_extern.cpp
		luash_cl/luash_cl_extern.cpp
	)
else()
	set(LUASH_SRCS )
endif()

if(XASH_VGUI2)
	set(VGUI2_SRCS
		vgui2/CBackGroundPanel.cpp
		vgui2/CBaseViewport.cpp
		vgui2/CClientMOTD.cpp
		vgui2/CClientVGUI.cpp
		vgui2/CHudViewPort.cpp
		vgui2/CGameUITestPanel.cpp
		vgui2/game_controls/buymenu.cpp
		vgui2/game_controls/buysubmenu.cpp
		vgui2/game_controls/teammenu.cpp
		vgui2/game_controls/classmenu.cpp
		vgui2/csmoe/CSBackGroundPanel.cpp
		vgui2/csmoe/vgui_int.cpp
		vgui2/csmoe/cstriketeammenu.cpp
		vgui2/csmoe/cstrikeclassmenu.cpp
		vgui2/csmoe/BuyMenu/buypresetbutton.cpp
		vgui2/csmoe/BuyMenu/cstrikebuymenu.cpp
		vgui2/csmoe/BuyMenu/cstrikebuymouseoverpanel.cpp
		vgui2/csmoe/BuyMenu/cstrikebuymouseoverpanelbutton.cpp
		vgui2/csmoe/BuyMenu/cstrikebuysubmenu.cpp
		vgui2/csmoe/BuyMenu/WeaponImagePanel.cpp
		vgui2/csmoe/zsh/zshelterteamhousingdlg.cpp
		vgui2/csmoe/zsh/zshelterteamhousingsubdlg.cpp
	)
	add_definitions(-DXASH_VGUI2=1)
else()
	set(VGUI2_SRCS )
endif()

set_source_files_properties(
    ${LUASH_SRCS} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
)

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
	set(INPUT_SRCS ./input_winrt.cpp)
else()
	set(INPUT_SRCS ./input_xash3d.cpp)
endif()

set(GAME_SHARED_SRCS
	../game_shared/configs/cfg_zsht.cpp
    )

set (CLDLL_SOURCES
	./cdll_int.cpp
    ./demo.cpp
    ./entity.cpp
    ./in_camera.cpp
    ./input.cpp
	./rain.cpp
    ./tri.cpp
	./util.cpp
	./view.cpp
	./draw_util.cpp
	./calcscreen.cpp
	./input_touch.cpp
	./fog.cpp


	${LUASH_SRCS}
	${INPUT_SRCS}
	${PM_SRCS}
	${HUD_SRCS}
	${WEAPONEVENT_SRCS}
	${CLWEAPON_SRCS}
	${HLWPN_SRCS}
	${STUDIORENDER_SRCS}
	${IMGUI_SRCS}
	${VGUI2_SRCS}
	${GAME_SHARED_SRCS}
)

include_directories (
	hud/
	studio/
	../cl_dll/
	../common/
	../engine/
	../pm_shared/
	../dlls/
	../game_shared/
	../public/
	../SourceSDK/public/
	)

if(XASH_STATIC_GAMELIB)
	add_library (${CLIENT_LIBRARY} STATIC ${CLDLL_SOURCES} "events/eventdef.h")
else()
	add_library (${CLIENT_LIBRARY} SHARED ${CLDLL_SOURCES})
endif()

if(APPLE OR ANDROID)
	add_definitions(
			-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
	)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(
		-DLINUX -D_LINUX
		-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
		)
endif()

if(MINGW)
add_definitions(
	-D_EMULATE_GLIBC
	-Dstricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp
		)
endif()

if(MSVC)
add_definitions(
	-DWIN32 -DDEDICATED -D_USING_V110_SDK71_ -D_WINDLL -D_MBCS
	-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE
)
endif()

add_definitions( -D_CS16CLIENT_ENABLE_GSRC_SUPPORT
	-DCLIENT_WEAPONS -DCLIENT_DLL
	-D_CS16CLIENT_ALLOW_SPECIAL_SCRIPTING
	 )

if(MSVC)
	add_compile_options(/fp:fast)
else()
	add_compile_options(-ffast-math)
endif()

target_link_libraries( ${CLIENT_LIBRARY} ${CMAKE_DL_LIBS} )

if(XASH_IMGUI)
	target_link_libraries( ${CLIENT_LIBRARY} imgui imgui_ext )
endif()

if(XASH_LUASH)
	target_link_libraries( ${CLIENT_LIBRARY} luash)
endif()

if(XASH_VGUI2)
	target_link_libraries( ${CLIENT_LIBRARY} vgui_controls)
	target_include_directories(${CLIENT_LIBRARY} PRIVATE vgui2/)
endif()

target_link_libraries( ${CLIENT_LIBRARY} platform_config )
target_link_libraries( ${CLIENT_LIBRARY} json_3p )

if(ANDROID OR IOS)
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			OUTPUT_NAME ${CLIENT_LIBRARY_NAME})
else()
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			PREFIX "" OUTPUT_NAME ${CLIENT_LIBRARY_NAME})
endif()

if(XASH_64BIT AND NOT ANDROID AND NOT IOS)
	get_target_property(CLIENT_LIBRARY_NAME ${CLIENT_LIBRARY} OUTPUT_NAME)
	if(NOT CLIENT_LIBRARY_NAME)
		set(CLIENT_LIBRARY_NAME ${CLIENT_LIBRARY})
	endif()
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES
			OUTPUT_NAME ${CLIENT_LIBRARY_NAME}64)
endif()

if(NOT XASH_STATIC_GAMELIB)
	install( TARGETS ${CLIENT_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GAMEDIR}/${CLIENT_INSTALL_DIR} )
endif()

if(XASH_PCH)
	target_precompile_headers(${CLIENT_LIBRARY} PRIVATE stdafx.h)
endif()

if(XASH_UNITY_BUILD)
	set_target_properties(${CLIENT_LIBRARY} PROPERTIES UNITY_BUILD ON)
endif()
