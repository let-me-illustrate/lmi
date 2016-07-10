// Provide C++ wrappers around PCRE2 C API.
//
// Copyright (C) 2021 Gregory W. Chicares.
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

#ifndef pcre_regex_hpp
#define pcre_regex_hpp

#include "config.hpp"

/// Currently we always use 8-bit version of PCRE2 library and assume UTF-8
/// input encoding. This code could be easily made generic and work with wide
/// character strings using UTF-16 or UTF-32, depending on the size of wchar_t
/// on the current platform, but there isn't any need for this, so keep things
/// simple for now.
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace pcre
{

namespace detail
{

// This private class is just a RAII wrapper for pcre2_match_data.
class match_data
{
  public:
    match_data() = default;

    explicit match_data(pcre2_code* code)
        {
        data_ = pcre2_match_data_create_from_pattern(code, nullptr);
        if(!data_)
            {
            throw std::runtime_error("pcre2_match_data_create_from_pattern failed");
            }
        }

    match_data(match_data const&) = delete;
    match_data& operator=(match_data const&) = delete;

    match_data(match_data&& m) noexcept
        {
        data_   = m.data_;
        m.data_ = nullptr;
        }

    match_data& operator=(match_data&& m) noexcept
        {
        pcre2_match_data_free(data_); // Guaranteed NOP if data_ is null.

        data_   = m.data_;
        m.data_ = nullptr;

        return *this;
        }

    ~match_data()
        {
        pcre2_match_data_free(data_);
        }

    [[nodiscard]] pcre2_match_data* as_pcre() const noexcept { return data_; }

  private:
    pcre2_match_data* data_ {nullptr};
};

} // namespace detail

/// Represents a single sub-expression match.
class sub_match
{
  public:
    sub_match() = default;

    [[nodiscard]] std::size_t start_pos() const noexcept { return start_; }
    [[nodiscard]] std::size_t end_pos()   const noexcept { return end_;   }

    [[nodiscard]] bool empty() const noexcept { return start_ == end_; }

    [[nodiscard]] std::string_view as_view() const noexcept
        {
        return { view_.data() + start_, end_ - start_ };
        }

    [[nodiscard]] operator std::string() const
        {
        return std::string{as_view()};
        }

    [[nodiscard]] bool operator==(std::string_view const& view) const noexcept
        {
        return as_view() == view;
        }

    [[nodiscard]] bool operator==(char c) const noexcept
        {
        return as_view() == std::string_view{&c, 1};
        }

  private:
    friend class match_results;

    sub_match(std::string_view const& view, PCRE2_SIZE start, PCRE2_SIZE end)
        :view_{view}
        ,start_{start}
        ,end_{end}
        {
        }

    std::string_view view_     ;
    std::size_t start_      {0};
    std::size_t end_        {0};
};

inline
std::ostream& operator<<(std::ostream& os, sub_match const& sm)
{
    return os << sm.as_view();
}

/// Represents the results of matching a regular expression.
class match_results : private detail::match_data
{
  public:
    match_results() = default;
    match_results(match_results const&) = delete;
    match_results& operator=(match_results const&) = delete;
    match_results(match_results&& m) = default;
    match_results& operator=(match_results&& m) = default;
    ~match_results() = default;

    [[nodiscard]] bool empty() const noexcept
        {
        return num_ == 0;
        }

    [[nodiscard]] explicit operator bool() const noexcept
        {
        return !empty();
        }

    [[nodiscard]] int size() const noexcept
        {
        return num_;
        }

    [[nodiscard]] sub_match operator[](int n) const
    {
        // Using this operator after failing to match succeeds by returning
        // nothing for compatibility with Boost.Regex.
        if(empty())
            return {};

        if(n < 0 || n >= size())
            {
            std::ostringstream oss;
            oss
                << "Invalid captured expression index " << n
                << " out of range 0.." << size()
                ;

            throw std::runtime_error(oss.str());
            }

        // 'ovector' is a vector of pairs of offsets.
        auto const ovector = pcre2_get_ovector_pointer(as_pcre());

        auto const start = ovector[2*n    ];
        auto const end   = ovector[2*n + 1];

        // It is possible for the start to be greater than end when using \K,
        // but we don't need to support it for now.
        if(start > end)
            throw std::runtime_error(R"(captures with \K not supported)");

        // Sub-pattern could have been unused and unset.
        if(start == PCRE2_UNSET)
            return {};

        // Note that this view could still be empty, but at least it is
        // guaranteed to be valid in this case.
        return { view_, start, end };
    }

  private:
    friend class regex;

    // Passing 'num' to the ctor is redundant as it could also be obtained from
    // match_data using pcre2_get_ovector_count(), but as we already have it in
    // the code creating match_results, we take it here as a micro optimization
    // to avoid calling this (admittedly, trivial) function unnecessarily.
    match_results(detail::match_data&& data, int num, std::string_view const& view)
        :detail::match_data{std::move(data)}
        ,num_{num}
        ,view_{view}
        {
        }

    // Number of subexpressions.
    int num_ {0};

    std::string_view view_;
};

/// Provides a way to define and use regular expressions using vaguely
/// Boost.Regex-compatible API.
class regex
{
  public:
    enum
        {ECMAScript = 0 // Does nothing, for compatibility with std::regex only.
        ,icase      = 1
        };

    // Define a Boost.Regex-compatible type.
    using flag_type = int;

    explicit regex(std::string_view const& pattern, int flags = 0)
    {
        // We always use PCRE2_DOTALL for compatibility with Boost.Regex.
        uint32_t options = PCRE2_DOTALL;
        if(flags & icase)
            {
            options |= PCRE2_CASELESS;
            }

        int errorcode = 0;
        PCRE2_SIZE erroroffset = 0;

        code_ = pcre2_compile
            (reinterpret_cast<PCRE2_SPTR>(pattern.data())
            ,pattern.size()
            ,options
            ,&errorcode
            ,&erroroffset
            ,nullptr    // default compile context
            );

        if(!code_)
            {
            std::ostringstream oss;
            oss
                << "Compiling regular expression \"" << pattern << "\" failed "
                << "at offset " << erroroffset << ": "
                << get_error_message(errorcode)
                ;

            throw std::runtime_error(oss.str());
            }
    }

    regex(regex const&) = delete;
    regex& operator=(regex const&) = delete;

    regex(regex&& rx) noexcept
        {
        code_ = rx.code_;
        rx.code_ = nullptr;
        }

    regex& operator=(regex&& rx) noexcept
        {
        pcre2_code_free(code_); // Guaranteed NOP if code_ is null.

        code_ = rx.code_;
        rx.code_ = nullptr;

        return *this;
        }

    ~regex()
        {
        pcre2_code_free(code_);
        }

    [[nodiscard]] match_results search(std::string_view const& subject) const
    {
        detail::match_data md{code_};

        int const rc = pcre2_match
            (code_
            ,reinterpret_cast<PCRE2_SPTR>(subject.data())
            ,subject.size()
            ,0              // starting offset
            ,0              // no options
            ,md.as_pcre()
            ,nullptr        // default match_results context
            );

        if(rc < 0)
            {
            switch(rc)
                {
                case PCRE2_ERROR_NOMATCH:
                    return match_results{};

                default:
                    std::ostringstream oss;
                    oss
                        << "Matching regular expression failed: "
                        << get_error_message(rc)
                        ;
                    throw std::runtime_error(oss.str());
                }
            }

        return { std::move(md), rc, subject };
    }

    [[nodiscard]] std::string replace
        (std::string_view const& subject
        ,std::string_view const& replacement
        ) const
        {
        // Assume that the replacement won't be more than twice as long as the
        // original string.
        //
        // This is almost certainly not optimal, but it's not clear how could
        // we do better in the general case.
        PCRE2_SIZE outlen = 2*subject.size();

        std::vector<PCRE2_UCHAR> buffer(outlen);

        detail::match_data md{code_};

        // We may need to try twice, first with the estimated buffer size and
        // then with the actually needed one, so define the lambda performing
        // the actual PCRE2 function call in order to retry it, if necessary.
        auto const call_substitute =
            [this, subject, replacement, &outlen, &buffer, &md]() -> int
            {
            return pcre2_substitute
                (code_
                ,reinterpret_cast<PCRE2_SPTR>(subject.data())
                ,subject.size()
                ,0          // starting offset
                ,PCRE2_SUBSTITUTE_OVERFLOW_LENGTH
                ,md.as_pcre()
                ,nullptr    // default match_results context
                ,reinterpret_cast<PCRE2_SPTR>(replacement.data())
                ,replacement.size()
                ,&buffer[0]
                ,&outlen
                );
            };

        int rc = call_substitute();
        if(rc == PCRE2_ERROR_NOMEMORY)
            {
            // The length must have been updated to be big enough now.
            buffer.resize(outlen);

            rc = call_substitute();
            }

        if(rc < 0)
            {
            std::ostringstream oss;
            oss
                << "Substituting regular expression failed: "
                << get_error_message(rc)
                ;
            throw std::runtime_error(oss.str());
            }

        return std::string{reinterpret_cast<char*>(&buffer[0]), outlen};
        }

  private:
    // Return the error message corresponding to the given error code.
    [[nodiscard]] static std::string get_error_message(int errorcode)
        {
        std::ostringstream oss;

        // PCRE documentation states that 128 characters should be enough for
        // all error messages, but we can afford being even more generous here.
        char buffer[256];
        const int rc = pcre2_get_error_message
            (errorcode
            ,reinterpret_cast<PCRE2_UCHAR*>(buffer)
            ,sizeof(buffer)
            );

        if(rc > 0)
            {
            oss << std::string_view{buffer, static_cast<std::size_t>(rc)};
            }
        else
            {
            switch(rc)
                {
                case PCRE2_ERROR_BADDATA:
                    oss << "unknown PCRE error " << errorcode;
                    break;

                case PCRE2_ERROR_NOMEMORY:
                    // Still show the part of the error message we could
                    // retrieve (which is guaranteed to be NUL-terminated).
                    oss
                        << std::string(buffer)
                        << " (error message was unexpectedly truncated)"
                        ;
                    break;

                default:
                    // We can do nothing but give up in this case.
                    oss
                        << "obtaining error message for PCRE error "
                        << errorcode
                        << " failed with error "
                        << rc
                        ;
                }
            }

        return oss.str();
        }

    pcre2_code* code_ {nullptr};
};

/// Container for match results returned by search_all().
class all_match_results
{
  public:
    all_match_results(std::string_view const& subject, regex const& rx)
        :subject_{subject}
        ,rx_{rx}
        {
        }

    class iterator
    {
      public:
        iterator() = default;
        iterator(iterator const&) = delete;
        iterator& operator=(iterator const&) = delete;

        [[nodiscard]] match_results const& operator*() noexcept
            {
            return match_;
            }

        iterator& operator++() noexcept
            {
            next();

            return *this;
            }

        [[nodiscard]] bool operator==(iterator const& other) const noexcept
            {
            // Only end iterators compare equally.
            return !results_ && !other.results_;
            }

      private:
        friend class all_match_results;

        explicit iterator(all_match_results* results) noexcept
            :results_{results}
            {
            next();
            }

        void next() noexcept
            {
            // Incrementing end iterator is not allowed and will crash.
            if(!results_->get_next_match(match_))
                results_ = nullptr;
            }

        // This pointer is null for the end iterator and valid otherwise.
        all_match_results* results_ {nullptr};

        // Always valid if results_ is non-null.
        match_results match_;
    };

    [[nodiscard]] iterator begin() noexcept { return iterator{this}; }
    [[nodiscard]] iterator end()   noexcept { return {};             }

  private:
    friend class iterator;

    [[nodiscard]] bool get_next_match(match_results& m) noexcept
        {
        if(subject_.empty())
            return false;

        m = rx_.search(subject_);
        if(!m)
            {
            subject_ = {};
            return false;
            }

        // Advance after this match.
        if(sub_match const sm = m[0]; !sm.empty())
            {
            // This is simple if it's not empty.
            subject_.remove_prefix(sm.end_pos());
            }
        else
            {
            // Generally speaking, to handle finding all occurrences of a
            // pattern that can match an empty string, we would need to retry
            // matching at the same offset with PCRE2_NOTEMPTY_ATSTART and
            // PCRE2_ANCHORED flags as explained in PCRE2 documentation.
            //
            // But for now keep things simple, as we don't use any such
            // patterns, and just advance to the next position (note that
            // because we never use PCRE2_NEWLINE_CRLF and similar, we also
            // don't need to worry about possibly advancing by 2 characters
            // when there is a newline at the current offset, as PCRE2
            // documentation also recommends and pcre2demo program shows).
            subject_.remove_prefix(1);
            }

        return true;
        }

    std::string_view subject_;
    regex const& rx_;
};

[[nodiscard]]
inline
match_results
search(std::string_view const& s, regex const& rx)
{
    return rx.search(s);
}

[[nodiscard]]
inline
std::string
replace(std::string_view const& s, regex const& rx, std::string_view const& r)
{
    return rx.replace(s, r);
}

[[nodiscard]]
inline
all_match_results
search_all(std::string_view const& s, regex const& rx)
{
    return all_match_results{s, rx};
}

} // namespace pcre

#endif // pcre_regex_hpp
