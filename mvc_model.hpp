// MVC Model base class.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: mvc_model.hpp,v 1.2 2006-07-09 18:39:07 chicares Exp $

#ifndef mvc_model_hpp
#define mvc_model_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <map>
#include <string>
#include <vector>

class any_entity;
class datum_base;

/// Design notes for class MvcModel.
///
/// Names() gives the names by which Model entities are accessed.
///
/// State() maps each element of Names() to a string representation of
/// its current value; this constitutes the Model's current state.
///
/// AdaptExternalities() updates any objects outside this class to
/// reflect its state.
///
/// CustomizeInitialValues() facilitates dynamic initialization. As a
/// motivating example, consider a derived class with calendar-date
/// members whose values are interdependent. Its ctor can easily
/// hardcode an indubitably-valid initial state, but it may be more
/// natural to use the current date for one particular member; yet
/// duplicating the Harmonize() and Transmogrify() logic in its ctor
/// is a poor way to make the other members consistent. It is better
/// to perform any dynamic customization of initial values in the
/// overridden virtual this function calls, then to make everything
/// consistent in this nonvirtual function if possible, and finally to
/// verify in TestInitialConsistency() that the resulting state is
/// consistent and stable.
///
/// EnforceRangeLimit() changes a range-constrained numeric datum's
/// value, if necessary, to ensure that it lies between its limits.
/// It is intended to be called by Transmogrify(), from which it is
/// separate in order to permit separate overriding of implementation
/// while enforcing invocation order. Generally, limits are changed in
/// Harmonize() only, and enforced before the virtual implementation
/// of Transmogrify() is called.
///
/// Harmonize() and Transmogrify() both enforce various relationships
/// among data and their associated controls. Harmonize() updates
/// range limits and conditional enablement, but does not affect the
/// value of any datum. Transmogrify() changes data values as required
/// to enforce consistency. Neither directly changes any control, of
/// course: that's the Controller's job. Harmonize() is notionally
/// const in that it must not change any datum's value--a condition
/// that is tested carefully, and engenders an exception if violated.
/// It cannot be physically const without making UDT members (other
/// than the UDT's value) mutable, which they must not be because they
/// affect the UDT's state, as observable by equality comparison or,
/// often, by mere inspection of the View.
///
/// Reconcile() calls Harmonize() and Transmogrify() one or more
/// times, until neither changes any data member's value.
///
/// TODO ?? Is that actually sufficient? Shouldn't the stopping
/// criterion be more stringent? Why not require that iteration
/// continue until no data member changes in any way?
///
/// TestInitialConsistency(), if called in the derived class's ctor,
/// ensures that its initial state is valid and stable. Stability
/// means idempotence under the Harmonize() and Transmogrify()
/// operations.
///
/// TODO ?? That should be ensured somehow. It's the postconstructor
/// problem described here:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/80ab79d85b150e17
/// Maybe a bool flag should be set here when TestInitialConsistency()
/// is called, and then checked in some function that's certain to be
/// called, such as the dtor--or perhaps even in the Controller ctor.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO MvcModel
{
    friend class MvcModelTest;

  public:
    typedef std::vector<std::string> NamesType;
    typedef std::map<std::string,std::string> StateType;

    MvcModel();
    virtual ~MvcModel();

    datum_base const* BaseDatumPointer(std::string const&) const;

    any_entity      & Entity(std::string const&)      ;
    any_entity const& Entity(std::string const&) const;

    NamesType const& Names() const;
    StateType        State() const;

    void Reconcile();

  protected:
    void TestInitialConsistency();

  private:
    void AdaptExternalities();
    void CustomizeInitialValues();
    void EnforceRangeLimit(std::string const&);
    void Harmonize();
    void Transmogrify();

    virtual void DoAdaptExternalities() = 0;

    virtual datum_base const* DoBaseDatumPointer(std::string const&) const = 0;

    virtual any_entity      & DoEntity(std::string const&)       = 0;
    virtual any_entity const& DoEntity(std::string const&) const = 0;

    virtual NamesType const& DoNames() const = 0;
    virtual StateType        DoState() const = 0;

    virtual void DoCustomizeInitialValues() = 0;
    virtual void DoEnforceRangeLimit     (std::string const&) = 0;
    virtual void DoHarmonize             () = 0;
    virtual void DoTransmogrify          () = 0;
};

#endif // mvc_model_hpp

