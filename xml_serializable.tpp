// Mixin for xml serialization.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "xml_serializable.hpp"

#include "alert.hpp"
#include "any_member.hpp"               // MemberSymbolTable<>
#include "contains.hpp"
#include "platform_dependent.hpp"       // access()
#include "xml_lmi.hpp"

#include <boost/filesystem/convenience.hpp> // basename()

#include <xmlwrapp/nodes_view.h>

#include <algorithm>                    // copy(), find()
#include <iterator>                     // back_inserter()
#include <sstream>
#include <type_traits>
#include <vector>

template<typename T>
xml_serializable<T>::~xml_serializable()
{
    // Assert that static_cast<T cv&> doesn't engender undefined
    // behavior, and that class T provides the expected operator[]()
    // and member_names() functions.
    static_assert(std::is_base_of_v<xml_serializable <T>,T>);
    static_assert(std::is_base_of_v<MemberSymbolTable<T>,T>);
}

template<typename T>
void xml_serializable<T>::load(fs::path const& path)
{
    if(access(path.string().c_str(), R_OK))
        {
        alarum()
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
    write_proem(document, fs::basename(path));
    immit_members_into(document.root_node());
    document.save(path.string());
}

template<typename T>
void xml_serializable<T>::read(xml::element const& x)
{
    if(xml_root_name() != x.get_name())
        {
        alarum()
            << "XML node name is '"
            << x.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            << LMI_FLUSH
            ;
        }

    int file_version = 0;
    if(!xml_lmi::get_attr(x, "version", file_version))
        {
        handle_missing_version_attribute();
        }

    std::ostringstream oss;

    std::map<std::string,value_type> detritus_map;

    std::list<std::string> residuary_names;
    std::copy
        (t().member_names().begin()
        ,t().member_names().end()
        ,std::back_inserter(residuary_names)
        );

    for(auto const& child : x.elements())
        {
        std::string node_tag(child.get_name());
        std::list<std::string>::iterator current_member = std::find
            (residuary_names.begin()
            ,residuary_names.end()
            ,node_tag
            );
        if(residuary_names.end() != current_member)
            {
            read_element(child, node_tag, file_version);
            residuary_names.erase(current_member);
            }
        else if(is_detritus(node_tag))
            {
            // Hold certain obsolete entities that must be translated.
            value_type v = fetch_element(child);
            redintegrate_ex_ante(file_version, node_tag, v);
            detritus_map[node_tag] = v;
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
    xml_lmi::set_attr(root, "version", class_version());

    for(auto const& i : t().member_names())
        {
        write_element(root, i);
        }
}

template<typename X, typename Y>
inline Y sfinae_cast
    (X const& x
    ,typename std::enable_if<std::is_same_v<X,Y>>::type* = nullptr
    )
{
    return x;
}

template<typename X, typename Y>
inline Y sfinae_cast
    (X const&
    ,typename std::enable_if<!std::is_same_v<X,Y>>::type* = nullptr
    )
{
    alarum() << "Impermissible type conversion." << LMI_FLUSH;
    return Y();
}

/// Retrieve an xml element's value.

template<typename T>
typename xml_serializable<T>::value_type xml_serializable<T>::fetch_element
    (xml::element const& e
    ) const
{
    return sfinae_cast<std::string,value_type>(xml_lmi::get_content(e));
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
/// type is not readily unerased. SOMEDAY !! But now that datatype is
/// available, selectively at least, as value_type...so should
/// from_xml() be used directly here? Or should this function be kept
/// for parallelism with write_element()?
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
    value_type v = fetch_element(e);
    redintegrate_ex_ante(file_version, name, v);
    t()[name] = sfinae_cast<value_type,std::string>(v);
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

/// Write proemial information such as a license notice.
///
/// This default implementation writes nothing.

template<typename T>
void xml_serializable<T>::write_proem
    (xml_lmi::xml_document& // document
    ,std::string const&     // file_leaf_name
    ) const
{
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
    alarum()
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
    static std::vector<std::string> const v
        {"Remove this string when adding the first removed entity."
        };
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
    ,value_type       & // value
    ) const
{
    if(class_version() == file_version)
        {
        return;
        }

    alarum()
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
    (int                                     file_version
    ,std::map<std::string,value_type> const& // detritus_map
    ,std::list<std::string>           const& // residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    alarum()
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
