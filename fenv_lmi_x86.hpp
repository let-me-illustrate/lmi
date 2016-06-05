// Manage floating-point environment: x86 implementation details.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef fenv_lmi_x86_hpp
#define fenv_lmi_x86_hpp

#include "config.hpp"

#include <bitset>
#include <stdexcept>

#if defined __BORLANDC__ || defined _MSC_VER
#   include <float.h>                   // nonstandard _control87()
#endif // defined __BORLANDC__ || defined _MSC_VER

#if defined LMI_X86
/// These functions manipulate the x86 fpu (80x87) control word. This
/// shouldn't be as difficult as it actually is. Part of the problem
/// is that C was strangely slow to adopt sophisticated numerics:
///   1980: 8087 introduced
///   1985: IEEE 754
///   1999: C finally gets fesetenv(), but without precision control *
/// which left setting the control word to nonstandard routines or
/// nonportable asm, and part is that function _control87() in the
/// widely-used ms C rtl takes an argument that differs gratuitously
/// from the hardware control word--see, e.g.:
///   http://groups.google.com/groups?selm=34775BB8.E10BA020%40tc.umn.edu
///
/// The x87 fpu control word defines these bits:
///   IM mask: invalid operation
///   DM mask: denormalized operand
///   ZM mask: zero divide
///   OM mask: overflow
///   UM mask: underflow
///   PM mask: precision
///   PC precision control
///   RC rounding control
/// Bit 12 is IC, the infinity-control bit, which is meaningful only
/// for the 8087 and 80287, but settable (and ignored) on later chips.
///
/// * "without precision control"
/// According to Goldberg:
///   http://docs.sun.com/source/806-3568/ncg_goldberg.html#4130
/// "fegetprec and fesetprec functions" were recommended in early
/// drafts, but "this recommendation was removed before the changes
/// were made to the C99 standard." The Rationale:
///   www.open-std.org/jtc1/sc22/wg14/www/C99RationaleV5.10.pdf
/// suggests that IEC 60559 is "ambivalent" as to whether precision
/// control must be dynamic. Yet IEEE 754r Draft 1.2.5 [G.2] says:
/// "changing the rounding direction or precision during execution may
/// help identify subprograms that are unusually sensitive to roundoff"

namespace fenv_lmi_x86 {} // doxygen workaround.

/// The hardware control word, represented as bitfields.

struct intel_control_word_for_exposition_only
{
    unsigned int IM:1;  // Bit  0.
    unsigned int DM:1;  // Bit  1.
    unsigned int ZM:1;  // Bit  2.
    unsigned int OM:1;  // Bit  3.
    unsigned int UM:1;  // Bit  4.
    unsigned int PM:1;  // Bit  5.
    unsigned int   :2;  // Bits 6-7.
    unsigned int PC:2;  // Bits 8-9: 00, 10, 11 --> 24, 53, 64 .
    unsigned int RC:2;  // Bits 10-11: 00, 01, 10, 11 --> near, down, up, trunc.
    unsigned int   :4;  // Bits 12-15.
};

/// The ms C rtl control word, represented as bitfields.

struct msw_control_word_for_exposition_only
{
    unsigned int PM:1;  // Bit  0.
    unsigned int UM:1;  // Bit  1.
    unsigned int OM:1;  // Bit  2.
    unsigned int ZM:1;  // Bit  3.
    unsigned int IM:1;  // Bit  4.
    unsigned int   :3;  // Bits 5-7: reserved.
    unsigned int RC:2;  // Bits 8-9: 00, 01, 10, 11 --> near, down, up, trunc.
    unsigned int   :6;  // Bits 10-15: reserved.
    unsigned int PC:2;  // Bits 16-17: 10, 01, 00 --> 24, 53, 64 .
    unsigned int   :1;  // Bit  18: reserved.
    unsigned int DM:1;  // Bit  19.
    unsigned int   :12; // Bits 20-31: reserved.
};

/// Precision-control values used by 80x87 hardware. The enumerators
/// are lowercase versions of the GNU/Linux <fenvwm.h> macros,
/// although the constant-expressions may differ.

enum e_ieee754_precision
    {fe_fltprec  = 0x00
    ,fe_dblprec  = 0x02
    ,fe_ldblprec = 0x03
    };

/// Precision-control values used by ms C rtl, which differ
/// gratuitously from hardware values.

enum e_msw_pc
    {msw_fltprec  = 0x02
    ,msw_dblprec  = 0x01
    ,msw_ldblprec = 0x00
    };

/// Rounding-control values. The enumerators are lowercase versions of
/// the C99 7.6/7 macros, although the constant-expressions may differ.

enum e_ieee754_rounding
    {fe_tonearest  = 0x00
    ,fe_downward   = 0x01
    ,fe_upward     = 0x02
    ,fe_towardzero = 0x03
    };

/// Parameters of 80x87 hardware control word.
///
/// Reserved bits:
///   'reserved_values' specifies values of reserved bits.
///   'settable' masks reserved bits.
/// These two variables are used to prevent setting reserved bits, and
/// to ensure that default settings exactly match those set by the
/// hardware at initialization with FINIT.

struct intel_control_word_parameters
{
    typedef unsigned short int integer_type;
    enum {nbits = 16};
    typedef e_ieee754_precision pc_type;
    typedef e_ieee754_rounding  rc_type;
    enum {reserved_values = 0x0040};
    enum {settable = 0x0f3f};
    enum {im_bit = 0};
    enum {dm_bit = 1};
    enum {zm_bit = 2};
    enum {om_bit = 3};
    enum {um_bit = 4};
    enum {pm_bit = 5};
    enum {pc_bit0 = 8};
    enum {rc_bit0 = 10};
    static pc_type pc24() {return fe_fltprec ;}
    static pc_type pc53() {return fe_dblprec ;}
    static pc_type pc64() {return fe_ldblprec;}
    static pc_type pcerror() {throw std::logic_error("Invalid fpu PC value.");}
};

/// Parameters of ms C rtl control word, which differ gratuitously
/// from hardware parameters.
///
/// 'settable' in terms of the ms macros is
///   _MCW_EM | _MCW_RC | _MCW_PC
/// but without regard to _MCW_IC, setting which has no meaning for
/// any hardware that supports msw.
///
/// 'reserved_values' can have no meaning here, for ms documents none.

struct msw_control_word_parameters
{
    typedef unsigned int integer_type;
    enum {nbits = 32};
    typedef e_msw_pc           pc_type;
    typedef e_ieee754_rounding rc_type;
    enum {reserved_values = 0x0000};
    enum {settable = 0x000b031f};
    enum {im_bit = 4};
    enum {dm_bit = 19};
    enum {zm_bit = 3};
    enum {om_bit = 2};
    enum {um_bit = 1};
    enum {pm_bit = 0};
    enum {pc_bit0 = 16};
    enum {rc_bit0 = 8};
    static pc_type pc24() {return msw_fltprec ;}
    static pc_type pc53() {return msw_dblprec ;}
    static pc_type pc64() {return msw_ldblprec;}
    static pc_type pcerror() {throw std::logic_error("Invalid fpu PC value.");}
};

/// Generic representation of 80x87 control word. The template
/// parameter selects between the hardware control word and the
/// gratuitously-different one used by the ms C rtl.

template<typename ControlWordType>
class control_word
{
    template<typename T> friend class control_word;
    friend int test_main(int, char*[]);

    typedef typename ControlWordType::integer_type integer_type;
    typedef typename ControlWordType::pc_type pc_type;
    typedef typename ControlWordType::rc_type rc_type;
    typedef typename std::bitset<ControlWordType::nbits>::reference ref_type;

  public:
    control_word(int w)
        {
        cw_ = ControlWordType::reserved_values | ControlWordType::settable & w;
        }

    template<typename X>
    control_word(control_word<X> x)
        :cw_(ControlWordType::reserved_values)
        {
        im() = x.im();
        dm() = x.dm();
        zm() = x.zm();
        om() = x.om();
        um() = x.um();
        pm() = x.pm();
        pc
            ((X::pc24() == x.pc()) ? ControlWordType::pc24()
            :(X::pc53() == x.pc()) ? ControlWordType::pc53()
            :(X::pc64() == x.pc()) ? ControlWordType::pc64()
            :                        ControlWordType::pcerror()
            );
        rc(x.rc());
        }

    void pc(pc_type e) {pc1() = 0x02 & e; pc0() = 0x01 & e;}
    void rc(rc_type e) {rc1() = 0x02 & e; rc0() = 0x01 & e;}
    pc_type pc() const {return pc_type(pc0() + 2 * pc1());}
    rc_type rc() const {return rc_type(rc0() + 2 * rc1());}

    integer_type cw() const {return static_cast<integer_type>(cw_.to_ulong());}
    std::bitset<ControlWordType::nbits> const& bits() const {return cw_;}

  private:
    ref_type im()     {return cw_[ControlWordType::im_bit];}
    ref_type dm()     {return cw_[ControlWordType::dm_bit];}
    ref_type zm()     {return cw_[ControlWordType::zm_bit];}
    ref_type om()     {return cw_[ControlWordType::om_bit];}
    ref_type um()     {return cw_[ControlWordType::um_bit];}
    ref_type pm()     {return cw_[ControlWordType::pm_bit];}
    ref_type pc0()    {return cw_[0 + ControlWordType::pc_bit0];}
    ref_type pc1()    {return cw_[1 + ControlWordType::pc_bit0];}
    ref_type rc0()    {return cw_[0 + ControlWordType::rc_bit0];}
    ref_type rc1()    {return cw_[1 + ControlWordType::rc_bit0];}

    // Not every implementation yet has operator[](std::size_t) const
    //   http://www.comeaucomputing.com/iso/lwg-defects.html#11
    // so test() is used where operator[] would suffice.

    bool  im()  const {return cw_.test(ControlWordType::im_bit);}
    bool  dm()  const {return cw_.test(ControlWordType::dm_bit);}
    bool  zm()  const {return cw_.test(ControlWordType::zm_bit);}
    bool  om()  const {return cw_.test(ControlWordType::om_bit);}
    bool  um()  const {return cw_.test(ControlWordType::um_bit);}
    bool  pm()  const {return cw_.test(ControlWordType::pm_bit);}
    bool  pc0() const {return cw_.test(0 + ControlWordType::pc_bit0);}
    bool  pc1() const {return cw_.test(1 + ControlWordType::pc_bit0);}
    bool  rc0() const {return cw_.test(0 + ControlWordType::rc_bit0);}
    bool  rc1() const {return cw_.test(1 + ControlWordType::rc_bit0);}

    std::bitset<ControlWordType::nbits> cw_;
};

typedef control_word<intel_control_word_parameters> intel_control_word;
typedef control_word<msw_control_word_parameters> msw_control_word;

// Conversion functions.

inline unsigned int intel_to_msw(intel_control_word i)
{
    return msw_control_word(i).cw();
}

inline unsigned int intel_to_msw(unsigned short int i)
{
    return intel_to_msw(intel_control_word(i));
}

inline unsigned short int msw_to_intel(msw_control_word m)
{
    return intel_control_word(m).cw();
}

inline unsigned short int msw_to_intel(unsigned int m)
{
    return msw_to_intel(msw_control_word(m));
}

/// Default settings for x87 fpu.

inline unsigned short int default_x87_control_word()
{
    return 0x037f;
}

/// Fetch settings for x87 fpu.

inline unsigned short int x87_control_word()
{
    volatile unsigned short int control_word = 0x0;
#   if defined __GNUC__
    asm volatile("fstcw %0" : : "m" (control_word));
#   elif defined __BORLANDC__
    control_word = static_cast<unsigned short int>(_control87(0, 0));
#   elif defined _MSC_VER
    // Test _MSC_VER last: some non-ms compilers or libraries define it.
    control_word = msw_to_intel(_control87(0, 0));
#   else // Unknown compiler or platform.
#       error Unknown compiler or platform.
#   endif // Unknown compiler or platform.
    return control_word;
}

/// Change settings for x87 fpu.

inline void x87_control_word(unsigned short int cw)
{
#   if defined __GNUC__
    volatile unsigned short int control_word = cw;
    asm volatile("fldcw %0" : : "m" (control_word));
#   elif defined __BORLANDC__
    _control87(cw, 0x0ffff);
#   elif defined _MSC_VER
    // Test _MSC_VER last: some non-ms compilers or libraries define it.
    _control87(intel_to_msw(cw),  0x0ffffffff);
#   else // Unknown compiler or platform.
#       error Unknown compiler or platform.
#   endif // Unknown compiler or platform.
}

#endif // LMI_X86

#endif // fenv_lmi_x86_hpp

