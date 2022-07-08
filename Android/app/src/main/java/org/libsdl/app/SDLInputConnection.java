package org.libsdl.app;

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;

public class SDLInputConnection extends BaseInputConnection {

    public SDLInputConnection(View targetView, boolean fullEditor) {
        super(targetView, fullEditor);

    }

    @Override
    public boolean sendKeyEvent(KeyEvent event) {
        /*
         * This used to handle the keycodes from soft keyboard (and IME-translated input from hardkeyboard)
         * However, as of Ice Cream Sandwich and later, almost all soft keyboard doesn't generate key presses
         * and so we need to generate them ourselves in commitText.  To avoid duplicates on the handful of keys
         * that still do, we empty this out.
         */

        /*
         * Return DOES still generate a key event, however.  So rather than using it as the 'click a button' key
         * as we do with physical keyboards, let's just use it to hide the keyboard.
         */

        if (event.getKeyCode() == KeyEvent.KEYCODE_ENTER) {
            if (SDLActivity.onNativeSoftReturnKey()) {
                return true;
            }
        }


        return super.sendKeyEvent(event);
    }

    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {

        for (int i = 0; i < text.length(); i++) {
            char c = text.charAt(i);
            if (c == '\n') {
                if (SDLActivity.onNativeSoftReturnKey()) {
                    return true;
                }
            }
            nativeGenerateScancodeForUnichar(c);
        }

        SDLInputConnection.nativeCommitText(text.toString(), newCursorPosition);

        return super.commitText(text, newCursorPosition);
    }

    @Override
    public boolean setComposingText(CharSequence text, int newCursorPosition) {

        nativeSetComposingText(text.toString(), newCursorPosition);

        return super.setComposingText(text, newCursorPosition);
    }

    public static native void nativeCommitText(String text, int newCursorPosition);

    public native void nativeGenerateScancodeForUnichar(char c);

    public native void nativeSetComposingText(String text, int newCursorPosition);

    @Override
    public boolean deleteSurroundingText(int beforeLength, int afterLength) {
        // Workaround to capture backspace key. Ref: http://stackoverflow.com/questions/14560344/android-backspace-in-webview-baseinputconnection
        // and https://bugzilla.libsdl.org/show_bug.cgi?id=2265
        if (beforeLength > 0 && afterLength == 0) {
            boolean ret = true;
            // backspace(s)
            while (beforeLength-- > 0) {
                boolean ret_key = sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL))
                        && sendKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL));
                ret = ret && ret_key;
            }
            return ret;
        }

        return super.deleteSurroundingText(beforeLength, afterLength);
    }
}

class SDLClipboardHandler implements
        ClipboardManager.OnPrimaryClipChangedListener {

    protected ClipboardManager mClipMgr;

    SDLClipboardHandler() {
        mClipMgr = (ClipboardManager) SDL.getContext().getSystemService(Context.CLIPBOARD_SERVICE);
        mClipMgr.addPrimaryClipChangedListener(this);
    }

    public boolean clipboardHasText() {
        return mClipMgr.hasPrimaryClip();
    }

    public String clipboardGetText() {
        ClipData clip = mClipMgr.getPrimaryClip();
        if (clip != null) {
            ClipData.Item item = clip.getItemAt(0);
            if (item != null) {
                CharSequence text = item.getText();
                if (text != null) {
                    return text.toString();
                }
            }
        }
        return null;
    }

    public void clipboardSetText(String string) {
        mClipMgr.removePrimaryClipChangedListener(this);
        ClipData clip = ClipData.newPlainText(null, string);
        mClipMgr.setPrimaryClip(clip);
        mClipMgr.addPrimaryClipChangedListener(this);
    }

    @Override
    public void onPrimaryClipChanged() {
        SDLActivity.onNativeClipboardChanged();
    }
}

