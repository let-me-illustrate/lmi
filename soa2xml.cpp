
#include "soa_helpers.hpp"

#include "actuarial_table.hpp"
#include "value_cast.hpp"

#include <xmlwrapp/attributes.h>
#include <xmlwrapp/document.h>
#include <xmlwrapp/node.h>

#include <ios>
#include <istream>

/************************************************************************
 misc helpers
 ************************************************************************/

template<typename T>
inline const char *as_str(T x)
{
    static std::string tmp;
    tmp = value_cast<std::string>(x);
    return tmp.c_str();
}


/************************************************************************
 conversion code
 ************************************************************************/

xml::node xml_for_aggregate_table(soa_actuarial_table const& t)
{
    xml::node n("aggregate");

    std::vector<double> const values =
        t.values(t.min_age(), t.max_age() - t.min_age() + 1);

    for(int i = 0; i < values.size(); i++)
        {
        xml::node v("value", as_str(values[i]));
        v.get_attributes().insert("age", as_str(t.min_age() + i));
        n.insert(v);
        }

    return n;
}

xml::node xml_for_duration_table(soa_actuarial_table const& t)
{
    xml::node n("duration");

    std::vector<double> const values =
        t.values(t.min_age(), t.max_age() - t.min_age() + 1);

    for(int i = 0; i < values.size(); i++)
        {
        xml::node v("value", as_str(values[i]));
        n.insert(v);
        }

    return n;
}

xml::node xml_for_select_and_ultimate_table(soa_actuarial_table const& t)
{
    xml::node n("select-and-ultimate");

    xml::node n_select("select");
    xml::node n_ultimate("ultimate");

    // Write the <select> portion:
    n_select.get_attributes().insert("period", as_str(t.select_period()));
    for(int age = t.min_age(); age <= t.max_select_age(); age++)
        {
            std::vector<double> data = t.values(age, t.select_period());
            xml::node n_row("row");
            n_row.get_attributes().insert("age", as_str(age));
            for (int s = 0; s < t.select_period(); s++)
                {
                xml::node v("value", as_str(data[s]));
                n_row.insert(v);
                }

            n_select.insert(n_row);
        }

    // Write the <ultimate> portion:
    for(int age = t.min_age(); age <= t.max_select_age(); age++)
        {
        std::vector<double> data = t.values(age, t.select_period() + 1);
        xml::node v("value", as_str(data.back()));
        v.get_attributes().insert("age", as_str(age + t.select_period()));
        n_ultimate.insert(v);
        }
    for(int age = t.max_select_age() + t.select_period() + 1; age <= t.max_age(); age++)
        {
        std::vector<double> data = t.values(age, 1);
        xml::node v("value", as_str(data.back()));
        v.get_attributes().insert("age", as_str(age));
        n_ultimate.insert(v);
        }

    n.insert(n_select);
    n.insert(n_ultimate);

    return n;
}


void export_single_table(char const* filename, int index, char const* description)
{
    fs::path table_path(filename);
    soa_actuarial_table table(filename, index);

    std::cout
        << table.table_type()
        << " table #"
        << index
        << ":\t"
        << description
        << std::endl;

    xml::node root("table");
    root.insert(xml::node("description", description));

    switch(table.table_type())
        {
        case 'A':
            root.insert(xml_for_aggregate_table(table));
            break;

        case 'D':
            root.insert(xml_for_duration_table(table));
            break;

        case 'S':
            root.insert(xml_for_select_and_ultimate_table(table));
            break;

        default:
            error(boost::format("Unknown table type '%1%'.") % table.table_type());
       }

    xml::document doc(root);

    char xmlsuffix[64];
    sprintf(xmlsuffix, "_%d.xtable", index);
    std::string const xmlfile = fs::basename(table_path) + xmlsuffix;
    doc.save_to_file(xmlfile.c_str());
}

void export_soa_file(char const* filename)
{
    const std::vector<soa_record_info> tables = list_soa_file_tables(filename);

    for(std::vector<soa_record_info>::const_iterator i = tables.begin()
        ;i != tables.end()
        ;++i)
        {
        export_single_table(filename, i->index, i->name.c_str());
        }
}


int main(int argc, char *argv[])
{
    try
    {
        for(int i = 1; i < argc; i++)
            {
            export_soa_file(argv[i]);
            }
        return 0;
    }
    catch ( const std::exception& e )
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
