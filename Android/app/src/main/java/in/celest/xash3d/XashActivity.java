package in.celest.xash3d;

import android.app.*;
import android.content.*;
import android.content.res.AssetManager;
import android.view.*;
import android.os.*;
import android.util.*;
import android.content.pm.*;
import android.net.Uri;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import android.provider.Settings.Secure;


import com.tencent.bugly.crashreport.BuglyLog;
import com.tencent.bugly.crashreport.CrashReport;

import org.libsdl.app.SDLActivity;

/**
 Xash Activity
 */
public class XashActivity extends Activity {

	// Main components
	protected static XashActivity mSingleton;
	public static String mArgv[];
	public static final int sdk = Integer.valueOf( Build.VERSION.SDK );
	public static final String TAG = "CSMoE:XashActivity";
	public static SharedPreferences mPref = null;
	public static AssetManager mAssetManager;

	// Setup
	@Override
	protected void onCreate( Bundle savedInstanceState ) 
	{
		Log.v( TAG, "onCreate()" );
		super.onCreate( savedInstanceState );
		
		// So we can call stuff from static callbacks
		mSingleton = this;

		// fullscreen
		requestWindowFeature( Window.FEATURE_NO_TITLE );
		
		int flags = WindowManager.LayoutParams.FLAG_FULLSCREEN | 
			WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;
		getWindow().setFlags( flags, flags );

		// landscapeSensor is not supported until API9
		if( sdk < 9 )
			setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE );
			
		mPref = this.getSharedPreferences( "engine", 0 );

		mAssetManager = this.getAssets();
		/*
		CrashReport.UserStrategy strategy = new CrashReport.UserStrategy(getApplicationContext());
		strategy.setAppVersion(String.valueOf(nativeGetBuildVersion()));
		strategy.setEnableNativeCrashMonitor(true);

		CrashReport.initCrashReport(getApplicationContext(), "39df575b0b", false, strategy);
		CrashReport.setIsDevelopmentDevice(getApplicationContext(), BuildConfig.DEBUG);
		*/
		Intent intent = new Intent(this, SDLActivity.class);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

		SharedPreferences.Editor editor = mPref.edit();
		editor.putString("argv", mPref.getString( "argv", "-dev 3 -log" ));
		editor.putString("basedir", getExternalFilesDir(this));
		editor.commit();
		editor.apply();
		startActivity(intent);
	}

	public static String getExternalFilesDir( Context ctx )
	{
		File f = ctx.getExternalFilesDir( null );

		f.mkdirs();

		return f.getAbsolutePath();
	}
	
	public static native int  nativeInit( Object arguments );
	public static native void nativeQuit();
	public static native int  nativeGetBuildVersion();

	public static native int setenv( String key, String value, boolean overwrite );

	public static void vibrate( int time ) 
	{
	}

	public static Context getContext() 
	{
		return mSingleton;
	}

	protected final String[] messageboxData = new String[2];
	public static void messageBox( String title, String text )
	{
		mSingleton.messageboxData[0] = title;
		mSingleton.messageboxData[1] = text;
		//mSingleton.runOnUiThread( new Runnable()
		mSingleton.runOnUiThread( new Runnable()
		{
			@Override
			public void run()
			{
				new AlertDialog.Builder( mSingleton )
					.setTitle( mSingleton.messageboxData[0] )
					.setMessage( mSingleton.messageboxData[1] )
					.setPositiveButton( "Ok", new DialogInterface.OnClickListener()
						{
							public void onClick( DialogInterface dialog, int whichButton )
							{
								synchronized( mSingleton.messageboxData )
								{
									mSingleton.messageboxData.notify();
								}
							}
						})
					.setCancelable( false )
					.show();
			}
		});
		synchronized( mSingleton.messageboxData ) 
		{
			try 
			{
				mSingleton.messageboxData.wait();
			} 
			catch( InterruptedException ex )
			{
				ex.printStackTrace();
			}
		}
	}

	public static void setIcon( String path )
	{
		Log.v( TAG, "setIcon(" + path + ")" );
	}

	public static void setTitle( String title )
	{
		Log.v( TAG, "setTitle(" + title + ")" );
	}

	public static String getAndroidID()
	{
		String str = Secure.getString( mSingleton.getContentResolver(), Secure.ANDROID_ID );
		
		if( str == null )
			return "";
		
		return str;
	}

	public static String loadID()
	{
		return mPref.getString( "xash_id", "" );
	}

	public static void saveID( String id )
	{
		SharedPreferences.Editor editor = mPref.edit();

		editor.putString( "xash_id", id );
		editor.commit();
	}
	
	public static void GenericUpdatePage()
	{
		mSingleton.startActivity( new Intent( Intent.ACTION_VIEW, Uri.parse("https://github.com/FWGS/xash3d/releases/latest" ) ) );
	}
	
	public static void PlatformUpdatePage()
	{
		// open GP
		try 
		{
			mSingleton.startActivity( new Intent( Intent.ACTION_VIEW, Uri.parse("market://details?id=in.celest.xash3d.hl") ) );
		}
		catch( android.content.ActivityNotFoundException e ) 
		{
			GenericUpdatePage();
		}
	}
	
	// Just opens browser or update page
	public static void shellExecute( String path )
	{
		if( path.equals("PlatformUpdatePage"))
		{
			PlatformUpdatePage();
			return;
		}
		else if( path.equals( "GenericUpdatePage" ))
		{
			GenericUpdatePage();
			return;
		}
	
		final Intent intent = new Intent(Intent.ACTION_VIEW).setData(Uri.parse(path));
		mSingleton.startActivity(intent);
	}

	// call from c
	public static AssetManager getAssetManager()
	{
		if(XashActivity.mAssetManager != null)
			return XashActivity.mAssetManager;
		return XashActivity.mSingleton.getAssets();
	}

	List<String> getAssetsFileList(String path) throws IOException {
		List<String> result = new LinkedList<>();

		String[] fileNames = getAssets().list(path);

		if (fileNames != null && fileNames.length > 0) {
			for (String s : fileNames)
				result.addAll(getAssetsFileList(path.isEmpty() ? s : path + "/" + s));
		} else {
			result.add(path);
		}
		return result;
	}

	public static float getDisplayDPI()
	{
		DisplayMetrics metrics = XashActivity.getContext().getResources().getDisplayMetrics();
		return metrics.density;
	}

	public static void logToBugly(String str)
	{
		BuglyLog.w("Xash", str);
	}
}