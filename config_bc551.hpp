// Configuration for borland bcc-5.5.1 .
//
// Copyright (C) 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// Configuration header for compiler quirks--bcc-5.5.1 .

#ifndef config_bc551_hpp
#define config_bc551_hpp

#if !defined OK_TO_INCLUDE_CONFIG_BC551_HPP
#   error This file is not intended for separate inclusion.
#endif // !defined OK_TO_INCLUDE_CONFIG_BC551_HPP

#if defined __BORLANDC__ && 0x0550 <= __BORLANDC__
#   // Copacetic.
#else  // Not borland 5.5.1+ .
#   error Use this file only for borland version 5.5.1 or greater.
#endif // Not borland 5.5.1+ .

// Do not define this macro, even though the compiler supports pch.
// The present pch file includes wx headers, but this compiler is
// inadequate for lmi and is used only for non-wx unit tests.
//
// #define LMI_COMPILER_USES_PCH

#include <cstdio>
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
    // COMPILER !! bc++5.5.1 fails to provide these as functions in
    // namespace std as required by C++98 [17.4.1.2].
#include <stdio.h>
    namespace std
    {
#   undef feof
    using ::feof;
#   undef ferror
    using ::ferror;
    } // namespace std
#include <cfloat>
    // COMPILER !! bc++5.5.1 got this wrong too.
    // See my post to borland.public.cppbuilder.language dated
    // 2001-11-04T08:28:56 PST
    using std::_max_dble;
#include <csignal>
    // COMPILER !! bc++5.5.1 got this wrong too.
    using std::_CatcherPTR;

// We include standard library headers this way:
//   #include <filename>
// using their standard names with no '.h' . While GNU tools look for
// 'filename', borland tools append an '.h' and look for 'filename.h' as
// permitted by 2.8/1, and indeed borland uses '.h' names for its standard
// library headers. For each standard header we use, we provide a forwarding
// header 'filename' with no '.h' so that GNU CPP can be used to generate
// autodependencies for command-line builds using 'make'.
//
// Unfortunately, bc++ never sees these forwarding headers, so we can't
// use them to patch this compiler's problems. What options do we have?
//
//  - Patch the borland headers. This may render the compiler unusable
//    for programs that use other workarounds. And other programs might
//    make different patches for the same reason. This is solipsistic.
//
//  - Override all standard headers by putting 'filename.h' in a directory
//    of our own that precedes the borland include directory on the search
//    path. But then there's no way to forward to the borland headers without
//    knowing where the borland headers reside, which may differ from one
//    installation to another. That could work well enough for makefiles,
//    but it's nasty with the borland IDE. (Yet it does work for gcc.)
//
//  - Write '#include <header.>' to convince borland tools that there really
//    is no '.h' . But we want our mainstream code to be standard.
//
//  - Include problematic headers in this file, which is included everywhere,
//    and put patches here.
//
// We choose the last option as the least evil, and mitigate the evil by
// including the problematic headers in 'pch.hpp' unless that creates
// additional problems (e.g. precompiling STL headers with bc++5.02 can
// produce internal compiler errors). The special borland headers use
// this approach.

#endif // config_bc551_hpp
