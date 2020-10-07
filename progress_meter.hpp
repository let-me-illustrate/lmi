// Show progress on lengthy operations, optionally letting users cancel them.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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
/// dawdle(): Pause for the number of seconds given in the argument.
///
/// reflect_progress(): Perform periodic processing: throw an
/// exception if the iteration counter equals or exceeds its maximum;
/// then increment the counter; then call show_progress_message() and
/// return its return value, which is false if the operation is to be
/// cancelled and true otherwise.
///
/// culminate(): Perform postprocessing: call culminate_ui(); then
/// throw if are_postconditions_met() returns false.
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
/// dtor: Warn if are_postconditions_met() returns false but the stack
/// is not being unwound. In that case, an exception should have been
/// thrown by culminate(), which therefore must not have been called
/// when it should have been.
///
/// do_dawdle(): Implement dawdle().
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
/// Private interface.
///
/// are_postconditions_met(): Determine whether postconditions have been
/// fulfilled, i.e., either
///  - the iteration counter equals its maximum, or
///  - the operation was cancelled.
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
/// dawdle() is a non-static public member. It cannot be a private
/// member called by reflect_progress(), as in this example:
///   for(...) {
///     if(condition)
///       do_something();
///     reflect_progress(seconds_to_dawdle);
/// because no pause is wanted when the condition is false. It cannot
/// be static, because it must call virtual do_dawdle() to distinguish
/// behavior by user interface. It's a member of this class because
/// the motivating use case involves a progress meter--so, for the wx
/// interface, do_dawdle() should call wxProgressDialog::Update(),
/// for the reasons explained here:
///   https://lists.nongnu.org/archive/html/lmi/2013-11/msg00006.html
/// and also so that pressing Cancel interrupts the delay. Because
/// wxProgressDialog::Update() is not const, do_dawdle() and dawdle()
/// cannot be const.
///
/// reflect_progress() throws an exception if the iteration counter
/// equals or exceeds its maximum. This condition is tested before
/// incrementing the counter. Thus, it enforces the invariant expected
/// in the canonical 'for' statement
///   for(int i = 0; i < maximum; ++i) {assert(i < maximum);}
///
/// It might seem natural to dispense with culminate() and fold its
/// code into the dtor. However, lmi dtors are designed not to throw,
/// so the dtor merely warns if culminate() appears not to have been
/// called when it should have been.
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
/// class: for example, title_ is not, but it is provided anyway in
/// case it someday becomes useful. It might seem desirable to omit
/// the corresponding create_progress_meter() arguments and set these
/// members through mutators in this base class after construction
/// instead of in a derived class's ctor; however, that would not work
/// in the wx case, because class wxProgressDialog, reasonably enough,
/// makes no provision for changing them after construction.

#ifndef progress_meter_hpp
#define progress_meter_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <iosfwd>
#include <memory>                       // unique_ptr
#include <string>

LMI_SO std::ostringstream& progress_meter_unit_test_stream();

class LMI_SO progress_meter
{
  public:
    enum enum_display_mode
        {e_normal_display
        ,e_quiet_display
        ,e_unit_test_mode
        };

    virtual ~progress_meter();

    void dawdle(int seconds);
    bool reflect_progress();
    void culminate();

  protected:
    progress_meter
        (int                max_count
        ,std::string const& title
        ,enum_display_mode
        );

    int count() const;
    int max_count() const;

    // This accessor is not actually used today; it serves only to
    // prevent Clang from complaining that the member it accesses is
    // otherwise unused. See:
    //   https://lists.nongnu.org/archive/html/lmi/2016-03/msg00035.html
    enum_display_mode display_mode() const {return display_mode_;}

    virtual void        do_dawdle            (int seconds);
    virtual std::string progress_message     () const = 0;
    virtual bool        show_progress_message()       = 0;
    virtual void        culminate_ui         ()       = 0;

  private:
    progress_meter(progress_meter const&) = delete;
    progress_meter& operator=(progress_meter const&) = delete;

    bool are_postconditions_met() const;

    int               count_;
    int               max_count_;
    std::string       title_;
    enum_display_mode display_mode_;
    bool              was_cancelled_;
};

LMI_SO std::unique_ptr<progress_meter> create_progress_meter
    (int                               max_count
    ,std::string const&                title = std::string()
    ,progress_meter::enum_display_mode       = progress_meter::e_normal_display
    );

typedef std::unique_ptr<progress_meter> (*progress_meter_creator_type)
    (int                               max_count
    ,std::string const&                title
    ,progress_meter::enum_display_mode
    );

LMI_SO bool set_progress_meter_creator(progress_meter_creator_type);

#endif // progress_meter_hpp
