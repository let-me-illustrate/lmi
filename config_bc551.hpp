// Configuration for borland bcc-5.5.1 .
//
// Copyright (C) 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: config_bc551.hpp,v 1.1 2005-02-23 12:37:20 chicares Exp $

// Configuration header for compiler quirks--bcc-5.5.1 .

#ifndef config_bc551_hpp
#define config_bc551_hpp

#ifndef OK_TO_INCLUDE_CONFIG_BC551_HPP
#   error This file is not intended for separate inclusion.
#endif // OK_TO_INCLUDE_CONFIG_BC551_HPP

#if defined __BORLANDC__ && __BORLANDC__ >= 0x0550
#   define LMI_COMPILER_USES_PCH
#   define LMI_TEMPLATE_ARRAY_PARM_DEFECT
#   include <cstdio>
    // COMPILER !! bc++5.5.1 Work around library bug: the 'stdin'
    // macro references '_streams' with no namespace qualifier,
    // but including <cstdio> rather than <stdio.h> puts that
    // referenced object in namespace std.
    // See my post to borland.public.cppbuilder.language dated
    //   2001-11-08T18:17:12 -0500
    using std::_streams;
    // COMPILER !! bc++5.5.1 windows headers need ::size_t.
    // See my post to borland.public.cppbuilder.language dated
    //   2001-11-08T18:23:48 -0500
    using std::size_t;
#   include <cfloat>
    // COMPILER !! bc++5.5.1 got this wrong too.
    // See my post to borland.public.cppbuilder.language dated
    // 2001-11-04T08:28:56 PST
    using std::_max_dble;
#   include <csignal>
    // COMPILER !! bc++5.5.1 got this wrong too.
    using std::_CatcherPTR;

#else // Not newer borland compiler.
#   error Use this file for borland version 5.5.1 or greater only.
#endif // Not newer borland compiler.

#endif // config_bc551_hpp

