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
#include "client.h"

#include <pthread.h>
#include <jni.h>
#include <sys/prctl.h>
#include <android/log.h>
#include <android/asset_manager_jni.h>

#include "SDL_system.h"

#include "tier1/strtools.h"

static struct jnimethods_s
{
	jclass actcls;
	JNIEnv *env;
	jmethodID vibrate;
	jmethodID messageBox;
	jmethodID getAndroidId;
	jmethodID saveID;
	jmethodID loadID;
	jmethodID shellExecute;
	jmethodID getAssetManager;
	jmethodID getDisplayDPI;
	jmethodID logToBugly;
} jni;


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

DECLARE_JNI_INTERFACE_VOID( int, nativeGetBuildVersion )
{
	return Q_buildnum();
}

DECLARE_JNI_INTERFACE_VOID( void, nativeOnLowMemory )
{
    return Sys_LowMemory();
}

void Android_SetupJNI()
{
	/* Init callbacks. */
	auto env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	jni.env = env;
	jni.actcls = env->FindClass("in/celest/xash3d/XashActivity");
	jni.vibrate = env->GetStaticMethodID(jni.actcls, "vibrate", "(I)V" );
	jni.messageBox = env->GetStaticMethodID(jni.actcls, "messageBox", "(Ljava/lang/String;Ljava/lang/String;)V");
	jni.getAndroidId = env->GetStaticMethodID(jni.actcls, "getAndroidID", "()Ljava/lang/String;");
	jni.saveID = env->GetStaticMethodID(jni.actcls, "saveID", "(Ljava/lang/String;)V");
	jni.loadID = env->GetStaticMethodID(jni.actcls, "loadID", "()Ljava/lang/String;");
	jni.shellExecute = env->GetStaticMethodID(jni.actcls, "shellExecute", "(Ljava/lang/String;)V");
	jni.getAssetManager = env->GetStaticMethodID(jni.actcls, "getAssetManager", "()Landroid/content/res/AssetManager;");
	jni.getDisplayDPI = env->GetStaticMethodID(jni.actcls, "getDisplayDPIf", "()F");
	jni.logToBugly = env->GetStaticMethodID(jni.actcls, "logToBugly", "(Ljava/lang/String;)V");
}

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

void Android_MessageBox(const char *title, const char *text)
{
	jni.env->CallStaticVoidMethod( jni.actcls, jni.messageBox, jni.env->NewStringUTF( title ), jni.env->NewStringUTF( text ) );
}

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

const char *Android_LoadID( void )
{
	static char id[65];
	jstring resultJNIStr = (jstring)jni.env->CallStaticObjectMethod( jni.actcls, jni.loadID );
	const char *resultCStr = jni.env->GetStringUTFChars( resultJNIStr, NULL );
	Q_strncpy( id, resultCStr, 64 );
	jni.env->ReleaseStringUTFChars( resultJNIStr, resultCStr );
	return id;
}

void Android_SaveID( const char *id )
{
	jni.env->CallStaticVoidMethod( jni.actcls, jni.saveID, jni.env->NewStringUTF( id ) );
}

void Android_Vibrate( float life, char flags )
{
	if( life )
		jni.env->CallStaticVoidMethod( jni.actcls, jni.vibrate, (int)life );
}

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
