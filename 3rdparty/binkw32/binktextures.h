// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

/*
This is Epics's high level API for using 3D hardware to do Bink GPU-assisted
decoding and color conversion. It is supported on every 3D platform that 
Bink runs on.

It's a simple API, so you should see your platform's example code
to see all the nitty-gritty details.


There two functions to start and stop Bink textures:

  Create_Bink_shaders:  This function creates the pixel shaders we use to
    do the color space conversion. On most platforms, you need to provide
    a pointer to GPU device pointer. 
    All of the other functions hang off the object that is returned here.

  Free_Bink_shaders:  Frees the pixel shaders created in Create_Bink_shaders.

There are four textures functions:

  Create_Bink_textures:  This function takes BINK object, creates the texture
    resources to render it quickly and binds them together for rendering.
  
  Free_Bink_textures:  Frees the resources allocated in Create_Bink_textures.

  Draw_Bink_textures:  Renders the textures onto the screen.

  Start_Bink_texture_update:  This function starts the texture updating process.
    It should be called before you call BinkDoFrame (synchronous or async).
    On some platforms, this locks/maps the texture for updating; on consoles, it
    makes sure that the GPU is done reading from the texture buffers that we're
    about to overwrite.

  Finish_Bink_texture_update:  This function finishes the texture updating process.
    It should be called after BinkDoFrame processing is done (in async mode: after
    BinkDoFrameAsyncWait returns true). This will unlock/unmap textures (where it
    applies) or issue a blit to transfer the new image data. On most consoles with
    unified memory, we decode directly into GPU-readable memory, and this operation
    is a no-op.

On Windows D3D9, there are two other functions:

  Before_Reset_Bink_textures: Call before you call the D3D Reset function.

  After_Reset_Bink_textures: Call after you call the D3D Reset function.

For Windows GL or D3D11, you simply free and re-create the textures completely
  whenever the device is lost or reset.



So, basically, playback works like this:

  1) Call Create_Bink_shaders to create the pixel shaders.
  
  2) Open the Bink file with the BINKNOFRAMEBUFFERS flag (the Binktexture
     implementations will allocate the memory to decompress into).

  3) Call Create_Bink_textures to create the textures and bind the textures
     into the Bink.

  4) Call Start_Bink_texture_update before BinkDoFrame.

  5) Call BinkDoFrame to decompress a video frame.

  6) Call Finish_Bink_texture_update after BinkDoFrame (or after
     BinkDoFrameAsyncWait returns true, if you're using async decoding).

  7) Draw the frame using Draw_Bink_textures.


And that's it! (Skipping over a few details - see the examples for all 
the details...)

Should drop in really quickly and it hides a ton of platform specific ugliness!
*/

#ifndef BINKTEXTURESH
#define BINKTEXTURESH

#ifndef __BINKH__
#include "bink.h"
#endif

RADDEFSTART

//=============================================================================

// On some platforms, you might have one app that uses BinkTextures with multiple
//   graphics APIs - these defines let you have multiple providers in one
//   EXE by suffixing the names of D3D9 and D3D11 (GL gets the non-suffix).
// This does mean that you must include D3D9 and/or D3D11 headers before
//   including BinkTextures.h!

#if defined( BINKTEXTURESGPUAPITYPE )

  #if defined( __d3d12_x_h__ ) || defined( __d3d12_xs_h__ ) ||defined( __d3d12_h__ ) || defined( BINKD3D12FUNCTIONS ) || defined( BINKD3D12GPUFUNCTIONS )
    #ifdef BINKD3D12GPUFUNCTIONS
      #define BINKTEXTURESSUFFIX D3D12GPU
    #else
      #define BINKTEXTURESSUFFIX D3D12
    #endif
  #elif defined( __d3d11_x_h__ ) || defined( __d3d11_h__ ) || defined( BINKD3D11FUNCTIONS ) || defined( BINKD3D11GPUFUNCTIONS )
    #ifdef BINKD3D11GPUFUNCTIONS
      #define BINKTEXTURESSUFFIX D3D11GPU
    #else
      #define BINKTEXTURESSUFFIX D3D11
    #endif
  #elif defined( _D3D9_H_ ) || defined( BINKD3D9FUNCTIONS )
    #define BINKTEXTURESSUFFIX D3D9
  #elif defined( BINKMETALFUNCTIONS )
    #define BINKTEXTURESSUFFIX Metal
  #elif defined( BINKVULKANFUNCTIONS )
    #define BINKTEXTURESSUFFIX Vulkan
  #elif defined(__gl_es20_h_) || defined(__gl_h_) || defined(__gl2_h_) || defined(__gl3_h_) || defined( BINKGLFUNCTIONS ) || defined(BINKGLGPUFUNCTIONS)
    #ifdef BINKGLGPUFUNCTIONS
      #define BINKTEXTURESSUFFIX GLGPU
    #else
      #define BINKTEXTURESSUFFIX GL
    #endif
  #elif defined(BINKNDAFUNCTIONS)
    #define BINKTEXTURESSUFFIX BINKNDAFUNCTIONS
  #elif defined(BINKRHIFUNCTIONS)
    #define BINKTEXTURESSUFFIX RHI
  #else
    #ifndef BINKTEXTURESSUFFIX
      #define BINKTEXTURESSUFFIX Null
    #endif
  #endif

  #define Create_Bink_shaders        RR_STRING_JOIN( Create_Bink_shaders,       BINKTEXTURESSUFFIX )

#endif

typedef struct BINKSHADERS BINKSHADERS;   // defined below
typedef struct BINKTEXTURES BINKTEXTURES; // defined below

//=============================================================================

// For some APIs, you need to pass structs as argument to our functions.
// This is where they get defined.

#if ( defined( __d3d12_h__ ) || defined( __d3d12_x_h__ ) || defined( __d3d12_xs_h__ ) ) && !defined( BINKTEXTURESD3D12STRUCTSDEF )

// BinkGPU can call you back at the beginning/end of its compute command lists in case you want to
// insert timestamp queries (or other markup) into the command list.
typedef void BINKGPUCOMMANDLISTCALLBACKD3D12( BINKTEXTURES * bink_tex, ID3D12GraphicsCommandList * cl, BOOL is_end, void * user_ptr );

// This is what you pass as "device" to Create_shaders.
typedef struct BINKCREATESHADERSD3D12 {
  // The D3D12 device to use
  ID3D12Device * device;
  
  // Command queue to submit work to in BinkGPU mode. Ignored in CPU decoding mode.
  ID3D12CommandQueue * gpu_mode_command_queue;

  // The fence to use for synchronization. Needs to be updated from app code!
  ID3D12Fence * fence; 

  // Prototype graphics pipeline state. Create Bink pipelines for the
  // specified set of rasterizer state (multisampling or not),
  // render target formats, etc. See binktexturesD3D12.cpp for
  // details.
  D3D12_GRAPHICS_PIPELINE_STATE_DESC prototype;

  // Callback to run at beginning/end of BinkGPU command list. Ignored in CPU decoding mode.
  BINKGPUCOMMANDLISTCALLBACKD3D12 * gpu_command_list_callback;
  void * gpu_command_list_user_ptr;
} BINKCREATESHADERSD3D12;

// This is what you pass as context to Draw_textures.
typedef struct BINKAPICONTEXTD3D12 {
  // ---- Input parameters you provide

  // Command list to render to
  ID3D12GraphicsCommandList * command_list; // Command list to render to

  // Fence value you're going to write via the command queue after executing
  // the command_list. The sequence of fence values written needs to be
  // monotonically increasing over time. This is what we use to synchronize
  // buffer upload ands resource management!
  UINT64 fence_value;

  // ---- Output parameters we return

  // In GPU decode mode, the draw command list may only run after decoding
  // the frame has finished. Thus you need to call "Wait" on your graphics
  // command queue to make sure our decode fence is signaled to the correct
  // value before decoding can start.
  //
  // NOTE: "out_gpu_decode_fence" is not AddRef'd before being returned, so please
  // don't call Release on it!

  ID3D12Fence * out_gpu_decode_fence;   // In GPU mode, this is the fence you have to call wait on before submitting command_list.
  UINT64 out_gpu_decode_fence_val;      // In GPU mode, this is the fence value you have to wait for before submitting command_list.
} BINKAPICONTEXTD3D12;

// We promise not to request more SRV descriptors than this per Bink, for any D3D12 BinkTextures implementation.
// So if you want to use a simple free list allocator for Bink video descriptors, you can just use this as your
// allocation granularity.
#define BINKTEXTURESD3D12_MAX_SRVS_PER_BINK     128

#define BINKTEXTURESD3D12STRUCTSDEF

#endif

#ifndef BINKCREATESHADERSPIPEANDQUEUEd
#define BINKCREATESHADERSPIPEANDQUEUEd

// Create_Bink_Shaders for some nda GPU implementations allows you to select which async compute pipe and queue ID to run
// decoding jobs on. If not specified, we use our default. (At the time of writing, this is pipe 6, queue 1).
// Fill out this struct and pass a pointer to it as "device" to Create_Bink_shaders.
typedef struct BINKCREATESHADERSPIPEANDQUEUE 
{
  // Which async compute pipe to use for BinkGPU. Ignored for CPU decoding.
  // Corresponds to the "pipeId" in ComputeQueue::initialize.
  U32 pipe_id;

  // Which queue to use for BinkGPU on the given compute pipe. Ignored for CPU decoding.
  // Corresponds to the "queueId" in ComputeQueue::initialize.
  U32 queue_id;
} BINKCREATESHADERSPIPEANDQUEUE;

#endif

#ifndef BINKTEXTURES_METAL_DEF
#define BINKTEXTURES_METAL_DEF

// Create_Bink_Shaders for Metal needs a few more parameters - pass a pointer to this structure.

typedef struct BINKCREATESHADERSMETAL 
{
  void * mtlDevice;
  void * stateDesc[2];          // MTLRenderPipelineDescriptor *
  void * mtlQueue;              // used on GPU-assist Bink Metal only
  U32 render_target_formats[2]; // MTLPixelFormat(s) that we can render to
} BINKCREATESHADERSMETAL;


typedef struct BINKAPICONTEXTMETAL
{
  void * command_buffer;  // id <MTLCommandBuffer> 
  void * command_encoder; // id <MTLRenderCommandEncoder>
  void * frame_texture;   // View.currentDrawable.texture
  U32 format_idx;         // Index into render_target_formats. See BINKCREATESHADERSMETAL
} BINKAPICONTEXTMETAL;

#endif

#ifndef BINKTEXTURES_VULKAN_DEF
#define BINKTEXTURES_VULKAN_DEF
typedef struct BINKCREATESHADERSVULKAN 
{
  void *physical_device;        // VkPhysicalDevice
  void *logical_device;         // VkDevice
  void *queue;                  // VkQueue (needs graphics+compute capabilities where compute is only used for gpu decode)
  U32 render_target_formats[2]; // VkFormat(s) that we can render to
} BINKCREATESHADERSVULKAN;

typedef struct BINKAPICONTEXTVULKAN
{
  void * cmdbuf;  // VkCommandBuffer
  U32 format_idx; // Index into render_target_formats. See BINKCREATESHADERSVULKAN
} BINKAPICONTEXTVULKAN;
#endif

//=============================================================================

// Creates shaders that Bink uses.
//   The value that you pass for device depends on the graphics API:
//     D3D9 -    The d3d9 device (type LPDIRECT3DDEVICE9)
//     D3D11 -   The d3d11 device (type D3D11Device *)
//     D3D12 -   Pointer to a filled-out BINKCREATESHADERSD3D12 struct
//     GL -      Parameter not used, just pass 0
//     PS4 -     Pass 0 for default parameters, or a pointer to a filled-out BINKCREATESHADERSPS4 struct
//     Metal   - Pointer to a filled out BINKCREATESHADERSMETAL struct
BINKSHADERS * Create_Bink_shaders( void * device );

// free our shaders
typedef void Free_Bink_shaders_ft( BINKSHADERS * shaders );

//=============================================================================

// allocate the textures that we'll need - shaders defines the device we use to
//   create them. The user_ptr gets stored in the BINKTEXTURES and also passed
//   to any underlying "alloc" functions we call.
typedef BINKTEXTURES * Create_Bink_textures_ft( BINKSHADERS * shaders, HBINK bink, void *user_ptr );

//=============================================================================

// frees the textures
typedef void Free_Bink_textures_ft( BINKTEXTURES * textures );

//=============================================================================
// Bink texture updating

// Start updating textures for this frame. Call before DoFrame.
typedef void Start_Bink_texture_update_ft( BINKTEXTURES * textures );

// Finish updating textures for this frame. Call after DoFrame is done.
typedef void Finish_Bink_texture_update_ft( BINKTEXTURES * textures );


//=============================================================================

// Bink texture drawing functions

// Draws the textures as a screen-aligned quad.

//   If you pass a zero for "shaders", we'll use the shader used to create the
//       textures (which is normally what you want).
//   The value that you pass for "graphics_context" depends on the graphics API:
//     D3D9 -    Parameter not used, just pass 0
//     D3D11 -   The d3d11 device context (type ID3D11DeviceContext *)
//     D3D12 -   Pointer to a BINKAPICONTEXTD3D12 struct with all fields filled out
//     GL -      Parameter not used, just pass 0
//     PS3 -     The Gcm context (usually, gCellGcmCurrentContext, type CellGcmContextData *)
//     PS4 -     The Gfx context pointer to use (type sce::Gnmx::GfxContext *)
//     Xbox360 - Parameter not used, just pass 0
//     XboxOne - The Xbox device context (type ID3D11DeviceContextX *)
//     NX -      The NX context pointer to use (type nn::gfx::CommandBuffer *)
//     Metal -   Pointer to a BINKAPICONTEXTMETAL struct with all the fields filled out
//     PS5 -     Pointer to the DrawCommandBuffer to use (no StateBuffer or Binder required)
typedef void Draw_Bink_textures_ft( BINKTEXTURES * textures,
                                    BINKSHADERS * shaders,
                                    void * graphics_context );

// Set the position to draw the video within the viewport.
//   If you video doesn't move, you can call this once,
//     otherwise call before each call to Draw_Bink_texture.
//   By default, the video fills the video port.
//
//   0,0, 1,1 is the default which completely fills the viewport
typedef void Set_Bink_draw_position_ft( BINKTEXTURES * textures,
                                        F32 x0, F32 y0, F32 x1, F32 y1 );


//   0,0, 1,0, 0,1 is the default which completely fills the viewport
typedef void Set_Bink_draw_corners_ft( BINKTEXTURES * textures,
                                        F32 Ax, F32 Ay, F32 Bx, F32 By, F32 Cx, F32 Cy );


// Set draw source rect (from 0 to 1, in texturespace, so 0,0, 1,1 is whole texture)
typedef void Set_Bink_source_rect_ft( BINKTEXTURES * textures,
                                      F32 u0, F32 v0, F32 u1, F32 v1 );

// Or these flags with draw_type_and_flags
#define BINKDRAWFLAG_SRGB_DECODE (0x80000000)

// Set the alpha settings for drawing.
//   alpha_value is just a constant blend value for entire video frame
//   draw_type is 0 = normal alpha, 1 = pre-multiplied alpha, 2 = opaque & copy alpha
//   draw_type also contains flags in the high bits:
//     BINKDRAWFLAG_SRGB_DECODE - decode sRGB to linear when rendering the video.
typedef void Set_Bink_alpha_settings_ft( BINKTEXTURES * textures,
                                         F32 alpha_value,
                                         S32 draw_type_and_flags );

// Set the hdr settings for drawing.
//   tonemap specifies whether you want linear output (0), or filmic tonemapped output (1), or ST2084 HDR output (2).
//   exposure is a scaling factor that happens before tonemapping (1.0=normal, <1.0 darken, >1.0 brighten)
//   out_luma specifies the maximum luminance to output when tonemapping
typedef void Set_Bink_hdr_settings_ft( BINKTEXTURES * texutres,
                                       S32 tonemap,
                                       F32 exposure, 
                                       S32 output_nits );


//=============================================================================

// For D3D9 only, these functions are called before and after you reset the D3D9 device
typedef void Before_Reset_Bink_textures_ft( BINKTEXTURES * textures );
typedef S32  After_Reset_Bink_textures_ft( BINKTEXTURES * textures );

//=============================================================================

#ifndef BINKTEXTURESTRUCTS
#define BINKTEXTURESTRUCTS

#define Create_Bink_textures( shader, bink, user_ptr )   (shader)->Create_textures( shader, bink, user_ptr )
#define Free_Bink_shaders( shader )                      (shader)->Free_shaders( shader )

#define Start_Bink_texture_update( textures )            (textures)->Start_texture_update( textures )
#define Finish_Bink_texture_update( textures )           (textures)->Finish_texture_update( textures )
#define Draw_Bink_textures( textures, shaders, gr_ctx )  (textures)->Draw_textures( textures, shaders, gr_ctx )
#define Set_Bink_draw_position( textures, x0,y0, x1,y1 ) (textures)->Set_draw_position( textures, x0,y0, x1,y1 )
#define Set_Bink_draw_corners( textures, Ax,Ay, Bx,By, Cx,Cy ) (textures)->Set_draw_corners( textures, Ax,Ay, Bx,By, Cx,Cy )
#define Set_Bink_source_rect( textures, u0,v0, u1,v1 )   (textures)->Set_source_rect( textures, u0,v0, u1,v1 )
#define Set_Bink_alpha_settings( textures, alpha, draw_type_and_flags ) (textures)->Set_alpha_settings( textures, alpha, draw_type_and_flags )
#define Set_Bink_hdr_settings( textures, tonemap, exposure, out_nits ) (textures)->Set_hdr_settings( textures, tonemap, exposure, out_nits )
#define Before_Reset_Bink_textures( textures )           {if ((textures)->Before_Reset_textures) (textures)->Before_Reset_textures( textures );}
#define After_Reset_Bink_textures( textures )            {if ((textures)->After_Reset_textures) (textures)->After_Reset_textures( textures );}
#define Free_Bink_textures( textures )                   (textures)->Free_textures( textures )

struct BINKSHADERS
{
  Create_Bink_textures_ft * Create_textures;
  Free_Bink_shaders_ft * Free_shaders;

  // you can use this for whatever you want
  UINTa user_data[ 4 ];
};

struct BINKTEXTURES
{
  // wrap around doframe
  Start_Bink_texture_update_ft * Start_texture_update;
  Finish_Bink_texture_update_ft * Finish_texture_update;

  // draw related functions
  Draw_Bink_textures_ft * Draw_textures;
  Set_Bink_draw_position_ft * Set_draw_position;
  Set_Bink_draw_corners_ft * Set_draw_corners;
  Set_Bink_source_rect_ft * Set_source_rect;
  Set_Bink_alpha_settings_ft * Set_alpha_settings;
  Set_Bink_hdr_settings_ft * Set_hdr_settings;

  // D3D9 only (everywhere else is null)
  Before_Reset_Bink_textures_ft * Before_Reset_textures;
  After_Reset_Bink_textures_ft * After_Reset_textures;

  Free_Bink_textures_ft * Free_textures;

  // user pointer specified at Create_Bink_textures time
  void * user_ptr;

  // these are the platform specific texture pointers that you can draw manually with
  void * Ytexture;
  void * cRtexture;
  void * cBtexture;
  void * Atexture;
  void * Htexture;

  // you can use this for whatever you want
  UINTa user_data[ 4 ];
};

#endif
//=============================================================================


// optional clean up, for when including the header multiple times for different APIs
#if defined(BINKTEXTURESCLEANUP)
  #undef Create_Bink_shaders 

  #ifdef BINKD3D9FUNCTIONS
    #undef BINKD3D9FUNCTIONS
  #endif
  #ifdef BINKD3D11FUNCTIONS
    #undef BINKD3D11FUNCTIONS
  #endif
  #ifdef BINKD3D11GPUFUNCTIONS
    #undef BINKD3D11GPUFUNCTIONS
  #endif
  #ifdef BINKD3D12FUNCTIONS
    #undef BINKD3D12FUNCTIONS
  #endif
  #ifdef BINKD3D12GPUFUNCTIONS
    #undef BINKD3D12GPUFUNCTIONS
  #endif
  #ifdef BINKNDAFUNCTIONS
    #undef BINKNDAFUNCTIONS 
  #endif
  #ifdef BINKTEXTURESSUFFIX
    #undef BINKTEXTURESSUFFIX 
  #endif
  #undef BINKTEXTURESH
  #undef BINKTEXTURESCLEANUP
#endif

RADDEFEND

#endif
