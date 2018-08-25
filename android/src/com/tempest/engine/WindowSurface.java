package com.tempest.engine;

import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.inputmethod.InputMethodManager;

public class WindowSurface extends SurfaceView
  implements SurfaceHolder.Callback {

  private final String TAG=getClass().getName();
  private boolean started=false;
  private Surface surface=null;

  public WindowSurface(Context context) {
    super(context);
    getHolder().addCallback(this);
    setFocusable(true);
    setFocusableInTouchMode(true);
    requestFocus();
    }

  public final Surface getSurface(){
    return surface;
    }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
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
    return true;
    }

  @Override
  public boolean dispatchKeyEvent(KeyEvent event) {
    Log.i( TAG, "dispatchKeyEvent()" + event.getCharacters() );
    Log.i( TAG, "k "+event.getUnicodeChar() );

    if( event.getCharacters()!=null ){
      onKeyCharEvent( event.getCharacters() );
      return true;
      }

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
      return super.onKeyUp(keyCode,event);
      } else {
      onKeyDownEvent(keyCode);
      return true;
      }
    }

  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event) {
    if( keyCode==KeyEvent.KEYCODE_BACK ){
      return super.onKeyUp(keyCode,event);
      } else {
      onKeyUpEvent(keyCode);
      return true;
      }
    }

  @Override
  public void surfaceCreated(SurfaceHolder holder) {
    surface=holder.getSurface();
    nativeSetSurface(surface);
    }

  @Override
  public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
    surface=holder.getSurface();
    nativeOnResize(surface, width, height);
    }

  @Override
  public void surfaceDestroyed(SurfaceHolder holder) {
    final Context context=getContext();
    if(context instanceof Activity){
      if(((Activity) context).isFinishing())
        nativeCloseEvent();
      }
    surface=null;
    nativeSetSurface(null);
    }

  @Override
  public void onAttachedToWindow(){
    super.onAttachedToWindow();
    nativeOnResume();
    }

  @Override
  public void onDetachedFromWindow(){
    nativeOnPause();
    super.onDetachedFromWindow();
    }

  @Override
  protected void onWindowVisibilityChanged(int visibility){
    if( visibility==VISIBLE ) {
      if(!started)
        nativeOnStart();
      started=true;
      } else {
      if(started)
        nativeOnStop();
      started=false;
      }
    super.onWindowVisibilityChanged(visibility);
    }
  
  void showSoftInput(){
    final Context ctx=getContext();
    InputMethodManager imm = (InputMethodManager)ctx.getSystemService(Context.INPUT_METHOD_SERVICE);
    if( imm==null )
      return;
    imm.showSoftInput(this, InputMethodManager.SHOW_FORCED);
  }

  void hideSoftInput(){
    final Context ctx=getContext();
    InputMethodManager imm = (InputMethodManager)ctx.getSystemService(Context.INPUT_METHOD_SERVICE);
    if( imm==null )
      return;

    imm.hideSoftInputFromWindow(getWindowToken(), 0);
    }

  void toggleSoftInput(){
    
    final Context ctx=getContext();
    InputMethodManager imm = (InputMethodManager)ctx.getSystemService(Context.INPUT_METHOD_SERVICE);
    if( imm==null )
      return;
    imm.toggleSoftInput(InputMethodManager.SHOW_FORCED,0);
    }

  private static native void nativeOnStart();
  private static native void nativeOnResume();
  private static native void nativeOnPause();
  private static native void nativeOnStop();

  public static native int   nativeCloseEvent();

  private static native void nativeOnTouch   ( int x, int y, int act, int pid );
  private static native void onKeyDownEvent  ( int k );
  private static native void onKeyUpEvent    ( int k );
  private static native void onKeyCharEvent  ( String s );
  private static native void nativeSetSurface( Surface surface );
  private static native void nativeOnResize  ( Surface surface, int w, int h );
  }
