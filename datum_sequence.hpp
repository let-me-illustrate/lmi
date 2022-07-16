// Input-sequence class for wx data-transfer framework.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef datum_sequence_hpp
#define datum_sequence_hpp

#include "config.hpp"

#include "datum_string.hpp"

#include "value_cast.hpp"

#include <map>
#include <string>

/// Abstract base class for MVC input sequences.
///
/// Sequences are formed of values and intervals. Intervals may always
/// be specified by numbers, keywords, or a combination of both. Each
/// sequence's semantics determines whether its allowable values may
/// be numbers, or keywords, or both; that's a fixed property of each
/// derived class.
///
/// For some sequences, no keywords are defined, and therefore none
/// are ever permitted. It is difficult, e.g., to conceive of a
/// keyword that would be useful for 7702A amounts-paid history.
///
/// For others, only keywords can be used, and numbers are never
/// permitted. Payment mode, e.g., is chosen from an enumerated list,
/// and numbers would at best be ambiguous synonyms:
///  -  1=annual, 12=monthly // payments per least-frequent mode
///  - 12=annual,  1=monthly // payments per  most-frequent mode
///  -  1=A, 2=S, 3=Q, 4=M   // order in which they might be listed
///
/// Still others permit both numbers and keywords. Specified amount,
/// e.g., must accommodate numeric entry.

class sequence_base
    :public datum_string_base
{
  public:
    bool equals(sequence_base const&) const;

    virtual bool numeric_values_are_allowable() const = 0;
    virtual bool keyword_values_are_allowable() const = 0;
    virtual std::string const default_keyword() const;
    virtual std::map<std::string,std::string> const allowed_keywords() const;

  protected:
    sequence_base();
    explicit sequence_base(std::string const&);

    sequence_base(sequence_base const&) = default;
    sequence_base(sequence_base&&) = default;
    sequence_base& operator=(sequence_base const&) = default;
    sequence_base& operator=(sequence_base&&) = default;
    ~sequence_base() override = default;

    sequence_base& operator=(std::string const&);

    void assert_sanity() const;
};

bool operator==(sequence_base const&, sequence_base const&);

// Specialize value_cast<> for each derived class, e.g., as follows:
//
// template<> inline leaf_class value_cast<leaf_class,std::string>
//     (std::string const& from)
// {
//     return leaf_class(from);
// }
//
// template<> inline std::string value_cast<std::string,leaf_class>
//     (leaf_class const& from)
// {
//     return from.value();
// }

/// Numeric MVC input sequence.

class numeric_sequence final
    :public sequence_base
{
  public:
    numeric_sequence();
    explicit numeric_sequence(std::string const&);

    numeric_sequence& operator=(std::string const&);

    bool numeric_values_are_allowable() const override {return true;}
    bool keyword_values_are_allowable() const override {return false;}
    std::map<std::string,std::string> const allowed_keywords() const override;
};

bool operator==(numeric_sequence const&, numeric_sequence const&);

template<> inline numeric_sequence value_cast<numeric_sequence,std::string>
    (std::string const& from)
{
    return numeric_sequence(from);
}

template<> inline std::string value_cast<std::string,numeric_sequence>
    (numeric_sequence const& from)
{
    return from.value();
}

/// MVC input sequence for payments.

class payment_sequence final
    :public sequence_base
{
  public:
    payment_sequence();
    explicit payment_sequence(std::string const&);

    payment_sequence& operator=(std::string const&);

    bool numeric_values_are_allowable() const override {return true;}
    bool keyword_values_are_allowable() const override {return true;}
    std::map<std::string,std::string> const allowed_keywords() const override;
};

bool operator==(payment_sequence const&, payment_sequence const&);

template<> inline payment_sequence value_cast<payment_sequence,std::string>
    (std::string const& from)
{
    return payment_sequence(from);
}

template<> inline std::string value_cast<std::string,payment_sequence>
    (payment_sequence const& from)
{
    return from.value();
}

/// MVC input sequence for payment mode.

class mode_sequence final
    :public sequence_base
{
  public:
    mode_sequence();
    explicit mode_sequence(std::string const&);

    mode_sequence& operator=(std::string const&);

    bool numeric_values_are_allowable() const override {return false;}
    bool keyword_values_are_allowable() const override {return true;}
    std::string const default_keyword() const override;
    std::map<std::string,std::string> const allowed_keywords() const override;
};

bool operator==(mode_sequence const&, mode_sequence const&);

template<> inline mode_sequence value_cast<mode_sequence,std::string>
    (std::string const& from)
{
    return mode_sequence(from);
}

template<> inline std::string value_cast<std::string,mode_sequence>
    (mode_sequence const& from)
{
    return from.value();
}

/// MVC input sequence for specified amount.

class specamt_sequence final
    :public sequence_base
{
  public:
    specamt_sequence();
    explicit specamt_sequence(std::string const&);

    specamt_sequence& operator=(std::string const&);

    bool numeric_values_are_allowable() const override {return true;}
    bool keyword_values_are_allowable() const override {return true;}
    std::map<std::string,std::string> const allowed_keywords() const override;
};

bool operator==(specamt_sequence const&, specamt_sequence const&);

template<> inline specamt_sequence value_cast<specamt_sequence,std::string>
    (std::string const& from)
{
    return specamt_sequence(from);
}

template<> inline std::string value_cast<std::string,specamt_sequence>
    (specamt_sequence const& from)
{
    return from.value();
}

/// MVC input sequence for death benefit option.

class dbo_sequence final
    :public sequence_base
{
  public:
    dbo_sequence();
    explicit dbo_sequence(std::string const&);

    dbo_sequence& operator=(std::string const&);

    bool numeric_values_are_allowable() const override {return false;}
    bool keyword_values_are_allowable() const override {return true;}
    std::string const default_keyword() const override;
    std::map<std::string,std::string> const allowed_keywords() const override;
};

bool operator==(dbo_sequence const&, dbo_sequence const&);

template<> inline dbo_sequence value_cast<dbo_sequence,std::string>
    (std::string const& from)
{
    return dbo_sequence(from);
}

template<> inline std::string value_cast<std::string,dbo_sequence>
    (dbo_sequence const& from)
{
    return from.value();
}

#endif // datum_sequence_hpp
