package com.tempest.engine;

import android.content.res.AssetManager;
import android.os.Handler;
import android.widget.Toast;

import java.lang.ref.WeakReference;
import java.util.Locale;

public class Application extends android.app.Application {
  private static WeakReference<Application> app;
  private Handler handler;

  @Override
  public void onCreate() {
    super.onCreate();

    handler = new Handler();
    app     = new WeakReference<>(this);

    System.loadLibrary("c++_shared");

    System.loadLibrary("Tempest");
    setupEnv();

    System.loadLibrary("main");

    final String nativePath=getApplicationInfo().nativeLibraryDir;
    invokeMainImpl(nativePath+"/libmain.so");
    }

  private void setupEnv(){
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
    nativeSetupDpi( getResources().getDisplayMetrics().density );

    Locale loc = getResources().getConfiguration().locale;
    nativeInitLocale( loc.getISO3Language() );
    }

  @SuppressWarnings("unused")
  private static void showToast( String s ){
    final Application ctx=(app==null ? null : app.get());
    final CharSequence text = s;

    ctx.handler.post(new Runnable() {
      public void run() {
        Toast.makeText(
          ctx,
          text,
          Toast.LENGTH_LONG ).show();
        }
      });
    }

  private static native void invokeMainImpl    ( String main );

  private static native void nativeSetAssets   ( AssetManager m );
  private static native void nativeSetupStorage( String internal, String external );
  private static native void nativeInitLocale  ( String locale );
  private static native void nativeSetupDpi    ( float dpi );
  }
