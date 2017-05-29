#include "jniextras.h"

#include <utility>

using namespace Tempest::Jni;

Class::Class() {
  }

Class::Class(JNIEnv& env, const char *name) : env(&env) {
  cls = env.FindClass(name);
  if( cls )
    cls = reinterpret_cast<jclass>( env.NewGlobalRef(cls) );
  }

Class::Class(Class &&other)  {
  std::swap(cls,other.cls);
  }

Class::~Class() {
  if(env && cls)
    env->DeleteGlobalRef(cls);
  }

Class& Class::operator = (Class &&other) {
  env = other.env;
  std::swap(cls,other.cls);
  return *this;
  }

bool Class::registerNatives(JNIEnv &env, const JNINativeMethod *m, size_t count) {
  if( cls ) {
    return env.RegisterNatives(cls,m,jint(count))>=0;
    }
  return false;
  }

bool Class::registerNatives(JNIEnv &env, const std::initializer_list<JNINativeMethod> &l) {
  if( l.size()==0 )
    return true;
  return registerNatives(env,&(*l.begin()),l.size());
  }


Object::Object(JNIEnv &env, const jobject obj)
  :env(&env),self(obj){
  if( self )
    self = env.NewGlobalRef(self);
  }

Object::Object(const Object &other) {
  *this = other;
  }

Object::~Object() {
  if( env && self )
    env->DeleteGlobalRef(self);
  }

Object &Object::operator =(const Object &other) {
  if( this==&other )
    return *this;

  if( env && self )
    env->DeleteGlobalRef(self);
  env  = other.env;
  self = env->NewGlobalRef(other.self);
  return *this;
  }

AndroidWindow::AndroidWindow(JNIEnv &env, jobject surface) {
  ptr = ANativeWindow_fromSurface(&env,surface);
  }

AndroidWindow::AndroidWindow(const AndroidWindow &other) {
  *this = other;
  }

AndroidWindow::~AndroidWindow() {
  if( ptr )
    ANativeWindow_release(ptr);
  }

AndroidWindow &AndroidWindow::operator =(const AndroidWindow &other) {
  if( other.ptr )
    ANativeWindow_acquire(other.ptr);

  if( ptr )
    ANativeWindow_release(ptr);

  ptr = other.ptr;
  return *this;
  }
