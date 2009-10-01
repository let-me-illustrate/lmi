/* This is public domain. It's not
 *   "Copyright (C) 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares"
 * but including that quoted string satisfies lmi's style-conformity
 * tests. Eventually this will become part of MinGW and cygwin, and
 * will then be removed from the lmi repository. Find the original at:
 *
 * http://cygwin.com/cgi-bin/cvsweb.cgi/src/winsup/mingw/mingwex/math/expm1.c?cvsroot=src
 *
 */

/*
 * Written 2005 by Gregory W. Chicares <gchicares@sbcglobal.net>.
 * Adapted to double by Danny Smith <dannysmith@users.sourceforge.net>.
 * Public domain.
 *
 * F2XM1's input is constrained to (-1, +1), so the domain of
 * 'x * LOG2EL' is (-LOGE2L, +LOGE2L). Outside that domain,
 * delegating to exp() handles C99 7.12.6.3/2 range errors.
 *
 * Constants from moshier.net, file cephes/ldouble/constl.c,
 * are used instead of M_LN2 and M_LOG2E, which would not be
 * visible with 'gcc std=c99'.  The use of these extended precision
 * constants also allows gcc to replace them with x87 opcodes.
 */

/* Begin local GWC modifications. */
#include "expm1.h"
/* End local GWC modifications. */

#include <math.h> /* expl() */
/* Begin local GWC modifications:
 * #include "cephes_mconf.h"
 */
#if !defined LMI_COMPILER_PROVIDES_EXPM1L
#   ifdef __GNUC__
long double expm1l(long double x);
#define LOGE2L  6.9314718055994530941723E-1L
#define LOG2EL  1.4426950408889634073599E0L

/*
 * http://savannah.nongnu.org/projects/lmi
 * email: <gchicares@sbcglobal.net>
 * snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
 *
 * $Id: expm1.c,v 1.12 2009-10-01 20:37:39 chicares Exp $
 *
 * End local GWC modifications. */

long double expm1l(long double x) /* Local GWC modification: type changed. */
{
  if (fabs(x) < LOGE2L)
    {
      x *= LOG2EL;
      __asm__("f2xm1" : "=t" (x) : "0" (x));
      return x;
    }
  else
    return exp(x) - 1.0;
}

double expm1(double x)
{
  if (fabs(x) < LOGE2L)
    {
      x *= LOG2EL;
      __asm__("f2xm1" : "=t" (x) : "0" (x));
      return x;
    }
  else
    return exp(x) - 1.0;
}

#define SQRT2 1.41421356237309504880L

long double log1pl(long double x)
{
  if (fabs(x) < 1.0 - 0.5 * SQRT2)
    {
      __asm__("fldln2\n\t" "fxch\n\t" "fyl2xp1" : "=t" (x) : "0" (x));
      return x;
    }
  else
    return log(1.0 + x);
}

/* COMPILER !! Apparently como compiles this file as C++ despite its '.c'
 * extension: else 'extern "C"' wouldn't be required (or permitted).
 */

/* Begin local GWC modifications. */
#   else // Not gcc.
    // COMPILER !! This workaround loses some accuracy.
#       ifdef __COMO__
            extern "C"
#       endif // __COMO__
            double expm1(double x) {return expm1l(x);}
#   endif // Not gcc.
#endif // !defined LMI_COMPILER_PROVIDES_EXPM1L

#if !defined LMI_COMPILER_PROVIDES_LOG1PL
// COMPILER !! This workaround loses some accuracy.
#       ifdef __COMO__
            extern "C"
#       endif // __COMO__
            double log1p(double x) {return log1pl(x);}
#endif // !defined LMI_COMPILER_PROVIDES_LOG1PL

/* End local GWC modifications. */

