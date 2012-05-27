
#include "actuarial_table.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp" // fs::path inserter

#include <boost/cstdint.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/static_assert.hpp>

#include <climits>   // CHAR_BIT

/************************************************************************
 misc helpers
 ************************************************************************/

inline void error(const boost::format& fmt)
{
    throw std::runtime_error(fmt.str());
}


/************************************************************************
 SOA actuarial table format helpers
 ************************************************************************/

struct soa_record_info
{
    int         index;
    std::string name;
};

std::vector<soa_record_info> list_soa_file_tables(const char *filename)
{
    std::vector<soa_record_info> v;

    fs::path index_path(filename);
    index_path = fs::change_extension(index_path, ".ndx");
    fs::ifstream index_ifs(index_path, ios_in_binary());
    if(!index_ifs)
        {
        error(boost::format("File '%1%' is required but could not be found.") % index_path);
        }

    // Index records have fixed length:
    //   4-byte integer:     table number
    //   50-byte char array: table name
    //   4-byte integer:     byte offset into '.dat' file
    // Table numbers are not necessarily consecutive or sorted.

    // SOA !! Assert endianness too? SOA tables are not portable;
    // probably they can easily be read only on x86 hardware.

    BOOST_STATIC_ASSERT(8 == CHAR_BIT);
    BOOST_STATIC_ASSERT(4 == sizeof(int));
    BOOST_STATIC_ASSERT(2 == sizeof(short int));

    int const index_record_length(58);
    char index_record[index_record_length] = {0};

    BOOST_STATIC_ASSERT(sizeof(boost::int32_t) <= sizeof(int));
    while(index_ifs)
        {
        index_ifs.read(index_record, index_record_length);
        if(index_record_length != index_ifs.gcount())
            {
            if(!index_ifs)
                break;
            error(
                boost::format("Table index file file '%1%': attempted to read %2% bytes, but got %3% bytes instead.")
                % index_path
                % index_record_length
                % index_ifs
                );
            }

        soa_record_info rec;
        rec.index = *reinterpret_cast<boost::int32_t*>(index_record);
        rec.name.assign(index_record + 4);
        v.push_back(rec);
        }

    return v;
}

