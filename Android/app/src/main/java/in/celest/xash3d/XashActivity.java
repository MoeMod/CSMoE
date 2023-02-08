package in.celest.xash3d;

import static android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION;

import android.app.*;
import android.content.*;
import android.content.res.AssetManager;
import android.view.*;
import android.os.*;
import android.util.*;
import android.net.Uri;

import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import in.celest.xash3d.csbtem.BuildConfig;
import android.provider.Settings.Secure;


import com.tencent.bugly.crashreport.BuglyLog;
import com.tencent.bugly.crashreport.CrashReport;

import org.libsdl.app.SDLActivity;
/**
 Xash Activity
 */
public class XashActivity extends SDLActivity {

	// Main components
	protected static XashActivity mSingleton;
	public static final String TAG = "CSMoE:XashActivity";
	public static Vibrator mVibrator;
	public static SharedPreferences mPref = null;
	public static AssetManager mAssetManager;

	// Load the .so
	static 
	{
		System.loadLibrary( "UE4" );
	}
	public static native int  nativeGetBuildVersion();

	// Setup
	@Override
	protected void onCreate( Bundle savedInstanceState ) 
	{
		Log.v( TAG, "onCreate()" );
		
		// So we can call stuff from static callbacks
		mSingleton = this;

		// fullscreen
		requestWindowFeature( Window.FEATURE_NO_TITLE );
		
		int flags = WindowManager.LayoutParams.FLAG_FULLSCREEN | 
			WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;

		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.P) {
			flags |= WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
		}

		getWindow().setFlags( flags, flags );

		final View decorView = getWindow().getDecorView();
		decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE | SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);

		mVibrator = ( Vibrator )getSystemService( Context.VIBRATOR_SERVICE );
			
		mPref = this.getSharedPreferences( "engine", 0 );

		mAssetManager = this.getAssets();

		CrashReport.UserStrategy strategy = new CrashReport.UserStrategy(getApplicationContext());
		strategy.setAppVersion(String.valueOf(nativeGetBuildVersion()));
		strategy.setEnableNativeCrashMonitor(true);


		CrashReport.initCrashReport(getApplicationContext(), "39df575b0b", false, strategy);
		CrashReport.setIsDevelopmentDevice(getApplicationContext(), BuildConfig.DEBUG);

		nativeSetenv("XASH3D_BASEDIR", getExternalFilesDir());
		//nativeSetenv("XASH3D_ENGLIBDIR", getFilesDir().getParentFile().getPath() + "/lib");
		//nativeSetenv("XASH3D_GAMELIBDIR", getFilesDir().getParentFile().getPath() + "/lib");
		//nativeSetenv("XASH3D_RODIR", "/android_asset/");
		//nativeSetenv("XASH3D_GAMEDIR", "csmoe");
		//nativeSetenv("XASH3D_EXTRAS_PAK1", getFilesDir().getPath() + "/extras.pak");
		//nativeSetenv("XASH3D_EXTRAS_PAK1", getFilesDir().getPath() + "/extras_cstrike.pak");
		super.onCreate(savedInstanceState);
	}

	@Override
	protected String[] getLibraries() {
		return new String[] {
				"UE4"
		};
	}

	@Override
	protected String[] getArguments() {
		return new String[]{ "-dev", "3", "-log", "-game", "csmoe" };
	}

	public static String getExternalFilesDir()
	{
		Context ctx = XashActivity.mSingleton;
		File f = ctx.getExternalFilesDir( null );

		f.mkdirs();

		return f.getAbsolutePath();
	}

	public static void vibrate( int time ) 
	{
		if( mVibrator.hasVibrator() )
		{
			mVibrator.vibrate( time );
		}
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

	// Just opens browser or update page
	public static void shellExecute( String path )
	{
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

	public static float getDisplayDPIf()
	{
		DisplayMetrics metrics = XashActivity.getContext().getResources().getDisplayMetrics();
		return metrics.density;
	}

	@Override
	public void onTrimMemory(int level)
	{
		super.onTrimMemory(level);
		if (level == TRIM_MEMORY_RUNNING_CRITICAL || level == TRIM_MEMORY_RUNNING_LOW || level == TRIM_MEMORY_RUNNING_MODERATE)
		{
			nativeLowMemory();
		}
	}

	public static void logToBugly(String str)
	{
		BuglyLog.w("Xash", str);
	}
}