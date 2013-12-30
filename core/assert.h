#ifndef ASSERT_H
#define ASSERT_H

namespace Tempest{

#ifndef T_NO_DEBUG

#define T_ASSERT(X)\
  Tempest::Detail::te_assert_impl(X, __FILE__, __LINE__, #X);

#define T_ASSERT_X(X, M)\
  Tempest::Detail::te_assert_impl(X, __FILE__, __LINE__, #X, M);

#define T_WARNING(X)\
  Tempest::Detail::te_warning_impl(X, __FILE__, __LINE__, #X);

#define T_WARNING_X(X, M)\
  Tempest::Detail::te_warning_impl(X, __FILE__, __LINE__, #X, M);

#else
#define T_ASSERT(X) {}
#define T_ASSERT_X(X, M) {}
#define T_WARNING(X) {}
#define T_WARNING_X(X, M) {}
#endif

namespace Detail{
  void te_assert_impl( bool a,
                 const char* file, int line,
                 const char* X, const char * msg = 0);
  void te_warning_impl( bool a,
                 const char* file, int line,
                 const char* X, const char * msg = 0);
  }

void installAssertHandler( void (*f)( const char *, int,
                                      const char *, const char *) );

void installWarningHandler( void (*f)( const char *, int,
                                       const char *, const char *) );
}

#endif // ASSERT_H
