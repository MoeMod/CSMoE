package in.celest.xash3d;

import static android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION;

import android.app.*;
import android.content.*;
import android.content.res.AssetManager;
import android.hardware.*;
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
public class XashActivity extends SDLActivity implements SensorEventListener
{

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

		sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		magnetometer = sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
		gyroscope = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
		if (bAllowIMU) {
			if (accelerometer != null) {
				sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_GAME);
			}
			if (magnetometer != null) {
				sensorManager.registerListener(this, magnetometer, SensorManager.SENSOR_DELAY_GAME);
			}
			if (gyroscope != null) {
				sensorManager.registerListener(this, gyroscope, SensorManager.SENSOR_DELAY_GAME);
			}
		}

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

	public static int getDeviceOrientation()
	{
		return mSingleton.getWindowManager().getDefaultDisplay().getRotation();
	}

	private boolean bAllowIMU = true;
	private SensorManager sensorManager;
	private Sensor accelerometer;
	private Sensor magnetometer;
	private Sensor gyroscope;
	static boolean first_acceleration_sample = true;
	static final float SampleDecayRate = 0.85f;

	private final float[] rotationMatrix = new float[9];
	private final float[] orientationAngles = new float[3];

	static float[] current_accelerometer = new float[]{0, 0, 0};
	static float[] current_gyroscope = new float[]{0, 0, 0};
	static float[] current_magnetometer = new float[]{0, 0, 0};

	static float[] filtered_gravity = new float[]{0, 0, 0};
	static float[] last_gravity = new float[]{0, 0, 0};
	static float[] last_tilt = new float[]{0, 0, 0};

	// active motion state to return to application on request
	static float[] current_tilt = new float[]{0, 0, 0};
	static float[] current_rotation_rate = new float[]{0, 0, 0};
	static float[] current_gravity = new float[]{0, 0, 0};
	static float[] current_acceleration = new float[]{0, 0, 0};

	@Override
	protected void onResume()
	{
		super.onResume();
		if (bAllowIMU) {
			if (accelerometer != null) {
				sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_GAME);
			}
			if (magnetometer != null) {
				sensorManager.registerListener(this, magnetometer, SensorManager.SENSOR_DELAY_GAME);
			}
			if (gyroscope != null) {
				sensorManager.registerListener(this, gyroscope, SensorManager.SENSOR_DELAY_GAME);
			}
		}
	}

	@Override
	protected void onPause()
	{
		super.onPause();
		if (bAllowIMU) {
			sensorManager.unregisterListener(this);
		}
	}

	@Override
	public void onSensorChanged(SensorEvent event)
	{
		if (bAllowIMU && accelerometer != null && magnetometer != null)
		{
			boolean bUpdate = false;
			boolean bGyroUpdate = false;

			if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
			{
				System.arraycopy(event.values, 0, current_accelerometer, 0, 3);
				bUpdate = true;

				// We use a low-pass filter to synthesize the gravity vector.
				if (!first_acceleration_sample)
				{
					filtered_gravity[0] = last_gravity[0] * SampleDecayRate + current_accelerometer[0]*(1.0f - SampleDecayRate);
					filtered_gravity[1] = last_gravity[1] * SampleDecayRate + current_accelerometer[1]*(1.0f - SampleDecayRate);
					filtered_gravity[2] = last_gravity[2] * SampleDecayRate + current_accelerometer[2]*(1.0f - SampleDecayRate);
				}
				first_acceleration_sample = false;
				last_gravity = filtered_gravity;
			}
			else if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD)
			{
				System.arraycopy(event.values, 0, current_magnetometer, 0, 3);
				bUpdate = true;
			}
			else if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE)
			{
				System.arraycopy(event.values, 0, current_gyroscope, 0, 3);
				bUpdate = true;
				bGyroUpdate = true;
			}

			// If we have motion samples we generate the single event.
			if (bUpdate)
			{
				// get the rotation matrix value, the convert those to Euler angle rotation values
				sensorManager.getRotationMatrix(rotationMatrix, null, current_accelerometer, current_magnetometer);
				sensorManager.getOrientation(rotationMatrix, orientationAngles);

				// protect against request from other thread
				synchronized(this)
				{
					// remember gravity
					current_gravity = filtered_gravity;

					// fix up the tilt mapping to proper coordinate frame
					current_tilt[0] = orientationAngles[1];
					current_tilt[1] = orientationAngles[2];
					current_tilt[2] = orientationAngles[0];

					// And take out the gravity from the accel to get the linear acceleration.
					current_acceleration[0] = current_accelerometer[0] - current_gravity[0];
					current_acceleration[1] = current_accelerometer[1] - current_gravity[1];
					current_acceleration[2] = current_accelerometer[2] - current_gravity[2];

					// Figure out the rotation rate
					if (bGyroUpdate)
					{
						// The rotation rate is the what the gyroscope gives us.
						current_rotation_rate = current_gyroscope;
					}
					else if (null == gyroscope)
					{
						// If we don't have a gyroscope at all we need to calc a rotation rate from delta from previous tilt.
						current_rotation_rate[0] = current_tilt[0] - last_tilt[0];
						current_rotation_rate[1] = current_tilt[1] - last_tilt[1];
						current_rotation_rate[2] = current_tilt[2] - last_tilt[2];
					}
					last_tilt = current_tilt;
				}

				// flag new sensor data ready
				nativeHandleSensorEvents(current_tilt, current_rotation_rate, current_gravity, current_acceleration);
			}
		}
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int i)
	{

	}

	public static native void nativeHandleSensorEvents(float[] tilt, float[] rotation_rate, float[] gravity, float[] acceleration);
}