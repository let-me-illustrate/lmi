dnl configure.ac script for lmi, process with autoconf to create configure
dnl
dnl Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Vadim Zeitlin.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License version 2 as
dnl published by the Free Software Foundation.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software Foundation,
dnl Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
dnl
dnl http://savannah.nongnu.org/projects/lmi
dnl email: <gchicares@sbcglobal.net>
dnl snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

dnl === Prologue ===

AC_REVISION($Id$)dnl

AC_PREREQ(2.56)
AC_INIT(LMI, 1.0.0, [gchicares@sbcglobal.net])
AM_INIT_AUTOMAKE()
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([lmi.png])
AC_CONFIG_HEADER([config.h])

dnl remember, "build" is where we compile, "host" is where the resulting
dnl program runs (which may be different from "build" for cross-compilation)
AC_CANONICAL_HOST

USE_LINUX=0
USE_WINDOWS=0
case "${host}" in
    *-*-linux* )
        USE_LINUX=1
        ;;

    *-*-cygwin* )
        USE_CYGWIN=1
        USE_WINDOWS=1
        ;;

    *-*-mingw32* )
        USE_MINGW32=1
        USE_WINDOWS=1
        ;;
esac

AM_CONDITIONAL(LMI_MSW, [test "$USE_WINDOWS" = 1])

dnl === Command line options ===

AC_ARG_WITH(boost-headers,
            [AC_HELP_STRING([--with-boost-headers=dir], [Boost headers directory])],
            lmi_boost_headers=$withval)
AC_ARG_WITH(boost-libs,
            [AC_HELP_STRING([--with-boost-libs=dir], [Boost libs installation directory])],
            lmi_boost_libs=$withval)
AC_ARG_WITH(boost-toolkit,
            [AC_HELP_STRING([--with-boost-toolkit=dir], [Boost installation toolkit (gcc, mgw, etc.)])],
            lmi_boost_toolkit=$withval)
AC_ARG_WITH(boost-filesystem-src,
            [AC_HELP_STRING([--with-boost-filesystem-src=dir], [Boost filesystem library sources directory (./libs/filesystem/src directory in the boost default package). That option is only useful for the in-project boost::filesystem library compilation])],
            lmi_boost_filesystem_src=$withval)

dnl add support for --wxdir, --wx-config, --wx-prefix, ----wx-exec-prefix
WX_CONFIG_OPTIONS

AC_ARG_ENABLE(debug,
             [AC_HELP_STRING([--enable-debug],
                             [compile with debug information])],
             [case "x${enableval}" in
                   x) debug_option=yes ;;
                xyes) debug_option=yes ;;
                 xno) debug_option=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
             esac], [debug_option=yes])
AC_ARG_ENABLE(optimize,
             [AC_HELP_STRING([--disable-optimize],
                             [generate non-optimized code])],
             [case "x${enableval}" in
                   x) optimize_option=no ;;
                xyes) optimize_option=yes ;;
                 xno) optimize_option=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-optimize) ;;
             esac], [optimize_option=default])

AC_ARG_ENABLE(profiling,
             [AC_HELP_STRING([--enable-profiling],
                             [generate a code suitable for profiling (note: linux only, using gprof)])],
             [case "x${enableval}" in
                   x) profiling_option=no ;;
                xyes) profiling_option=yes ;;
                 xno) profiling_option=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-profiling) ;;
             esac], [profiling_option=default])

AC_ARG_ENABLE(mpatrol,
             [AC_HELP_STRING([--with-mpatrol],
                             [use mpatrol for memory allocation debugging])],
             [case "x${withvap}" in
                   x) lmi_mpatrol_option=yes ;;
                xyes) lmi_mpatrol_option=yes ;;
                 xno) lmi_mpatrol_option=no ;;
                   *) AC_MSG_ERROR(bad value ${withvap} for --with-mpatrol) ;;
             esac], lmi_mpatrol_option=no)
AM_CONDITIONAL(LMI_WITH_MEMORY_DEBUG, [ test "x$lmi_mpatrol_option" = "xyes" ])

AC_ARG_ENABLE(cgicc,
             [AC_HELP_STRING([--enable-cgicc],
                             [build cgi targets using installed cgicc.])],
             [case "x${enableval}" in
                   x) lmi_cgicc_option=yes;;
                xyes) lmi_cgicc_option=yes;;
                 xno) lmi_cgicc_option=no;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-cgicc[=yes|no]) ;;
             esac],
             lmi_cgicc_option=no)

dnl Note: PCH is not enabled by default, because it doesn't bring any
dnl       speed improvements on typical setups. Because of that, it doesn't
dnl       seem worth autodetecting PCH support either.
AC_ARG_ENABLE(pch,
             [AC_HELP_STRING([--enable-pch],
                             [use GCC precompiled headers])],
             [case "x${enableval}" in
                   x) lmi_use_pch=yes ;;
                xyes) lmi_use_pch=yes ;;
                 xno) lmi_use_pch=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-pch[=yes|no]) ;;
             esac], lmi_use_pch=no)
AM_CONDITIONAL(LMI_USE_PCH, [ test "x$lmi_use_pch" = "xyes" ])

dnl Pass standard options to wx.
WX_STANDARD_OPTIONS([debug,shared,toolkit,wxshared])

dnl make the installation prefix available to C++ code
if test "x$prefix" != "xNONE"; then
    lmi_prefix=$prefix
else
    lmi_prefix=$ac_default_prefix
fi

AC_DEFINE_UNQUOTED(LMI_INSTALL_PREFIX, "$lmi_prefix", [Program installation prefix])


dnl === Program checks ===

dnl this magic incantation is needed to prevent AC_PROG_CC from setting the
dnl default CFLAGS to "-g -O2") -- we don't need this as we add -g and -O
dnl flags ourselves below (and "-g" is inappropriate for msvc anyhow)
CFLAGS=${CFLAGS:=}
AC_PROG_CC

CXXFLAGS=${CXXFLAGS:=}
AC_PROG_CXX

AC_PROG_LD

if test "$USE_WINDOWS" = "1"; then
    AC_CHECK_TOOL([WINDRES], [windres], no)
fi

dnl Until the ihs_pios.*pp facilities are made portable use explicit flag to GCC
dnl to ensure that sizeof(long double) is 12 bytes for binary compatibility.
if test "x$GXX" == "xyes"; then
    CPPFLAGS="$CPPFLAGS -m96bit-long-double"
fi

dnl add debug option if necessary
if test "$debug_option" = "yes"; then
    case "$CC" in
        cl) DEBUG_FLAG="/Zi";;
        *)  DEBUG_FLAG="-g";;
    esac

    CFLAGS="$CFLAGS $DEBUG_FLAG"
    CXXFLAGS="$CXXFLAGS $DEBUG_FLAG"
    LDFLAGS="$LDFLAGS $DEBUG_FLAG"

    dnl disable optimizations by default in debug build but don't disable them
    dnl if explicit --enable-optimize was given
    if test "$optimize_option" != "yes"; then
        optimize_option="no"
    fi
else
    CPPFLAGS="$CPPFLAGS -DNDEBUG"
fi

dnl and optimize options too
if test "$optimize_option" = "default"; then
    optimize_option="yes"
fi

if test "$optimize_option" = "yes"; then
    case "$CC" in
        cl)     OPT_FLAG="/O";;
        gcc)    OPT_FLAG="-O2";;
        *)      OPT_FLAG="-O";;
    esac

    CFLAGS="$CFLAGS $OPT_FLAG"
    CXXFLAGS="$CXXFLAGS $OPT_FLAG"

    dnl disable profiling by default when optimizing but don't disable it
    dnl if explicit --enable-profiling was given
    if test "$profiling_option" != "yes"; then
        profiling_option="no"
    fi
fi

dnl add profiling information if necessary
if test "$profiling_option" = "yes"; then
    case "$CC" in
        cl) PROFILING_FLAG="";;
        *)  PROFILING_FLAG="-pg";;
    esac

    LDFLAGS="$LDFLAGS $PROFILING_FLAG"
fi

dnl === Checks for standard headers/functions ===

AC_CHECK_HEADERS(curses.h)
if test "$ac_cv_header_curses_h" != "yes"; then
    AC_CHECK_HEADERS(conio.h)
    if test "$ac_cv_header_conio_h" != "yes"; then
        AC_MSG_ERROR([Header for getch() not found: neither curses.h nor conio.h are available.])
    fi
fi

dnl the functions below are typically found in libm and not libc so make sure
dnl we look there
AC_SEARCH_LIBS([log10l], [m],
    [AC_DEFINE(HAVE_LOG10L, [1], [Define this if you have log10l function])]
)
AC_SEARCH_LIBS([fabsl], [m],
    [AC_DEFINE(HAVE_FABSL, [1], [Define this if you have fabsl function])]
)

dnl check for snprintf: note that when compiling under gcc using -std= flag the
dnl standard headers do not declare snprintf() and this is why we don't use
dnl the usual AC_CHECK_FUNCS but reproduce the same workaround as used in
dnl platform_dependent.hpp (see comment there) which makes it possible to
dnl detect this function in all cases
AC_CACHE_CHECK([for snprintf],
    lmi_cv_func_snprintf,
    [
        AC_LINK_IFELSE([AC_LANG_PROGRAM(
                [[
                #if defined __GNUC__ && defined __STRICT_ANSI__
                #   undef __STRICT_ANSI__
                #endif
                #include <stdio.h>]],
                [[
                char buf[1];
                return snprintf(buf, 1, "%i", 0);
                ]]
            )],
            lmi_cv_func_snprintf=yes,
            lmi_cv_func_snprintf=no
        )
    ]
)
if test "x$lmi_cv_func_snprintf" = "xyes"; then
    AC_DEFINE([HAVE_SNPRINTF], [1], [Define this if you have snprintf function])
else
    dnl TODO: for MSVC support we should check for _snprintf()
    AC_MSG_ERROR([The snprintf() function not found on your system])
fi

AM_CONDITIONAL(HAVE_SNPRINTF, [test "x$lmi_cv_func_snprintf" == "xyes"])
AM_CONDITIONAL(LMI_WITH_CGI, [test "x$lmi_cgicc_option" == "xyes"])

dnl existing code already uses these macros so continue to use them instead of
dnl the usual HAVE_STRTOF etc.
AC_CHECK_FUNC(strtof, AC_DEFINE(LMI_COMPILER_PROVIDES_STRTOF, [1], [Define this if you have strtof() function]))
AC_CHECK_FUNC(strtold, AC_DEFINE(LMI_COMPILER_PROVIDES_STRTOLD, [1], [Define this if you have strtold() function]))
AC_CHECK_FUNC(rint, AC_DEFINE(LMI_COMPILER_PROVIDES_RINT, [1], [Define this if you have rint() function]))

dnl === Library checks ===

dnl --- curses.h
AC_SEARCH_LIBS([getch], [ncurses], [], [], [])

dnl === Configure libtool ===

dnl We can't do this at the end because we need the correct values
dnl for enable_shared/static variables which appears to be defined
dnl by AC_PROG_LIBTOOL (for ex. with --disable-shared you get only
dnl enable_shared=no before this point and enable_static=yes only after)
dnl Wx configuration part needs exactly that feature to understand
dnl what library type build we're using.
AC_DISABLE_STATIC   dnl don't waste time on static build by default
AC_PROG_LIBTOOL

dnl use C++ compiler as we're checking for C++ libraries/headers from now on
dnl (we couldn't do it before as libtool tests must be done with C compiler)
AC_LANG(C++)

dnl --- wxWidgets ---

WX_UNICODE=no

WX_CONVERT_STANDARD_OPTIONS_TO_WXCONFIG_FLAGS
WX_CONFIG_CHECK([2.8.0], [wx="yes"],
                         [AC_MSG_ERROR([No wxWidgets headers libs were found!])],
                         [],[$WXCONFIG_FLAGS])

save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS"

dnl check that the library options allow to build lmi
AC_TRY_COMPILE([#include <wx/setup.h>],
    [
    #if !wxUSE_DOC_VIEW_ARCHITECTURE || \
        !wxUSE_MDI_ARCHITECTURE || \
        !wxUSE_PRINTING_ARCHITECTURE || \
        !(wxUSE_STL || wxUSE_STD_STRING)
        wrong wx library options
    #endif
    ],
    [],
    AC_MSG_ERROR([Please configure wxWidgets using --enable-std_string.])
)

dnl these options are not fatal but it's better not to use them for various
dnl (mostly licensing) reasons
AC_TRY_COMPILE([#include <wx/setup.h>],
    [
    #if wxUSE_APPLE_IEEE || \
        wxUSE_LIBJPEG || \
        wxUSE_LIBTIFF || \
        wxUSE_GIF || \
        wxUSE_THREADS
        inappropriate wx library options
    #endif
    ],
    [],
    AC_MSG_WARN([Inappropriate wxWidgets build options detected but continuing.])
)

CXXFLAGS=$save_CXXFLAGS

WX_STANDARD_OPTIONS_SUMMARY_MSG

dnl --- Boost (required) ---
if test "x$lmi_boost_headers" != "x"; then
    CPPFLAGS="$CPPFLAGS -I$lmi_boost_headers"
    errmsg="even in $lmi_boost_headers"
else
    errmsg="use --with-boost-headers=dir
if they are installed in non default location"
fi

AC_CHECK_HEADER([boost/type_traits.hpp],
    [],
    [AC_MSG_ERROR([Boost headers not found, $errmsg])]
)

AC_CHECK_HEADER([boost/math/special_functions/expm1.hpp],
    [],
    [AC_MSG_ERROR([Boost version with boost::math::expm1() required, $errmsg])]
)

dnl By default no particular toolkit suffix should be needed when linking
dnl against boost libraries. Except for mingw where it seems to be
dnl required. Because under mingw default installation does _not_ create
dnl a symbolic link without the tollkit suffix (-mgw), at least
dnl for the version 1.33.1. Therefore explicitly set it if no toolkit specified.
if test "x$lmi_boost_toolkit" = "x"; then
    if test "x$USE_MINGW32" = "x1"; then
        lmi_boost_toolkit=mgw
    fi
fi

dnl The default boost library name should be used unless otherwise specified
dnl via --with-boost-tookit options, in which case we need to add
dnl toolkit suffix to the library name (example -lboost_filesystem-gcc-mt).
if test "x$lmi_boost_toolkit" != "x"; then
    lmi_boost_toolkit="-$lmi_boost_toolkit"
fi
boost_libsystem="boost_system$lmi_boost_toolkit"
boost_libfs="boost_filesystem$lmi_boost_toolkit"
boost_libregex="boost_regex$lmi_boost_toolkit"

save_LIBS=$LIBS
save_LDFLAGS=$LDFLAGS
if test "x$lmi_boost_libs" != "x"; then
    BOOST_LDFLAGS="$BOOST_LDFLAGS -L$lmi_boost_libs "
    LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
    errmsg="even in $lmi_boost_libs"
else
    errmsg="use --with-boost-libs=dir
if it is installed in non default location"
fi

dnl latest versions of boost extract some common functionality in
dnl libboost_system which we must link when using any other Boost library, so
dnl check for it first
AC_CHECK_LIB($boost_libsystem,main,BOOST_LIBS="-l$boost_libsystem")

dnl usually when we want to check for a library we use AC_CHECK_LIB and main()
dnl but since under mingw with -pedantic and -std= options test does not pass
dnl we are forced to use a bit more sophisticated construct

LIBS="$LIBS $BOOST_LIBS -l$boost_libfs"
LMI_BOOSTFS_BUILD=0
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/filesystem/operations.hpp>]],
    [[boost::filesystem::current_path()]])],
    [],
    [LMI_BOOSTFS_BUILD=1
    AC_MSG_WARN([Boost filesystem library $boost_libfs not found, $errmsg])]
)

LIBS="$save_LIBS -l$boost_libregex"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/regex.hpp>]],
    [[boost::regex *r]])],
    [],
    [AC_MSG_ERROR([Boost regex library $boost_libregex not found, $errmsg])]
)

LDFLAGS=$save_LDFLAGS
LIBS=$save_LIBS

if test "x$LMI_BOOSTFS_BUILD" == "x1"; then

dnl we have no boost::filesystem library available on the machine - lets check for its sources
LMI_BOOSTFS_SRC=$lmi_boost_filesystem_src
dnl if the user has not specified --with-boost-filesystem-src then guess it using --with-boost-headers
if test "x$LMI_BOOSTFS_SRC" == "x"; then
    if test "x$lmi_boost_headers" != "x"; then
        LMI_BOOSTFS_SRC="$lmi_boost_headers/../libs/filesystem/src"
    fi
fi
if test "x$LMI_BOOSTFS_SRC" == "x"; then
    LMI_BOOSTFS_SRC="."
fi

for f in convenience.cpp exception.cpp operations_posix_windows.cpp path_posix_windows.cpp; do
  if ! test -r $LMI_BOOSTFS_SRC/$f; then
    no_boost_fs=1
  fi
done

if test "x$no_boost_fs" == "x1"; then
    AC_MSG_ERROR([no boost::filesystem source files found in ($LMI_BOOSTFS_SRC)])
fi

BOOST_LIBS=third_party/boostfs/libboost_filesystem.la
else
BOOST_LIBS="$BOOST_LDFLAGS $BOOST_LIBS -l$boost_libfs"
fi

BOOST_LIBS="$BOOST_LIBS -l$boost_libregex"

dnl contains flags needed to link to externally or internally built
dnl boost libraries
AC_SUBST(BOOST_LIBS)

dnl indicates weither do we need to build internally boost::filesystem library
AM_CONDITIONAL([LMI_BOOSTFS_BUILD], [test "x$LMI_BOOSTFS_BUILD" == "x1"])
dnl indicates the directory path where sources could be found
AC_SUBST(LMI_BOOSTFS_SRC)

dnl check for the other required libraries: under Unix we use pkg-config which
dnl is easily available for (and often already installed under) any Linux
dnl system but when using MSYS we do the checks manually as installing
dnl pkg-config under Windows is unfortunately not trivial and so we can't rely
dnl on its presence on the target system
if test "$USE_LINUX" = "1"; then
    PKG_CHECK_MODULES(GTK, gtk+-2.0)
    PKG_CHECK_MODULES(XMLWRAPP, xmlwrapp >= 0.6.0 xsltwrapp)
else
    if test -z "$XMLWRAPP_CFLAGS" -o -z "$XMLWRAPP_LIBS"; then
        dnl --- XMLWRAPP (required) -------------------
        AC_CHECK_PROG(lmi_xmlwrapp_config, xmlwrapp-config, yes, no)
        if test "x$lmi_xmlwrapp_config" != "xyes"; then
            AC_MSG_ERROR([Unable to find xmlwrapp configuration program xmlwrapp-config in the path.])
        fi

        dnl make sure xmlwrapp was compiled with XSLT support:
        if xmlwrapp-config --libs | grep -v -q xslt; then
            AC_MSG_ERROR([xmlwrapp with XSLT support is required.])
        fi
    fi

    if test -z "$XMLWRAPP_CFLAGS"; then
        XMLWRAPP_CFLAGS=`xmlwrapp-config --cxxflags`
    fi

    if test -z "$XMLWRAPP_LIBS"; then
        XMLWRAPP_LIBS=`xmlwrapp-config --libs`
    fi

    AC_SUBST(XMLWRAPP_CFLAGS)
    AC_SUBST(XMLWRAPP_LIBS)
fi

save_CXXFLAGS=$CXXFLAGS
save_LIBS=$LIBS
CXXFLAGS="$CXXFLAGS $XMLWRAPP_CFLAGS"
LIBS="$LIBS $XMLWRAPP_LIBS"

dnl check that the library options allow to build lmi
AC_TRY_COMPILE([#include <xmlwrapp/xmlwrapp.h>],
    [xml::document doc("dummy");],
    [],
    AC_MSG_ERROR([Unable to find appropriate xmlwrapp configuration. Use --with-xmlwrapp-config to specify the location of xmlwrapp-config.])
)

CXXFLAGS=$save_CXXFLAGS
LIBS=$save_LIBS

dnl --- CGICC (optional) ----------------
if test "x$lmi_cgicc_option" != "xno"; then
    lmi_found_cgicc=yes
    AC_CHECK_HEADER("cgicc/CgiDefs.h", [], [lmi_found_cgicc=no])
    AC_CHECK_FILE("libcgicc.la", [], [lmi_found_cgicc=no])
    if test "x$lmi_found_cgicc" == "xno"; then
        for m_lmi_cgicc_guess in /usr/include /usr/local/include
        do
            echo "checking with m_lmi_cgicc_guess=$m_lmi_cgicc_guess"
            AC_CHECK_HEADER([$m_lmi_cgicc_guess/cgicc/CgiDefs.h],
                [ok_lmi_cgicc_include=$m_lmi_cgicc_guess], [])
            if test "x$ok_lmi_cgicc_include" != "x"; then break; fi
        done
        for m_lmi_cgicc_lib_guess in /usr/lib /usr/local/lib
        do
            echo "checking with m_lmi_cgicc_lib_guess=$m_lmi_cgicc_lib_guess"
            AC_CHECK_FILE([$m_lmi_cgicc_lib_guess/libcgicc.la],
                [ok_lmi_cgicc_lib="$m_lmi_cgicc_lib_guess"], [])
            if test "x$ok_lmi_cgicc_lib" != "x"; then break; fi
        done
        if test "x$ok_lmi_cgicc_include" != "x"; then
            if test "x$ok_lmi_cgicc_lib" != "x"; then
                lmi_found_cgicc=yes
            fi
        fi
    fi
    if test "x$lmi_cgicc_option" == "xyes"; then
        if test "x$lmi_found_cgicc" == "xno"; then
            AC_MSG_ERROR([Cannot find cgicc installation on the system. Please install cgicc...])
        fi
    fi
    lmi_cgicc_option=$lmi_found_cgicc
fi

if test "x$lmi_cgicc_option" == "xyes"; then
    CGICC_INCLUDES=$ok_lmi_cgicc_include
    CGICC_INCLUDE_FLAGS="-I $ok_lmi_cgicc_include"
    CGICC_LIBS="$ok_lmi_cgicc_lib/libcgicc.la"
    CGICC_LIB_LDADD="-l$ok_lmi_cgicc_lib/libcgicc.la"
else
    CGICC_INCLUDES=
    CGICC_INCLUDE_FLAGS=
    CGICC_LIBS=
    CGICC_LIB_LDADD=
fi

AM_CONDITIONAL(LMI_WITH_CGI, [test "x$lmi_cgicc_option" == "xyes"])
AC_SUBST(CGICC_INCLUDES)
AC_SUBST(CGICC_INCLUDE_FLAGS)
AC_SUBST(CGICC_LIBS)
AC_SUBST(CGICC_LIB_LDADD)

dnl --- enable all possible warnings for gcc/g++ ----
CFLAGS_gcc_common="\
    -pedantic-errors \
    -Werror \
    -Wall \
    -Wcast-align \
    -Wdeprecated-declarations \
    -Wdisabled-optimization \
    -Wimport \
    -Wmultichar \
    -Wno-long-long \
    -Wpacked \
    -Wpointer-arith \
    -Wsign-compare \
    -Wundef \
    -Wwrite-strings"

if test "x$GCC" == "xyes"; then
    CFLAGS="$CFLAGS $CFLAGS_gcc_common -std=c99 -Wmissing-prototypes"
fi

if test "x$GXX" == "xyes"; then
    CXXFLAGS="$CXXFLAGS $CFLAGS_gcc_common \
        -Wctor-dtor-privacy \
        -Wdeprecated \
        -Wnon-template-friend \
        -Woverloaded-virtual \
        -Wpmf-conversions \
        -Wsynth"

    dnl g++ 4.4 doesn't compile wxWidgets code that relies on use of empty
    dnl macro arguments, because it's behavior undefined by the standard
    dnl prior to C++0x or C99; if the compiler is too old to understand
    dnl -std=c++0x, it's also too old to complain about this and so we can
    dnl use -std=c++98
    AX_CXX_CHECK_FLAG([-std=c++0x],[],[],
                      [CXXFLAGS="$CXXFLAGS -std=c++0x"],
                      [CXXFLAGS="$CXXFLAGS -std=c++98"])

    dnl we need to use this option with g++ 4.3 to prevent its complaints
    dnl about "-funit-at-a-time is required for inlining of functions that are
    dnl only called once" in debug builds
    LMI_CXX_ADD_IF_SUPPORTED(-fno-inline-functions-called-once)

    dnl this one must be disabled with g++ 4.x because there are tons of
    dnl occurrences of this warning in both wx and LMI sources
    LMI_CXX_ADD_IF_SUPPORTED(-Wno-parentheses)

    dnl This makes Boost compile with -Wundef; see workhorse.make for
    dnl more detailed explanation.
    CXXFLAGS="$CXXFLAGS -DBOOST_STRICT_CONFIG"
fi

dnl === Generate output files ===
AC_CONFIG_FILES([Makefile third_party/Makefile third_party/boostfs/Makefile])
AC_OUTPUT
