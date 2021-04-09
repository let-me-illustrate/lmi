// List of headers to precompile for use with wx.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef pchlist_wx_hpp
#define pchlist_wx_hpp

#if !defined LMI_OKAY_TO_INCLUDE_PCHLIST_WX_HPP
#   error This file is not intended for separate inclusion.
#endif // !defined LMI_OKAY_TO_INCLUDE_PCHLIST_WX_HPP

#if defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH

// This list includes the most frequently used wx headers, based
// on gcc's dependencies output.

#   include "config.hpp"

#   include "product_editor.hpp"
#   include "view_ex.hpp"
#   include "wx_new.hpp"
#   include "wx_utility.hpp"
#   include "wx_workarounds.hpp"

#   include <wx/wx.h>

#endif // defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH

#endif // pchlist_wx_hpp
