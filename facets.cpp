// Custom facets.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "facets.hpp"

#include <algorithm>                    // copy()

// Class C_is_not_whitespace_ctype is intended for extracting
// std::strings from a stream without stopping at char <C>. It is a
// derived work based on Dietmar Kuehl's article
//   http://groups.google.com/groups?selm=7vpld2%24jj%241%40nnrp1.deja.com
// which bears no copyright notice, as is usual in usenet.
//
// Ross Smith points out
//   http://groups.google.com/groups?selm=3820A76F.3952E808%40ihug.co.nz
// that ' ' can be used as an array index even if type 'char' is
// signed, because 2.2/3 requires it to have a positive value.
// The template parameter is specifically unsigned to avoid compiler
// warnings about [] with a negative index.
//
// GWC modified this class
//   in 2004 (see Motivation below)
// and in any later year shown above; any defect in it should not
// reflect on Dietmar Kuehl's or Ross Smith's reputation.

// Motivation
//
// It seems curious that std::locale takes ownership of a facet passed
// as a naked pointer [22.1.1.2/12]. The default arguments in that
// paragraph and, e.g., 22.2.1.3.2/2-3 encourage applications programs
// to create facet objects with operator new without writing any
// corresponding operator delete. Perhaps I'm prejudiced against this
// because I at first misunderstood it, as described here:
//   http://www.google.com/groups?selm=411eec7b%40newsgroups.borland.com
// but I believe it's a sound habit either to pair new and delete
// calls carefully; or, far better, to write them only in library code
// such as this file and never in applications code, unless required
// by pre-modern third-party libraries such as wxWindows; or, best of
// all, to write them not at all--and that practice works so well with
// the C++ standard library in general that an exception for this case
// just seems ugly.
//
// This custom facet, therefore, passes std::ctype<char>'s constructor
// a third argument that differs from the default, specifying instead
// that the locale should not take responsibility for deletion.
//
// Implementation note--with this original line:
//   rc[C] &= ~std::ctype_base::space;
// gcc-7.3 warns:
//   conversion to 'std::ctype_base::mask {aka short unsigned int}' from 'int'
// which seems incorrect: according to C++17 (N4659),
//   [category.ctype]
// 'space' is of type 'mask', a bitmask type for which
//   [bitmask.types]
// operator~(bitmask) returns the bitmask type, and
// operator&=() returns a reference to the bitmask type.
//
// But compare this defect report:
//   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3110.html
// "Bitmask types need operator~ in order to clear a value from the bitmask
// type, as show [sic] in 17.5.2.1.3 paragraph 4:
// - To clear a value Y in an object X is to evaluate the expression X &= ~Y.
// However the definition for fmtflags above does not have well-specified
// behaviour if the underlying type is smaller than int, because ~int(f) is
// likely to produce a value outside the range of fmtflags."
//
// The underlying problem is that 'short unsigned int' does not fulfill the
// bitmask requirements, as unary operator ~ performs integral promotions.
//
// This:
//   rc[C] &= static_cast<std::ctype_base::mask>(~std::ctype_base::space);
// is the least awful workaround that avoids the warning.

namespace
{
    template<unsigned char C>
    class C_is_not_whitespace_ctype: public std::ctype<char>
    {
      public:
        C_is_not_whitespace_ctype()
            :std::ctype<char>(get_table(), false, 1)
            {}

      private:
        static std::ctype_base::mask const* get_table()
            {
            static std::ctype_base::mask rc[table_size];
            std::copy(classic_table(), classic_table() + table_size, rc);
            // See "Implementation note" above.
            rc[C] &= static_cast<std::ctype_base::mask>(~std::ctype_base::space);
            return rc;
            }
    };

    template<typename Facet>
    std::locale const& locale_with_facet()
    {
        static Facet f;
        static std::locale custom_locale = std::locale(std::locale(), &f);
        return custom_locale;
    }
} // Unnamed namespace.

// Convenience interfaces.

std::locale const& blank_is_not_whitespace_locale()
{
    return locale_with_facet<C_is_not_whitespace_ctype<' '>>();
}

std::locale const& tab_is_not_whitespace_locale()
{
    return locale_with_facet<C_is_not_whitespace_ctype<'\t'>>();
}
