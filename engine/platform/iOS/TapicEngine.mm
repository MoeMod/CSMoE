/*
TapicEngine.mm - vibrate support for iOS
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#import <AudioToolbox/AudioToolbox.h>

extern "C"{

#include "TapicEngine.h"

void TapicEngine_Vibrate(TapicEngineSoundID id)
{
	AudioServicesPlaySystemSound(static_cast<SystemSoundID>(id));
}

}
