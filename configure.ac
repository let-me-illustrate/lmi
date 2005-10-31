dnl configure.ac script for lmi, process with autoconf to create configure
dnl
dnl Copyright (C) 2005 Vadim Zeitlin.
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
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
dnl
dnl http://savannah.nongnu.org/projects/lmi
dnl email: <chicares@cox.net>
dnl snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

dnl === Prologue ===

AC_REVISION($Id: configure.ac,v 1.2 2005-10-31 14:03:17 zeitlin Exp $)dnl

AC_PREREQ(2.59)
AC_INIT(LMI, 1.0.0, [chicares@cox.net])
AM_INIT_AUTOMAKE()

AC_CONFIG_SRCDIR([lmi.xpm])
AC_CONFIG_HEADER([config.h])

dnl remember, "build" is where we compile, "host" is where the resulting
dnl program runs (which may be different from "build" for cross-compilation)
AC_CANONICAL_HOST

case "${host}" in
    *-*-linux* )
        USE_LINUX=1;
        ;;

    *-*-cygwin* )
        USE_CYGWIN=1;
        USE_WINDOWS=1;
        ;;

    *-*-mingw32* )
        USE_MINGW32=1;
        USE_WINDOWS=1;
        ;;
esac

AM_CONDITIONAL(LMI_WINDOWS, [test "x$USE_WINDOWS" = "x1"])


dnl === Command line options ===

AC_ARG_WITH(boost-headers,
            [AC_HELP_STRING([--with-boost-headers=dir], [Boost headers directory])],
            lmi_boost_headers=$withval)
AC_ARG_WITH(boost-libs,
            [AC_HELP_STRING([--with-boost-libs=dir], [Boost libs installation directory])],
            lmi_boost_libs=$withval)

 
AC_ARG_WITH(wx,
            [AC_HELP_STRING([--with-wx=dir], [wxWidgets directory])],
            lmi_wx_dir=$withval)
AC_ARG_WITH(wx-version,
            [AC_HELP_STRING([--with-wx-version=version], [wxWidgets version to search for - 2.5, 2.6, 2.7 are guessed by default])],
            lmi_wx_version=$withval)
AC_ARG_WITH(wx-headers,
            [AC_HELP_STRING([--with-wx-headers=dir], [wxWidgets header files directory])],
            lmi_wx_headers_dir=$withval)
AC_ARG_WITH(wx-libs,
            [AC_HELP_STRING([--with-wx-libs=dir], [wxWidgets libraries directory])],
            lmi_wx_libs_dir=$withval)
AC_ARG_WITH(wx-setup,
            [AC_HELP_STRING([--with-wx-setup=dir], [wxWidgets setup.h directory (contains wx/setup.h file)])],
            lmi_wx_setup_dir=$withval)


AC_ARG_ENABLE(debug,
             [AC_HELP_STRING([--enable-debug],
                             [compile with debug information])],
             [case "x${enableval}" in
                   x) debug=yes ;;
                xyes) debug=yes ;;
                 xno) debug=no ;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
             esac], [debug=no])
AM_CONDITIONAL(LMI_DEBUG, [test "x$debug" = "xyes"])

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

AC_ARG_ENABLE(tests,
             [AC_HELP_STRING([--enable-tests],
                             [Build tests.])],
             [case "x${enableval}" in
                   x) lmi_tests_option=yes ;;
                xyes) lmi_tests_option=yes ;;
                 xno) lmi_tests_option=no;;
                   *) AC_MSG_ERROR(bad value ${enableval} for --enable-tests=[no*|yes], default is no) ;;
             esac],
             lmi_tests_option=no)
AM_CONDITIONAL(LMI_WITH_TESTS, [test "x$lmi_tests_option" == "xyes"])


dnl === Program checks ===
AC_PROG_CC
AC_PROG_CXX

AM_CONDITIONAL(LMI_GCC, [test "x$GCC" == "xyes"])
AM_CONDITIONAL(LMI_GXX, [test "x$GXX" == "xyes"])

AC_DISABLE_STATIC   dnl don't waste time on static build by default
AC_PROG_LIBTOOL

AC_PROG_LD

dnl === Checks for standard headers/functions ===

AC_CHECK_HEADERS(curses.h)
if test "$ac_cv_header_curses_h" != "yes"; then
    AC_CHECK_HEADERS(conio.h)
    if test "$ac_cv_header_conio_h" != "yes"; then
        AC_MSG_ERROR([Header for getch() not found: neither curses.h nor conio.h are available.])
    fi
fi

AC_CHECK_FUNCS(log10l fabsl strtold)

dnl existing code already uses these macros so continue to use them instead of
dnl the usual HAVE_EXPM1/HAVE_LOG1P
AC_CHECK_FUNC(expm1, AC_DEFINE(LMI_COMPILER_HAS_EXPM1, [1], [Define this if you have expm1() function]))
AC_CHECK_FUNC(log1p, AC_DEFINE(LMI_COMPILER_HAS_LOG1P, [1], [Define this if you have log1p() function]))



dnl === Library checks ===

dnl use C++ compiler as we're checking for C++ libraries/headers
AC_LANG(C++)


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


dnl --- wxWidgets ---

AM_OPTIONS_WXCONFIG

dnl normally we should get all we need from wx-config
AM_PATH_WXCONFIG([2.6.0], [wx="yes"])

dnl if we didn't (may happen under Windows), try to find wx headers and
dnl libraries ourselves
if test "x$wx" != "xyes"; then

if test "x$lmi_wx_version" != "x"; then
    lmi_wx_version_guess="$lmi_wx_version"
else
    lmi_wx_version_guess="2.7 2.6 2.5"
fi

lmi_wx_basename="msw-ansi"
if test "x$debug" == "xyes"; then
    lmi_wx_basename="$lmi_wx_basename-debug"
fi
for m_wx_version in $lmi_wx_version_guess
do
    echo "checking for wxWidgets version $m_wx_version :"
    dnl the order of checks by priority
    dnl   1. check --with-wx
    dnl        use --with-wx-headers if specified
    dnl        use --with-wx-libs if specified'
    dnl        if not found => throw an error
    dnl   2. check wx-config
    dnl        if found, use it
    dnl   3. if still no libs found, then guess
    dnl        using $wxwin and some common install paths
    dnl   4. if not found report an error and description of what was done
    dnl
    dnl 1 and 3 are very similar, so what we really do:
    dnl  First we check 2, and then 1 and 3

    if test "x$lmi_wx_config" == "xyes"; then
        dnl FIXME: we use wx-config only if no specific details were given
        if test "x$lmi_wx_dir" == "x"; then
            if test "x$lmi_wx_headers_dir" == "x"; then
                if test "x$lmi_wx_libs_dir" == "x"; then
                    if test "x$lmi_wx_setup_dir" == "x"; then

                        WX_CONFIG="[wx-config --unicode=no --static=no --debug=$debug --version=$m_wx_version]"

                        `$WX_CONFIG 2>/dev/null`

                        if test "x$?" == "x0"; then
                            WX_FLAGS=`$WX_CONFIG --cxxflags`
                            WX_LIBS=`$WX_CONFIG --libs`
                            echo "checking for $WX_CONFIG ... yes"
                            break; dnl from the lmi_wx_version_guess loop
                        else
                            echo "checking for $WX_CONFIG ... no"
                        fi
                    fi
                fi
            fi
        fi
    fi

    m_wx_root=$lmi_wx_dir

    if test "x$m_wx_root" == "x"; then
        m_wx_root="$wxwin"
    fi

    dnl take values provided by user
    m_wx_headers_guess="$lmi_wx_headers_dir"
    m_wx_libs_guess="$lmi_wx_libs_dir"
    m_wx_setup_guess="$lmi_wx_setup_dir"

    dnl now look for the root directory hint
    if test "x$m_wx_root" != "x"; then
        dnl we are given a hint on the wxWidgets root directory
        if test "x$m_wx_headers_guess" == "x"; then
            m_wx_headers_guess="$m_wx_root/include"
        fi
        if test "x$m_wx_libs_guess" == "x"; then
            m_wx_libs_guess="$m_wx_root/lib"
        fi
    else
        dnl no wxWidgets root directory is given - search common install paths
        if test "x$m_wx_headers_guess" == "x"; then
            m_wx_headers_guess="/usr/local/include/wx-$m_wx_version"
        fi
        if test "x$m_wx_libs_guess" == "x"; then
            m_wx_libs_guess="/usr/lib /usr/local/lib"
        fi
    fi
    if test "x$m_wx_setup_guess" == "x"; then
        dnl try every lib directory
        for tmp_wx_libs in $m_wx_libs_guess
        do
            m_wx_setup_guess="$m_wx_setup_guess $tmp_wx_libs/wx/include/$lmi_wx_basename-$m_wx_version"
        done
        dnl and also try the headers diectories
        m_wx_setup_guess="$m_wx_setup_guess $m_wx_headers_guess"
    fi

    dnl now do try to find the files

    dnl wx/setup.h in m_wx_setup_guess
    for m_wx_setup in $m_wx_setup_guess
    do
        AC_CHECK_FILE([$m_wx_setup/wx/setup.h], [ok_wx_setup=$m_wx_setup])
        if test "x$ok_wx_setup" != "x"; then
            for m_wx_headers in $m_wx_headers_guess
            do
                AC_CHECK_FILE([$m_wx_headers/wx/chkconf.h], [ok_wx_headers=$m_wx_headers])
                if test "x$ok_wx_headers" != "x"; then
                    for m_wx_libs in $m_wx_libs_guess
                    do
                        echo "guessing libs in $m_wx_libs ..."
                        lmi_wx_libs_needed="base core adv xml xrc"
                        lmi_wx_lib_prefix=wx_msw
                        if test "x$debug" == "xyes"; then
                            lmi_wx_lib_prefix="$lmi_wx_lib_prefix""d"
                        fi
                        lmi_wx_lib_prefix="$lmi_wx_lib_prefix""_"
                        lmi_wx_lib_suffix="-$m_wx_version"
                        ok_wx_libs=$m_wx_libs
                        for m_wx_lib_needed in $lmi_wx_libs_needed
                        do
                            AC_CHECK_FILE("$m_wx_libs/$lmi_wx_lib_prefix$m_wx_lib_needed$lmi_wx_lib_suffix", [],
                            [ok_wx_libs=""])
                            if test "x$ok_wx_libs" == "x"; then
                                break;
                            fi
                        done
                        if test "x$ok_wx_libs" != "x"; then
                            WX_FLAGS="-I$ok_wx_setup -I$ok_wx_headers"
                            WX_LIBS="-L$ok_wx_libs -lwx_$lmi_wx_basename""_xrc-$m_wx_version"
                            echo "using the following wxWidgets configuration:"
                            echo "  - version         $m_wx_version"
                            echo "  - setup.h         $ok_wx_setup/wx/setup.h"
                            echo "  - headers         $ok_wx_headers"
                            echo "  - libraries       $ok_wx_libs"
                            break;
                        fi
                    done
                fi
            done
        fi
    done
done dnl lmi_wx_version_guess

if test "x$WX_FLAGS" == "x"; then
    AC_MSG_ERROR([No wxWidgets headers or setup.h were found!..])
fi
if test "x$WX_LIBS" == "x"; then
    AC_MSG_ERROR([No wxWidgets libs were found!..])
fi
echo "------------wxWidgets-config--------"
echo "using the following flags:"
echo "  CPPFLAGS    $WX_FLAGS"
echo "  LDDFLAGS    $WX_LIBS"
echo "--------end-wxWidgets-config-------"

AC_SUBST(WX_FLAGS)
AC_SUBST(WX_LIBS)

fi dnl AM_PATH_WXCONFIG failed


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


if test "x$USE_MINGW32" = "x1"; then
    boost_libfs=boost_filesystem-mgw
else
    boost_libfs=boost_filesystem
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
AC_CHECK_LIB([$boost_libfs], main,
    [],
    [AC_MSG_ERROR([Boost filesystem library $boost_libfs not found, $errmsg])]
)

LDFLAGS=$save_LDFLAGS
LIBS=$save_LIBS
BOOST_LIBS="$BOOST_LDFLAGS-l$boost_libfs"
AC_SUBST(BOOST_LIBS)


dnl --- LIBXML (required) ---------------
AC_CHECK_PROG(lmi_xml2_config, xml2-config, yes, no)
if test "x$lmi_xml2_config" != "xyes"; then
    AC_MSG_ERROR([Unable to find libxml configuration program xml2-config in the path.])
fi
LIBXML_FLAGS=`xml2-config --cflags`
LIBXML_LIBS=`xml2-config --libtool-libs`

AC_SUBST(LIBXML_FLAGS)
AC_SUBST(LIBXML_LIBS)


dnl --- XMLWRAPP (required) -------------------
AC_CHECK_PROG(lmi_xmlwrapp_config, xmlwrapp-config, yes, no)
if test "x$lmi_xmlwrapp_config" != "xyes"; then
    AC_MSG_ERROR([Unable to find libxmlwrapp configuration program xmlwrapp-config in the path.])
fi
dnl TODO: Pass --debug option and so on (depends on the actual build)

XMLWRAPP_FLAGS=`xmlwrapp-config --cxxflags`
XMLWRAPP_LIBS=`xmlwrapp-config --libs`

AC_SUBST(XMLWRAPP_FLAGS)
AC_SUBST(XMLWRAPP_LIBS)

AC_CACHE_CHECK([if we are using recent version of xmlwrapp],
    lmi_cv_lib_xmlwrapp_recent,
    [
        save_CXXFLAGS=$CXXFLAGS
        CXXFLAGS="$CXXFLAGS $XMLWRAPP_FLAGS"
        AC_TRY_COMPILE([#include <xmlwrapp/document.h>
            #include <xmlwrapp/tree_parser.h>
            #include <xmlwrapp/node.h>],
            [
                xml::tree_parser parser("");
                parser.get_document().get_root_node();
            ],
            lmi_cv_lib_xmlwrapp_recent=yes,
            lmi_cv_lib_xmlwrapp_recent=no
        )
    ]
)
if test "x$lmi_cv_lib_xmlwrapp_recent" = "xyes"; then
    AC_DEFINE([USING_CURRENT_XMLWRAPP], [1], [Define this if you have the latest version of libxmlwrapp])
fi

dnl === Generate output files ===
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
