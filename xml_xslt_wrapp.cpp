// Include xmlwrapp and xsltwrapp sources.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#if defined __GNUC__ && !defined __clang__
#   pragma GCC diagnostic ignored "-Wextra-semi"
#   pragma GCC diagnostic ignored "-Wnull-dereference"
#   pragma GCC diagnostic ignored "-Wsuggest-override"
#   pragma GCC diagnostic ignored "-Wswitch-enum"
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif // defined __GNUC__ && !defined __clang__

#define HAVE_BOOST_POOL_SINGLETON_POOL_HPP

#include "third_party/xmlwrapp/src/libxml/ait_impl.cxx"
#include "third_party/xmlwrapp/src/libxml/attributes.cxx"
#include "third_party/xmlwrapp/src/libxml/document.cxx"
#include "third_party/xmlwrapp/src/libxml/dtd_impl.cxx"
#include "third_party/xmlwrapp/src/libxml/errors.cxx"
#include "third_party/xmlwrapp/src/libxml/event_parser.cxx"
#include "third_party/xmlwrapp/src/libxml/init.cxx"
#include "third_party/xmlwrapp/src/libxml/node.cxx"
#include "third_party/xmlwrapp/src/libxml/node_iterator.cxx"
#include "third_party/xmlwrapp/src/libxml/node_manip.cxx"
#include "third_party/xmlwrapp/src/libxml/nodes_view.cxx"
#include "third_party/xmlwrapp/src/libxml/relaxng.cxx"
#include "third_party/xmlwrapp/src/libxml/schema.cxx"
#include "third_party/xmlwrapp/src/libxml/tree_parser.cxx"
#include "third_party/xmlwrapp/src/libxml/utility.cxx"
#include "third_party/xmlwrapp/src/libxml/xpath.cxx"
#include "third_party/xmlwrapp/src/libxslt/init.cxx"
#include "third_party/xmlwrapp/src/libxslt/stylesheet.cxx"

#undef HAVE_BOOST_POOL_SINGLETON_POOL_HPP
