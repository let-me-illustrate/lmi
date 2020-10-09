#!/bin/sh

# Install libxml2, libxslt and xmlwrapp with options suitable for lmi.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

set -vxe

[ -n "$LMI_COMPILER" ] || { printf '%s\n' "no LMI_COMPILER" && exit 1; }
[ -n "$LMI_TRIPLET"  ] || { printf '%s\n' "no LMI_TRIPLET"  && exit 2; }

# Configurable settings ########################################################

coefficiency=${coefficiency:-"--jobs=$(nproc)"}

MAKE=${MAKE:-"make $coefficiency"}

# Note that we forcibly add local to the prefix, even if a non-default value
# is specified, because this is currently hard-coded in localxxxdir values.
prefix=${prefix:-/opt/lmi}/local
exec_prefix="${exec_prefix:-$prefix/${LMI_COMPILER}_${LMI_TRIPLET}}"

xml_skip_clean="${xml_skip_clean:-0}"

# Variables that normally should be left alone #################################

srcdir=$(dirname "$(readlink --canonicalize "$0")")

build_dir="${exec_prefix}/xml-ad_hoc"

build_type=$(/usr/share/libtool/build-aux/config.guess)

case "$build_type" in
    (*-*-cygwin*)
        export PATH="/opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw/bin:${PATH}"
        ;;
esac

export PKG_CONFIG_PATH="$exec_prefix/lib/pkgconfig"

# Don't prepend libraries and headers paths with the sysroot directory.
export PKG_CONFIG_SYSROOT_DIR=

# For 'host' and 'build' configure options, see:
#   https://cygwin.com/ml/cygwin/2002-01/msg00837.html
# Do not use '-fno-omit-frame-pointer' because of an ICE:
#   https://lists.nongnu.org/archive/html/lmi/2019-06/msg00018.html

# '--disable-dependency-tracking' is required with the MinGW toolchain
# in a Cygwin shell, to prevent a catastrophic dependency-tracking
# failure. Apparently the problem is colons in header paths, e.g.:
#   c:/MinGW-20050827/bin/../lib/gcc/mingw32/3.4.4/include/stddef.h:
# which elicit fatal errors such as this:
#   .deps/DOCBparser.Plo:1: *** multiple target patterns.  Stop.

# We can't have new lines in the CFLAGS, so get rid of them explicitly.
xmlsoft_common_cflags=$(echo '
  -fno-ms-extensions
  -frounding-math
  -Wno-cpp
  -Wno-discarded-qualifiers
  -Wno-format
  -Wno-format-extra-args
  -Wno-implicit-fallthrough
  -Wno-implicit-function-declaration
  -Wno-maybe-uninitialized
  -Wno-misleading-indentation
  -Wno-missing-format-attribute
  -Wno-missing-prototypes
  -Wno-nested-externs
  -Wno-pedantic
  -Wno-pointer-sign
  -Wno-sign-compare
  -Wno-strict-prototypes
  -Wno-suggest-attribute=format
  -Wno-unused-but-set-variable
  -Wno-unused-function
  -Wno-unused-parameter
  -Wno-unused-variable
' | tr '\n' ' ' | tr -s ' ' )

third_party_libraries_common_options="
  --prefix=$prefix
  --exec-prefix=$exec_prefix
  --build=$build_type
  --host=$LMI_TRIPLET
  --disable-dependency-tracking
  --disable-static
  --enable-shared
"

xmlsoft_common_options="
  $third_party_libraries_common_options
  --with-debug
  --without-python
"

# This variable is used inside eval, which is beyond spellcheck's ken.
# shellcheck disable=SC2034
libxml2_options="
  $xmlsoft_common_options
  --without-lzma
  --with-schemas
  --without-iconv
  --without-modules
  --without-schematron
  --without-zlib
"

# The '--with-libxml-prefix' option is documented thus:
#   "Specify location of libxml config"
# and libxml2 installs 'xml2-config' under its own $exec_prefix if
# that differs from its own $prefix. It would seem clearer if this
# libxslt option were named '--with-libxml-exec-prefix'.

# As above.
# shellcheck disable=SC2034
libxslt_options="
  $xmlsoft_common_options
  --with-libxml-prefix=$exec_prefix
  --without-crypto
"

xmlwrapp_options="
  $third_party_libraries_common_options
  --disable-docs
  --disable-examples
  --disable-tests
"

# Actually build ##############################################################

# Nonchalantly remove pkgconfig and cmake subdirectories, even though
# other libraries might someday write files in them, because lmi never
# uses them outside of this makefile.
#
# It would probably be much better to use an 'uninstall' target in
# each package's makefile.
if [ "$xml_skip_clean" != 1 ]; then
    rm --force --recursive "$exec_prefix"/bin/*xml2*
    rm --force --recursive "$exec_prefix"/bin/*xmlwrapp*
    rm --force --recursive "$exec_prefix"/bin/*xslt*
    rm --force --recursive "$exec_prefix"/bin/xmllint*
    rm --force --recursive "$exec_prefix"/bin/xmlcatalog*
    rm --force --recursive "$exec_prefix"/include/libxml2
    rm --force --recursive "$exec_prefix"/include/libxmlwrapp
    rm --force --recursive "$exec_prefix"/include/libxslt
    rm --force --recursive "$exec_prefix"/include/libxsltwrapp
    rm --force --recursive "$exec_prefix"/include/libexslt
    rm --force --recursive "$exec_prefix"/lib/*xml2*
    rm --force --recursive "$exec_prefix"/lib/*xmlwrapp*
    rm --force --recursive "$exec_prefix"/lib/*xslt*
    rm --force --recursive "$exec_prefix"/lib/cmake
    rm --force --recursive "$exec_prefix"/lib/pkgconfig
    rm --force --recursive "$build_dir"
fi

for lib in libxml2 libxslt; do
    libdir="$srcdir/third_party/$lib"
    if [ ! -x "$libdir/configure" ]; then
        cd "$libdir"
        NOCONFIGURE=1 ./autogen.sh
    fi
    mkdir --parents "$build_dir/$lib"
    cd "$build_dir/$lib"
    # 'configure' options must not be double-quoted
    # shellcheck disable=SC2046
    "$libdir/configure" \
        LDFLAGS='-lws2_32' \
        CPPFLAGS='-w' \
        CFLAGS="-g -O2 $xmlsoft_common_cflags" \
        $(eval "echo \$${lib}_options")
    $MAKE install
done

# Building xmlwrapp is similar, but sufficiently different to not try to fit it
# into the loop above, but reuse the same structure for it just to emphasize
# the similarity.
for lib in xmlwrapp; do
    libdir="$srcdir/third_party/$lib"
    if [ ! -x "$libdir/configure" ]; then
        cd "$libdir"
        ./bootstrap
    fi
    mkdir --parents "$build_dir/$lib"
    cd "$build_dir/$lib"
    # shellcheck disable=SC2046
    "$libdir/configure" \
        PKG_CONFIG_LIBDIR=$exec_prefix/lib/pkgconfig \
        $xmlwrapp_options
    $MAKE install
done

# autotools: 'make install' doesn't respect group permissions--see:
#   https://lists.gnu.org/archive/html/automake/2019-01/msg00000.html
# After the 'chmod' calls, the 'find' command should find nothing.
#
# Someday it may be necessary to add a line like this to the recipe:
#   export lt_cv_to_tool_file_cmd=func_convert_file_cygwin_to_w32
# but that's not necessary for now. See:
#   https://lists.nongnu.org/archive/html/lmi/2011-06/msg00025.html

chmod -R g=u "$build_dir"
chmod -R g=u "$prefix"/include/libexslt
chmod -R g=u "$prefix"/include/libxml2
chmod -R g=u "$prefix"/include/libxslt
chmod -R g=u "$prefix"/include/xmlwrapp
chmod -R g=u "$prefix"/include/xsltwrapp
chmod -R g=u "$prefix"/share/doc/libxml2-*
chmod -R g=u "$prefix"/share/doc/libxslt-*
chmod -R g=u "$prefix"/share/gtk-doc/html/libxml2
chmod -R g=u "$exec_prefix"/bin
chmod -R g=u "$exec_prefix"/lib
chmod -R g=u "$prefix"/share/aclocal
chmod -R g=u "$prefix"/share/man
find "$prefix" -perm -200 -not -perm -020

exit 0
