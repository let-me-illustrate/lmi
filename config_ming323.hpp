// Configuration for MinGW gcc-3.2.3 .
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: config_ming323.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

// Configuration header for compiler quirks--mingw gcc-3.2.3 .

#ifndef config_ming323_hpp
#define config_ming323_hpp

#ifndef OK_TO_INCLUDE_CONFIG_MING323_HPP
#   error This file is not intended for separate inclusion.
#endif // OK_TO_INCLUDE_CONFIG_MING323_HPP

#if defined __MINGW32__ && defined __GNUC__ && __GNUC__ == 3 && __GNUC_MINOR__ == 2

    // dannysmith's email of 2003-12-08:
/*
    extern double  (*_imp__pow) (double, double);
    static inline double pow (double x, double y)
    {
        __volatile__ double res = (*_imp__pow)(x, y);
        return res;
    }
    #define pow(_x, _y) __pow(_x, _y)
*/
    // Doesn't seem to work. Need to ask on mingw mailing list.

#else // Not old gnu compiler.
#   error Use this file for mingw gcc version 3.2.3 only.
#endif // Not old gnu compiler.

#endif // config_ming323_hpp

