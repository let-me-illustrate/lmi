// Symbolic member names (obsolete version).
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: any_member_ihs.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

// Refer to data members by symbolic names.

#ifndef any_member_ihs_hpp
#define any_member_ihs_hpp

#include "config.hpp"

#ifndef BC_BEFORE_5_5

#include "value_cast_ihs.hpp"

#include <algorithm>
#include <ios>
#include <iosfwd>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

#include "max_stream_precision.hpp" // TODO ?? experimental

// Class 'any_member_ihs' is a derived work adapted from boost::any,
// which is described as follows:
// [boost::any notices begin]
// what:  variant type boost::any
// who:   contributed by Kevlin Henney,
//        with features contributed and bugs found by
//        Ed Brey, Mark Rodgers, Peter Dimov, and James Curran
// when:  July 2001
// where: tested with BCC 5.5, MSVC 6.0, and g++ 2.95
// Copyright Kevlin Henney, 2000, 2001. All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives, and that no
// charge may be made for the software and its documentation except to cover
// cost of distribution.
//
// This software is provided "as is" without express or implied warranty.
// [boost::any notices end]

// Modified 2001-08-23 by Gregory W. Chicares and in any later years
// indicated above. Any defects should not reflect on Kevlin Henney's
// reputation.

namespace boost
{
    template<typename ClassType>
    class any_member_ihs
    {
    public: // structors

        any_member_ihs()
          : content(0)
        {
        }

        template<typename ValueType>
        any_member_ihs(ClassType * a_object, ValueType const& value)
          : content(new holder<ValueType>(a_object, value))
        {
        }

        any_member_ihs(any_member_ihs const& other)
          : content(other.content ? other.content->clone() : 0)
        {
        }

        ~any_member_ihs()
        {
            delete content;
        }

    public: // modifiers

        any_member_ihs & swap(any_member_ihs & rhs)
        {
            std::swap(content, rhs.content);
            return *this;
        }

        any_member_ihs & operator=(any_member_ihs<ClassType> const& rhs)
        {
            any_member_ihs<ClassType>(rhs).swap(*this);
            return *this;
        }

        // We might have called this assign_through_pointer(),
        // which is more descriptive, but we want to write the
        // operation as an assignment in user code.
        any_member_ihs & operator=(std::string const& s)
        {
            content->assign_through_pointer(s);
            return *this;
        }

        any_member_ihs & operator=(char const* s)
        {
            return operator=(std::string(s));
        }

    public: // queries

    // TODO ?? It would be nicer not to rely on conversion through string....
        bool operator==(any_member_ihs<ClassType> const& rhs) const
        {
// TODO ?? Track down the problem here: error on conversion from empty string.
//            std::string s = value_cast_ihs<std::string>(rhs);
//
//            std::string s = rhs.str();
//            return *content == s;
//
        return rhs.str() == str();
        }

        // TODO ?? Try boost::operators instead.
        bool operator!=(any_member_ihs<ClassType> const& rhs) const
        {
            return !operator==(rhs);
        }

        void print(std::ostream& os) const
        {
            content->print(os);
        }

        std::string str() const
        {
            std::ostringstream oss;
            content->print(oss);
            return oss.str();
        }

    // Comeau complains that clone() is inaccessible if this is private.
    public: // types

        class placeholder
        {
        public: // structors

            virtual ~placeholder()
            {
            }

        public: // modifiers

            // We might have called this operator=(),
            // but then it ought to return something,
            // yet we need nothing this function could return.
            virtual void assign_through_pointer(std::string const& s) = 0;

        public: // queries

            virtual placeholder * clone() const = 0;

            virtual bool operator==(std::string const&) const = 0;

            virtual void print(std::ostream&) const = 0;
        };

        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

            holder(ClassType * a_object, ValueType const& value)
              : object(a_object)
              , held(value)
            {
            }

        public: // modifiers

            void assign_through_pointer(std::string const& s)
            {
            // This is the place where we must reunify the object
            // and the pointer to member.
                object->*held = value_cast_ihs(s, object->*held);
            }

        public: // queries

            virtual bool operator==(std::string const& s) const
            {
//            return object->*held == s;
                return s == value_cast_ihs(object->*held, s + " ");
//return s == object->*held.str();
            }

            virtual placeholder * clone() const
            {
                return new holder(object, held);
            }

            virtual void print(std::ostream& os) const
            {
                os.setf(std::ios_base::scientific, std::ios_base::floatfield);
                os.precision(max_stream_precision());
                os << object->*held;
            }

        public: // representation

            ClassType * object;
            ValueType held;

        };

    private: // representation

        placeholder * content;

    };

    template<typename ClassType>
    std::ostream& operator<<(std::ostream& os, any_member_ihs<ClassType> const& z)
    {
        z.print(os);
        return os;
    }

} // namespace boost

template<typename D>
class MemberSymbolTable_ihs
{
public:
    typedef std::map<std::string, boost::any_member_ihs<D> > member_map_t       ;
    typedef typename member_map_t::value_type            member_map_value_t ;
    typedef typename member_map_t::key_type              member_map_key_t   ;
// TODO ?? bc++5.5.1 doesn't like this; is it correct?
//    typedef typename member_map_t::data_type             member_map_data_t  ;
    typedef boost::any_member_ihs<D>                         member_map_data_t  ;

    std::vector<std::string> map_keys() const
        {
        std::vector<std::string> keys;
        typename member_map_t::const_iterator m_i;
        for
            (m_i  = m.begin()
            ;m_i != m.end  ()
            ;++m_i
            )
            {
            keys.push_back((*m_i).first);
            }
        return keys;
        }

// TODO ?? Can member_map() and map_keys() possibly be static?
// Can the keys at least be generated staticly?

/* TODO ?? This experimental stuff is an attempt to call virtual function
ascribe_members() when this class is constructed; looks like it's doomed,
but let's write up why....
    MemberSymbolTable_ihs()
        {
std::cout << "MemberSymbolTable_ihs default ctor called" << std::endl;
        ascribe_members();
        }

    MemberSymbolTable_ihs(MemberSymbolTable_ihs const&)
        {
std::cout << "MemberSymbolTable_ihs copy ctor called" << std::endl;
        ascribe_members();
        }

    virtual ~MemberSymbolTable_ihs()
        {
        }

    MemberSymbolTable_ihs& operator=(MemberSymbolTable_ihs const&)
        {
std::cout << "MemberSymbolTable_ihs operator=() called" << std::endl;
        ascribe_members();
        return *this;
        }
*/

    boost::any_member_ihs<D>& operator[](std::string const& s)
        {
        typename member_map_t::iterator i = m.find(s);
        if(m.end() != i)
            {
            return i->second;
            }
        else
            {
            throw no_such_member(s);
            }
        }

    boost::any_member_ihs<D> const& operator[](std::string const& s) const
        {
        typename member_map_t::const_iterator i = m.find(s);
        if(m.end() != i)
            {
            return i->second;
            }
        else
            {
            throw no_such_member(s);
            }
        }

    member_map_t const& member_map()
        {
        return m;
        }

protected:

    template<typename T, typename Z>
    void ascribe(char const* s, T Z::*p2m)
        {
        m.insert(member_map_value_t(s, boost::any_member_ihs<D>(static_cast<D*>(this), p2m)));
        }

private:

/* TODO ?? This experimental stuff doesn't work...see above.
    virtual void ascribe_members()
        {
std::cout << "MemberSymbolTable_ihs::ascribe_members() called" << std::endl;
//dynamic_cast<D&>(*this).ascribe_members(); // NOT
        std::map<std::string, boost::any_member_ihs<D> >::iterator m_i;
        static_cast<D*>(this), (*m_i).second;
//std::cout << "Wow, did that work?" << std::endl;
//        std::map<std::string, boost::any_member_ihs<D> >::iterator m_i;
//        for(m_i = m.begin(); m_i != m.end(); ++m_i)
//            {
//            boost::any_member_ihs<D>(static_cast<D*>(this), (*m_i).second);
//            m[(*m_i).first] = boost::any_member_ihs<D>(static_cast<D*>(this), (*m_i).second);
//            }
        }
*/

    member_map_t m;

    class no_such_member : public std::runtime_error
    {
      public:
        explicit no_such_member(std::string const& what_arg)
        :std::runtime_error
            ( "MemberSymbolTable_ihs<" + std::string(typeid(D).name()) + ">:"
            " no ascribed member named '" + what_arg + "'."
            )
        {}
    };

};

#endif // BC_BEFORE_5_5

// COMPILER!! bc++5.5.1 doesn't like a template at the end of a file, so...
#if defined __BORLANDC__ && __BORLANDC__ >= 0x0550
extern int borland_workaround;
#endif // bc++5.5.1 workaround

#endif // any_member_ihs_hpp

