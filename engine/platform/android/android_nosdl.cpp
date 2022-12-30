/*
android_nosdl.c - android backend
Copyright (C) 2016 mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "common.h"
#if XASH_VIDEO == VIDEO_ANDROID
#include <pthread.h>
#include "input.h"
#include "joyinput.h"
#include "touch.h"
#include "client.h"
#include "gl_local.h" // glConfig, glState, convars
#include <android/log.h>
#include <jni.h>
#ifdef XASH_NANOGL
#include <GL/nanogl.h>
#endif
#ifdef XASH_GL4ES
#include "gl4esinit.h"
#include "gl4eshint.h"
#endif
#include <EGL/egl.h> // nanogl
#include <errno.h>
#include <android/asset_manager_jni.h>
convar_t *android_sleep;

#ifdef XASH_IMGUI
#include "imgui_impl_xash.h"
#endif

#include "tier1/strtools.h"

#ifndef JAVA_EXPORT
#define JAVA_EXPORT // a1ba: workaround for my IDE, where Java files are not included
#endif


static const int s_android_scantokey[] =
{
	0,				K_LEFTARROW,	K_RIGHTARROW,	K_AUX26,		K_ESCAPE,		// 0
	K_AUX26,		K_AUX25,		'0',			'1',			'2',			// 5
	'3',			'4',			'5',			'6',			'7',			// 10
	'8',			'9',			'*',			'#',			K_UPARROW,		// 15
	K_DOWNARROW,	K_LEFTARROW,	K_RIGHTARROW,	K_ENTER,		K_AUX32,		// 20
	K_AUX31,		K_AUX29,		K_AUX28,		K_AUX27,		'a',			// 25
	'b',			'c',			'd',			'e',			'f',			// 30
	'g',			'h',			'i',			'j',			'k',			// 35
	'l',			'm',			'n',			'o',			'p',			// 40
	'q',			'r',			's',			't',			'u',			// 45
	'v',			'w',			'x',			'y',			'z',			// 50
	',',			'.',			K_ALT,			K_ALT,			K_SHIFT,		// 55
	K_SHIFT,		K_TAB,			K_SPACE,		0,				0,				// 60
	0,				K_ENTER,		K_BACKSPACE,	'`',			'-',			// 65
	'=',			'[',			']',			'\\',			';',			// 70
	'\'',			'/',			'@',			K_KP_NUMLOCK,	0,				// 75
	0,				'+',			'`',			0,				0,				// 80
	0,				0,				0,				0,				0,				// 85
	0,				0,				K_PGUP,			K_PGDN,			0,				// 90
	0,				K_AUX1,			K_AUX2,			K_AUX14,		K_AUX3,			// 95
	K_AUX4,			K_AUX15,		K_AUX6,			K_AUX7,			K_JOY1,			// 100
	K_JOY2,			K_AUX10,		K_AUX11,		K_ESCAPE,		K_ESCAPE,		// 105
	0,				K_ESCAPE,		K_DEL,			K_CTRL,			K_CTRL,			// 110
	K_CAPSLOCK,		0,				0,				0,				0,				// 115
	0,				K_PAUSE,		K_HOME,			K_END,			K_INS,			// 120
	0,				0,				0,				0,				0,				// 125
	0,				K_F1,			K_F2,			K_F3,			K_F4,			// 130
	K_F5,			K_F6,			K_F7,			K_F8,			K_F9,			// 135
	K_F10,			K_F11,			K_F12,			K_KP_NUMLOCK,		K_KP_INS,			// 140
	K_KP_END,		K_KP_DOWNARROW,	K_KP_PGDN,		K_KP_LEFTARROW,	K_KP_5,			// 145
	K_KP_RIGHTARROW,K_KP_HOME,		K_KP_UPARROW,	K_KP_PGUP,		K_KP_SLASH,		// 150
	0,				K_KP_MINUS,		K_KP_PLUS,		K_KP_DEL,		',',			// 155
	K_KP_ENTER,		'=',			'(',			')'
};

#define ANDROID_MAX_EVENTS 64
#define MAX_FINGERS 10

typedef enum event_type
{
	event_touch_down,
	event_touch_up,
	event_touch_move,
	event_key_down,
	event_key_up,
	event_set_pause,
	event_resize,
	event_joyhat,
	event_joyball,
	event_joybutton,
	event_joyaxis,
	event_joyadd,
	event_joyremove,
	event_onpause,
	event_ondestroy,
	event_onresume,
	event_onfocuschange
} eventtype_t;

typedef struct touchevent_s
{
	float x;
	float y;
	float dx;
	float dy;
} touchevent_t;

typedef struct joyball_s
{
	short xrel;
	short yrel;
	byte ball;
} joyball_t;

typedef struct joyhat_s
{
	byte hat;
	byte key;
} joyhat_t;

typedef struct joyaxis_s
{
	short val;
	byte axis;
} joyaxis_t;

typedef struct joybutton_s
{
	int down;
	byte button;
} joybutton_t;

typedef struct keyevent_s
{
	int code;
} keyevent_t;

typedef struct event_s
{
	eventtype_t type;
	int arg;
	union
	{
		touchevent_t touch;
		joyhat_t hat;
		joyball_t ball;
		joyaxis_t axis;
		joybutton_t button;
		keyevent_t key;
	};
} event_t;

typedef struct finger_s
{
	float x, y;
	qboolean down;
} finger_t;

static struct {
	pthread_mutex_t mutex; // this mutex is locked while not running frame, used for events synchronization
	pthread_mutex_t framemutex; // this mutex is locked while engine is running and unlocked while it reading events, used for pause in background.
	event_t queue[ANDROID_MAX_EVENTS];
	volatile int count;
	finger_t fingers[MAX_FINGERS];
	char inputtext[256];
	float mousex, mousey;
} events = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };

static struct jnimethods_s
{
	jclass actcls;
	JavaVM *vm;
	JNIEnv *env;
	jmethodID swapBuffers;
	jmethodID toggleEGL;
	jmethodID enableTextInput;
	jmethodID vibrate;
	jmethodID messageBox;
	jmethodID createGLContext;
	jmethodID getGLAttribute;
	jmethodID deleteGLContext;
	jmethodID notify;
	jmethodID setTitle;
	jmethodID setIcon;
	jmethodID getAndroidId;
	jmethodID saveID;
	jmethodID loadID;
	jmethodID showMouse;
	jmethodID shellExecute;
	jmethodID getAssetManager;
	jmethodID getDisplayDPI;
	jmethodID logToBugly;
	int width, height;
} jni;

static struct nativeegl_s
{
	qboolean valid;
	EGLDisplay dpy;
	EGLSurface surface;
} negl;

static struct jnimouse_s
{
	float x, y;
} jnimouse;

#define Android_Lock() pthread_mutex_lock(&events.mutex);
#define Android_Unlock() pthread_mutex_unlock(&events.mutex);
#define Android_PushEvent() Android_Unlock()

typedef void(*pfnChangeGame)(const char *progname);
void Android_UpdateSurface( void );
int EXPORT Host_Main( int argc, const char **argv, const char *progname, int bChangeGame, pfnChangeGame func );
void VID_SetMode( void );
void S_Activate( qboolean active );

/*
========================
Android_AllocEvent

Lock event queue and return pointer to next event.
Caller must do Android_PushEvent() to unlock queue after setting parameters.
========================
*/
event_t *Android_AllocEvent()
{
	Android_Lock();
	if( events.count == ANDROID_MAX_EVENTS )
	{
		events.count--; //override last event
		__android_log_print( ANDROID_LOG_ERROR, "Xash", "Too many events!!!" );
	}
	return &events.queue[ events.count++ ];
}

/*
========================
Android_RunEvents

Execute all events from queue
========================
*/
void Android_RunEvents()
{
	int i;

	// enter events read
	Android_Lock();
	pthread_mutex_unlock( &events.framemutex );

	for( i = 0; i < events.count; i++ )
	{
		switch( events.queue[i].type )
		{
		case event_touch_down:
		case event_touch_up:
		case event_touch_move:
			IN_TouchEvent( (touchEventType)events.queue[i].type, events.queue[i].arg,
						   events.queue[i].touch.x, events.queue[i].touch.y,
						   events.queue[i].touch.dx, events.queue[i].touch.dy, 1.0f );
			break;

		case event_key_down:
			Key_Event( events.queue[i].arg, true );

			if( events.queue[i].arg == K_AUX31 || events.queue[i].arg == K_AUX29 )
			{
				host.force_draw_version = true;
				host.force_draw_version_time = host.realtime + FORCE_DRAW_VERSION_TIME;
			}
			break;
		case event_key_up:
			Key_Event( events.queue[i].arg, false );

			if( events.queue[i].arg == K_AUX31 || events.queue[i].arg == K_AUX29 )
			{
				host.force_draw_version = true;
				host.force_draw_version_time = host.realtime + FORCE_DRAW_VERSION_TIME;
			}
			break;

		case event_set_pause:
			// destroy EGL surface when hiding application
			if( !events.queue[i].arg )
			{
				host.state = HOST_FRAME;
				S_Activate( true );
				jni.env->CallStaticVoidMethod( jni.actcls, jni.toggleEGL, 1 );
				Android_UpdateSurface();
				Android_SwapInterval( gl_swapInterval->integer );
				host.force_draw_version = true;
				host.force_draw_version_time = host.realtime + FORCE_DRAW_VERSION_TIME;
			}
			if( events.queue[i].arg )
			{
				host.state = HOST_NOFOCUS;
				S_Activate( false );
				jni.env->CallStaticVoidMethod( jni.actcls, jni.toggleEGL, 0 );
				negl.valid = false;
			}
			break;

		case event_resize:
			// reinitialize EGL and change engine screen size
			if( host.state == HOST_NORMAL && ( scr_width->integer != jni.width || scr_height->integer != jni.height ) )
			{
				jni.env->CallStaticVoidMethod( jni.actcls, jni.toggleEGL, 0 );
				jni.env->CallStaticVoidMethod( jni.actcls, jni.toggleEGL, 1 );
				Android_UpdateSurface();
				Android_SwapInterval( gl_swapInterval->integer );
				VID_SetMode();
			}
			break;
		case event_joyadd:
			Joy_AddEvent( events.queue[i].arg );
			break;
		case event_joyremove:
			Joy_RemoveEvent( events.queue[i].arg );
			break;
		case event_joyball:
			if( !joy_found->integer )
				Joy_AddEvent( 0 );
			Joy_BallMotionEvent( events.queue[i].arg, events.queue[i].ball.ball,
								 events.queue[i].ball.xrel, events.queue[i].ball.yrel );
			break;
		case event_joyhat:
			if( !joy_found->integer )
				Joy_AddEvent( 0 );
			Joy_HatMotionEvent( events.queue[i].arg, events.queue[i].hat.hat, events.queue[i].hat.key );
			break;
		case event_joyaxis:
			if( !joy_found->integer )
				Joy_AddEvent( 0 );
			Joy_AxisMotionEvent( events.queue[i].arg, events.queue[i].axis.axis, events.queue[i].axis.val );
			break;
		case event_joybutton:
			if( !joy_found->integer )
				Joy_AddEvent( 0 );
			Joy_ButtonEvent( events.queue[i].arg, events.queue[i].button.button, (byte)events.queue[i].button.down );
			break;
		case event_ondestroy:
			//host.skip_configs = true; // skip config save, because engine may be killed during config save
			Sys_Quit();
			jni.env->CallStaticVoidMethod( jni.actcls, jni.notify );
			break;
		case event_onpause:
#ifdef PARANOID_CONFIG_SAVE
			switch( host.state )
			{
			case HOST_INIT:
			case HOST_CRASHED:
			case HOST_ERR_FATAL:
				MsgDev( D_WARN, "Abnormal host state during onPause (%d), skipping config save!\n", host.state );
				break;
			default:
				// restore all latched cheat cvars
				Cvar_SetCheatState( true );
				Host_WriteConfig();
			}
#endif
			// disable sound during call/screen-off
			S_Activate( false );
			host.state = HOST_NOFOCUS;
			// stop blocking UI thread
			jni.env->CallStaticVoidMethod( jni.actcls, jni.notify );

			break;
		case event_onresume:
			// re-enable sound after onPause
			host.state = HOST_FRAME;
			S_Activate( true );
			host.force_draw_version = true;
			host.force_draw_version_time = host.realtime + FORCE_DRAW_VERSION_TIME;
			break;
		case event_onfocuschange:
			host.force_draw_version = true;
			host.force_draw_version_time = host.realtime + FORCE_DRAW_VERSION_TIME;
			break;
		}
	}

	events.count = 0; // no more events

#ifdef XASH_IMGUI
	if(!ImGui_ImplGL_CharCallbackUTF( events.inputtext )) {
#endif

	// text input handled separately to allow unicode symbols
	for( i = 0; events.inputtext[i]; i++ )
	{
		int ch;

		// if engine does not use utf-8, we need to convert it to preferred encoding
		if( !Q_stricmp( cl_charset->string, "utf-8" ) )
			ch = (unsigned char)events.inputtext[i];
		else
			ch = Con_UtfProcessCharForce( (unsigned char)events.inputtext[i] );

		if( !ch ) // utf-8
			continue;

		// some keyboards may send enter as text
		if( ch == '\n' )
		{
			Key_Event( K_ENTER, true );
			Key_Event( K_ENTER, false );
			continue;
		}

		// otherwise just push it by char, text render will decode unicode strings
		CL_CharEvent( ch );
	}

#ifdef XASH_IMGUI
	}
#endif

	events.inputtext[0] = 0; // no more text

	jnimouse.x += events.mousex;
	events.mousex = 0;
	jnimouse.y += events.mousey;
	events.mousey = 0;

	//end events read
	Android_Unlock();
	pthread_mutex_lock( &events.framemutex );
}

/*
=====================================================
JNI callbacks

On application start, setenv and onNativeResize called from
ui thread to set up engine configuration
nativeInit called directly from engine thread and will not return until exit.
These functions may be called from other threads at any time:
nativeKey
nativeTouch
onNativeResize
nativeString
nativeSetPause
=====================================================
*/
#define DECLARE_JNI_INTERFACE( ret, name, ... ) \
	extern "C" JNIEXPORT ret JNICALL Java_in_celest_xash3d_XashActivity_##name( JNIEnv *env, jclass clazz, __VA_ARGS__ )
#define DECLARE_JNI_INTERFACE_VOID( ret, name ) \
	extern "C" JNIEXPORT ret JNICALL Java_in_celest_xash3d_XashActivity_##name( JNIEnv *env, jclass clazz )
#include <sys/prctl.h>

DECLARE_JNI_INTERFACE( int, nativeInit, jobjectArray array )
{
	int i;
	int argc;
	int status;
	/* Prepare the arguments. */

	int len = env->GetArrayLength(array);
	char* argv[1 + len + 1];
	argc = 0;
	argv[argc++] = strdup("app_process");
	for (i = 0; i < len; ++i) {
		const char* utf;
		char* arg = NULL;
		jstring string = (jstring)env->GetObjectArrayElement(array, i);
		if (string) {
			utf = env->GetStringUTFChars(string, 0);
			if (utf) {
				arg = strdup(utf);
				env->ReleaseStringUTFChars(string, utf);
			}
			env->DeleteLocalRef(string);
		}
		if (!arg) {
			arg = strdup("");
		}
		argv[argc++] = arg;
	}
	argv[argc] = NULL;
	prctl(PR_SET_DUMPABLE, 1);

	/* Init callbacks. */

	jni.env = env;
	jni.actcls = env->FindClass("in/celest/xash3d/XashActivity");
	jni.swapBuffers = env->GetStaticMethodID(jni.actcls, "swapBuffers", "()V");
	jni.toggleEGL = env->GetStaticMethodID(jni.actcls, "toggleEGL", "(I)V");
	jni.enableTextInput = env->GetStaticMethodID(jni.actcls, "showKeyboard", "(I)V");
	jni.vibrate = env->GetStaticMethodID(jni.actcls, "vibrate", "(I)V" );
	jni.messageBox = env->GetStaticMethodID(jni.actcls, "messageBox", "(Ljava/lang/String;Ljava/lang/String;)V");
	jni.createGLContext = env->GetStaticMethodID(jni.actcls, "createGLContext", "(II)Z");
	jni.getGLAttribute = env->GetStaticMethodID(jni.actcls, "getGLAttribute", "(I)I");
	jni.deleteGLContext = env->GetStaticMethodID(jni.actcls, "deleteGLContext", "()Z");
	jni.notify = env->GetStaticMethodID(jni.actcls, "engineThreadNotify", "()V");
	jni.setTitle = env->GetStaticMethodID(jni.actcls, "setTitle", "(Ljava/lang/String;)V");
	jni.setIcon = env->GetStaticMethodID(jni.actcls, "setIcon", "(Ljava/lang/String;)V");
	jni.getAndroidId = env->GetStaticMethodID(jni.actcls, "getAndroidID", "()Ljava/lang/String;");
	jni.saveID = env->GetStaticMethodID(jni.actcls, "saveID", "(Ljava/lang/String;)V");
	jni.loadID = env->GetStaticMethodID(jni.actcls, "loadID", "()Ljava/lang/String;");
	jni.showMouse = env->GetStaticMethodID(jni.actcls, "showMouse", "(I)V");
	jni.shellExecute = env->GetStaticMethodID(jni.actcls, "shellExecute", "(Ljava/lang/String;)V");
	jni.getAssetManager = env->GetStaticMethodID(jni.actcls, "getAssetManager", "()Landroid/content/res/AssetManager;");
	jni.getDisplayDPI = env->GetStaticMethodID(jni.actcls, "getDisplayDPI", "()F");
	jni.logToBugly = env->GetStaticMethodID(jni.actcls, "logToBugly", "(Ljava/lang/String;)V");

	//nanoGL_Init();
	/* Run the application. */

	status = Host_Main( argc, (const char**)argv, getenv("XASH3D_GAMEDIR"), false, NULL );

	/* Release the arguments. */

	for (i = 0; i < argc; ++i)
		free(argv[i]);

	return status;
}

DECLARE_JNI_INTERFACE( void, onNativeResize, jint width, jint height )
{
	event_t *event;

	if( !width || !height )
		return;

	jni.width=width, jni.height=height;

	// alloc update event to change screen size
	event = Android_AllocEvent();
	event->type = event_resize;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE_VOID( void, nativeQuit )
{
}

DECLARE_JNI_INTERFACE( void, nativeSetPause, jint pause )
{
	event_t *event = Android_AllocEvent();
	event->type = event_set_pause;
	event->arg = pause;
	Android_PushEvent();

	// if pause enabled, hold engine by locking frame mutex.
	// Engine will stop after event reading and will not continue untill unlock
	if( android_sleep && android_sleep->value )
	{
		if( pause )
			pthread_mutex_lock( &events.framemutex );
		else
			pthread_mutex_unlock( &events.framemutex );
	}
}

DECLARE_JNI_INTERFACE_VOID( void, nativeUnPause )
{
	// UnPause engine before sending critical events
	if( android_sleep && android_sleep->integer )
			pthread_mutex_unlock( &events.framemutex );
}

DECLARE_JNI_INTERFACE( void, nativeKey, jint down, jint code )
{
	event_t *event;

	if( code < 0 )
	{
		event = Android_AllocEvent();
		event->arg = (-code) & 255;
		event->type = down?event_key_down:event_key_up;
		Android_PushEvent();
	}
	else
	{
		if( code >= ( sizeof( s_android_scantokey ) / sizeof( s_android_scantokey[0] ) ) )
			return;

		event = Android_AllocEvent();
		event->type = down?event_key_down:event_key_up;
		event->arg = s_android_scantokey[code];
		Android_PushEvent();
	}
}

DECLARE_JNI_INTERFACE( void, nativeString, jstring string )
{
	char* str = (char *) env->GetStringUTFChars(string, NULL);

	Android_Lock();
	strncat( events.inputtext, str, 256 );
	Android_Unlock();

	env->ReleaseStringUTFChars(string, str);
}

#ifdef SOFTFP_LINK
DECLARE_JNI_INTERFACE( void, nativeTouch, jint finger, jint action, jfloat x, jfloat y ) __attribute__((pcs("aapcs")));
#endif
DECLARE_JNI_INTERFACE( void, nativeTouch, jint finger, jint action, jfloat x, jfloat y )
{
	float dx, dy;
	event_t *event;

	// if something wrong with android event
	if( finger > MAX_FINGERS )
		return;

	// not touch action?
	if( !( action >=0 && action <= 2 ) )
		return;

	// 0.0f .. 1.0f
	x /= jni.width;
	y /= jni.height;

	if( action )
		dx = x - events.fingers[finger].x, dy = y - events.fingers[finger].y;
	else
		dx = dy = 0.0f;
	events.fingers[finger].x = x, events.fingers[finger].y = y;

	// check if we should skip some events
	if( ( action == 2 ) && ( !dx && !dy ) )
		return;

	if( ( action == 0 ) && events.fingers[finger].down )
		return;

	if( ( action == 1 ) && !events.fingers[finger].down )
		return;

	if( action == 2 && !events.fingers[finger].down )
			action = 0;

	if( action == 0 )
		events.fingers[finger].down = true;
	else if( action == 1 )
		events.fingers[finger].down = false;

	event = Android_AllocEvent();
	event->arg = finger;
	event->type = (event_type)action;
	event->touch.x = x;
	event->touch.y = y;
	event->touch.dx = dx;
	event->touch.dy = dy;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE( void, nativeBall, jint id, jbyte ball, jshort xrel, jshort yrel )
{
	event_t *event = Android_AllocEvent();

	event->type = event_joyball;
	event->arg = id;
	event->ball.ball = ball;
	event->ball.xrel = xrel;
	event->ball.yrel = yrel;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE( void, nativeHat, jint id, jbyte hat, jbyte key, jboolean down )
{
	static byte engineKeys;

	if( !key )
		engineKeys = 0; // centered;

	if( down )
		engineKeys |= key;
	else
		engineKeys &= ~key;

	event_t *event = Android_AllocEvent();
	event->type = event_joyhat;
	event->arg = id;
	event->hat.hat = hat;
	event->hat.key = engineKeys;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE( void, nativeAxis, jint id, jbyte axis, jshort val )
{
	event_t *event = Android_AllocEvent();
	event->type = event_joyaxis;
	event->arg = id;
	event->axis.axis = axis;
	event->axis.val = val;

	__android_log_print(ANDROID_LOG_VERBOSE, "Xash", "axis %i %i", axis, val );
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE( void, nativeJoyButton, jint id, jbyte button, jboolean down )
{
	event_t *event = Android_AllocEvent();
	event->type = event_joybutton;
	event->arg = id;
	event->button.button = button;
	event->button.down = down;
	__android_log_print(ANDROID_LOG_VERBOSE, "Xash", "button %i", button );
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE( void, nativeJoyAdd, jint id )
{
	event_t *event = Android_AllocEvent();
	event->type = event_joyadd;
	event->arg = id;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE( void, nativeJoyDel, jint id )
{
	event_t *event = Android_AllocEvent();
	event->type = event_joyremove;
	event->arg = id;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE_VOID( void, nativeOnResume )
{
	event_t *event = Android_AllocEvent();
	event->type = event_onresume;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE_VOID( void, nativeOnFocusChange )
{
	event_t *event = Android_AllocEvent();
	event->type = event_onfocuschange;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE_VOID( void, nativeOnPause )
{
	event_t *event = Android_AllocEvent();
	event->type = event_onpause;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE_VOID( void, nativeOnDestroy )
{
	event_t *event = Android_AllocEvent();
	event->type = event_ondestroy;
	Android_PushEvent();
}

DECLARE_JNI_INTERFACE( int, setenv, jstring key, jstring value, jboolean overwrite )
{
	char* k = (char *) env->GetStringUTFChars(key, NULL);
	char* v = (char *) env->GetStringUTFChars(value, NULL);
	int err = setenv(k, v, overwrite);
	env->ReleaseStringUTFChars(key, k);
	env->ReleaseStringUTFChars(value, v);
	return err;
}


DECLARE_JNI_INTERFACE( void, nativeMouseMove, jfloat x, jfloat y )
{
	Android_Lock();
	events.mousex += x;
	events.mousey += y;
	Android_Unlock();
}

DECLARE_JNI_INTERFACE( int, nativeTestWritePermission, jstring jPath )
{
	char *path = (char *)env->GetStringUTFChars(jPath, NULL);
	FILE *fd;
	char testFile[PATH_MAX];
	int ret = 0;
	
	// maybe generate new file everytime?
	Q_snprintf( testFile, PATH_MAX, "%s/.testfile", path );
	
	__android_log_print( ANDROID_LOG_VERBOSE, "Xash", "nativeTestWritePermission: file=%s", testFile );
	
	fd = fopen( testFile, "w+" );
	
	if( fd )
	{
		__android_log_print( ANDROID_LOG_VERBOSE, "Xash", "nativeTestWritePermission: passed" );
		ret = 1;
		fclose( fd );
		
		remove( testFile );
	}
	else
	{
		__android_log_print( ANDROID_LOG_VERBOSE, "Xash", "nativeTestWritePermission: error=%s", strerror( errno ) );
	}
	
	env->ReleaseStringUTFChars( jPath, path );
	
	return ret;
}

DECLARE_JNI_INTERFACE_VOID( int, nativeGetBuildVersion )
{
	return Q_buildnum();
}

DECLARE_JNI_INTERFACE_VOID( void, nativeOnLowMemory )
{
	return Sys_LowMemory();
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad( JavaVM *vm, void *reserved )
{
	return JNI_VERSION_1_6;
}


/*
========================
Android_SwapBuffers

Update screen. Use native EGL if possible
========================
*/
void Android_SwapBuffers()
{
	if( negl.valid )
		eglSwapBuffers( negl.dpy, negl.surface );
	else
	{
#ifdef XASH_NANOGL
		nanoGL_Flush();
#endif
#ifdef XASH_WES
	    wes_vertbuffer_flush();
#endif
		jni.env->CallStaticVoidMethod( jni.actcls, jni.swapBuffers );
	}
}

/*
========================
Android_GetScreenRes

Resolution got from last resize event
========================
*/
void Android_GetScreenRes(int *width, int *height)
{
	*width=jni.width, *height=jni.height;
}

/*
========================
Android_Init

Initialize android-related cvars
========================
*/
void Android_Init()
{
	android_sleep = Cvar_Get( "android_sleep", "1", CVAR_ARCHIVE, "Enable sleep in background" );
}

/*
========================
Android_UpdateSurface

Check if we may use native EGL without jni calls
========================
*/
void Android_UpdateSurface( void )
{
	negl.valid = false;

	if( Sys_CheckParm("-nonativeegl") )
		return; //disabled by user

	negl.dpy = eglGetCurrentDisplay();

	if( negl.dpy == EGL_NO_DISPLAY )
		return;

	negl.surface = eglGetCurrentSurface(EGL_DRAW);

	if( negl.surface == EGL_NO_SURFACE )
		return;

	// now check if swapBuffers does not give error
	if( eglSwapBuffers( negl.dpy, negl.surface ) == EGL_FALSE )
		return;

	// double check
	if( eglGetError() != EGL_SUCCESS )
		return;

	__android_log_print( ANDROID_LOG_VERBOSE, "Xash", "native EGL enabled" );

	negl.valid = true;
}

/*
========================
Android_GetGLAttribute
========================
*/
static int Android_GetGLAttribute( int eglAttr )
{
	int ret = jni.env->CallStaticIntMethod( jni.actcls, jni.getGLAttribute, eglAttr );
	// MsgDev(D_INFO, "Android_GetGLAttribute( %i ) => %i\n", eglAttr, ret );
	return ret;
}

/*
========================
Android_InitGL
========================
*/
qboolean Android_InitGL()
{
	int colorBits[3];
	qboolean result;
#ifdef XASH_NANOGL
	int glesVersion = 1;
#endif
#if defined(XASH_WES) || defined(XASH_GL4ES)
	int glesVersion = 2;
#endif
	result = jni.env->CallStaticBooleanMethod( jni.actcls, jni.createGLContext, (int)gl_stencilbits->integer, glesVersion );
	
	colorBits[0] = Android_GetGLAttribute( EGL_RED_SIZE );
	colorBits[1] = Android_GetGLAttribute( EGL_GREEN_SIZE );
	colorBits[2] = Android_GetGLAttribute( EGL_BLUE_SIZE );
	glConfig.color_bits = colorBits[0] + colorBits[1] + colorBits[2];
	glConfig.alpha_bits = Android_GetGLAttribute( EGL_ALPHA_SIZE );
	glConfig.depth_bits = Android_GetGLAttribute( EGL_DEPTH_SIZE );
	glConfig.stencil_bits = Android_GetGLAttribute( EGL_STENCIL_SIZE );
	glState.stencilEnabled = glConfig.stencil_bits ? true : false;

	//Android_UpdateSurface();
	
	return result;
}

/*
========================
Android_ShutdownGL
========================
*/
void Android_ShutdownGL()
{
	jni.env->CallStaticBooleanMethod( jni.actcls, jni.deleteGLContext );
}

/*
========================
Android_EnableTextInput

Show virtual keyboard
========================
*/
void Android_EnableTextInput( qboolean enable, qboolean force )
{
	if( force )
		jni.env->CallStaticVoidMethod( jni.actcls, jni.enableTextInput, enable );
	else if( enable )
	{
		if( !host.textmode )
		{
			jni.env->CallStaticVoidMethod( jni.actcls, jni.enableTextInput, 1 );
		}
		host.textmode = true;
	}
	else
	{
		jni.env->CallStaticVoidMethod( jni.actcls, jni.enableTextInput, 0 );
		host.textmode = false;
	}
}

/*
========================
Android_Vibrate
========================
*/
void Android_Vibrate( float life, char flags )
{
	if( life )
		jni.env->CallStaticVoidMethod( jni.actcls, jni.vibrate, (int)life );
}

/*
========================
Android_GetNativeObject
========================
*/
void *Android_GetNativeObject( const char *objName )
{
	static const char *availObjects[] = { "JNIEnv", "ActivityClass", NULL };
	void *object = NULL;
	
	if( !objName )
	{
		object = (void*)availObjects;
	}
	else if( !strcasecmp( objName, "JNIEnv" ) )
	{
		object = (void*)jni.env;
	}
	else if( !strcasecmp( objName, "ActivityClass" ) )
	{
		object = (void*)jni.actcls;
	}
	
	return object;
}

/*
========================
Android_MessageBox

Show messagebox and wait for OK button press
========================
*/
void Android_MessageBox(const char *title, const char *text)
{
	jni.env->CallStaticVoidMethod( jni.actcls, jni.messageBox, jni.env->NewStringUTF( title ), jni.env->NewStringUTF( text ) );
}

/*
========================
Android_SwapInterval
========================
*/
void Android_SwapInterval( int interval )
{
	// there is no eglSwapInterval in EGL10/EGL11 classes,
	// so only native backend supported
	if( negl.valid )
		eglSwapInterval( negl.dpy, interval );
}

/*
========================
Android_SetTitle
========================
*/
void Android_SetTitle( const char *title )
{
	jni.env->CallStaticVoidMethod( jni.actcls, jni.setTitle, jni.env->NewStringUTF( title ) );
}

/*
========================
Android_SetIcon
========================
*/
void Android_SetIcon( const char *path )
{
	jni.env->CallStaticVoidMethod( jni.actcls, jni.setIcon, jni.env->NewStringUTF( path ) );

}

/*
========================
Android_GetAndroidID
========================
*/
const char *Android_GetAndroidID( void )
{
	static char id[65];

	if( id[0] )
		return id;

	jstring resultJNIStr = (jstring)jni.env->CallStaticObjectMethod( jni.actcls, jni.getAndroidId );
	const char *resultCStr = jni.env->GetStringUTFChars( resultJNIStr, NULL );
	Q_strncpy( id, resultCStr, 64 );
	jni.env->ReleaseStringUTFChars( resultJNIStr, resultCStr );

	if( !id[0] )
		return NULL;

	return id;
}

/*
========================
Android_LoadID
========================
*/
const char *Android_LoadID( void )
{
	static char id[65];
	jstring resultJNIStr = (jstring)jni.env->CallStaticObjectMethod( jni.actcls, jni.loadID );
	const char *resultCStr = jni.env->GetStringUTFChars( resultJNIStr, NULL );
	Q_strncpy( id, resultCStr, 64 );
	jni.env->ReleaseStringUTFChars( resultJNIStr, resultCStr );
	return id;
}

/*
========================
Android_SaveID
========================
*/
void Android_SaveID( const char *id )
{
	jni.env->CallStaticVoidMethod( jni.actcls, jni.saveID, jni.env->NewStringUTF( id ) );
}

/*
========================
Android_MouseMove
========================
*/
void Android_MouseMove( float *x, float *y )
{
	*x = jnimouse.x;
	*y = jnimouse.y;
	jnimouse.x = 0;
	jnimouse.y = 0;
	//MsgDev( D_INFO, "Android_MouseMove: %f %f\n", *x, *y );
}

/*
========================
Android_AddMove
========================
*/
void Android_AddMove( float x, float y)
{
	jnimouse.x += x;
	jnimouse.y += y;
}

/*
========================
Android_ShowMouse
========================
*/
void Android_ShowMouse( qboolean show )
{
	if( m_ignore->integer )
		show = true;
	jni.env->CallStaticVoidMethod( jni.actcls, jni.showMouse, show );
}

/*
========================
Android_ShellExecute
========================
*/
void Android_ShellExecute( const char *path, const char *parms )
{
	jstring jstr;

	if( !path )
		return; // useless

	// get java.lang.String
	jstr = jni.env->NewStringUTF( path );

	// open browser
	jni.env->CallStaticVoidMethod(jni.actcls, jni.shellExecute, jstr);

	// no need to free jstr
}

AAssetManager *Android_GetAssetManager()
{
	jobject am = jni.env->CallStaticObjectMethod(jni.actcls, jni.getAssetManager);
	return AAssetManager_fromJava(jni.env, am);
}

void Android_AssetList(stringlist_t *list, const char *path, void (*stringlistappend)( stringlist_t *list, const char *text ))
{
	jobject assetManager_object = jni.env->CallStaticObjectMethod(jni.actcls, jni.getAssetManager);
	jmethodID list_method = jni.env->GetMethodID(jni.env->GetObjectClass(assetManager_object), "list", "(Ljava/lang/String;)[Ljava/lang/String;");
	jstring path_object = jni.env->NewStringUTF(path);
	jobjectArray files_object = (jobjectArray)jni.env->CallObjectMethod( assetManager_object, list_method, path_object);
	jni.env->DeleteLocalRef(path_object);

	int length = jni.env->GetArrayLength(files_object);

	for (int i = 0; i < length; i++)
	{
		jstring jstr = (jstring)jni.env->GetObjectArrayElement(files_object, i);

		const char * filename = jni.env->GetStringUTFChars(jstr, NULL);

		if (filename != NULL)
		{
			stringlistappend(list, filename);
			jni.env->ReleaseStringUTFChars(jstr, filename);
		}

		jni.env->DeleteLocalRef(jstr);
	}
}

float Android_GetDisplayDPI()
{
	return jni.env->CallStaticFloatMethod(jni.actcls, jni.getDisplayDPI);
}


void Android_Log(const char *str)
{
	__android_log_print( ANDROID_LOG_DEBUG, "Xash", "%s", str );

	jchar jbuffer[256];
	int size = Q_UTF8ToUTF16(str, jbuffer, sizeof(jbuffer));

	jstring jstr = jni.env->NewString( jbuffer, size );
	jni.env->CallStaticVoidMethod(jni.actcls, jni.logToBugly, jstr);
	jni.env->DeleteLocalRef(jstr);
}

#endif
