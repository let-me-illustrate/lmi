// The file system path class for lmi.
//
// Copyright (C) 2020, 2021 Gregory W. Chicares.
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

#ifndef path_hpp
#define path_hpp

#include "config.hpp"

#include <filesystem>
#include <ostream>
#include <string>
#include <system_error> // std::error_code

namespace fs
{

/// Make several standard classes and functions available in this namespace.

using std::filesystem::directory_entry;
using std::filesystem::directory_iterator;
using std::filesystem::file_time_type;
using std::filesystem::filesystem_error;

using std::filesystem::create_directory;
using std::filesystem::exists;
using std::filesystem::is_directory;
using std::filesystem::last_write_time;
using std::filesystem::remove;
using std::filesystem::rename;

/// Class representing the file system path.
///
/// This class is as similar as possible to std::filesystem::path, while
/// being different from it in two crucial aspects:
///
///  1. Having a different class allows to provide a more user-friendly
///     operator<<() for it which does not quote the path, as the standard
///     class does.
///
///  2. Our class always uses UTF-8 encoding for all strings, whether we use
///     C++17 or C++20, while the standard class was changed in an incompatible
///     way in the latter standard version, might change again in the future,
///     and in the meanwhile the behaviour of the existing implementations of
///     the standard library depends on the exact version and is also subject
///     to change.
///
/// Unfortunately this does require duplicating the standard class here, but
/// this class is guaranteed to provide a (strict, as we don't implement some
/// of the parts that we don't need at all) subset of std::filesystem::path
/// functionality, so when the problems above are not relevant any longer, it
/// should be possible to simply replace it with a using declaration making the
/// standard class available in this namespace.

class path
{
  public:
    /// Constructors and destructor.

    path() noexcept = default;

    path(path const& p)
        :path_{p.path_}
        {
        }

    path(path&& p) noexcept
        :path_{std::move(p.path_)}
        {
        }

    path(std::filesystem::path const& p)
        :path_{p}
        {
        }

    path(std::filesystem::path&& p) noexcept
        :path_{std::move(p)}
        {
        }

    path(std::string const& source)
        :path_{std::filesystem::u8path(source)}
        {
        }

    path(char const* source)
        :path_{std::filesystem::u8path(source)}
        {
        }

    ~path() = default;

    /// Assignments.

    path& operator=(path const& p)
        {
        path_ = p.path_;
        return *this;
        }

    path& operator=(path&& p) noexcept
        {
        path_ = std::move(p.path_);
        return *this;
        }

    path& operator=(std::filesystem::path const& p) noexcept
        {
        path_ = p;
        return *this;
        }

    path& operator=(std::filesystem::path&& p) noexcept
        {
        path_ = std::move(p);
        return *this;
        }

    path& operator=(std::string const& source)
        {
        path_ = std::filesystem::u8path(source);
        return *this;
        }

    path& operator=(char const* source)
        {
        path_ = std::filesystem::u8path(source);
        return *this;
        }

    /// Appends.

    path& operator/=(path const& p)
        {
        path_ /= p.path_;
        return *this;
        }

    path& operator/=(std::string const& source)
        {
        path_ /= source;
        return *this;
        }

    /// Modifiers.

    void clear() noexcept
        {
        path_.clear();
        }

    path& make_preferred()
        {
        path_.make_preferred();
        return *this;
        }

    path& remove_filename()
        {
        path_.remove_filename();
        return *this;
        }

    path& replace_filename(path const& replacement)
        {
        path_.replace_filename(replacement.path_);
        return *this;
        }

    path& replace_extension(path const& replacement = path())
        {
        path_.replace_extension(replacement.path_);
        return *this;
        }

    void swap(path& rhs) noexcept
        {
        path_.swap(rhs.path_);
        }

    /// Native format observer.

    operator std::filesystem::path() const
        {
        return path_;
        }

    /// The function is used only in native_path(). Returns the UTF-8 string
    /// with the native separators ('\\' under Windows).
    /// Note that the corresponding function doesn't exist in
    /// std::filesystem::path class.

    std::string native_string() const
        {
        // make_preferred() changes the path, so make copy of path_.
        std::filesystem::path tmp{path_};
        return tmp.make_preferred().u8string();
        }

    /// Returns the UTF-8 encoded string in the lexically normalized generic
    /// format. The functionality differs from the standard one to use the
    /// short and simple function name which is used in most of cases.

    std::string string() const
        {
        return path_.lexically_normal().generic_u8string();
        }

    /// Generation.

    path lexically_normal() const
        {
        return path_.lexically_normal();
        }

    path lexically_relative(path const& base) const
        {
        return path_.lexically_relative(base.path_);
        }

    path lexically_proximate(path const& base) const
        {
        return path_.lexically_proximate(base.path_);
        }

    /// Decomposition.

    path root_name() const
        {
        return path_.root_name();
        }

    path root_directory() const
        {
        return path_.root_directory();
        }

    path root_path() const
        {
        return path_.root_path();
        }

    path relative_path() const
        {
        return path_.relative_path();
        }

    path parent_path() const
        {
        return path_.parent_path();
        }

    path filename() const
        {
        return path_.filename();
        }

    path stem() const
        {
        return path_.stem();
        }

    path extension() const
        {
        return path_.extension();
        }

    /// Query.

    [[nodiscard]] bool empty() const noexcept
        {
        return path_.empty();
        }

    [[nodiscard]] bool has_root_name() const
        {
        return path_.has_root_name();
        }

    [[nodiscard]] bool has_root_directory() const
        {
        return path_.has_root_directory();
        }

    [[nodiscard]] bool has_root_path() const
        {
        return path_.has_root_path();
        }

    [[nodiscard]] bool has_relative_path() const
        {
        return path_.has_relative_path();
        }

    [[nodiscard]] bool has_parent_path() const
        {
        return path_.has_parent_path();
        }

    [[nodiscard]] bool has_filename() const
        {
        return path_.has_filename();
        }

    [[nodiscard]] bool has_stem() const
        {
        return path_.has_stem();
        }

    [[nodiscard]] bool has_extension() const
        {
        return path_.has_extension();
        }

    [[nodiscard]] bool is_absolute() const
        {
        return path_.is_absolute();
        }

    [[nodiscard]] bool is_relative() const
        {
        return path_.is_relative();
        }

  private:
    std::filesystem::path path_;

    friend bool operator==(path const& lhs, path const& rhs) noexcept;
    friend bool operator!=(path const& lhs, path const& rhs) noexcept;
    friend bool operator<(path const& lhs, path const& rhs) noexcept;
    friend bool operator<=(path const& lhs, path const& rhs) noexcept;
    friend bool operator>(path const& lhs, path const& rhs) noexcept;
    friend bool operator>=(path const& lhs, path const& rhs) noexcept;
    friend path operator/(path const& lhs, path const& rhs);
    friend class ifstream;
    friend class ofstream;
};

/// Non-member operators.

inline bool operator==(path const& lhs, path const& rhs) noexcept
{
    return lhs.path_ == rhs.path_;
}

inline bool operator!=(path const& lhs, path const& rhs) noexcept
{
    return lhs.path_ != rhs.path_;
}

inline bool operator<(path const& lhs, path const& rhs) noexcept
{
    return lhs.path_ < rhs.path_;
}

inline bool operator<=(path const& lhs, path const& rhs) noexcept
{
    return lhs.path_ <= rhs.path_;
}

inline bool operator>(path const& lhs, path const& rhs) noexcept
{
    return lhs.path_ > rhs.path_;
}

inline bool operator>=(path const& lhs, path const& rhs) noexcept
{
    return lhs.path_ >= rhs.path_;
}

inline path operator/(path const& lhs, path const& rhs)
{
    return lhs.path_ / rhs.path_;
}

template<class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, path const& p)
{
    return os << p.string();
}

/// Provide wrappers of the functions in std::filesystem returning standard
/// path objects returning the same objects of our own type.

inline path absolute(path const& p)
{
    return std::filesystem::absolute(p);
}

inline path absolute(path const& p, std::error_code& ec)
{
    return std::filesystem::absolute(p, ec);
}

inline path current_path()
{
    return std::filesystem::current_path();
}

inline path current_path(std::error_code& ec)
{
    return std::filesystem::current_path(ec);
}

} // namespace fs

#endif // path_hpp
