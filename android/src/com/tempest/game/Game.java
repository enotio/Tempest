package com.tempest.game;

import com.tempest.R;

import android.os.Bundle;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

public class Game extends com.tempest.TempestActivity {
	@Override
	protected void onCreate(Bundle savedInstanceState) {  
    requestWindowFeature(Window.FEATURE_NO_TITLE);
    getWindow().setFlags(
        WindowManager.LayoutParams.FLAG_FULLSCREEN,
        WindowManager.LayoutParams.FLAG_FULLSCREEN
        );
    
    setContentView(R.layout.activity_tempest);
    SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
    
    super.onCreate(savedInstanceState, surfaceView);
	  }
  }