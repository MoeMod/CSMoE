package in.celest.xash3d;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import java.io.FileOutputStream;
import java.io.InputStream;
import android.content.SharedPreferences;

public class InstallReceiver extends BroadcastReceiver {
	private static final String TAG = "XASH3D";
	@Override
	public void onReceive(Context context, Intent arg1) {
		String pkgname = arg1.getData().getEncodedSchemeSpecificPart();
		Log.d( TAG, "Install received, package " + pkgname );
    		if( context.getPackageName().equals(pkgname) )
    			;
	}
	public static SharedPreferences mPref = null;
	
}
