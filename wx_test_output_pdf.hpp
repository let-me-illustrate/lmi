// Helper class for working with PDF output files in wx test suite.
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef wx_test_output_pdf_hpp
#define wx_test_output_pdf_hpp

#include "config.hpp"

#include "configurable_settings.hpp"
#include "path.hpp"
#include "wx_test_output.hpp"

/// Specialized version of output_file_existence_checker for the output PDF
/// files: it takes just the base name of the file, without neither the
/// directory part nor the .pdf extension, in its ctor and also takes care of
/// deleting the .fo.xml created as a side effect of PDF generation when the
/// PDF file itself is removed.
class output_pdf_existence_checker :public output_file_existence_checker
{
  public:
    explicit output_pdf_existence_checker(std::string const& base_name)
        :output_file_existence_checker
            (make_full_print_path(base_name + ".pdf")
            )
        ,fo_xml_path_
            (make_full_print_path(base_name + ".fo.xml")
            )
        {
        // We do not remove .fo.xml file here, this is unnecessary as we don't
        // particularly care whether it exists or not because we never check
        // for its existence.
        }

    ~output_pdf_existence_checker()
        {
        // Do remove the .fo.xml file to avoid littering the print directory
        // with the files generated during the test run.
        try
            {
            fs::remove(fo_xml_path_);
            }
        catch(...)
            {
            }
        }

  private:
    // Return the full path in the print directory for the file with the given
    // leaf name.
    static fs::path make_full_print_path(std::string const& leaf)
        {
        fs::path p(configurable_settings::instance().print_directory());
        p /= leaf;
        return p;
        }

    fs::path fo_xml_path_;
};

#endif // wx_test_output_pdf_hpp
