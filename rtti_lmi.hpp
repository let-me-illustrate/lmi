// Extensions to C++ run-time type information.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef rtti_lmi_hpp
#define rtti_lmi_hpp

#include "config.hpp"

#if defined __GNUC__
#   include <cstdlib>                   // free()
#   include <cxxabi.h>
#endif // defined __GNUC__

#include <ostream>
#include <string>
#include <typeinfo>

class RttiLmiTest;

namespace lmi
{

  namespace detail
  {
    /// Demangle C++ names. Only gcc is supported at this time, and
    /// only from version 4, because the gcc-3.x demangler is so
    /// gravely broken that it actually segfaults on some input: see
    ///   http://article.gmane.org/gmane.comp.gnu.mingw.user/19874/
    ///     [2006-06-08T16:41:35Z from Greg Chicares]
    ///   http://sf.net/tracker/?func=detail&atid=102435&aid=1504421&group_id=2435

#if defined __GNUC__ && 40000 <= LMI_GCC_VERSION
    inline std::string Demangle(char const* mangled_name)
    {
        int status = 0;
        char* demangled_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
        std::string s = (0 == status) ? demangled_name : mangled_name;
        std::free(demangled_name);
        return s;
    }
#else  // gcc-3.x or prior, or not gcc at all.
    inline std::string Demangle(char const* mangled_name) {return mangled_name;}
#endif // gcc-3.x or prior, or not gcc at all.
  } // namespace detail

/// This wrapper makes std::type_info generally useful.
///
/// Class lmi::TypeInfo meets the following requirements:
///   Assignable
///   CopyConstructible
///   EqualityComparable
///   LessThanComparable
/// and may therefore be used, e.g., in containers and with standard
/// algorithms. It is not
///   DefaultConstructible
/// because that does not seem useful. It is
///   StreamInsertible
/// but not
///   StreamExtractible
/// because the result of formatting a std::type_info object in the
/// most natural way, with name(), is not reversible.
///
/// Some discussion of std::type_info is in order here, given that a
/// misuse in a widely-read book by a celebrated author
///   http://www.aristeia.com/BookErrata/mec++-errata_frames.html
/// went unreported for over six years: see the 2002-05-07 item that
/// explains why the addresses of std::type_info objects should not
/// be compared directly.
///
/// Explicitly-defaulted special member functions do the right thing.
/// It may seem odd to say that of a class with a pointer member, but
/// std::type_info is not Copyable, so a deep copy is not possible;
/// neither is it necessary, because a std::type_info object is, in
/// effect, a smart pointer managed by the implementation.
///   https://web.archive.org/web/20200115093104/http://www.two-sdg.demon.co.uk/curbralan/papers/RTTI.html
/// "This is one of the few cases in C++ programming where it is
/// reasonable for a programmer to take the address associated with a
/// reference and hold onto it for later use:
///   type_info [const]* info = &typeid(*ptr);
/// ", as long as the pointer is dereferenced before use in equality
/// comparisons.
///
/// The ctor taking a std::type_info argument is not explicit because
/// the typeid operator may return an object whose dynamic type is,
/// e.g., std::extended_type_info [5.2.8/1].
///
/// It is not possible to emulate operator typeid, because it traps
/// certain attempts to dereference a null pointer operand. No ctor
///   template<typename T>
///   TypeInfo(T t): ti_(&typeid(t)) {}
/// is provided for this reason, and also to avoid any seeming
/// ambiguity in self-referential cases:
///   int i;
///   lmi::TypeInfo ti(typeid(i));
///   lmi::TypeInfo(i);   // Would refer to typeid(int).
///   lmi::TypeInfo(&i);  // Would refer to typeid(int*).
///   lmi::TypeInfo(ti);  // Must  refer to typeid(int).
///   lmi::TypeInfo(&ti); // Would refer to typeid(lmi::TypeInfo*).
/// Those problems are avoided by requiring the idiomatic usage
///   lmi::TypeInfo(typeid(X));
/// and resisting the temptation to add syntactic sugar.

class TypeInfo final
{
    friend class ::RttiLmiTest;

  public:
    TypeInfo(std::type_info const& z): ti_(&z) {}

    bool operator==(TypeInfo const& z) const {return *z.ti_ == *ti_;}
    bool  operator<(TypeInfo const& z) const {return ti_->before(*z.ti_);}

    std::string Name() const {return detail::Demangle(ti_->name());}

  private:
    std::type_info const* ti_;
};

inline std::ostream& operator<<(std::ostream& os, TypeInfo const& z)
{
    return os << z.Name();
}

} // namespace lmi

#endif // rtti_lmi_hpp
