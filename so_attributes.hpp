// Shared-object visibility (elf) and export-import (msw) attributes.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// On applicable platforms, optionally use shared-object attributes
// for symbols decorated with macro LMI_SO .
//
// If LMI_USE_SO_ATTRIBUTES is not defined, then LMI_SO is ignored.
// This is the default behavior. Otherwise:
//
// - For gcc (or any other toolchain that defines gcc macros), if the
// platform is not msw, then elf is assumed, and symbols decorated
// with LMI_SO have 'default' visibility. This is intended to be used
// with '-fvisibility=hidden'. This implementation hasn't yet been
// tested; it is based on comments given here
//   http://mail.python.org/pipermail/c++-sig/2004-April/007139.html
// by the author of the gcc 'visibility' patch.
//
// - For msw, LMI_SO controls the dll export-import attributes,
// depending on which of LMI_BUILD_SO (export) and LMI_USE_SO (import)
// is defined. Exactly one of LMI_BUILD_SO and LMI_USE_SO must be
// defined.

// Whether using these attributes is beneficial is an interesting
// question.
//
// For msw--the platform for which they were originally devised--a
// case can be made against using them with gcc. The maintainers have
// striven to render them unnecessary since version 3 or so; as this
// is written in early 2005, gcc imports symbols from dlls quite
// robustly without them. Omitting them makes building slightly more
// efficient--see
//   http://article.gmane.org/gmane.comp.gnu.mingw.user/14751
//     [2005-01-18T01:09:29Z from Greg Chicares]
// An important benefit of omitting them is that static and shared
// libraries can then be built from the same object files.
//
// On the other hand, for elf platforms, gcc-4.x uses a deliberately
// compatible attribute for symbol visibility, a feature for which
// significant benefits are claimed:
//   http://www.nedprod.com/programs/gccvisibility.html
//     [if that becomes inaccessible, this began as a copy:
//   https://gcc.gnu.org/wiki/Visibility
//     but, being a wiki, it has been subject to erosion; the
//     implementation here is closer to the authoritative original]
// The elf 'visibility' and msw dll 'declspec' attributes are similar
// enough to use the same decoration macro, but they do different
// things. In code that uses the shared object, the macro has an
// empty definition for elf, but for msw, it expands to a pseudo
// pragma that is designed to avoid a thunk.
//
// At any rate, for compilers other than gcc on the msw platform, the
// dll attributes are generally required.

#ifndef so_attributes_hpp
#define so_attributes_hpp

#include "config.hpp"

#if defined LMI_USE_SO_ATTRIBUTES
#
#   if defined LMI_MSW
#       define LMI_IMPORT __declspec(dllimport)
#       define LMI_EXPORT __declspec(dllexport)
#       define LMI_HIDDEN
#       define LMI_PUBLIC
#   elif defined __GNUC__
#       define LMI_IMPORT
#       define LMI_EXPORT __attribute__((visibility("default")))
#       define LMI_HIDDEN __attribute__((visibility("hidden")))
#       define LMI_PUBLIC __attribute__((visibility("default")))
#   else  // !defined LMI_MSW && !defined __GNUC__
#       error Unknown platform. Consider contributing support.
#   endif // !defined LMI_MSW && !defined __GNUC__
#
#   if defined LMI_BUILD_SO && defined LMI_USE_SO
#       error Both LMI_BUILD_SO and LMI_USE_SO defined.
#   endif // defined LMI_BUILD_SO && defined LMI_USE_SO
#
#   if defined LMI_BUILD_SO
#       define LMI_SO LMI_EXPORT
#   elif defined LMI_USE_SO
#       define LMI_SO LMI_IMPORT
#   else  // !defined LMI_BUILD_SO && !defined LMI_USE_SO
#       error Neither LMI_BUILD_SO nor LMI_USE_SO defined.
#   endif // !defined LMI_BUILD_SO && !defined LMI_USE_SO
#
#else  // !defined LMI_USE_SO_ATTRIBUTES
#   define LMI_SO
#endif // !defined LMI_USE_SO_ATTRIBUTES

// Forward declaration macro.
//
// If a class defined with an attribute chosen by a macro:
//   class ATTRIB foo {}
// is to be forward declared, should the forward declaration specify
// the attribute macro, or not? For some ancient msw compilers, this:
//   class ATTRIB foo;
// was apparently required, and it was accepted by mingw.org gcc up
// to version 3.4.5 at least. However, for MinGW-w64 gcc since at
// least version 4.9.1, it elicits a warning, and ATTRIB must be
// dropped from the forward declaration, thus:
//   class foo;
// Here, the latter form is used by default, and the former form is
// preserved in case it's ever needed again (below, with a jocular
// conditional evoking ancient tools). See:
//   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00097.html

#if defined LMI_USE_SO_ATTRIBUTES
#   if defined __BORLANDC__
#       define LMI_SO_FWD_DECL LMI_SO
#   else  // !defined __BORLANDC__
#       define LMI_SO_FWD_DECL
#   endif // !defined __BORLANDC__
#else  // !defined LMI_USE_SO_ATTRIBUTES
#   define LMI_SO_FWD_DECL
#endif // !defined LMI_USE_SO_ATTRIBUTES

#endif // so_attributes_hpp
