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

GENERATE_TEMPLATE_GETTER(AccuracyDefault) // TDeployDoubleMode TDeployDefault TGeneralData TReloadDefault
GENERATE_TEMPLATE_GETTER(AccuracyMin) // TCheckAccuracyBoundary
GENERATE_TEMPLATE_GETTER(AccuracyMax) // TCheckAccuracyBoundary
GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(DoubleMode_SyncAmmo, false) // TDeployDoubleMode
GENERATE_TEMPLATE_GETTER(DoubleMode_LinkedWeaponClassName) // TDeployDoubleMode
GENERATE_TEMPLATE_GETTER(DoubleMode_ChangeTime) // TDeployDoubleMode
GENERATE_TEMPLATE_GETTER(MaxClip) // TDoubleModeType TGeneralData TGetItemInfoDefault TReloadDefault
GENERATE_TEMPLATE_GETTER(V_Model) // TDeployDoubleMode TDeployDefault TGeneralData
GENERATE_TEMPLATE_GETTER(P_Model) // TDeployDoubleMode TDeployDefault TGeneralData
GENERATE_TEMPLATE_GETTER(W_Model) // TGeneralData
GENERATE_TEMPLATE_GETTER(AnimExtension) // TDeployDoubleMode TDeployDefault
GENERATE_TEMPLATE_GETTER(ANIM_IDLE1) // TWeaponIdleDefault
GENERATE_TEMPLATE_GETTER(ANIM_CHANGE) // TDeployDoubleMode
GENERATE_TEMPLATE_GETTER(ANIM_DRAW) // TDeployDoubleMode TDeployDefault
GENERATE_TEMPLATE_GETTER(ANIM_RELOAD) // TReloadDefault
GENERATE_TEMPLATE_GETTER(DefaultReloadTime) // TReloadDefault
GENERATE_TEMPLATE_GETTER(DefaultDeployTime) // TDeployDoubleMode TDeployDefault
GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(WeaponIdleTime, 20s) // TDeployDoubleMode TDeployDefault
GENERATE_TEMPLATE_GETTER_WITH_DEFAULT(Distance, 8192) // TFirePistol
GENERATE_TEMPLATE_GETTER(Penetration) // TFirePistol TFireRifle
GENERATE_TEMPLATE_GETTER(Damage) // TFirePistol TFireRifle
GENERATE_TEMPLATE_GETTER(DamageDefault) // TFirePistol TFireRifle
GENERATE_TEMPLATE_GETTER(DamageZB) // TGetDamageDefault
GENERATE_TEMPLATE_GETTER(DamageZBS) // TGetDamageDefault
GENERATE_TEMPLATE_GETTER(RangeModifier) // TFirePistol TFireRifle
GENERATE_TEMPLATE_GETTER(BulletType) // TFirePistol TFireRifle
GENERATE_TEMPLATE_GETTER(ItemSlot) // TFirePistol TFireRifle TGeneralData
GENERATE_TEMPLATE_GETTER(MaxSpeed) // TGeneralData
GENERATE_TEMPLATE_GETTER(KnockBack) // TGeneralData
GENERATE_TEMPLATE_GETTER(ClassName) // TGeneralData TGetItemInfoDefault
GENERATE_TEMPLATE_GETTER(WeaponId) // TGeneralData TGetItemInfoDefault
GENERATE_TEMPLATE_GETTER(ItemInfoData) // TGetItemInfoDefault
GENERATE_TEMPLATE_GETTER(EventFile) // TPrecacheEvent
GENERATE_TEMPLATE_GETTER(CycleTime) // TPrimaryAttackRifle
GENERATE_TEMPLATE_GETTER(CycleTimeZoomed) // TPrimaryAttackRifle
GENERATE_TEMPLATE_GETTER(PrimaryAttackWalkingMiniumSpeed) // TPrimaryAttackRifle
GENERATE_TEMPLATE_GETTER(KickBackWalking) // TRecoilKickBack
GENERATE_TEMPLATE_GETTER(KickBackNotOnGround) // TRecoilKickBack
GENERATE_TEMPLATE_GETTER(KickBackDucking) // TRecoilKickBack
GENERATE_TEMPLATE_GETTER(KickBackDefault) // TRecoilKickBack
GENERATE_TEMPLATE_GETTER(RecoilPunchAngleDelta) // TRecoilPunch
GENERATE_TEMPLATE_GETTER(ZoomFOV) // TSecondaryAttackSniperZoom1 TSecondaryAttackZoom
GENERATE_TEMPLATE_GETTER(ZoomFOV1) // TSecondaryAttackSniperZoom2
GENERATE_TEMPLATE_GETTER(ZoomFOV2) // TSecondaryAttackSniperZoom2
}
