// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#ifndef __BINKH__
#define __BINKH__

#ifndef EXPMACRO
#define EXPMACRO
#endif

#ifndef EXPGROUP
#define EXPGROUP(dummy)
#endif

EXPGROUP(_NullGroup)
#define BinkDate    "2022-05-08" EXPMACRO    

#define BinkVersion     "2022.05" EXPMACRO
#define BinkMajorVersion    2022  EXPMACRO    
#define BinkMinorVersion       5  EXPMACRO
#define BinkReleaseNumber      0  EXPMACRO 

#define BinkCustomization      0  EXPMACRO 
#define BinkBuildNumber        0  EXPMACRO 

EXPGROUP(_RootGroup)


#ifndef __RADRES__

#ifndef __RADTYPESH__
#include "egttypes.h"
#endif

RADDEFSTART

typedef struct BINK * HBINK;

// high level IO callbacks
struct BINKIO;
typedef S32  (RADLINK * BINKIOOPEN)         (struct BINKIO * Bnkio, const char  *name, U32 flags);
typedef U64  (RADLINK * BINKIOREADHEADER)   (struct BINKIO * Bnkio, S64 Offset, void * Dest,U64 Size);
typedef U64  (RADLINK * BINKIOREADFRAME)    (struct BINKIO * Bnkio, U32 Framenum,S64 origofs,void * dest,U64 size);
typedef U64  (RADLINK * BINKIOGETBUFFERSIZE)(struct BINKIO * Bnkio, U64 Size);
typedef void (RADLINK * BINKIOSETINFO)      (struct BINKIO * Bnkio, void * Buf,U64 Size,U64 FileSize,U32 simulate);
typedef U64  (RADLINK * BINKIOIDLE)         (struct BINKIO * Bnkio);
typedef void (RADLINK * BINKIOCLOSE)        (struct BINKIO * Bnkio);
typedef S32  (RADLINK * BINKIOBGCONTROL)    (struct BINKIO * Bnkio, U32 Control);

// functions that the high level IO system 
typedef void (RADLINK * BINKCBSUSPEND)      (struct BINKIO * Bnkio);
typedef S32  (RADLINK * BINKCBTRYSUSPEND)   (struct BINKIO * Bnkio);
typedef void (RADLINK * BINKCBRESUME)       (struct BINKIO * Bnkio);
typedef void (RADLINK * BINKCBIDLE)         (struct BINKIO * Bnkio);
typedef void (RADLINK * BINKCBSIMULATE)     (struct BINKIO * Bnkio, U64 amt, U32 timer );
typedef U32  (RADLINK * BINKCBTIMER)        ( void );
typedef void (RADLINK * BINKCBFLIPENDIAN)   ( void * ptr, U64 size );
typedef U64  (RADLINK * BINKCBLOCKEDADD)    ( U64 volatile * ptr, S64 amt );
typedef void (RADLINK * BINKCBMEMCPY)       ( void * d, void const * s, U64 amt );

typedef struct BINKIO {
  BINKIOREADHEADER ReadHeader;
  BINKIOREADFRAME  ReadFrame;
  BINKIOGETBUFFERSIZE GetBufferSize;
  BINKIOSETINFO SetInfo;

  BINKIOIDLE Idle;
  BINKIOCLOSE Close;
  BINKIOBGCONTROL BGControl;
  HBINK bink;

  // these are callbacks back into bink itself
  BINKCBSUSPEND suspend_callback;
  BINKCBTRYSUSPEND try_suspend_callback;
  BINKCBRESUME resume_callback;
  BINKCBIDLE idle_on_callback;

  BINKCBSIMULATE simulate_callback;
  BINKCBTIMER timer_callback;
  BINKCBFLIPENDIAN flipendian_callback;
  BINKCBLOCKEDADD lockedadd_callback;

  BINKCBMEMCPY memcpy_callback;
  void * align_ptrs[3];

  volatile U64 BytesRead;
  volatile U64 BufSize;
  volatile U64 BufHighUsed;
  volatile U64 CurBufSize;

  volatile U64 CurBufUsed;
  volatile U32 ReadError;
  volatile U32 DoingARead;

  volatile U32 Working;
  volatile U32 TotalTime;
  volatile U32 ForegroundTime;
  volatile U32 IdleTime;

  volatile U32 ThreadTime;
  volatile U32 Suspended;
  U32 align[ 2 ];
  
  volatile U8 iodata[128+32];
} BINKIO;

struct BINKSND;
typedef S32  (RADLINK * BINKSNDOPEN)       (struct BINKSND * BnkSnd, U32 freq, S32 chans, U32 flags, HBINK bink);
typedef S32  (RADLINK * BINKSNDREADY)      (struct BINKSND * BnkSnd);
typedef S32  (RADLINK * BINKSNDLOCK)       (struct BINKSND * BnkSnd, U8 * * addr, U32 * len);
typedef S32  (RADLINK * BINKSNDUNLOCK)     (struct BINKSND * BnkSnd, U32 filled);
typedef void (RADLINK * BINKSNDVOLUME)     (struct BINKSND * BnkSnd, S32 volume);
typedef void (RADLINK * BINKSNDPAN)        (struct BINKSND * BnkSnd, S32 pan);
typedef void (RADLINK * BINKSNDSPKOUTVOLS) (struct BINKSND * BnkSnd, F32 * volumes, U32 total );
typedef S32  (RADLINK * BINKSNDONOFF)      (struct BINKSND * BnkSnd, S32 status);
typedef S32  (RADLINK * BINKSNDPAUSE)      (struct BINKSND * BnkSnd, S32 status);
typedef void (RADLINK * BINKSNDCLOSE)      (struct BINKSND * BnkSnd);
typedef S32  (RADLINK * BINKSNDPROP)       (struct BINKSND * BnkSnd, S32 prop, UINTa* value);

typedef U32 (RADLINK *  BINKTIMERREAD)     (void);
RADEXPFUNC void RADEXPLINK BinkSetTimerRead( BINKTIMERREAD uread );

typedef BINKSNDOPEN  (RADLINK * BINKSNDSYSOPEN) (UINTa param);
typedef BINKSNDOPEN  (RADLINK * BINKSNDSYSOPEN2) (UINTa param1, UINTa param2);

typedef struct BINKSND {
  U8 * sndwritepos;       // current write position

  U32 unused;             // can be repurposed eventually

  U32 sndbufsize;         // sound buffer size
  U8 * sndbuf;            // sound buffer
  U8 * sndend;            // end of the sound buffer
  UINTa sndcomp;          // sound compression handle

  U8 * sndreadpos;        // current read position

  U32 orig_freq;
  U32 freq;
  S32 chans;

  S32 BestSize;
  U32 BestSizeMask;
  S32 OnOff;
  U32 Latency;
  U32 VideoScale;

  U32 sndendframe;            // frame number that the sound ends on

  U32 sndpad;                 // padded this much audio
  S32 sndprime;               // amount of data to prime the playahead

  S32 ThreadServiceType;      // 0    = normal (calls ready/lock/unlock for each BINKSND)
                              // -num = global (calls unlock and expects the sound handler 
                              //          to mix *all* BINKSNDS) "num" is how often to call
                              //          unlock, for example "-5" is every 5 ms)
                              // 1    = No background thread processing at all (sound system
                              //          will do it's own threading)
                              // 2    = No background thread processing at all, but setup
                              //          thread mutexes, so that BinkService() can be
                              //          call from an external background thread.                                       

  U32 SoundDroppedOut;

  U32 align[2];

  U8 snddata[256];

  BINKSNDREADY Ready;
  BINKSNDLOCK Lock;
  BINKSNDUNLOCK Unlock;
  BINKSNDVOLUME Volume;
  BINKSNDPAN Pan;
  BINKSNDPAUSE Pause;
  BINKSNDONOFF SetOnOff;
  BINKSNDCLOSE Close;
  BINKSNDSPKOUTVOLS SpeakerVols;
  BINKSNDPROP Property;

} BINKSND;


typedef struct BINKRECT {
  S32 Left,Top,Width,Height;
} BINKRECT;

#define BINKMAXDIRTYRECTS 1

typedef struct BUNDLEPOINTERS {
  void* typeptr;
  void* type16ptr;
  void* colorptr;
  void* bits2ptr;

  void* motionXptr;
  void* motionYptr;
  void* dctptr;
  void* mdctptr;

  void* patptr;
} BUNDLEPOINTERS;

typedef struct BINKPLANE
{
  void * Buffer;
  S32 Allocate;
  U32 BufferPitch;
} BINKPLANE;

typedef struct BINKFRAMEPLANESET
{
  BINKPLANE YPlane;
  BINKPLANE cRPlane;
  BINKPLANE cBPlane;
  BINKPLANE APlane;
  BINKPLANE HPlane;
} BINKFRAMEPLANESET;

#define BINKMAXFRAMEBUFFERS 3

typedef struct BINKFRAMEBUFFERS
{
  S32 TotalFrames;
  U32 YABufferWidth;
  U32 YABufferHeight;
  U32 cRcBBufferWidth;
  U32 cRcBBufferHeight;

  U32 FrameNum;  // 0 to (TotalFrames-1)
  BINKFRAMEPLANESET Frames[ BINKMAXFRAMEBUFFERS ];
} BINKFRAMEBUFFERS;

#define BINKMAXPLANES 5
#define BINKMAXSLICES 13

typedef struct BINKGPUDATABUFFERS BINKGPUDATABUFFERS;

typedef struct BINKSLICES
{
  U32 slice_inc;
  U32 slice_count;
  U32 slice_pitch;
  U32 slices[BINKMAXSLICES];
} BINKSLICES;

typedef struct BINK {
  U32 Width;                  // Width (1 based, 640 for example)
  U32 Height;                 // Height (1 based, 480 for example)
  U32 Frames;                 // Number of frames (1 based, 100 = 100 frames)
  U32 FrameNum;               // Frame to *be* displayed (1 based)

  U32 LastFrameNum;           // Last frame decompressed or skipped (1 based)
  U32 FrameRate;              // Frame Rate Numerator
  U32 FrameRateDiv;           // Frame Rate Divisor (frame rate=numerator/divisor)
  U32 ReadError;              // Non-zero if a read error has ocurred

  U32 OpenFlags;              // flags used on open
  U32 BinkType;               // Bink flags
  U32 LargestFrameSize;       // Largest frame size
  U32 FrameSize;              // The current frame's size in bytes

  U32 SndSize;                // The current frame sound tracks' size in bytes
  U32 FrameChangePercent;     // very rough percentage of the frame that changed
  S32 NumTracks;              // how many tracks
  S32 NumRects;

  U32 soundon;                // sound turned on?
  U32 videoon;                // video turned on?
  U32 needio;                 // set to 1, if we need an io before the next binkframe
  S32 closing;                // are we closing?

  BINKRECT FrameRects[BINKMAXDIRTYRECTS];// Dirty rects from BinkGetRects

  F32 ColorSpace[16];         // 4x4 matrix to use for colorspace conversion
                              // in HDR, 0 = smpte2084(maxnits/10k)*0.5, 1/2=ct/cp scale, 3/4=ct/cp bias, 5 = maxnits

  U64 FileOffset;             // Offset into the file where the Bink starts        
  U64 Highest1SecRate;        // Highest 1 sec data rate

  BINKFRAMEBUFFERS * FrameBuffers; // Bink frame buffers that we decompress to

  // everything below is for internal Bink use

  struct BINK * next_bink;    // linked list of open binks that are being access from threads
  void * compframe;           // compressed frame data
  S32 * trackindexes;         // track indexes

  U64 Size;                   // size of file
  U64 iosize;                 // io buffer size
  U64 totalmem;               // total memory used
  U64 compframeoffset;        // compressed frame offset

  S32 async_in_progress[8];   // is an async decompression in progress

  U64 lastfileread;           // last place we read binkfile
  U32 marker;
  S32 Paused;                 // is the bink movie paused?
  
  U32 skippedlastblit;        // skipped last frame?
  U32 compframesize;          // compressed frame size
  U32 compframekey;           // if this frame a key frame
  U32 playingtracks;          // how many tracks are playing

  BINKSLICES slices;

  BUNDLEPOINTERS bunp;        // pointers to internal temporary memory

  // we are at ptr ofs 1 here (bunp has 9 ptrs)
  BINKSND * bsnd;             // SND structures

  BINKGPUDATABUFFERS * gpu;   // pointer to gpu data pointers

  // now we are at ptr ofs 3 here
  U64* frameoffsets;          // offsets of each of the frames

  // back on alignement here
  U32 bink_unique;            // unique number
  S32 in_bink_list;           // is this bink in the linked list?
  U32 last_time_almost_empty; // time of last almost empty IO buffer
  S32 allkeys;                // are all frames keyframes?

  U32 * tracksizes;           // largest single frame of track
  U32 * tracktypes;           // type of each sound track
  S32 * trackIDs;             // external track numbers
  void * preloadptr;          // preloaded compressed frame data

  S32 simulate;               // simulate speed variables
  S32 adjustsim;
  U32 Highest1SecFrame;       // Highest 1 sec data rate starting frame
  U32 compframenum;           // frame to background load

  BINKIO bio;                 // IO structure 

  U8 pri_io_mutex[ 192 ];     // taken when priority io in progress

  U8 io_mutex[ 192 ];         // taken when background io in progress

  U8 snd_mutex[ 192 ];        // taken when background snd in progress

  U8 * ioptr;                 // io buffer ptr
  U32 * rtframetimes;         // start times for runtime frames
  U32 * rtadecomptimes;       // decompress times for runtime frames
  U32 * rtvdecomptimes;       // decompress times for runtime frames

  U32 * rtblittimes;          // blit times for runtime frames
  U32 * rtreadtimes;          // read times for runtime frames
  U32 * rtidlereadtimes;      // idle read times for runtime frames
  U32 * rtthreadreadtimes;    // thread read times for runtime frames

  U32 runtimeframes;          // max frames for runtime analysis
  S32 rtindex;                // index of where we are in the runtime frames
  U32 InternalFrames;         // how many frames were potentially compressed
  U32 numrects;               // number of rects from BinkGetRects

  U32 playedframes;           // how many frames have we played
  U32 firstframetime;         // very first frame start
  U32 limit_speakers;
  U32 soundskips;             // number of sound stops

  U32 startblittime;          // start of blit period
  U32 startsynctime;          // start of synched time
  U32 startsyncframe;         // frame of startsynctime
  U32 twoframestime;          // two frames worth of time

  U32 slowestframetime;       // slowest frame in ms
  U32 slowestframe;           // slowest frame number
  U32 slowest2frametime;      // second slowest frame in ms
  U32 slowest2frame;          // second slowest frame

  U32 timevdecomp;            // total time decompressing video
  U32 timeadecomp;            // total time decompressing audio
  U32 timeblit;               // total time blitting
  U32 timeopen;               // total open time

  U32 fileframerate;          // frame rate originally in the file
  U32 fileframeratediv;
  U32 lastblitflags;          // flags used on last blit
  U32 lastdecompframe;        // last frame number decompressed

  U32 lastfinisheddoframe;    // time that the last do frame finished
  U32 lastresynctime;         // last loop point that we did a resync on
  U32 doresync;               // should we do a resync in the next doframe?
  U32 skipped_status_this_frame;//0=not checked this frame, 1=no skip, 2=skip

  U32 very_delayed;           // is this frame more than 725 ms late?
  U32 skippedblits;           // how many blits were skipped
  U32 skipped_in_a_row;       // how many frames have we skipped in a row
  U32 paused_sync_diff;       // sync delta at the time of a pause

  void * alloccompframe;      // compressed frame data base address
  BINKFRAMEBUFFERS * allocatedframebuffers; // pointer to internally allocated buffers
  void * seam;
  void * binkaudiomem;        // memory for the bink audio structures

  void * opt_ptr;

  U32    opt_bytes;
  U32 changepercent;          // how much roughly did the current frame change?

  void * subimage;
  void * subvalid;
  U32    subimagesize;
} BINK;


typedef struct BINKSUMMARY {
  U64 TotalReadSpeed;         // Total io speed (bytes/second)
  U64 HighestMemAmount;       // Highest amount of memory allocated
  U64 TotalIOMemory;          // Total extra memory allocated
  U64 HighestIOUsed;          // Highest extra memory actually used
  U64 Highest1SecRate;        // Highest 1 second rate
  U32 Width;                  // Width of frames
  U32 Height;                 // Height of frames
  U32 TotalTime;              // total time (ms)
  U32 FileFrameRate;          // frame rate
  U32 FileFrameRateDiv;       // frame rate divisor
  U32 FrameRate;              // frame rate
  U32 FrameRateDiv;           // frame rate divisor
  U32 TotalOpenTime;          // Time to open and prepare for decompression
  U32 TotalFrames;            // Total Frames
  U32 TotalPlayedFrames;      // Total Frames played
  U32 SkippedFrames;          // Total number of skipped frames
  U32 SkippedBlits;           // Total number of skipped blits
  U32 SoundSkips;             // Total number of sound skips
  U32 TotalBlitTime;          // Total time spent blitting
  U32 TotalReadTime;          // Total time spent reading
  U32 TotalVideoDecompTime;   // Total time spent decompressing video
  U32 TotalAudioDecompTime;   // Total time spent decompressing audio
  U32 TotalIdleReadTime;      // Total time spent reading while idle
  U32 TotalBackReadTime;      // Total time spent reading in background
  U32 SlowestFrameTime;       // Slowest single frame time (ms)
  U32 Slowest2FrameTime;      // Second slowest single frame time (ms)
  U32 SlowestFrameNum;        // Slowest single frame number
  U32 Slowest2FrameNum;       // Second slowest single frame number
  U32 AverageDataRate;        // Average data rate of the movie
  U32 AverageFrameSize;       // Average size of the frame
  U32 Highest1SecFrame;       // Highest 1 second start frame
} BINKSUMMARY;


typedef struct BINKREALTIME {
  U64 ReadBufferSize;         // size of read buffer
  U64 ReadBufferUsed;         // amount of read buffer currently used
  U64 FramesDataRate;         // data rate for these frames
  U32 FrameNum;               // Current frame number
  U32 FrameRate;              // frame rate
  U32 FrameRateDiv;           // frame rate divisor
  U32 Frames;                 // frames in this sample period
  U32 FramesTime;             // time is ms for these frames
  U32 FramesVideoDecompTime;  // time decompressing these frames
  U32 FramesAudioDecompTime;  // time decompressing these frames
  U32 FramesReadTime;         // time reading these frames
  U32 FramesIdleReadTime;     // time reading these frames at idle
  U32 FramesThreadReadTime;   // time reading these frames in background
  U32 FramesBlitTime;         // time blitting these frames
} BINKREALTIME;

#define BINKMARKER1  'fKIB'
#define BINKMARKER2  'gKIB'    // new Bink files use this tag
#define BINKMARKER3  'hKIB'    // newer Bink files use this tag
#define BINKMARKER4  'iKIB'    // even newer Bink files use this tag
#define BINKMARKER5  'kKIB'    // added new colorspace, custom colorspaces and constant alpha

#define BINKMARKERV2   'f2BK'
#define BINKMARKERV22  'g2BK'  // bink 2.2
#define BINKMARKERV23  'h2BK'  // bink 2.3 
#define BINKMARKERV23D 'i2BK'  // bink 2.3d version
#define BINKMARKERV25  'j2BK'  // bink 2.5 (new alpha stuff)
#define BINKMARKERV25K 'k2BK'  // bink 2.5k (custom colorspaces)
#define BINKMARKERV27  'm2BK'  // bink 2.7 (hdr)
#define BINKMARKERV28  'n2BK'  // bink audio 2 support

typedef struct BINKHEADER {
  U32 Marker;                 // Bink marker
  U32 Size;                   // size of the file-8
  U32 Frames;                 // Number of frames (1 based, 100 = 100 frames)
  U32 LargestFrameSize;       // Size in bytes of largest frame
  U32 InternalFrames;         // Number of internal frames

  U32 Width;                  // Width (1 based, 640 for example)
  U32 Height;                 // Height (1 based, 480 for example)
  U32 FrameRate;              // frame rate
  U32 FrameRateDiv;           // frame rate divisor (framerate/frameratediv=fps)

  U32 Flags;                  // bink flags
  U32 NumTracks;              // number of tracks
} BINKHEADER;


typedef struct BINK_OPEN_OPTIONS
{
  U64 FileOffset;
  S64 IOBufferSize;
  S32 ForceRate;
  S32 ForceRateDiv;
  S32 Simulate;
  U32 TotTracks;
  S32 * TrackNums;
  BINKIOOPEN UserOpen;
} BINK_OPEN_OPTIONS;

//=======================================================================
#define BINKYAINVERT           0x00000800L // Reverse Y and A planes when blitting (for debugging)
#define BINKFRAMERATE          0x00001000L // Override fr (call BinkFrameRate first)
#define BINKPRELOADALL         0x00002000L // Preload the entire animation
#define BINKSNDTRACK           0x00004000L // Set the track number to play
#define BINKOLDFRAMEFORMAT     0x00008000L // using the old Bink frame format (internal use only)
#define BINKCUSTOMCOLORSPACE   0x00010000L // file contains a custom colorspace matrix (only internal now)
#define BINKGRAYSCALE          0x00020000L // Force Bink to use grayscale
#define BINKAUDIO2             0x00040000L // File contains Bink Audio 2 tracks
#define BINKNOSKIP             0x00080000L // Don't skip frames if falling behind
#define BINKALPHA              0x00100000L // Decompress alpha plane (if present)
#define BINKNOFILLIOBUF        0x00200000L // Don't Fill the IO buffer (in BinkOpen and BinkCopyTo)
#define BINKSIMULATE           0x00400000L // Simulate the speed (call BinkSim first)
#define BINKFILEHANDLE         0x00800000L // Use when passing in a file handle
#define BINKIOSIZE             0x01000000L // Set an io size (call BinkIOSize first)
#define BINKIOPROCESSOR        0x02000000L // Set an io processor (call BinkIO first)
#define BINKFROMMEMORY         0x04000000L // Use when passing in a pointer to the file
#define BINKNOTHREADEDIO       0x08000000L // Don't use a background thread for IO
#define BINKFILEDATALOADONLY   0x10000000L // Just load the header and the data only
#define BINKNOFRAMEBUFFERS     0x00000400L // Don't allocate internal frame buffers - application must call BinkRegisterFrameBuffers
#define BINKNOYPLANE           0x00000200L // Don't decompress the Y plane (internal flag)
#define BINKGPU                0x00000100L // Open Bink in GPU mode
#define BINKWILLLOOP           0x00000080L // You are planning to loop this Bink.
#define BINKDONTCLOSETHREADS   0x00000040L // Don't close threads on BinkClose (call BinkFreeGlobals to close threads)
#define BINKFILEOFFSET         0x00000020L // Use file offset specified by BinkSetFileOffset
#define BINKYCRCBNEW           0x00000010L // File uses the new ycrcb colorspace (usually Bink 2)
#define BINKUSETRIPLEBUFFERING 0x00000008L // Use triple buffering in the framebuffers
#define BINKHDR                0x00000004L // Video is an HDR video
#define BINK_SLICES_2          0x00000000L // Bink 2 file has two slices
#define BINK_SLICES_3          0x00000001L // Bink 2 file has three slices
#define BINK_SLICES_4          0x00000002L // Bink 2 file has four slices
#define BINK_SLICES_8          0x00000003L // Bink 2 file has eight slices
#define BINK_SLICES_MASK       0x00000003L // mask against openflags to get the slice flags

// these flags are only used in the compressor, so we reuse some of the playback flags
#define BINK_ALPHA_WEIGHT          0x80000L  
#define BINK_VAQ_OFF               0x01000000
#define BINK_VAQ_LIGHT             0x02000000
#define BINK_VAQ_HEAVY             0x03000000
#define BINK_VAQ_CONTROL_MASK      0x03000000
#define BINK_DEBLOCK_UANDD_OFF     0x4000L
#define BINK_DEBLOCK_LTOR_OFF      0x8000L
#define BINK_DEBLOCK_ALL_OFF       0xC000L
#define BINK_DEBLOCK_CONTROL_MASK  0xC000L


#define BINKSURFACE32BGRx      3
#define BINKSURFACE32RGBx      4
#define BINKSURFACE32BGRA      5
#define BINKSURFACE32RGBA      6
#define BINKSURFACE5551        8
#define BINKSURFACE555         9
#define BINKSURFACE565        10
#define BINKSURFACE32ARGB     12
#define BINKSURFACEMASK       15

#define BINKGOTOQUICK          1
#define BINKGOTOQUICKSOUND     2

#define BINKGETKEYPREVIOUS     0 
#define BINKGETKEYNEXT         1
#define BINKGETKEYCLOSEST      2
#define BINKGETKEYNOTEQUAL   128

#define BINKDOFRAMEALL           1
#define BINKDOFRAMEHALF          2
#define BINKDOFRAMETHIRD         3
#define BINKDOFRAMEFOURTH        4
#define BINKDOFRAMEEIGHTH        5
#define BINKDOFRAMESTART       256
#define BINKDOFRAMEEND         512

#define BINKPLATFORMSOUNDTHREAD       1
#define BINKPLATFORMIOTHREAD          2
#define BINKPLATFORMSUBMITTHREADCOUNT 3
#define BINKPLATFORMSUBMITTHREADS     4

#ifdef __RADIPHONE__
    #define BINKPLATFORMIOSINTERRUPTEND 5
#endif

#define BINKPLATFORMASYNCTHREADS   1024  // +0 to 7 for which async

//=======================================================================

RADEXPFUNC void * RADEXPLINK BinkLogoAddress(void);

RADEXPFUNC void RADEXPLINK BinkSetError(char const * err);
RADEXPFUNC char * RADEXPLINK BinkGetError(void);

RADEXPFUNC HBINK RADEXPLINK BinkOpen(char const * name,U32 flags);
RADEXPFUNC HBINK RADEXPLINK BinkOpenWithOptions(char const * name, BINK_OPEN_OPTIONS const * boo,U32 flags);

RADEXPFUNC void RADEXPLINK BinkGetFrameBuffersInfo( HBINK bink, BINKFRAMEBUFFERS * fbset );
RADEXPFUNC void RADEXPLINK BinkRegisterFrameBuffers( HBINK bink, BINKFRAMEBUFFERS * fbset );
RADEXPFUNC S32  RADEXPLINK BinkDoFrame(HBINK bnk);
RADEXPFUNC S32  RADEXPLINK BinkDoFramePlane(HBINK bnk,U32 which_planes);
RADEXPFUNC void RADEXPLINK BinkNextFrame(HBINK bnk);
RADEXPFUNC S32  RADEXPLINK BinkWait(HBINK bnk);
RADEXPFUNC void RADEXPLINK BinkClose(HBINK bnk);
RADEXPFUNC S32  RADEXPLINK BinkPause(HBINK bnk,S32 pause);
RADEXPFUNC S32  RADEXPLINK BinkCopyToBuffer(HBINK bnk,void* dest,S32 destpitch,U32 destheight,U32 destx,U32 desty,U32 flags);
RADEXPFUNC S32  RADEXPLINK BinkCopyToBufferRect(HBINK bnk,void* dest,S32 destpitch,U32 destheight,U32 destx,U32 desty,U32 srcx, U32 srcy, U32 srcw, U32 srch, U32 flags);
RADEXPFUNC S32  RADEXPLINK BinkGetRects(HBINK bnk);
RADEXPFUNC void RADEXPLINK BinkGoto(HBINK bnk,U32 frame,S32 flags);  // use 1 for the first frame
RADEXPFUNC U32  RADEXPLINK BinkGetKeyFrame(HBINK bnk,U32 frame,S32 flags);
RADEXPFUNC void RADEXPLINK BinkFreeGlobals( void );
RADEXPFUNC S32  RADEXPLINK BinkGetPlatformInfo( S32 bink_platform_enum, void * output );

RADEXPFUNC S32  RADEXPLINK BinkSetVideoOnOff(HBINK bnk,S32 onoff);
RADEXPFUNC S32  RADEXPLINK BinkSetSoundOnOff(HBINK bnk,S32 onoff);
RADEXPFUNC void RADEXPLINK BinkSetVolume(HBINK bnk, U32 trackid, S32 volume);
RADEXPFUNC void RADEXPLINK BinkSetPan(HBINK bnk,U32 trackid, S32 pan);
RADEXPFUNC void RADEXPLINK BinkSetSpeakerVolumes(HBINK bnk,U32 trackid, U32 * speaker_indexes, S32 * volumes, U32 total);
#define BinkSetMixBins(bnk,trackid,speaker_indexes,total) BinkSetSpeakerVolumes(bnk,trackid,speaker_indexes,0,total)
#define BinkSetMixBinVolumes(bnk,trackid,speaker_indexes,volumes,total) BinkSetSpeakerVolumes(bnk,trackid,speaker_indexes,volumes,total)
RADEXPFUNC void RADEXPLINK BinkService(HBINK bink);

RADEXPFUNC S32  RADEXPLINK BinkShouldSkip(HBINK bink);


// low level IO callbacks - used with the default IO system
typedef S32  (RADLINK * BINKFILEOPEN)  ( UINTa * out_user_data, char const * name, U64 * optional_out_size );  // returns 1 on success
typedef U64  (RADLINK * BINKFILEREAD)  ( UINTa * out_user_data, void * dest, U64 bytes );
typedef void (RADLINK * BINKFILESEEK)  ( UINTa * out_user_data, U64 pos );
typedef void (RADLINK * BINKFILECLOSE) ( UINTa * out_user_data );
RADEXPFUNC void RADEXPLINK BinkSetOSFileCallbacks( BINKFILEOPEN o, BINKFILEREAD r, BINKFILESEEK s, BINKFILECLOSE c );


#define BINKBGIOSUSPEND 1
#define BINKBGIORESUME  2
#define BINKBGIOWAIT    0x80000000
#define BINKBGIOTRYWAIT 0x40000000

RADEXPFUNC S32  RADEXPLINK BinkControlBackgroundIO(HBINK bink,U32 control);

RADEXPFUNC void RADEXPLINK BinkSetWillLoop(HBINK bink,S32 onoff);

// utility stuff
RADEXPFUNC U32  RADEXPLINK BinkUtilCPUs(void);
RADEXPFUNC void * RADEXPLINK BinkUtilMalloc(U64 bytes);
RADEXPFUNC void RADEXPLINK BinkUtilFree(void * ptr);
RADEXPFUNC S32  RADEXPLINK BinkAllocateFrameBuffers( HBINK bp, BINKFRAMEBUFFERS * set, U32 minimum_alignment );

RADEXPFUNC S32  RADEXPLINK BinkStartAsyncThread( S32 thread_num, void const * param );
RADEXPFUNC S32  RADEXPLINK BinkDoFrameAsync(HBINK bink, U32 thread_one, U32 thread_two );
RADEXPFUNC S32  RADEXPLINK BinkDoFrameAsyncMulti(HBINK bink, U32* threads, S32 thread_count );
RADEXPFUNC S32  RADEXPLINK BinkDoFrameAsyncWait(HBINK bink, S32 us);
RADEXPFUNC S32  RADEXPLINK BinkRequestStopAsyncThread( S32 thread_num );
RADEXPFUNC S32  RADEXPLINK BinkRequestStopAsyncThreadsMulti( U32* threads, S32 thread_count );
RADEXPFUNC S32  RADEXPLINK BinkWaitStopAsyncThread( S32 thread_num );
RADEXPFUNC S32  RADEXPLINK BinkWaitStopAsyncThreadsMulti( U32* threads, S32 thread_count );

#ifdef __RADBIGENDIAN__

RADEXPFUNC void RADEXPLINK BinkFlipEndian32( void * buffer, U64 bytes );

#endif


typedef struct BINKTRACK * HBINKTRACK;

typedef struct BINKTRACK
{
  U32 Frequency;
  U32 Channels;
  U32 MaxSize;

  HBINK bink;
  UINTa sndcomp;
  S32 trackindex;
} BINKTRACK;


RADEXPFUNC HBINKTRACK RADEXPLINK BinkOpenTrack(HBINK bnk,U32 trackindex);
RADEXPFUNC void RADEXPLINK BinkCloseTrack(HBINKTRACK bnkt);
RADEXPFUNC U32  RADEXPLINK BinkGetTrackData(HBINKTRACK bnkt,void * dest,U32 destlen);

RADEXPFUNC U32  RADEXPLINK BinkGetTrackType(HBINK bnk,U32 trackindex);
RADEXPFUNC U32  RADEXPLINK BinkGetTrackMaxSize(HBINK bnk,U32 trackindex);
RADEXPFUNC U32  RADEXPLINK BinkGetTrackID(HBINK bnk,U32 trackindex);

RADEXPFUNC void RADEXPLINK BinkGetSummary(HBINK bnk,BINKSUMMARY * sum);
RADEXPFUNC void RADEXPLINK BinkGetRealtime(HBINK bink,BINKREALTIME * run,U32 frames);

RADEXPFUNC void RADEXPLINK BinkSetFileOffset(U64 offset);
RADEXPFUNC void RADEXPLINK BinkSetSoundTrack(U32 total_tracks, U32 * tracks);
RADEXPFUNC void RADEXPLINK BinkSetIO(BINKIOOPEN io);
RADEXPFUNC void RADEXPLINK BinkSetFrameRate(U32 forcerate,U32 forceratediv);
RADEXPFUNC void RADEXPLINK BinkSetSimulate(U32 sim);
RADEXPFUNC void RADEXPLINK BinkSetIOSize(U64 iosize);

RADEXPFUNC S32 RADEXPLINK BinkLoadSubtitles( HBINK bink, char const * srt_file );
RADEXPFUNC char const * RADEXPLINK BinkCurrentSubtitle( HBINK bink, U32 * iterate, U32 * opt_out_start_ms, U32 * opt_out_end_ms );
RADEXPFUNC char const * RADEXPLINK BinkGetSubtitleByIndex( HBINK bink, U32 index, U32 * opt_out_start_ms, U32 * opt_out_end_ms );


RADEXPFUNC S32  RADEXPLINK BinkSetSoundSystem(BINKSNDSYSOPEN open, UINTa param);
RADEXPFUNC S32  RADEXPLINK BinkSetSoundSystem2(BINKSNDSYSOPEN2 open, UINTa param1, UINTa param2);

#ifdef __RADIPHONE__
  RADEXPFUNC void RADEXPLINK BinkIOSPostAudioInterruptEnd();
#endif

RADEXPFUNC void RADEXPLINK BinkSetAssetManager(void* asset_manager);

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenSLES( UINTa param1, UINTa param2 );
#define BinkSoundUseOpenSLES( frequency, number_of_speakers ) BinkSetSoundSystem2( BinkOpenSLES, frequency, number_of_speakers )

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenDirectSound(UINTa param); // don't call directly
#define BinkSoundUseDirectSound(lpDS) BinkSetSoundSystem(BinkOpenDirectSound,(UINTa)(lpDS))

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenWaveOut(UINTa param); // don't call directly
#define BinkSoundUseWaveOut() BinkSetSoundSystem(BinkOpenWaveOut,0)

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenXAudio2(UINTa param1,UINTa param2); // don't call directly
#define BinkSoundUseXAudio2(IXAudio2_ptr, IXAudio2MasteringVoice_ptr) BinkSetSoundSystem2(BinkOpenXAudio2,(UINTa)(IXAudio2_ptr), (UINTa)(IXAudio2MasteringVoice_ptr))

RADEXPFUNC S32 RADEXPLINK BinkFindXAudio2WinDevice( void * wchar_out, char const * match );
#define BinkSoundUseXAudio2WinDevice(char_ptr_to_device_name) BinkSetSoundSystem2(BinkOpenXAudio2,(UINTa)(-2), (UINTa)(char_ptr_to_device_name))

// on desktop windows, BinkSoundUseXAudio2 will call one of these two functions automatically, 
//    but you can directly call, if you want to be sure you are getting the correct version
#define BinkSoundUseXAudio27(IXAudio2_ptr, IXAudio2MasteringVoice_ptr) BinkSetSoundSystem2(BinkOpenXAudio27,(UINTa)(IXAudio2_ptr), (UINTa)(IXAudio2MasteringVoice_ptr))
#define BinkSoundUseXAudio28(IXAudio2_ptr, IXAudio2MasteringVoice_ptr) BinkSetSoundSystem2(BinkOpenXAudio28,(UINTa)(IXAudio2_ptr), (UINTa)(IXAudio2MasteringVoice_ptr))
#define BinkSoundUseXAudio29(IXAudio2_ptr, IXAudio2MasteringVoice_ptr) BinkSetSoundSystem2(BinkOpenXAudio28,(UINTa)(IXAudio2_ptr), (UINTa)(IXAudio2MasteringVoice_ptr))

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenXAudio27(UINTa param1,UINTa param2); // don't call directly
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenXAudio28(UINTa param1,UINTa param2); // don't call directly
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenXAudio29(UINTa param1,UINTa param2); // don't call directly

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenCoreAudio( UINTa frequency, UINTa num_speakers );
#define BinkSoundUseCoreAudio(frequency, num_speakers) BinkSetSoundSystem2(BinkOpenCoreAudio, frequency, num_speakers) // mac or ios

#ifdef __RADLINUX__
  RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenOAL( UINTa frequency, UINTa num_speakers );
  #define BinkSoundUseOpenAL(frequency, num_speakers) BinkSetSoundSystem2(BinkOpenOAL, frequency, num_speakers)

  RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenPulseAudio( UINTa param1, UINTa param2 ); // don't call directly
  #define BinkSoundUsePulseAudio( frequency, number_of_speakers ) BinkSetSoundSystem2( BinkOpenPulseAudio, frequency, number_of_speakers )
#endif

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenLibAudio( U32 param ); // don't call directly
#define BinkSoundUseLibAudio( number_of_speakers ) BinkSetSoundSystem( BinkOpenLibAudio, number_of_speakers )

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenAudioRenderer( UINTa param ); // don't call directly
#define BinkSoundUseAudioRenderer( close_all_audio ) BinkSetSoundSystem( BinkOpenAudioRenderer, close_all_audio ) // pass non-zero to end all sound playback

RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenWebAudio(UINTa param);  // emscripten - don't call directly
#define BinkSoundUseWebAudio( rate ) BinkSetSoundSystem(BinkOpenWebAudio,rate)  // use 0 for default (48khz)

// number_of_speakers is 2 or 8
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenAudioOut( UINTa num_speaker );
#define BinkSoundUseSonySound( number_of_speakers ) BinkSetSoundSystem( BinkOpenAudioOut, number_of_speakers )

// number_of_speakers is 2 or 8
RADEXPFUNC BINKSNDOPEN RADEXPLINK BinkOpenAudioOut2( UINTa num_speaker );
#define BinkSoundUseAudioOut2( frequency, number_of_speakers ) BinkSetSoundSystem( BinkOpenAudioOut2, number_of_speakers )

typedef void * (RADLINK * BINKMEMALLOC) (U64 bytes);
typedef void   (RADLINK * BINKMEMFREE)  (void * ptr);

RADEXPFUNC void RADEXPLINK BinkSetMemory(BINKMEMALLOC a,BINKMEMFREE f);

RADEXPFUNC void RADEXPLINK BinkUseTelemetry3( void * api, U32 mask );

//=========================================================================

// All this stuff is part of the internal BinkGPU API,
// and some of it may end up getting compiled into shaders!

#define BINKGPUMAXEDGE      16384               // Max width or height of a BinkGPU video. Must be a multiple of 32.
#define BINKGPUMAXBLOCKS32  (BINKGPUMAXEDGE/32)
#define BINKGPUMAXBLOCKS8   (BINKGPUMAXEDGE/8)

// "Flags" field for BINKGPUDATABUFFERS
#define BINKGPUDEBLOCKUD       0x0001
#define BINKGPUDEBLOCKLR       0x0002
#define BINKGPUNEWCLAMP        0x0004
#define BINKGPUBINK25          0x0008
#define BINKGPUCONSTANTA       0x0010

#define BINKGPUPLANE_Y      0
#define BINKGPUPLANE_CR     1
#define BINKGPUPLANE_CB     2
#define BINKGPUPLANE_A      3
#define BINKGPUPLANE_H      4

typedef struct BINKGPUMOVEC
{
  S16 rel_x;
  S16 rel_y;
} BINKGPUMOVEC;

typedef struct BINKGPURANGE
{
  U32 Offset;                                   // in bytes
  U32 Size;                                     // in bytes
} BINKGPURANGE;

struct BINKGPUDATABUFFERS
{
  // Set up by BinkGetGPUDataBuffersInfo
  U8 const * ScanOrderTable;

  // Needs to be set up before call to DoFrame
  void * Dc[ BINKMAXPLANES ];
  void * Ac[ BINKMAXPLANES ];
  BINKGPURANGE * AcRanges[ BINKMAXPLANES ];     // One AC range per slice, per plane.
  BINKGPUMOVEC * Motion;
  U8 * TopFlag;
  U8 * DeblockUdFlag;
  U8 * DeblockLrFlag;

  U32 AcBufSize[ BINKMAXPLANES ];               // in bytes

  U32 BufferWidth;
  U32 BufferHeight;

  // Filled out by DoFrame.
  U32 DcThreshold;
  U32 Flags; 
  U32 FillAlphaValue;
  
  S32 MotionXClamp[4];                          // Old clamp: (tl_fullpel,wh_fullpel,tl_halfpel,wh_halfpel)
  S32 MotionYClamp[4];                          // New clamp: (min_fullpel,max_fullpel,min_halfpel,max_halfpel)
  S32 HasDataBeenDecoded;
};

RADEXPFUNC void RADEXPLINK BinkRegisterGPUDataBuffers( HBINK bink, BINKGPUDATABUFFERS * gpu );
RADEXPFUNC S32  RADEXPLINK BinkGetGPUDataBuffersInfo( HBINK bink, BINKGPUDATABUFFERS * gpu );

RADDEFEND

#endif

// @cdep pre $addcompilerincludedir($clipfilename($file)) $ignore(TakeCPP)

#endif

