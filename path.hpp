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
#include <fstream>
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
        :path_{from_u8path(source)}
        {
        }

    path(char const* source)
        :path_{from_u8path(source)}
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
        path_ = from_u8path(source);
        return *this;
        }

    path& operator=(char const* source)
        {
        path_ = from_u8path(source);
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
        return u8string_as_string(tmp.make_preferred().u8string());
        }

    /// Returns the UTF-8 encoded string in the lexically normalized generic
    /// format. The functionality differs from the standard one to use the
    /// short and simple function name which is used in most of cases.

    std::string string() const
        {
        return u8string_as_string(path_.lexically_normal().generic_u8string());
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

#if defined __cpp_char8_t
    static std::string u8string_as_string(std::u8string const& s8)
        {
        // In C++20 u8string is a specialization of basic_string<> for char8_t
        // which is a different type from char, hence u8string cannot be used
        // for basically anything. In lmi we assume that all non-ASCII strings
        // use UTF-8 encoding, so we simply reuse u8string contents as normal
        // char string. Note that this cast is safe because of special property
        // of char pointers that can be used to iterate over any buffer and
        // that u8string contents can always be stored in string (unlike vice
        // versa).
        //
        // Also note that the input string can't contain embedded NULs here, as
        // they're not allowed in file paths, hence there is no need to use
        // size.
        return reinterpret_cast<char const*>(s8.c_str());
        }
#endif // defined __cpp_char8_t

    // Until C++20 u8string() and generic_u8string() return std::string, so
    // there is no need to do anything and, hopefully, the compiler will just
    // optimize this function call away.
    //
    // Note that we define this overload even when using C++20, as some
    // std::filesystem implementations haven't been updated to return
    // std::u8string from path::u8string() (which some have already been), so
    // it's simpler to always define it and let the compiler select the
    // appropriate overload to call.
    static std::string u8string_as_string(std::string const& s8)
        {
        return s8;
        }

    // Provide wrapper for u8path() C++17 function deprecated in C++20:
    // normally we should just use std::filesystem::path ctor from char8_t
    // string here, but not all implementations provide it right now (notably
    // libc++ used by clang 10 in the CI builds does not), so keep using
    // u8path() for now, and just disable the deprecation warning for it.
    template<typename T>
    static std::filesystem::path from_u8path(T arg)
        {
#if defined LMI_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif // defined LMI_CLANG
#if defined LMI_MSC
#   pragma warning(push)
#   pragma warning(disable : 4996)
#endif // defined LMI_MSC

        return std::filesystem::u8path(arg);

#if defined LMI_MSC
#   pragma warning(pop)
#endif // defined LMI_MSC
#if defined LMI_CLANG
#   pragma clang diagnostic pop
#endif // defined LMI_CLANG
        }

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

/// File stream classes working with fs::path.
///
/// These classes are equivalents of the standard classes with the same name
/// that can be constructed from std::filesystem::path objects, but not from
/// fs::path objects in a context in which implicit conversions, such as those
/// performed by fs::path operator std::filesystem::path(), are disabled. I.e.
/// these classes allow the following code to compile
///
/// fs::path p = ...;
/// fs::ifstream ifs{p};
///
/// while with std::ifstream only
///
/// std::ifstream ifs(p);
///
/// could be used or an explicit conversion would be required.
///
/// Another advantage of using these classes is that, like fs::path itself,
/// they interpret all strings as being in UTF-8, while the standard file
/// streams use the current locale encoding for them.

class ifstream final
    :public std::ifstream
{
  public:
    ifstream() = default;

    explicit ifstream
        (path const& p
        ,std::ios_base::openmode mode = std::ios_base::in
        )
        :std::ifstream(p.path_, mode)
        {
        }

    ifstream(ifstream const&) = delete;
    ifstream(ifstream&&) = delete;

    void open
        (path const& p
        ,std::ios_base::openmode mode = std::ios_base::in
        )
        {
            std::ifstream::open
                (p.path_
                ,mode
                );
        }
};

class ofstream final
    :public std::ofstream
{
  public:
    ofstream() = default;

    explicit ofstream
        (path const& p
        ,std::ios_base::openmode mode = std::ios_base::out
        )
        :std::ofstream(p.path_, mode)
        {
        }

    ofstream(ofstream const&) = delete;
    ofstream(ofstream&&) = delete;

    void open
        (path const& p
        ,std::ios_base::openmode mode = std::ios_base::out
        )
        {
        std::ofstream::open
            (p.path_
            ,mode
            );
        }
};

} // namespace fs

#endif // path_hpp
