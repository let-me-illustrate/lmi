# Parse 'nychthemeral_test.sh' output for errors.
#
# Copyright (C) 2018, 2019 Gregory W. Chicares.
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
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

/^# install; check physical closure/d
/^# cgi and cli tests/d
/^Test common gateway interface:/d
/^Test command line interface:/d
/^Test sample.cns:/d
/^Test sample.ill:/d
/^# system test/d
/^System test:/d
/^All [1-9][0-9]* files match./d
/^# unit tests/d
/^[1-9][0-9]* tests succeeded/d
/^# build with shared-object attributes/d
/^# cgi and cli tests in libstdc++ debug mode/d
/^Test common gateway interface:/d
/^Test command line interface:/d
/^Test sample.cns:/d
/^Test sample.ill:/d
/^# unit tests in libstdc++ debug mode/d
/^[1-9][0-9]* tests succeeded/d
/^# test concinnity/d
/^  Problems detected by xmllint:/d
/^  Miscellaneous problems:/d
/^  *[1-9][0-9]* source files/d
/^  *[1-9][0-9]* source lines/d
/^  *[1-9][0-9]* marked defects/d
/^# xrc tests/d
/^# schema tests/d
/^# test all valid emission types/d
/^$/d
