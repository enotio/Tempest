package com.tempest.game;

import java.util.Locale;

import android.os.Bundle;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

import com.tempest.TempestActivityBase;

public class TempestActivity extends TempestActivityBase  {
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

  }
