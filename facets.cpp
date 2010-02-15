// Custom facets.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "facets.hpp"

#include <algorithm> // std::copy()

// Class blank_is_not_whitespace_ctype is intended for extracting
// std::strings from a stream without stopping at blanks. It is a
// derived work based on Dietmar Kuehl's article
//   http://groups.google.com/groups?selm=7vpld2%24jj%241%40nnrp1.deja.com
// which bears no copyright notice, as is usual in usenet.
//
// Ross Smith points out
//   http://groups.google.com/groups?selm=3820A76F.3952E808%40ihug.co.nz
// that ' ' can be used as an array index even if type 'char' is
// signed, because 2.2/3 requires it to have a positive value.
// However, gcc has a warning for 'char' array indexes that doesn't
// recognize this exception, so a workaround for "[' ']" is used here.
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

namespace
{
    class blank_is_not_whitespace_ctype: public std::ctype<char>
    {
      public:
        blank_is_not_whitespace_ctype()
            :std::ctype<char>(get_table(), false, 1)
            {}

        // Standard library of MSVC 7 doesn't define the value of
        // std::ctype<char>::table_size in the header so it cannot be used as a
        // compile-time constant below. To work around this problem we define
        // our own table_size for this compiler which happens to coincide with
        // the value effectively used by MSVC 7 (and should also be generally
        // big enough for any sane implementation)
#if defined(_MSC_VER) && (_MSC_VER <= 0x1400)
        enum
        {
            table_size = UCHAR_MAX + 1
        };
#endif // _MSC_VER 7.x workaround

      private:
        static std::ctype_base::mask const* get_table()
            {
            static std::ctype_base::mask m_[table_size];
            std::copy(classic_table(), classic_table() + table_size, m_);
            int const space = ' ';
            m_[space] = static_cast<std::ctype_base::mask>
                (m_[space] & ~std::ctype_base::space
                );
            return m_;
            }
    };

    template<typename Facet>
    std::locale const& locale_with_facet()
    {
        static Facet f;
        // This message [rejoin lines split here for readability]
        //   http://groups.google.com/groups
        //     ?selm=nsa2fvssom2ggk0ohj05n7rfdk05a5co96%404ax.com
        // describes the technique used here to avoid the Most Vexing
        // Parse.
        static std::locale custom_locale = std::locale
            (std::locale()
            ,&f
            );
        return custom_locale;
    }
} // Unnamed namespace.

// Convenience interface.

std::locale const& blank_is_not_whitespace_locale()
{
    return locale_with_facet<blank_is_not_whitespace_ctype>();
}

