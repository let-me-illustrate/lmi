# Makefile: object lists.
#
# Copyright (C) 2005 Gregory W. Chicares.
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: objects.make,v 1.24 2005-05-18 21:14:33 chicares Exp $

################################################################################

# External libraries whose source is compiled here. Their source
# is reached through 'vpath' directives. See the rationale in
# 'workhorse.make'.

# Boost filesystem library. The other boost libraries that lmi
# requires are implemented entirely in headers.

boost_filesystem_objects := \
  convenience.o \
  exception.o \
  path_posix_windows.o \
  operations_posix_windows.o \

# GNU cgicc.

# TODO ?? Track down the problems in cgicc-3.2.3 that make it
# difficult to use on the msw platform. Until then use cgicc-3.1.4,
# which has fewer such problems and a known workaround.

cgicc_objects = $(cgicc_3_1_4_objects)

cgicc_3_2_3_objects = \
  CgiEnvironment.o \
  CgiInput.o \
  CgiUtils.o \
  Cgicc.o \
  FormEntry.o \
  FormFile.o \
  HTMLAttribute.o \
  HTMLAttributeList.o \
  HTMLDoctype.o \
  HTMLElement.o \
  HTMLElementList.o \
  HTTPContentHeader.o \
  HTTPCookie.o \
  HTTPHTMLHeader.o \
  HTTPHeader.o \
  HTTPPlainHeader.o \
  HTTPRedirectHeader.o \
  HTTPResponseHeader.o \
  HTTPStatusHeader.o \
  MStreamable.o \

cgicc_3_1_4_objects = \
  CgiEnvironment.o \
  CgiUtils.o \
  Cgicc.o \
  FormEntry.o \
  FormFile.o \
  HTMLAttributes.o \
  HTMLElements.o \
  HTTPHeaders.o \
  MStreamable.o \

# xmlwrapp library from pmade.org .

xmlwrapp_objects := \
  event_parser.o \
  init.o \
  node.o \
  node_iterator.o \
  tree_parser.o \
  xmlwrapp_ex.o \

################################################################################

# Illustrations: the antediluvian branch. This code is fairly simple
# and clean, but less powerful than the production branch. For
# historical reasons, these 'branches' aren't maintained in separate
# cvs branches; instead, all their source code resides in the same
# place, and conflicts are prevented by prepending 'ihs_' to the
# names of many files in the production branch. Gradually the
# differences are being eliminated.

antediluvian_common_objects := \
  $(boost_filesystem_objects) \
  $(xmlwrapp_objects) \
  accountvalue.o \
  actuarial_table.o \
  alert.o \
  antediluvian_stubs.o \
  avdebug.o \
  basicvalues.o \
  calendar_date.o \
  configurable_settings.o \
  crc32.o \
  database.o \
  data_directory.o \
  dbdict.o \
  dbnames.o \
  dbvalue.o \
  death_benefits.o \
  expm1.o \
  fenv_lmi.o \
  file_command.o \
  getopt.o \
  global_settings.o \
  group_values.o \
  input_sequence.o \
  input_seq_helpers.o \
  inputillus.o \
  inputillus_sequences.o \
  inputillus_term_rider.o \
  inputillus_xml_io.o \
  inputs.o \
  inputstatus.o \
  interest_rates.o \
  kludges.o \
  ledger.o \
  ledger_base.o \
  ledger_invariant.o \
  ledger_text_formats.o \
  ledger_variant.o \
  ledger_xml_io.o \
  ledger_xsl.o \
  ledgervalues.o \
  license.o \
  loads.o \
  miscellany.o \
  mortality_rates.o \
  multiple_cell_document.o \
  outlay.o \
  path_utility.o \
  progress_meter.o \
  rounding_rules.o \
  sigfpe.o \
  single_cell_document.o \
  solve.o \
  surrchg_rates.o \
  system_command.o \
  timer.o \
  xenumtypes.o \
  xrange.o \

antediluvian_cli_objects := \
  alert_cli.o \
  file_command_cli.o \
  main_cli.o \
  main_common.o \
  progress_meter_cli.o \

antediluvian_cgi_objects := \
  $(cgicc_objects) \
  alert_cgi.o \
  file_command_cgi.o \
  main_cgi.o \
  main_common.o \
  progress_meter_cgi.o \

################################################################################

# Illustrations: the production branch.

lmi_common_objects := \
  $(boost_filesystem_objects) \
  $(xmlwrapp_objects) \
  actuarial_table.o \
  alert.o \
  calendar_date.o \
  configurable_settings.o \
  crc32.o \
  data_directory.o \
  dbnames.o \
  death_benefits.o \
  expm1.o \
  fenv_lmi.o \
  file_command.o \
  global_settings.o \
  group_values.o \
  ihs_acctval.o \
  ihs_avdebug.o \
  ihs_avmly.o \
  ihs_avsolve.o \
  ihs_avstrtgy.o \
  ihs_basicval.o \
  ihs_commfns.o \
  ihs_database.o \
  ihs_dbdict.o \
  ihs_dbvalue.o \
  ihs_fpios.o \
  ihs_funddata.o \
  ihs_inputillus_term_rider.o \
  ihs_irc7702.o \
  ihs_irc7702a.o \
  ihs_loads.o \
  ihs_mortal.o \
  ihs_pios.o \
  ihs_proddata.o \
  ihs_rnddata.o \
  input_sequence.o \
  input_seq_helpers.o \
  inputillus.o \
  inputillus_sequences.o \
  inputillus_xml_io.o \
  inputs.o \
  inputstatus.o \
  interest_rates.o \
  kludges.o \
  ledger.o \
  ledger_base.o \
  ledger_invariant.o \
  ledger_text_formats.o \
  ledger_variant.o \
  ledger_xml_io.o \
  ledger_xsl.o \
  ledgervalues.o \
  license.o \
  mathmisc.o \
  md5.o \
  miscellany.o \
  multiple_cell_document.o \
  outlay.o \
  path_utility.o \
  progress_meter.o \
  rounding_rules.o \
  secure_date.o \
  security.o \
  sigfpe.o \
  single_cell_document.o \
  surrchg_rates.o \
  system_command.o \
  tiered_charges.o \
  timer.o \
  xenumtypes.o \
  xrange.o \

lmi_cli_objects := \
  alert_cli.o \
  file_command_cli.o \
  getopt.o \
  main_cli.o \
  main_common.o \
  progress_meter_cli.o \

lmi_wx_objects := \
  about_dialog.o \
  alert_wx.o \
  ce_product_name.o \
  census_document.o \
  census_view.o \
  date_control.o \
  datum_base.o \
  datum_boolean.o \
  datum_string.o \
  docmanager_ex.o \
  docmdichildframe_ex.o \
  facets.o \
  file_command_wx.o \
  getopt.o \
  illustration_document.o \
  illustration_view.o \
  input.o \
  input_harmonization.o \
  lmi.rc.o \
  main_common.o \
  main_wx.o \
  mc_enum.o \
  mc_enum_types.o \
  previewframe_ex.o \
  progress_meter_wx.o \
  text_doc.o \
  text_view.o \
  tn_range_types.o \
  transferor.o \
  view_ex.o \
  wx_checks.o \
  xml_notebook.o \

################################################################################

# GPT server. This has not been maintained and may not work.

# Only these files are unique to the server application:

gps_objects_unique_to_server := \
  ihs_server7702.o \
  ihs_server7702io.o \

# Only these files are directly concerned with the GPT:

gps_objects_directly_concerned_with_gpt := \
  ihs_irc7702a.o \
  ihs_irc7702.o \

# These files provide general product support:

gps_objects := \
  $(gps_objects_unique_to_server) \
  $(gps_objects_directly_concerned_with_gpt) \
  actuarial_table.o \
  alert.o \
  alert_cli.o \
  calendar_date.o \
  crc32.o \
  data_directory.o \
  dbnames.o \
  death_benefits.o \
  expm1.o \
  fenv_lmi.o \
  ihs_basicval.o \
  ihs_commfns.o \
  ihs_database.o \
  ihs_dbdict.o \
  ihs_dbvalue.o \
  ihs_fpios.o \
  ihs_funddata.o \
  ihs_loads.o \
  ihs_mortal.o \
  ihs_pios.o \
  ihs_proddata.o \
  ihs_rnddata.o \
  inputs.o \
  inputstatus.o \
  interest_rates.o \
  kludges.o \
  mathmisc.o \
  miscellany.o \
  outlay.o \
  rounding_rules.o \
  surrchg_rates.o \
  tiered_charges.o \
  timer.o \
  xenumtypes.o \
  xrange.o \

################################################################################

# Unit tests.

unit_test_targets := \
  actuarial_table_test$(EXEEXT) \
  alert_test$(EXEEXT) \
  any_member_test$(EXEEXT) \
  argv0_test$(EXEEXT) \
  calendar_date_test$(EXEEXT) \
  callback_test$(EXEEXT) \
  commutation_functions_test$(EXEEXT) \
  comma_punct_test$(EXEEXT) \
  crc32_test$(EXEEXT) \
  financial_test$(EXEEXT) \
  input_seq_test$(EXEEXT) \
  input_test$(EXEEXT) \
  irc7702a_test$(EXEEXT) \
  kludges_test$(EXEEXT) \
  materially_equal_test$(EXEEXT) \
  mathmisc_test$(EXEEXT) \
  math_functors_test$(EXEEXT) \
  mc_enum_test$(EXEEXT) \
  mpatrol_patch_test$(EXEEXT) \
  ncnnnpnn_test$(EXEEXT) \
  numeric_io_test$(EXEEXT) \
  obstruct_slicing_test$(EXEEXT) \
  passkey_test$(EXEEXT) \
  path_utility_test$(EXEEXT) \
  progress_meter_test$(EXEEXT) \
  quiet_nan_test$(EXEEXT) \
  round_to_test$(EXEEXT) \
  snprintf_test$(EXEEXT) \
  stream_cast_test$(EXEEXT) \
  test_tools_test$(EXEEXT) \
  timer_test$(EXEEXT) \
  tn_range_test$(EXEEXT) \
  value_cast_ihs_test$(EXEEXT) \
  value_cast_test$(EXEEXT) \
  wx_new_test$(EXEEXT) \
  xenum_test$(EXEEXT) \
  xrange_test$(EXEEXT) \
  zero_test$(EXEEXT) \

# Link these objects for all tests:
common_test_objects := \
  fenv_lmi.o \
  getopt.o \
  kludges.o \
  license.o \

actuarial_table_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  actuarial_table.o \
  actuarial_table_test.o \
  timer.o \

alert_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  alert_test.o \

any_member_test$(EXEEXT): \
  $(common_test_objects) \
  any_member_test.o \

argv0_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  argv0_test.o \

calendar_date_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  calendar_date.o \
  calendar_date_test.o \

callback_test$(EXEEXT): \
  $(common_test_objects) \
  callback_test.o \

commutation_functions_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  commutation_functions_test.o \
  expm1.o \
  ihs_commfns.o \
  timer.o \
  xenumtypes.o \

comma_punct_test$(EXEEXT): \
  $(common_test_objects) \
  comma_punct_test.o \

crc32_test$(EXEEXT): \
  $(common_test_objects) \
  crc32.o \
  crc32_test.o \

financial_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  financial_test.o \
  mathmisc.o \
  timer.o \

input_seq_test$(EXEEXT): \
  $(common_test_objects) \
  input_seq_test.o \
  input_sequence.o \

input_test$(EXEEXT): LIBS = $(LIBXML2_LIBS)
input_test$(EXEEXT): \
  $(LIBXML2_LIBS) \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  $(xmlwrapp_objects) \
  alert.o \
  alert_cli.o \
  calendar_date.o \
  configurable_settings.o \
  data_directory.o \
  database.o \
  dbdict.o \
  dbnames.o \
  dbvalue.o \
  global_settings.o \
  input_seq_helpers.o \
  input_sequence.o \
  input_test.o \
  inputillus.o \
  inputillus_sequences.o \
  inputillus_term_rider.o \
  inputillus_xml_io.o \
  inputs.o \
  inputstatus.o \
  kludges.o \
  license.o \
  multiple_cell_document.o \
  single_cell_document.o \
  xenumtypes.o \
  xrange.o \
  $(extra_libs) \

irc7702a_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  ihs_irc7702a.o \
  irc7702a_test.o \
  mathmisc.o \

kludges_test$(EXEEXT): \
  $(common_test_objects) \
  kludges_test.o \

materially_equal_test$(EXEEXT): \
  $(common_test_objects) \
  materially_equal_test.o \

mathmisc_test$(EXEEXT): \
  $(common_test_objects) \
  mathmisc_test.o \

math_functors_test$(EXEEXT): \
  $(common_test_objects) \
  expm1.o \
  math_functors_test.o \
  timer.o \

mc_enum_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  datum_base.o \
  facets.o \
  global_settings.o \
  mc_enum.o \
  mc_enum_test.o \
  mc_enum_test_aux.o \

mpatrol_patch_test$(EXEEXT): \
  $(common_test_objects) \
  mpatrol_patch_test.o \

ncnnnpnn_test$(EXEEXT): \
  $(common_test_objects) \
  ncnnnpnn_test.o \

numeric_io_test$(EXEEXT): \
  $(common_test_objects) \
  numeric_io_test.o \
  timer.o \

obstruct_slicing_test$(EXEEXT): \
  $(common_test_objects) \
  obstruct_slicing_test.o \
  timer.o \

passkey_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  calendar_date.o \
  md5.o \
  passkey_test.o \
  secure_date.o \
  system_command.o \

path_utility_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  alert.o \
  miscellany.o \
  path_utility.o \
  path_utility_test.o \

progress_meter_test$(EXEEXT): \
  $(common_test_objects) \
  progress_meter.o \
  progress_meter_cli.o \
  progress_meter_test.o \

quiet_nan_test$(EXEEXT): \
  $(common_test_objects) \
  quiet_nan_test.o \

round_to_test$(EXEEXT): \
  $(common_test_objects) \
  round_to_test.o \

snprintf_test$(EXEEXT): \
  $(common_test_objects) \
  snprintf_test.o \

stream_cast_test$(EXEEXT): \
  $(common_test_objects) \
  facets.o \
  stream_cast_test.o \

test_tools_test$(EXEEXT): \
  $(common_test_objects) \
  test_tools_test.o \

timer_test$(EXEEXT): \
  $(common_test_objects) \
  timer.o \
  timer_test.o \

tn_range_test$(EXEEXT): \
  $(common_test_objects) \
  datum_base.o \
  tn_range_test.o \
  tn_range_test_aux.o \

value_cast_ihs_test$(EXEEXT): \
  $(common_test_objects) \
  value_cast_ihs_test.o \

value_cast_test$(EXEEXT): \
  $(common_test_objects) \
  value_cast_test.o \

wx_new_test$(EXEEXT): \
  $(common_test_objects) \
  wx_new_test.o \

xenum_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  xenum_sample.o \
  xenum_test.o \

xrange_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  calendar_date.o \
  xrange.o \
  xrange_test.o \

zero_test$(EXEEXT): \
  $(common_test_objects) \
  alert.o \
  alert_cli.o \
  zero_test.o \

################################################################################

# Custom tools built from source.

elapsed_time$(EXEEXT): \
  alert.o \
  alert_cli.o \
  elapsed_time.o \
  system_command.o \
  timer.o \

generate_passkey$(EXEEXT): \
  alert.o \
  alert_cli.o \
  calendar_date.o \
  generate_passkey.o \
  md5.o \
  secure_date.o \
  system_command.o \

ihs_crc_comp$(EXEEXT): \
  ihs_crc_comp.o \

################################################################################

# Product files.

# TODO ?? Experiment with writing all dependencies and target-specific
# variable definitions here. If that works well, use the technique
# elsewhere.

product_files$(EXEEXT): lmi_dllflag := -DLMI_USE_DLL

product_files$(EXEEXT): \
  alert_cli.o \
  generate_product_files.o \
  my_db.o \
  my_fund.o \
  my_prod.o \
  my_rnd.o \
  my_tier.o \
  xenumtypes.o \
  liblmi$(SHREXT) \

