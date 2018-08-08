// Platform-independent support for report tables.
//
// Copyright (C) 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "report_table.hpp"

#include "alert.hpp"
#include "math_functions.hpp"           // outward_quotient()

/// Compute column widths.
///
/// First, allocate adequate width to each inelastic column; then
/// distribute any excess width left over among elastic columns.
///
/// The width of each inelastic column reflects:
///  - a mask like "999,999" (ideally, there would instead be a
///    quasi-global data structure mapping symbolic column names
///    to their corresponding headers and maximal widths)
///  - the header width
///  - PDF !! the bilateral margins added as a first step below
/// The margins may be slightly reduced by this function to make
/// everything fit when it otherwise wouldn't.

void set_column_widths
    (int                             total_width
    ,int                             column_margin
    ,std::vector<table_column_info>& all_columns
    )
//
// total_width    max table width (page width - page margins)
// column_margin  spacing on both left and right of column
// all_columns    std::vector<table_column_info>
//   table_column_info::col_width_ is the only member changed
{
    // PDF !! Unconditionally add bilateral margins even though they
    // may conditionally be removed below. This is a questionable
    // design decision; if it is later reversed, then remove the
    // comment about it above the implementation.
    for(auto& i : all_columns)
        {
        if(!i.is_elastic())
            {
            i.col_width_ += 2 * column_margin;
            }
        }

    // Number of columns.
    int number_of_columns = 0;

    // Number of elastic columns.
    int number_of_elastic_columns = 0;

    // Total width of all inelastic columns.
    int total_inelastic_width = 0;

    for(auto const& i : all_columns)
        {
        ++number_of_columns;

        if(i.is_elastic())
            {
            ++number_of_elastic_columns;
            }
        else
            {
            total_inelastic_width += i.col_width();
            }
        }

    if(total_width < total_inelastic_width)
        {
        // The inelastic columns don't all fit with their original
        // one-em presumptive bilateral margins. Try to make them fit
        // by reducing the margins slightly.
        //
        // The number of pixels that would need to be removed is:
        auto const overflow = total_inelastic_width - total_width;

        // Because inelastic columns take more than the available
        // horizontal space, there's no room to fit any elastic
        // columns, so the column-fitting problem is overconstrained.
        // Therefore, don't even try reducing margins if there are any
        // elastic columns.
        if(!number_of_elastic_columns)
            {
// Also calculate the number of pixels by which it overflows for each column
            // We need to round up in division here to be sure that all columns
            // fit into the available width.
            auto const overflow_per_column = outward_quotient
                (overflow
                ,number_of_columns
                );
// Now determine whether reducing the margins will make the table fit.
// If that works, then do it; else don't do it, and print a warning.
//
// column_margin is the padding on each side of every column, so
// the number of pixels between columns, as the table was originally
// laid out, is two times column_margin--which, as we just determined,
// was too generous, so we're going to try reducing it.
// Then this conditional compares
//   the number of pixels by which we must shrink each column, to
//   the number of pixels of padding between columns
// Reducing the padding is a workable strategy if the desired reduction
// is less than the padding.
//
// Is this as good as it can be, given that coordinates are integers?
// Answer: Yes--the integers count points, not ems or characters, and
// typographers wouldn't use any finer unit for this task.
            if(overflow_per_column <= 2 * column_margin)
                {
                // We are going to reduce the total width by more than
                // necessary, in general, because of rounding up above, so
                // compensate for it by giving 1 extra pixel until we run out
                // of these "underflow" pixels.
// Defect: the number of pixels separating columns might now be zero.
// '9' is five PDF pixels wide; do we need, say, two pixels between columns?
//
// Suggestion: change the
//   overflow_per_column <= column_margin
// condition to something like:
//    overflow_per_column <= column_margin - 4 // two pixels on each side
//    overflow_per_column <= column_margin - 2 // one pixel on each side
                auto underflow = overflow_per_column * number_of_columns - overflow;

                for(auto& i : all_columns)
                    {
                    i.col_width_ -= overflow_per_column;
                    if(0 < underflow)
                        {
                        ++i.col_width_;
                        --underflow;
                        }
                    }

                column_margin -= (overflow_per_column + 1) / 2;

                // We condensed the columns enough to make them fit, so no need
                // for the warning and we don't have any elastic columns, so
                // we're done.
                return;
                }
// If overflow_per_column is 1, then column_margin -= 1
// "           "          "  2,   "        "           1
// "           "          "  3,   "        "           2
// "           "          "  4,   "        "           2
// The 'underflow' logic shrinks columns by the exact number of pixels
// to use up all the available width. But the column_margin reduction
// isn't exact due to truncation: when the margin is added (on both sides),
// is the total of all (margin+column+margin) widths lower than the maximum,
// so that this is just a small aesthetic issue, or is it too wide, so that
// not everything fits?
//
// Answer:
// This is an issue of aligning the column text, not of fitting, because the
// margin is used when positioning the text inside the column width. And the
// width is correct, so the worst that can happen here is that the text is
// offset by 0.5 pixels -- but, of course, if we rounded it down, it would be
// offset by 0.5 pixels in the other direction. So maybe we should write
//
//     column_margin -= overflow_per_column / 2;
//
// just because it's shorter and not necessarily worse (nor better).
            }

        warning()
            << "Not enough space for all " << number_of_columns << " columns."
            << "\nPrintable width is " << total_width << " points."
            << "\nData alone require " << total_inelastic_width - 2 * column_margin * number_of_columns
            << " points without any margins for legibility."
            << "\nColumn margins of " << column_margin << " points on both sides"
            << " would take up " << 2 * column_margin * number_of_columns << " additional points."
            << LMI_FLUSH
            ;
        return;
        }

    // Lay out elastic columns in whatever space is left over after
    // accounting for all inelastic columns. Clip to make them fit.
    //
    // If there's more than enough space for them, then expand them
    // to consume all available space.
    if(number_of_elastic_columns)
        {
        int const width_of_each_elastic_column = outward_quotient
            (total_width - total_inelastic_width
            ,number_of_elastic_columns
            );

        for(auto& i : all_columns)
            {
            if(i.is_elastic())
                {
                i.col_width_ = width_of_each_elastic_column;
                }
            }
        }
}
