#!/bin/sh

# Write a bland 'configurable_settings.xml'.

# Copyright (C) 2022 Gregory W. Chicares.
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

# Write the XML file to the directory optionally specified as a
# command-line argument, else to the current directory.

if [ "$#" -gt 1 ]; then
  printf '%s arguments given, but zero or one expected.\n' "$#"
  exit 2
fi

directory=.

if [ "$#" -eq 1 ]; then
  directory="$(readlink --canonicalize-existing --no-newline "$1")" ||
  { printf 'No such directory "%s"--exiting.\n' "$1"; exit 3; }
fi

# {root_name} is like std::filesystem::root_name().
# Override it on the command line if it should not be null.

cat >"$directory"/configurable_settings.xml <<EOF
<?xml version="1.0"?>
<configurable_settings version="2">
  <calculation_summary_columns/>
  <census_paste_palimpsestically>1</census_paste_palimpsestically>
  <cgi_bin_log_filename>cgi_bin.log</cgi_bin_log_filename>
  <custom_input_0_filename>custom.ini</custom_input_0_filename>
  <custom_input_1_filename>custom.inix</custom_input_1_filename>
  <custom_output_0_filename>custom.out0</custom_output_0_filename>
  <custom_output_1_filename>custom.out1</custom_output_1_filename>
  <default_input_filename>${root_name:-}/etc/opt/lmi/default.ill</default_input_filename>
  <libraries_to_preload/>
  <offer_hobsons_choice>0</offer_hobsons_choice>
  <print_directory>${root_name:-}/opt/lmi/print</print_directory>
  <seconds_to_pause_between_printouts>10</seconds_to_pause_between_printouts>
  <skin_filename>skin.xrc</skin_filename>
  <spreadsheet_file_extension>.tsv</spreadsheet_file_extension>
  <use_builtin_calculation_summary>1</use_builtin_calculation_summary>
</configurable_settings>
EOF
