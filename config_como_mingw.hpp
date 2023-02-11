// Configuration for Como with MinGW as the underlying C compiler.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

// Configuration header for compiler quirks.

#ifndef config_como_mingw_hpp
#define config_como_mingw_hpp

#if !defined OK_TO_INCLUDE_CONFIG_COMO_WITH_MINGW_HPP
#   error This file is not intended for separate inclusion.
#endif // !defined OK_TO_INCLUDE_CONFIG_COMO_WITH_MINGW_HPP

#if defined LMI_COMO_WITH_MINGW
#   // Copacetic.
#else  // Not Como with MinGW as the underlying C compiler.
#   error Use this file only for Como with MinGW as the underlying C compiler.
#endif // Not Como with MinGW as the underlying C compiler.

#if !defined c_plusplus
#   define LMI_COMO_STRICT_MODE
#endif // !defined c_plusplus

#if defined LMI_COMO_STRICT_MODE && !defined LMI_COMO_MS_MODE
#   define __declspec(deliberately_ignored)
#endif // defined LMI_COMO_STRICT_MODE && !defined LMI_COMO_MS_MODE

#if 202 <= LMI_MINGW_VERSION
#   error Reconsider como configuration to reflect recent libmingwex.
#else  // LMI_MINGW_VERSION < 202
#   define snprintf _snprintf
#   define vsnprintf _vsnprintf
#endif // LMI_MINGW_VERSION < 202

#include <cstdio>
    // COMPILER !! como 4.3.3 apparently should do this, but doesn't.
    // See, e.g.,
    //   http://groups.google.com/group/comp.std.c++/msg/05fcbb27929c6046
    namespace std {using ::FILE;}

#endif // config_como_mingw_hpp
