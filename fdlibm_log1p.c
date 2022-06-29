// FDLIBM log1p().
//
// Copyright (C) 2022 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// Analects of fdlibm's log1p() implementation, gathered from relevant
// portions of the fdlibm sources on 2022-05-20. URL for the original:
//    https://www.netlib.org/cgi-bin/netlibfiles.txt?format=txt&filename=fdlibm%2Fs_log1p.c
// Adapted for lmi by GWC; any defects introduced should not reflect
// on the reputations of the original authors.

#include "fdlibm.hpp"

#if defined LMI_GCC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wsign-conversion"
#endif // defined LMI_GCC

/* @(#)s_log1p.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

// $NetBSD: s_log1p.c,v 1.8 1995/05/10 20:47:46 jtc Exp $
/* Modified by Naohiko Shimizu/Tokai University, Japan 1997/08/25,
   for performance improvement on pipelined processors.
// https://sourceware.org/git/?p=glibc.git;a=blobdiff;f=sysdeps/libm-ieee754/s_log1p.c;h=4ca01b1ff555693a3fa3459df82bef1c24c82668;hp=086c0dce6c118e2364aa883d0997d32493d43a97;hb=923609d1497f3116d57b297e3e84fc07b2b15b20;hpb=0d9f67937f0c9329c35c2c0d15848ab8316dc520
*/

/* double log1p(double x)
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *                      1+x = 2^k * (1+f),
 *         where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *      Note. If k=0, then f=x is exact. However, if k!=0, then f
 *      may not be representable exactly. In that case, a correction
 *      term is need. Let u=1+x rounded. Let c = (1+x)-u, then
 *      log(1+x) - log(u) ~ c/u. Thus, we proceed to compute log(u),
 *      and add back the correction term c/u.
 *      (Note: when x > 2**53, one can simply return log(x))
 *
 *   2. Approximation of log1p(f).
 *      Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *            = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *            = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 *      a polynomial of degree 14 to approximate R The maximum error
 *      of this polynomial approximation is bounded by 2**-58.45. In
 *      other words,
 *                      2      4      6      8      10      12      14
 *          R(z) ~ Lp1*s +Lp2*s +Lp3*s +Lp4*s +Lp5*s  +Lp6*s  +Lp7*s
 *      (the values of Lp1 to Lp7 are listed in the program)
 *      and
 *          |      2          14          |     -58.45
 *          | Lp1*s +...+Lp7*s    -  R(z) | <= 2
 *          |                             |
 *      Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *      In order to guarantee error in log below 1ulp, we compute log
 *      by
 *              log1p(f) = f - (hfsq - s*(hfsq+R)).
 *
 *      3. Finally, log1p(x) = k*ln2 + log1p(f).
 *                           = k*ln2_hi+(f-(hfsq-(s*(hfsq+R)+k*ln2_lo)))
 *      Here ln2 is split into two floating point number:
 *           ln2_hi + ln2_lo,
 *      where n*ln2_hi is always exact for |n| < 2000.
 *
 * Special cases:
 *      log1p(x) is NaN with signal if x < -1 (including -INF) ;
 *      log1p(+INF) is +INF; log1p(-1) is -INF with signal;
 *      log1p(NaN) is that NaN with no signal.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 *
 * Note: Assuming log() return accurate answer, the following
 *       algorithm can be used to compute log1p(x) to within a few ULP:
 *
 *              u = 1+x;
 *              if(u==1.0) return x ; else
 *                         return log(u)*(x/(u-1.0));
 *
 *       See HP-15C Advanced Functions Handbook, p.193.
 */

static const double
ln2_hi  =  6.93147180369123816490e-01, // 3fe62e42 fee00000
ln2_lo  =  1.90821492927058770002e-10, // 3dea39ef 35793c76
two54   =  1.80143985094819840000e+16, // 43500000 00000000
Lp[] = {0.0 // not used
       ,6.666666666666735130e-01 // 3FE55555 55555593
       ,3.999999999940941908e-01 // 3FD99999 9997FA04
       ,2.857142874366239149e-01 // 3FD24924 94229359
       ,2.222219843214978396e-01 // 3FCC71C5 1D8E78AF
       ,1.818357216161805012e-01 // 3FC74664 96CB03DE
       ,1.531383769920937332e-01 // 3FC39A09 D078C69F
       ,1.479819860511658591e-01 // 3FC2F112 DF3E5244
};

#define Lp1 Lp[1]
#define Lp2 Lp[2]
#define Lp3 Lp[3]
#define Lp4 Lp[4]
#define Lp5 Lp[5]
#define Lp6 Lp[6]
#define Lp7 Lp[7]

static double zero = 0.0;

double fdlibm_log1p(double x)
{
    double hfsq=0,f=0,c=0,s=0,z=0,R=0,u=0,z2=0,z4=0,z6=0,R1=0,R2=0,R3=0,R4=0;
    int32_t k=0,hx=0,hu=0,ax=0;

    hx = hi_int(x);                             // high word of x
    ax = hx&0x7fffffff;

    k = 1;
    if (hx < 0x3FDA827A) {                      // x < 0.41422
        if(ax>=0x3ff00000) {                    // x <= -1.0
            if(x==-1.0) return -two54/zero;     // log1p(-1)=+inf
            else return (x-x)/(x-x);            // log1p(x<-1)=NaN
        }
        if(ax<0x3e200000) {                     // |x| < 2**-29
            if(two54+x>zero                     // raise inexact
                    &&ax<0x3c900000)            // |x| < 2**-54
                return x;
            else
                return x - x*x*0.5;
        }
        if(hx>0||hx<=((int32_t)0xbfd2bec3)) {
            k=0;f=x;hu=1;                       // -0.2929<x<0.41422
        }
    }
    if (hx >= 0x7ff00000) return x+x;
    if(k!=0) {
        if(hx<0x43400000) {
            u  = 1.0+x;
            hu = hi_int(u);                     // high word of u
            k  = (hu>>20)-1023;
            c  = (k>0)? 1.0-(u-x):x-(u-1.0);    // correction term
            c /= u;
        } else {
            u  = x;
            hu = hi_int(u);                     // high word of u
            k  = (hu>>20)-1023;
            c  = 0;
        }
        hu &= 0x000fffff;
        if(hu<0x6a09e) {
            SET_HIGH_WORD(u, hu|0x3ff00000);    // normalize u
        } else {
            k += 1;
            SET_HIGH_WORD(u, hu|0x3fe00000);    // normalize u/2
            hu = (0x00100000-hu)>>2;
        }
        f = u-1.0;
    }
    hfsq=0.5*f*f;
    if(hu==0) {                                 // |f| < 2**-20
        if(f==zero) {
            if(k==0) return zero;
            else {c += k*ln2_lo; return k*ln2_hi+c;}
        }
        R = hfsq*(1.0-0.66666666666666666*f);
        if(k==0) return f-R; else
                 return k*ln2_hi-((R-(k*ln2_lo+c))-f);
    }
    s = f/(2.0+f);
    z = s*s;
//  performance improvement: Naohiko Shimizu 19970825
//  R = z*(Lp1+z*(Lp2+z*(Lp3+z*(Lp4+z*(Lp5+z*(Lp6+z*Lp7))))));
    R1 =     z*Lp1; z2=z*z;
    R2 = Lp2+z*Lp3; z4=z2*z2;
    R3 = Lp4+z*Lp5; z6=z4*z2;
    R4 = Lp6+z*Lp7;
    R = R1 + z2*R2 + z4*R3 + z6*R4;
    if(k==0) return f-(hfsq-s*(hfsq+R)); else
             return k*ln2_hi-((hfsq-(s*(hfsq+R)+(k*ln2_lo+c)))-f);
}
#if defined LMI_GCC
#   pragma GCC diagnostic pop
#endif // defined LMI_GCC
