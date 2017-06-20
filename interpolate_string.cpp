// Interpolate string containing embedded variable references.
//
// Copyright (C) 2017 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "interpolate_string.hpp"

#include "alert.hpp"

#include <stdexcept>

std::string interpolate_string
    (char const* s
    ,std::function<std::string (std::string const&)> const& lookup
    )
{
    std::string out;

    // This is probably not going to be enough as replacements of the
    // interpolated variables tend to be longer than the variables names
    // themselves, but it's difficult to estimate the resulting string length
    // any better than this.
    out.reserve(strlen(s));

    for(char const* p = s; *p; ++p)
        {
        if(p[0] == '$' && p[1] == '{')
            {
            std::string name;
            for(p += 2;; ++p)
                {
                if(*p == '\0')
                    {
                    alarum()
                        << "Unmatched opening brace at position "
                        << (p - s - 1 - name.length())
                        << std::flush
                        ;
                    }

                if(*p == '}')
                    {
                    // We don't check here if name is not empty, as there is no
                    // real reason to do it. Empty variable name may seem
                    // strange, but why not allow using "${}" to insert
                    // something into the interpolated string, after all?
                    out += lookup(name);
                    break;
                    }

                if(p[0] == '$' && p[1] == '{')
                    {
                    // We don't allow nested interpolations, so this can only
                    // be result of an error, e.g. a forgotten '}' before it.
                    alarum()
                        << "Unexpected nested interpolation at position "
                        << (p - s + 1)
                        << " (outer interpolation starts at "
                        << (p - s - 1 - name.length())
                        << ")"
                        << std::flush
                        ;
                    }

                // We don't impose any restrictions on the kind of characters
                // that can occur in the variable names neither because there
                // just doesn't seem to be anything to gain from it.
                name += *p;
                }
            }
        else
            {
            out += *p;
            }
        }

    return out;
}
