#ifndef JNIEXTRAS_H
#define JNIEXTRAS_H

#include <Tempest/Platform>
#include <cstdlib>

#ifdef __ANDROID__

#include <initializer_list>
#include <android/native_window_jni.h>
#include <jni.h>

namespace Tempest {

namespace Jni{
  class Class {
    public:
      Class();
      Class(JNIEnv& env,const char* name);
      Class(JNIEnv& env, jclass cls);
      Class(Class &&other);
      ~Class();

      Class& operator = (Class &&other);

      operator bool   () const { return cls!=nullptr; }
      operator jclass ()       { return cls; }

      bool registerNatives(JNIEnv &env, const JNINativeMethod *m, size_t count);
      bool registerNatives(JNIEnv &env, const std::initializer_list<JNINativeMethod>& l);

      template<class ... Args>
      void callStaticVoid(JNIEnv &env, const char* func,const char* sig,Args ... a) {
        if(cls==nullptr )
          return;

        jmethodID m = env.GetStaticMethodID(cls,func,sig);
        clearException(env);
        if( m!=nullptr )
          env.CallStaticVoidMethod(cls,m,a...);
        }

      template<class ... Args>
      jobject callStaticObject(JNIEnv &env, const char* func,const char* sig,Args ... a) {
        if(cls==nullptr )
          return nullptr;

        jmethodID m = env.GetStaticMethodID(cls,func,sig);
        clearException(env);
        if( m!=nullptr )
          return env.CallStaticObjectMethod(cls,m,a...);
        return nullptr;
        }

      template<class ... Args>
      void callVoid(JNIEnv &env, jobject self, const char* func,const char* sig,Args ... a) {
        if(cls==nullptr || self==nullptr )
          return;

        jmethodID m = env.GetMethodID(cls,func,sig);
        clearException(env);
        if( m!=nullptr )
          env.CallVoidMethod(self,m,a...);
        }

      template<class ... Args>
      jobject callObject(JNIEnv &env, jobject self, const char* func,const char* sig,Args ... a) {
        if(cls==nullptr || self==nullptr )
          return nullptr;

        jmethodID m = env.GetMethodID(cls,func,sig);
        clearException(env);
        if( m!=nullptr )
          return env.CallObjectMethod(self,m,a...);
        return nullptr;
        }

    private:
      JNIEnv* env=nullptr;
      jclass  cls=nullptr;

      void clearException(JNIEnv& env){
        // avoid jni crash
        if( !env.ExceptionOccurred() )
          return;
        env.ExceptionDescribe();
        env.ExceptionClear();
        }
    };

  class Object {
    public:
      Object() = default;
      Object(JNIEnv& env,const jobject obj);
      Object(const Object &other);
      ~Object();

      operator bool    () const { return self!=nullptr; }
      operator jobject ()       { return self; }

      Object& operator = (const Object &other);

    private:
      JNIEnv* env =nullptr;
      jobject self=nullptr;
    };

  class AndroidWindow {
    public:
      AndroidWindow() = default;
      AndroidWindow(JNIEnv &env, jobject surface);
      AndroidWindow(const AndroidWindow& other);
      ~AndroidWindow();

      operator bool           () const { return ptr!=nullptr; }
      operator ANativeWindow* ()       { return ptr; }

      AndroidWindow& operator = (const AndroidWindow &other);

    private:
      ANativeWindow* ptr=nullptr;
    };
  }
}

#endif

#endif // JNIEXTRAS_H
