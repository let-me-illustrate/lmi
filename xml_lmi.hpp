// Interface to xmlwrapp.
//
// Copyright (C) 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: xml_lmi.hpp,v 1.5 2006-11-05 16:37:07 chicares Exp $

#ifndef xml_lmi_hpp
#define xml_lmi_hpp

#include "config.hpp"

#include "xml_lmi_fwd.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <xmlwrapp/node.h> // xml::node::const_iterator

#include <iosfwd>
#include <string>
#include <vector>

/// Interface to xmlwrapp.

namespace xml_lmi
{
    typedef xml::node::const_iterator   NodeConstIterator;
    typedef xml::node::const_iterator   ElementPointer;
    typedef std::vector<ElementPointer> ElementContainer;

    class dom_parser
        :private boost::noncopyable
    {
        typedef xml::tree_parser DomParser;
        typedef xml::init        Initializer;

      public:
        dom_parser(std::string const& filename);
        dom_parser(std::istream&);
        ~dom_parser();

#if defined USING_CURRENT_XMLWRAPP
        Document const& document() const;
#endif // defined USING_CURRENT_XMLWRAPP
        Element const& root_node(std::string const& expected_name) const;

      private:
        std::string                    error_context_;
        boost::scoped_ptr<Initializer> initializer_;
        boost::scoped_ptr<DomParser>   parser_;
    };

    /// Create a container of pointers to an element's child elements.
    /// The contents are notionally pointers, but actually iterators,
    /// because that's the abstraction xmlwrapp provides; they seem to
    /// function as incrementable smart pointers.
    ///
    /// If the second argument is specified, then only elements having
    /// the given name are placed in the container.
    ///
    /// Only direct children are considered: children of child nodes
    /// are not. Only child nodes that are elements are placed in the
    /// container; other types of nodes are not.
    ///
    /// Precondition: No child element pointer returned by xml-library
    /// calls is null.
    ///
    /// Postcondition: The container holds no null pointers.
    ///
    /// Throws: an exception, via fatal_error(), if a precondition is
    /// violated, or if xml-library calls throw an exception derived
    /// from std::exception.

    ElementContainer child_elements
        (Element const&
        ,std::string const& name = std::string()
        );

    /// Retrieve an xml element's full text-node contents.
    ///
    /// The contents of all text-node children are concatenated.
    ///
    /// Only direct children are considered: children of child nodes
    /// are not.

    std::string get_content(Element const&);

    /// Retrieve an xml element's name.

    std::string get_name(Element const&);
} // namespace xml_lmi

#endif //xml_lmi_hpp

