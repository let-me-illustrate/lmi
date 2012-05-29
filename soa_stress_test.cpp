// Test SOA tables against xml equivalents.
//
// Copyright (C) 2012 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "soa_helpers.hpp"

#include "actuarial_table.hpp"
#include "assert_lmi.hpp"
#include "materially_equal.hpp"

#include <cmath>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>

inline bool almost_equal_doubles(double a, double b)
{
    return materially_equal(a, b, 2.0E-15L);
}

inline bool almost_equal_doubles(std::vector<double> const& a, std::vector<double> const& b)
{
    if(a.size() != b.size())
        return false;

    size_t const size = a.size();
    for(size_t i = 0; i < size; i++)
        {
        if(!almost_equal_doubles(a[i], b[i]))
            return false;
        }

    return true;
}

void report_vector_difference
    (int start
    ,std::vector<double> const& data_xml
    ,std::vector<double> const& data_soa
    )
{
    std::cerr << boost::format("Results differ for %1% values starting at %2%:")
               % data_xml.size()
               % start
              << std::endl;

    std::cerr << boost::format("   \t%|25|\t%|25|") % "xml" % "soa" << std::endl;
    for(unsigned int i = 0; i < data_xml.size(); i++)
        {
            if(!almost_equal_doubles(data_xml[i], data_soa[i]))
                {
                std::cerr << boost::format("[%d]\t%|25.20|\t%|25.20|")
                           % i
                           % data_xml[i]
                           % data_soa[i]
                          << std::endl;
                }
        }
    throw std::runtime_error("XML table data are incorrect");
}

void test_single_table(char const* filename, int index)
{
    soa_actuarial_table soa(filename, index);
    xml_actuarial_table xml(filename, index);

    LMI_ASSERT( soa.table_type() == xml.table_type()         );

    if (xml.table_type() != e_table_duration)
        {
        LMI_ASSERT( soa.min_age() == xml.min_age()               );
        LMI_ASSERT( soa.max_age() == xml.max_age()               );
        }

    if(xml.table_type() == e_table_select_and_ultimate)
        {
        LMI_ASSERT( soa.select_period() == xml.select_period()   );
        LMI_ASSERT( soa.max_select_age() == xml.max_select_age() );
        }

    for(int start = xml.min_age(); start < xml.max_age(); start++)
        {
        for(int length = 1; length <= xml.max_age() - start; length++)
            {
            std::vector<double> result_xml = xml.values(start, length);
            std::vector<double> result_soa = soa.values(start, length);
            LMI_ASSERT(result_xml.size() == static_cast<unsigned int>(length));
            LMI_ASSERT(result_soa.size() == static_cast<unsigned int>(length));
            if(!almost_equal_doubles(result_xml, result_soa))
                report_vector_difference(start, result_xml, result_soa);
            }
        }
}

void stress_test(char const* filename)
{
    std::vector<soa_record_info> const tables = list_soa_file_tables(filename);

    for(std::vector<soa_record_info>::const_iterator i = tables.begin()
        ;i != tables.end()
        ;++i)
        {
        try
            {
            std::cout << "Testing " << filename << ", " << i->index  << std::endl;
            test_single_table(filename, i->index);
            }
        catch(std::exception const& e)
            {
            error(boost::format("In file %1%, table %2%:\n%3%")
                % filename
                % i->index
                % e.what());
            }
        }
}

int main(int argc, char* argv[])
{
    try
        {
        for(int i = 1; i < argc; i++)
            {
            stress_test(argv[i]);
            }
        return 0;
        }
    catch(std::exception const& e)
        {
        std::cerr << "Error:" << std::endl << e.what() << std::endl;
        return 1;
        }
}

