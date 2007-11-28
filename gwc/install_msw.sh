#!/bin/sh

# For msw, download and build lmi and required libraries.

# Copyright (C) 2007 Gregory W. Chicares.
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
# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: install_msw.sh,v 1.2 2007-11-28 16:55:28 chicares Exp $

mkdir --parents /cygdrive/c/opt/lmi/src/lmi
mount "C:/opt/lmi" "/opt/lmi"

cd /opt/lmi/src

export CVS_RSH="ssh"
export CVSROOT=":pserver:anonymous@cvs.savannah.nongnu.org:/sources/lmi"
cvs -z3 checkout lmi

cd /opt/lmi/src/lmi

rm --force --recursive scratch
rm --force --recursive /cygdrive/c/MinGW-20050827
make prefix=/cygdrive/c/MinGW-20050827 -f install_mingw.make

make -f install_miscellanea.make clobber
make -f install_miscellanea.make

make -f install_libxml2_libxslt.make

make -f install_wx.make

make wx_dir=/opt/lmi/wx-scratch/wxWidgets-2.8.6/gcc344/ wx_build_dir=/opt/lmi/local/bin wx_config_check
make wx_dir=/opt/lmi/wx-scratch/wxWidgets-2.8.6/gcc344/ wx_build_dir=/opt/lmi/local/bin install

echo Done.

