# Makefile: object lists.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

################################################################################

# External libraries whose source is compiled here. Their source
# is reached through 'vpath' directives. See the rationale in
# 'workhorse.make'.

# Boost filesystem and regex libraries. The other boost libraries that
# lmi requires are implemented entirely in headers.
#
# As for listing the object files here, the regex author says:
#   http://groups.google.com/group/boost-list/msg/7f925ca50d69384b
# | add the libs/regex/src/*.cpp files to your project

boost_filesystem_objects := \
  convenience.o \
  exception.o \
  path_posix_windows.o \
  operations_posix_windows.o \

boost_regex_objects := \
  c_regex_traits.o \
  cpp_regex_traits.o \
  cregex.o \
  fileiter.o \
  icu.o \
  instances.o \
  posix_api.o \
  regex.o \
  regex_debug.o \
  regex_raw_buffer.o \
  regex_traits_defaults.o \
  static_mutex.o \
  usinstances.o \
  w32_regex_traits.o \
  wc_regex_traits.o \
  wide_posix_api.o \
  winstances.o \

# These object files are used in both an application and a shared
# library that it links to, only for builds that use shared-library
# 'attributes'. This workaround is used merely because we don't yet
# build these objects as a library. TODO ?? The duplication is not
# correct: it validates linking, but the linked applications don't
# run correctly.

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

# For systems that already have boost and cgicc libraries
# installed, define 'HAVE_THIRD_PARTY_LIBRARIES' to use them instead
# of using the workarounds above.
#
# TODO ?? It would make more sense to build these as lib*.a, instead
# of as collections of object files, on systems that don't already
# have them as libraries.

ifdef HAVE_THIRD_PARTY_LIBRARIES
  boost_filesystem_objects :=
  boost_regex_objects :=
  cgicc_objects :=
endif

################################################################################

# All non-wx auxiliary programs built here (that don't link 'liblmi'
# already) share a common main() implementation that performs certain
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
  actuarial_table.o \
  alert.o \
  calendar_date.o \
  ce_product_name.o \
  ce_skin_name.o \
  configurable_settings.o \
  crc32.o \
  custom_io_0.o \
  custom_io_1.o \
  data_directory.o \
  database.o \
  datum_base.o \
  datum_boolean.o \
  datum_sequence.o \
  datum_string.o \
  dbdict.o \
  dbnames.o \
  dbo_rules.o \
  dbvalue.o \
  death_benefits.o \
  emit_ledger.o \
  facets.o \
  fenv_guard.o \
  fenv_lmi.o \
  file_command.o \
  getopt.o \
  global_settings.o \
  group_quote_pdf_gen.o \
  group_values.o \
  html.o \
  illustrator.o \
  input.o \
  input_harmonization.o \
  input_realization.o \
  input_sequence.o \
  input_sequence_aux.o \
  input_sequence_parser.o \
  input_xml_io.o \
  interest_rates.o \
  interpolate_string.o \
  ledger.o \
  ledger_base.o \
  ledger_evaluator.o \
  ledger_invariant.o \
  ledger_invariant_init.o \
  ledger_pdf.o \
  ledger_text_formats.o \
  ledger_variant.o \
  ledger_variant_init.o \
  ledgervalues.o \
  license.o \
  loads.o \
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  multiple_cell_document.o \
  mvc_model.o \
  my_proem.o \
  name_value_pairs.o \
  null_stream.o \
  outlay.o \
  path_utility.o \
  pdf_command.o \
  premium_tax.o \
  progress_meter.o \
  round_glibc.o \
  sigfpe.o \
  single_cell_document.o \
  system_command.o \
  timer.o \
  tn_range_types.o \
  unwind.o \
  xml_lmi.o \
  yare_input.o \

################################################################################

# Illustrations: the antediluvian branch. This code is fairly simple
# and clean, but less powerful than the production branch. For
# historical reasons, these 'branches' aren't maintained in separate
# repository branches; instead, all their source code resides in the
# same place, and conflicts are prevented by prepending 'ihs_' to the
# names of many files in the production branch. Gradually the
# differences are being eliminated.

antediluvian_common_objects := \
  $(common_common_objects) \
  accountvalue.o \
  antediluvian_stubs.o \
  basicvalues.o \
  mortality_rates.o \
  solve.o \

################################################################################

# Illustrations: the production branch.

lmi_common_objects := \
  $(common_common_objects) \
  authenticity.o \
  basic_tables.o \
  commutation_functions.o \
  cso_table.o \
  financial.o \
  fund_data.o \
  gpt_commutation_functions.o \
  gpt_input.o \
  gpt_server.o \
  gpt_specamt.o \
  gpt_state.o \
  gpt_xml_document.o \
  i7702.o \
  i7702_init.o \
  ihs_acctval.o \
  ihs_avdebug.o \
  ihs_avmly.o \
  ihs_avsolve.o \
  ihs_avstrtgy.o \
  ihs_basicval.o \
  ihs_irc7702.o \
  ihs_irc7702a.o \
  ihs_mortal.o \
  ihs_server7702.o \
  irc7702.o \
  irc7702_tables.o \
  lingo.o \
  lmi.o \
  md5.o \
  md5sum.o \
  mec_input.o \
  mec_server.o \
  mec_state.o \
  mec_xml_document.o \
  mortality_rates_fetch.o \
  preferences_model.o \
  product_data.o \
  report_table.o \
  rounding_rules.o \
  stratified_algorithms.o \
  stratified_charges.o \
  verify_products.o \

skeleton_objects := \
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
  file_command_wx.o \
  gpt_document.o \
  gpt_view.o \
  group_quote_pdf_gen_wx.o \
  icon_monger.o \
  illustration_document.o \
  illustration_view.o \
  input_sequence_entry.o \
  main_common.o \
  mec_document.o \
  mec_view.o \
  msw_workarounds.o \
  multidimgrid_any.o \
  multidimgrid_tools.o \
  mvc_controller.o \
  mvc_view.o \
  pdf_command_wx.o \
  pdf_writer_wx.o \
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
  skeleton.o \
  system_command_wx.o \
  text_doc.o \
  text_view.o \
  tier_document.o \
  tier_view.o \
  tier_view_editor.o \
  transferor.o \
  view_ex.o \
  wx_checks.o \
  wx_table_generator.o \
  wx_utility.o \

lmi_wx_objects := \
  main_wx.o \

wx_test_objects := \
  $(boost_regex_objects) \
  main_wx_test.o \
  wx_test_about_version.o \
  wx_test_benchmark_census.o \
  wx_test_calculation_summary.o \
  wx_test_config_settings.o \
  wx_test_create_open.o \
  wx_test_default_input.o \
  wx_test_default_update.o \
  wx_test_expiry_dates.o \
  wx_test_input_sequences.o \
  wx_test_input_validation.o \
  wx_test_log_errors.o \
  wx_test_paste_census.o \
  wx_test_pdf_create.o \
  wx_test_validate_output.o \

ifneq (,$(RC))
  lmi_wx_objects  += lmi_msw_res.o
  wx_test_objects += lmi_msw_res.o
endif

################################################################################

# Unit tests.

# Override this variable to exclude tests that are inappropriate in
# context--e.g., tests that don't even compile with a particular
# toolchain. For example:
#
# ifeq (foobar,$(build_type))
#   excluded_unit_test_targets += calendar_date_test
# endif

excluded_unit_test_targets :=

unit_test_targets := \
  account_value_test \
  actuarial_table_test \
  alert_test \
  any_member_test \
  assert_lmi_test \
  authenticity_test \
  bourn_cast_test \
  cache_file_reads_test \
  calendar_date_test \
  callback_test \
  comma_punct_test \
  commutation_functions_test \
  configurable_settings_test \
  contains_test \
  crc32_test \
  currency_test \
  dbo_rules_test \
  et_vector_test \
  expression_template_0_test \
  fenv_lmi_test \
  file_command_test \
  financial_test \
  getopt_test \
  global_settings_test \
  gpt_cf_triad_test \
  gpt_test \
  handle_exceptions_test \
  i7702_test \
  ieee754_test \
  input_sequence_test \
  input_test \
  interpolate_string_test \
  irc7702_tables_test \
  irc7702a_test \
  istream_to_string_test \
  ledger_test \
  loads_test \
  map_lookup_test \
  materially_equal_test \
  math_functions_test \
  mc_enum_test \
  md5sum_test \
  miscellany_test \
  monnaie_test \
  mortality_rates_test \
  name_value_pairs_test \
  ncnnnpnn_test \
  numeric_io_test \
  path_utility_test \
  premium_tax_test \
  print_matrix_test \
  product_file_test \
  progress_meter_test \
  rate_table_test \
  regex_test \
  report_table_test \
  round_test \
  round_to_test \
  rtti_lmi_test \
  safely_dereference_as_test \
  sandbox_test \
  snprintf_test \
  ssize_lmi_test \
  stratified_algorithms_test \
  stream_cast_test \
  system_command_test \
  test_tools_test \
  timer_test \
  tn_range_test \
  value_cast_test \
  vector_test \
  wx_new_test \
  xml_serialize_test \
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
  unwind.o \

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
  cso_table.o \
  timer.o \
  xml_lmi.o \

alert_test$(EXEEXT): \
  $(common_test_objects) \
  alert_test.o \

any_member_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  any_member_test.o \
  calendar_date.o \
  facets.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \

assert_lmi_test$(EXEEXT): \
  $(common_test_objects) \
  assert_lmi_test.o \

# MD5 !! Remove "timer.o" below.
authenticity_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  authenticity.o \
  authenticity_test.o \
  calendar_date.o \
  global_settings.o \
  md5.o \
  md5sum.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  system_command.o \
  system_command_non_wx.o \
  timer.o \

bourn_cast_test$(EXEEXT): \
  $(common_test_objects) \
  bourn_cast_test.o \
  timer.o \

cache_file_reads_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  cache_file_reads_test.o \
  timer.o \

calendar_date_test$(EXEEXT): \
  $(common_test_objects) \
  calendar_date.o \
  calendar_date_test.o \
  null_stream.o \
  timer.o \

callback_test$(EXEEXT): \
  $(common_test_objects) \
  callback_test.o \

comma_punct_test$(EXEEXT): \
  $(common_test_objects) \
  comma_punct_test.o \

commutation_functions_test$(EXEEXT): \
  $(common_test_objects) \
  commutation_functions.o \
  commutation_functions_test.o \
  cso_table.o \
  timer.o \

configurable_settings_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  configurable_settings.o \
  configurable_settings_test.o \
  data_directory.o \
  datum_base.o \
  facets.o \
  global_settings.o \
  mc_enum.o \
  mc_enum_types.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  xml_lmi.o \

contains_test$(EXEEXT): \
  $(common_test_objects) \
  contains_test.o \

crc32_test$(EXEEXT): \
  $(common_test_objects) \
  crc32.o \
  crc32_test.o \

currency_test$(EXEEXT): \
  $(common_test_objects) \
  currency_test.o \
  timer.o \

dbo_rules_test$(EXEEXT): \
  $(common_test_objects) \
  datum_base.o \
  dbo_rules.o \
  dbo_rules_test.o \
  facets.o \
  mc_enum.o \
  mc_enum_types.o \
  timer.o \

et_vector_test$(EXEEXT): \
  $(common_test_objects) \
  et_vector_test.o \
  timer.o \

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
  calendar_date.o \
  financial.o \
  financial_test.o \
  null_stream.o \
  stratified_algorithms.o \
  timer.o \

getopt_test$(EXEEXT): \
  $(common_test_objects) \
  getopt_test.o \

global_settings_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  global_settings.o \
  global_settings_test.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \

gpt_cf_triad_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  commutation_functions.o \
  cso_table.o \
  global_settings.o \
  gpt_cf_triad_test.o \
  gpt_commutation_functions.o \
  ihs_irc7702.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  timer.o \

gpt_test$(EXEEXT): \
  $(common_test_objects) \
  commutation_functions.o \
  cso_table.o \
  gpt_commutation_functions.o \
  gpt_test.o \
  irc7702.o \

handle_exceptions_test$(EXEEXT): \
  $(common_test_objects) \
  handle_exceptions_test.o \

i7702_test$(EXEEXT): \
  $(common_test_objects) \
  i7702.o \
  i7702_test.o \
  timer.o \

ieee754_test$(EXEEXT): \
  $(common_test_objects) \
  ieee754_test.o \

input_sequence_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  global_settings.o \
  input_sequence.o \
  input_sequence_parser.o \
  input_sequence_test.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \

input_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  ce_product_name.o \
  configurable_settings.o \
  data_directory.o \
  database.o \
  datum_base.o \
  datum_sequence.o \
  datum_string.o \
  dbdict.o \
  dbnames.o \
  dbo_rules.o \
  dbvalue.o \
  facets.o \
  global_settings.o \
  input.o \
  input_harmonization.o \
  input_realization.o \
  input_sequence.o \
  input_sequence_aux.o \
  input_sequence_parser.o \
  input_test.o \
  input_xml_io.o \
  lmi.o \
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  multiple_cell_document.o \
  mvc_model.o \
  my_proem.o \
  null_stream.o \
  path_utility.o \
  premium_tax.o \
  product_data.o \
  single_cell_document.o \
  stratified_charges.o \
  timer.o \
  tn_range_types.o \
  xml_lmi.o \
  yare_input.o \

interpolate_string_test$(EXEEXT): \
  $(common_test_objects) \
  interpolate_string.o \
  interpolate_string_test.o \

irc7702_tables_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  commutation_functions.o \
  cso_table.o \
  global_settings.o \
  irc7702_tables.o \
  irc7702_tables_test.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \

irc7702a_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  global_settings.o \
  ihs_irc7702a.o \
  irc7702a_test.o \
  mec_state.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  stratified_algorithms.o \
  xml_lmi.o \

istream_to_string_test$(EXEEXT): \
  $(common_test_objects) \
  istream_to_string_test.o \
  timer.o \

ledger_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  configurable_settings.o \
  crc32.o \
  data_directory.o \
  datum_base.o \
  facets.o \
  global_settings.o \
  ledger.o \
  ledger_base.o \
  ledger_evaluator.o \
  ledger_invariant.o \
  ledger_test.o \
  ledger_text_formats.o \
  ledger_variant.o \
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  timer.o \
  xml_lmi.o \

loads_test$(EXEEXT): \
  $(common_test_objects) \
  loads.o \
  loads_test.o \
  timer.o \

map_lookup_test$(EXEEXT): \
  $(common_test_objects) \
  map_lookup_test.o \

materially_equal_test$(EXEEXT): \
  $(common_test_objects) \
  materially_equal_test.o \

math_functions_test$(EXEEXT): \
  $(common_test_objects) \
  math_functions_test.o \
  timer.o \

mc_enum_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  ce_product_name.o \
  datum_base.o \
  facets.o \
  global_settings.o \
  mc_enum.o \
  mc_enum_test.o \
  mc_enum_test_aux.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \

md5sum_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  md5.o \
  md5sum.o \
  md5sum_test.o \

miscellany_test$(EXEEXT): \
  $(common_test_objects) \
  miscellany.o \
  miscellany_test.o \

monnaie_test$(EXEEXT): \
  $(common_test_objects) \
  monnaie_test.o \
  timer.o \

mortality_rates_test$(EXEEXT): \
  $(common_test_objects) \
  ihs_mortal.o \
  mortality_rates_test.o \

name_value_pairs_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  global_settings.o \
  miscellany.o \
  name_value_pairs.o \
  name_value_pairs_test.o \
  null_stream.o \
  path_utility.o \

ncnnnpnn_test$(EXEEXT): \
  $(common_test_objects) \
  ncnnnpnn_test.o \

numeric_io_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  numeric_io_test.o \
  path_utility.o \
  timer.o \

path_utility_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  path_utility_test.o \

premium_tax_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  data_directory.o \
  database.o \
  datum_base.o \
  dbdict.o \
  dbnames.o \
  dbvalue.o \
  facets.o \
  global_settings.o \
  lmi.o \
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  my_proem.o \
  null_stream.o \
  path_utility.o \
  premium_tax.o \
  premium_tax_test.o \
  product_data.o \
  stratified_charges.o \
  xml_lmi.o \

print_matrix_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  cso_table.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  print_matrix_test.o \

product_file_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  data_directory.o \
  database.o \
  datum_base.o \
  dbdict.o \
  dbnames.o \
  dbvalue.o \
  facets.o \
  fund_data.o \
  global_settings.o \
  lingo.o \
  lmi.o \
  mc_enum.o \
  mc_enum_types.o \
  mc_enum_types_aux.o \
  miscellany.o \
  my_proem.o \
  null_stream.o \
  path_utility.o \
  premium_tax.o \
  product_data.o \
  product_file_test.o \
  rounding_rules.o \
  stratified_charges.o \
  timer.o \
  xml_lmi.o \

progress_meter_test$(EXEEXT): \
  $(common_test_objects) \
  null_stream.o \
  progress_meter.o \
  progress_meter_cli.o \
  progress_meter_test.o \
  timer.o \

rate_table_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  crc32.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  rate_table.o \
  rate_table_test.o \

regex_test$(EXEEXT): \
  $(boost_regex_objects) \
  $(common_test_objects) \
  regex_test.o \
  timer.o \

report_table_test$(EXEEXT): \
  $(common_test_objects) \
  report_table.o \
  report_table_test.o \

round_test$(EXEEXT): \
  $(common_test_objects) \
  round_glibc.o \
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

ssize_lmi_test$(EXEEXT): \
  $(common_test_objects) \
  ssize_lmi_test.o \

stratified_algorithms_test$(EXEEXT): \
  $(common_test_objects) \
  stratified_algorithms_test.o \

stream_cast_test$(EXEEXT): \
  $(common_test_objects) \
  facets.o \
  stream_cast_test.o \
  timer.o \

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
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  datum_base.o \
  facets.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  tn_range_test.o \
  tn_range_test_aux.o \

value_cast_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  facets.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  value_cast_test.o \

vector_test$(EXEEXT): \
  $(common_test_objects) \
  timer.o \
  vector_test.o \

wx_new_test$(EXEEXT): \
  $(common_test_objects) \
  wx_new_test.o \

xml_serialize_test$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(common_test_objects) \
  calendar_date.o \
  global_settings.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  timer.o \
  xml_lmi.o \
  xml_serialize_test.o \

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

# Copy this binary to the local bin directory, so that it's available
# for 'authenticity_test' and 'system_command_test'.
# MD5 !! Obviate this by rewriting those unit tests.
lmi_md5sum$(EXEEXT): POST_LINK_COMMAND = $(INSTALL) -m 0775 $@ $(localbindir)
lmi_md5sum$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(main_auxiliary_common_objects) \
  getopt.o \
  md5.o \
  md5sum.o \
  md5sum_cli.o \

# MD5 !! Remove "timer.o" below.
generate_passkey$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(main_auxiliary_common_objects) \
  authenticity.o \
  calendar_date.o \
  generate_passkey.o \
  global_settings.o \
  md5.o \
  md5sum.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  system_command.o \
  system_command_non_wx.o \
  timer.o \

ihs_crc_comp$(EXEEXT): \
  $(main_auxiliary_common_objects) \
  ihs_crc_comp.o \

rate_table_tool$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(main_auxiliary_common_objects) \
  calendar_date.o \
  crc32.o \
  getopt.o \
  global_settings.o \
  license.o \
  miscellany.o \
  null_stream.o \
  path_utility.o \
  rate_table.o \
  rate_table_tool.o \

test_coding_rules_test := PERFORM=$(PERFORM) $(srcdir)/test_coding_rules_test.sh
test_coding_rules$(EXEEXT): POST_LINK_COMMAND = $(test_coding_rules_test)
test_coding_rules$(EXEEXT): \
  $(boost_filesystem_objects) \
  $(boost_regex_objects) \
  $(main_auxiliary_common_objects) \
  miscellany.o \
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
  miscellany.o \
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
  my_lingo.o \
  my_prod.o \
  my_proem.o \
  my_rnd.o \
  my_tier.o \
  liblmi$(SHREXT) \

# This file does not end in backslash-newline.
