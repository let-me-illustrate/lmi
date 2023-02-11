# Master automake makefile, run automake to generate Makefile.am from it (this
# is also done by autogen.sh script)
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Vadim Zeitlin.
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

AM_DEFAULT_SOURCE_EXT = .cpp

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
    wx_test

if !LMI_MSW
  bin_PROGRAMS += test_coding_rules
endif

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
    libmain_auxiliary_common.la \
    libtest_common.la

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
    account_value_test \
    actuarial_table_test \
    alert_test \
    any_member_test \
    archetype_test \
    assert_lmi_test \
    authenticity_test \
    bin_exp_test \
    bourn_cast_test \
    cache_file_reads_test \
    calendar_date_test \
    callback_test \
    comma_punct_test \
    commutation_functions_test \
    configurable_settings_test \
    contains_test \
    crc32_test \
    crtp_base_test \
    currency_test \
    dbo_rules_test \
    duff_fmt_test \
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
    input_seq_test \
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
    null_stream_test \
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
    smf_test \
    snprintf_test \
    ssize_lmi_test \
    stratified_algorithms_test \
    stream_cast_test \
    system_command_test \
    test_tools_test \
    timer_test \
    tn_range_test \
    ul_utilities_test \
    value_cast_test \
    vector_test \
    wx_new_test \
    xml_serialize_test \
    zero_test

check_PROGRAMS = $(TESTS)

# MD5 !! Two unit tests, 'authenticity_test' and 'system_command_test',
# rely on finding an 'lmi_md5sum' binary in PATH, where it's found only
# by the time the test runs, so ensure it can be found there. But this
# should be removed because lmi no longer uses that program.
AM_TESTS_ENVIRONMENT = PATH=.:$$PATH

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
    $(XMLWRAPP_LIBS)

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
    fdlibm_expm1.c \
    fdlibm_log1p.c \
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
    math_functions.cpp \
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
    sigfpe.cpp \
    unwind.cpp

# Note: apparently useless setting of target_CXXFLAGS to its default value of
# AM_CXXFLAGS is required for its side effect, as doing this forces automake to
# prefix the name of the object file with "target", which is necessary to avoid
# the clash between the same files compiled as part of different targets.
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

liblmi_la_SOURCES = \
    authenticity.cpp \
    basic_tables.cpp \
    commutation_functions.cpp \
    cso_table.cpp \
    fund_data.cpp \
    gpt7702.cpp \
    gpt_commutation_functions.cpp \
    gpt_input.cpp \
    gpt_server.cpp \
    gpt_specamt.cpp \
    gpt_state.cpp \
    gpt_xml_document.cpp \
    i7702.cpp \
    i7702_init.cpp \
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
    ul_utilities.cpp \
    verify_products.cpp \
    $(liblmi_common_sources)
liblmi_la_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
liblmi_la_CFLAGS = $(AM_CFLAGS)
liblmi_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
liblmi_la_LIBADD = \
    $(XMLWRAPP_LIBS)

libcgi_la_SOURCES = \
    alert_cgi.cpp \
    file_command_cgi.cpp \
    main_cgi.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    progress_meter_cgi.cpp \
    system_command_non_wx.cpp

libcgi_la_CXXFLAGS = $(CGICC_INCLUDE_FLAGS) $(AM_CXXFLAGS)
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
lmi_md5sum_CXXFLAGS = $(AM_CXXFLAGS)
lmi_md5sum_LDADD = \
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
    libmain_auxiliary_common.la

elapsed_time_SOURCES = \
    elapsed_time.cpp \
    system_command.cpp \
    system_command_non_wx.cpp \
    timer.cpp
elapsed_time_CXXFLAGS = $(AM_CXXFLAGS)
elapsed_time_LDADD = \
    libmain_auxiliary_common.la

antediluvian_cgi_CXXFLAGS = $(AM_CXXFLAGS)
antediluvian_cgi_LDADD = \
    libcgi.la \
    libantediluvian.la \
    $(CGICC_LIBS)

antediluvian_cli_SOURCES = \
    $(cli_sources)
antediluvian_cli_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
antediluvian_cli_LDADD = \
    libantediluvian.la \
    liblmi.la \
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
    $(XMLWRAPP_LIBS)

rate_table_tool_SOURCES = \
    alert_cli.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    rate_table.cpp \
    rate_table_tool.cpp
rate_table_tool_LDADD = \
    liblmi.la \
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
test_coding_rules_CXXFLAGS = $(AM_CXXFLAGS) $(PCRE2_CFLAGS)
test_coding_rules_LDADD = \
  libmain_auxiliary_common.la \
  $(PCRE2_LIBS)

# unit tests
libtest_common_la_SOURCES = \
  alert.cpp \
  alert_cli.cpp \
  calendar_date.cpp \
  facets.cpp \
  fenv_lmi.cpp \
  global_settings.cpp \
  getopt.cpp \
  license.cpp \
  miscellany.cpp \
  null_stream.cpp \
  path_utility.cpp \
  timer.cpp \
  unwind.cpp
libtest_common_la_CXXFLAGS = $(AM_CXXFLAGS)

account_value_test_LDADD = \
  libtest_common.la

actuarial_table_test_SOURCES = \
  actuarial_table.cpp \
  actuarial_table_test.cpp \
  cso_table.cpp \
  xml_lmi.cpp
actuarial_table_test_CXXFLAGS = $(AM_CXXFLAGS)
actuarial_table_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

alert_test_LDADD = \
  libtest_common.la

any_member_test_LDADD = \
  libtest_common.la

archetype_test_SOURCES = \
  archetype.cpp \
  archetype_test.cpp
archetype_test_CXXFLAGS = $(AM_CXXFLAGS)
archetype_test_LDADD = \
  libtest_common.la

assert_lmi_test_LDADD = \
  libtest_common.la

authenticity_test_SOURCES = \
  authenticity.cpp \
  authenticity_test.cpp \
  md5.cpp \
  md5sum.cpp \
  system_command.cpp \
  system_command_non_wx.cpp
authenticity_test_CXXFLAGS = $(AM_CXXFLAGS)
authenticity_test_LDADD = \
  libtest_common.la

bin_exp_test_SOURCES = \
  bin_exp.cpp \
  bin_exp_test.cpp
bin_exp_test_CXXFLAGS = $(AM_CXXFLAGS)
bin_exp_test_LDADD = \
  libtest_common.la

bourn_cast_test_LDADD = \
  libtest_common.la

cache_file_reads_test_LDADD = \
  libtest_common.la

calendar_date_test_LDADD = \
  libtest_common.la

callback_test_LDADD = \
  libtest_common.la

comma_punct_test_LDADD = \
  libtest_common.la

commutation_functions_test_SOURCES = \
  commutation_functions.cpp \
  commutation_functions_test.cpp \
  cso_table.cpp \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  math_functions.cpp
commutation_functions_test_CXXFLAGS = $(AM_CXXFLAGS)
commutation_functions_test_LDADD = \
  libtest_common.la

configurable_settings_test_SOURCES = \
  configurable_settings.cpp \
  configurable_settings_test.cpp \
  data_directory.cpp \
  datum_base.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  xml_lmi.cpp
configurable_settings_test_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
configurable_settings_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

contains_test_LDADD = \
  libtest_common.la

crc32_test_SOURCES = \
  crc32.cpp \
  crc32_test.cpp
crc32_test_CXXFLAGS = $(AM_CXXFLAGS)
crc32_test_LDADD = \
  libtest_common.la

crtp_base_test_SOURCES = \
  crtp_base_test.cpp
crtp_base_test_CXXFLAGS = $(AM_CXXFLAGS)
crtp_base_test_LDADD = \
  libtest_common.la

currency_test_LDADD = \
  libtest_common.la

dbo_rules_test_SOURCES = \
  datum_base.cpp \
  dbo_rules.cpp \
  dbo_rules_test.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp
dbo_rules_test_CXXFLAGS = $(AM_CXXFLAGS)
dbo_rules_test_LDADD = \
  libtest_common.la

duff_fmt_test_LDADD = \
  libtest_common.la

et_vector_test_LDADD = \
  libtest_common.la

# This test uses uBLAS which requires std::allocator members removed in C++20,
# so ask to explicitly re-enable them when using libc++ (defining this symbol
# should be harmless otherwise, so don't bother making it conditional).
expression_template_0_test_CXXFLAGS = $(AM_CXXFLAGS) -D_LIBCPP_ENABLE_CXX20_REMOVED_ALLOCATOR_MEMBERS
expression_template_0_test_LDADD = \
  libtest_common.la

fenv_lmi_test_SOURCES = \
  fenv_guard.cpp \
  fenv_lmi_test.cpp
fenv_lmi_test_CXXFLAGS = $(AM_CXXFLAGS)
fenv_lmi_test_LDADD = \
  libtest_common.la

file_command_test_SOURCES = \
  file_command.cpp \
  file_command_cli.cpp \
  file_command_test.cpp
file_command_test_CXXFLAGS = $(AM_CXXFLAGS)
file_command_test_LDADD = \
  libtest_common.la

financial_test_SOURCES = \
  financial_test.cpp
financial_test_CXXFLAGS = $(AM_CXXFLAGS)
financial_test_LDADD = \
  libtest_common.la

getopt_test_LDADD = \
  libtest_common.la

global_settings_test_LDADD = \
  libtest_common.la

gpt_cf_triad_test_SOURCES = \
  commutation_functions.cpp \
  cso_table.cpp \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  gpt_cf_triad_test.cpp \
  gpt_commutation_functions.cpp \
  ihs_irc7702.cpp \
  math_functions.cpp
gpt_cf_triad_test_CXXFLAGS = $(AM_CXXFLAGS)
gpt_cf_triad_test_LDADD = \
  libtest_common.la

gpt_test_SOURCES = \
  commutation_functions.cpp \
  cso_table.cpp \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  gpt7702.cpp \
  gpt_commutation_functions.cpp \
  gpt_test.cpp \
  math_functions.cpp
gpt_test_CXXFLAGS = $(AM_CXXFLAGS)
gpt_test_LDADD = \
  libtest_common.la

handle_exceptions_test_LDADD = \
  libtest_common.la

i7702_test_SOURCES = \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  i7702.cpp \
  i7702_test.cpp \
  math_functions.cpp
i7702_test_CXXFLAGS = $(AM_CXXFLAGS)
i7702_test_LDADD = \
  libtest_common.la

ieee754_test_LDADD = \
  libtest_common.la

input_seq_test_SOURCES = \
  input_sequence.cpp \
  input_sequence_parser.cpp \
  input_sequence_test.cpp
input_seq_test_CXXFLAGS = $(AM_CXXFLAGS)
input_seq_test_LDADD = \
  libtest_common.la

input_test_SOURCES = \
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
  multiple_cell_document.cpp \
  mvc_model.cpp \
  my_proem.cpp \
  premium_tax.cpp \
  product_data.cpp \
  single_cell_document.cpp \
  stratified_charges.cpp \
  tn_range_types.cpp \
  xml_lmi.cpp \
  yare_input.cpp
input_test_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
input_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

interpolate_string_test_SOURCES = \
  interpolate_string.cpp \
  interpolate_string_test.cpp
interpolate_string_test_CXXFLAGS = $(AM_CXXFLAGS)
interpolate_string_test_LDADD = \
  libtest_common.la

irc7702_tables_test_SOURCES = \
  commutation_functions.cpp \
  cso_table.cpp \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  irc7702_tables.cpp \
  irc7702_tables_test.cpp \
  math_functions.cpp
irc7702_tables_test_CXXFLAGS = $(AM_CXXFLAGS)
irc7702_tables_test_LDADD = \
  libtest_common.la

irc7702a_test_SOURCES = \
  ihs_irc7702a.cpp \
  irc7702a_test.cpp \
  mec_state.cpp \
  stratified_algorithms.cpp \
  xml_lmi.cpp
irc7702a_test_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
irc7702a_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

istream_to_string_test_LDADD = \
  libtest_common.la

ledger_test_SOURCES = \
  configurable_settings.cpp \
  crc32.cpp \
  data_directory.cpp \
  datum_base.cpp \
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
  xml_lmi.cpp
ledger_test_CXXFLAGS = $(AM_CXXFLAGS)
ledger_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

loads_test_SOURCES = \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  loads.cpp \
  loads_test.cpp \
  math_functions.cpp
loads_test_CXXFLAGS = $(AM_CXXFLAGS)
loads_test_LDADD = \
  libtest_common.la

map_lookup_test_LDADD = \
  libtest_common.la

materially_equal_test_LDADD = \
  libtest_common.la

math_functions_test_SOURCES = \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  math_functions.cpp \
  math_functions_test.cpp
math_functions_test_CXXFLAGS = $(AM_CXXFLAGS)
math_functions_test_LDADD = \
  libtest_common.la

mc_enum_test_SOURCES = \
  ce_product_name.cpp \
  datum_base.cpp \
  mc_enum.cpp \
  mc_enum_test.cpp \
  mc_enum_test_aux.cpp
mc_enum_test_CXXFLAGS = $(AM_CXXFLAGS)
mc_enum_test_LDADD = \
  libtest_common.la

md5sum_test_SOURCES = \
  md5.cpp \
  md5sum.cpp \
  md5sum_test.cpp
md5sum_test_CXXFLAGS = $(AM_CXXFLAGS)
md5sum_test_LDADD = \
  libtest_common.la

miscellany_test_LDADD = \
  libtest_common.la

monnaie_test_LDADD = \
  libtest_common.la

mortality_rates_test_SOURCES = \
  fdlibm_expm1.c \
  fdlibm_log1p.c \
  ihs_mortal.cpp \
  math_functions.cpp \
  mortality_rates_test.cpp
mortality_rates_test_CXXFLAGS = $(AM_CXXFLAGS)
mortality_rates_test_LDADD = \
  libtest_common.la

name_value_pairs_test_SOURCES = \
  name_value_pairs.cpp \
  name_value_pairs_test.cpp
name_value_pairs_test_CXXFLAGS = $(AM_CXXFLAGS)
name_value_pairs_test_LDADD = \
  libtest_common.la

null_stream_test_LDADD = \
  libtest_common.la

numeric_io_test_LDADD = \
  libtest_common.la

path_utility_test_SOURCES = \
    path_utility_test.cpp \
    wine_workarounds.cpp
path_utility_test_LDADD = \
  libtest_common.la

premium_tax_test_SOURCES = \
  data_directory.cpp \
  database.cpp \
  datum_base.cpp \
  dbdict.cpp \
  dbnames.cpp \
  dbvalue.cpp \
  lmi.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  mc_enum_types_aux.cpp \
  my_proem.cpp \
  premium_tax.cpp \
  premium_tax_test.cpp \
  product_data.cpp \
  stratified_charges.cpp \
  xml_lmi.cpp
premium_tax_test_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
premium_tax_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

print_matrix_test_SOURCES = \
  cso_table.cpp \
  print_matrix_test.cpp
print_matrix_test_CXXFLAGS = $(AM_CXXFLAGS)
print_matrix_test_LDADD = \
  libtest_common.la

product_file_test_SOURCES = \
  data_directory.cpp \
  database.cpp \
  datum_base.cpp \
  dbdict.cpp \
  dbnames.cpp \
  dbvalue.cpp \
  fund_data.cpp \
  lingo.cpp \
  lmi.cpp \
  mc_enum.cpp \
  mc_enum_types.cpp \
  mc_enum_types_aux.cpp \
  my_proem.cpp \
  premium_tax.cpp \
  product_data.cpp \
  product_file_test.cpp \
  rounding_rules.cpp \
  stratified_charges.cpp \
  xml_lmi.cpp
product_file_test_CXXFLAGS = $(AM_CXXFLAGS)
product_file_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

progress_meter_test_SOURCES = \
  progress_meter.cpp \
  progress_meter_cli.cpp \
  progress_meter_test.cpp
progress_meter_test_CXXFLAGS = $(AM_CXXFLAGS)
progress_meter_test_LDADD = \
  libtest_common.la

rate_table_test_SOURCES = \
  crc32.cpp \
  rate_table.cpp \
  rate_table_test.cpp
rate_table_test_CXXFLAGS = $(AM_CXXFLAGS)
rate_table_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

regex_test_LDADD = \
  libtest_common.la \
  $(PCRE2_LIBS)

report_table_test_SOURCES = \
  report_table.cpp \
  report_table_test.cpp
report_table_test_CXXFLAGS = $(AM_CXXFLAGS)
report_table_test_LDADD = \
  libtest_common.la

round_test_SOURCES = \
  round_glibc.c \
  round_test.cpp
round_test_CXXFLAGS = $(AM_CXXFLAGS)
round_test_LDADD = \
  libtest_common.la

round_to_test_LDADD = \
  libtest_common.la

rtti_lmi_test_LDADD = \
  libtest_common.la

safely_dereference_as_test_SOURCES = \
  safely_dereference_as_test.cpp
safely_dereference_as_test_CXXFLAGS = $(AM_CXXFLAGS)
safely_dereference_as_test_LDADD = \
  libtest_common.la

sandbox_test_LDADD = \
  libtest_common.la

smf_test_SOURCES = \
  smf_test.cpp
smf_test_CXXFLAGS = $(AM_CXXFLAGS)
smf_test_LDADD = \
  libtest_common.la

snprintf_test_LDADD =\
  libtest_common.la

ssize_lmi_test_LDADD = \
  libtest_common.la

stratified_algorithms_test_SOURCES = \
  stratified_algorithms_test.cpp
stratified_algorithms_test_CXXFLAGS = $(AM_CXXFLAGS)
stratified_algorithms_test_LDADD = \
  libtest_common.la

stream_cast_test_LDADD = \
  libtest_common.la

system_command_test_SOURCES = \
  system_command.cpp \
  system_command_non_wx.cpp \
  system_command_test.cpp
system_command_test_CXXFLAGS = $(AM_CXXFLAGS)
system_command_test_LDADD = \
  libtest_common.la

test_tools_test_LDADD = \
  libtest_common.la

timer_test_LDADD = \
  libtest_common.la

tn_range_test_SOURCES = \
  datum_base.cpp \
  tn_range_test.cpp \
  tn_range_test_aux.cpp
tn_range_test_CXXFLAGS = $(AM_CXXFLAGS)
tn_range_test_LDADD = \
  libtest_common.la

ul_utilities_test_SOURCES = \
  ul_utilities.cpp \
  ul_utilities_test.cpp
ul_utilities_test_CXXFLAGS = $(AM_CXXFLAGS)
ul_utilities_test_LDADD = \
  libtest_common.la

value_cast_test_LDADD = \
  libtest_common.la

vector_test_LDADD = \
  libtest_common.la

wx_new_test_LDADD = \
  libtest_common.la

xml_serialize_test_SOURCES = \
  xml_lmi.cpp \
  xml_serialize_test.cpp
xml_serialize_test_CXXFLAGS = $(AM_CXXFLAGS) $(XMLWRAPP_CFLAGS)
xml_serialize_test_LDADD = \
  libtest_common.la \
  $(XMLWRAPP_LIBS)

zero_test_LDADD = \
  libtest_common.la

# headers: we need to include them or they wouldn't appear in the distribution
# this list should contain everything in `ls *.h *.hpp *.tpp *.xpp`
noinst_HEADERS = \
    about_dialog.hpp \
    account_value.hpp \
    actuarial_table.hpp \
    alert.hpp \
    any_entity.hpp \
    any_member.hpp \
    archetype.hpp \
    assert_lmi.hpp \
    authenticity.hpp \
    basic_tables.hpp \
    basic_values.hpp \
    bin_exp.hpp \
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
    crtp_base.hpp \
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
    duff_fmt.hpp \
    edit_mvc_docview_parameters.hpp \
    emit_ledger.hpp \
    exit_codes.hpp \
    facets.hpp \
    fdlibm.hpp \
    fenv_guard.hpp \
    fenv_lmi.hpp \
    fenv_lmi_x86.hpp \
    file_command.hpp \
    financial.hpp \
    force_linking.hpp \
    fund_data.hpp \
    getopt.hpp \
    global_settings.hpp \
    gpt7702.hpp \
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
    smf.hpp \
    so_attributes.hpp \
    ssize_lmi.hpp \
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
    ul_utilities.hpp \
    unwind.hpp \
    value_cast.hpp \
    verify_products.hpp \
    version.hpp \
    view_ex.hpp \
    view_ex.tpp \
    wine_workarounds.hpp \
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
