// Validate configurable-settings file for binary distributions.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "path.hpp"
#include "wx_test_case.hpp"

/*
    Test configurable_settings.xml file.

    Implement the following verifications from the official specification:

    1. Inspect 'configurable_settings.xml' for the following:

     A. The file exists in '/etc/opt/lmi'

     B. This element is empty in all distributions:
         <libraries_to_preload/>

     C. We package several distributions each month that have their
        own "skin" and "default" files. Each time this test is run,
        only one pair of the following lines exists at a time:

PDF !! Here and passim, change 'fop' to something reasonable, e.g.:
  <default_input_filename>c:/spool/coli_boli_default.ill</default_input_filename>

          <skin_filename>skin_coli_boli.xrc</skin_filename>
          <default_input_filename>c:/fop-0.20.5/coli_boli_default.ill</default_input_filename>

          <skin_filename>skin_group_carveout.xrc</skin_filename>
          <default_input_filename>c:/fop-0.20.5/group_carveout_default.ill</default_input_filename>

          <skin_filename>skin_single_premium.xrc</skin_filename>
          <default_input_filename>c:/fop-0.20.5/default.ill</default_input_filename>
          [Something like "single_premium_default.ill" might be expected,
          but "default.ill" really is wanted here for historical reasons.]

          ['skin.xrc' deliberately omitted--not currently distributed.]
 */

/// Validate configurable-settings file for binary distributions.
///
/// Run this test only when the '--distribution' option is given.
/// The invariants it tests are not universally appropriate.
///
/// This test may someday be replaced by a shell script, which would
/// be a better fit for its intended purpose. In particular, we want
/// to run the GUI-test suite only once, and then combine the lmi
/// binaries with selected input skins and default-input files; these
/// tests check the combinations, and thus must be run once for each
/// combination, but we don't want to repeat all the GUI tests for
/// each combination. Of course, this individual test can be run in
/// isolation for each combination, but for that use case a script
/// would be simpler solution. We'll reconsider this later.

LMI_WX_TEST_CASE(configurable_settings)
{
    skip_if_not_distribution();

    LMI_ASSERT(fs::exists("/etc/opt/lmi/configurable_settings.xml"));

    configurable_settings const& settings = configurable_settings::instance();
    LMI_ASSERT_EQUAL(settings.libraries_to_preload(), "");

    std::string const& skin = settings.skin_filename();
    std::string const& default_input = settings.default_input_filename();
    LMI_ASSERT_WITH_MSG
        (  "skin_coli_boli.xrc"      == skin
        || "skin_group_carveout.xrc" == skin
        || "skin_single_premium.xrc" == skin
        ,"unknown skin " << skin
        );
    if("skin_coli_boli.xrc" == skin)
        {
        LMI_ASSERT_EQUAL(default_input, "c:/fop-0.20.5/coli_boli_default.ill");
        }
    if("skin_group_carveout.xrc" == skin)
        {
        LMI_ASSERT_EQUAL(default_input, "c:/fop-0.20.5/group_carveout_default.ill");
        }
    if("skin_single_premium.xrc" == skin)
        {
        LMI_ASSERT_EQUAL(default_input, "c:/fop-0.20.5/default.ill");
        }
}
