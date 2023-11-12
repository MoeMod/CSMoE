
#include "common.h"
#include "input.h"

#if ( TARGET_OS_IOS || TARGET_OS_IPHONE )
#include "platform/iOS/IOSInput.h"
#endif

#if defined __ANDROID__
#include "platform/android/android-main.h"
#endif

convar_t *gyroscope_pitch;
convar_t *gyroscope_yaw;

vec3_t IN_GetRotationState()
{
#if ( TARGET_OS_IOS || TARGET_OS_IPHONE )
    vec3_t Attitude, RotationRate, Gravity, Acceleration;
    IOSInputInterface_GetRotatedMovementData(Attitude, RotationRate, Gravity, Acceleration);
    return { RotationRate.y, RotationRate.z, RotationRate.x };
#endif
#if defined __ANDROID__
    vec3_t Attitude, RotationRate, Gravity, Acceleration;
    Android_GetMotionData(Attitude, RotationRate, Gravity, Acceleration);
    switch(Android_GetDeviceOrientation())
    {
        case 0:
            return {}; // PORTRAIT
        case 1:
            return { RotationRate.y, RotationRate.x, RotationRate.z }; // LANDSCAPE
        case 2:
            return {}; // PORTRAIT_FLIPPED
        case 3:
            return { -RotationRate.y, -RotationRate.x, -RotationRate.z }; // LANDSCAPE_FLIPPED
    }
    return {};
#endif
    return {};
}

void IN_GyroscopeInit()
{
    gyroscope_pitch = Cvar_Get( "gyroscope_pitch", "0.2", CVAR_ARCHIVE, "gyroscope pitch sensitivity" );
    gyroscope_yaw = Cvar_Get( "gyroscope_yaw", "0.25", CVAR_ARCHIVE, "gyroscope yaw sensitivity" );
}

void IN_GyroscopeAppendMove( float *yaw, float *pitch )
{
    vec3_t RotationState = IN_GetRotationState();
    *pitch += RotationState.x * gyroscope_pitch->value;
    *yaw += RotationState.y * gyroscope_yaw->value;
}