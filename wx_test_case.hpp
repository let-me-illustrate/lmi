// Supporting framework for wx interface test cases.
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

#ifndef wx_test_case_hpp
#define wx_test_case_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <boost/filesystem/path.hpp>

/// Base class for the test case objects.
///
/// It is only supposed to be used by LMI_WX_TEST_CASE macro and not directly.
class wx_base_test_case
    :private lmi::uncopyable<wx_base_test_case>
{
  public:
    /// The function actually executing the test code.
    ///
    /// This function should throw an exception to signal any failures.
    virtual void run() = 0;

    char const* get_name() const { return m_name; }

    // Only required to fix g++ warning about a class having virtual functions
    // but a non-virtual dtor, as this class is not used polymorphically the
    // dtor doesn't really need to be virtual.
    virtual ~wx_base_test_case() { }

    /// Skip the test if the specified file is not supported.
    ///
    /// Check if the possibility to open such files is provided by the program
    /// in its current configuration: some file types are conditionally enabled
    /// only if special command line arguments are provided, so it is normal
    /// for them to not be available and this shouldn't result in the test
    /// errors.
    ///
    /// Notice that this method needs to be public to be usable from helpers of
    /// the tests and not just from the test code itself.
    ///
    /// The file doesn't need to exist, but must have the correct extension.
    ///
    /// Throws test_skipped_exception if the file is not supported.
    void skip_if_not_supported(char const* file);

    /// Return the base directory containing the test files.
    ///
    /// This is the same directory as is used by get_test_file_path_for(),
    /// prefer to use that function if possible.
    fs::path get_test_files_path() const;

    /// Return the full path for the file with the given base name (which
    /// should include the extension, but no path components).
    ///
    /// The directory of the returned path can be changed by using the command
    /// line --gui_test_path option when running the test.
    std::string get_test_file_path_for(std::string const& basename) const;

    /// Return true if running in distribution testing mode.
    ///
    /// This method is used to partially skip execution of the tests that are
    /// specific to the binary program distribution. If the entire test should
    /// be skipped, prefer to use skip_if_not_distribution() instead.
    bool is_distribution_test() const;

    /// Skip the test if not running in distribution testing mode.
    ///
    /// This method can be used to skip execution of a test entirely unless
    /// --distribution command line option was specified.
    ///
    /// Throws test_skipped_exception if the distribution option was not given.
    void skip_if_not_distribution();

  protected:
    /// The argument must be a literal, as we just store the pointer.
    explicit wx_base_test_case(char const* name);

    char const* const m_name;
};

/// Define a test function and register it with the application tester.
///
/// Usage is:
///
///     LMI_WX_TEST_CASE(my_test)
///     {
///         ... code of the test ...
///     }
#define LMI_WX_TEST_CASE(name) \
class wx_test_case_##name \
    :public wx_base_test_case \
{ \
  public: \
    wx_test_case_##name() \
        :wx_base_test_case(#name) \
        { \
        } \
 \
    virtual void run(); \
}; \
static wx_test_case_##name wx_test_case_##name##_instance; \
void wx_test_case_##name::run()

#endif // wx_test_case_hpp
