// FDLIBM expm1().
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

// Analects of fdlibm's expm1() implementation, gathered from relevant
// portions of the fdlibm sources on 2022-05-20. URL for the original:
//    https://www.netlib.org/cgi-bin/netlibfiles.txt?format=txt&filename=fdlibm%2Fs_expm1.c
// Adapted for lmi by GWC; any defects introduced should not reflect
// on the reputations of the original authors.

#include "fdlibm.hpp"

#if defined LMI_GCC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wfloat-conversion"
#   pragma GCC diagnostic ignored "-Wsign-conversion"
#endif // defined LMI_GCC

/* @(#)s_expm1.c 1.5 04/04/22 */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

// $NetBSD: s_expm1.c,v 1.8 1995/05/10 20:47:09 jtc Exp $
/* Modified by Naohiko Shimizu/Tokai University, Japan 1997/08/25,
   for performance improvement on pipelined processors.
// https://sourceware.org/git/?p=glibc.git;a=blobdiff;f=sysdeps/libm-ieee754/s_expm1.c;h=ed1aba527be837d4ed7e65e9c86b3fc3e14e65d7;hp=c8354b7262686370a761494dc785bb59fdf45477;hb=923609d1497f3116d57b297e3e84fc07b2b15b20;hpb=0d9f67937f0c9329c35c2c0d15848ab8316dc520
*/

/* expm1(x)
 * Returns exp(x)-1, the exponential of x minus 1.
 *
 * Method
 *   1. Argument reduction:
 *      Given x, find r and integer k such that
 *
 *               x = k*ln2 + r,  |r| <= 0.5*ln2 ~ 0.34658
 *
 *      Here a correction term c will be computed to compensate
 *      the error in r when rounded to a floating-point number.
 *
 *   2. Approximating expm1(r) by a special rational function on
 *      the interval [0,0.34658]:
 *      Since
 *          r*(exp(r)+1)/(exp(r)-1) = 2+ r^2/6 - r^4/360 + ...
 *      we define R1(r*r) by
 *          r*(exp(r)+1)/(exp(r)-1) = 2+ r^2/6 * R1(r*r)
 *      That is,
 *          R1(r**2) = 6/r *((exp(r)+1)/(exp(r)-1) - 2/r)
 *                   = 6/r * ( 1 + 2.0*(1/(exp(r)-1) - 1/r))
 *                   = 1 - r^2/60 + r^4/2520 - r^6/100800 + ...
 *      We use a special Remes algorithm on [0,0.347] to generate
 *      a polynomial of degree 5 in r*r to approximate R1. The
 *      maximum error of this polynomial approximation is bounded
 *      by 2**-61. In other words,
 *          R1(z) ~ 1.0 + Q1*z + Q2*z**2 + Q3*z**3 + Q4*z**4 + Q5*z**5
 *      where  Q1  =  -1.6666666666666567384E-2,
 *             Q2  =   3.9682539681370365873E-4,
 *             Q3  =  -9.9206344733435987357E-6,
 *             Q4  =   2.5051361420808517002E-7,
 *             Q5  =  -6.2843505682382617102E-9;
 *      (where z=r*r, and the values of Q1 to Q5 are listed below)
 *      with error bounded by
 *          |                  5           |     -61
 *          | 1.0+Q1*z+...+Q5*z   -  R1(z) | <= 2
 *          |                              |
 *
 *      expm1(r) = exp(r)-1 is then computed by the following
 *      specific way which minimize the accumulation rounding error:
 *                             2     3
 *                            r     r    [ 3 - (R1 + R1*r/2)  ]
 *            expm1(r) = r + --- + --- * [--------------------]
 *                            2     2    [ 6 - r*(3 - R1*r/2) ]
 *
 *      To compensate the error in the argument reduction, we use
 *              expm1(r+c) = expm1(r) + c + expm1(r)*c
 *                         ~ expm1(r) + c + r*c
 *      Thus c+r*c will be added in as the correction terms for
 *      expm1(r+c). Now rearrange the term to avoid optimization
 *      screw up:
 *                      (      2                                    2 )
 *                      ({  ( r    [ R1 -  (3 - R1*r/2) ]  )  }    r  )
 *       expm1(r+c)~r - ({r*(--- * [--------------------]-c)-c} - --- )
 *                      ({  ( 2    [ 6 - r*(3 - R1*r/2) ]  )  }    2  )
 *                      (                                             )
 *
 *                 = r - E
 *   3. Scale back to obtain expm1(x):
 *      From step 1, we have
 *         expm1(x) = either 2^k*[expm1(r)+1] - 1
 *                  = or     2^k*[expm1(r) + (1-2^-k)]
 *   4. Implementation notes:
 *      (A). To save one multiplication, we scale the coefficient Qi
 *           to Qi*2^i, and replace z by (x^2)/2.
 *      (B). To achieve maximum accuracy, we compute expm1(x) by
 *        (i)   if x < -56*ln2, return -1.0, (raise inexact if x!=inf)
 *        (ii)  if k=0, return r-E
 *        (iii) if k=-1, return 0.5*(r-E)-0.5
 *        (iv)  if k=1 if r < -0.25, return 2*((r+0.5)- E)
 *                     else          return  1.0+2.0*(r-E);
 *        (v)   if (k<-2||k>56) return 2^k(1-(E-r)) - 1 (or exp(x)-1)
 *        (vi)  if k <= 20, return 2^k((1-2^-k)-(E-r)), else
 *        (vii) return 2^k(1-((E+2^-k)-r))
 *
 * Special cases:
 *      expm1(INF) is INF, expm1(NaN) is NaN;
 *      expm1(-INF) is -1, and
 *      for finite argument, only expm1(0)=0 is exact.
 *
 * Accuracy:
 *      according to an error analysis, the error is always less than
 *      1 ulp (unit in the last place).
 *
 * Misc. info.
 *      For IEEE double
 *          if x >  7.09782712893383973096e+02 then expm1(x) overflow
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double
huge        = 1.0e+300,
tiny        = 1.0e-300,
o_threshold = 7.09782712893383973096e+02, // 0x40862E42, 0xFEFA39EF
ln2_hi      = 6.93147180369123816490e-01, // 0x3fe62e42, 0xfee00000
ln2_lo      = 1.90821492927058770002e-10, // 0x3dea39ef, 0x35793c76
invln2      = 1.44269504088896338700e+00, // 0x3ff71547, 0x652b82fe
        // scaled coefficients related to expm1
Q[] = { 1.0
      ,-3.33333333333331316428e-02 // BFA11111 111110F4
      , 1.58730158725481460165e-03 // 3F5A01A0 19FE5585
      ,-7.93650757867487942473e-05 // BF14CE19 9EAADBB7
      , 4.00821782732936239552e-06 // 3ED0CFCA 86E65239
      ,-2.01099218183624371326e-07 // BE8AFDB7 6E09C32D
};

#define one Q[0]
#define Q1  Q[1]
#define Q2  Q[2]
#define Q3  Q[3]
#define Q4  Q[4]
#define Q5  Q[5]

double fdlibm_expm1(double x)
{
    double y=0,hi=0,lo=0,c=0,t=0,e=0,hxs=0,hfx=0,r1=0,h2=0,h4=0,R1=0,R2=0,R3=0;
    int32_t k=0,xsb=0;
    uint32_t hx=0;

    hx  = hi_uint(x);           // high word of x
    xsb = hx&0x80000000;        // sign bit of x
    if(xsb==0) y=x; else y= -x; // y = |x|
    hx &= 0x7fffffff;           // high word of |x|

    // filter out huge and non-finite argument
    if(hx >= 0x4043687A) {      // if |x|>=56*ln2
        if(hx >= 0x40862E42) {  // if |x|>=709.78...
          if(hx>=0x7ff00000) {
            if(((hx&0xfffff)|lo_uint(x))!=0)
                 return x+x;    // NaN
            else return (xsb==0)? x:-1.0; // exp(+-inf)={inf,-1}
          }
            if(x > o_threshold) return huge*huge; // overflow
        }
        if(xsb!=0) {            // x < -56*ln2, return -1.0 with inexact
            if(x+tiny<0.0)      // raise inexact
            return tiny-one;    // return -1
        }
    }

    // argument reduction
    if(hx > 0x3fd62e42) {       // if  |x| > 0.5 ln2
        if(hx < 0x3FF0A2B2) {   // and |x| < 1.5 ln2
            if(xsb==0)
                {hi = x - ln2_hi; lo =  ln2_lo;  k =  1;}
            else
                {hi = x + ln2_hi; lo = -ln2_lo;  k = -1;}
        } else {
        k  = invln2*x+((xsb==0)?0.5:-0.5);
        t  = k;
        hi = x - t*ln2_hi;      // t*ln2_hi is exact here
        lo = t*ln2_lo;
        }
        x  = hi - lo;
        c  = (hi-x)-lo;
    }
    else if(hx < 0x3c900000) {  // when |x|<2**-54, return x
        t = huge+x;             // return x with inexact flags when x!=0
        return x - (t-(huge+x));
    }
    else k = 0;

    // x is now in primary range
    hfx = 0.5*x;
    hxs = x*hfx;
//  performance improvement: Naohiko Shimizu 19970825
//  r1 = one+hxs*(Q1+hxs*(Q2+hxs*(Q3+hxs*(Q4+hxs*Q5))));
    R1 = one+hxs*Q1; h2 = hxs*hxs;
    R2 = Q2+hxs*Q3; h4 = h2*h2;
    R3 = Q4+hxs*Q5;
    r1 = R1 + h2*R2 + h4*R3;
    t  = 3.0-r1*hfx;
    e  = hxs*((r1-t)/(6.0 - x*t));
    if(k==0) return x - (x*e-hxs); // c is 0
    else {
        e  = (x*(e-c)-c);
        e -= hxs;
        if(k== -1) return 0.5*(x-e)-0.5;
        if(k==1) {
            if(x < -0.25) return -2.0*(e-(x+0.5));
            else           return  one+2.0*(x-e);
        }
        if (k <= -2 || k>56) {     // suffice to return exp(x)-1
            y = one-(e-x);
            // add k to y's exponent, casting to avoid undefined
            // behavior (left shift of a negative signed integer)
            SET_HIGH_WORD(y, hi_uint(y) + (int32_t)(((uint32_t)k)<<20));
            return y-one;
        }
        t = one;
        if(k<20) {
               SET_HIGH_WORD(t, 0x3ff00000 - (0x200000>>k)); // t=1-2^-k
               y = t-(e-x);
               SET_HIGH_WORD(y, hi_uint(y) + (k<<20)); // add k to y's exponent
        } else {
               SET_HIGH_WORD(t, (0x3ff-k)<<20);        // 2^-k
               y = x-(e+t);
               y += one;
               SET_HIGH_WORD(y, hi_uint(y) + (k<<20)); // add k to y's exponent
        }
    }
    return y;
}
#if defined LMI_GCC
#   pragma GCC diagnostic pop
#endif // defined LMI_GCC
