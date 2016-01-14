// Include xmlwrapp and xsltwrapp sources.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#define HAVE_BOOST_POOL_SINGLETON_POOL_HPP

#include "libxml/ait_impl.cxx"
#include "libxml/attributes.cxx"
#include "libxml/document.cxx"
#include "libxml/dtd_impl.cxx"
#include "libxml/errors.cxx"
#include "libxml/event_parser.cxx"
#include "libxml/init.cxx"
#include "libxml/node.cxx"
#include "libxml/node_iterator.cxx"
#include "libxml/node_manip.cxx"
#include "libxml/nodes_view.cxx"
#include "libxml/schema.cxx"
#include "libxml/tree_parser.cxx"
#include "libxml/utility.cxx"
#include "libxslt/init.cxx"
#include "libxslt/stylesheet.cxx"

#undef HAVE_BOOST_POOL_SINGLETON_POOL_HPP

