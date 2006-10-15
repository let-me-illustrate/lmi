dnl configure.ac script for lmi, process with autoconf to create configure
dnl
dnl Copyright (C) 2005, 2006 Vadim Zeitlin.
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
dnl email: <chicares@cox.net>
dnl snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

dnl === Prologue ===

AC_REVISION($Id: configure.ac,v 1.22.2.1 2006-10-15 23:26:24 etarassov Exp $)dnl

AC_PREREQ(2.56)
AC_INIT(LMI, 1.0.0, [chicares@cox.net])
AM_INIT_AUTOMAKE()

AC_CONFIG_SRCDIR([lmi.xpm])
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
            [AC_HELP_STRING([--with-boost-filesystem-src=dir], [Boost filesystem library sources directory (./libs/filesystem/src directory in the boost default package). That option is only usefull for the in-project boost::filesystem library compilation])],
            lmi_boost_filesystem_src=$withval)


AC_ARG_WITH(wx,
            [AC_HELP_STRING([--with-wx=dir],
                            [wxWidgets directory (defaults to WXWIN env var)])],
            lmi_wx_dir=$withval)
AC_ARG_WITH(wx-version,
            [AC_HELP_STRING([--with-wx-ver=version],
                            [wxWidgets version to search for (default 2.6, 2.7)])],
            lmi_wx_version=$withval)
AC_ARG_WITH(wx-headers,
            [AC_HELP_STRING([--with-wx-headers=dir],
                            [wxWidgets header files directory])],
            lmi_wx_headers_dir=$withval)
AC_ARG_WITH(wx-libs,
            [AC_HELP_STRING([--with-wx-libs=dir],
                            [wxWidgets libraries directory])],
            lmi_wx_libs_dir=$withval)
AC_ARG_WITH(wx-setup,
            [AC_HELP_STRING([--with-wx-setup=dir],
                            [wx/setup.h directory])],
            lmi_wx_setup_dir=$withval)


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

dnl === Program checks ===

dnl this magic incantation is needed to prevent AC_PROG_CC from setting the
dnl default CFLAGS to "-g -O2") -- we don't need this as we add -g and -O
dnl flags ourselves below (and "-g" is inappropriate for msvc anyhow)
CFLAGS=${CFLAGS:=}
AC_PROG_CC

CXXFLAGS=${CXXFLAGS:=}
AC_PROG_CXX

AC_PROG_LD

dnl enable all possible warnings for gcc/g++
CFLAGS_gcc_common="\
    -pedantic \
    -Wall \
    -Wcast-align \
    -Wconversion \
    -Wdeprecated-declarations \
    -Wdisabled-optimization \
    -Wimport \
    -Wmultichar \
    -Wno-long-long \
    -Wpacked \
    -Wpointer-arith \
    -Wsign-compare \
    -Wwrite-strings"

if test "x$GCC" == "xyes"; then
    CFLAGS="$CFLAGS $CFLAGS_gcc_common -std=c99 -Wmissing-prototypes"
fi

if test "x$GXX" == "xyes"; then
    CXXFLAGS="$CXXFLAGS $CFLAGS_gcc_common -std=c++98 \
        -Wdeprecated \
        -Wnon-template-friend \
        -Woverloaded-virtual \
        -Wpmf-conversions \
        -Wsynth"
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
                [return snprintf("", 0, "");]
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
dnl the usual HAVE_EXPM1/HAVE_LOG1P
AC_CHECK_FUNC(strtold, AC_DEFINE(LMI_COMPILER_PROVIDES_STRTOLD, [1], [Define this if you have strtold() function]))
AC_CHECK_FUNC(expm1, AC_DEFINE(LMI_COMPILER_PROVIDES_EXPM1, [1], [Define this if you have expm1() function]))
AC_CHECK_FUNC(log1p, AC_DEFINE(LMI_COMPILER_PROVIDES_LOG1P, [1], [Define this if you have log1p() function]))
AC_CHECK_FUNC(rint, AC_DEFINE(LMI_COMPILER_PROVIDES_RINT, [1], [Define this if you have rint() function]))

dnl === Library checks ===

dnl use C++ compiler as we're checking for C++ libraries/headers
AC_LANG(C++)


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

dnl --- wxWidgets ---

AM_OPTIONS_WXCONFIG

dnl normally we should get all we need from wx-config
wx_config_params="--unicode=no --debug=$debug_option"
dnl disable use of static libraries with --static=no
if test "x$enable_static" == "xno"; then
wx_config_params="$wx_config_params --static=no"
fi
dnl disable use of shared libraries with --static=yes
if test "x$enable_shared" == "xno"; then
wx_config_params="$wx_config_params --static=yes"
fi
dnl otherwise use any library type we have for wx

AM_PATH_WXCONFIG([2.6.0], [wx="yes"],[],[],[$wx_config_params])

dnl if we didn't (may happen under Windows), try to find wx headers and
dnl libraries ourselves
if test "x$wx" != "xyes"; then

if test "x$lmi_wx_version" != "x"; then
    lmi_wx_version_guess="$lmi_wx_version"
else
    lmi_wx_version_guess="2.7 2.6 2.5"
fi

lmi_wx_basename="msw-ansi"
if test "$debug_option" == "yes"; then
    lmi_wx_basename="$lmi_wx_basename-debug"
fi
for lmi_wx_version in $lmi_wx_version_guess; do
    echo "checking for wxWidgets version $lmi_wx_version:"
    dnl the order of checks by priority
    dnl   1. check --with-wx
    dnl        use --with-wx-headers if specified
    dnl        use --with-wx-libs if specified'
    dnl        if not found => throw an error
    dnl   2. if still no libs found, then guess
    dnl        using $wxwin and some common install paths
    dnl   3. if not found report an error and description of what was done
    lmi_wx_root=$lmi_wx_dir

    if test "x$lmi_wx_root" == "x"; then
        lmi_wx_root="$WXWIN"
    fi

    dnl take values provided by user
    lmi_wx_headers_guess="$lmi_wx_headers_dir"
    lmi_wx_libs_guess="$lmi_wx_libs_dir"
    lmi_wx_setup_guess="$lmi_wx_setup_dir"

    dnl now look for the root directory hint
    if test "x$lmi_wx_root" != "x"; then
        dnl we are given a hint on the wxWidgets root directory
        if test "x$lmi_wx_headers_guess" == "x"; then
            lmi_wx_headers_guess="$lmi_wx_root/include"
        fi
        if test "x$lmi_wx_libs_guess" == "x"; then
            lmi_wx_libs_guess="$lmi_wx_root/lib"
        fi
    else
        dnl no wxWidgets root directory is given - search common install paths
        if test "x$lmi_wx_headers_guess" == "x"; then
            lmi_wx_headers_guess="/usr/local/include/wx-$lmi_wx_version"
        fi
        if test "x$lmi_wx_libs_guess" == "x"; then
            lmi_wx_libs_guess="/usr/lib /usr/local/lib"
        fi
    fi
    if test "x$lmi_wx_setup_guess" == "x"; then
        dnl try every lib directory
        for tmp_wx_libs in $lmi_wx_libs_guess
        do
            lmi_wx_setup_guess="$lmi_wx_setup_guess $tmp_wx_libs/wx/include/$lmi_wx_basename-$lmi_wx_version"
        done
        dnl and also try the headers diectories
        lmi_wx_setup_guess="$lmi_wx_setup_guess $lmi_wx_headers_guess"
    fi

    dnl now do try to find the files

    dnl wx/setup.h in lmi_wx_setup_guess
    for lmi_wx_setup in $lmi_wx_setup_guess
    do
        AC_CHECK_FILE([$lmi_wx_setup/wx/setup.h], [ok_wx_setup=$lmi_wx_setup])
        if test "x$ok_wx_setup" != "x"; then
            for lmi_wx_headers in $lmi_wx_headers_guess
            do
                AC_CHECK_FILE([$lmi_wx_headers/wx/chkconf.h], [ok_wx_headers=$lmi_wx_headers])
                if test "x$ok_wx_headers" != "x"; then
                    for lmi_wx_libs in $lmi_wx_libs_guess
                    do
                        echo "guessing libs in $lmi_wx_libs ..."
                        lmi_wx_libs_needed="base core adv xml xrc"
                        lmi_wx_lib_prefix=wx_msw
                        if test "$debug_option" == "yes"; then
                            lmi_wx_lib_prefix="$lmi_wx_lib_prefix""d"
                        fi
                        lmi_wx_lib_prefix="$lmi_wx_lib_prefix""_"
                        lmi_wx_lib_suffix="-$lmi_wx_version"
                        ok_wx_libs=$lmi_wx_libs
                        for lmi_wx_lib_needed in $lmi_wx_libs_needed
                        do
                            AC_CHECK_FILE("$lmi_wx_libs/$lmi_wx_lib_prefix$lmi_wx_lib_needed$lmi_wx_lib_suffix", [],
                            [ok_wx_libs=""])
                            if test "x$ok_wx_libs" == "x"; then
                                break
                            fi
                        done
                        if test "x$ok_wx_libs" != "x"; then
                            WX_CXXFLAGS="-I$ok_wx_setup -I$ok_wx_headers"
                            WX_LIBS="-L$ok_wx_libs -lwx_$lmi_wx_basename""_xrc-$lmi_wx_version"
                            echo "using the following wxWidgets configuration:"
                            echo "  - version         $lmi_wx_version"
                            echo "  - setup.h         $ok_wx_setup/wx/setup.h"
                            echo "  - headers         $ok_wx_headers"
                            echo "  - libraries       $ok_wx_libs"
                            break
                        fi
                    done
                fi
            done
        fi
    done
done dnl lmi_wx_version_guess

if test "x$WX_CXXFLAGS" == "x"; then
    AC_MSG_ERROR([No wxWidgets headers or setup.h were found!])
fi
if test "x$WX_LIBS" == "x"; then
    AC_MSG_ERROR([No wxWidgets libs were found!])
fi

WX_CPPFLAGS=$WX_CXXFLAGS
AC_SUBST(WX_CPPFLAGS)
AC_SUBST(WX_CXXFLAGS)
AC_SUBST(WX_LIBS)

fi dnl AM_PATH_WXCONFIG failed

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


if test "x$lmi_boost_toolkit" == "x"; then
    if test "x$USE_MINGW32" = "x1"; then
        lmi_boost_toolkit=mgw
    else
        lmi_boost_toolkit=gcc
    fi
fi

dnl Boost library name should be guessed in a more sophisticated manner
if test "x$USE_MINGW32" = "x1"; then
    boost_libfs="boost_filesystem-$lmi_boost_toolkit"
elif test "x$USE_CYGWIN" = "x1"; then
    boost_libfs="boost_filesystem-$lmi_boost_toolkit-mt-s"
else
    boost_libfs="boost_filesystem"
fi

save_LIBS=$LIBS
save_LDFLAGS=$LDFLAGS
if test "x$lmi_boost_libs" != "x"; then
    BOOST_LDFLAGS="-L$lmi_boost_libs "
    LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
    errmsg="even in $lmi_boost_libs"
else
    errmsg="use --with-boost-libs=dir
if it is installed in non default location"
fi

dnl usually when we want to check for a library we use AC_CHECK_LIB and main()
dnl but since under mingw with -pedantic and -std= options test does not pass
dnl we are forced to use a bit more sophisticated construct
LIBS="$LIBS -l$boost_libfs"
LMI_BOOSTFS_BUILD=0
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <boost/filesystem/operations.hpp>]],
    [[boost::filesystem::current_path()]])],
    [],
    [LMI_BOOSTFS_BUILD=1
    AC_MSG_WARN([Boost filesystem library $boost_libfs not found, $errmsg])]
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

AC_CHECK_FILE([$LMI_BOOSTFS_SRC/convenience.cpp], [], [no_boost_fs=1])
AC_CHECK_FILE([$LMI_BOOSTFS_SRC/exception.cpp], [], [no_boost_fs=1])
AC_CHECK_FILE([$LMI_BOOSTFS_SRC/operations_posix_windows.cpp], [], [no_boost_fs=1])
AC_CHECK_FILE([$LMI_BOOSTFS_SRC/path_posix_windows.cpp], [], [no_boost_fs=1])
if test "x$no_boost_fs" == "x1"; then
    AC_MSG_ERROR([no boost::filesystem source files found in ($LMI_BOOSTFS_SRC)])
fi

BOOST_LIBS=third_party/boostfs/libboost_filesystem.la
else
BOOST_LIBS="$BOOST_LDFLAGS -l$boost_libfs"
fi
dnl contains flags needed to link to externally or internally built boost::filesystem library
AC_SUBST(BOOST_LIBS)

dnl indicates weither do we need to build internally boost::filesystem library
AM_CONDITIONAL([LMI_BOOSTFS_BUILD], [test "x$LMI_BOOSTFS_BUILD" == "x1"])
dnl indicates the directory path where sources could be found
AC_SUBST(LMI_BOOSTFS_SRC)

dnl --- LIBXML (required) ---------------
AC_CHECK_PROG(lmi_xml2_config, xml2-config, yes, no)
if test "x$lmi_xml2_config" != "xyes"; then
    AC_MSG_ERROR([Unable to find libxml configuration program xml2-config in the path.])
fi
LIBXML_FLAGS=`xml2-config --cflags`
LIBXML_LIBS=`xml2-config --libtool-libs`

AC_SUBST(LIBXML_FLAGS)
AC_SUBST(LIBXML_LIBS)

dnl --- libxml++ (required) -------------------
PKG_CHECK_MODULES(LIBXMLPP, libxml++-2.6 >= 2.6.0)

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

dnl === Generate output files ===
AC_CONFIG_FILES([Makefile third_party/Makefile third_party/boostfs/Makefile])
AC_OUTPUT
