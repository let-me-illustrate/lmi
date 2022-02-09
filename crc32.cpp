// 32-bit cyclic redundancy check.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "crc32.hpp"

#include "ssize_lmi.hpp"

#include <cstdint>

// This is a derived work based on Mark Adler's original 'makecrc.c',
// extracted from gnu gzip
//   gnu/gzip/gzip-1.2.4a.tar.gz
// where it is captioned
//   "Not copyrighted 1990 Mark Adler".
//
// On 2005-01-13, Gregory W. Chicares rewrote Mark Adler's original
// code to work as part of this CRC class. Any defect here should not
// reflect on Mark Adler's reputation.
//
// Here is Mark Adler's original documentation, copied from the gzip
// sources, trivially reformatted by GWC:
//
// [Mark Adler's original documentation begins.]
//
// Generate a table for a byte-wise 32-bit CRC calculation on the polynomial:
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.
//
// Polynomials over GF(2) are represented in binary, one bit per coefficient,
// with the lowest powers in the most significant bit.  Then adding polynomials
// is just exclusive-or, and multiplying a polynomial by x is a right shift by
// one.  If we call the above polynomial p, and represent a byte as the
// polynomial q, also with the lowest power in the most significant bit (so the
// byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is (q*x^32) mod p,
// where a mod b means the remainder after dividing a by b.
//
// This calculation is done using the shift-register method of multiplying and
// taking the remainder.  The register is initialized to zero, and for each
// incoming bit, x^32 is added mod p to the register if the bit is a one (where
// x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
// x (which is shifting right by one and adding x^32 mod p if the bit shifted
// out is a one).  We start with the highest power (least significant bit) of
// q and repeat for all eight bits of q.
//
// The table is simply the CRC of all possible eight bit values.  This is all
// the information needed to generate CRC's on data a byte at a time for all
// combinations of CRC register values and incoming bytes.  The table is
// written to stdout as 256 long-int hexadecimal values in C language format.
//
// [Mark Adler's original documentation ends.]
//
// GWC changed the original code to write the table to a variable
// instead of printing it to a table.
//
namespace
{
    std::vector<unsigned int> const& make_table_0xedb88320U()
    {
        static int const n = 256;
        static unsigned int crc_array[n];

        std::uint32_t c; // CRC shift register.
        std::uint32_t e; // Polynomial exclusive-or pattern.
        int i;           // Counter for all possible eight bit values.
        int k;           // Byte being shifted into crc apparatus.

        // Terms of polynomial defining this crc (except x^32).
        static int p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

        // Make exclusive-or pattern from polynomial (0xedb88320).
        e = 0;
        for(i = 0; i < lmi::ssize(p); ++i)
            {
            e |= std::uint32_t(1) << (31 - p[i]);
            }

        for(i = 1; i < 256; ++i)
            {
            c = i;
            // The idea to initialize the register with the byte instead
            // of zero was stolen from Haruhiko Okumura's ar002.
            for(k = 8; k; --k)
                {
                c = c & 1 ? (c >> 1) ^ e : c >> 1;
                }
            crc_array[i] = c;
            }

        static std::vector<unsigned int> const crc_vector(crc_array, crc_array + n);
        return crc_vector;
    }

    std::vector<unsigned int> const& crc_table()
        {
        static std::vector<unsigned int> const v = make_table_0xedb88320U();
        return v;
        }
} // Unnamed namespace.

//============================================================================
CRC::CRC()
    :value_ {0xffffffffU}
{
}

// This function is a derived work based on Gary S. Brown's original,
// which is used in many free programs including gnu parted and gnu
// commoncpp. Many others have implemented it, sometimes with
// restrictions that make their implementations non-free, but with no
// effect on the original's freedom. See, for example,
//   http://ecos.sourceware.org/ml/ecos-maintainers/2004-08/msg00012.html
//   http://lists.gnu.org/archive/html/bug-parted/2000-11/msg00140.html
// On 2005-01-13, Gregory W. Chicares rewrote Gary S. Brown's original
// code to work as part of this CRC class. Any defect here should not
// reflect on Gary S. Brown's reputation.
//
CRC& CRC::update
    (unsigned char const*  buf
    ,int                   len
    )
{

    for(int j = 0; j < len; ++j)
        {
        value_ = crc_table()[(value_ ^ buf[j]) & 0xff] ^ (value_ >> 8);
        }
    return *this;
}

//============================================================================
unsigned int CRC::value() const
{
    return value_ ^ 0xffffffffU;
}

//============================================================================
CRC& CRC::operator+=(std::string const& z)
{
    for(auto const& j : z)
        {
        operator+=(j);
        }
    return *this;
}
