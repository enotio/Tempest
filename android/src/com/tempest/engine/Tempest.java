package com.tempest.engine;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Handler;
import android.widget.Toast;

import java.lang.ref.WeakReference;
import java.util.Locale;

public class Tempest {
  private static WeakReference<Tempest> app;

  private final Context _mCtx;
  private final Handler _mHandler;
  private final Thread  _mMainThread;

  public static Tempest createApp(android.app.Application ctx) {
    final Tempest instance=(app==null ? null : app.get());
    if( instance!=null )
      return instance;
    return new Tempest(ctx);
    }

  private Tempest(android.app.Application context) {
    if(context==null)
      throw new NullPointerException();

    _mHandler = new Handler();
    _mCtx     = context;
    app       = new WeakReference<>(this);

    System.loadLibrary("c++_shared");
    System.loadLibrary("Tempest");

    nativeSetApplication(context);
    setupEnv(_mCtx);

    System.loadLibrary("main");

    final String nativePath=context.getApplicationInfo().nativeLibraryDir;
    _mMainThread = new Thread(new Runnable() {
      @Override
      public void run() {
        invokeMainImpl(nativePath + "/libmain.so");
        System.exit(0);
        }
      });
    _mMainThread.setName("main");
    _mMainThread.start();
    }

  private void setupEnv(Context ctx){
    nativeSetAssets( ctx.getAssets() );

    String external, internal;
    try {
      internal = ctx.getFilesDir().getAbsolutePath();
    }
    catch( Exception e ){
      internal = "";
    }

    try {
      external = ctx.getExternalFilesDir(null).getAbsolutePath();
    }
    catch( Exception e ){
      external = "";
    }

    nativeSetupStorage(internal, external);
    nativeSetupDpi( ctx.getResources().getDisplayMetrics().density );

    Locale loc = ctx.getResources().getConfiguration().locale;
    nativeInitLocale( loc.getISO3Language() );
  }

  @SuppressWarnings("unused")
  private static void showToast( String s ){
    final Tempest ctx=(app==null ? null : app.get());
    final CharSequence text = s;

    ctx._mHandler.post(new Runnable() {
      public void run() {
        Toast.makeText(
          ctx._mCtx,
          text,
          Toast.LENGTH_LONG ).show();
      }
    });
  }

  private static native void invokeMainImpl      ( String main );

  private static native void nativeSetApplication( android.app.Application app );
  private static native void nativeSetAssets     ( AssetManager m );
  private static native void nativeSetupStorage  ( String internal, String external );
  private static native void nativeInitLocale    ( String locale );
  private static native void nativeSetupDpi      ( float dpi );
  }
