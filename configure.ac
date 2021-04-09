dnl configure.ac script for lmi, process with autoconf to create configure
dnl
dnl Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Vadim Zeitlin.
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
dnl https://savannah.nongnu.org/projects/lmi
dnl email: <gchicares@sbcglobal.net>
dnl snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

dnl === Prologue ===

AC_PREREQ(2.56)
AC_INIT(lmi, 1.0.0, [gchicares@sbcglobal.net])
AM_INIT_AUTOMAKE([foreign])
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
        USE_WINDOWS=1
        ;;

    *-*-mingw32* )
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

dnl Check if the compiler supports C++17: this is a rather ad hoc test as it
dnl doesn't check for full C++17 support, but it's enough for the subset of it
dnl that is currently used in lmi and this check passes with with g++ 6, which
dnl can be used for compiling lmi even though it doesn't implement C++17 fully.
m4_define([lmi_cxx_std17_test], [[
#ifndef __cplusplus
#error Not C++ compiler.
#elif __cplusplus <= 201402L
#error Only C++14.
#else
static_assert(true);
#endif
]])

AC_CACHE_CHECK([whether compiler supports C++17 without any options],
    lmi_cv_cxx_std17,
    [
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([lmi_cxx_std17_test])],
            lmi_cv_cxx_std17=yes,
            lmi_cv_cxx_std17=no
            )
        AC_LANG_POP([C++])
    ]
)

if test "$lmi_cv_cxx_std17" = "no"; then
    AX_CXX_CHECK_FLAG([-std=c++17],,,[CXXFLAGS="$CXXFLAGS -std=c++17"],
        AC_MSG_ERROR([Can't enable C++17 support required for building lmi.])
    )
fi

dnl These options must be always used for lmi unit tests to pass.
common_options="-fno-ms-extensions -frounding-math"

CFLAGS="$CFLAGS $common_options"
CXXFLAGS="$CXXFLAGS $common_options"

AC_PROG_LD

# This is a workaround for the harmless but annoying warning
#
# ar: `u' modifier ignored since `D' is the default (see `U')
#
# given by Linux systems using recent binutils with the switch to building
# deterministic archives (that can't include the timestamps) when building all
# static libraries with default "cru" flags used by Libtool up to 2.4.6.
AR_FLAGS=cr

AC_CACHE_CHECK([if using clang],
    lmi_cv_prog_clang,
    [
        AC_LANG_PUSH([C++])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
                [],
                [[
                #ifndef __clang__
                    not clang
                #endif
                ]],
            )],
            lmi_cv_prog_clang=yes,
            lmi_cv_prog_clang=no
        )
        AC_LANG_POP([C++])
    ]
)

CLANG=$lmi_cv_prog_clang

if test "$USE_WINDOWS" = "1"; then
    AC_CHECK_TOOL([WINDRES], [windres], no)
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
        *gcc)   OPT_FLAG="-O2";;
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

AM_CONDITIONAL(LMI_WITH_CGI, [test "x$lmi_cgicc_option" == "xyes"])

dnl === Library checks ===

dnl --- curses.h
AC_SEARCH_LIBS([getch], [ncurses], [], [], [])

dnl === Configure libtool ===

dnl Hack: ensure that libtool doesn't do anything "smart" to determine whether
dnl it can link MSW DLLs with their dependencies because this is never a
dnl problem for this platform.
dnl
dnl This obviously relies on the details of libtool implementation and can
dnl break with the future libtool versions (last version tested is 2.4.2).
if test "$USE_WINDOWS" = "1"; then
    lt_cv_deplibs_check_method=pass_all
fi

dnl We can't do this at the end because we need the correct values
dnl for enable_shared/static variables which appears to be defined
dnl by AC_PROG_LIBTOOL (for ex. with --disable-shared you get only
dnl enable_shared=no before this point and enable_static=yes only after)
dnl Wx configuration part needs exactly that feature to understand
dnl what library type build we're using.
LT_INIT([disable-static,win32-dll])

dnl use C++ compiler as we're checking for C++ libraries/headers from now on
dnl (we couldn't do it before as libtool tests must be done with C compiler)
AC_LANG(C++)

dnl --- wxWidgets ---

WX_CONFIG_CHECK([3.0.0], [wx="yes"],
                         [AC_MSG_FAILURE([No wxWidgets headers libs were found!])],
                         [],[$WXCONFIG_FLAGS])

save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"

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
    AC_MSG_FAILURE([Please configure wxWidgets using --enable-std_string.])
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

WX_DETECT_STANDARD_OPTION_VALUES
WX_STANDARD_OPTIONS_SUMMARY_MSG

dnl --- wxPdfDocument ---
AC_CHECK_HEADER([wx/pdfdc.h],
    [],
    [AC_MSG_FAILURE([wxPdfDocument header not found.])],
    [AC_INCLUDES_DEFAULT()]
)

dnl Defer to the same macro used by wxPdfDocument itself for constructing its
dnl library name.
WX_LIKE_LIBNAME([pdfdoc_lib], [wxcode], [pdfdoc])
AC_CHECK_LIB([$pdfdoc_lib], [main],
    [WXPDFDOC_LIBS="-l$pdfdoc_lib"],
    [AC_MSG_FAILURE([wxPdfDocument library $pdfdoc_lib not found.])])

AC_SUBST(WXPDFDOC_LIBS)

dnl Restore pre-wx CPPFLAGS value only now, we needed wx flags for
dnl wxPdfDocument checks.
CPPFLAGS=$save_CPPFLAGS

dnl --- Boost (required) ---
if test "x$lmi_boost_headers" != "x"; then
    CPPFLAGS="$CPPFLAGS -I$lmi_boost_headers"
    errmsg="even in $lmi_boost_headers"
else
    errmsg="use --with-boost-headers=dir
if they are installed in non default location"
fi

dnl Disable the use of std::auto_ptr<> which is not available in C++17 (and is
dnl actually not available, rather than still being there and just generating
dnl deprecation warnings, in clang libc++).
CPPFLAGS="$CPPFLAGS -DBOOST_NO_AUTO_PTR"

AC_CHECK_HEADER([boost/type_traits.hpp],
    [],
    [AC_MSG_FAILURE([Boost headers not found, $errmsg])],
    [AC_INCLUDES_DEFAULT()]
)

dnl The default boost library name should be used unless otherwise specified
dnl via --with-boost-tookit options, in which case we need to add
dnl toolkit suffix to the library name (example -lboost_filesystem-gcc-mt).
if test "x$lmi_boost_toolkit" != "x"; then
    lmi_boost_toolkit="-$lmi_boost_toolkit"
fi
boost_libsystem="boost_system$lmi_boost_toolkit"
boost_libfs="boost_filesystem$lmi_boost_toolkit"
boost_libregex="boost_regex$lmi_boost_toolkit"

save_CPPFLAGS=$CPPFLAGS
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

dnl Prevent warnings due to the use of "register" in Boost.Regex headers that
dnl are actually fatal errors with clang in C++17 mode.
CPPFLAGS="$save_CPPFLAGS -Dregister="

dnl latest versions of boost extract some common functionality in
dnl libboost_system which we must link when using any other Boost library, so
dnl check for it first
AC_CHECK_LIB($boost_libsystem,main,BOOST_LIBS="-l$boost_libsystem")

dnl usually when we want to check for a library we use AC_CHECK_LIB and main()
dnl but since under mingw with -pedantic and -std= options test does not pass
dnl we are forced to use a bit more sophisticated construct

LIBS="$LIBS $BOOST_LIBS -l$boost_libfs"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/filesystem/operations.hpp>]],
    [[boost::filesystem::current_path()]])],
    [],
    [AC_MSG_FAILURE([Boost filesystem library $boost_libfs not found, $errmsg])]
)

LIBS="$save_LIBS -l$boost_libregex"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/regex.hpp>]],
    [[boost::regex *r]])],
    [],
    [AC_MSG_FAILURE([Boost regex library $boost_libregex not found, $errmsg])]
)

LDFLAGS=$save_LDFLAGS
LIBS=$save_LIBS
CPPFLAGS=$save_CPPFLAGS

BOOST_LIBS="-l$boost_libfs -l$boost_libregex $BOOST_LIBS"

dnl contains flags needed to link to externally or internally built
dnl boost libraries
AC_SUBST(BOOST_LIBS)

dnl check for the other required libraries: under Unix we use pkg-config which
dnl is easily available for (and often already installed under) any Linux
dnl system but when using Cygwin we do the checks manually as installing
dnl pkg-config under Windows is unfortunately not trivial and so we can't rely
dnl on its presence on the target system
if test "$USE_LINUX" = "1"; then
    PKG_CHECK_MODULES(XMLWRAPP, xmlwrapp >= 0.7.0 xsltwrapp)
else
    if test -z "$XMLWRAPP_CFLAGS" -o -z "$XMLWRAPP_LIBS"; then
        dnl --- XMLWRAPP (required) -------------------
        AC_CHECK_PROGS(lmi_xmlwrapp_config, xmlwrapp-config, not-found)
        if test "$lmi_xmlwrapp_config" = "not-found"; then
            AC_MSG_ERROR([Unable to find xmlwrapp configuration program xmlwrapp-config in the path.])
        fi

        dnl make sure xmlwrapp was compiled with XSLT support:
        if $lmi_xmlwrapp_config --libs | grep -v -q xslt; then
            AC_MSG_ERROR([xmlwrapp with XSLT support is required.])
        fi
    fi

    if test -z "$XMLWRAPP_CFLAGS"; then
        XMLWRAPP_CFLAGS=`$lmi_xmlwrapp_config --cxxflags`
    fi

    if test -z "$XMLWRAPP_LIBS"; then
        XMLWRAPP_LIBS=`$lmi_xmlwrapp_config --libs`
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
    AC_MSG_FAILURE([Unable to find appropriate xmlwrapp configuration. Use --with-xmlwrapp-config to specify the location of xmlwrapp-config.])
)

CXXFLAGS=$save_CXXFLAGS
LIBS=$save_LIBS

dnl --- libunwind and libdw, only used under Linux and only with gcc/libstdc++
if test "$USE_LINUX" = "1" -a "$CLANG" != "yes"; then
    PKG_CHECK_MODULES(UNWIND, libunwind libdw)
    CXXFLAGS="$CXXFLAGS $UNWIND_CFLAGS"
    LIBS="$LIBS $UNWIND_LIBS -ldl"
fi

dnl --- CGICC (optional) ----------------
if test "x$lmi_cgicc_option" != "xno"; then
    lmi_found_cgicc=yes
    AC_CHECK_HEADER("cgicc/CgiDefs.h", [], [lmi_found_cgicc=no], [AC_INCLUDES_DEFAULT()])
    AC_CHECK_FILE("libcgicc.la", [], [lmi_found_cgicc=no])
    if test "x$lmi_found_cgicc" == "xno"; then
        for m_lmi_cgicc_guess in /usr/include /usr/local/include
        do
            echo "checking with m_lmi_cgicc_guess=$m_lmi_cgicc_guess"
            AC_CHECK_HEADER([$m_lmi_cgicc_guess/cgicc/CgiDefs.h],
                [ok_lmi_cgicc_include=$m_lmi_cgicc_guess], [],
                [AC_INCLUDES_DEFAULT()]
            )
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

dnl Try to enable as many useful warnings as possible depending on the
dnl compiler (and its version) used.
if test "$GCC" == "yes"; then
    dnl These flags are understood by both gcc and clang and, hopefully, other
    dnl compatible compilers.
    c_warnings_flags="\
        -pedantic-errors \
        -Werror \
        -Wall \
        -Wextra \
        -Wundef"

    if test "$CLANG" = "yes"; then
        :
    else
        dnl Assume this is the real gcc.
        c_warnings_flags="$c_warnings_flags \
            -Wcast-align \
            -Wdisabled-optimization \
            -Wimport \
            -Wmultichar \
            -Wpacked \
            -Wpointer-arith \
            -Wshadow \
            -Wsign-compare \
            -Wunreachable-code \
            -Wwrite-strings"
    fi

    CFLAGS="$CFLAGS $c_warnings_flags -std=c99 -Wmissing-prototypes"
fi

if test "x$GXX" == "xyes"; then
    if test "$CLANG" = "yes"; then
        cxx_warnings_flags="$cxx_warnings_flags \
            -Wno-string-plus-int \
            -Wno-mismatched-tags"
    else
        cxx_warnings_flags="$cxx_warnings_flags \
        -Wctor-dtor-privacy \
        -Wno-deprecated-declarations \
        -Wnon-template-friend \
        -Woverloaded-virtual \
        -Wpmf-conversions \
        -Wsynth"
    fi

    CXXFLAGS="$CXXFLAGS $c_warnings_flags $cxx_warnings_flags"

    dnl we need to use this option with g++ 4.3 to prevent its complaints
    dnl about "-funit-at-a-time is required for inlining of functions that are
    dnl only called once" in debug builds
    dnl
    dnl NB: this is needed for any '*.c' so we must add it to CFLAGS too
    if test "$CLANG" != "yes"; then
        LMI_C_CXX_ADD_IF_SUPPORTED(-fno-inline-functions-called-once)
    fi

    dnl this one must be disabled because there are tons of occurrences of
    dnl this warning in both wx and lmi sources
    if test "$CLANG" = "yes"; then
        LMI_CXX_ADD_IF_SUPPORTED(-Wno-logical-op-parentheses)
        LMI_CXX_ADD_IF_SUPPORTED(-Wno-bitwise-op-parentheses)
    else
        LMI_CXX_ADD_IF_SUPPORTED(-Wno-parentheses)
    fi

    dnl Many instances of these warnings are given in Boost 1.33.1 headers, so
    dnl we unfortunately have to disable them even if they're potentially
    dnl useful, especially the latter one.
    LMI_CXX_ADD_IF_SUPPORTED(-Wno-unused-local-typedefs)
    LMI_CXX_ADD_IF_SUPPORTED(-Wno-unused-variable)

    dnl This makes Boost compile with -Wundef; see workhorse.make for
    dnl more detailed explanation.
    CXXFLAGS="$CXXFLAGS -DBOOST_STRICT_CONFIG"
fi

dnl === Generate output files ===

dnl These files are used by various tests and must be present in the build
dnl directory. The configurable settings file contains just the bare minimum
dnl required, but what little is there, is really needed for the tests to pass.
AC_CONFIG_COMMANDS([configurable_settings.xml], [
cat >configurable_settings.xml <<EOF
<?xml version="1.0"?>
<configurable_settings version="2">
  <print_directory>.</print_directory>
  <spreadsheet_file_extension>.tsv</spreadsheet_file_extension>
</configurable_settings>
EOF
])
AC_CONFIG_COMMANDS([sample.policy], [touch sample.policy])
AC_CONFIG_LINKS([sample.cns:sample.cns sample.ill:sample.ill])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
