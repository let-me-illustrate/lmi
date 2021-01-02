// Interpolate string containing embedded variable references.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "interpolate_string.hpp"

#include "alert.hpp"

#include <cstring>                      // strlen()
#include <stack>
#include <stdexcept>
#include <vector>

namespace
{

// Information about a single section that we're currently in.
struct section_info
{
    section_info(std::string const& name, bool active)
        :name_   {name}
        ,active_ {active}
    {
    }

    // Name of the section, i.e. the part after "#".
    //
    // PDF !! In C++14 this could be replaced with string_view which would
    // save on memory allocations without compromising safety, as we know
    // that the input string doesn't change during this function execution.
    std::string const name_;

    // If true, output section contents, otherwise simply eat it.
    bool const active_;

    // Note: we could also store the position of the section start here to
    // improve error reporting. Currently this is done as templates we use
    // are small and errors shouldn't be difficult to find even without the
    // exact position, but this could change in the future.
};

// The only context we need is the stack of sections entered so far.
using context = std::stack<section_info, std::vector<section_info>>;

// The real interpolation recursive function, called by the public one to do
// all the work.
void do_interpolate_string_in_context
    (char const* s
    ,lookup_function const& lookup
    ,std::string& out
    ,context& sections
    ,std::string const& variable_name = std::string()
    ,int recursion_level = 0
    )
{
    // Guard against too deep recursion to avoid crashing on code using too
    // many nested expansions (either unintentionally, e.g. due to including a
    // partial from itself, or maliciously).
    //
    // The maximum recursion level is chosen completely arbitrarily, the only
    // criteria are that it shouldn't be too big to crash due to stack overflow
    // before it is reached nor too small to break legitimate use cases.
    if(100 <= recursion_level)
        {
        alarum()
            << "Nesting level too deep while expanding \""
            << variable_name
            << "\""
            << std::flush
            ;
        }

    // Check if the output is currently active or suppressed because we're
    // inside an inactive section.
    auto const is_active = [&sections]()
        {
            return sections.empty() || sections.top().active_;
        };

    for(char const* p = s; *p; ++p)
        {
        // As we know that the string is NUL-terminated, it is safe to check
        // the next character.
        if(p[0] == '{' && p[1] == '{')
            {
            std::string name;
            auto const pos_start = p - s + 1;
            for(p += 2;; ++p)
                {
                if(*p == '\0')
                    {
                    alarum()
                        << "Unmatched opening brace at position "
                        << pos_start
                        << std::flush
                        ;
                    }

                if(p[0] == '}' && p[1] == '}')
                    {
                    switch(name.empty() ? '\0' : name[0])
                        {
                        case '#':
                        case '^':
                            {
                            auto const real_name = name.substr(1);
                            // If we're inside a disabled section, it doesn't
                            // matter whether this one is active or not.
                            bool active = is_active();
                            if(active)
                                {
                                auto const value = lookup
                                    (real_name
                                    ,interpolate_lookup_kind::section
                                    );
                                if(value == "1")
                                    {
                                    active = true;
                                    }
                                else if(value == "0")
                                    {
                                    active = false;
                                    }
                                else
                                    {
                                    alarum()
                                        << "Invalid value '"
                                        << value
                                        << "' of section '"
                                        << real_name
                                        << "' at position "
                                        << pos_start
                                        << ", only \"0\" or \"1\" allowed"
                                        << std::flush
                                        ;
                                    }

                                if(name[0] == '^')
                                    {
                                    active = !active;
                                    }
                                }

                            sections.emplace(real_name, active);
                            }
                            break;

                        case '/':
                            if(sections.empty())
                                {
                                alarum()
                                    << "Unexpected end of section '"
                                    << name.substr(1)
                                    << "' at position "
                                    << pos_start
                                    << " without previous section start"
                                    << std::flush
                                    ;
                                }
                            if(name.compare(1, std::string::npos, sections.top().name_) != 0)
                                {
                                alarum()
                                    << "Unexpected end of section '"
                                    << name.substr(1)
                                    << "' at position "
                                    << pos_start
                                    << " while inside the section '"
                                    << sections.top().name_
                                    << "'"
                                    << std::flush
                                    ;
                                }
                            sections.pop();
                            break;

                        case '>':
                            if(is_active())
                                {
                                auto const& real_name = name.substr(1);

                                do_interpolate_string_in_context
                                    (lookup
                                        (real_name
                                        ,interpolate_lookup_kind::partial
                                        ).c_str()
                                    ,lookup
                                    ,out
                                    ,sections
                                    ,real_name
                                    ,recursion_level + 1
                                    );
                                }
                            break;

                        case '!':
                            // This is a comment, we just ignore it completely.
                            break;

                        default:
                            if(is_active())
                                {
                                // We don't check here if name is not empty, as
                                // there is no real reason to do it. Empty
                                // variable name may seem strange, but why not
                                // allow using "{{}}" to insert something into
                                // the interpolated string, after all?
                                do_interpolate_string_in_context
                                    (lookup
                                        (name
                                        ,interpolate_lookup_kind::variable
                                        ).c_str()
                                    ,lookup
                                    ,out
                                    ,sections
                                    ,name
                                    ,recursion_level + 1
                                    );
                                }
                        }

                    // We consume two characters here ("}}"), not one, as in a
                    // usual loop iteration.
                    ++p;
                    break;
                    }

                if(p[0] == '{' && p[1] == '{')
                    {
                    // We don't allow nested interpolations, so this can only
                    // be result of an error, e.g. a forgotten "}}" somewhere.
                    alarum()
                        << "Unexpected nested interpolation at position "
                        << pos_start
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
        else if(is_active())
            {
            out += *p;
            }
        }
}

} // Unnamed namespace.

std::string interpolate_string
    (char const* s
    ,lookup_function const& lookup
    )
{
    std::string out;

    // This is probably not going to be enough as replacements of the
    // interpolated variables tend to be longer than the variables names
    // themselves, but it's difficult to estimate the resulting string length
    // any better than this.
    out.reserve(std::strlen(s));

    // The stack contains all the sections that we're currently in.
    std::stack<section_info, std::vector<section_info>> sections;

    do_interpolate_string_in_context(s, lookup, out, sections);

    if(!sections.empty())
        {
        alarum()
            << "Unclosed section '"
            << sections.top().name_
            << "'"
            << std::flush
            ;
        }

    return out;
}
