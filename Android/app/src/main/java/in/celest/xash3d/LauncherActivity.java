package in.celest.xash3d;

import android.app.*;
import android.content.*;
import android.graphics.*;
import android.graphics.drawable.*;
import android.net.*;
import android.os.*;
import android.text.*;
import android.text.method.*;
import android.text.style.*;
import android.util.*;
import android.view.*;
import android.widget.*;
import in.celest.xash3d.csbtem.*;
import java.io.*;
import java.net.*;
import java.util.LinkedList;
import java.util.List;

import org.json.*;
import android.preference.*;

import androidx.loader.content.Loader;

import su.xash.fwgslib.*;

public class LauncherActivity extends Activity 
{
	// public final static String ARGV = "in.celest.xash3d.MESSAGE";
	public final static int sdk = FWGSLib.sdk;
	public final static String UPDATE_LINK = "https://api.github.com/repos/FWGS/xash3d-android-project/releases"; // releases/latest doesn't return prerelease and drafts
	static SharedPreferences mPref;
	
	static EditText cmdArgs, resScale, resWidth, resHeight;
	static ToggleButton useVolume, resizeWorkaround;
	static CheckBox	immersiveMode;
	static TextView tvResPath, resResult;
	static RadioButton radioScale, radioCustom;
	static RadioGroup scaleGroup;
	static CheckBox resolution;
	static Spinner pixelSpinner;
	
	static int mEngineWidth, mEngineHeight;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		//this.requestWindowFeature(Window.FEATURE_NO_TITLE);
				
		if( CertCheck.dumbAntiPDALifeCheck( this ) )
		{
			finish();
			return;
		}
		
		setContentView(R.layout.activity_launcher);

		TabHost tabHost = (TabHost) findViewById(R.id.tabhost);

		tabHost.setup();
		
		TabHost.TabSpec tabSpec;
		tabSpec = tabHost.newTabSpec("tabtag1");
		tabSpec.setIndicator(getString(R.string.text_tab1));
		tabSpec.setContent(R.id.tab1);
		tabHost.addTab(tabSpec);

		tabSpec = tabHost.newTabSpec("tabtag2");
		tabSpec.setIndicator(getString(R.string.text_tab2));
		tabSpec.setContent(R.id.tab2);
		tabHost.addTab(tabSpec);
		

		mPref        = getSharedPreferences("engine", 0);
		cmdArgs      = (EditText) findViewById(R.id.cmdArgs);
		useVolume    = (ToggleButton) findViewById( R.id.useVolume );
		pixelSpinner = (Spinner) findViewById( R.id.pixelSpinner );
		resizeWorkaround = (ToggleButton) findViewById( R.id.enableResizeWorkaround );
		tvResPath    = (TextView) findViewById( R.id.textView_path );
		immersiveMode = (CheckBox) findViewById( R.id.immersive_mode );
		resolution = (CheckBox) findViewById(R.id.resolution);
		resWidth = (EditText) findViewById(R.id.resolution_width);
		resHeight = (EditText) findViewById(R.id.resolution_height);
		resScale = (EditText) findViewById(R.id.resolution_scale);
		radioCustom = (RadioButton) findViewById(R.id.resolution_custom_r);
		radioScale = (RadioButton) findViewById(R.id.resolution_scale_r);
		scaleGroup = (RadioGroup) findViewById( R.id.scale_group );
		resResult = (TextView) findViewById( R.id.resolution_result );
		
		final String[] list = {
			"32 bit (RGBA8888)",
			"24 bit (RGB888)",
			"16 bit (RGB565)",
			"16 bit (RGBA5551)",
			"16 bit (RGBA4444)",
			"8 bit (RGB332)"
		};
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,android.R.layout.simple_spinner_dropdown_item, list);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		pixelSpinner.setAdapter(adapter);
		((Button)findViewById( R.id.button_launch )).setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View v) 
			{
				startXash(v);
			}
		});
		((Button)findViewById( R.id.button_about )).setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View v) 
			{
				aboutXash(v);
			}
		});
		useVolume.setChecked(mPref.getBoolean("usevolume",true));
		//updateToBeta.setChecked(mPref.getBoolean("check_betas", false));
		updatePath(mPref.getString("basedir", FWGSLib.getDefaultXashPath(this) ) );
		cmdArgs.setText(mPref.getString("argv","-dev 3 -log"));
		pixelSpinner.setSelection(mPref.getInt("pixelformat", 0));
		resizeWorkaround.setChecked(mPref.getBoolean("enableResizeWorkaround", true));
		resolution.setChecked( mPref.getBoolean("resolution_fixed", false ) );
		
		DisplayMetrics metrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(metrics);
		
		// Swap resolution here, because engine is always(should be always) run in landscape mode
		if( FWGSLib.isLandscapeOrientation( this ) )
		{
			mEngineWidth = metrics.widthPixels;
			mEngineHeight = metrics.heightPixels;
		}
		else
		{
			mEngineWidth = metrics.heightPixels;
			mEngineHeight = metrics.widthPixels;
		}		
		
		resWidth.setText(String.valueOf(mPref.getInt("resolution_width", mEngineWidth )));
		resHeight.setText(String.valueOf(mPref.getInt("resolution_height", mEngineHeight )));
		resScale.setText(String.valueOf(mPref.getFloat("resolution_scale", 2.0f)));
		
		resWidth.addTextChangedListener( resTextChangeWatcher );
		resHeight.addTextChangedListener( resTextChangeWatcher );
		resScale.addTextChangedListener( resTextChangeWatcher );
		
		if( mPref.getBoolean("resolution_custom", false) )
			radioCustom.setChecked(true);
		else radioScale.setChecked(true);
		
		radioCustom.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener()
		{
			@Override
			public void onCheckedChanged( CompoundButton v, boolean isChecked )
			{
				updateResolutionResult();
				toggleResolutionFields();
			}
		} );
		resolution.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener()
		{
			@Override
			public void onCheckedChanged( CompoundButton v, boolean isChecked )
			{
				hideResolutionSettings( !isChecked );
			}
		});
		
		
		if( sdk >= 19 )
		{
			immersiveMode.setChecked(mPref.getBoolean("immersive_mode", true));
		}
		else
		{
			immersiveMode.setVisibility(View.GONE); // not available
		}


		hideResolutionSettings( !resolution.isChecked() );
		updateResolutionResult();
		toggleResolutionFields();
	}
	
	@Override
	public void onResume()
	{
		super.onResume();
	}

	void updatePath( String text )
	{
		tvResPath.setText(getString(R.string.text_res_path) + ":\n" + text );
		checkExtractAssets();
	}
	
	void hideResolutionSettings( boolean hide )
	{
		scaleGroup.setVisibility( hide ? View.GONE : View.VISIBLE );
	}
		
	TextWatcher resTextChangeWatcher = new TextWatcher()
	{
		@Override
		public void afterTextChanged(Editable s){}
		
		@Override
		public void beforeTextChanged(CharSequence s, int start, int count, int after){}
		
		@Override
		public void onTextChanged(CharSequence s, int start, int before, int count)
		{
			updateResolutionResult();
		}
	};
	
	void updateResolutionResult( )
	{
		int w, h;
		if( radioCustom.isChecked() )
		{
			w = getCustomEngineWidth();
			h = getCustomEngineHeight();
		}
		else
		{
			final float scale = getResolutionScale();
			w = (int)((float)mEngineWidth / scale);
			h = (int)((float)mEngineHeight / scale);
		}
		
		resResult.setText( getString( R.string.resolution_result ) + w + "x" + h );
	}
	
	void toggleResolutionFields()
	{
		boolean isChecked = radioCustom.isChecked();
		resWidth.setEnabled( isChecked );
		resHeight.setEnabled( isChecked );
		resScale.setEnabled( !isChecked );
	}
	
	float getResolutionScale()
	{
		return FWGSLib.atof( resScale.getText().toString(), 1.0f );
	}
	
	int getCustomEngineHeight()
	{
		return FWGSLib.atoi( resHeight.getText().toString(), mEngineHeight );
	}
	
	int getCustomEngineWidth()
	{
		return FWGSLib.atoi( resWidth.getText().toString(), mEngineWidth );
	}
	
    public void startXash(View view)
    {
		Intent intent = new Intent(this, XashActivity.class);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

		SharedPreferences.Editor editor = mPref.edit();
		editor.putString("argv", cmdArgs.getText().toString());
		editor.putBoolean("usevolume",useVolume.isChecked());
		editor.putInt("pixelformat", pixelSpinner.getSelectedItemPosition());
		editor.putBoolean("enableResizeWorkaround",resizeWorkaround.isChecked());
		editor.putBoolean("resolution_fixed", resolution.isChecked());
		editor.putBoolean("resolution_custom", radioCustom.isChecked());
		editor.putFloat("resolution_scale", getResolutionScale() );
		editor.putInt("resolution_width", getCustomEngineWidth() );
		editor.putInt("resolution_height", getCustomEngineHeight() );
		
		if( sdk >= 19 )
			editor.putBoolean("immersive_mode", immersiveMode.isChecked());
		else
			editor.putBoolean("immersive_mode", false); // just in case...
		editor.commit();
		startActivity(intent);
    }

	public void aboutXash(View view)
	{
		final Activity a = this;
		this.runOnUiThread(new Runnable() 
		{
			public void run()
			{
				final Dialog dialog = new Dialog(a);
				dialog.setContentView(R.layout.about);
				dialog.setCancelable(true);
				dialog.show();
				TextView tView6 = (TextView) dialog.findViewById(R.id.textView6);
				tView6.setMovementMethod(LinkMovementMethod.getInstance());
				((Button)dialog.findViewById( R.id.button_about_ok )).setOnClickListener(new View.OnClickListener(){
	       			@Override
	    			public void onClick(View v) {
	    				dialog.cancel();
					}
				});

			}
		});
	}

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        //getMenuInflater().inflate(R.menu.menu_launcher, menu);
		menu.add("About");
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        /*if (id == R.id.action_settings) {
            return true;
        }*/

		if (item.getTitle().equals("About")) {
				aboutXash(null);
		}

        return super.onOptionsItemSelected(item);
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

	public void checkExtractAssets() {
		try {
			List<String> list = getAssetsFileList("");

			if( mPref.getInt( "assetscount", 0 ) != list.size() ) {
				Thread t = new Thread(new AssetCopyHandler(this, list));
				t.start();
			}

		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}

class AssetCopyHandler implements Runnable {

	final ProgressDialog pd;
	final Context context;
	final List<String> fileList;

	public AssetCopyHandler(Context ctx, List<String> l) {
		context = ctx;
		fileList = l;
		pd = new ProgressDialog(context);

		pd.setMessage("Preparing...");
		pd.setTitle("Copying game assets");
		pd.setCancelable(false);

		pd.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
		pd.show();
	}

	Handler progressHandler = new Handler(new Handler.Callback() {
		@Override
		public boolean handleMessage(Message msg) {
			if(msg.what == 0)
				pd.setMax(msg.arg1);
			else if(msg.what == 1)
			{
				pd.incrementProgressBy(msg.arg1);
				pd.setMessage((String)msg.obj);
			}
			else if(msg.what == 2)
				pd.dismiss();
			else if(msg.what == 3)
				new AlertDialog.Builder( context )
						.setTitle( "Error copying game assets" )
						.setMessage( (String)msg.obj )
						.setPositiveButton( "Ok", null)
						.setCancelable( false )
						.show();
			return true;
		}
	});


	@Override
	public void run() {
		try {
			Message.obtain(progressHandler,0, fileList.size(), 0).sendToTarget();
			String savePath = context.getExternalFilesDir(null).getPath();
			byte[] buffer = new byte[1024];
			for(String s : fileList) {

				InputStream is = context.getAssets().open(s);

				File file = new File(savePath + "/" + s);
				if(!file.exists()) {
					file.getParentFile().mkdirs();
					file.createNewFile();
				}
				FileOutputStream fos = new FileOutputStream(file,false);
				int byteCount = 0;
				while ((byteCount = is.read(buffer)) != -1) {
					fos.write(buffer, 0, byteCount);
				}
				fos.flush();
				is.close();
				fos.close();

				Message.obtain(progressHandler,1, 1, 0, s).sendToTarget();
			}
			LauncherActivity.mPref.edit().putInt( "assetscount", fileList.size() ).apply();
		} catch (Exception e) {
			e.printStackTrace();
			Message.obtain(progressHandler,3, e.getMessage()).sendToTarget();
		}
		Message.obtain(progressHandler,2, 0, 0).sendToTarget();
	}
}
