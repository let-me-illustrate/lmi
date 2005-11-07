// Configuration for Como with MinGW as the underlying C compiler.
//
// Copyright (C) 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: config_como_mingw.hpp,v 1.1 2005-11-07 02:48:20 chicares Exp $

// Configuration header for compiler quirks.

#ifndef config_como_mingw_hpp
#define config_como_mingw_hpp

#ifndef OK_TO_INCLUDE_CONFIG_COMO_WITH_MINGW_HPP
#   error This file is not intended for separate inclusion.
#endif // OK_TO_INCLUDE_CONFIG_COMO_WITH_MINGW_HPP

#if defined LMI_COMO_WITH_MINGW
    // COMPILER !! Access the (somewhat defective) msvcrt _snprintf().
#   define snprintf _snprintf
#else // Not Como with MinGW as the underlying C compiler.
#   error Use this file for Como with MinGW as the underlying C compiler only.
#endif // Not Como with MinGW as the underlying C compiler.

#endif // config_como_mingw_hpp

