#ifndef __AC_ASSERT_H__
#define __AC_ASSERT_H__

#include <stdio.h>
#ifndef __ASSERT_H__
#define __ASSERT_H__
#include <assert.h>
#endif

#ifdef __AC_NAMESPACE
namespace __AC_NAMESPACE {
#endif

namespace ac {
  inline void ac_assert(const char *filename, int lineno, const char *expr_str, bool expr) {
#if !defined(CCS_SCVERIFY) && !defined(__SYNTHESIS__)
    if ( ! expr )
      printf("%s: %d: %s: ASSERTION VIOLATION\n", filename, lineno, expr_str);
#endif // endif __SYNTHESIS__
  }
  inline void ac_cover(const char *filename, int lineno, const char *expr_str, bool expr) {
#if !defined(CCS_SCVERIFY) && !defined(__SYNTHESIS__)
    if ( expr )
      printf("%s: %d: %s: COVERAGE ENCOUNTERED\n", filename, lineno, expr_str);
#endif // endif __SYNTHESIS__
  }
}

#ifdef __AC_NAMESPACE
}
#endif

// Replace macros during SYNTHESIS and SCVERIFY
// Leave original simulation untouched
#if defined(CCS_SCVERIFY) || defined(__SYNTHESIS__)
#ifndef CALYPTO_SC
#define assert(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
#define sc_assert(expr) ac::ac_assert(__FILE__, __LINE__, #expr, expr)
#endif // CALYPTO_SC
#endif // CCS_SCVERIFY || __SYNTHESIS__
#define cover(expr) ac::ac_cover(__FILE__, __LINE__, #expr, expr)
#endif // endif __AC_ASSERT_H__
