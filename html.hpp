// Utilities for representing and generating HTML.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef html_hpp
#define html_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>
#include <utility>                      // move()

/// Namespace for helpers used for HTML generation.
///
/// Main idea is to avoid generating HTML using raw strings, which is error
/// prone and difficult to read and maintain. One source of errors is
/// forgetting to escape special characters, such as "<" or "&", and html::text
/// class helps with this by providing from() function doing it automatically.
///
/// Another one is forgetting to close a tag (or closing a wrong one) and while
/// html::text is too low level to help with this, html::element can be used
/// for structured HTML generation, which guarantees that the result is
/// well-formed. By using predefined constants in html::tag and html::attr
/// namespaces, typos in the element names can also be automatically avoided.

namespace html
{

/// Represents a piece of text containing HTML.
///
/// This is a separate type for type safety, e.g. to avoid passing raw,
/// unescaped, strings to a function expecting HTML (or, less catastrophically,
/// but still wrongly, passing already escaped HTML to a function doing
/// escaping internally).
///
/// As it still needs to be converted to a string sooner or later to be really
/// used, it does provide a conversion, as_html(), which uses move semantics,
/// so it requires an rvalue reference and must be the last member function
/// called on an instance of this class.

class text
{
  public:
    // This type has value semantics.
    text() = default;
    text(text const&) = default;
    text(text&&) = default;
    text& operator=(text const&) = default;
    text& operator=(text&&) = default;

    /// Escape special XML characters in the given string, ensuring that it
    /// appears correctly inside HTML element contents. Notice that we don't
    /// need to escape quotes here as we never use the result of this function
    /// inside an HTML attribute, only inside HTML elements.

    static text from(std::string const& s)
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

    /// Use the given string with HTML inside it directly. No escaping is done
    /// by this ctor.

    static text from_html(std::string s)
    {
        return text{std::move(s)};
    }

    /// Just a symbolic name for a non breaking space HTML entity.

    static text nbsp()
    {
        return text::from_html("&nbsp;");
    }

    /// Append another text fragment to this one.
    ///
    /// This function allows chained invocation for appending more than one
    /// fragment at once.

    text& operator+=(text const& t)
    {
        html_ += t.html_;

        return *this;
    }

    std::string&& as_html() &&
    {
        return std::move(html_);
    }

  private:
    // This move ctor is private and does not perform any escaping.
    explicit text(std::string&& html)
        :html_ {html}
    {
    }

    std::string html_;
};

/// Represents a single attribute of an HTML element.

class attribute
{
  public:
    explicit attribute(char const* name)
        :name_ {name}
    {
    }

    attribute operator()(std::string value) const
    {
        return attribute(name_, std::move(value));
    }

    std::string as_string() const;

  private:
    attribute(char const* name, std::string&& value)
        :name_  {name}
        ,value_ {std::move(value)}
    {
    }

    char const* const name_;
    std::string const value_;
};

namespace detail
{

class LMI_SO any_element
{
  public:
    // Ctor should only be used with literal strings as argument.
    explicit any_element(char const* name)
        :name_ {name}
    {
    }

  protected:
    // Return the opening tag of the element, with attributes, if any.
    std::string get_start() const;

    // Add the given attribute to our attributes string.
    void update_attributes(attribute const& attr);

    char const* const name_;

  private:
    std::string       attributes_;
};

} // namespace detail

/// Represents a normal HTML element which can have content inside it.
///
/// This class uses the so called fluent API model in which calls to its
/// different functions return the object itself and so can be chained together.
/// For example (assuming an implicit "using namespace html"):
///
///     auto para_with_link =
///         tag::p[attr::align("center")]
///             (text("Link to "))
///             (tag::a[attr::href("https://lmi.nongnu.org/")]
///                 (text::from("lmi project page"))
///             )
///         ;

class LMI_SO element : private detail::any_element
{
  public:
    // Ctor should only be used with literal strings as argument.
    explicit element(char const* name)
        :detail::any_element(name)
    {
    }

    element(element const&) = default;
    element(element&&) = default;

    /// Add an attribute.

    element operator[](attribute const& attr) const&
    {
        element e{*this};
        e.update_attributes(attr);
        return e;
    }

    element&& operator[](attribute const& attr) &&
    {
        update_attributes(attr);
        return std::move(*this);
    }

    /// Add inner contents.

    element operator()(text contents) const&
    {
        element e{*this};
        e.update_contents(std::move(contents).as_html());
        return e;
    }

    element&& operator()(text contents) &&
    {
        update_contents(std::move(contents).as_html());
        return std::move(*this);
    }

    /// Convert to HTML text with this element and its contents.
    ///
    /// This implicit conversion operator is not really dangerous as it is
    /// normal to represent an HTML element as HTML text and it's very
    /// convenient to have it as it allows to accept either another element or
    /// text in our own operator() and also use operator+() defined below to
    /// concatenate HTML elements without having to convert them to text
    /// beforehand.

    operator text() const;

  private:
    void update_contents(std::string&& contents);

    std::string contents_;
};

/// Represents a void HTML element which can't have anything inside it.

class void_element : private detail::any_element
{
  public:
    explicit void_element(char const* name)
        :detail::any_element(name)
    {
    }

    void_element(void_element const&) = default;
    void_element(void_element&&) = default;

    void_element operator[](attribute const& attr) const&
    {
        void_element e{*this};
        e.update_attributes(std::move(attr));
        return e;
    }

    void_element&& operator[](attribute const& attr) &&
    {
        update_attributes(std::move(attr));
        return std::move(*this);
    }

    operator text() const
    {
        return text::from_html(get_start());
    }
};

/// Namespace for HTML attributes.

namespace attr
{

extern LMI_SO attribute const align;
extern LMI_SO attribute const cellpadding;
extern LMI_SO attribute const cellspacing;
extern LMI_SO attribute const colspan;
extern LMI_SO attribute const nowrap;
extern LMI_SO attribute const size;
extern LMI_SO attribute const valign;
extern LMI_SO attribute const width;

} // namespace attr

/// Namespace for HTML tags.

namespace tag
{

extern LMI_SO element      const b;
extern LMI_SO void_element const br;
extern LMI_SO element      const font;
extern LMI_SO element      const i;
extern LMI_SO element      const p;
extern LMI_SO element      const table;
extern LMI_SO element      const td;
extern LMI_SO element      const tr;

} // namespace tag

inline
text operator+(text t1, text const& t2)
{
    text t{std::move(t1)};
    t += t2;
    return t;
}

} // namespace html

#endif // html_hpp
