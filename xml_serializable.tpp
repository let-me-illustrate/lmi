// Mixin for xml serialization.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_serializable.hpp"

#include "alert.hpp"
#include "any_member.hpp"         // MemberSymbolTable<>
#include "contains.hpp"
#include "platform_dependent.hpp" // access()
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

#include <xmlwrapp/nodes_view.h>

#include <algorithm>              // std::copy(), std::find()
#include <iterator>               // std::back_inserter
#include <sstream>
#include <vector>

template<typename T>
xml_serializable<T>::~xml_serializable()
{
    // Assert that static_cast<T cv&> doesn't engender undefined
    // behavior, and that class T provides the expected operator[]()
    // and member_names() functions.
    //
    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT
        ((
           boost::is_base_and_derived<xml_serializable <T>,T>::value
        && boost::is_base_and_derived<MemberSymbolTable<T>,T>::value
        ));
}

template<typename T>
void xml_serializable<T>::load(fs::path const& path)
{
    if(access(path.string().c_str(), R_OK))
        {
        fatal_error()
            << "File '"
            << path.string()
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    xml_lmi::dom_parser parser(path.string());
    xml::element const& root = parser.root_node(xml_root_name());
    read(root);
}

template<typename T>
void xml_serializable<T>::save(fs::path const& path) const
{
    xml_lmi::xml_document document(xml_root_name());
    immit_members_into(document.root_node());
    document.save(path.string());
}

template<typename T>
void xml_serializable<T>::read(xml::element const& x)
{
    if(xml_root_name() != x.get_name())
        {
        fatal_error()
            << "XML node name is '"
            << x.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            << LMI_FLUSH
            ;
        }

    std::string file_version_string;
    if(!xml_lmi::get_attr(x, "version", file_version_string))
        {
        handle_missing_version_attribute();
        file_version_string = "0";
        }
    int file_version = value_cast<int>(file_version_string);

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

    std::ostringstream oss;

    std::map<std::string, std::string> detritus_map;

    std::list<std::string> residuary_names;
    std::copy
        (t().member_names().begin()
        ,t().member_names().end()
        ,std::back_inserter(residuary_names)
        );
    std::list<std::string>::iterator current_member;

    xml::const_nodes_view const elements(x.elements());
    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi child = elements.begin(); child != elements.end(); ++child)
        {
        std::string node_tag(child->get_name());
        current_member = std::find
            (residuary_names.begin()
            ,residuary_names.end()
            ,node_tag
            );
        if(residuary_names.end() != current_member)
            {
            read_element(*child, node_tag, file_version);
            residuary_names.erase(current_member);
            }
        else if(is_detritus(node_tag))
            {
            // Hold certain obsolete entities that must be translated.
            std::string s = xml_lmi::get_content(*child);
            redintegrate_ex_ante(file_version, node_tag, s);
            detritus_map[node_tag] = s;
            }
        else
            {
            bool b = contains(t().member_names(), node_tag);
            std::string s = b ? "[duplicate]" : "[unrecognized]";
            oss << "  '" << node_tag << "' " << s << "\n";
            }
        }
    if(!oss.str().empty())
        {
        warning() << "Discarded XML elements:\n" << oss.str() << LMI_FLUSH;
        }

    redintegrate_ex_post(file_version, detritus_map, residuary_names);

    redintegrate_ad_terminum();
}

template<typename T>
void xml_serializable<T>::write(xml::element& x) const
{
    xml::element root(xml_root_name().c_str());
    immit_members_into(root);
    x.push_back(root);
}

/// The associated instance of class T.
///
/// This member function is 'inline' because it wouldn't make sense
/// for any external file to call it.

template<typename T>
inline T& xml_serializable<T>::t()
{
    return static_cast<T&>(*this);
}

/// The associated instance of class T.
///
/// This member function is 'inline' because it wouldn't make sense
/// for any external file to call it.

template<typename T>
inline T const& xml_serializable<T>::t() const
{
    return static_cast<T const&>(*this);
}

template<typename T>
void xml_serializable<T>::immit_members_into(xml::element& root) const
{
// XMLWRAPP !! There's no way to set an integer attribute.
    std::string const version(value_cast<std::string>(class_version()));
    xml_lmi::set_attr(root, "version", version.c_str());

    std::vector<std::string>::const_iterator i;
    for(i = t().member_names().begin(); i != t().member_names().end(); ++i)
        {
        write_element(root, *i);
        }
}

/// Backward-compatibility serial number of class T's xml version.

template<typename T>
int xml_serializable<T>::class_version() const
{
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Root tag (when T is saved as the root of a document).

template<typename T>
std::string const& xml_serializable<T>::xml_root_name() const
{
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Read an xml element.
///
/// This default implementation is appropriate only for streamable
/// types (for which any_member<T>::operator=(std::string const&) is
/// valid). Override it wherever that precondition does not hold.
///
/// xml_serialize::from_xml() does nearly the same thing, but in a
/// type-dependent way; thus, it doesn't have the precondition above.
/// However, the datum here has been subject to type erasure and its
/// type is not readily unerased.
///
/// The xml::element argument is the element to be read, which is
/// already available through an iterator in read().

template<typename T>
void xml_serializable<T>::read_element
    (xml::element const& e
    ,std::string const&  name
    ,int                 file_version
    )
{
    std::string s = xml_lmi::get_content(e);
    redintegrate_ex_ante(file_version, name, s);
    t()[name] = s;
}

/// Write an xml element.
///
/// This default implementation is appropriate only for streamable
/// types (for which any_member<T>::str() is valid). Override it
/// wherever that precondition does not hold.
///
/// xml_serialize::set_element() does nearly the same thing, but it
/// asserts a precondition that the parent has no element with the
/// given tagname. Such an assertion here would impose a speed penalty
/// of fourteen percent (measured with the 'input_test' unit test),
/// yet would serve no purpose because immit_members_into() iterates
/// across std::map keys, which are guaranteed to be unique.
///
/// The xml::element argument is the parent of the element to be
/// written.

template<typename T>
void xml_serializable<T>::write_element
    (xml::element&        parent
    ,std::string const&   name
    ) const
{
    parent.push_back(xml::element(name.c_str(), t()[name].str().c_str()));
}

/// React to absence of required 'version' attribute.
///
/// This default implementation throws an informative exception.
///
/// A derived class may override this with a do-nothing implementation
/// if it is necessary to extend backward compatibility to historical
/// xml files that originally had no such attribute.

template<typename T>
void xml_serializable<T>::handle_missing_version_attribute() const
{
    fatal_error()
        << "XML tag <"
        << xml_root_name()
        << "> lacks required version attribute."
        << LMI_FLUSH
        ;
}

/// Ascertain whether an element-tag is obsolete.
///
/// Tags that were present in older versions and later removed are
/// recognized and redintegrated with other tags (elsewhere). If
/// they're subsequently resurrected, then they may resume their
/// original or an enhanced function.

template<typename T>
bool xml_serializable<T>::is_detritus(std::string const&) const
{
    return false;
    // Pastable specimen implementation for derived classes:
#if 0
    static std::string const a[] =
        {"Remove this string when adding the first removed entity."
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return contains(v, s);
#endif // 0
}

/// Provide for backward compatibility before assigning values.
///
/// Motivation: transform an old enumerative string to a contemporary
/// equivalent, before assigning it to a variable of mc_enum type for
/// which it would no longer be syntactically valid.
///
/// For example, if gender had formerly been stored as {M, F} but now
/// {Male, Female} is used instead, then this function would be
/// overridden to transform the old representation to the new:
///   <gender>M</gender> --> <gender>Male</gender>
/// This transformation avoids attempting to assign 'M' to a 'gender'
/// enumeration, which would elicit a runtime error.
///
/// The element's text contents are given as a modifiable reference.

template<typename T>
void xml_serializable<T>::redintegrate_ex_ante
    (int                file_version
    ,std::string const& // name
    ,std::string      & // value
    ) const
{
    if(class_version() == file_version)
        {
        return;
        }

    fatal_error()
        << "Incompatible file version."
        << " An explicit override is necessary."
        << LMI_FLUSH
        ;
}

/// Provide for backward compatibility after assigning values.
///
/// Motivation: transform an old value that remains syntactically
/// valid but is no longer semantically inappropriate due to changes
/// in code that uses it; or assign an appropriate default for an
/// element that was not present in earlier versions.
///
/// For example, if a person's first and last names had been stored
/// separately but are now combined in a single element, then this
/// function would be overridden to transform this:
///   <firstname>John</firstname> <lastname>Brown</lastname>
/// to this:
///   <name>John Brown</name>
///
/// As another example, suppose issue and effective dates are now
/// distinguished, whereas formerly only issue date had been stored.
/// It would be reasonable to override this function to copy the
/// issue-date value to a new effective date element.
///
/// The 'residuary_names' argument contains all "ascribed" class
/// member names that did not occur as element tags in the xml file
/// being read. This is useful for verifying that an element expected
/// to be absent actually was absent.
///
/// The 'detritus_map' argument contains names and values of all tags
/// that have already been processed if they were marked as detritus:
/// i.e., if they were used only in an earlier version. In the first
/// example above, 'firstname' and 'lastname' would be "detritus".

template<typename T>
void xml_serializable<T>::redintegrate_ex_post
    (int                                       file_version
    ,std::map<std::string, std::string> const& // detritus_map
    ,std::list<std::string>             const& // residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    fatal_error()
        << "Incompatible file version."
        << " An explicit override is necessary."
        << LMI_FLUSH
        ;
}

/// Perform any required after-the-fact fixup.
///
/// Override this function to do anything that's necessary after all
/// elements have been read, but doesn't fit anywhere else.

template<typename T>
void xml_serializable<T>::redintegrate_ad_terminum()
{
}

