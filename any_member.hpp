// Symbolic member names.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// This is a derived work based on boost::any, which bears the following
// copyright and permissions notice:
// [boost::any notice begins]
// Copyright Kevlin Henney, 2000, 2001. All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives, and that no
// charge may be made for the software and its documentation except to cover
// cost of distribution.
//
// This software is provided "as is" without express or implied warranty.
// [boost::any notice ends]

// Modified 2001 by Gregory W. Chicares as discussed here:
//   http://groups.yahoo.com/group/boost/message/16626
// to support a symbolic member idiom. GWC further modified this code
// in 2004 and in any later year shown above; any defect in it should
// not reflect on Kevlin Henney's reputation.

// Design notes--cast member templates
//
// Member template exact_cast() is safe, but limited: it casts only
// to the object's exact original type. It is a private member, but
// a corresponding free function template is generally accessible.
//
// Function template member_cast() is designed for general use. It
// does everything exact_cast() does, and can also cast to a base
// class of the object's actual type if an appropriate specialization
// of class template reconstitutor is found.

#ifndef any_member_hpp
#define any_member_hpp

#include "config.hpp"

#include "any_entity.hpp"

#include "assert_lmi.hpp"
#include "obstruct_slicing.hpp"
#include "rtti_lmi.hpp"
#include "value_cast.hpp"

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits.hpp>
#else  // defined __BORLANDC__
#   define BOOST_STATIC_ASSERT(deliberately_ignored) class IgNoRe
#endif // defined __BORLANDC__

#include <boost/utility.hpp>

#include <algorithm> // std::lower_bound(), std::swap()
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

// Definition of class placeholder.

// The implicitly-defined copy ctor and copy assignment operator do
// the right thing.

// A virtual member template here would permit
//    template<typename X>
//    void placeholder::call(void (X::*)());
// but 14.5.2/3 doesn't allow virtual member templates. This message
//   http://groups.google.com/groups?selm=7f6de0%24t1t%241%40nnrp1.dejanews.com
// is one of the more complete in a thread discussing the rationale.

class placeholder
{
  public:
    virtual ~placeholder();
    virtual placeholder& assign(placeholder const&) = 0;
    virtual placeholder& assign(std::string const&) = 0;
    virtual placeholder* clone() const = 0;
    virtual bool equals(placeholder const&) const = 0;
    virtual std::string str() const = 0;
    virtual std::type_info const& type() const = 0;
#if defined LMI_MSC
    virtual void* defraud() const = 0;
#endif // defined LMI_MSC
};

// Implementation of class placeholder.

inline placeholder::~placeholder()
{}

// Definition of class holder.

template<typename ClassType, typename ValueType>
class holder
    :public placeholder
    ,private boost::noncopyable
{
    // Friendship is extended to class any_member only to support its
    // cast operations.
    template<typename T> friend class any_member;

  public:
    holder(ClassType*, ValueType const&);
    virtual ~holder();

    // placeholder required implementation.
    virtual holder& assign(placeholder const&);
    virtual holder& assign(std::string const&);
    virtual placeholder* clone() const;
    virtual bool equals(placeholder const&) const;
    virtual std::string str() const;
    virtual std::type_info const& type() const;
#if defined LMI_MSC
    virtual void* defraud() const;
#endif // defined LMI_MSC

  private:
    ClassType* object_;
    ValueType held_;
};

// Implementation of class holder.

template<typename ClassType, typename ValueType>
holder<ClassType,ValueType>::holder(ClassType* object, ValueType const& value)
    :object_(object)
    ,held_(value)
{}

template<typename ClassType, typename ValueType>
holder<ClassType,ValueType>::~holder()
{}

template<typename ClassType, typename ValueType>
holder<ClassType,ValueType>& holder<ClassType,ValueType>::assign
    (placeholder const& other
    )
{
    LMI_ASSERT(other.type() == type());
    typedef holder<ClassType,ValueType> holder_type;
    holder_type const& z = static_cast<holder_type const&>(other);
    LMI_ASSERT(z.object_);
    LMI_ASSERT(object_);
    object_->*held_ = (z.object_)->*(z.held_);
    return *this;
}

template<typename ClassType, typename ValueType>
holder<ClassType,ValueType>& holder<ClassType,ValueType>::assign
    (std::string const& s
    )
{
    LMI_ASSERT(object_);
    object_->*held_ = value_cast(s, object_->*held_);
    return *this;
}

template<typename ClassType, typename ValueType>
placeholder* holder<ClassType,ValueType>::clone() const
{
    return new holder(object_, held_);
}

template<typename ClassType, typename ValueType>
bool holder<ClassType,ValueType>::equals(placeholder const& other) const
{
    // Deemed unequal if types differ or either object_ is null.
    if(other.type() != type())
        {
        return false;
        }
    typedef holder<ClassType,ValueType> holder_type;
    holder_type const& z = static_cast<holder_type const&>(other);
    return object_ && z.object_ && (z.object_)->*(z.held_) == object_->*held_;
}

template<typename ClassType, typename ValueType>
std::string holder<ClassType,ValueType>::str() const
{
    LMI_ASSERT(object_);
    return value_cast<std::string>(object_->*held_);
}

template<typename ClassType, typename ValueType>
std::type_info const& holder<ClassType,ValueType>::type() const
{
    return typeid(ValueType);
}

#if defined LMI_MSC
template<typename ClassType, typename ValueType>
void* holder<ClassType,ValueType>::defraud() const
{
    LMI_ASSERT(object_);
    return &(object_->*held_);
}
#endif // defined LMI_MSC

// Definition of class any_member.

// This class is necessarily Assignable, so that a std::map can hold it.

template<typename ClassType>
class any_member;

template<typename MemberType, typename ClassType>
MemberType* exact_cast(any_member<ClassType>&);

template<typename MemberType, typename ClassType>
MemberType* member_cast(any_member<ClassType>&);

struct any_member_test;

template<typename ClassType>
class any_member
    :virtual private obstruct_slicing<any_member<ClassType> >
    ,public any_entity
{
    template<typename MemberType, typename CT>
    friend MemberType* exact_cast(any_member<CT>&);

    template<typename MemberType, typename CT>
    friend MemberType* member_cast(any_member<CT>&);

    friend struct any_member_test;

  public:
    any_member();
    any_member(any_member const&);
    virtual ~any_member();

    template<typename ValueType>
    any_member(ClassType*, ValueType const&);

    any_member& swap(any_member&);
    any_member& operator=(any_member const&);
    any_member& operator=(std::string const&);
    bool operator==(any_member const&) const;
    bool operator!=(any_member const&) const;

#if defined __BORLANDC__
    // COMPILER !! The borland compiler, defectively it would seem,
    // fails to recognize the friendship granted to function template
    // member_cast() above.
    template<typename ExactMemberType>
    ExactMemberType* exact_cast();
#endif // defined __BORLANDC__

    // any_entity required implementation.
    virtual std::string str() const;
    virtual std::type_info const& type() const;

  private:
#if !defined __BORLANDC__
    template<typename ExactMemberType>
    ExactMemberType* exact_cast();
#endif // !defined __BORLANDC__

    // any_entity required implementation.
    virtual any_member& assign(std::string const&);

    ClassType* object_;
    placeholder* content_;
};

// Implementation of class any_member.

template<typename ClassType>
any_member<ClassType>::any_member()
    :object_(0)
    ,content_(0)
{}

template<typename ClassType>
any_member<ClassType>::any_member(any_member const& other)
    :obstruct_slicing<any_member<ClassType> >()
    ,any_entity (other)
    ,object_    (other.object_)
    ,content_   (other.content_ ? other.content_->clone() : 0)
{}

template<typename ClassType>
any_member<ClassType>::~any_member()
{
    delete content_;
}

template<typename ClassType>
template<typename ValueType>
any_member<ClassType>::any_member(ClassType* object, ValueType const& value)
    :object_(object)
    ,content_(new holder<ClassType,ValueType>(object, value))
{}

template<typename ClassType>
any_member<ClassType>& any_member<ClassType>::swap(any_member& rhs)
{
    std::swap(content_, rhs.content_);
    return *this;
}

template<typename ClassType>
any_member<ClassType>& any_member<ClassType>::operator=
    (any_member<ClassType> const& other
    )
{
    // This would be wrong:
//    any_member<ClassType>(other).swap(*this);
    // because it would swap the ClassType* object, bizarrely placing
    // a pointer to a member of one object into another object's
    // symbol table.
    LMI_ASSERT(other.content_);
    LMI_ASSERT(content_);
    content_->assign(*other.content_);
    return *this;
}

template<typename ClassType>
any_member<ClassType>& any_member<ClassType>::operator=(std::string const& s)
{
    return assign(s);
}

template<typename ClassType>
bool any_member<ClassType>::operator==
    (any_member<ClassType> const& other
    ) const
{
    return content_ && other.content_ && content_->equals(*other.content_);
}

template<typename ClassType>
bool any_member<ClassType>::operator!=
    (any_member<ClassType> const& other
    ) const
{
    return !operator==(other);
}

template<typename ClassType>
std::string any_member<ClassType>::str() const
{
    LMI_ASSERT(content_);
    return content_->str();
}

template<typename ClassType>
std::type_info const& any_member<ClassType>::type() const
{
    return content_ ? content_->type() : typeid(void);
}

template<typename ClassType>
template<typename ExactMemberType>
ExactMemberType* any_member<ClassType>::exact_cast()
{
    typedef ExactMemberType ClassType::* pmd_type;
    if(type() != typeid(pmd_type))
        {
        return 0;
        }
    typedef holder<ClassType,pmd_type> holder_type;
    LMI_ASSERT(content_);
#if !defined LMI_MSC
    pmd_type pmd = static_cast<holder_type*>(content_)->held_;
    LMI_ASSERT(object_);
    return &(object_->*pmd);
#else  // defined LMI_MSC
    return static_cast<ExactMemberType*>(content_->defraud());
#endif // defined LMI_MSC
}

template<typename ClassType>
any_member<ClassType>& any_member<ClassType>::assign(std::string const& s)
{
    LMI_ASSERT(content_);
    content_->assign(s);
    return *this;
}

/// Definition of class template reconstitutor.
///
/// Class template reconstitutor matches pointer-to-member types.
/// It is intended as an auxiliary to class template any_member, to
/// facilitate obtaining a pointer to a base class B from a pointer
/// to member of a class D that is derived from B, particularly in
/// order to call a virtual function declared in B.
///
/// If the MemberType argument is not an exact match, then it tries
/// types derived from MemberType, using knowledge embedded in its
/// reconstitute() member function. A specialization for a base class
/// B can search derived types for an exact D ClassType::* match.
///
/// A pointer is returned rather than a reference for two reasons: the
/// base class might be abstract; and a suitable conversion path might
/// fail to exist, as in the default unspecialized case.
///
/// Motivation.
///
/// Suppose class D is derived from class B, and class T contains a D.
/// There is no well-defined way to convert a D T::* to a B T::*: see
///   http://groups.google.com/group/comp.std.c++/msg/9b9e23f038f63d63
///
/// Suppose it is desired to call virtual B::foo() on all members of T
/// that are derived from B. With an object of class T, it is easy to
/// ascertain that a member of type D is suitable, and to obtain a B*
/// from it. However, when any_member<T> holds a D T::*, its typeid is
/// available but its type is not, and obtaining a B* requires the
/// series of conversions
///   any_member<T> --> D T::* --> D --> B*
/// which, notably, requires knowledge of the original type D. When
/// many classes D0, D1,...Dn are derived from the same base B, it is
/// necessary to test each Dk.

template<typename MemberType, typename ClassType>
struct reconstitutor
{
    static MemberType* reconstitute(any_member<ClassType> const&)
        {
        return 0;
        }
};

/// Implementation of free function template exact_cast().
///
/// Generally prefer free function template member_cast().

template<typename MemberType, typename ClassType>
MemberType* exact_cast(any_member<ClassType>& member)
{
    return member.template exact_cast<MemberType>();
}

template<typename MemberType, typename ClassType>
MemberType const* exact_cast(any_member<ClassType> const& member)
{
    return exact_cast<MemberType>(const_cast<any_member<ClassType>&>(member));
}

/// Implementation of free function template member_cast().
///
/// Hesitate to specialize this function. Instead, specialize class
/// template reconstitutor. See:
///   "Why Not Specialize Function Templates?"
///   http://www.gotw.ca/publications/mill17.htm
///
/// Returns a pointer, of the specified type, to the held object.
///
/// Precondition: MemberType must be either the exact type of the held
/// object, or a base class of that exact type.
///
/// Postcondition: The return value is not zero.
///
/// Throws if the return value would be zero.
///
/// This function template is not intended for testing convertibility,
/// which can easily be done with member function type(). Instead, it
/// is intended to perform a conversion that's known to be valid, and
/// it validates that precondition--so obtaining a null pointer is
/// treated as failure, and throws an exception.

template<typename MemberType, typename ClassType>
MemberType* member_cast(any_member<ClassType>& member)
{
    MemberType* z = (member.type() == typeid(MemberType ClassType::*))
        ? member.template exact_cast<MemberType>()
        : reconstitutor<MemberType,ClassType>::reconstitute(member)
        ;
    if(!z)
        {
        std::ostringstream oss;
        oss
            << "Cannot cast from '"
            << lmi::TypeInfo(member.type())
            << "' to '"
            << lmi::TypeInfo(typeid(MemberType))
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    return z;
}

template<typename MemberType, typename ClassType>
MemberType const* member_cast(any_member<ClassType> const& member)
{
    return member_cast<MemberType>(const_cast<any_member<ClassType>&>(member));
}

// Definition of class MemberSymbolTable.

// By its nature, this class is Noncopyable: it holds a map of
// pointers to member, which need to be initialized instead of copied
// when a derived class is copied. Its Noncopyability is implemented
// natively: deriving from boost::noncopyable would prevent class C
// from deriving from MemberSymbolTable<C> and boost::noncopyable.
//
// A do-nothing constructor is specified in order to prevent compilers
// from warning of its absence. It's protected because this class
// should not be instantiated as a most-derived object.

template<typename ClassType>
class MemberSymbolTable
{
    typedef std::map<std::string, any_member<ClassType> > member_map_type;
    typedef typename member_map_type::value_type member_pair_type;

  public:
    virtual ~MemberSymbolTable();

    any_member<ClassType>      & operator[](std::string const&)      ;
    any_member<ClassType> const& operator[](std::string const&) const;

    MemberSymbolTable<ClassType>& assign(MemberSymbolTable<ClassType> const&);
    bool equals(MemberSymbolTable<ClassType> const&) const;
    std::vector<std::string> const& member_names() const;

  protected:
    MemberSymbolTable();

#if !defined __BORLANDC__
    template<typename ValueType, typename SameOrBaseClassType>
    void ascribe(std::string const&, ValueType SameOrBaseClassType::*);
#else  // defined __BORLANDC__
    // The borland compiler defectively can't handle non-inline member
    // function templates.
    template<typename ValueType, typename SameOrBaseClassType>
    void ascribe(std::string const& s, ValueType SameOrBaseClassType::* p2m)
        {
        ClassType* class_object = static_cast<ClassType*>(this);
        map_.insert
            (member_pair_type(s, any_member<ClassType>(class_object, p2m))
            );
        typedef std::vector<std::string>::iterator svi;
        svi i = std::lower_bound(member_names_.begin(), member_names_.end(), s);
        member_names_.insert(i, s);
        }
#endif // defined __BORLANDC__

  private:
    MemberSymbolTable(MemberSymbolTable const&);
    MemberSymbolTable& operator=(MemberSymbolTable const&);

    void complain_that_no_such_member_is_ascribed(std::string const&) const;

    member_map_type map_;
    std::vector<std::string> member_names_;
};

// Implementation of class MemberSymbolTable.

template<typename ClassType>
MemberSymbolTable<ClassType>::MemberSymbolTable()
{}

template<typename ClassType>
MemberSymbolTable<ClassType>::~MemberSymbolTable()
{}

// operator[]() returns a known member; unlike std::map::operator[](),
// it never adds a new pair to the map, and it complains if such an
// addition is attempted.

template<typename ClassType>
void MemberSymbolTable<ClassType>::complain_that_no_such_member_is_ascribed
    (std::string const& name
    ) const
{
    std::ostringstream oss;
    oss
        << "Symbol table for class "
        << lmi::TypeInfo(typeid(ClassType))
        << " ascribes no member named '"
        << name
        << "'."
        ;
    throw std::runtime_error(oss.str());
}

template<typename ClassType>
any_member<ClassType>& MemberSymbolTable<ClassType>::operator[]
    (std::string const& s
    )
{
    typename member_map_type::iterator i = map_.find(s);
    if(map_.end() == i)
        {
        complain_that_no_such_member_is_ascribed(s);
        }
    return i->second;
}

template<typename ClassType>
any_member<ClassType> const& MemberSymbolTable<ClassType>::operator[]
    (std::string const& s
    ) const
{
    typename member_map_type::const_iterator i = map_.find(s);
    if(map_.end() == i)
        {
        complain_that_no_such_member_is_ascribed(s);
        }
    return i->second;
}

#if !defined __BORLANDC__
template<typename ClassType>
template<typename ValueType, typename SameOrBaseClassType>
void MemberSymbolTable<ClassType>::ascribe
    (std::string const& s
    ,ValueType SameOrBaseClassType::* p2m
    )
{
    // Assert that the static_cast doesn't engender undefined behavior.
    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT
        ((
        boost::is_base_and_derived
            <MemberSymbolTable<ClassType>
            ,ClassType
            >::value
        ));
    BOOST_STATIC_ASSERT
        ((
            boost::is_same
                <SameOrBaseClassType
                ,ClassType
                >::value
        ||  boost::is_base_and_derived
                <SameOrBaseClassType
                ,ClassType
                >::value
        ));

    ClassType* class_object = static_cast<ClassType*>(this);
    map_.insert(member_pair_type(s, any_member<ClassType>(class_object, p2m)));
    typedef std::vector<std::string>::iterator svi;
    // TODO ?? This would appear to be O(N^2).
    svi i = std::lower_bound(member_names_.begin(), member_names_.end(), s);
    member_names_.insert(i, s);
}
#endif // !defined __BORLANDC__

template<typename ClassType>
MemberSymbolTable<ClassType>& MemberSymbolTable<ClassType>::assign
    (MemberSymbolTable<ClassType> const& z
    )
{
    typedef std::vector<std::string>::const_iterator mnci;
    for(mnci i = member_names().begin(); i != member_names().end(); ++i)
        {
        operator[](*i) = z[*i];
        }
    return *this;
}

template<typename ClassType>
bool MemberSymbolTable<ClassType>::equals
    (MemberSymbolTable<ClassType> const& z
    ) const
{
    typedef std::vector<std::string>::const_iterator mnci;
    for(mnci i = member_names().begin(); i != member_names().end(); ++i)
        {
        if(z[*i] != operator[](*i))
            {
            return false;
            }
        }
    return true;
}

template<typename ClassType>
std::vector<std::string> const& MemberSymbolTable<ClassType>::member_names
    (
    ) const
{
    return member_names_;
}

/// Implementation of free function template member_state(), which
/// maps each element of member_names() to a string representation of
/// its current value: the derived-class object's current state.

template<typename ClassType>
std::map<std::string,std::string> member_state
    (MemberSymbolTable<ClassType> const& object
    )
{
    std::map<std::string,std::string> z;
    std::vector<std::string> const& names = object.member_names();
    typedef std::vector<std::string>::const_iterator mnci;
    for(mnci i = names.begin(); i != names.end(); ++i)
        {
        z[*i] = object[*i].str();
        }
    return z;
}

#endif // any_member_hpp

