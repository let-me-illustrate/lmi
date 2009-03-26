# Makefile: object lists.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

# $Id: objects.make,v 1.146 2009-03-26 16:47:29 chicares Exp $

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

# These object files are used in both an application and a shared
# library that it links to, only for builds that use shared-library
# 'attributes'. This workaround is used merely because we don't yet
# build these objects as a library.

ifneq (,$(USE_SO_ATTRIBUTES))
  duplicated_objects = $(boost_filesystem_objects)
endif

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
  ait_impl.o \
  attributes.o \
  document.o \
  dtd_impl.o \
  event_parser.o \
  init.o \
  node.o \
  node_iterator.o \
  node_manip.o \
  tree_parser.o \
  utility.o \

# For systems that already have boost, cgicc, and xmlwrapp libraries
# installed, define 'HAVE_THIRD_PARTY_LIBRARIES' to use them instead
# of using the workarounds above.
#
# TODO ?? It would make more sense to build these as lib*.a, instead
# of as collections of object files, on systems that don't already
# have them as libraries.

ifdef HAVE_THIRD_PARTY_LIBRARIES
  boost_filesystem_objects :=
  cgicc_objects :=
  xmlwrapp_objects :=
endif

################################################################################

# All non-wx auxiliary programs built here (other than illustration
# systems) share a common main() implementation that performs certain
# default initialization and catches and handles exceptions. They
# should use this variable in their object lists unless some of these
# objects are included in libraries they already use.

main_auxiliary_common_objects := \
  alert.o \
  alert_cli.o \
  fenv_lmi.o \
  main_common.o \
  main_common_non_wx.o \
  sigfpe.o \

################################################################################

# Generic interface-drivers that can be linked to either calculation
# branch.

cgi_objects := \
  $(cgicc_objects) \
  $(duplicated_objects) \
  alert_cgi.o \
  file_command_cgi.o \
  main_cgi.o \
  main_common.o \
  main_common_non_wx.o \
  progress_meter_cgi.o \
  system_command_non_wx.o \

cli_objects := \
  $(duplicated_objects) \
  alert_cli.o \
  file_command_cli.o \
  main_cli.o \
  main_common.o \
  main_common_non_wx.o \
  progress_meter_cli.o \
  system_command_non_wx.o \

################################################################################

# Illustrations: files shared by the antediluvian and production branches.

common_common_objects := \
  $(boost_filesystem_objects) \
  $(xmlwrapp_objects) \
  actuarial_table.o \
  alert.o \
  calendar_date.o \
  ce_product_name.o \
  configurable_settings.o \
  crc32.o \
  custom_io_0.o \
  data_directory.o \
  datum_base.o \
  datum_boolean.o \
  datum_string.o \
  dbnames.o \
  death_benefits.o \
  emit_ledger.o \
  expm1.o \
  facets.o \
  fenv_guard.o \
  fenv_lmi.o \
  file_command.o \
  getopt.o \
  global_settings.o \
  group_values.o \
  illustrator.o \
  input.o \
  input_harmonization.o \
  input_realization.o \
  input_seq_helpers.o \
  input_sequence.o \
  input_xml_io.o \
  interest_rates.o \
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
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  multiple_cell_document.o \
  mvc_model.o \
  name_value_pairs.o \
  null_stream.o \
  outlay.o \
  path_utility.o \
  product_names.o \
  progress_meter.o \
  rounding_rules.o \
  sigfpe.o \
  single_cell_document.o \
  streamable.o \
  surrchg_rates.o \
  system_command.o \
  timer.o \
  tn_range_types.o \
  xml_lmi.o \
  xslt_lmi.o \
  yare_input.o \

################################################################################

# Illustrations: the antediluvian branch. This code is fairly simple
# and clean, but less powerful than the production branch. For
# historical reasons, these 'branches' aren't maintained in separate
# cvs branches; instead, all their source code resides in the same
# place, and conflicts are prevented by prepending 'ihs_' to the
# names of many files in the production branch. Gradually the
# differences are being eliminated.

antediluvian_common_objects := \
  $(common_common_objects) \
  accountvalue.o \
  antediluvian_stubs.o \
  avdebug.o \
  basicvalues.o \
  database.o \
  dbdict.o \
  dbvalue.o \
  mortality_rates.o \
  solve.o \

################################################################################

# Illustrations: the production branch.

lmi_common_objects := \
  $(common_common_objects) \
  authenticity.o \
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
  ihs_irc7702.o \
  ihs_irc7702a.o \
  ihs_mortal.o \
  ihs_pios.o \
  ihs_proddata.o \
  ihs_rnddata.o \
  md5.o \
  mortality_rates_fetch.o \
  preferences_model.o \
  stratified_algorithms.o \
  stratified_charges.o \

lmi_wx_objects := \
  $(duplicated_objects) \
  about_dialog.o \
  alert_wx.o \
  census_document.o \
  census_view.o \
  database_document.o \
  database_view.o \
  database_view_editor.o \
  default_view.o \
  docmanager_ex.o \
  docmdichildframe_ex.o \
  file_command_wx.o \
  icon_monger.o \
  illustration_document.o \
  illustration_view.o \
  main_common.o \
  main_wx.o \
  msw_workarounds.o \
  multidimgrid_any.o \
  multidimgrid_tools.o \
  mvc_controller.o \
  mvc_view.o \
  policy_document.o \
  policy_view.o \
  preferences_view.o \
  previewframe_ex.o \
  product_editor.o \
  progress_meter_wx.o \
  rounding_document.o \
  rounding_view.o \
  rounding_view_editor.o \
  single_choice_popup_menu.o \
  system_command_wx.o \
  text_doc.o \
  text_view.o \
  tier_document.o \
  tier_view.o \
  tier_view_editor.o \
  transferor.o \
  view_ex.o \
  wx_checks.o \
  wx_utility.o \

ifneq (,$(RC))
  lmi_wx_objects += lmi.rc.o
endif

################################################################################

# GPT server. This has not been maintained and may not work.

# Only these files are unique to the server application:

gpt_objects_unique_to_server := \
  ihs_server7702.o \
  ihs_server7702io.o \

# Only these files are directly concerned with the GPT:

gpt_objects_directly_concerned_with_gpt := \
  ihs_irc7702a.o \
  ihs_irc7702.o \

# These files provide general product support:

gpt_objects := \
  $(boost_filesystem_objects) \
  $(gpt_objects_unique_to_server) \
  $(gpt_objects_directly_concerned_with_gpt) \
  $(xmlwrapp_objects) \
  actuarial_table.o \
  alert.o \
  alert_cli.o \
  calendar_date.o \
  ce_product_name.o \
  configurable_settings.o \
  crc32.o \
  data_directory.o \
  datum_base.o \
  datum_string.o \
  dbnames.o \
  death_benefits.o \
  expm1.o \
  facets.o \
  fenv_lmi.o \
  global_settings.o \
  ihs_basicval.o \
  ihs_commfns.o \
  ihs_database.o \
  ihs_dbdict.o \
  ihs_dbvalue.o \
  ihs_fpios.o \
  ihs_funddata.o \
  ihs_mortal.o \
  ihs_pios.o \
  ihs_proddata.o \
  ihs_rnddata.o \
  input.o \
  input_harmonization.o \
  input_realization.o \
  input_seq_helpers.o \
  input_sequence.o \
  input_xml_io.o \
  interest_rates.o \
  loads.o \
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  mortality_rates_fetch.o \
  mvc_model.o \
  null_stream.o \
  outlay.o \
  path_utility.o \
  product_names.o \
  rounding_rules.o \
  stratified_algorithms.o \
  stratified_charges.o \
  streamable.o \
  surrchg_rates.o \
  tn_range_types.o \
  timer.o \
  xml_lmi.o \
  yare_input.o \

libgpt.a libgpt$(SHREXT): EXTRA_LDFLAGS =
libgpt.a libgpt$(SHREXT): $(gpt_objects)
gpt_server$(EXEEXT)     : $(gpt_objects)

gpt_so_test$(EXEEXT): ihs_server7702test.o libgpt$(SHREXT)

################################################################################

# Unit tests.

# 'round_test' fails spectacularly with MinGW gcc, and is therefore
# suppressed until we upgrade to the new libmingwex version that it
# was designed to test.
#
# Override this variable to exclude other tests where appropriate,
# e.g., when a test doesn't even compile with a particular toolchain.

excluded_unit_test_targets := \
  round_test \

unit_test_targets := \
  account_value_test \
  actuarial_table_test \
  alert_test \
  any_member_test \
  assert_lmi_test \
  authenticity_test \
  calendar_date_test \
  callback_test \
  commutation_functions_test \
  comma_punct_test \
  crc32_test \
  expression_template_0_test \
  fenv_lmi_test \
  file_command_test \
  financial_test \
  getopt_test \
  global_settings_test \
  handle_exceptions_test \
  input_seq_test \
  input_test \
  irc7702a_test \
  istream_to_string_test \
  loads_test \
  map_lookup_test \
  materially_equal_test \
  math_functors_test \
  mc_enum_test \
  miscellany_test \
  mortality_rates_test \
  mpatrol_patch_test \
  name_value_pairs_test \
  ncnnnpnn_test \
  numeric_io_test \
  obstruct_slicing_test \
  path_utility_test \
  print_matrix_test \
  progress_meter_test \
  quiet_nan_test \
  regex_test \
  round_test \
  round_to_test \
  rtti_lmi_test \
  safely_dereference_as_test \
  sandbox_test \
  snprintf_test \
  stratified_algorithms_test \
  stream_cast_test \
  system_command_test \
  test_tools_test \
  timer_test \
  tn_range_test \
  value_cast_test \
  vector_test \
  wx_new_test \
  zero_test \

unit_test_targets := \
  $(addsuffix $(EXEEXT), \
    $(filter-out $(excluded_unit_test_targets), $(unit_test_targets) \
    ) \
  )

# Link these objects for all tests:
common_test_objects := \
  alert.o \
  alert_cli.o \
  fenv_lmi.o \
  getopt.o \
  license.o \

# List required object files explicitly for each test unless several
# dozen are required. List only object files, not libraries, to avoid
# the overhead of creating libraries: a particular unit test may be
# built and run many times in succession during iterative development,
# and any unnecessary overhead is unwelcome.

account_value_test$(EXEEXT): \
  $(common_test_objects) \
  account_value_test.o \

actuarial_table_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  actuarial_table.o \
  actuarial_table_test.o \
  timer.o \

alert_test$(EXEEXT): \
  $(common_test_objects) \
  alert_test.o \

any_member_test$(EXEEXT): \
  $(common_test_objects) \
  any_member_test.o \
  facets.o \

assert_lmi_test$(EXEEXT): \
  $(common_test_objects) \
  assert_lmi_test.o \

authenticity_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  authenticity.o \
  authenticity_test.o \
  calendar_date.o \
  global_settings.o \
  md5.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  system_command.o \
  system_command_non_wx.o \

calendar_date_test$(EXEEXT): \
  $(common_test_objects) \
  calendar_date.o \
  calendar_date_test.o \
  null_stream.o \
  timer.o \

callback_test$(EXEEXT): \
  $(common_test_objects) \
  callback_test.o \

commutation_functions_test$(EXEEXT): \
  $(common_test_objects) \
  commutation_functions_test.o \
  expm1.o \
  ihs_commfns.o \
  timer.o \

comma_punct_test$(EXEEXT): \
  $(common_test_objects) \
  comma_punct_test.o \

crc32_test$(EXEEXT): \
  $(common_test_objects) \
  crc32.o \
  crc32_test.o \

expression_template_0_test$(EXEEXT): \
  $(common_test_objects) \
  expression_template_0_test.o \
  timer.o \

fenv_lmi_test$(EXEEXT): \
  $(common_test_objects) \
  fenv_guard.o \
  fenv_lmi_test.o \

file_command_test$(EXEEXT): \
  $(common_test_objects) \
  file_command.o \
  file_command_cli.o \
  file_command_test.o \

financial_test$(EXEEXT): \
  $(common_test_objects) \
  financial_test.o \
  null_stream.o \
  stratified_algorithms.o \
  timer.o \

getopt_test$(EXEEXT): \
  $(common_test_objects) \
  getopt.o \
  getopt_test.o \

global_settings_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  global_settings.o \
  global_settings_test.o \
  miscellany.o \
  path_utility.o \

handle_exceptions_test$(EXEEXT): \
  $(common_test_objects) \
  handle_exceptions_test.o \

input_seq_test$(EXEEXT): \
  $(common_test_objects) \
  input_seq_test.o \
  input_sequence.o \

input_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  $(xmlwrapp_objects) \
  calendar_date.o \
  ce_product_name.o \
  configurable_settings.o \
  data_directory.o \
  database.o \
  datum_base.o \
  datum_string.o \
  dbdict.o \
  dbnames.o \
  dbvalue.o \
  facets.o \
  global_settings.o \
  input.o \
  input_harmonization.o \
  input_realization.o \
  input_seq_helpers.o \
  input_sequence.o \
  input_test.o \
  input_xml_io.o \
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  multiple_cell_document.o \
  mvc_model.o \
  null_stream.o \
  path_utility.o \
  product_names.o \
  single_cell_document.o \
  streamable.o \
  timer.o \
  tn_range_types.o \
  xml_lmi.o \
  yare_input.o \

irc7702a_test$(EXEEXT): \
  $(common_test_objects) \
  ihs_irc7702a.o \
  irc7702a_test.o \
  stratified_algorithms.o \

istream_to_string_test$(EXEEXT): \
  $(common_test_objects) \
  istream_to_string_test.o \
  timer.o \

loads_test$(EXEEXT): \
  $(common_test_objects) \
  expm1.o \
  loads.o \
  loads_test.o \
  timer.o \

map_lookup_test$(EXEEXT): \
  $(common_test_objects) \
  map_lookup_test.o \

materially_equal_test$(EXEEXT): \
  $(common_test_objects) \
  materially_equal_test.o \

math_functors_test$(EXEEXT): \
  $(common_test_objects) \
  expm1.o \
  math_functors_test.o \
  timer.o \

mc_enum_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  datum_base.o \
  facets.o \
  global_settings.o \
  mc_enum.o \
  mc_enum_test.o \
  mc_enum_test_aux.o \
  miscellany.o \
  path_utility.o \

miscellany_test$(EXEEXT): \
  $(common_test_objects) \
  miscellany.o \
  miscellany_test.o \

mortality_rates_test$(EXEEXT): \
  $(common_test_objects) \
  ihs_mortal.o \
  mortality_rates_test.o \

mpatrol_patch_test$(EXEEXT): \
  $(common_test_objects) \
  mpatrol_patch_test.o \

name_value_pairs_test$(EXEEXT): \
  $(common_test_objects) \
  facets.o \
  name_value_pairs.o \
  name_value_pairs_test.o \

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

path_utility_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  global_settings.o \
  miscellany.o \
  path_utility.o \
  path_utility_test.o \

print_matrix_test$(EXEEXT): \
  $(common_test_objects) \
  facets.o \
  print_matrix_test.o \

progress_meter_test$(EXEEXT): \
  $(common_test_objects) \
  null_stream.o \
  progress_meter.o \
  progress_meter_cli.o \
  progress_meter_test.o \

quiet_nan_test$(EXEEXT): \
  $(common_test_objects) \
  quiet_nan_test.o \

regex_test$(EXEEXT): EXTRA_LDFLAGS = -lboost_regex-gcc-1_33_1
regex_test$(EXEEXT): \
  $(common_test_objects) \
  regex_test.o \
  timer.o \

round_test$(EXEEXT): \
  $(common_test_objects) \
  round_test.o \

round_to_test$(EXEEXT): \
  $(common_test_objects) \
  round_to_test.o \

rtti_lmi_test$(EXEEXT): \
  $(common_test_objects) \
  rtti_lmi_test.o \

safely_dereference_as_test$(EXEEXT): \
  $(common_test_objects) \
  safely_dereference_as_test.o \

sandbox_test$(EXEEXT): \
  $(common_test_objects) \
  sandbox_test.o \

snprintf_test$(EXEEXT): \
  $(common_test_objects) \
  snprintf_test.o \

stratified_algorithms_test$(EXEEXT): \
  $(common_test_objects) \
  stratified_algorithms_test.o \

stream_cast_test$(EXEEXT): \
  $(common_test_objects) \
  facets.o \
  stream_cast_test.o \

system_command_test$(EXEEXT): \
  $(common_test_objects) \
  system_command.o \
  system_command_non_wx.o \
  system_command_test.o \

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
  facets.o \
  tn_range_test.o \
  tn_range_test_aux.o \

value_cast_test$(EXEEXT): \
  $(common_test_objects) \
  facets.o \
  value_cast_test.o \

vector_test$(EXEEXT): \
  $(common_test_objects) \
  timer.o \
  vector_test.o \

wx_new_test$(EXEEXT): \
  $(common_test_objects) \
  wx_new_test.o \

zero_test$(EXEEXT): \
  $(common_test_objects) \
  null_stream.o \
  zero_test.o \

################################################################################

# Custom tools built from source.

elapsed_time$(EXEEXT): \
  $(main_auxiliary_common_objects) \
  elapsed_time.o \
  system_command.o \
  system_command_non_wx.o \
  timer.o \

generate_passkey$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(main_auxiliary_common_objects) \
  authenticity.o \
  calendar_date.o \
  generate_passkey.o \
  global_settings.o \
  md5.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  system_command.o \
  system_command_non_wx.o \

ihs_crc_comp$(EXEEXT): \
  $(main_auxiliary_common_objects) \
  ihs_crc_comp.o \

test_coding_rules_test := $(src_dir)/test_coding_rules_test.sh
test_coding_rules$(EXEEXT): POST_LINK_COMMAND = $(test_coding_rules_test)
test_coding_rules$(EXEEXT): EXTRA_LDFLAGS = -lboost_regex-gcc-1_33_1
test_coding_rules$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(main_auxiliary_common_objects) \
  my_test_coding_rules.o \
  test_coding_rules.o \

# Wrappers for borland tools.

bcc_ar$(EXEEXT): \
  $(main_auxiliary_common_objects) \
  bcc_ar.o \
  getopt.o \
  license.o \
  system_command.o \
  system_command_non_wx.o \

bcc_cc$(EXEEXT): \
  $(main_auxiliary_common_objects) \
  bcc_cc.o \
  getopt.o \
  license.o \
  system_command.o \
  system_command_non_wx.o \

bcc_ld$(EXEEXT): \
  $(main_auxiliary_common_objects) \
  bcc_ld.o \
  getopt.o \
  license.o \
  system_command.o \
  system_command_non_wx.o \

bcc_rc$(EXEEXT): \
  $(main_auxiliary_common_objects) \
  bcc_rc.o \
  getopt.o \
  license.o \
  system_command.o \
  system_command_non_wx.o \

################################################################################

# Product files.

# TODO ?? Experiment with writing all dependencies and target-specific
# variable definitions here. If that works well, use the technique
# elsewhere.

product_files$(EXEEXT): \
  alert_cli.o \
  generate_product_files.o \
  main_common.o \
  main_common_non_wx.o \
  my_db.o \
  my_fund.o \
  my_prod.o \
  my_rnd.o \
  my_tier.o \
  liblmi$(SHREXT) \

