// Numerical algorithms for stratified rates.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "stratified_algorithms.hpp"

#include "assert_lmi.hpp"

#include <cmath>

//============================================================================
// Determine net amount after subtracting a tiered proportion
// Intended use: convert gross premium to net premium
// Only two tiers are allowed--more would require vector arguments
double TieredGrossToNet
    (double GrossAmount
    ,double Breakpoint
    ,double LoadUpToBreakpoint
    ,double LoadBeyondBreakpoint
    )
{
    double amount_up_to_breakpoint = std::min(GrossAmount, Breakpoint);
    double amount_over_breakpoint = GrossAmount - amount_up_to_breakpoint;
    return
            GrossAmount
        -   amount_up_to_breakpoint * LoadUpToBreakpoint
        -   amount_over_breakpoint * LoadBeyondBreakpoint
        ;
}

//============================================================================
// Inverse of TieredGrossToNet()
double TieredNetToGross
    (double NetAmount
    ,double Breakpoint
    ,double LoadUpToBreakpoint
    ,double LoadBeyondBreakpoint
    )
{
    if(0 == NetAmount)
        {
        return NetAmount;
        }

    LMI_ASSERT(LoadUpToBreakpoint   < 1.0);
    LMI_ASSERT(LoadBeyondBreakpoint < 1.0);

    double z = NetAmount / (1.0 - LoadUpToBreakpoint);
    if(Breakpoint < z && LoadUpToBreakpoint != LoadBeyondBreakpoint)
        {
        z =
            (
                NetAmount
            +   Breakpoint * (LoadUpToBreakpoint - LoadBeyondBreakpoint)
            )
            /   (1.0 - LoadBeyondBreakpoint)
            ;
        }

// Here's the algebra:
//
//  Gross = Net + Tgt (LoadTgt - LoadExc) / (1 - LoadExc)
//
//  Gross * (1 - LoadExc) = Net + Tgt * (LoadTgt - LoadExc)
//
//  Gross * (1 - LoadExc) - Tgt * (LoadTgt - LoadExc) = Net
//
//  Gross * (1 - LoadExc) + Tgt * (LoadExc - LoadTgt) = Net
//
//  Gross - Gross * LoadExc + Tgt * LoadExc - Tgt * LoadTgt = Net
//
//  Gross + Tgt * LoadExc - Gross * LoadExc - Tgt * LoadTgt = Net
//
//  Gross + LoadExc * (Tgt - Gross) - LoadTgt * Tgt = Net
//
//  Gross - LoadExc * (Gross - Tgt) - LoadTgt * Tgt = Net
//
//  which is true as long as Tgt < Gross, which is the condition above

// TODO ?? Don't discuss how to test it--instead, add a unit test.
    // The present function is tougher than its inverse--use inverse to test it
/*
    double n = TieredGrossToNet
        (z
        ,Breakpoint
        ,LoadUpToBreakpoint
        ,LoadBeyondBreakpoint
        );

    double chi = (NetAmount - n) / NetAmount;
    // There is no particular reason for the magic number .000001
    LMI_ASSERT(std::fabs(chi) <= .000001);
*/

    return z;
}
