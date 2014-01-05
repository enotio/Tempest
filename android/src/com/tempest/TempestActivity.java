package com.tempest;

import android.os.Bundle;
import android.app.Activity;
import android.content.res.AssetManager;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.Window;
import android.view.WindowManager;
import android.util.Log;

public class TempestActivity extends Activity
implements SurfaceHolder.Callback  {
  private static String TAG = "TempestActivity";

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
    int action = event.getAction();
    int x      = Math.round(event.getX());
    int y      = Math.round(event.getY());
    int pid    = event.getPointerId(0);
    
    if (action==MotionEvent.ACTION_MOVE){
     nativeOnTouch( x, y, 2, pid );
     }
    else if (action==MotionEvent.ACTION_DOWN){
     nativeOnTouch( x, y, 0, pid );
     }
    else if (action==MotionEvent.ACTION_UP){
     nativeOnTouch( x, y, 1, pid );
     }
  
    return true;
    }
  
  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event) {
    if( keyCode==KeyEvent.KEYCODE_BACK ){
      if( onClose(0) )
        return true; else
        return super.onKeyUp(keyCode, event);
      } else {
      onKeyEvent(keyCode);
      return true;
      }
    }

  @Override
  public void onBackPressed(){
    if( !onClose(1) )
      super.onBackPressed();
    }

  private int closeEventId = -1, closeRes = -1;
  private boolean onClose( int eid ){
    if( closeEventId==-1 )
      closeEventId = eid;
    
    if( closeEventId==eid )     
      closeRes = nativeCloseEvent();
    
    return closeRes==1;
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
  static native void onKeyEvent( int k );
  static native int  nativeCloseEvent();
  static native void nativeSetSurface(Surface surface);
  static native void nativeOnResize( Surface surface, int w, int h );
  static native void nativeSetAssets( AssetManager m );
  static native void nativeSetupStorage( String internal, String external );
    
  static {
    System.loadLibrary("gnustl_shared");
    System.loadLibrary("Tempest");
    System.loadLibrary("network");
    System.loadLibrary("game");
  }
}
