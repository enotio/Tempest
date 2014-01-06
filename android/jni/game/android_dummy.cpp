#ifdef __ANDROID__
#include <jni.h>
#include <Tempest/SystemAPI>
#include <Tempest/Application>

template< class T >
void mCall( T m ){
  void (*mx)(int, const char**) = reinterpret_cast<void (*)(int, const char**)>(m);
  static const char * args = "TempestApplication";
  mx(0, &args);
  }

static void invokeMainImpl(){
  int main( int, char** );

  //Eclipse sucks, set nonzero value if something wierd happend at debuging
  Tempest::Application::sleep(0);
  mCall(main);
  }

jint JNI_OnLoad(JavaVM *vm, void */*reserved*/){
  static JNINativeMethod methodTable[] = {
    {"invokeMainImpl",  "()V", (void *) invokeMainImpl  }
    };

  JNIEnv* env;
  if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK ) {
    return -1;
    }

  jclass tempestClass = env->FindClass( Tempest::SystemAPI::androidActivityClass().c_str() );
  if (!tempestClass) {
    return -1;
    }

  env->RegisterNatives( tempestClass,
                        methodTable,
                        sizeof(methodTable) / sizeof(methodTable[0]) );

  return JNI_VERSION_1_6;
  }

#endif
