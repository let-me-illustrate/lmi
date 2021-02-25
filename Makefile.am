# Master automake makefile, run automake to generate Makefile.am from it (this
# is also done by autogen.sh script)
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Vadim Zeitlin.
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

##############################################################################
# Common compilation flags
##############################################################################

AM_CPPFLAGS = -I$(srcdir)/tools/pete-2.1.1

# On windows use auto import feature
if LMI_MSW
    AM_LDFLAGS=--enable-auto-import
else
    AM_LDFLAGS=
endif

ACLOCAL_AMFLAGS = -I aclocal

##############################################################################
# All Targets
##############################################################################

# targets going in bin
bin_PROGRAMS = \
    lmi_cli \
    lmi_wx \
    elapsed_time \
    lmi_md5sum \
    generate_passkey \
    antediluvian_cli \
    ihs_crc_comp \
    product_files \
    rate_table_tool \
    test_coding_rules \
    wx_test

# MD5 !! 'objects.make' copies lmi_md5sum$(EXEEXT) to $localbindir,
# which is not a standard autotools directory. This doesn't matter
# for production, because lmi itself no longer calls an external
# program to validate its data files; but its absence may cause some
# unit tests to fail.

# other programs which may or not be built depending on configure arguments
EXTRA_PROGRAMS = \
    antediluvian_cgi

# libtool libraries installed in lib
lib_LTLIBRARIES = \
    liblmi.la \
    libskeleton.la

# libtool convenience libraries which are not installed at all
noinst_LTLIBRARIES = \
    libantediluvian.la \
    libwx_new.la \
    libmain_auxiliary_common.la

# data files
xrcdir = $(pkgdatadir)
dist_xrc_DATA = $(shell ls -1 $(srcdir)/*.png $(srcdir)/*.xrc) \
    lmi.ico

xsldir = $(pkgdatadir)
dist_xsl_DATA = $(shell ls -1 $(srcdir)/*.xsl)

# CGI stuff
if LMI_WITH_CGI
    lib_LTLIBRARIES += libcgi.la
    bin_PROGRAMS += antediluvian_cgi
endif

# tests
TESTS = \
    test_account_value \
    test_actuarial_table \
    test_alert \
    test_any_member \
    test_assert_lmi \
    test_authenticity \
    test_bourn_cast \
    test_cache_file_reads \
    test_calendar_date \
    test_callback \
    test_comma_punct \
    test_commutation_functions \
    test_configurable_settings \
    test_contains \
    test_crc32 \
    test_currency \
    test_dbo_rules \
    test_expression_template_0 \
    test_fenv_lmi \
    test_file_command \
    test_financial \
    test_getopt \
    test_global_settings \
    test_gpt \
    test_handle_exceptions \
    test_ieee754 \
    test_input_seq \
    test_input \
    test_interpolate_string \
    test_irc7702_tables \
    test_irc7702a \
    test_istream_to_string \
    test_ledger \
    test_loads \
    test_map_lookup \
    test_materially_equal \
    test_math_functions \
    test_mc_enum \
    test_md5sum \
    test_miscellany \
    test_monnaie \
    test_mortality_rates \
    test_name_value_pairs \
    test_ncnnnpnn \
    test_numeric_io \
    test_path_utility \
    test_premium_tax \
    test_print_matrix \
    test_product_file \
    test_progress_meter \
    test_rate_table \
    test_regex \
    test_report_table \
    test_round \
    test_round_to \
    test_rtti_lmi \
    test_safely_dereference_as \
    test_sandbox \
    test_snprintf \
    test_ssize_lmi \
    test_stratified_algorithms \
    test_stream_cast \
    test_system_command \
    test_test_tools \
    test_timer \
    test_tn_range \
    test_value_cast \
    test_vector \
    test_wx_new \
    test_xml_serialize \
    test_zero

check_PROGRAMS = $(TESTS)

##############################################################################
# Targets definitions
##############################################################################

# wx GUI's files are in a library to be shared with the tests
libskeleton_la_SOURCES = \
    about_dialog.cpp \
    alert_wx.cpp \
    census_document.cpp \
    census_view.cpp \
    database_document.cpp \
    database_view.cpp \
    database_view_editor.cpp \
    default_view.cpp \
    docmanager_ex.cpp \
    file_command_wx.cpp \
    gpt_document.cpp \
    gpt_view.cpp \
    group_quote_pdf_gen_wx.cpp \
    icon_monger.cpp \
    illustration_document.cpp \
    illustration_view.cpp \
    input_sequence_entry.cpp \
    main_common.cpp \
    mec_document.cpp \
    mec_view.cpp \
    msw_workarounds.cpp \
    multidimgrid_any.cpp \
    multidimgrid_tools.cpp \
    mvc_controller.cpp \
    mvc_view.cpp \
    pdf_command_wx.cpp \
    pdf_writer_wx.cpp \
    policy_document.cpp \
    policy_view.cpp \
    preferences_view.cpp \
    previewframe_ex.cpp \
    product_editor.cpp \
    progress_meter_wx.cpp \
    rounding_document.cpp \
    rounding_view.cpp \
    rounding_view_editor.cpp \
    single_choice_popup_menu.cpp \
    skeleton.cpp \
    system_command_wx.cpp \
    text_doc.cpp \
    text_view.cpp \
    tier_document.cpp \
    tier_view.cpp \
    tier_view_editor.cpp \
    transferor.cpp \
    view_ex.cpp \
    wx_table_generator.cpp \
    wx_utility.cpp

# main program executables
lmi_wx_SOURCES = \
    main_wx.cpp

libskeleton_la_CXXFLAGS = \
     $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(XMLWRAPP_CFLAGS) $(GTK_CFLAGS)
libskeleton_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
libskeleton_la_LIBADD = \
    liblmi.la \
    libwx_new.la \
    $(XMLWRAPP_LIBS) \
    $(GTK_LIBS) \
    $(BOOST_LIBS) \
    $(WXPDFDOC_LIBS) \
    $(WX_LIBS)

lmi_wx_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(XMLWRAPP_CFLAGS) $(GTK_CFLAGS)
# $(CGICC_INCLUDE_FLAGS)

if LMI_MSW
## use old style suffix rule instead of the pattern one as automake only
## recognizes the old style rules
.rc.o:
	$(WINDRES) -I $(top_srcdir) $(WX_CPPFLAGS) $< $@
endif
lmi_wx_LDADD = \
    liblmi.la \
    libskeleton.la \
    libwx_new.la \
    $(XMLWRAPP_LIBS) \
    $(GTK_LIBS) \
    $(BOOST_LIBS) \
    $(WXPDFDOC_LIBS) \
    $(WX_LIBS)
if LMI_MSW
    lmi_wx_SOURCES += lmi_msw_res.rc
    lmi_wx_CXXFLAGS += -mwindows
endif

cli_sources = \
    alert_cli.cpp \
    file_command_cli.cpp \
    main_cli.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    progress_meter_cli.cpp \
    system_command_non_wx.cpp

lmi_cli_SOURCES = \
    $(cli_sources)
lmi_cli_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
lmi_cli_LDADD = \
    liblmi.la \
    $(XMLWRAPP_LIBS) \
    $(BOOST_LIBS)

wx_test_SOURCES = \
  main_wx_test.cpp \
  wx_test_about_version.cpp \
  wx_test_benchmark_census.cpp \
  wx_test_calculation_summary.cpp \
  wx_test_config_settings.cpp \
  wx_test_create_open.cpp \
  wx_test_default_input.cpp \
  wx_test_default_update.cpp \
  wx_test_expiry_dates.cpp \
  wx_test_input_sequences.cpp \
  wx_test_input_validation.cpp \
  wx_test_log_errors.cpp \
  wx_test_paste_census.cpp \
  wx_test_pdf_create.cpp \
  wx_test_validate_output.cpp
wx_test_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS)
wx_test_LDADD = \
    liblmi.la \
    libskeleton.la \
    libwx_new.la \
    $(XMLWRAPP_LIBS) \
    $(GTK_LIBS) \
    $(BOOST_LIBS) \
    $(WXPDFDOC_LIBS) \
    $(WX_LIBS)

# libraries

liblmi_common_sources = \
    actuarial_table.cpp \
    alert.cpp \
    calendar_date.cpp \
    ce_product_name.cpp \
    ce_skin_name.cpp \
    configurable_settings.cpp \
    crc32.cpp \
    custom_io_0.cpp \
    custom_io_1.cpp \
    data_directory.cpp \
    database.cpp \
    datum_base.cpp \
    datum_boolean.cpp \
    datum_sequence.cpp \
    datum_string.cpp \
    dbdict.cpp \
    dbnames.cpp \
    dbo_rules.cpp \
    dbvalue.cpp \
    death_benefits.cpp \
    emit_ledger.cpp \
    facets.cpp \
    fenv_guard.cpp \
    fenv_lmi.cpp \
    file_command.cpp \
    getopt.cpp \
    global_settings.cpp \
    group_values.cpp \
    group_quote_pdf_gen.cpp \
    html.cpp \
    illustrator.cpp \
    input.cpp \
    input_harmonization.cpp \
    input_realization.cpp \
    input_sequence.cpp \
    input_sequence_aux.cpp \
    input_sequence_parser.cpp \
    input_xml_io.cpp \
    interest_rates.cpp \
    interpolate_string.cpp \
    ledger.cpp \
    ledger_base.cpp \
    ledger_evaluator.cpp \
    ledger_invariant.cpp \
    ledger_invariant_init.cpp \
    ledger_pdf.cpp \
    ledger_text_formats.cpp \
    ledger_variant.cpp \
    ledger_variant_init.cpp \
    ledgervalues.cpp \
    license.cpp \
    loads.cpp \
    mc_enum.cpp \
    mc_enum_types.cpp \
    mc_enum_types_aux.cpp \
    miscellany.cpp \
    multiple_cell_document.cpp \
    mvc_model.cpp \
    my_proem.cpp \
    name_value_pairs.cpp \
    null_stream.cpp \
    outlay.cpp \
    path_utility.cpp \
    pdf_command.cpp \
    premium_tax.cpp \
    progress_meter.cpp \
    round_glibc.c \
    sigfpe.cpp \
    single_cell_document.cpp \
    system_command.cpp \
    timer.cpp \
    tn_range_types.cpp \
    unwind.cpp \
    xml_lmi.cpp \
    yare_input.cpp

libmain_auxiliary_common_la_SOURCES = \
    alert.cpp \
    alert_cli.cpp \
    fenv_lmi.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    sigfpe.cpp
libmain_auxiliary_common_la_CXXFLAGS = $(AM_CXXFLAGS)

libantediluvian_la_SOURCES = \
    accountvalue.cpp \
    antediluvian_stubs.cpp \
    basicvalues.cpp \
    mortality_rates.cpp \
    solve.cpp \
    $(liblmi_common_sources)
libantediluvian_la_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
libantediluvian_la_CFLAGS = $(AM_CFLAGS)
libantediluvian_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
libantediluvian_la_LIBADD = \
    $(BOOST_LIBS)

liblmi_la_SOURCES = \
    authenticity.cpp \
    basic_tables.cpp \
    commutation_functions.cpp \
    cso_table.cpp \
    financial.cpp \
    fund_data.cpp \
    gpt_input.cpp \
    gpt_server.cpp \
    gpt_specamt.cpp \
    gpt_state.cpp \
    gpt_xml_document.cpp \
    i7702.cpp \
    ihs_acctval.cpp \
    ihs_avdebug.cpp \
    ihs_avmly.cpp \
    ihs_avsolve.cpp \
    ihs_avstrtgy.cpp \
    ihs_basicval.cpp \
    ihs_irc7702.cpp \
    ihs_irc7702a.cpp \
    ihs_mortal.cpp \
    ihs_server7702.cpp \
    irc7702_tables.cpp \
    lingo.cpp \
    lmi.cpp \
    md5.cpp \
    md5sum.cpp \
    mec_input.cpp \
    mec_server.cpp \
    mec_state.cpp \
    mec_xml_document.cpp \
    mortality_rates_fetch.cpp \
    preferences_model.cpp \
    product_data.cpp \
    report_table.cpp \
    rounding_rules.cpp \
    stratified_algorithms.cpp \
    stratified_charges.cpp \
    verify_products.cpp \
    $(liblmi_common_sources)
liblmi_la_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
liblmi_la_CFLAGS = $(AM_CFLAGS)
liblmi_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
liblmi_la_LIBADD = \
    $(XMLWRAPP_LIBS) \
    $(BOOST_LIBS)

libcgi_la_SOURCES = \
    alert_cgi.cpp \
    file_command_cgi.cpp \
    main_cgi.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    progress_meter_cgi.cpp \
    system_command_non_wx.cpp

libcgi_la_CXXFLAGS = $(BOOST_INCLUDE_FLAGS) $(CGICC_INCLUDE_FLAGS) $(AM_CXXFLAGS)
libcgi_la_LIBADD = $(CGICC_LIBS)

libwx_new_la_SOURCES = wx_new.cpp
libwx_new_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
libwx_new_la_LIBADD = $(WX_LIBS)

# auxiliary executables

lmi_md5sum_SOURCES = \
    getopt.cpp \
    md5.cpp \
    md5sum.cpp \
    md5sum_cli.cpp
lmi_md5sum_CXXFLAGS = $(AM_CXXFLAGS) $(BOOST_INCLUDE_FLAGS)
lmi_md5sum_LDADD = \
    $(BOOST_LIBS) \
    libmain_auxiliary_common.la

# MD5 !! Remove "timer.cpp" below.
generate_passkey_SOURCES = \
    authenticity.cpp \
    calendar_date.cpp \
    generate_passkey.cpp \
    global_settings.cpp \
    md5.cpp \
    md5sum.cpp \
    miscellany.cpp \
    null_stream.cpp \
    path_utility.cpp \
    system_command.cpp \
    system_command_non_wx.cpp \
    timer.cpp
generate_passkey_CXXFLAGS = $(AM_CXXFLAGS)
generate_passkey_LDADD = \
    $(BOOST_LIBS) \
    libmain_auxiliary_common.la

elapsed_time_SOURCES = \
    elapsed_time.cpp \
    system_command.cpp \
    system_command_non_wx.cpp \
    timer.cpp
elapsed_time_CXXFLAGS = $(AM_CXXFLAGS)
elapsed_time_LDADD = \
    libmain_auxiliary_common.la

antediluvian_cgi_CXXFLAGS = $(AM_CXXFLAGS) $(BOOST_INCLUDE_FLAGS)
antediluvian_cgi_LDADD = \
    libcgi.la \
    libantediluvian.la \
    $(CGICC_LIBS) \
    $(BOOST_LIBS)

antediluvian_cli_SOURCES = \
    $(cli_sources)
antediluvian_cli_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
antediluvian_cli_LDADD = \
    libantediluvian.la \
    liblmi.la \
    $(BOOST_LIBS) \
    $(XMLWRAPP_LIBS)

ihs_crc_comp_SOURCES = ihs_crc_comp.cpp
ihs_crc_comp_LDADD = libmain_auxiliary_common.la

product_files_SOURCES = \
    alert_cli.cpp \
    generate_product_files.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    my_db.cpp \
    my_fund.cpp \
    my_lingo.cpp \
    my_prod.cpp \
    my_proem.cpp \
    my_rnd.cpp \
    my_tier.cpp
product_files_LDADD = \
    liblmi.la \
    $(BOOST_LIBS) \
    $(XMLWRAPP_LIBS)

rate_table_tool_SOURCES = \
    alert_cli.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    rate_table.cpp \
    rate_table_tool.cpp
rate_table_tool_LDADD = \
    liblmi.la \
    $(BOOST_LIBS) \
    $(XMLWRAPP_LIBS)

test_coding_rules_SOURCES = \
  alert.cpp \
  alert_cli.cpp \
  fenv_lmi.cpp \
  main_common.cpp \
  main_common_non_wx.cpp \
  miscellany.cpp \
  my_test_coding_rules.cpp \
  sigfpe.cpp \
  test_coding_rules.cpp
test_coding_rules_CXXFLAGS = $(AM_CXXFLAGS) $(BOOST_INCLUDE_FLAGS)
test_coding_rules_LDADD = \
  libmain_auxiliary_common.la \
  $(BOOST_LIBS)

# unit tests
common_test_objects = \
    alert.cpp \
    alert_cli.cpp \
    fenv_lmi.cpp \
    getopt.cpp \
    license.cpp \
    unwind.cpp

test_account_value_SOURCES = \
  $(common_test_objects) \
  account_value_test.cpp
test_account_value_CXXFLAGS = $(AM_CXXFLAGS)

test_actuarial_table_SOURCES = \
  $(common_test_objects) \
  actuarial_table.cpp \
  actuarial_table_test.cpp \
  cso_table.cpp \
  timer.cpp \
  xml_lmi.cpp
test_actuarial_table_CXXFLAGS = $(AM_CXXFLAGS)
test_actuarial_table_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_alert_SOURCES = \
  $(common_test_objects) \
  alert_test.cpp
test_alert_CXXFLAGS = $(AM_CXXFLAGS)

test_any_member_SOURCES = \
  $(common_test_objects) \
  any_member_test.cpp \
  calendar_date.cpp \
  facets.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp
test_any_member_CXXFLAGS = $(AM_CXXFLAGS)
test_any_member_LDADD = \
  $(BOOST_LIBS)

test_assert_lmi_SOURCES = \
  $(common_test_objects) \
  assert_lmi_test.cpp
test_assert_lmi_CXXFLAGS = $(AM_CXXFLAGS)

# MD5 !! Remove "timer.cpp" below.
test_authenticity_SOURCES = \
  $(common_test_objects) \
  authenticity.cpp \
  authenticity_test.cpp \
  calendar_date.cpp \
  global_settings.cpp \
  md5.cpp \
  md5sum.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  system_command.cpp \
  system_command_non_wx.cpp \
  timer.cpp
test_authenticity_CXXFLAGS = $(AM_CXXFLAGS)
test_authenticity_LDADD = \
  $(BOOST_LIBS)

test_bourn_cast_SOURCES = \
  $(common_test_objects) \
  bourn_cast_test.cpp \
  timer.cpp
test_bourn_cast_CXXFLAGS = $(AM_CXXFLAGS)

test_cache_file_reads_SOURCES = \
  $(common_test_objects) \
  cache_file_reads_test.cpp \
  timer.cpp
test_cache_file_reads_CXXFLAGS = $(AM_CXXFLAGS)
test_cache_file_reads_LDADD = \
  $(BOOST_LIBS)

test_calendar_date_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  calendar_date_test.cpp \
  null_stream.cpp \
  timer.cpp
test_calendar_date_CXXFLAGS = $(AM_CXXFLAGS)

test_callback_SOURCES = \
  $(common_test_objects) \
  callback_test.cpp
test_callback_CXXFLAGS = $(AM_CXXFLAGS)

test_comma_punct_SOURCES = \
  $(common_test_objects) \
  comma_punct_test.cpp
test_comma_punct_CXXFLAGS = $(AM_CXXFLAGS)

test_commutation_functions_SOURCES = \
  $(common_test_objects) \
  commutation_functions.cpp \
  commutation_functions_test.cpp \
  cso_table.cpp \
  timer.cpp
test_commutation_functions_CXXFLAGS = $(AM_CXXFLAGS)

test_configurable_settings_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  configurable_settings.cpp \
  configurable_settings_test.cpp \
  data_directory.cpp \
  datum_base.cpp \
  facets.cpp \
  global_settings.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  xml_lmi.cpp
test_configurable_settings_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
test_configurable_settings_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_contains_SOURCES = \
  $(common_test_objects) \
  contains_test.cpp
test_contains_CXXFLAGS = $(AM_CXXFLAGS)

test_crc32_SOURCES = \
  $(common_test_objects) \
  crc32.cpp \
  crc32_test.cpp
test_crc32_CXXFLAGS = $(AM_CXXFLAGS)

test_currency_SOURCES = \
  $(common_test_objects) \
  currency_test.cpp \
  timer.cpp
test_currency_CXXFLAGS = $(AM_CXXFLAGS)

test_dbo_rules_SOURCES = \
  $(common_test_objects) \
  datum_base.cpp \
  dbo_rules.cpp \
  dbo_rules_test.cpp \
  facets.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  timer.cpp
test_dbo_rules_CXXFLAGS = $(AM_CXXFLAGS)

test_expression_template_0_SOURCES = \
  $(common_test_objects) \
  expression_template_0_test.cpp \
  timer.cpp
test_expression_template_0_CXXFLAGS = $(AM_CXXFLAGS)

test_fenv_lmi_SOURCES = \
  $(common_test_objects) \
  fenv_guard.cpp \
  fenv_lmi_test.cpp
test_fenv_lmi_CXXFLAGS = $(AM_CXXFLAGS)

test_file_command_SOURCES = \
  $(common_test_objects) \
  file_command.cpp \
  file_command_cli.cpp \
  file_command_test.cpp
test_file_command_CXXFLAGS = $(AM_CXXFLAGS)

test_financial_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  financial.cpp \
  financial_test.cpp \
  null_stream.cpp \
  stratified_algorithms.cpp \
  timer.cpp
test_financial_CXXFLAGS = $(AM_CXXFLAGS)

test_getopt_SOURCES = \
  $(common_test_objects) \
  getopt_test.cpp
test_getopt_CXXFLAGS = $(AM_CXXFLAGS)

test_global_settings_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  global_settings.cpp \
  global_settings_test.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp
test_global_settings_CXXFLAGS = $(AM_CXXFLAGS)
test_global_settings_LDADD = \
  $(BOOST_LIBS)

test_gpt_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  commutation_functions.cpp \
  cso_table.cpp \
  global_settings.cpp \
  gpt_commutation_functions.cpp \
  gpt_test.cpp \
  ihs_irc7702.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  timer.cpp
test_gpt_CXXFLAGS = $(AM_CXXFLAGS)
test_gpt_LDADD = \
  $(BOOST_LIBS)

test_handle_exceptions_SOURCES = \
  $(common_test_objects) \
  handle_exceptions_test.cpp
test_handle_exceptions_CXXFLAGS = $(AM_CXXFLAGS)

test_ieee754_SOURCES = \
  $(common_test_objects) \
  ieee754_test.cpp
test_ieee754_CXXFLAGS = $(AM_CXXFLAGS)

test_input_seq_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  global_settings.cpp \
  input_sequence.cpp \
  input_sequence_parser.cpp \
  input_sequence_test.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp
test_input_seq_CXXFLAGS = $(AM_CXXFLAGS)
test_input_seq_LDADD = \
  $(BOOST_LIBS)

test_input_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  ce_product_name.cpp \
  configurable_settings.cpp \
  data_directory.cpp \
  database.cpp \
  datum_base.cpp \
  datum_sequence.cpp \
  datum_string.cpp \
  dbdict.cpp \
  dbnames.cpp \
  dbo_rules.cpp \
  dbvalue.cpp \
  facets.cpp \
  global_settings.cpp \
  input.cpp \
  input_harmonization.cpp \
  input_realization.cpp \
  input_sequence.cpp \
  input_sequence_aux.cpp \
  input_sequence_parser.cpp \
  input_test.cpp \
  input_xml_io.cpp \
  lmi.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  mc_enum_types_aux.cpp \
  miscellany.cpp \
  multiple_cell_document.cpp \
  mvc_model.cpp \
  my_proem.cpp \
  null_stream.cpp \
  path_utility.cpp \
  premium_tax.cpp \
  product_data.cpp \
  single_cell_document.cpp \
  stratified_charges.cpp \
  timer.cpp \
  tn_range_types.cpp \
  xml_lmi.cpp \
  yare_input.cpp
test_input_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
test_input_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_interpolate_string_SOURCES = \
  $(common_test_objects) \
  interpolate_string.cpp \
  interpolate_string_test.cpp
test_interpolate_string_CXXFLAGS = $(AM_CXXFLAGS)

test_irc7702_tables_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  commutation_functions.cpp \
  cso_table.cpp \
  global_settings.cpp \
  irc7702_tables.cpp \
  irc7702_tables_test.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp
test_irc7702_tables_CXXFLAGS = $(AM_CXXFLAGS)
test_irc7702_tables_LDADD = \
  $(BOOST_LIBS)

test_irc7702a_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  global_settings.cpp \
  ihs_irc7702a.cpp \
  irc7702a_test.cpp \
  mec_state.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  stratified_algorithms.cpp \
  xml_lmi.cpp
test_irc7702a_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
test_irc7702a_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_istream_to_string_SOURCES = \
  $(common_test_objects) \
  istream_to_string_test.cpp \
  timer.cpp
test_istream_to_string_CXXFLAGS = $(AM_CXXFLAGS)

test_ledger_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  configurable_settings.cpp \
  crc32.cpp \
  data_directory.cpp \
  datum_base.cpp \
  facets.cpp \
  global_settings.cpp \
  ledger.cpp \
  ledger_base.cpp \
  ledger_evaluator.cpp \
  ledger_invariant.cpp \
  ledger_test.cpp \
  ledger_text_formats.cpp \
  ledger_variant.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  mc_enum_types_aux.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  timer.cpp \
  xml_lmi.cpp
test_ledger_CXXFLAGS = $(AM_CXXFLAGS)
test_ledger_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_loads_SOURCES = \
  $(common_test_objects) \
  loads.cpp \
  loads_test.cpp \
  timer.cpp
test_loads_CXXFLAGS = $(AM_CXXFLAGS)

test_map_lookup_SOURCES = \
  $(common_test_objects) \
  map_lookup_test.cpp
test_map_lookup_CXXFLAGS = $(AM_CXXFLAGS)

test_materially_equal_SOURCES = \
  $(common_test_objects) \
  materially_equal_test.cpp
test_materially_equal_CXXFLAGS = $(AM_CXXFLAGS)

test_math_functions_SOURCES = \
  $(common_test_objects) \
  math_functions_test.cpp \
  timer.cpp
test_math_functions_CXXFLAGS = $(AM_CXXFLAGS)

test_mc_enum_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  ce_product_name.cpp \
  datum_base.cpp \
  facets.cpp \
  global_settings.cpp \
  mc_enum.cpp \
  mc_enum_test.cpp \
  mc_enum_test_aux.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp
test_mc_enum_CXXFLAGS = $(AM_CXXFLAGS)
test_mc_enum_LDADD = \
  $(BOOST_LIBS)

test_md5sum_SOURCES = \
  $(common_test_objects) \
  md5.cpp \
  md5sum.cpp \
  md5sum_test.cpp
test_md5sum_CXXFLAGS = $(AM_CXXFLAGS)
test_md5sum_LDADD = \
  $(BOOST_LIBS)

test_miscellany_SOURCES = \
  $(common_test_objects) \
  miscellany.cpp \
  miscellany_test.cpp
test_miscellany_CXXFLAGS = $(AM_CXXFLAGS)

test_monnaie_SOURCES = \
  $(common_test_objects) \
  monnaie_test.cpp \
  timer.cpp
test_monnaie_CXXFLAGS = $(AM_CXXFLAGS)

test_mortality_rates_SOURCES = \
  $(common_test_objects) \
  ihs_mortal.cpp \
  mortality_rates_test.cpp
test_mortality_rates_CXXFLAGS = $(AM_CXXFLAGS)

test_name_value_pairs_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  global_settings.cpp \
  miscellany.cpp \
  name_value_pairs.cpp \
  name_value_pairs_test.cpp \
  null_stream.cpp \
  path_utility.cpp
test_name_value_pairs_CXXFLAGS = $(AM_CXXFLAGS)
test_name_value_pairs_LDADD = \
  $(BOOST_LIBS)

test_ncnnnpnn_SOURCES = \
  $(common_test_objects) \
  ncnnnpnn_test.cpp
test_ncnnnpnn_CXXFLAGS = $(AM_CXXFLAGS)

test_numeric_io_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  numeric_io_test.cpp \
  path_utility.cpp \
  timer.cpp
test_numeric_io_CXXFLAGS = $(AM_CXXFLAGS)
test_numeric_io_LDADD = \
  $(BOOST_LIBS)

test_path_utility_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  path_utility_test.cpp
test_path_utility_CXXFLAGS = $(AM_CXXFLAGS)
test_path_utility_LDADD = \
  $(BOOST_LIBS)

test_premium_tax_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  data_directory.cpp \
  database.cpp \
  datum_base.cpp \
  dbdict.cpp \
  dbnames.cpp \
  dbvalue.cpp \
  facets.cpp \
  global_settings.cpp \
  lmi.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  mc_enum_types_aux.cpp \
  miscellany.cpp \
  my_proem.cpp \
  null_stream.cpp \
  path_utility.cpp \
  premium_tax.cpp \
  premium_tax_test.cpp \
  product_data.cpp \
  stratified_charges.cpp \
  xml_lmi.cpp
test_premium_tax_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
test_premium_tax_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_print_matrix_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  cso_table.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  print_matrix_test.cpp
test_print_matrix_CXXFLAGS = $(AM_CXXFLAGS)
test_print_matrix_LDADD = \
  $(BOOST_LIBS)

test_product_file_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  data_directory.cpp \
  database.cpp \
  datum_base.cpp \
  dbdict.cpp \
  dbnames.cpp \
  dbvalue.cpp \
  facets.cpp \
  fund_data.cpp \
  global_settings.cpp \
  lingo.cpp \
  lmi.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  mc_enum_types_aux.cpp \
  miscellany.cpp \
  my_proem.cpp \
  null_stream.cpp \
  path_utility.cpp \
  premium_tax.cpp \
  product_data.cpp \
  product_file_test.cpp \
  rounding_rules.cpp \
  stratified_charges.cpp \
  timer.cpp \
  xml_lmi.cpp
test_product_file_CXXFLAGS = $(AM_CXXFLAGS)
test_product_file_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_progress_meter_SOURCES = \
  $(common_test_objects) \
  null_stream.cpp \
  progress_meter.cpp \
  progress_meter_cli.cpp \
  progress_meter_test.cpp \
  timer.cpp
test_progress_meter_CXXFLAGS = $(AM_CXXFLAGS)

test_rate_table_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  crc32.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  rate_table.cpp \
  rate_table_test.cpp
test_rate_table_CXXFLAGS = $(AM_CXXFLAGS)
test_rate_table_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_regex_SOURCES = \
  $(common_test_objects) \
  regex_test.cpp \
  timer.cpp
test_regex_CXXFLAGS = $(AM_CXXFLAGS)
test_regex_LDADD = \
  $(BOOST_LIBS)

test_report_table_SOURCES = \
  $(common_test_objects) \
  report_table.cpp \
  report_table_test.cpp
test_report_table_CXXFLAGS = $(AM_CXXFLAGS)

test_round_SOURCES = \
  $(common_test_objects) \
  round_glibc.c \
  round_test.cpp
test_round_CXXFLAGS = $(AM_CXXFLAGS)

test_round_to_SOURCES = \
  $(common_test_objects) \
  round_to_test.cpp
test_round_to_CXXFLAGS = $(AM_CXXFLAGS)

test_rtti_lmi_SOURCES = \
  $(common_test_objects) \
  rtti_lmi_test.cpp
test_rtti_lmi_CXXFLAGS = $(AM_CXXFLAGS)

test_safely_dereference_as_SOURCES = \
  $(common_test_objects) \
  safely_dereference_as_test.cpp
test_safely_dereference_as_CXXFLAGS = $(AM_CXXFLAGS)

test_sandbox_SOURCES = \
  $(common_test_objects) \
  sandbox_test.cpp
test_sandbox_CXXFLAGS = $(AM_CXXFLAGS)

test_snprintf_SOURCES = \
  $(common_test_objects) \
  snprintf_test.cpp
test_snprintf_CXXFLAGS = $(AM_CXXFLAGS)

test_ssize_lmi_SOURCES = \
  $(common_test_objects) \
  ssize_lmi_test.cpp
test_ssize_lmi_CXXFLAGS = $(AM_CXXFLAGS)

test_stratified_algorithms_SOURCES = \
  $(common_test_objects) \
  stratified_algorithms_test.cpp
test_stratified_algorithms_CXXFLAGS = $(AM_CXXFLAGS)

test_stream_cast_SOURCES = \
  $(common_test_objects) \
  facets.cpp \
  stream_cast_test.cpp \
  timer.cpp
test_stream_cast_CXXFLAGS = $(AM_CXXFLAGS)

test_system_command_SOURCES = \
  $(common_test_objects) \
  system_command.cpp \
  system_command_non_wx.cpp \
  system_command_test.cpp
test_system_command_CXXFLAGS = $(AM_CXXFLAGS)

test_test_tools_SOURCES = \
  $(common_test_objects) \
  test_tools_test.cpp
test_test_tools_CXXFLAGS = $(AM_CXXFLAGS)

test_timer_SOURCES = \
  $(common_test_objects) \
  timer.cpp \
  timer_test.cpp
test_timer_CXXFLAGS = $(AM_CXXFLAGS)

test_tn_range_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  datum_base.cpp \
  facets.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  tn_range_test.cpp \
  tn_range_test_aux.cpp
test_tn_range_CXXFLAGS = $(AM_CXXFLAGS)
test_tn_range_LDADD = \
  $(BOOST_LIBS)

test_value_cast_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  facets.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  value_cast_test.cpp
test_value_cast_CXXFLAGS = $(AM_CXXFLAGS)
test_value_cast_LDADD = \
  $(BOOST_LIBS)

test_vector_SOURCES = \
  $(common_test_objects) \
  timer.cpp \
  vector_test.cpp
test_vector_CXXFLAGS = $(AM_CXXFLAGS)

test_wx_new_SOURCES = \
  $(common_test_objects) \
  wx_new_test.cpp
test_wx_new_CXXFLAGS = $(AM_CXXFLAGS)

test_xml_serialize_SOURCES = \
  $(common_test_objects) \
  calendar_date.cpp \
  global_settings.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  timer.cpp \
  xml_lmi.cpp \
  xml_serialize_test.cpp
test_xml_serialize_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
test_xml_serialize_LDADD = \
  $(BOOST_LIBS) \
  $(XMLWRAPP_LIBS)

test_zero_SOURCES = \
  $(common_test_objects) \
  null_stream.cpp \
  zero_test.cpp
test_zero_CXXFLAGS = $(AM_CXXFLAGS)

# headers: we need to include them or they wouldn't appear in the distribution
# this list should contain everything in `ls *.h *.hpp *.tpp *.xpp`
noinst_HEADERS = \
    about_dialog.hpp \
    account_value.hpp \
    actuarial_table.hpp \
    alert.hpp \
    any_entity.hpp \
    any_member.hpp \
    assert_lmi.hpp \
    authenticity.hpp \
    basic_tables.hpp \
    basic_values.hpp \
    boost_regex.hpp \
    bourn_cast.hpp \
    cache_file_reads.hpp \
    calendar_date.hpp \
    callback.hpp \
    catch_exceptions.hpp \
    ce_product_name.hpp \
    ce_skin_name.hpp \
    census_document.hpp \
    census_view.hpp \
    comma_punct.hpp \
    commutation_functions.hpp \
    config.hpp \
    config_bc551.hpp \
    config_como_mingw.hpp \
    config_ming323.hpp \
    configurable_settings.hpp \
    contains.hpp \
    crc32.hpp \
    cso_table.hpp \
    currency.hpp \
    custom_io_0.hpp \
    custom_io_1.hpp \
    data_directory.hpp \
    database.hpp \
    database_document.hpp \
    database_view.hpp \
    database_view_editor.hpp \
    datum_base.hpp \
    datum_boolean.hpp \
    datum_sequence.hpp \
    datum_string.hpp \
    dbdict.hpp \
    dbindex.hpp \
    dbnames.hpp \
    dbnames.xpp \
    dbvalue.hpp \
    death_benefits.hpp \
    default_view.hpp \
    deserialize_cast.hpp \
    docmanager_ex.hpp \
    edit_mvc_docview_parameters.hpp \
    emit_ledger.hpp \
    exit_codes.hpp \
    facets.hpp \
    fenv_guard.hpp \
    fenv_lmi.hpp \
    fenv_lmi_x86.hpp \
    file_command.hpp \
    financial.hpp \
    force_linking.hpp \
    fund_data.hpp \
    getopt.hpp \
    global_settings.hpp \
    gpt_commutation_functions.hpp \
    gpt_document.hpp \
    gpt_input.hpp \
    gpt_server.hpp \
    gpt_specamt.hpp \
    gpt_state.hpp \
    gpt_view.hpp \
    gpt_xml_document.hpp \
    group_quote_pdf_gen.hpp \
    group_values.hpp \
    handle_exceptions.hpp \
    html.hpp \
    i7702.hpp \
    icon_monger.hpp \
    ieee754.hpp \
    ihs_irc7702.hpp \
    ihs_irc7702a.hpp \
    ihs_server7702.hpp \
    illustration_document.hpp \
    illustration_view.hpp \
    illustrator.hpp \
    input.hpp \
    input_sequence.hpp \
    input_sequence_aux.hpp \
    input_sequence_entry.hpp \
    input_sequence_interval.hpp \
    input_sequence_parser.hpp \
    interest_rates.hpp \
    interpolate_string.hpp \
    irc7702_tables.hpp \
    istream_to_string.hpp \
    ledger.hpp \
    ledger_base.hpp \
    ledger_evaluator.hpp \
    ledger_invariant.hpp \
    ledger_pdf.hpp \
    ledger_text_formats.hpp \
    ledger_variant.hpp \
    ledgervalues.hpp \
    license.hpp \
    lingo.hpp \
    lmi.hpp \
    loads.hpp \
    loads_impl.hpp \
    main_common.hpp \
    map_lookup.hpp \
    materially_equal.hpp \
    math_functions.hpp \
    mc_enum.hpp \
    mc_enum.tpp \
    mc_enum_aux.hpp \
    mc_enum_fwd.hpp \
    mc_enum_metadata.hpp \
    mc_enum_test_aux.hpp \
    mc_enum_test_aux_enums.hpp \
    mc_enum_type_enums.hpp \
    mc_enum_types.hpp \
    mc_enum_types.xpp \
    mc_enum_types_aux.hpp \
    md5sum.hpp \
    md5.hpp \
    mec_document.hpp \
    mec_input.hpp \
    mec_server.hpp \
    mec_state.hpp \
    mec_view.hpp \
    mec_xml_document.hpp \
    miscellany.hpp \
    monnaie.hpp \
    mortality_rates.hpp \
    msw_workarounds.hpp \
    multidimgrid_any.hpp \
    multidimgrid_safe.hpp \
    multidimgrid_safe.tpp \
    multidimgrid_tools.hpp \
    multiple_cell_document.hpp \
    mvc_controller.hpp \
    mvc_controller.tpp \
    mvc_model.hpp \
    mvc_view.hpp \
    my_proem.hpp \
    name_value_pairs.hpp \
    ncnnnpnn.hpp \
    null_stream.hpp \
    numeric_io_cast.hpp \
    numeric_io_traits.hpp \
    oecumenic_enumerations.hpp \
    outlay.hpp \
    path_utility.hpp \
    pchfile.hpp \
    pdf_command.hpp \
    platform_dependent.hpp \
    policy_document.hpp \
    policy_view.hpp \
    preferences_model.hpp \
    preferences_view.hpp \
    premium_tax.hpp \
    previewframe_ex.hpp \
    print_matrix.hpp \
    product_data.hpp \
    product_editor.hpp \
    progress_meter.hpp \
    report_table.hpp \
    round_to.hpp \
    rounding_document.hpp \
    rounding_rules.hpp \
    rounding_view.hpp \
    rounding_view_editor.hpp \
    rtti_lmi.hpp \
    safely_dereference_as.hpp \
    sample.hpp \
    sigfpe.hpp \
    single_cell_document.hpp \
    single_choice_popup_menu.hpp \
    skeleton.hpp \
    so_attributes.hpp \
    ssize_lmi.hpp \
    stl_extensions.hpp \
    stratified_algorithms.hpp \
    stratified_charges.hpp \
    stratified_charges.xpp \
    stream_cast.hpp \
    system_command.hpp \
    test_tools.hpp \
    text_doc.hpp \
    text_view.hpp \
    tier_document.hpp \
    tier_view.hpp \
    tier_view_editor.hpp \
    timer.hpp \
    tn_range.hpp \
    tn_range.tpp \
    tn_range_fwd.hpp \
    tn_range_test_aux.hpp \
    tn_range_type_trammels.hpp \
    tn_range_types.hpp \
    transferor.hpp \
    unwind.hpp \
    value_cast.hpp \
    verify_products.hpp \
    version.hpp \
    view_ex.hpp \
    view_ex.tpp \
    wx_checks.hpp \
    wx_new.hpp \
    wx_table_generator.hpp \
    wx_test_case.hpp \
    wx_test_date.hpp \
    wx_test_document.hpp \
    wx_test_new.hpp \
    wx_test_output.hpp \
    wx_test_output_pdf.hpp \
    wx_test_statusbar.hpp \
    wx_utility.hpp \
    wx_workarounds.hpp \
    xml_lmi.hpp \
    xml_lmi_fwd.hpp \
    xml_serializable.hpp \
    xml_serializable.tpp \
    xml_serialize.hpp \
    yare_input.hpp \
    zero.hpp
