package com.tempest;

import java.util.Locale;

import android.os.Bundle;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.res.AssetManager;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;
import android.util.Log;

public class TempestActivity extends Activity
implements SurfaceHolder.Callback  {
  private static String TAG = "TempestActivity";
  static TempestActivity thiz;

  @Override
  protected void onCreate(Bundle savedInstanceState) {   
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    getWindow().setFlags(
        WindowManager.LayoutParams.FLAG_FULLSCREEN,
        WindowManager.LayoutParams.FLAG_FULLSCREEN
        );
    
    setContentView(R.layout.activity_tempest);
    SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
    
    onCreate(savedInstanceState, surfaceView);
    }

  protected void onCreate( Bundle savedInstanceState, SurfaceView surfaceView ) {
    thiz = this;
    
    super.onCreate(savedInstanceState);
    Log.i(TAG, "onCreate()"); 

    nativeSetAssets( getAssets() );
    
    String external, internal;
    try {
      internal = getFilesDir().getAbsolutePath();
      }
    catch( Exception e ){
      internal = "";
      }
    
    try {
      external = getExternalFilesDir(null).getAbsolutePath();
      }
    catch( Exception e ){
      external = "";
      }
    
    nativeSetupStorage(internal, external);
    nativeSetupDpi( getResources().getDisplayMetrics().densityDpi );
    
    Locale loc = getResources().getConfiguration().locale;
    nativeInitLocale( loc.getISO3Language() );
    nativeOnCreate();

    surfaceView.getHolder().addCallback(this);
    }

  protected void onDestroy(){
    nativeOnDestroy();
    }
  
  @Override
  protected void onStart() {
    super.onStart();
    Log.i(TAG, "onStart()");
    nativeOnStart();
    }

  @Override
  protected void onResume() {
    super.onResume();
    Log.i(TAG, "onResume()");
    nativeOnResume();
    }

  @Override
  protected void onPause() {
    Log.i(TAG, "onPause()");
    nativeOnPause();
    super.onPause();
    }

  @Override
  protected void onStop() {
    Log.i(TAG, "onStop()");
    nativeOnStop();
    super.onStop();
    }
  
  @Override
  public boolean onTouchEvent(MotionEvent event) {
    Log.i(TAG, "onTouchEvent()");
    int index   = (event.getAction() & MotionEvent.ACTION_POINTER_INDEX_MASK)
                    >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
    int pid = event.getPointerId(index);
    int x = 0, y = 0, act = -1;

    switch (event.getActionMasked()) {
      case MotionEvent.ACTION_MOVE: {
        for( int p = 0; p < event.getPointerCount(); p++ ){     
          x = Math.round( event.getX(p) );
          y = Math.round( event.getY(p) );
          pid = event.getPointerId(p);
          
          nativeOnTouch( x, y, 2, pid );
          }
        break;
        }
    
      case MotionEvent.ACTION_DOWN: {        
        x = Math.round( event.getX() );
        y = Math.round( event.getY() );
        act = 0;
        break;
        }

      case MotionEvent.ACTION_UP: {    
        x = Math.round( event.getX() );
        y = Math.round( event.getY() );
        act = 1;
        break;
        }

      case MotionEvent.ACTION_POINTER_DOWN: {
        x = Math.round( event.getX(index) );
        y = Math.round( event.getY(index) );
        act = 0;
        break;
        }

      case MotionEvent.ACTION_POINTER_UP: {
        x = Math.round( event.getX(index) );
        y = Math.round( event.getY(index) );
        act = 1;
        break;
        }
      }
    
    if( act>=0 )
      nativeOnTouch( x, y, act, pid );
    Log.i(TAG, "~onTouchEvent()");
    return true;   
    }
  @Override
  public boolean dispatchKeyEvent(KeyEvent event) {
    
    Log.i( TAG, "dispatchKeyEvent()" + event.getCharacters() );
    Log.i( TAG, "k "+event.getUnicodeChar() );

    if( event.getCharacters()!=null ){
      onKeyCharEvent( event.getCharacters() );
      return true;
      } else
    if( event.getUnicodeChar()!=0 &&
        event.getUnicodeChar()!=10 &&
        event.getUnicodeChar()!=13 ){
      if( event.getAction() == KeyEvent.ACTION_DOWN )
        onKeyCharEvent( Character.toString((char) event.getUnicodeChar()) );
      return true;
      }
    
    return super.dispatchKeyEvent(event);
    }
  
  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    if( keyCode==KeyEvent.KEYCODE_BACK ){
      onClose(2);
      return true;
      } else {
      onKeyDownEvent(keyCode);
      return true;
      }
    }
  
  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event) {
    if( keyCode==KeyEvent.KEYCODE_BACK ){
      onClose(0);
      return true;
      } else {
      onKeyUpEvent(keyCode);
      return true;
      }
    }

  @Override
  public void onBackPressed(){
    onClose(1);
    }

  private int closeEventId = -1;
  private void onClose( int eid ){
    if( closeEventId==-1 )
      closeEventId = eid;
    
    if( closeEventId==eid )     
      nativeCloseEvent();
    }

  public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
    nativeOnResize(holder.getSurface(), w, h);
    }

  public void surfaceCreated(SurfaceHolder holder) {
    nativeSetSurface(holder.getSurface());
    }

  public void surfaceDestroyed(SurfaceHolder holder) {
    nativeSetSurface(null);
    }
  
  private static void showToast( String s ){
    final Context context   = thiz.getApplicationContext();
    final CharSequence text = s;
    
    thiz.runOnUiThread(new Runnable() {
        public void run() {
            Toast.makeText(
                context,
                text, 
                Toast.LENGTH_LONG ).show();
        }
    });    
    }
  
  private static void showSoftInput(){
    InputMethodManager imm = (InputMethodManager)thiz.getSystemService(INPUT_METHOD_SERVICE);
    if( imm==null )
      return;
    //imm.toggleSoftInput(InputMethodManager.SHOW_FORCED,0);
    View view = thiz.getWindow().getDecorView().getRootView();
    imm.showSoftInput(view, 0);
    }
  
  private static void hideSoftInput(){
    InputMethodManager imm = (InputMethodManager)thiz.getSystemService(INPUT_METHOD_SERVICE);
    if( imm==null )
      return;

    View view = thiz.getWindow().getDecorView().getRootView();
    imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
    }
  
  private static void toggleSoftInput(){
    InputMethodManager imm = (InputMethodManager)thiz.getSystemService(INPUT_METHOD_SERVICE);
    if( imm==null )
      return;
    
    imm.toggleSoftInput(InputMethodManager.SHOW_FORCED,0);
    }
  
  static void invokeMain(){
    invokeMainImpl();
    exitApp();
    }
  
  static void exitApp(){
    System.runFinalizersOnExit(true);
    System.exit(0);
    }
  
  static native void invokeMainImpl();

  static native void nativeOnCreate();
  static native void nativeOnDestroy();

  static native void nativeOnStart();
  static native void nativeOnResume();
  static native void nativeOnPause();
  static native void nativeOnStop();
  
  static native void nativeInitLocale( String locale );
  static native void nativeSetupDpi  ( int dpi );
  
  static native void nativeOnTouch( int x, int y, int act, int pid );
  static native void onKeyDownEvent( int k );
  static native void onKeyUpEvent( int k );
  static native void onKeyCharEvent( String s );
  static native int  nativeCloseEvent();
  static native void nativeSetSurface(Surface surface);
  static native void nativeOnResize( Surface surface, int w, int h );
  static native void nativeSetAssets( AssetManager m );
  static native void nativeSetupStorage( String internal, String external );
  
  static void loadLib( String lib ){
    try {
      System.loadLibrary(lib);
      }
    catch( java.lang.UnsatisfiedLinkError e ){
      Log.e( "", "lib not loaded: \"" + lib + "\"" );
      throw e;
      }
    
    }
    
  static {
    try{
      loadLib("gnustl_shared");
      loadLib("bullet");
      loadLib("Tempest");
      loadLib("network");
      loadLib("game");
      }
    catch( java.lang.UnsatisfiedLinkError e ){
      //FIXME: no way to display message correctly
      Context c = TempestApplication.getContext();
      
      AlertDialog errorDialog = new AlertDialog.Builder(c).create();
      errorDialog.setMessage("Fatal error, your application can't be started.");
      errorDialog.show();
      }
    }
  }
