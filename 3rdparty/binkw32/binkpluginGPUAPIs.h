// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

#include "egttypes.h"
#include "binkpluginplatform.h"

RADDEFSTART

typedef struct RENDERTARGETVIEW RENDERTARGETVIEW;

typedef int setup_gpu_func( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
typedef void shutdown_gpu_func( void );
typedef void * createtextures_gpu_func( void * bink );
typedef void begindraw_gpu_func( void );
typedef void enddraw_gpu_func( void );
typedef void statepush_gpu_func( void ); // on stateful apis (d3d9, d3d11, gl) - this tries to save the gpu state (kind of a losing battle)
typedef void statepop_gpu_func( void );

typedef void selectscreenrendertarget_func( void * screen_texture_target, S32 width, S32 height, U32 sdr_or_hdr, S32 current_resource_state ); // sdr=0, hdr=1
typedef void selectrendertarget_func( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); // current_res_state < 0 means assume default
typedef void clearrendertarget_func( );

typedef struct BINKPLUGINGPUAPIFUNCS
{
  setup_gpu_func * setup;
  shutdown_gpu_func * shutdown;
  createtextures_gpu_func * createtextures;
  begindraw_gpu_func * begindraw;
  enddraw_gpu_func * enddraw;
  statepush_gpu_func * statepush; 
  statepop_gpu_func * statepop;
  selectscreenrendertarget_func * selectscreenrendertarget;
  selectrendertarget_func * selectrendertarget;
  clearrendertarget_func * clearrendertarget;
} BINKPLUGINGPUAPIFUNCS;

#ifndef BUILDING_FOR_UNREAL_ONLY

#if PLATFORM_HAS_VULKAN
int setup_vulkan( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_vulkan( void );
void * createtextures_vulkan( void * bink );
void begindraw_vulkan( void );
void enddraw_vulkan( void );
void selectscreenrendertarget_vulkan( void * screen_texture_target, S32 width, S32 height, U32 sdr_or_hdr, S32 current_resource_state ); 
void selectrendertarget_vulkan( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_vulkan( );
#define BinkVulkanAPI { setup_vulkan, shutdown_vulkan, createtextures_vulkan, begindraw_vulkan, enddraw_vulkan, 0, 0, selectscreenrendertarget_vulkan ,selectrendertarget_vulkan, clearrendertarget_vulkan }
#else
#define BinkVulkanAPI {0}
#endif

#if PLATFORM_HAS_D3D12
int setup_d3d12( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_d3d12( void );
void * createtextures_d3d12( void * bink );
void begindraw_d3d12( void );
void enddraw_d3d12( void );
void selectscreenrendertarget_d3d12( void * screen_texture_target, S32 width, S32 height, U32 sdr_or_hdr, S32 current_resource_state ); 
void selectrendertarget_d3d12( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_d3d12( );
#define BinkD3D12API { setup_d3d12, shutdown_d3d12, createtextures_d3d12, begindraw_d3d12, enddraw_d3d12, 0, 0, selectscreenrendertarget_d3d12,selectrendertarget_d3d12,clearrendertarget_d3d12 }
#else
#define BinkD3D12API {0}
#endif

#if PLATFORM_HAS_D3D11
int setup_d3d11( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_d3d11( void );
void * createtextures_d3d11( void * bink );
void begindraw_d3d11( void );
void enddraw_d3d11( void );
void selectrendertarget_d3d11( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_d3d11( );
#define BinkD3D11API { setup_d3d11, shutdown_d3d11, createtextures_d3d11, begindraw_d3d11, enddraw_d3d11, begindraw_d3d11, enddraw_d3d11, 0, selectrendertarget_d3d11, clearrendertarget_d3d11 }
#else
#define BinkD3D11API {0}
#endif

#if PLATFORM_HAS_D3D9
int setup_d3d9( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_d3d9( void );
void * createtextures_d3d9( void * bink );
void begindraw_d3d9( void );
void enddraw_d3d9( void );
void selectrendertarget_d3d9( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_d3d9( );
#define BinkD3D9API { setup_d3d9, shutdown_d3d9, createtextures_d3d9, begindraw_d3d9, enddraw_d3d9, begindraw_d3d9, enddraw_d3d9, 0, selectrendertarget_d3d9, clearrendertarget_d3d9 }
#else
#define BinkD3D9API {0}
#endif

#if PLATFORM_HAS_GL
int setup_gl( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_gl( void );
void * createtextures_gl( void * bink );
void begindraw_gl( void );
void enddraw_gl( void );
void selectrendertarget_gl( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_gl( );
#define BinkGLAPI { setup_gl, shutdown_gl, createtextures_gl, begindraw_gl, enddraw_gl, begindraw_gl, enddraw_gl, 0, selectrendertarget_gl, clearrendertarget_gl }
#else
#define BinkGLAPI {0}
#endif

#if PLATFORM_HAS_METAL
int setup_metal( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_metal( void );
void * createtextures_metal( void * bink );
void begindraw_metal( void );
void enddraw_metal( void );
void selectscreenrendertarget_metal( void * screen_texture_target, S32 width, S32 height, U32 sdr_or_hdr, S32 current_resource_state ); 
void selectrendertarget_metal( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_metal( );
#define BinkMetalAPI { setup_metal, shutdown_metal, createtextures_metal, begindraw_metal, enddraw_metal, 0, 0, selectscreenrendertarget_metal, selectrendertarget_metal, clearrendertarget_metal }
#else
#define BinkMetalAPI {0}
#endif

#if PLATFORM_HAS_NDA
int setup_nda( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_nda( void );
void * createtextures_nda( void * bink );
void begindraw_nda( void );
void enddraw_nda( void );
void selectrendertarget_nda( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_nda( );
#define BinkNDAAPI { setup_nda, shutdown_nda, createtextures_nda, begindraw_nda, enddraw_nda, 0,0, selectscreenrendertarget_nda, selectrendertarget_nda, clearrendertarget_nda }
#else
#define BinkNDAAPI {0}
#endif

#else

#if PLATFORM_HAS_RHI
int setup_rhi( void * device, BINKPLUGININITINFO * info, S32 gpu_assisted, void ** context );
void shutdown_rhi( void );
void * createtextures_rhi( void * bink );
void begindraw_rhi( void );
void enddraw_rhi( void );
void selectscreenrendertarget_rhi( void * screen_texture_target, S32 width, S32 height, U32 sdr_or_hdr, S32 current_resource_state ); 
void selectrendertarget_rhi( void * texture_target, S32 width, S32 height, S32 do_clear, U32 sdr_or_hdr, S32 current_resource_state ); 
void clearrendertarget_rhi( );
#if PLATFORM_HAS_NDA
#define BinkRHIAPI { setup_rhi, shutdown_rhi, createtextures_rhi, begindraw_rhi, enddraw_rhi, 0,0, selectscreenrendertarget_rhi, selectrendertarget_rhi, clearrendertarget_rhi }
#else
#define BinkRHIAPI { setup_rhi, shutdown_rhi, createtextures_rhi, begindraw_rhi, enddraw_rhi, begindraw_rhi, enddraw_rhi, selectscreenrendertarget_rhi, selectrendertarget_rhi, clearrendertarget_rhi }
#endif
#else
#define BinkRHIAPI {0}
#endif

#endif

RADDEFEND
