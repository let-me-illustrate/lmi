// Alert messages.
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

// $Id: alert.hpp,v 1.2 2005-01-29 02:47:41 chicares Exp $

#ifndef alert_hpp
#define alert_hpp

#include "config.hpp"

#include "expimp.hpp"

// Instead of <iosfwd>, include <ostream> to make std::flush available
// to modules that use the LMI_ASSERT macro.
#include <ostream>
#include <string>

// Print user messages in a manner appropriate to the interface and
// platform by writing to the std::ostreams these functions return.
// For instance, for a command-line interface, purely informational
// messages might be written to std::cout, and error messages to
// std::cerr, while for a gui, a messagebox might pop up when the
// stream is flushed. This interface is deliberately abstract; the
// concrete behaviors are implementation details.

// Here's how the various streams are intended to be used.
//
// status: Routine notifications arising in normal processing, such as
// "file saved". They might be displayed unobtrusively in a gui, for
// instance on a statusbar; a command-line interface might not display
// them at all.
//
// warning: Significant runtime problems that should be brought to the
// user's attention: the program works, but not in exactly the way the
// user wanted. A gui would probably use a messagebox here.
//
// hobsons_choice: Serious runtime problems that users may be allowed
// to bypass, though at their own peril. Perhaps a gui would offer
// that option where a cgi-bin or command-line interface would not.
//
// fatal_error: Dire runtime problems that prevent the system from
// performing a requested action running in any reasonable manner.
// Generally, an exception would be thrown; a gui might catch it,
// terminate the action in an orderly way, and yet remain active,
// while a command-line program might terminate.

// The motivation is to make code like
//   std::ostringstream oss;
//   oss << "warning: " << some_data << " is invalid";
//   oss << " in context " << some_other_data;
//   if(using_gui) messagebox(oss.str()(;
//   else if(using_cgi) std::cout << html_format(oss.str());
//   else ...
// easier to write, e.g.
//   warning() << "error: " << some_data << " is invalid";
//   warning() << " in context " << some_other_data << std::flush;

// Design decisions, and paths not taken.
//
// The most important principle is to behave as std::cerr behaves:
//   std::cerr << "error" << some_variable << std::endl;
// This suggests a std::stringbuf contained in a std::ostream that is
// some sort of singleton. The archives of comp.lang.c++.moderated
// show that others have used the same design.
//
// The output destination could easily be expressed as a manipulator:
//   single_ostream << "error" << some_variable << messagebox;
// That might be a slightly simpler design. And it is intended that an
// exception be thrown for fatal errors at least, which seems more
// like an independent action than a consequence of flushing a stream.
// But following the std::cerr paradigm is the least surprising
// approach, and it seems natural enough to emit the contents of the
// buffer when std::flush is called.
//
// The same example
//   single_ostream << "error" << some_variable << messagebox;
// illustrates the decision to express the destination conceptually
// rather than physically. A command-line program might very well pop
// up a gui message box, but such a feature seems unnecessary here.
// It seems better to offer a sufficient variety of streams, a bit
// broader than the standard output streams, and implement them for
// each interface or platform separately. The supplied implementation
// factors common data and operations into a base class to reduce the
// effort required for customization.
//
// There must be one stream object for each output type in order to
// preserve state, accumulating successive data in a std::stringbuf.
// This means that these streams must be initialized as some sort of
// singleton, and therefore should not be used in the destructor of a
// dynamically-initialized object of static storage duration.
//
// One could imagine using an optional trace file to log gui alert
// messages. If desired, that could be an implementation detail of the
// gui implementation. It would not be useful in a command-line
// implementation, so it's not considered here. A gui might display
// an alert in a window with a distinct submessage in its caption, but
// there's no natural way to express that on a command-line.
//
// Others have written streams to log trace information for debugging.
// That is outside the scope of this design, so no consideration is
// given to features specific to tracing such as prefixing messages
// with a timestamp or choosing enablement of streams at run time.

std::ostream& LMI_EXPIMP status();
std::ostream& LMI_EXPIMP warning();
std::ostream& LMI_EXPIMP hobsons_choice();
std::ostream& LMI_EXPIMP fatal_error();

// Implement these functions for each platform.
void status_alert         (std::string const&);
void warning_alert        (std::string const&);
void hobsons_choice_alert (std::string const&);
void fatal_error_alert    (std::string const&);
// This must be called exactly once. See platform-specific implementations.
// unused ret val--so we can assign--singleton
bool LMI_EXPIMP set_alert_functions
    (void(*status_alert_function_pointer        )(std::string const&)
    ,void(*warning_alert_function_pointer       )(std::string const&)
    ,void(*hobsons_choice_alert_function_pointer)(std::string const&)
    ,void(*fatal_error_alert_function_pointer   )(std::string const&)
    );

// Ask whether to continue or abort when Hobson's choice is offered.
// Making this a function eliminates duplication and ensures that the
// question is always posed in the same terms.

std::string const& LMI_EXPIMP hobsons_prompt();
std::ostream& LMI_EXPIMP hobsons_prompt(std::ostream&);

#define LMI_ALERT_LOCATION            \
       "\n[file "  << __FILE__        \
    << ", line " << __LINE__ << "]\n"

#define LMI_FLUSH      \
    LMI_ALERT_LOCATION \
    << std::flush

#define LMI_ASSERT(condition)                              \
    if(!(condition))                                       \
        {                                                  \
        hobsons_choice()                                   \
            << "Assertion '" << (#condition) << "' failed" \
            << LMI_ALERT_LOCATION                          \
            << LMI_FLUSH                                   \
            ;                                              \
        }                                                  \
    do {} while(0) // Eat semicolon written after macro invocation.

#endif // alert_hpp

