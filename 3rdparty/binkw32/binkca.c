// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

//-----------------------------------------------------------------------------
//
// NOTE TO ANYONE DISTIRBUTING TO OSX:
//
// Be sure your examples add this prior to initialization.
//
//
// //
// // This is a critical bit of code! Prior to OSX 10.6 (I believe), the Audio Object's
// // runloop ran on its own thread. After that, it was attached to the app's main thread,
// // and if you ever starved it, you just lost notifications. This code causes the 
// // run loop to detach and run its own thread.
// //
// CFRunLoopRef Loop = NULL;
// AudioObjectPropertyAddress PA =
// {
//     kAudioHardwarePropertyRunLoop,
//     kAudioObjectPropertyScopeGlobal,
//     kAudioObjectPropertyElementMaster
// };
// AudioObjectSetPropertyData( kAudioObjectSystemObject, &PA, 0, NULL, sizeof(CFRunLoopRef), &Loop);
//
//-----------------------------------------------------------------------------
#include "bink.h"
#include "rrAtomics.h"

#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioToolbox.h>


static const int buffer_count = 4;

// must result in 16 bytes alignment. this was originally at 256 but despite everything
// succeeding no audio actually came out, so it's at 512 to be closer to the original
// radss_coreaudio size.
static const int samples_per_buffer = 512;

static AudioQueueRef audio_queue;
static AudioQueueBufferRef buffers[buffer_count];
static int buffer_size_bytes;
static unsigned int too_late;
static unsigned int buffer_index;
static unsigned int volatile queued_count;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static void CoreAudioCallback(void* inUser, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    // this just does an atomic decrement that never goes negative
    U32 queued = rrAtomicLoadAcquire32(&queued_count);
    for(;;)
    {
      U32 newq = ( queued == 0 ) ? 0 : ( queued - 1 );
      if ( rrAtomicCAS32( &queued_count, &queued, newq ) )
        break;
    }
}
static void bink_coreaudio_close();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static int bink_coreaudio_open(int freq, int chans, int * fragsize)
{
    queued_count = 0;
    buffer_index = 0;

    OSStatus Result = 0;

    AudioStreamBasicDescription DeviceDesc;
    DeviceDesc.mSampleRate = (F64)freq;
    DeviceDesc.mFormatID = kAudioFormatLinearPCM;
    DeviceDesc.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    DeviceDesc.mBytesPerPacket = chans * 2;
    DeviceDesc.mFramesPerPacket = 1;
    DeviceDesc.mBytesPerFrame = chans * 2;
    DeviceDesc.mChannelsPerFrame = chans;
    DeviceDesc.mBitsPerChannel = 16;

    Result = AudioQueueNewOutput(&DeviceDesc, CoreAudioCallback, 0, 0, 0, 0, &audio_queue);
    if (Result != 0)
    {
        bink_coreaudio_close();
        return 0;
    }

    buffer_size_bytes = samples_per_buffer * sizeof(short) * chans;

    too_late = ( ( buffer_count * samples_per_buffer ) * 1000 ) / freq;

    for (unsigned int i = 0; i < buffer_count; i++)
    {
        Result = AudioQueueAllocateBuffer(audio_queue, buffer_size_bytes, &buffers[i]);
        if (Result != 0)
        {
            bink_coreaudio_close();
            return 0;
        }
    }

    Result = AudioQueueStart(audio_queue, 0);
    if (Result)
    {
        bink_coreaudio_close();
        return 0;
    }

    *fragsize = buffer_size_bytes;
    return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void bink_coreaudio_close()
{
    // Destroys the buffers also...
    if (audio_queue)
        AudioQueueDispose(audio_queue, true);
    audio_queue = 0;

    queued_count = 0;
    buffer_index = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern U32 RADTimerRead();

static int bink_coreaudio_write(const void * data, int length) 
{
    static U32 last_time = 0;

    U32 new_time = RADTimerRead();
    if ( ( new_time - last_time ) > too_late )  // have we fallen behind, if so reset
       AudioQueueReset( audio_queue );
    last_time = new_time;

    memcpy(buffers[buffer_index]->mAudioData, data, length);
    buffers[buffer_index]->mAudioDataByteSize = length;
   
    AudioQueueEnqueueBuffer(audio_queue, buffers[buffer_index], 0, 0);

    rrAtomicAddExchange32(&queued_count,1);

    buffer_index++;
    if (buffer_index >= buffer_count)
        buffer_index=0;

    return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static int bink_coreaudio_ready()
{
    int avail_count = buffer_count - rrAtomicLoadAcquire32(&queued_count);
    return avail_count * buffer_size_bytes;
}

static int bink_coreaudio_check( int * freq, int * chans, int * latency_ms, int * poll_ms )
{
    // core audio should be able to handle whatever freq/chans
    if (*freq == 0)
        *freq = 48000;
    if (*chans < 2)
        *chans = 2;
    
    *latency_ms = ( ( buffer_count * samples_per_buffer ) * 1000 ) / 48000;

    *poll_ms = 5;
    return 1;
}

#include "binkssmix.h"

static audio_funcs funcs = { bink_coreaudio_open, bink_coreaudio_close, bink_coreaudio_ready, bink_coreaudio_write, bink_coreaudio_check };

#ifdef __cplusplus // apparently some compilers will try to compile this as a cpp file.
extern "C"
#endif
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenCoreAudio( UINTa freq, UINTa speakers )
{
    return BinkOpenSSMix( &funcs, (U32)freq, (U32)speakers );
}

#ifdef __cplusplus // apparently some compilers will try to compile this as a cpp file.
extern "C"
#endif
void bink_coreaudio_ios_resume()
{
    // after an interrupt, the queue is stopped
    AudioQueueStart(audio_queue, 0);
}