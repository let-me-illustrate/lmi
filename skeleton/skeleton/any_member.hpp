// Symbolic member names.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: any_member.hpp,v 1.1.1.1 2004-05-15 19:58:00 chicares Exp $

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

// Modified 2001, by Gregory W. Chicares as discussed here:
//   http://groups.yahoo.com/group/boost/message/16626
// to support a symbolic member idiom. GWC further modified this code
// in 2004 and in any later year shown above; any defect in it should
// not reflect on Kevlin Henney's reputation.

#ifndef any_member_hpp
#define any_member_hpp

#include "config.hpp"

#include "value_cast.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/arithmetic_traits.hpp>

#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

// A virtual member template here would permit
//    template<typename X>
//    void placeholder::call(void (X::*)());
// but the language doesn't allow virtual member templates.
//
// A virtual function like
//    void holder<ClassType,ValueType>::call(void (ValueType::*)()){}
// isn't allowed, by gcc at least, if ValueType is a builtin type.

// TODO ?? Rename--perhaps static_holder and dynamic_holder?

// TODO ?? Document that the implicitly-declared special member functions
// are appropriate for class placeholder--and any other class where true.

// TODO ?? As to call(), libstdc++ documentation says:
// There are a total of 16 = 2^4 function objects in this family.
//   (1) Member functions taking no arguments vs member functions taking
//       one argument.
// We may need more than one argument.
//   (2) Call through pointer vs call through reference.
// Not a big problem: looks like this just saves typing '&'.
//   (3) Member function with void return type vs member function with
//       non-void return type.
// Not a problem for compilers (g++, como) that can return void.
//   (4) Const vs non-const member function.
// Apparently not a problem here.

class placeholder
{
  public:
// TODO ?? expunge    virtual ~placeholder() {}
    virtual placeholder& operator=(std::string const&) = 0;
    virtual placeholder* clone() const = 0;
    virtual std::type_info const& type() const = 0;
};

// Declaration of class holder.

template<typename ClassType, typename ValueType>
class holder
    :public placeholder
{
    template<typename T> friend class any_member;

  public:
    holder(ClassType*, ValueType const&);
    holder& operator=(std::string const&);
    
    std::type_info const& type() const;
    placeholder* clone() const;

  private:
    holder(holder const&);
    holder& operator=(holder const&);

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
    // Object and pointer to member reunified here.
    // Assume every ValueType has operator=(std::string).
    object_->*held_ = value_cast(s, object_->*held_);
    return *this;
}

template<typename ClassType, typename ValueType>
std::type_info const& holder<ClassType,ValueType>::type() const
{
    return typeid(ValueType);
}

template<typename ClassType, typename ValueType>
placeholder* holder<ClassType,ValueType>::clone() const
{
    return new holder(object_, held_);
}

// Declaration of class any_member.

template<typename ClassType>
class any_member
{
  public:
    any_member();
    any_member(any_member const&);
    ~any_member();

    template<typename ValueType>
    any_member(ClassType*, ValueType const&);

    any_member& swap(any_member&);
    any_member& operator=(any_member<ClassType> const&);
    any_member& operator=(std::string const&);

    // TODO ?? Worry about const later.
    // TODO ?? Would it be better to return a reference?
    template<typename ValueType>
    ValueType cast() const;

    std::type_info const& type() const;

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
    :object_(other.object_)
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
template<typename ValueType>
ValueType any_member<ClassType>::cast() const
{
    typedef ValueType ClassType::* pmd_type;
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

template<typename ClassType>
std::type_info const& any_member<ClassType>::type() const
{
    return content_ ? content_->type() : typeid(void);
}

// Declaration of class MemberSymbolTable.

template<typename ClassType>
class MemberSymbolTable
{
    typedef std::map<std::string, any_member<ClassType> > member_map;
    typedef typename member_map::value_type member_pair;

  public:
    any_member<ClassType>& operator[](std::string const&);

    template<typename ValueType>
    void ascribe(std::string const&, ValueType ClassType::*);

  private:
    member_map m_;
};

// Implementation of class MemberSymbolTable.

template<typename ClassType>
any_member<ClassType>& MemberSymbolTable<ClassType>::operator[]
    (std::string const& s
    )
{
    typename member_map::iterator i = m_.find(s);
    if(m_.end() != i)
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
template<typename ValueType>
void MemberSymbolTable<ClassType>::ascribe
    (std::string const& s
    ,ValueType ClassType::*p2m
    )
{
    // Here, the behavior of dynamic_cast is well defined, whereas
    // static_cast might display undefined behavior if ClassType
    // does not have MemberSymbolTable<ClassType> as a base class.
    // But static_cast actually gives a diagnostic at compile time
    // with gcc and comeau, so it seems safe as long as at least one
    // of those compilers is used.
#if 0
    ClassType* class_object = dynamic_cast<ClassType*>(this);
    if(!class_object)
        {
        std::ostringstream oss;
        oss
            << "Class '"
            << typeid(ClassType).name()
            << "' must have base class MemberSymbolTable<itself>."
            ;
        throw std::runtime_error(oss.str());
        }
#endif // 0
    ClassType* class_object = static_cast<ClassType*>(this);
    m_.insert(member_pair(s, any_member<ClassType>(class_object, p2m)));
}

#endif // any_member_hpp

