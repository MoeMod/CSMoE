//
// Created by 小白白 on 2023/11/11.
//

#include "common.h"
#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>

CMMotionManager *MotionManager = nullptr;

float CenterPitch, CenterRoll;
float LastPitch, LastRoll;
bool bIsCalibrationRequested;
CMAttitude* ReferenceAttitude = nullptr;
vec3_t FilteredAccelerometer;

// from UE4 FIOSInputInterface::GetMovementData
void IOSInputInterface_GetMovementData(vec3_t& Attitude, vec3_t& RotationRate, vec3_t& Gravity, vec3_t& Acceleration)
{
    // initialize on first use
    if (MotionManager == nil)
    {
        // Look to see if we can create the motion manager
        MotionManager = [[CMMotionManager alloc] init];

        // Check to see if the device supports full motion (gyro + accelerometer)
        if (MotionManager.deviceMotionAvailable)
        {
            MotionManager.deviceMotionUpdateInterval = 0.02;

            // Start the Device updating motion
            [MotionManager startDeviceMotionUpdates];
        }
        else
        {
            [MotionManager startAccelerometerUpdates];
            CenterPitch = CenterPitch = 0;
            bIsCalibrationRequested = false;
        }
    }

    // do we have full motion data?
    if (MotionManager.deviceMotionActive)
    {
        // Grab the values
        // TODO : alloc detected here. dont copy or retain
        __unsafe_unretained CMAttitude* CurrentAttitude = MotionManager.deviceMotion.attitude;
        CMRotationRate CurrentRotationRate = MotionManager.deviceMotion.rotationRate;
        CMAcceleration CurrentGravity = MotionManager.deviceMotion.gravity;
        CMAcceleration CurrentUserAcceleration = MotionManager.deviceMotion.userAcceleration;

        // apply a reference attitude if we have been calibrated away from default
        if (ReferenceAttitude)
        {
            [CurrentAttitude multiplyByInverseOfAttitude : ReferenceAttitude];
        }

        // convert to UE3
        Attitude = { float(CurrentAttitude.pitch), float(CurrentAttitude.yaw), float(CurrentAttitude.roll) };
        RotationRate = { float(CurrentRotationRate.x), float(CurrentRotationRate.y), float(CurrentRotationRate.z) };
        Gravity = { float(CurrentGravity.x), float(CurrentGravity.y), float(CurrentGravity.z) };
        Acceleration = { float(CurrentUserAcceleration.x), float(CurrentUserAcceleration.y), float(CurrentUserAcceleration.z) };
    }
    else
    {
        // get the plain accleration
        CMAcceleration RawAcceleration = [MotionManager accelerometerData].acceleration;
        vec3_t NewAcceleration(RawAcceleration.x, RawAcceleration.y, RawAcceleration.z);

        // storage for keeping the accelerometer values over time (for filtering)
        static bool bFirstAccel = true;

        // how much of the previous frame's acceleration to keep
        const float VectorFilter = bFirstAccel ? 0.0f : 0.85f;
        bFirstAccel = false;

        // apply new accelerometer values to last frames
        FilteredAccelerometer = FilteredAccelerometer * VectorFilter + (1.0f - VectorFilter) * NewAcceleration;

        // create an normalized acceleration vector
        auto FinalAcceleration = -FilteredAccelerometer.Normalize();

        // calculate Roll/Pitch
        float CurrentPitch = atan2(FinalAcceleration.y, FinalAcceleration.z);
        float CurrentRoll = -atan2(FinalAcceleration.x, FinalAcceleration.z);

        // if we want to calibrate, use the current values as center
        if (bIsCalibrationRequested)
        {
            CenterPitch = CurrentPitch;
            CenterRoll = CurrentRoll;
            bIsCalibrationRequested = false;
        }

        CurrentPitch -= CenterPitch;
        CurrentRoll -= CenterRoll;

        Attitude = { CurrentPitch, 0, CurrentRoll };
        RotationRate = { LastPitch - CurrentPitch, 0, LastRoll - CurrentRoll };
        Gravity = { 0, 0, 0 };

        // use the raw acceleration for acceleration
        Acceleration = NewAcceleration;

        // remember for next time (for rotation rate)
        LastPitch = CurrentPitch;
        LastRoll = CurrentRoll;
    }
}

void IOSInputInterface_ModifyVectorByOrientation(vec3_t& Vec, bool bIsRotation)
{
    UIInterfaceOrientation Orientation = [[UIApplication sharedApplication] statusBarOrientation];

    switch (Orientation)
    {
        case UIInterfaceOrientationPortrait:
            // this is the base orientation, so nothing to do
            break;

        case UIInterfaceOrientationPortraitUpsideDown:
            if (bIsRotation)
            {
                // negate roll and pitch
                Vec.x = -Vec.x;
                Vec.z = -Vec.z;
            }
            else
            {
                // negate x/y
                Vec.x = -Vec.x;
                Vec.y = -Vec.y;
            }
            break;

        case UIInterfaceOrientationLandscapeRight:
            if (bIsRotation)
            {
                // swap and negate (as needed) roll and pitch
                float Temp = Vec.x;
                Vec.x = -Vec.z;
                Vec.z = Temp;
                Vec.y *= -1.0f;
            }
            else
            {
                // swap and negate (as needed) x and y
                float Temp = Vec.x;
                Vec.x = -Vec.y;
                Vec.y = Temp;
            }
            break;

        case UIInterfaceOrientationLandscapeLeft:
            if (bIsRotation)
            {
                // swap and negate (as needed) roll and pitch
                float Temp = Vec.x;
                Vec.x = -Vec.z;
                Vec.z = -Temp;
            }
            else
            {
                // swap and negate (as needed) x and y
                float Temp = Vec.x;
                Vec.x = Vec.y;
                Vec.y = -Temp;
            }
            break;
    }
}

void IOSInputInterface_GetRotatedMovementData(vec3_t& Attitude, vec3_t& RotationRate, vec3_t& Gravity, vec3_t& Acceleration)
{
    // Update motion controls.
    IOSInputInterface_GetMovementData(Attitude, RotationRate, Gravity, Acceleration);

    // Fix-up yaw to match directions
    Attitude.y = -Attitude.y;
    RotationRate.y = -RotationRate.y;

    // munge the vectors based on the orientation
    IOSInputInterface_ModifyVectorByOrientation(Attitude, true);
    IOSInputInterface_ModifyVectorByOrientation(RotationRate, true);
    IOSInputInterface_ModifyVectorByOrientation(Gravity, false);
    IOSInputInterface_ModifyVectorByOrientation(Acceleration, false);
}
