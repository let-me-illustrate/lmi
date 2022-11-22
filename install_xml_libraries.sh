#!/bin/sh

# Install libxml2, libxslt and xmlwrapp with options suitable for lmi.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

build_type=$(/usr/share/misc/config.guess)

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
#   .deps/DOCBparser.Plo:1: [*]* multiple target patterns.  Stop.

# We can't have new lines in the CFLAGS, so get rid of them explicitly.
xmlsoft_common_cflags=$(echo '
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

xmlsoft_common_ldflags=""
case "$LMI_TRIPLET" in
    *-*-cygwin*|*-*-mingw*)
        xmlsoft_common_ldflags="$xmlsoft_common_ldflags -lws2_32"
        ;;
esac

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

# This variable is used inside eval, which is beyond shellcheck's ken.
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

# Optionally, clean beforehand ################################################

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

# Create an XML catalog #######################################################

# This forestalls about a thousand lines of bogus error messages.

throwaway_catalog=/tmp/xml_catalog

cat >"$throwaway_catalog" <<EOF
<?xml version="1.0"?>
<!DOCTYPE catalog PUBLIC "-//OASIS//DTD Entity Resolution XML Catalog V1.0//EN" "http://www.oasis-open.org/committees/entity/release/1.0/catalog.dtd">
<catalog xmlns="urn:oasis:names:tc:entity:xmlns:xml:catalog">
  <rewriteURI uriStartString="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
   rewritePrefix="file://$PWD/third_party/libxml2/test/valid/dtds/xhtml1-transitional.dtd"/>
  <rewriteURI uriStartString="http://www.w3.org/TR/xhtml1/DTD/xhtml-lat1.ent"
   rewritePrefix="file://$PWD/third_party/libxml2/test/valid/dtds/xhtml-lat1.ent"/>
  <rewriteURI uriStartString="http://www.w3.org/TR/xhtml1/DTD/xhtml-special.ent"
   rewritePrefix="file://$PWD/third_party/libxml2/test/valid/dtds/xhtml-special.ent"/>
  <rewriteURI uriStartString="http://www.w3.org/TR/xhtml1/DTD/xhtml-symbol.ent"
   rewritePrefix="file://$PWD/third_party/libxml2/test/valid/dtds/xhtml-symbol.ent"/>
</catalog>
EOF

export XML_CATALOG_FILES="$throwaway_catalog"

# Actually build ##############################################################

for lib in libxml2 libxslt; do
    printf 'Building %s with %s for %s.\n' "$lib" "$LMI_COMPILER" "$LMI_TRIPLET"
    libdir="$srcdir/third_party/$lib"
    cd "$libdir"
    if [ ! -x "configure" ]; then
        NOCONFIGURE=1 ./autogen.sh
    else
        # Just refresh the existing build files if necessary: we don't use
        # autogen.sh in this case because it forces recreating them, which is
        # usually unnecessary, but we do want to do it if any of configure
        # dependencies (e.g. configure.ac) changes.
        autoreconf --install
    fi
    mkdir --parents "$build_dir/$lib"
    cd "$build_dir/$lib"

    case "$LMI_COMPILER" in
        (gcc)
            valid_math="-frounding-math -fsignaling-nans"
            # 'configure' options must not be double-quoted
            # shellcheck disable=SC2046
            "$libdir/configure" \
                CPPFLAGS='-w' \
                CFLAGS="-g -O2 $xmlsoft_common_cflags -fno-ms-extensions $valid_math" \
                LDFLAGS="$xmlsoft_common_ldflags" \
                $(eval "echo \$${lib}_options") || err=$?
            ;;
        (clang)
            valid_math="-Woverriding-t-option -ffp-model=strict -ffp-exception-behavior=ignore -Wno-overriding-t-option"
            # 'configure' options must not be double-quoted
            # shellcheck disable=SC2046
            "$libdir/configure" \
                CC=clang \
                CXX=clang++ \
                CPPFLAGS='-w' \
                CFLAGS="-g -O2 $xmlsoft_common_cflags -fno-ms-extensions $valid_math" \
                LDFLAGS="$xmlsoft_common_ldflags -fuse-ld=lld" \
                $(eval "echo \$${lib}_options") || err=$?
                ;;
            (*)
                printf '%s\n' "Unknown toolchain '$LMI_COMPILER'."
                return 2;
            ;;
    esac

    if [ -n "$err" ]; then
        error_marker='*''*''*'
        # Use ${error_marker} instead of a literal triple asterisk,
        # because this script uses 'set -vx' and routinely echoing
        # a line that contains the error marker's value makes it
        # harder to find actual errors that use that marker.
        echo "${error_marker} Configuring failed; 'config.log' contains:"
        echo "-------------------------------------------------------"
        cat config.log
        echo "-------------------------------------------------------"
        exit $err
    fi
    $MAKE install
    printf 'Built %s with %s for %s.\n' "$lib" "$LMI_COMPILER" "$LMI_TRIPLET"
done

# Building xmlwrapp is similar, but sufficiently different to not try to fit it
# into the loop above, but reuse the same structure for it just to emphasize
# the similarity.
# shellcheck disable=SC2043
for lib in xmlwrapp; do
    printf 'Building %s with %s for %s.\n' "$lib" "$LMI_COMPILER" "$LMI_TRIPLET"
    libdir="$srcdir/third_party/$lib"
    # As this library doesn't have any special autogen.sh script, we can just
    # always run autoreconf unconditionally (without "--force").
    cd "$libdir"
    autoreconf --install
    mkdir --parents "$build_dir/$lib"
    cd "$build_dir/$lib"

    case "$LMI_COMPILER" in
        (gcc)
            valid_math="-frounding-math -fsignaling-nans"
            # 'configure' options must not be double-quoted
            # shellcheck disable=SC2086
            "$libdir/configure" \
                PKG_CONFIG_LIBDIR="$exec_prefix"/lib/pkgconfig \
                  CFLAGS="-g -O2 -fno-ms-extensions $valid_math" \
                CXXFLAGS="-g -O2 -fno-ms-extensions $valid_math" \
                $xmlwrapp_options || err=$?
            ;;
        (clang)
            valid_math="-Woverriding-t-option -ffp-model=strict -ffp-exception-behavior=ignore -Wno-overriding-t-option"
            # 'configure' options must not be double-quoted
            # shellcheck disable=SC2086
            "$libdir/configure" \
                PKG_CONFIG_LIBDIR="$exec_prefix"/lib/pkgconfig \
                      CC=clang \
                     CXX=clang++ \
                  CFLAGS="-g -O2 -fno-ms-extensions $valid_math" \
                CXXFLAGS="-g -O2 -fno-ms-extensions $valid_math -stdlib=libc++" \
                LDFLAGS="-fuse-ld=lld -stdlib=libc++" \
                $xmlwrapp_options || err=$?
            ;;
        (*)
            printf '%s\n' "Unknown toolchain '$LMI_COMPILER'."
            return 2;
            ;;
    esac

    if [ -n "$err" ]; then
        error_marker='*''*''*'
        echo "${error_marker} Configuring failed; 'config.log' contains:"
        echo "-------------------------------------------------------"
        cat config.log
        echo "-------------------------------------------------------"
        exit $err
    fi
    $MAKE install
    printf 'Built %s with %s for %s.\n' "$lib" "$LMI_COMPILER" "$LMI_TRIPLET"
done

# Expunge the throwaway XML catalog.
rm --force "$throwaway_catalog"

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
chmod -R g=u "$prefix"/share/doc/libxml2
chmod -R g=u "$prefix"/share/doc/libxslt-*
chmod -R g=u "$prefix"/share/gtk-doc/html/libxml2
chmod -R g=u "$exec_prefix"/bin
chmod -R g=u "$exec_prefix"/lib
chmod -R g=u "$prefix"/share/aclocal
chmod -R g=u "$prefix"/share/man
find "$prefix" -perm -200 -not -perm -020

exit 0
