// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

// Copyright 2008-2016 RAD Game Tools

#include "rrCpu.h"
#include "rrAtomics.h"

#if !defined(DLL_FOR_GENERIC_OS)

static U32 maxcpus = 1;
static U32 htcpus = 0;  

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <intrin.h>

typedef BOOL (WINAPI *LPFN_GLPI)(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
    PDWORD);

#define rRelationProcessorPackage 3

static char order[ RR_MAX_CPUS ];

#if defined(__RADX86__) && !defined(__RADX64__)

static LONG WINAPI handle_badcpu( struct _EXCEPTION_POINTERS * info )
{
  if ( info->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION )
  {
    #ifdef __RADX64__
    info->ContextRecord->Rip += 2; // skips a cpuid
    #else
    info->ContextRecord->Eip += 2; // skips a cpuid
    #endif
    return EXCEPTION_CONTINUE_EXECUTION;
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

#endif

static S32 readcpu()
{
  static S32 checked=0;

  if (!checked)
  {
    SYSTEM_INFO info;
    HANDLE kernel32Handle;
    LPFN_GLPI glpi;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    DWORD len;
      
    UINTa htarr[ RR_MAX_CPUS ];
    UINTa pk[ RR_MAX_CPUS ];
    UINTa * h = htarr;
    UINTa * p = pk;

    checked=1;

    info.dwNumberOfProcessors=0;
    GetSystemInfo(&info);
    if (info.dwNumberOfProcessors)
    {
      maxcpus=info.dwNumberOfProcessors;
    }
    if ( maxcpus > RR_MAX_CPUS )
      maxcpus = RR_MAX_CPUS;

    kernel32Handle = GetModuleHandle(_T("KERNEL32") );
    if (kernel32Handle != 0)
      glpi = (LPFN_GLPI) GetProcAddress( (HMODULE)kernel32Handle, "GetLogicalProcessorInformation");
    else
      glpi = 0;
  
    if ( glpi == 0 )
    {
      U32 i;
      U32 ht;
  
      // setup default order without any info (just make a reasonable guess)
      setup_default:
      
      ht = 0;

#ifdef __RAD64__
      { 
        int registers[4];
        __cpuid(registers,1);
        ht = ( registers[3] & 0x10000000 ) ? 1 : 0;
      }
#else  
      {
        LPTOP_LEVEL_EXCEPTION_FILTER flt;

        flt = SetUnhandledExceptionFilter( &handle_badcpu );
        ht = 0;

        _asm
        {
          mov eax, 1
          cpuid
          mov [ht],edx
        }
 
        SetUnhandledExceptionFilter( flt );
      }
      ht = ( ht & 0x10000000 ) ? 1 : 0;
#endif
  
      if ( ht )
      {
        U32 c, m;
        char * o;
        
        m = maxcpus;
        o = order;
        
        htcpus = m / 2; 
        
        if ( m & 1 )
          *o++ = (char) --m;
          
        // do entries for the non-HT cores
        c = m >> 1;
        if ( ( m & 3 ) == 0 )
        {
          // if divisible by four, treat as 2 sock*2 HT*n
          U32 q = c >> 1;
          for( i = 0; i < q ; i++ )
          {
            o[ i ] = (char) ( ( i << 1 ) + 1 );
            o[ i + q ] = (char) ( i << 1 );
          }
        }     
        else
        {
          // otherwise, just c cores and c HT
          for( i = 0; i < c ; i++ )
            o[ i ] = (char) ( i + 1 );
          o[ c - 1 ] = 0;
        }
  
        // now do the HT cores
        for( i = 0 ; i < c ; i++ )
          o[ c + i ] = (char) ( c + o[ c - i - 1 ] );
      }
      else
      {
        for( i = 0 ; i < maxcpus ; i++ )
          order[ i ] = (char) ( i + 1 );
        order[ maxcpus - 1 ] = 0;  
      }
   
  
      return 1;
    }  
  
    len = 0;
    glpi( 0, &len );
    if ( len == 0 )
      goto setup_default;
      
    buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) _alloca( len );
    if ( buffer == 0 )
      goto setup_default;
  
    if ( !glpi( buffer, &len ) )
      goto setup_default;
  
    while ( len >= sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) )
    {
      switch ( buffer->Relationship ) 
      {
        case RelationProcessorCore:
          // this gives us a bitmask of hyperthread related cpus
          if ( (U32) ( h - htarr ) < maxcpus )
            *h++ = buffer->ProcessorMask;
          break;
  
        case rRelationProcessorPackage:
          // this gives us a bitmask of cpus that share a physical package (a socket)
          if ( (U32) ( p - pk ) < maxcpus )
            *p++ = buffer->ProcessorMask;
          break;
        default: break;
      }
      ++buffer;
      len -= sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    }
    
    // now build the order of cpus
    {
      char * os = order;
      char * oe = order + maxcpus - 1;
      
      // lay down the packages of CPUs in reverse order
      SINTa pr = ( p - pk ) - 1;
      if ( ( pr < 0 ) && ( ( h - htarr ) == 0 ) )
        goto setup_default;
      
      // if we never found a package info, make one by oring all the HT
      if ( pr < 0 )
      {
        UINTa * hc;
        pr = 1;
        pk[ 0 ] = 0;
        for( hc = htarr ; hc < h ; hc++ )
          pk[ 0 ] |= hc[ 0 ];
      }
      
      htcpus = 0;
      
      do
      {
        UINTa b;
  
        // now find each core within the package
        for( b = 0 ; b < (sizeof(UINTa)*8) ; b++ )
        {
          // are there any left?
          if ( pk[ pr ] == 0 )
            break;
            
          // do we have one?
          if ( pk[ pr ] & ( ( (UINTa) 1 ) << b ) )
          {
            UINTa * hc;
            
            // yup, record it
            if ( os <= oe )
              *os++ = (char) b;
            pk[ pr ] &= ~( ( (UINTa) 1 ) << b );
            
            // now, for this core, search the hyperthread list to see
            //   if any cores alias with this one
            for( hc = htarr ; hc < h ; hc++ )
            {
              if ( hc[ 0 ] & ( ( (UINTa) 1 ) << b ) )
              {
                UINTa hb;
                      
                hc[ 0 ] &= ~( ( (UINTa) 1 ) << b );
        
                // search from the top down, so that we lay down
                //   the HT cores in reverse order at the *end*
                //   of the cpu order list
                for( hb = ((sizeof(UINTa)*8)-1) ; hb > b ; hb-- )
                {
                  if ( hc[ 0 ] == 0 )
                    break;
                
                  if ( hc[ 0 ] & ( ( (UINTa) 1 ) << hb ) )
                  {
                    // found one, so record it at the end
                    if ( os <= oe )
                      *oe-- = (char) hb;
                    ++htcpus;

                    // clear this cpu from both the HT list and the package list
                    hc[ 0 ] &= ~( ( (UINTa) 1 ) << hb );
                    pk[ pr ] &= ~( ( (UINTa) 1 ) << hb );
                  }
                }
                break;
              }
            }
          }
        }
        --pr;
      } while ( pr >= 0 );

    }          
    if ( htcpus > RR_MAX_CPUS )
      htcpus = RR_MAX_CPUS;
    return 1;
  }
  
  return 0;
}


RADDEFFUNC S32 RADLINK rrGetTotalCPUs( void )
{
  readcpu();
  return maxcpus;
}


RADDEFFUNC S32 RADLINK rrGetTotalSlowerCPUs( void )
{
  readcpu();
  return htcpus;
}


RADDEFFUNC S32 RADLINK rrGetCPUCoreByPrecedence( S32 index )
{
  readcpu();
  return order[ index % maxcpus ];
}

RADDEFFUNC S32 RADLINK rrGetSlowerCPUCoreByPrecedence( S32 index )
{
  readcpu();
  if ( htcpus )
    return order[ ( index + ( maxcpus - htcpus ) ) % maxcpus ];
  else
  {
    index = index % maxcpus;
    return order[ maxcpus - index - 1 ];
  }  
}  

// CB : rrThreadGetSpinCount is a good amount to spin
//  in particular it is 1 if not multi-core
RADDEFFUNC S32 RADLINK rrGetSpinCount( void )
{
  static volatile S32 s_spinCount = -1;
  if ( s_spinCount == -1 )
  {
    S32 cpus;
    
    // !! be careful ! rrGetTotalCPUs() might use rrThread primitives which call me !
    // first store something other than -1 so I won't recurse :
    rrAtomicStoreRelease32((U32 *)&s_spinCount,1);
      
    cpus = rrGetTotalCPUs();
    
    if ( cpus == 1 )
    {
      rrAtomicStoreRelease32((U32 *)&s_spinCount,1);
    }
    else
    {
      // CB : RAD_DEFAULT_SPIN_COUNT should be a static int, not a #define
      //  it should be set to 1 on single core machines
      // #define RAD_DEFAULT_SPIN_COUNT 500
      rrAtomicStoreRelease32((U32 *)&s_spinCount,500);
    }
  }
  return s_spinCount;
}

#if defined(__RADX64__)

rrbool rrCPU_HasSSE2( void )
{
  // x64 has SSE2
  return 1;
}

#elif defined(__RADX86__)

// These are the bit flags that get set on calling cpuid
// with register eax set to 1
#define _MMX_FEATURE_BIT        0x00800000
#define _SSE_FEATURE_BIT        0x02000000
#define _SSE2_FEATURE_BIT       0x04000000

rrbool rrCPU_HasSSE2( void )
{
  static U32 checked = 0;
  if ( checked == 0 )
  {
    LPTOP_LEVEL_EXCEPTION_FILTER flt;
    DWORD dwStandard = 0;
    DWORD dwFeature = 0;

    flt = SetUnhandledExceptionFilter( &handle_badcpu );

    __asm 
		{
        push ebx
        push ecx
        push edx

        // get the Standard bits
        mov eax, 1
        cpuid
        mov dwStandard, eax
        mov dwFeature, edx

        pop ecx
        pop ebx
        pop edx
		}

    SetUnhandledExceptionFilter( flt );
    checked = (dwFeature & _SSE2_FEATURE_BIT) ? 2 : 1;
  }

  return ( checked == 2 ) ? 1 : 0;
}

#endif

#else

S32 RADLINK rrGetTotalCPUs( void )
{
  // for generic os, just return 4 cores
  return 4;
}

RADDEFFUNC S32 RADLINK rrGetSpinCount( void )
{
  return 300;
}

RADDEFFUNC S32 RADLINK rrGetCPUCoreByPrecedence( S32 index )
{
  return index;
}
#endif
