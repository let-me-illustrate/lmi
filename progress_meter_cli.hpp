// Show progress on lengthy operations--command-line interface.
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

// $Id: progress_meter_cli.hpp,v 1.1 2005-04-19 14:02:45 chicares Exp $

#ifndef progress_meter_cli_hpp
#define progress_meter_cli_hpp

#include "config.hpp"

#include "progress_meter.hpp"

// Implicitly-declared special member functions do the right thing.

class progress_meter_cli
    :public progress_meter
{
  public:
    progress_meter_cli(int max_count, std::string const& title);
    virtual ~progress_meter_cli();
    virtual std::string progress_message() const;
    virtual bool show_progress_message() const;
};

#endif // progress_meter_cli_hpp

