// The path class.
//
// Copyright (C) 2020 Gregory W. Chicares.
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

#include "so_attributes.hpp"

#include <filesystem>
#include <fstream>
#include <ostream>
#include <string>
#include <system_error> // std::error_code

namespace fs
{

/// Using the classes from std::filesystem.

using std::filesystem::directory_entry;
using std::filesystem::directory_iterator;
using std::filesystem::file_time_type;
using std::filesystem::filesystem_error;

/// Using the functions, which takes the path as an argument to
/// allow passing std::string to them.

using std::filesystem::create_directory;
using std::filesystem::exists;
using std::filesystem::is_directory;
using std::filesystem::last_write_time;
using std::filesystem::remove;
using std::filesystem::rename;

/// The path class.
///
/// Motivation.
/// Implement own operator<<, which doesn't wrap the path with the double
/// quotes, like std::filesystem::path::operator<< does.
/// Also always use UTF8 encoding when construct from the string and allow
/// convetring to the string only with UFT8 encoding.

class LMI_SO path
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

    path& replace_filename(const path& replacement)
        {
        path_.replace_filename(replacement.path_);
        return *this;
        }

    path& replace_extension(const path& replacement = path())
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

    /// The function is used only in native_path(). Returns the UFT8 string
    /// with the native separators ('\\' under Windows).
    /// Note that the corresponding function doesn't exist in
    /// std::filesystem::path class.

    std::string native_string() const
        {
        // make_preferred() changes the path, so make copy of path_.
        std::filesystem::path tmp{path_};
        return u8string_as_string(tmp.make_preferred().u8string());
        }
 
    /// Returns the UFT8 encoded string in the lexically normalized generic
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

    path lexically_relative(const path& base) const
        {
        return path_.lexically_relative(base.path_);
        }

    path lexically_proximate(const path& base) const
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

#ifdef __cpp_char8_t
    static std::string u8string_as_string(std::u8string const& s8)
        {
        // In C++20 u8string is a specialization of basic_string<> for char8_t
        // which is a different type from char, hence u8string cannot be used
        // for basically anything. In lmi we assume that all non-ASCII strings
        // use UTF-8 encoding, so we simply reuse u8string contents as normal
        // char string. Note that this case is safe because of special property
        // of char pointers that can be used to iterate over any buffer and
        // that u8string contents can always be stored in string (unlike vice
        // versa).
        //
        // Also note that the input string can't contain embedded NULs here, as
        // they're not allowed in file paths, hence there is no need to use
        // size.
        return reinterpret_cast<char const*>(s8.c_str());
        }
#else
    // Until C++20 u8string() and generic_u8string() return std::string, so
    // there is no need to do anything and, hopefully, the compiler will just
    // optimize this function call away.
    static std::string u8string_as_string(std::string const& s8)
        {
        return s8;
        }
#endif

    friend bool operator==(const path& lhs, const path& rhs) noexcept;
    friend bool operator!=(const path& lhs, const path& rhs) noexcept;
    friend bool operator<(const path& lhs, const path& rhs) noexcept;
    friend bool operator<=(const path& lhs, const path& rhs) noexcept;
    friend bool operator>(const path& lhs, const path& rhs) noexcept;
    friend bool operator>=(const path& lhs, const path& rhs) noexcept;
    friend path operator/(const path& lhs, const path& rhs);
    friend class ifstream;
    friend class ofstream;
};

/// Non-member operators.

inline bool operator==(const path& lhs, const path& rhs) noexcept
{
    return lhs.path_ == rhs.path_;
}

inline bool operator!=(const path& lhs, const path& rhs) noexcept
{
    return lhs.path_ != rhs.path_;
}

inline bool operator<(const path& lhs, const path& rhs) noexcept
{
    return lhs.path_ < rhs.path_;
}

inline bool operator<=(const path& lhs, const path& rhs) noexcept
{
    return lhs.path_ <= rhs.path_;
}

inline bool operator>(const path& lhs, const path& rhs) noexcept
{
    return lhs.path_ > rhs.path_;
}

inline bool operator>=(const path& lhs, const path& rhs) noexcept
{
    return lhs.path_ >= rhs.path_;
}

inline path operator/(const path& lhs, const path& rhs)
{
    return lhs.path_ / rhs.path_;
}

template<class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const path& p)
{
    return os << p.string();
}

/// Implement own functions, which return the path.

inline path absolute(const path& p)
{
    return std::filesystem::absolute(p);
}

inline path absolute(const path& p, std::error_code& ec)
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

/// File streams.
///
/// Motivation: our own file streams can be constructed from our own path
/// class. fs::path has `operator std::filesystem::path()`, but the following
/// code fails:
///
/// fs::path p{"file.ext"};
/// fs::ifstream s{p};
///
/// So implement our own classes for the exact match of constructor parameters.
///
/// Note that fs::path can be constructed from a string using fs::u8path()
/// which leads to differences with the standard file streams constructing:
///
/// std::string filename{"unicode_filename.ext"};
/// fs::ifstream fs1 {filename}; // The filename treated as UTF8 encoded.
/// std::ifstream fs2{filename}; // Used the current locale.

class LMI_SO ifstream final
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
    ifstream(ifstream&&) = default;

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

class LMI_SO ofstream final
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
    ofstream(ofstream&&) = default;

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
