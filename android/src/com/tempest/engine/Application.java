package com.tempest.engine;

public class Application extends android.app.Application {
  @SuppressWarnings("unused")
  private Tempest _mTempest;

  @Override
  public void onCreate() {
    super.onCreate();
    _mTempest=Tempest.createApp(this);
    }
  }
