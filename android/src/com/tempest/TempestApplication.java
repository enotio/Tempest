package com.tempest;

import android.app.Application;
import android.content.Context;

public class TempestApplication extends Application {
  private static Context context;
    
  public void onCreate(){
    context=getApplicationContext();
    }

  public static Context getContext() {
    return context;    
    } 
  }
