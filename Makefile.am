# Master automake makefile, run automake to generate Makefile.am from it (this
# is also done by autogen.sh script)
#
# Copyright (C) 2005, 2006 Vadim Zeitlin.
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

# $Id: Makefile.am,v 1.23.2.4 2006-11-07 01:47:59 etarassov Exp $


SUBDIRS = third_party

##############################################################################
# All Targets
##############################################################################

# targets going in bin
bin_PROGRAMS =   \
    lmi_cli   \
    lmi_wx \
    elapsed_time    \
    generate_passkey    \
    antediluvian_cli    \
    ihs_crc_comp    \
    product_files

# other programs which may or not be built depending on configure arguments
EXTRA_PROGRAMS = \
    antediluvian_cgi \
    gpt_server
#    gpt_server - does not contain no main() function, bug?..
#    lmi_cgi - restore the target in the future

# libtool libraries installed in lib
lib_LTLIBRARIES =   \
    liblmi.la

# libtool convenience libraries which are not installed at all
noinst_LTLIBRARIES = \
    libantediluvian.la \
    libwx_new.la \
    libmain_auxiliary_common.la

# data files
#
# TODO: we should install them in $(pkgdatadir)/resources or something similar
#       (but for this the code in main_wx.cpp must be modified to allow it)
xrcdir = $(bindir)
dist_xrc_DATA = \
    about.xpm \
    census_view.xpm \
    close.xpm \
    delete-row.xpm \
    default_cell.xpm \
    edit-case.xpm \
    edit-cell.xpm \
    edit-class.xpm \
    exit.xpm \
    fixed-width.xpm \
    help.xpm \
    illustration_view.xpm \
    insert-rows.xpm \
    lmi.xpm \
    menus.xrc \
    new.xpm \
    open.xpm \
    paste-case.xpm \
    print-case.xpm \
    print-cell.xpm \
    print-options.xpm \
    print-preview.xpm \
    print.xpm \
    run-case.xpm \
    run-cell.xpm \
    save.xpm \
    saveas.xpm \
    text_view.xpm \
    toolbar.xrc \
    varying-width.xpm \
    write_spreadsheet.xpm \
    xml_notebook.xrc \
    xml_notebook_coli_boli.xrc \
    xml_notebook_private_placement.xrc \
    xml_notebook_single_premium.xrc

# CGI stuff
if LMI_WITH_CGI
    lib_LTLIBRARIES += libcgi.la
    bin_PROGRAMS += antediluvian_cgi
endif

# tests
TESTS = \
    test_actuarial_table \
    test_alert \
    test_any_member \
    test_argv0 \
    test_calendar_date \
    test_callback \
    test_coding_rules \
    test_commutation_functions \
    test_comma_punct \
    test_crc32 \
    test_expression_template_0 \
    test_fenv_lmi \
    test_financial \
    test_global_settings \
    test_input_seq \
    test_input \
    test_irc7702a \
    test_materially_equal \
    test_math_functors \
    test_mc_enum \
    test_mpatrol_patch \
    test_name_value_pairs \
    test_ncnnnpnn \
    test_numeric_io \
    test_obstruct_slicing \
    test_passkey \
    test_path_utility \
    test_progress_meter \
    test_quiet_nan \
    test_round_to \
    test_snprintf \
    test_stratified_algorithms \
    test_stream_cast \
    test_test_tools \
    test_timer \
    test_tn_range \
    test_value_cast \
    test_vector \
    test_lib_wx_new \
    test_xenum \
    test_xml_resources \
    test_xrange \
    test_zero

check_PROGRAMS = $(TESTS)
check_LTLIBRARIES = \
    libtest_commons.la

####################################
# On windows use auto import feature
####################################
if LMI_MSW
    AM_LDFLAGS=--enable-auto-import
else
    AM_LDFLAGS=
endif


##############################################################################
# Targets definitions
##############################################################################

# main program executables
lmi_wx_SOURCES = \
    about_dialog.cpp \
    alert_wx.cpp \
    ce_product_name.cpp \
    census_document.cpp \
    census_view.cpp \
    datum_base.cpp \
    datum_boolean.cpp \
    datum_string.cpp \
    default_view.cpp \
    docmanager_ex.cpp \
    docmdichildframe_ex.cpp \
    facets.cpp \
    file_command_wx.cpp \
    illustration_document.cpp \
    illustration_view.cpp \
    input.cpp \
    input_harmonization.cpp \
    main_common.cpp \
    main_wx.cpp \
    mc_enum.cpp \
    mc_enum_types.cpp \
    msw_workarounds.cpp \
    mvc_controller.cpp \
    mvc_view.cpp \
    previewframe_ex.cpp \
    progress_meter_wx.cpp \
    properties_model.cpp \
    properties_view.cpp \
    single_choice_popup_menu.cpp \
    text_doc.cpp \
    text_view.cpp \
    tn_range_types.cpp \
    transferor.cpp \
    view_ex.cpp \
    wx_checks.cpp \
    wx_utility.cpp

lmi_wx_CXXFLAGS = $(AM_CXXFLAGS) $(WX_CXXFLAGS) $(LIBXMLPP_CFLAGS) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS)
# $(CGICC_INCLUDE_FLAGS)

if LMI_MSW
## use old style suffix rule instead of the pattern one as automake only
## recognizes the old style rules
.rc.o:
	windres -I $(top_srcdir) $(WX_CPPFLAGS) $< $@

    lmi_wx_SOURCES += lmi.rc
    lmi_wx_CXXFLAGS += -mwindows
endif
lmi_wx_LDADD =  \
    liblmi.la \
    libwx_new.la \
    $(LIBXMLPP_LIBS) \
    $(LIBXSLT_LIBS) \
    $(LIBXML_LIBS) \
    $(BOOST_LIBS) \
    $(WX_LIBS)
if LMI_MSW
    lmi_wx_SOURCES += lmi.rc
    lmi_wx_CXXFLAGS += -mwindows
endif



cli_sources = \
    alert_cli.cpp \
    file_command_cli.cpp \
    main_cli.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    progress_meter_cli.cpp

lmi_cli_SOURCES = \
    $(cli_sources)
lmi_cli_CXXFLAGS = $(AM_CXXFLAGS) $(LIBXMLPP_CFLAGS) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS)
lmi_cli_LDADD = \
    liblmi.la \
    $(LIBXMLPP_LIBS) \
    $(BOOST_LIBS)

# libraries

liblmi_common_sources = \
    actuarial_table.cpp \
    alert.cpp \
    calendar_date.cpp \
    configurable_settings.cpp \
    crc32.cpp \
    custom_io_0.cpp \
    data_directory.cpp \
    dbnames.cpp \
    death_benefits.cpp \
    expm1.c \
    facets.cpp \
    fenv_lmi.cpp \
    file_command.cpp \
    getopt.cpp \
    global_settings.cpp \
    group_values.cpp \
    input_sequence.cpp \
    input_seq_helpers.cpp \
    inputillus.cpp \
    inputillus_sequences.cpp \
    inputillus_xml_io.cpp \
    inputs.cpp \
    inputstatus.cpp \
    interest_rates.cpp \
    ledger.cpp \
    ledger_base.cpp \
    ledger_invariant.cpp \
    ledger_text_formats.cpp \
    ledger_variant.cpp \
    ledger_xml_io.cpp \
    ledger_xsl.cpp \
    ledgervalues.cpp \
    license.cpp \
    loads.cpp \
    miscellany.cpp \
    multiple_cell_document.cpp \
    name_value_pairs.cpp \
    outlay.cpp \
    path_utility.cpp \
    product_names.cpp \
    progress_meter.cpp \
    rounding_rules.cpp \
    sigfpe.cpp \
    single_cell_document.cpp \
    streamable.cpp \
    surrchg_rates.cpp \
    system_command.cpp \
    timer.cpp \
    xenumtypes.cpp \
    xml_lmi.cpp \
    xrange.cpp

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
    avdebug.cpp \
    basicvalues.cpp \
    database.cpp \
    dbdict.cpp \
    dbvalue.cpp \
    inputillus_term_rider.cpp \
    mortality_rates.cpp \
    solve.cpp \
    $(liblmi_common_sources)
libantediluvian_la_CXXFLAGS = $(AM_CXXFLAGS) $(LIBXMLPP_CFLAGS) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS)
libantediluvian_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
libantediluvian_la_LIBADD = \
    $(BOOST_LIBS)



liblmi_la_SOURCES = \
    ihs_acctval.cpp \
    ihs_avdebug.cpp \
    ihs_avmly.cpp \
    ihs_avsolve.cpp \
    ihs_avstrtgy.cpp \
    ihs_basicval.cpp \
    ihs_commfns.cpp \
    ihs_database.cpp \
    ihs_dbdict.cpp \
    ihs_dbvalue.cpp \
    ihs_fpios.cpp \
    ihs_funddata.cpp \
    ihs_inputillus_term_rider.cpp \
    ihs_irc7702.cpp \
    ihs_irc7702a.cpp \
    ihs_mortal.cpp \
    ihs_pios.cpp \
    ihs_proddata.cpp \
    ihs_rnddata.cpp \
    md5.cpp \
    mvc_model.cpp \
    secure_date.cpp \
    security.cpp \
    stratified_algorithms.cpp \
    stratified_charges.cpp \
    $(liblmi_common_sources)
liblmi_la_CXXFLAGS = $(AM_CXXFLAGS) $(LIBXMLPP_CFLAGS) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS)
liblmi_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
liblmi_la_LIBADD = \
    $(LIBXMLPP_LIBS) \
    $(LIBXSLT_LIBS) \
    $(LIBXML_LIBS) \
    $(BOOST_LIBS)

libcgi_la_SOURCES = \
    alert_cgi.cpp \
    file_command_cgi.cpp \
    main_cgi.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    progress_meter_cgi.cpp

libcgi_la_CXXFLAGS = $(BOOST_INCLUDE_FLAGS) $(CGICC_INCLUDE_FLAGS) $(AM_CXXFLAGS)
libcgi_la_LIBADD = $(CGICC_LIBS)



libwx_new_la_SOURCES = wx_new.cpp
libwx_new_la_LDFLAGS = -no-undefined $(AM_LDFLAGS)
libwx_new_la_LIBADD = $(WX_LIBS)


# auxiliary executables

generate_passkey_SOURCES = \
    calendar_date.cpp \
    generate_passkey.cpp \
    md5.cpp \
    secure_date.cpp \
    system_command.cpp
generate_passkey_CXXFLAGS = $(AM_CXXFLAGS)
generate_passkey_LDADD = \
    $(BOOST_LIBS) \
    libmain_auxiliary_common.la

gpt_server_SOURCES = $(top_builddir)/dummy.cpp
gpt_server_CXXFLAGS = $(AM_CXXFLAGS)
gpt_server_LDADD = \
    liblmi.la

elapsed_time_SOURCES = \
    elapsed_time.cpp system_command.cpp timer.cpp
elapsed_time_CXXFLAGS = $(AM_CXXFLAGS)
elapsed_time_LDADD = \
    libmain_auxiliary_common.la

antediluvian_cgi_SOURCES = $(top_builddir)/dummy.cpp
antediluvian_cgi_CXXFLAGS = $(AM_CXXFLAGS) $(BOOST_INCLUDE_FLAGS)
antediluvian_cgi_LDADD = \
    libcgi.la \
    libantediluvian.la   \
    $(CGICC_LIBS) \
    $(BOOST_LIBS)

antediluvian_cli_SOURCES = \
    $(cli_sources)
antediluvian_cli_CXXFLAGS = $(AM_CXXFLAGS) $(LIBXMLPP_CFLAGS) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS)
antediluvian_cli_LDADD = \
    libantediluvian.la  \
    $(BOOST_LIBS) \
    $(LIBXMLPP_LIBS)

ihs_crc_comp_SOURCES = ihs_crc_comp.cpp
ihs_crc_comp_LDADD = libmain_auxiliary_common.la

product_files_SOURCES = \
    generate_product_files.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    my_db.cpp \
    my_fund.cpp \
    my_prod.cpp \
    my_rnd.cpp \
    my_tier.cpp \
    xenumtypes.cpp
product_files_LDADD = \
    liblmi.la   \
    $(BOOST_LIBS) \
    $(LIBXMLPP_LIBS)


dummy.cpp:
	touch $@


# unit tests
libtest_commons_la_SOURCES = \
    alert.cpp \
    alert_cli.cpp \
    fenv_lmi.cpp \
    getopt.cpp \
    license.cpp \
    expm1.c
libtest_commons_la_CFLAGS = $(AM_CFLAGS)

test_actuarial_table_SOURCES = \
  actuarial_table.cpp \
  actuarial_table_test.cpp \
  timer.cpp
test_actuarial_table_CXXFLAGS = $(AM_CXXFLAGS)
test_actuarial_table_LDADD = \
  $(BOOST_LIBS) \
  libtest_commons.la

test_alert_SOURCES = \
  alert_test.cpp
test_alert_CXXFLAGS = $(AM_CXXFLAGS)
test_alert_LDADD = \
  libtest_commons.la

test_any_member_SOURCES =   \
  any_member_test.cpp \
  facets.cpp
test_any_member_CXXFLAGS = $(AM_CXXFLAGS)
test_any_member_LDADD =   \
  libtest_commons.la

test_argv0_SOURCES =    \
  argv0_test.cpp
test_argv0_CXXFLAGS = $(AM_CXXFLAGS)
test_argv0_LDADD =    \
  $(BOOST_LIBS) \
  libtest_commons.la

test_calendar_date_SOURCES =    \
  calendar_date.cpp \
  calendar_date_test.cpp
test_calendar_date_CXXFLAGS = $(AM_CXXFLAGS)
test_calendar_date_LDADD =    \
  libtest_commons.la

test_callback_SOURCES = \
  callback_test.cpp
test_callback_CXXFLAGS = $(AM_CXXFLAGS)
test_callback_LDADD = \
  libtest_commons.la

test_coding_rules_SOURCES = \
  test_coding_rules.cpp
test_coding_rules_CXXFLAGS = $(BOOST_INCLUDE_FLAGS) $(AM_CXXFLAGS)
test_coding_rules_LDADD = \
  $(BOOST_LIBS) \
  libmain_auxiliary_common.la

test_commutation_functions_SOURCES =    \
  commutation_functions_test.cpp \
  ihs_commfns.cpp \
  timer.cpp \
  xenumtypes.cpp
test_commutation_functions_CXXFLAGS = $(AM_CXXFLAGS)
test_commutation_functions_LDADD =    \
  libtest_commons.la

test_comma_punct_SOURCES =  \
  comma_punct_test.cpp
test_comma_punct_CXXFLAGS = $(AM_CXXFLAGS)
test_comma_punct_LDADD =  \
  libtest_commons.la

test_crc32_SOURCES =    \
  crc32.cpp \
  crc32_test.cpp
test_crc32_CXXFLAGS = $(AM_CXXFLAGS)
test_crc32_LDADD =    \
  libtest_commons.la

test_expression_template_0_SOURCES =    \
  expression_template_0_test.cpp \
  timer.cpp
test_expression_template_0_CXXFLAGS = $(AM_CXXFLAGS)
test_expression_template_0_LDADD =    \
  libtest_commons.la

test_fenv_lmi_SOURCES = \
  fenv_lmi.cpp \
  fenv_lmi_test.cpp
test_fenv_lmi_CXXFLAGS = $(AM_CXXFLAGS)
test_fenv_lmi_LDADD = \
  libtest_commons.la

test_financial_SOURCES =    \
  financial_test.cpp \
  stratified_algorithms.cpp \
  timer.cpp
test_financial_CXXFLAGS = $(AM_CXXFLAGS)
test_financial_LDADD =    \
  libtest_commons.la

test_global_settings_SOURCES =  \
  global_settings.cpp \
  global_settings_test.cpp \
  miscellany.cpp \
  path_utility.cpp
test_global_settings_CXXFLAGS = $(AM_CXXFLAGS)
test_global_settings_LDADD =  \
  $(BOOST_LIBS) \
  libtest_commons.la

test_input_seq_SOURCES =    \
  input_seq_test.cpp \
  input_sequence.cpp
test_input_seq_CXXFLAGS = $(AM_CXXFLAGS)
test_input_seq_LDADD =    \
  libtest_commons.la


test_input_SOURCES =    \
  alert.cpp \
  alert_cli.cpp \
  calendar_date.cpp \
  configurable_settings.cpp \
  data_directory.cpp \
  database.cpp \
  dbdict.cpp \
  dbnames.cpp \
  dbvalue.cpp \
  facets.cpp \
  global_settings.cpp \
  input_seq_helpers.cpp \
  input_sequence.cpp \
  input_test.cpp \
  inputillus.cpp \
  inputillus_sequences.cpp \
  inputillus_term_rider.cpp \
  inputillus_xml_io.cpp \
  inputs.cpp \
  inputstatus.cpp \
  license.cpp \
  miscellany.cpp \
  multiple_cell_document.cpp \
  path_utility.cpp \
  product_names.cpp \
  single_cell_document.cpp \
  streamable.cpp \
  xenumtypes.cpp \
  xml_lmi.cpp \
  xrange.cpp

test_input_CXXFLAGS = $(AM_CXXFLAGS) $(LIBXMLPP_CFLAGS) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS)
test_input_LDADD =    \
  $(BOOST_LIBS) \
  libtest_commons.la \
  $(LIBXMLPP_LIBS) \
  $(test_input_extra_libs)

test_irc7702a_SOURCES = \
  ihs_irc7702a.cpp \
  irc7702a_test.cpp \
  stratified_algorithms.cpp
test_irc7702a_CXXFLAGS = $(AM_CXXFLAGS)
test_irc7702a_LDADD = \
  libtest_commons.la

test_materially_equal_SOURCES = \
  materially_equal_test.cpp
test_materially_equal_CXXFLAGS = $(AM_CXXFLAGS)
test_materially_equal_LDADD = \
  libtest_commons.la

test_math_functors_SOURCES =    \
  math_functors_test.cpp \
  timer.cpp
test_math_functors_CXXFLAGS = $(AM_CXXFLAGS)
test_math_functors_LDADD =    \
  libtest_commons.la

test_mc_enum_SOURCES =  \
  datum_base.cpp \
  facets.cpp \
  global_settings.cpp \
  mc_enum.cpp \
  mc_enum_test.cpp \
  mc_enum_test_aux.cpp \
  miscellany.cpp \
  path_utility.cpp
test_mc_enum_CXXFLAGS = $(AM_CXXFLAGS)
test_mc_enum_LDADD =  \
  $(BOOST_LIBS) \
  libtest_commons.la

test_mpatrol_patch_SOURCES =    \
  mpatrol_patch_test.cpp
test_mpatrol_patch_CXXFLAGS = $(AM_CXXFLAGS)
test_mpatrol_patch_LDADD =    \
  libtest_commons.la

test_name_value_pairs_SOURCES = \
  name_value_pairs.cpp \
  name_value_pairs_test.cpp
test_name_value_pairs_CXXFLAGS = $(AM_CXXFLAGS)
test_name_value_pairs_LDADD = \
  libtest_commons.la

test_ncnnnpnn_SOURCES = \
  ncnnnpnn_test.cpp
test_ncnnnpnn_CXXFLAGS = $(AM_CXXFLAGS)
test_ncnnnpnn_LDADD = \
  libtest_commons.la

test_numeric_io_SOURCES =   \
  numeric_io_test.cpp \
  timer.cpp
test_numeric_io_CXXFLAGS = $(AM_CXXFLAGS)
test_numeric_io_LDADD =   \
  libtest_commons.la

test_obstruct_slicing_SOURCES = \
  obstruct_slicing_test.cpp \
  timer.cpp
test_obstruct_slicing_CXXFLAGS = $(AM_CXXFLAGS)
test_obstruct_slicing_LDADD = \
  libtest_commons.la

test_passkey_SOURCES =  \
  calendar_date.cpp \
  md5.cpp \
  passkey_test.cpp \
  secure_date.cpp \
  system_command.cpp
test_passkey_CXXFLAGS = $(AM_CXXFLAGS)
test_passkey_LDADD =  \
  $(BOOST_LIBS) \
  libtest_commons.la

test_path_utility_SOURCES = \
  miscellany.cpp \
  path_utility.cpp \
  path_utility_test.cpp
test_path_utility_CXXFLAGS = $(AM_CXXFLAGS)
test_path_utility_LDADD = \
  $(BOOST_LIBS) \
  libtest_commons.la

test_progress_meter_SOURCES =   \
  progress_meter.cpp \
  progress_meter_cli.cpp \
  progress_meter_test.cpp
test_progress_meter_CXXFLAGS = $(AM_CXXFLAGS)
test_progress_meter_LDADD =   \
  libtest_commons.la

test_quiet_nan_SOURCES =    \
  quiet_nan_test.cpp
test_quiet_nan_CXXFLAGS = $(AM_CXXFLAGS)
test_quiet_nan_LDADD = \
  libtest_commons.la

test_round_to_SOURCES = \
  round_to_test.cpp
test_round_to_CXXFLAGS = $(AM_CXXFLAGS)
test_round_to_LDADD = \
  libtest_commons.la

test_snprintf_SOURCES = \
  snprintf_test.cpp
test_snprintf_CXXFLAGS = $(AM_CXXFLAGS)
test_snprintf_LDADD = \
  libtest_commons.la

test_stratified_algorithms_SOURCES =    \
  stratified_algorithms_test.cpp
test_stratified_algorithms_CXXFLAGS = $(AM_CXXFLAGS)
test_stratified_algorithms_LDADD =    \
  libtest_commons.la

test_stream_cast_SOURCES =  \
  facets.cpp \
  stream_cast_test.cpp
test_stream_cast_CXXFLAGS = $(AM_CXXFLAGS)
test_stream_cast_LDADD =  \
  libtest_commons.la

test_test_tools_SOURCES =   \
  test_tools_test.cpp
test_test_tools_CXXFLAGS = $(AM_CXXFLAGS)
test_test_tools_LDADD =   \
  libtest_commons.la

test_timer_SOURCES =    \
  timer.cpp \
  timer_test.cpp
test_timer_CXXFLAGS = $(AM_CXXFLAGS)
test_timer_LDADD =    \
  libtest_commons.la

test_tn_range_SOURCES = \
  datum_base.cpp \
  tn_range_test.cpp \
  tn_range_test_aux.cpp
test_tn_range_CXXFLAGS = $(AM_CXXFLAGS)
test_tn_range_LDADD = \
  libtest_commons.la

test_value_cast_SOURCES =   \
  facets.cpp \
  value_cast_test.cpp
test_value_cast_CXXFLAGS = $(AM_CXXFLAGS)
test_value_cast_LDADD =   \
  libtest_commons.la

test_vector_SOURCES =   \
  vector_test.cpp
test_vector_CXXFLAGS = $(AM_CXXFLAGS)
test_vector_LDADD =   \
  libtest_commons.la

test_lib_wx_new_SOURCES =   \
  wx_new_test.cpp
test_lib_wx_new_CXXFLAGS = $(AM_CXXFLAGS)
test_lib_wx_new_LDADD =   \
  libtest_commons.la

test_xenum_SOURCES =    \
  xenum_sample.cpp \
  xenum_test.cpp
test_xenum_CXXFLAGS = $(AM_CXXFLAGS)
test_xenum_LDADD =    \
  libtest_commons.la

test_xml_resources_SOURCES = \
  alert.cpp \
  alert_cli.cpp \
  xml_resources_test.cpp
test_xml_resources_CXXFLAGS = $(AM_CXXFLAGS) $(LIBXMLPP_CFLAGS) $(LIBXSLT_CFLAGS) $(LIBXML_CFLAGS)
test_xml_resources_LDADD = \
  $(BOOST_LIBS) \
  $(LIBXMLPP_LIBS) \
  $(LIBXML_LIBS) \
  $(LIBXSLT_LIBS) \
  liblmi.la \
  libtest_commons.la

test_xrange_SOURCES =   \
  calendar_date.cpp \
  facets.cpp \
  xrange.cpp \
  xrange_test.cpp
test_xrange_CXXFLAGS = $(AM_CXXFLAGS)
test_xrange_LDADD =   \
  libtest_commons.la

test_zero_SOURCES = \
  zero_test.cpp
test_zero_CXXFLAGS = $(AM_CXXFLAGS)
test_zero_LDADD = \
  libtest_commons.la


# headers: we need to include them or they wouldn't appear in the distribution
noinst_HEADERS = \
    about_dialog.hpp \
    account_value.hpp \
    actuarial_table.hpp \
    alert.hpp \
    any_member.hpp \
    argv0.hpp \
    basic_values.hpp \
    calculate.hpp \
    calendar_date.hpp \
    callback.hpp \
    catch_exceptions.hpp \
    ce_product_name.hpp \
    census_document.hpp \
    census_view.hpp \
    comma_punct.hpp \
    config.hpp \
    config_bc551.hpp \
    config_ming323.hpp \
    configurable_settings.hpp \
    copy_n_.hpp \
    crc32.hpp \
    custom_io_0.hpp \
    data_directory.hpp \
    database.hpp \
    datum_base.hpp \
    datum_boolean.hpp \
    datum_string.hpp \
    dbdict.hpp \
    dbindex.hpp \
    dbnames.hpp \
    dbvalue.hpp \
    death_benefits.hpp \
    default_view.hpp \
    docmanager_ex.hpp \
    docmdichildframe_ex.hpp \
    enums7702.hpp \
    exit_codes.hpp \
    facets.hpp \
    fenv_lmi.hpp \
    fenv_lmi_x86.hpp \
    file_command.hpp \
    financial.hpp \
    getopt.hpp \
    global_settings.hpp \
    group_values.hpp \
    ihs_commfns.hpp \
    ihs_dbdict.hpp \
    ihs_dbvalue.hpp \
    ihs_fpios.hpp \
    ihs_funddata.hpp \
    ihs_irc7702.hpp \
    ihs_irc7702a.hpp \
    ihs_pios.hpp \
    ihs_proddata.hpp \
    ihs_rnddata.hpp \
    ihs_server7702.hpp \
    ihs_server7702io.hpp \
    ihs_standalone.hpp \
    ihs_x_type.hpp \
    illustration_document.hpp \
    illustration_view.hpp \
    input.hpp \
    input_seq_helpers.hpp \
    input_sequence.hpp \
    inputillus.hpp \
    inputs.hpp \
    inputstatus.hpp \
    interest_rates.hpp \
    is_sorted_.hpp \
    istream_to_string.hpp \
    ledger.hpp \
    ledger_base.hpp \
    ledger_invariant.hpp \
    ledger_text_formats.hpp \
    ledger_variant.hpp \
    ledger_xsl.hpp \
    ledgervalues.hpp \
    license.hpp \
    loads.hpp \
    loads_impl.hpp \
    main_common.hpp \
    main_wx.hpp \
    materially_equal.hpp \
    math_functors.hpp \
    max_stream_precision.hpp \
    mc_enum.hpp \
    mc_enum_fwd.hpp \
    mc_enum_test_aux.hpp \
    mc_enum_test_aux_enums.hpp \
    mc_enum_type_enums.hpp \
    mc_enum_types.hpp \
    msw_workarounds.hpp \
    mvc_controller.hpp \
    mvc_view.hpp \
    md5.hpp \
    miscellany.hpp \
    mortality_rates.hpp \
    multiple_cell_document.hpp \
    name_value_pairs.hpp \
    ncnnnpnn.hpp \
    numeric_io_cast.hpp \
    numeric_io_traits.hpp \
    obstruct_slicing.hpp \
    outlay.hpp \
    path_utility.hpp \
    pchfile.hpp \
    platform_dependent.hpp \
    previewframe_ex.hpp \
    product_names.hpp \
    progress_meter.hpp \
    quiet_nan.hpp \
    round_to.hpp \
    rounding_rules.hpp \
    secure_date.hpp \
    security.hpp \
    sigfpe.hpp \
    single_cell_document.hpp \
    single_choice_popup_menu.hpp \
    stratified_algorithms.hpp \
    stratified_charges.hpp \
    stream_cast.hpp \
    streamable.hpp \
    surrchg_rates.hpp \
    system_command.hpp \
    test_tools.hpp \
    text_doc.hpp \
    text_view.hpp \
    timer.hpp \
    tn_range.hpp \
    tn_range_fwd.hpp \
    tn_range_test_aux.hpp \
    tn_range_type_trammels.hpp \
    tn_range_types.hpp \
    transferor.hpp \
    value_cast.hpp \
    version.hpp \
    view_ex.hpp \
    wx_checks.hpp \
    wx_new.hpp \
    wx_utility.hpp \
    wx_workarounds.hpp \
    xenum.hpp \
    xenum_sample.hpp \
    xenumfwd.hpp \
    xenumtypes.hpp \
    xml_notebook.hpp \
    xml_lmi.hpp \
    xrange.hpp \
    xrangefwd.hpp \
    xrangetypes.hpp \
    zero.hpp
