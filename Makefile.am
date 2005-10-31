# Master automake makefile, run automake to generate Makefile.am from it (this
# is also done by autogen.sh script)
#
# Copyright (C) 2005 Vadim Zeitlin.
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

# $Id: Makefile.am,v 1.3 2005-10-31 21:04:09 zeitlin Exp $

##############################################################################
# All Targets
##############################################################################

# targets going in bin
bin_PROGRAMS =   \
    lmi_wx \
    lmi_cli   \
    elapsed_time    \
    generate_passkey    \
    antediluvian_cli    \
    ihs_crc_comp    \
    product_files

# other programs which may or not be built depending on configure arguments
EXTRA_PROGRAMS = \
    antediluvian_cgi

# libtool libraries installed in lib
lib_LTLIBRARIES =   \
    liblmi.la   \
    liblmi_common.la \
    libcli.la   \
    libantediluvian.la \
    libwx_new.la

# other libraries which may, or not, be built
EXTRA_LTLIBRARIES =   \
    libmain_auxiliary_common.la \
    libtest_commons.la


# data files
#
# TODO: install xpm and xrc in share/resources or some other such directory
#       (we need to update code in main_wx.cpp to look for them there), as
#       as we have to install them in bin we must fool automake by using
#       SCRIPTS and not DATA primary
dist_bin_SCRIPTS = \
    about.xpm \
    census_view.xpm \
    close.xpm \
    delete-row.xpm \
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
if LMI_WITH_TESTS
    bin_PROGRAMS += \
        test_actuarial_table test_alert test_any_member test_argv0  \
        test_calendar_date test_callback test_commutation_functions \
        test_comma_punct test_crc32 test_expression_template_0 test_financial   \
        test_global_settings test_input_seq test_input test_irc7702a    \
        test_materially_equal test_math_functors test_mc_enum test_mpatrol_patch  \
        test_name_value_pairs test_ncnnnpnn test_numeric_io test_obstruct_slicing \
        test_passkey test_path_utility test_progress_meter test_quiet_nan   \
        test_round_to test_snprintf test_stratified_algorithms test_stream_cast \
        test_test_tools test_timer test_tn_range test_value_cast test_vector \
        test_lib_wx_new test_xenum test_xrange test_zero
endif


##############################################################################
# Compiler options
##############################################################################

# initialize them to avoid automake warnings when using += below
AM_CPPFLAGS =
AM_CFLAGS =
AM_CXXFLAGS =
AM_LDFLAGS =

if LMI_DEBUG
    AM_CFLAGS += -g
    AM_CXXFLAGS += -g
    AM_LDFLAGS += -g
else
    AM_CPPFLAGS += -DNDEBUG
endif

gcc_common_warnings := \
  -pedantic \
  -Wall \
  -Wcast-align \
  -Wconversion \
  -Wdeprecated-declarations \
  -Wdisabled-optimization \
  -Wendif-labels \
  -Wimport \
  -Wmultichar \
  -Wpacked \
  -Wpointer-arith \
  -Wsign-compare \
  -Wwrite-strings

# Some boost libraries treat 'long long' as part of the language,
# which it probably soon will be, so permit it now.
gcc_common_warnings += -Wno-long-long

gcc_c_warnings := \
  $(gcc_common_warnings) \
  -std=c99 \
  -Wmissing-prototypes

gcc_cxx_warnings := \
  $(gcc_common_warnings) \
  -std=c++98 \
  -Wctor-dtor-privacy \
  -Wdeprecated \
  -Wnon-template-friend \
  -Woverloaded-virtual \
  -Wpmf-conversions \
  -Wsynth

# TODO ?? VZ reports that
#  -Winvalid-offsetof
# gives an error with GNU/Linux gcc-3.3 and 3.4, though that seems
# to contradict the gcc manual. This should be investigated and
# possibly reported on gcc bugzilla.

# Too many warnings on correct code, e.g. exact comparison to zero:
#  -Wfloat-equal

# WX !! The wx library triggers many warnings with these flags:

# -Wextra is for g++ >= 3.4 only
gcc_common_extra_warnings := \
  -Wextra \
  -Wcast-qual \
  -Wredundant-decls
#  -Wundef -- generates spurios warnings in boost headers


# Boost normally makes '-Wundef' give spurious warnings:
#   http://aspn.activestate.com/ASPN/Mail/Message/boost/1822550
# but defining BOOST_STRICT_CONFIG:
#   http://www.boost.org/libs/config/config.htm#user_settable
# makes '-Wundef' usable, because boost-1.31.0 doesn't seem to need
# any workarounds for gcc-3.3+ . However, it gives a number of
# warnings with wx-2.5.4 (that have been fixed in a later version).

# Too many warnings for various boost libraries:
#  -Wold-style-cast
#  -Wshadow

# Too many warnings for libstdc++:
#  -Wunreachable-code

# Since at least gcc-3.4.2, -Wmissing-prototypes is deprecated as
# being redundant for C++.

if LMI_GCC
    AM_CFLAGS += $(gcc_c_warnings)
endif
if LMI_GXX
    AM_CXXFLAGS += $(gcc_cxx_warnings)
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
    date_control.cpp \
    datum_base.cpp \
    datum_boolean.cpp \
    datum_string.cpp \
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
    previewframe_ex.cpp \
    progress_meter_wx.cpp \
    text_doc.cpp \
    text_view.cpp \
    tn_range_types.cpp \
    transferor.cpp \
    view_ex.cpp \
    xml_notebook.cpp \
    xmlwrapp_ex.cpp

lmi_wx_CXXFLAGS = $(WX_CXXFLAGS) $(CGICC_INCLUDE_FLAGS) $(XMLWRAPP_FLAGS) $(AM_CXXFLAGS)
if LMI_WINDOWS
    lmi_wx_CXXFLAGS += -DLMI_USE_DLL -mwindows
endif
lmi_wx_LDADD =  \
    liblmi.la \
    liblmi_common.la \
    libwx_new.la   \
    $(BOOST_LIBS) \
    $(XMLWRAPP_LIBS) \
    $(WX_LIBS)

lmi_cli_SOURCES = $(top_builddir)/dummy.cpp
lmi_cli_CXXFLAGS=$(CGICC_INCLUDE_FLAGS)
lmi_cli_LDADD = \
    libcli.la \
    liblmi.la    \
    liblmi_common.la    \
    $(BOOST_LIBS)

# libraries

liblmi_common_la_SOURCES = \
    accountvalue.cpp \
    actuarial_table.cpp \
    alert.cpp \
    basicvalues.cpp \
    database.cpp \
    calendar_date.cpp \
    configurable_settings.cpp \
    crc32.cpp \
    custom_io_0.cpp \
    data_directory.cpp \
    dbdict.cpp \
    dbvalue.cpp \
    dbnames.cpp \
    death_benefits.cpp \
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
    inputillus_term_rider.cpp \
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
    md5.cpp \
    miscellany.cpp \
    mortality_rates.cpp \
    multiple_cell_document.cpp \
    name_value_pairs.cpp \
    outlay.cpp \
    path_utility.cpp \
    progress_meter.cpp \
    rounding_rules.cpp \
    secure_date.cpp \
    security.cpp \
    sigfpe.cpp \
    single_cell_document.cpp \
    solve.cpp \
    surrchg_rates.cpp \
    system_command.cpp \
    timer.cpp \
    xenumtypes.cpp \
    xmlwrapp_ex.cpp \
    xrange.cpp
liblmi_common_la_LIBADD = \
    $(BOOST_LIBS) \
    $(XMLWRAPP_LIBS)

libmain_auxiliary_common_la_SOURCES = \
    alert.cpp \
    alert_cli.cpp \
    fenv_lmi.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    sigfpe.cpp



libantediluvian_la_SOURCES = \
    accountvalue.cpp antediluvian_stubs.cpp avdebug.cpp basicvalues.cpp database.cpp dbdict.cpp \
    dbvalue.cpp inputillus_term_rider.cpp mortality_rates.cpp solve.cpp
libantediluvian_la_LIBADD = \
    liblmi_common.la


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
    secure_date.cpp \
    security.cpp \
    stratified_algorithms.cpp \
    stratified_charges.cpp
liblmi_la_LIBADD = \
    liblmi_common.la

libcgi_la_SOURCES = \
    alert_cgi.cpp \
    file_command_cgi.cpp \
    main_cgi.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    progress_meter_cgi.cpp

libcgi_la_CXXFLAGS = $(BOOST_INCLUDE_FLAGS) $(CGICC_INCLUDE_FLAGS) $(AM_CXXFLAGS)
libcgi_la_LIBADD = \
    $(CGICC_LIBS)

libcli_la_SOURCES = \
    alert_cli.cpp \
    file_command_cli.cpp \
    main_cli.cpp \
    main_common.cpp \
    main_common_non_wx.cpp \
    progress_meter_cli.cpp
libcli_la_LIBADD = -lncurses

libtest_commons_la_SOURCES = \
    alert.cpp \
    alert_cli.cpp \
    fenv_lmi.cpp \
    getopt.cpp \
    license.cpp

libwx_new_la_SOURCES = \
    wx_new.cpp
libwx_new_la_LIBADD = \
    $(WX_LIBS)


# auxiliary executables

generate_passkey_SOURCES = \
    calendar_date.cpp generate_passkey.cpp md5.cpp secure_date.cpp system_command.cpp
generate_passkey_CXXFLAGS = $(AM_CXXFLAGS)
generate_passkey_LDADD = \
    $(BOOST_LIBS) libmain_auxiliary_common.la

elapsed_time_SOURCES = \
    elapsed_time.cpp system_command.cpp timer.cpp
elapsed_time_CXXFLAGS = $(AM_CXXFLAGS)
elapsed_time_LDADD = \
    libmain_auxiliary_common.la

antediluvian_cgi_SOURCES = $(top_builddir)/dummy.cpp
antediluvian_cgi_CXXFLAGS = $(BOOST_INCLUDE_FLAGS) $(AM_CXXFLAGS)
antediluvian_cgi_LDADD = \
    libcgi.la \
    libantediluvian.la   \
    $(CGICC_LIBS) \
    $(BOOST_LIBS)

antediluvian_cli_SOURCES = $(top_builddir)/dummy.cpp
antediluvian_cli_CXXFLAGS = $(AM_CXXFLAGS)
antediluvian_cli_LDADD = \
    libcli.la \
    libantediluvian.la  \
    $(BOOST_LIBS)  \
    $(XMLWRAPP_LIBS)

ihs_crc_comp_SOURCES = ihs_crc_comp.cpp
ihs_crc_comp_LDADD = libmain_auxiliary_common.la

product_files_SOURCES = \
    generate_product_files.cpp my_db.cpp my_fund.cpp my_prod.cpp my_rnd.cpp my_tier.cpp xenumtypes.cpp
product_files_CXXFLAGS = $(AM_CXXFLAGS)
product_files_LDADD = \
    liblmi.la   \
    libmain_auxiliary_common.la    \
    $(BOOST_LIBS)


dummy.cpp:
	touch $@


# unit tests
#
# TODO: use automake built-in support for the tests?
test_actuarial_table_SOURCES = \
  actuarial_table.cpp \
  actuarial_table_test.cpp \
  timer.cpp
test_actuarial_table_CXXFLAGS = $(AM_CXXFLAGS)
test_actuarial_table_LDADD = \
  $(BOOST_LIBS)    \
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

test_financial_SOURCES =    \
  financial_test.cpp \
  stratified_algorithms.cpp \
  timer.cpp
test_financial_CXXFLAGS = $(AM_CXXFLAGS)
test_financial_LDADD =    \
  libtest_commons.la

test_global_settings_SOURCES =  \
  global_settings.cpp \
  global_settings_test.cpp
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
  multiple_cell_document.cpp \
  single_cell_document.cpp \
  xenumtypes.cpp \
  xmlwrapp_ex.cpp \
  xrange.cpp

test_input_CXXFLAGS = $(XMLWRAPP_FLAGS) $(AM_CXXFLAGS)
test_input_LDADD =    \
  $(BOOST_LIBS) \
  libtest_commons.la \
  $(XMLWRAPP_LIBS) \
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
  mc_enum_test_aux.cpp
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


