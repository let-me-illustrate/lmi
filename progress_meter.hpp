// Show progress on lengthy operations, optionally letting users cancel them.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: progress_meter.hpp,v 1.5 2005-10-05 13:43:40 chicares Exp $

/// Design notes for class progress_meter.
///
/// This abstract base class displays progress on operations expected
/// to take a long time, so that users know the application is doing
/// something and can see how much longer it will take. Intended use:
/// a concrete derived class is instantiated at the beginning of the
/// operation, and the resulting object's reflect_progress() function
/// is called on each of a number of iterations known in advance; each
/// time it's called, it may provide the user an opportunity to cancel
/// the operation. A concrete implementation should be provided for
/// each user interface; the default implementation does almost nothing
/// and is probably appropriate for cgi-bin.
///
/// Data members.
///
/// count_: Number of iterations completed so far.
///
/// max_count_: Total number of iterations anticipated.
///
/// title_: A string suitable (e.g.) as a message-box title.
///
/// Public interface.
///
/// Implicitly-declared special member functions do the right thing.
///
/// reflect_progress(): Throw an exception if the iteration counter
/// equals or exceeds its maximum; then increment the counter; then
/// call show_progress_message() and return its return value, which is
/// optionally return false to cancel the operation.
///
/// Protected interface--nonvirtual.
///
/// ctor: Initialize data members from ctor arguments.
///
/// count(), max_count(): Return the value of the iteration counter,
/// or its maximum, respectively. Derived classes need these to access
/// the private data.
///
/// Protected interface--virtual.
///
/// show_progress_message(): Display a message indicating progress as
/// appropriate; optionally return false to cancel the operation.
///
/// progress_message(): Return a string to be displayed when progress
/// is reported.
///
/// Nonmember functions.
///
/// create_progress_meter(): Create an instance of a derived class by
/// invoking its ctor.
///
/// set_progress_meter_creator(): Set the function pointer used by
/// create_progress_meter().
///
/// Design alternatives considered; rationale for design choices.
///
/// Function reflect_progress() throws an exception if the iteration
/// counter equals or exceeds its maximum. This condition is tested
/// before incrementing the counter. Thus, it enforces the invariant
/// expected in the canonical 'for' statement
///   for(int i = 0; i < maximum; ++i) {assert(i < maximum);}
/// Arguably this is useless, but it's quite inexpensive, and it's
/// worth the cost if it ever finds a problem. One might also consider
/// trying to ensure that the counter reaches its maximum, but often
/// loops are exited early; and the most direct way to enforce that
/// invariant would be to detect it in the dtor, which shouldn't throw.
///
/// An argument could be made for making count() public. That's easy
/// enough to change if wanted, but would promote a usage for which
/// this class isn't ideally suited.
///
/// Virtual function progress_message() could be seen as needless: its
/// raison d'être is only to be called by show_progress_message(),
/// which is already virtual. The rationale for separating them is that
/// two interfaces might use the same strings but display them by
/// different means. Perhaps this is rococo.
///
/// The nonmember functions support a particular artifice whose
/// intention is to let a shared library use only this base class,
/// while the application provides a derived-class implementation that
/// is switchable at link time. Thus, a command-line-interface program
/// might link 'progress_meter_cli.o', while gui programs would link an
/// object compiled from some gui implementation. The artifice that
/// accomplishes this in the present implementation is a callback
/// function pointer. The support functions are nonmembers because of
/// Meyer's reasoning in his well-known paper
///   "How Non-Member Functions Improve Encapsulation"
/// although they might instead have been static members. This general
/// technique could be abstracted with templates. With the gnu linker,
/// '--export-dynamic' could be used instead of the explicit callback;
/// that is not done in the present implementation because it's not
/// known whether all other toolsets provide an equivalent facility.
/// This might be the best approach in any case: see
///   http://sourceforge.net/mailarchive/message.php?msg_id=11527978

#ifndef progress_meter_hpp
#define progress_meter_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <boost/shared_ptr.hpp>

#include <string>

class LMI_EXPIMP progress_meter
{
  public:
    bool reflect_progress();

  protected:
    progress_meter(int max_count, std::string const& title);
    virtual ~progress_meter();

    int count() const;
    int max_count() const;

    virtual std::string progress_message() const = 0;
    virtual bool show_progress_message() const = 0;

  private:
    int count_;
    int max_count_;
    std::string title_;
};

boost::shared_ptr<progress_meter> LMI_EXPIMP create_progress_meter
    (int max_count
    ,std::string const& title = ""
    );

typedef boost::shared_ptr<progress_meter> (*progress_meter_creator_type)
    (int max_count
    ,std::string const& title
    );

bool LMI_EXPIMP set_progress_meter_creator(progress_meter_creator_type);

#endif // progress_meter_hpp

