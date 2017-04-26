// Text representing HTML contents.
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

#ifndef html_text_hpp
#define html_text_hpp

#include "config.hpp"

#include <string>
#include <utility>                      // std::move

/// Namespace for helpers used for HTML generation.
namespace html
{

/// Namespace for the support HTML tags.
///
/// Tags are only used as template arguments, so they don't need to be defined,
/// just declared -- and tag::info below specialized for them.

namespace tag
{

struct b;
struct br;
struct p;

template<typename T>
struct info;

template<>
struct info<b>
{
    static char const* get_name() { return "b"; }
    static bool has_end() { return true; }
};

template<>
struct info<br>
{
    static char const* get_name() { return "br"; }
    static bool has_end() { return false; }
};

template<>
struct info<p>
{
    static char const* get_name() { return "p"; }
    static bool has_end() { return true; }
};

} // namespace tag

/// Represents a piece of text containing HTML.
///
/// This is a separate type for type safety, e.g. to avoid passing raw,
/// unescaped, strings to a function expecting HTML (or, less catastrophically,
/// but still wrongly, passing already escaped HTML to a function doing
/// escaping internally).
///
/// As it still needs to be converted to a string sooner or later to be really
/// used, it does provide a conversion -- but it can be used only once.
class text
{
public:
    // This type has value semantics.
    text() = default;
    text(text const&) = default;
    text(text &&) = default;
    text& operator=(text const&) = default;
    text& operator=(text&&) = default;

    /// Escape special XML characters in the given string, ensuring that it
    /// appears correctly inside HTML element contents. Notice that we don't
    /// need to escape quotes here as we never use the result of this function
    /// inside an HTML attribute, only inside HTML elements.
    static text escape_for_html_elem(std::string const& s)
    {
        std::string z;
        z.reserve(s.length());
        for(auto const& c : s)
            {
            switch(c)
                {
                case '<': z += "&lt;" ; break;
                case '>': z += "&gt;" ; break;
                case '&': z += "&amp;"; break;
                default : z += c      ;
                }
            }

        return text{std::move(z)};
    }

    /// Wrap contents of this HTML snippet into the given tag (or just prepend
    /// the tag if it doesn't have the matching end tag).
    template <typename T>
    text wrap() const
    {
        return do_wrap(tag::info<T>::get_name(), tag::info<T>::has_end());
    }

    /// Wrap contents of this HTML snippet into the given tag (or just prepend
    /// the tag if it doesn't have the matching end tag), but only if it is not
    /// empty -- otherwise just return empty text.
    ///
    /// For the tags without matching closing tags, such as e.g. "<br>",
    /// wrapping the text means just prepending the tag to it. This is still
    /// done only if the text is not empty.
    template <typename T>
    text wrap_if_not_empty() const
    {
        return m_html.empty() ? text{} : wrap<T>();
    }

    /// Append another HTML fragment to this one.
    ///
    /// This method allows chained invocation for appending more than one
    /// fragment at once.
    text& operator+=(text t)
    {
        m_html += std::move(t).m_html;

        return *this;
    }

    /// Return the string with UTF-8 encoded HTML text of the given object
    /// consuming it in the process.
    ///
    /// Notice that this method can only be called on rvalue references, and so
    /// it can't be used more than once.
    std::string&& as_string() &&
    {
        return std::move(m_html);
    }

private:
    // This move ctor is private and does not perform any escaping.
    text(std::string&& html)
        :m_html{html}
    {
    }

    // Type-independent part implementation of public wrap(): having it as a
    // separate function avoids template bloat.
    text do_wrap(char const* outer_tag, bool has_end) const
    {
        std::string z;
        z.reserve(m_html.length() + 10);

        z = '<';
        z += outer_tag;
        z += '>';

        z += m_html;

        if(has_end)
            {
            z += "</";
            z += outer_tag;
            z += '>';
            }

        return text{std::move(z)};
    }


    std::string m_html;
};

inline
text operator+(text t1, text t2)
{
    auto t{t1};
    t += t2;
    return t;
}

} // namespace html

#endif // html_text_hpp
