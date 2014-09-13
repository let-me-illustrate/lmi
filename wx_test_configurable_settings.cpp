// Configurable settings test case for the GUI test suite.
//
// Copyright (C) 2014 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "wx_test_case.hpp"

#include <boost/filesystem/operations.hpp>

LMI_WX_TEST_CASE(configurable_settings)
{
    LMI_ASSERT(fs::exists("/etc/opt/lmi/configurable_settings.xml"));

    configurable_settings const& settings = configurable_settings::instance();
    LMI_ASSERT_EQUAL(settings.libraries_to_preload(), "");
    LMI_ASSERT_EQUAL(settings.xsl_fo_command(), "CMD /c c:/fop-0.20.5/fop");

    std::string skin = settings.skin_filename();
    std::string default_input = settings.default_input_filename();
    LMI_ASSERT_WITH_MSG
        (  "skin_coli_boli.xrc" == skin
        || "skin_group_carveout.xrc" == skin
        || "skin_group_carveout2.xrc" == skin
        || "reg_d.xrc" == skin
        ,"unknown skin " << skin
        );
    if ("skin_coli_boli.xrc" == skin)
        {
        LMI_ASSERT_EQUAL(default_input, "c:/fop-0.20.5/coli_boli_default.ill");
        }
    if ("skin_group_carveout.xrc" == skin)
        {
        LMI_ASSERT_EQUAL(default_input, "c:/fop-0.20.5/group_carveout_default.ill");
        }
    if ("skin_group_carveout2.xrc" == skin)
        {
        LMI_ASSERT_EQUAL(default_input, "c:/fop-0.20.5/group_carveout_default.ill");
        }
    if ("reg_d.xrc" == skin)
        {
        LMI_ASSERT_EQUAL(default_input, "c:/fop-0.20.5/private_placement_default.ill");
        }
}
