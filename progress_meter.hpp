// Show progress on lengthy operations, optionally letting users cancel them.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

/// Design notes for class progress_meter.
///
/// This abstract base class displays progress on operations expected
/// to take a long time, so that users know the application is doing
/// something; a particular implementation might provide other useful
/// information such as an estimate of how much longer it will take.
///
/// A concrete implementation is required for each user interface.
///
/// Intended use.
///
/// Create an instance of a concrete derived class before the first
/// iteration, specifying the total number of iterations, which is
/// presumed to be ascertainable in advance.
///
/// Call reflect_progress() at the end of each iteration. If it
/// returns false, then stop iterating.
///
/// After the last iteration, call culminate().
///
/// Public interface.
///
/// enum enum_display_mode: Specifies display mode. Any concrete
/// implementation is free to disregard it and behave as though
/// 'e_normal_display' had been specified.
///   e_normal_display: This default is suitable for most purposes.
///   e_quiet_display: This is merely a hint that the progress meter
///     should display less information than normal. Of the concrete
///     implementations provided, this affects only the command-line
///     variant, for which it completely suppresses all progress-meter
///     output.
///   e_unit_test_mode: Shunts all progress-meter output to a stream
///     specified by progress_meter_unit_test_stream(), to facilitate
///     unit testing. Used only with the command-line interface.
///
/// reflect_progress(): Perform periodic processing: throw an
/// exception if the iteration counter equals or exceeds its maximum;
/// then increment the counter; then call show_progress_message() and
/// return its return value, which is false if the operation is to be
/// cancelled and true otherwise.
///
/// culminate(): Perform postprocessing: call culminate_ui(); then
/// display a warning if the iteration counter doesn't equal its
/// maximum, unless the operation was cancelled.
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
/// progress_message(): Return a string to be displayed when progress
/// is reported.
///
/// show_progress_message(): Display a message indicating progress as
/// appropriate; optionally return false to cancel the operation.
///
/// culminate_ui(): Apply finishing touches to the user interface. For
/// example, the command-line implementation writes a newline and
/// flushes its stream.
///
/// Data members.
///
/// count_: Number of iterations completed so far.
///
/// max_count_: Total number of iterations anticipated.
///
/// title_: A string suitable (e.g.) as a message-box title.
///
/// display_mode_: enum_display_mode value.
///
/// was_cancelled_: True iff the operation was cancelled.
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
/// reflect_progress() throws an exception if the iteration counter
/// equals or exceeds its maximum. This condition is tested before
/// incrementing the counter. Thus, it enforces the invariant expected
/// in the canonical 'for' statement
///   for(int i = 0; i < maximum; ++i) {assert(i < maximum);}
/// Arguably this is useless, but it's quite inexpensive, and it's
/// worth the cost if it ever finds a problem.
///
/// culminate() warns if the iteration counter hasn't been incremented
/// exactly to its maximum, unless the operation was cancelled. This
/// is taken as a postcondition of code that uses the progress meter,
/// so it might seem natural to test it in this class's dtor; however,
/// the postcondition won't be established if the metered loop is
/// exited by throwing an exception. Perhaps std::uncaught_exception
/// could distinguish that special case, but as this is written in
/// 2007-06 some compilers don't implement that standard facility
/// reliably. Postcondition failure engenders only a warning because
/// of other possibilities--e.g., a loop might throw an exception
/// inside a try-block whose catch-clause doesn't rethrow; perhaps it
/// would be better to throw an exception instead, depending on the
/// value of a boolean argument.
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
/// might link 'progress_meter_cli.o', while GUI programs would link an
/// object compiled from some GUI implementation. The artifice that
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
///   http://article.gmane.org/gmane.comp.gnu.mingw.user/15847
///     [2005-04-20T01:20:14Z from Greg Chicares]
///
/// Not all data members are actually accessed in any concrete derived
/// class: max_count_ and title_ are not, but are provided anyway in
/// case they someday become useful. It might seem desirable to omit
/// the corresponding create_progress_meter() arguments and set these
/// members through mutators in this base class after construction
/// instead of in a derived class's ctor; however, that would not work
/// in the wx case, because class wxProgressDialog, reasonably enough,
/// makes no provision for changing them after construction.

#ifndef progress_meter_hpp
#define progress_meter_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#include <boost/shared_ptr.hpp>

#include <iosfwd>
#include <string>

std::ostringstream& LMI_SO progress_meter_unit_test_stream();

class LMI_SO progress_meter
    :private lmi::uncopyable<progress_meter>
{
  public:
    enum enum_display_mode
        {e_normal_display
        ,e_quiet_display
        ,e_unit_test_mode
        };

    bool reflect_progress();
    void culminate();

  protected:
    progress_meter
        (int                max_count
        ,std::string const& title
        ,enum_display_mode
        );

    virtual ~progress_meter();

    int count() const;
    int max_count() const;

    virtual std::string progress_message() const = 0;
    virtual bool show_progress_message() = 0;
    virtual void culminate_ui() = 0;

  private:
    int               count_;
    int               max_count_;
    std::string       title_;
    enum_display_mode display_mode_;
    bool              was_cancelled_;
};

boost::shared_ptr<progress_meter> LMI_SO create_progress_meter
    (int                               max_count
    ,std::string const&                title = ""
    ,progress_meter::enum_display_mode       = progress_meter::e_normal_display
    );

typedef boost::shared_ptr<progress_meter> (*progress_meter_creator_type)
    (int                               max_count
    ,std::string const&                title
    ,progress_meter::enum_display_mode
    );

bool LMI_SO set_progress_meter_creator(progress_meter_creator_type);

#endif // progress_meter_hpp

