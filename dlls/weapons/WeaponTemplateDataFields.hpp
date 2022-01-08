/*
WeaponTemplateDataFields.hpp
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

namespace df {

DF_GENERATE_TEMPLATE_GETTER(AccuracyDefault) // TDeployDoubleMode TDeployDefault TGeneralData TReloadDefault
DF_GENERATE_TEMPLATE_GETTER(AccuracyMin) // TCheckAccuracyBoundary
DF_GENERATE_TEMPLATE_GETTER(AccuracyMax) // TCheckAccuracyBoundary
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(DoubleMode_SyncAmmo, false) // TDeployDoubleMode
DF_GENERATE_TEMPLATE_GETTER(DoubleMode_LinkedWeaponClassName) // TDeployDoubleMode
DF_GENERATE_TEMPLATE_GETTER(DoubleMode_ChangeTime) // TDeployDoubleMode
DF_GENERATE_TEMPLATE_GETTER(MaxClip) // TDoubleModeType TGeneralData TGetItemInfoDefault TReloadDefault
DF_GENERATE_TEMPLATE_GETTER(V_Model) // TDeployDoubleMode TDeployDefault TGeneralData
DF_GENERATE_TEMPLATE_GETTER(P_Model) // TDeployDoubleMode TDeployDefault TGeneralData
DF_GENERATE_TEMPLATE_GETTER(W_Model) // TGeneralData
DF_GENERATE_TEMPLATE_GETTER(AnimExtension) // TDeployDoubleMode TDeployDefault
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(AnimExtensionFemale, "\0") // TDeployDoubleMode TDeployDefault
DF_GENERATE_TEMPLATE_GETTER(ANIM_IDLE1) // TWeaponIdleDefault
DF_GENERATE_TEMPLATE_GETTER(ANIM_CHANGE) // TDeployDoubleMode
DF_GENERATE_TEMPLATE_GETTER(ANIM_DRAW) // TDeployDoubleMode TDeployDefault
DF_GENERATE_TEMPLATE_GETTER(ANIM_RELOAD) // TReloadDefault
DF_GENERATE_TEMPLATE_GETTER(DefaultReloadTime) // TReloadDefault
DF_GENERATE_TEMPLATE_GETTER(DefaultDeployTime) // TDeployDoubleMode TDeployDefault
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(WeaponIdleTime, 20s) // TDeployDoubleMode TDeployDefault
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(Distance, 8192) // TFirePistol
DF_GENERATE_TEMPLATE_GETTER(Penetration) // TFirePistol TFireRifle
DF_GENERATE_TEMPLATE_GETTER(DamageDefault) // TGetDamageDefault TFirePistol TFireRifle
DF_GENERATE_TEMPLATE_GETTER(DamageZB) // TGetDamageDefault
DF_GENERATE_TEMPLATE_GETTER(DamageZBS) // TGetDamageDefault
DF_GENERATE_TEMPLATE_GETTER(RangeModifier) // TFirePistol TFireRifle
DF_GENERATE_TEMPLATE_GETTER(BulletType) // TFirePistol TFireRifle
DF_GENERATE_TEMPLATE_GETTER(ItemSlot) // TFirePistol TFireRifle TGeneralData
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(PlayerMaxSpeed, 250) // TGeneralData
DF_GENERATE_TEMPLATE_GETTER(KnockBack) // TGeneralData
DF_GENERATE_TEMPLATE_GETTER(ClassName) // TGeneralData TGetItemInfoDefault
DF_GENERATE_TEMPLATE_GETTER(WeaponId) // TGeneralData TGetItemInfoDefault
DF_GENERATE_TEMPLATE_GETTER(ItemInfoData) // TGetItemInfoDefault
DF_GENERATE_TEMPLATE_GETTER(EventFile) // TPrecacheEvent
DF_GENERATE_TEMPLATE_GETTER(CycleTime) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER(CycleTimeZoomed) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(PrimaryAttackWalkingMiniumSpeed, 140) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER(KickBackWalking) // TRecoilKickBack
DF_GENERATE_TEMPLATE_GETTER(KickBackNotOnGround) // TRecoilKickBack
DF_GENERATE_TEMPLATE_GETTER(KickBackDucking) // TRecoilKickBack
DF_GENERATE_TEMPLATE_GETTER(KickBackDefault) // TRecoilKickBack
DF_GENERATE_TEMPLATE_GETTER(RecoilPunchAngleDelta) // TRecoilPunch
DF_GENERATE_TEMPLATE_GETTER(ZoomFOV) // TSecondaryAttackSniperZoom1 TSecondaryAttackZoom
DF_GENERATE_TEMPLATE_GETTER(ZoomFOV1) // TSecondaryAttackSniperZoom2
DF_GENERATE_TEMPLATE_GETTER(ZoomFOV2) // TSecondaryAttackSniperZoom2
DF_GENERATE_TEMPLATE_GETTER(AccuracyCalc) // TFireRifle
DF_GENERATE_TEMPLATE_GETTER(SpreadCalcDefault) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER(SpreadCalcNotOnGround) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER(SpreadCalcWalking) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER(SpreadCalcDucking) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER(SpreadCalcZoomed) // TPrimaryAttackRifle
DF_GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(PlayerAttackAnimation, PLAYER_ATTACK1) // TFirePistol
}
