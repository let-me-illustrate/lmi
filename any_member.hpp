// Symbolic member names.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: any_member.hpp,v 1.6 2005-08-02 21:22:11 chicares Exp $

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
// Member template cast() is safe, but limited: it casts only to the
// object's exact original type. Member template cast_blithely() casts
// to any type, but cannot know whether that's safe.
//
// Motivation. Consider a base class B with member function b(), and a
// class D derived from B. Suppose that an instance d of D is held
// here. If it is known that d is a D, then cast() can be used:
// although the object's type is lost and dynamic_cast therefore can't
// be used, static_cast performs the type conversion [5.2.9/9] and
// RTTI ensures that the conversion is safe. But if it is known only
// that d is a B, but not that it is also a D, then invoking d.b()
// requires casting d blithely to a B, and the language provides no
// builtin facility for validating that cast. The type D is known
// inside class holder, and the type B is known to clients of class
// any_member, but between them lies a placeholder* through which type
// information can pass only through std::type_info, which cannot
// validate that a D is a B. And 14.5.2/3 doesn't allow virtual member
// templates, for reasons explained here:
//   http://groups.google.com/groups?selm=7f6de0%24t1t%241%40nnrp1.dejanews.com
//
// Member template cast_blithely() returns a pointer so that virtual
// functions of an abstract base class may be called through it--an
// instance cannot be returned in that case.
//
// With the same lack of safety, a family of call() member templates
// could be defined here. That seems less desirable because it would
// require a potentially unbounded set of signatures.

// Design notes: numeric stream input and output
//
// Member function holder::write() explicitly invokes function
// template value_cast() on its held object before writing it to
// the std::ostream. This adds some overhead, which is probably
// significant for held objects of type char*, but data members
// generally should be of type std::string instead, so the cost seems
// unimportant. The benefit is that arithmetic types are written with
// all possible decimal precision.

#ifndef any_member_hpp
#define any_member_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "value_cast.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/arithmetic_traits.hpp>
#include <boost/utility.hpp>

#include <algorithm>
#include <iosfwd>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

// Declaration of class placeholder.

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
    virtual placeholder& operator=(std::string const&) = 0;
    virtual placeholder* clone() const = 0;
    virtual std::type_info const& type() const = 0;
    virtual std::ostream& write(std::ostream&) const = 0;
};

// Implementation of class placeholder.

inline placeholder::~placeholder()
{}

// Declaration of class holder.

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

    // placeholder overrides.
    virtual holder& operator=(std::string const&);
    virtual placeholder* clone() const;
    virtual std::type_info const& type() const;
    virtual std::ostream& write(std::ostream&) const;

  private:
    ClassType* object_;
    ValueType held_;
};

// Implementation of class holder.

template<typename ClassType, typename ValueType>
holder<ClassType,ValueType>::holder
    (ClassType* object
    ,ValueType const& value
    )
    :object_(object)
    ,held_(value)
{}

template<typename ClassType, typename ValueType>
holder<ClassType,ValueType>& holder<ClassType,ValueType>::operator=
    (std::string const& s
    )
{
    object_->*held_ = value_cast(s, object_->*held_);
    return *this;
}

template<typename ClassType, typename ValueType>
placeholder* holder<ClassType,ValueType>::clone() const
{
    return new holder(object_, held_);
}

template<typename ClassType, typename ValueType>
std::type_info const& holder<ClassType,ValueType>::type() const
{
    return typeid(ValueType);
}

template<typename ClassType, typename ValueType>
std::ostream& holder<ClassType,ValueType>::write(std::ostream& os) const
{
    os << value_cast<std::string>(object_->*held_);
    return os;
}

// Declaration of class any_member.

// This class is necessarily Assignable, so that a std::map can hold it.

// It holds a pointer to a ClassType object only so for its cast
// operations.

template<typename ClassType>
class any_member
    :virtual private obstruct_slicing<any_member<ClassType> >
{
  public:
    any_member();
    any_member(any_member const&);
    ~any_member();

    template<typename ValueType>
    any_member(ClassType*, ValueType const&);

    any_member& swap(any_member&);
    any_member& operator=(any_member const&);
    any_member& operator=(std::string const&);

    template<typename ArbitraryType>
    ArbitraryType cast() const;

    template<typename ArbitraryType>
    ArbitraryType* cast_blithely() const;

    std::string str() const;
    std::type_info const& type() const;
    std::ostream& write(std::ostream&) const;

  private:
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
    ,object_(other.object_)
    ,content_(other.content_ ? other.content_->clone() : 0)
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
    (any_member<ClassType> const& rhs
    )
{
    any_member<ClassType>(rhs).swap(*this);
    return *this;
}

template<typename ClassType>
any_member<ClassType>& any_member<ClassType>::operator=(std::string const& s)
{
    content_->operator=(s);
    return *this;
}

template<typename ClassType>
template<typename ArbitraryType>
ArbitraryType any_member<ClassType>::cast() const
{
    typedef ArbitraryType ClassType::* pmd_type;
    if(type() != typeid(pmd_type))
        {
        std::ostringstream oss;
        oss
            << "Cannot cast from '"
            << type().name()
            << "' to '"
            << typeid(pmd_type).name()
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    typedef holder<ClassType,pmd_type> holder_type;
    pmd_type pmd = static_cast<holder_type*>(content_)->held_;
    return object_->*pmd;
}

// TODO ?? What if we pass a typed null pointer up to placeholder,
// and try to convert it with dynamic_cast in holder?

template<typename ClassType>
template<typename ArbitraryType>
ArbitraryType* any_member<ClassType>::cast_blithely() const
{
    typedef ArbitraryType ClassType::* pmd_type;
    typedef holder<ClassType,pmd_type> holder_type;
    pmd_type pmd = static_cast<holder_type*>(content_)->held_;
    return &(object_->*pmd);
}

template<typename ClassType>
std::string any_member<ClassType>::str() const
{
    std::ostringstream oss;
    content_->write(oss);
    return oss.str();
}

template<typename ClassType>
std::type_info const& any_member<ClassType>::type() const
{
    return content_ ? content_->type() : typeid(void);
}

template<typename ClassType>
std::ostream& any_member<ClassType>::write(std::ostream& os) const
{
    return content_->write(os);
}

template<typename ClassType>
std::ostream& operator<<(std::ostream& os, any_member<ClassType> const& z)
{
    return z.write(os);
}

// Free functions that complement class any_member.

// TODO ?? Also consider operator==(std::string), and boost::operators.

template<typename ClassType>
bool operator==(any_member<ClassType> const& lhs, any_member<ClassType> const& rhs)
{
// TODO ?? Compare contents directly instead.
    return lhs.str() == rhs.str();
}

template<typename ClassType>
bool operator!=(any_member<ClassType> const& lhs, any_member<ClassType> const& rhs)
{
    return !(lhs == rhs);
}

// Declaration of class MemberSymbolTable.

// By its nature, this class is Noncopyable: it holds a map of
// pointers to member, which need to be initialized instead of copied
// when a derived class is copied. Its Noncopyability is implemented
// natively: deriving from boost::noncopyable would prevent class C
// from deriving from MemberSymbolTable<C> and boost::noncopyable.
//
// TODO ?? Alternatively, one might define a copy ctor that
// (automatically) ascribe()s all members to the appropriate object.

// A do-nothing constructor is specified in order to prevent compilers
// from warning of its absence. It's protected because this class
// should not be instantiated as a most-derived object.

template<typename ClassType>
class MemberSymbolTable
{
    typedef std::map<std::string, any_member<ClassType> > map_type;
    typedef typename map_type::value_type map_value_type;

  public:
    any_member<ClassType>& operator[](std::string const&);
    any_member<ClassType> const& operator[](std::string const&) const;
    std::vector<std::string> const& member_names() const;

  protected:
    MemberSymbolTable();
    template<typename ValueType, typename RelatedClassType>
    void ascribe(std::string const&, ValueType RelatedClassType::*);

  private:
    MemberSymbolTable(MemberSymbolTable const&);
    MemberSymbolTable const& operator=(MemberSymbolTable const&);

    std::vector<std::string> cached_member_names() const;

    map_type map_;
// TODO ?? With como-4.3.3, uncommenting this line moves a failure
// from one unit test to another. It's a poor idea to assume this is
// a compiler defect without investigating it carefully.
//    std::vector<std::string> dummy_member_;
};

// Implementation of class MemberSymbolTable.

template<typename ClassType>
MemberSymbolTable<ClassType>::MemberSymbolTable()
{
}

// operator[]() returns a known member; unlike std::map::operator[](),
// it never adds a new pair to the map.

// INELEGANT !! The const and non-const operator[]() implementations
// are nearly identical; what's commmon should be factored out.

template<typename ClassType>
any_member<ClassType>& MemberSymbolTable<ClassType>::operator[]
    (std::string const& s
    )
{
    typename map_type::iterator i = map_.find(s);
    if(map_.end() != i)
        {
        return i->second;
        }
    else
        {
        std::ostringstream oss;
        oss
            << "Symbol table for class '"
            << typeid(ClassType).name()
            << "' ascribes no member named '"
            << s
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
}

template<typename ClassType>
any_member<ClassType> const& MemberSymbolTable<ClassType>::operator[]
    (std::string const& s
    ) const
{
    typename map_type::const_iterator i = map_.find(s);
    if(map_.end() != i)
        {
        return i->second;
        }
    else
        {
        std::ostringstream oss;
        oss
            << "Symbol table for class '"
            << typeid(ClassType).name()
            << "' ascribes no member named '"
            << s
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
}

template<typename ClassType>
std::vector<std::string> const& MemberSymbolTable<ClassType>::member_names() const
{
    static std::vector<std::string> member_name_vector(cached_member_names());
    return member_name_vector;
}

template<typename ClassType>
template<typename ValueType, typename RelatedClassType>
void MemberSymbolTable<ClassType>::ascribe
    (std::string const& s
    ,ValueType RelatedClassType::*p2m
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
                <RelatedClassType
                ,ClassType
                >::value
        ||  boost::is_base_and_derived
                <RelatedClassType
                ,ClassType
                >::value
        ));
    ClassType* class_object = static_cast<ClassType*>(this);
    map_.insert(map_value_type(s, any_member<ClassType>(class_object, p2m)));
}

// TODO ?? This is invalid if ascribe() is called after the first
// invocation of cached_member_names(). If names are to be cached,
// then they should be cached in a class member that ascribe() can
// check and perhaps update directly.

template<typename ClassType>
std::vector<std::string> MemberSymbolTable<ClassType>::cached_member_names() const
{
    std::vector<std::string> member_name_vector;
    member_name_vector.reserve(map_.size());
    typename map_type::const_iterator i;
    for(i = map_.begin(); i != map_.end(); ++i)
        {
        member_name_vector.push_back(i->first);
        }
    return member_name_vector;
}

#endif // any_member_hpp

