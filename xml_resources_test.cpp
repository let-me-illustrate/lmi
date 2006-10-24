// Xml resources--unit test.
//
// Copyright (C) 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: xml_resources_test.cpp,v 1.1.2.8 2006-10-24 13:35:37 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "configurable_settings.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"
#include "timer.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>

#include <libxml/xmlschemas.h>
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>

namespace
{

std::string prepend_xslt_directory(std::string const& filename)
{
    boost::filesystem::path dir;
    try
        {
        dir = configurable_settings::instance().xslt_directory();
        }
    catch(boost::filesystem::filesystem_error const& err)
        {
        std::ostringstream oss;
        oss
            << "Xslt directory is invalid '"
            << configurable_settings::instance().xslt_directory()
            << "'."
            ;
        BOOST_CRITICAL_ERROR(oss.str());
        }

    try
        {
        boost::filesystem::path const file = dir / filename;
        return file.string();
        }
    catch(boost::filesystem::filesystem_error const& err)
        {
        std::ostringstream oss;
        oss
            << "Filename '"
            << filename
            << "' can not be appended to xslt_directory '"
            << configurable_settings::instance().xslt_directory()
            << "'."
            ;
        BOOST_CRITICAL_ERROR(oss.str());
        }

    return filename;
}

/// Class that allows us generate xml output similar to that of Ledger
/// but which could contain deliberatly injected errors for testing purposes.

class LedgerOutput
{
  public:
    LedgerOutput() {}
    LedgerOutput(LedgerOutput const& rhs)
        :scalars_(rhs.scalars_)
        ,vectors_(rhs.vectors_)
    {}

    // put scalar value
    LedgerOutput& set
        (std::string const& node
        ,std::string const& name
        ,std::string const& basis // empty means no basis
        ,std::string const& value
        )
    {
        scalars_[std::make_pair(node, std::make_pair(name, basis))] = value;
        return *this;
    }
    // put vector value
    LedgerOutput& set
        (std::string const& node
        ,std::string const& name
        ,std::string const& basis // empty means no basis
        ,std::vector<std::string> const& values
    )
    {
        vectors_[std::make_pair(node, std::make_pair(name, basis))] = values;
        return *this;
    }

    std::string output() const
    {
        xml_lmi::Document doc;
        xml_lmi::Element& root = *doc.create_root_node("illustration");

        for
            (scalars_t::const_iterator sit = scalars_.begin()
            ,send = scalars_.end()
            ;sit != send
            ;++sit
            )
            {
            node_t const& node = sit->first;

            xml_lmi::Element& scalar = *root.add_child(node.first);
            scalar.set_attribute("name", node.second.first);
            if (!node.second.second.empty())
                {
                scalar.set_attribute("basis", node.second.second);
                }

            scalar.add_child_text( sit->second );
            }
        for
            (vectors_t::const_iterator vit = vectors_.begin()
            ,vend = vectors_.end()
            ;vit != vend
            ;++vit
            )
            {
            node_t const& node = vit->first;

            xml_lmi::Element& vector_node = *root.add_child(node.first);
            vector_node.set_attribute("name", node.second.first);
            if (!node.second.second.empty())
                {
                vector_node.set_attribute("basis", node.second.second);
                }

            for
                (std::vector<std::string>::const_iterator dit = vit->second.begin()
                ,dend = vit->second.end()
                ;dit != dend
                ;++dit
                )
                {
                vector_node.add_child("duration")->add_child_text( *dit );
                }
        }

        std::ostringstream oss;
        oss << doc;
        return oss.str();
    }

  private:
    // node, name, basis, i.e. <$node name='$name' basis='$basis' />
    typedef std::pair<std::string, std::pair<std::string, std::string> > node_t;
    // scalars
    typedef std::map<node_t, std::string> scalars_t;
    // vectors
    typedef std::map<node_t, std::vector<std::string> > vectors_t;
    scalars_t scalars_;
    vectors_t vectors_;
};

bool validate_xml_doc_against_schema(xmlDocPtr doc, xmlSchemaPtr schema)
{
    if(!doc || !schema)
        return false;
    xmlSchemaValidCtxtPtr vctxt
        = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors
        (vctxt
        ,(xmlSchemaValidityErrorFunc) fprintf
        ,(xmlSchemaValidityWarningFunc) fprintf
        ,stderr
        );
    int ret = xmlSchemaValidateDoc(vctxt, doc);
    xmlSchemaFreeValidCtxt(vctxt);
    return (ret == 0);
}

bool validate_ledger_against_schema(LedgerOutput const& output, xmlSchemaPtr schema)
{
    std::string xml = output.output();
    xmlDocPtr doc = xmlParseMemory(xml.c_str(), xml.size());
    return validate_xml_doc_against_schema(doc, schema);
}

bool apply_xslt_to_doc(std::string const& filename, xmlDocPtr doc)
{
    xsltStylesheetPtr xsl = LedgerFormatterFactory::Instance().GetStylesheet(filename);
    xmlDocPtr res = xsltApplyStylesheet(xsl, doc, NULL);
    bool result = (res != NULL);
    xmlFreeDoc(res);
    return result;
}

} // Unnamed namespace.

int test_main(int, char*[])
{
    configurable_settings const& cs
        (configurable_settings::instance()
        );

    // test schema.xsd itself
    // ----------------------
    std::string const schema_filename
        (prepend_xslt_directory(cs.xml_schema_filename())
        );

    xmlSchemaParserCtxtPtr schema_ctxt
        (xmlSchemaNewParserCtxt(schema_filename.c_str())
        );
    BOOST_TEST(schema_ctxt != NULL);

    xmlSchemaSetParserErrors
        (schema_ctxt
        ,(xmlSchemaValidityErrorFunc) fprintf
        ,(xmlSchemaValidityWarningFunc) fprintf
        ,stderr
        );
    xmlSchemaPtr schema = xmlSchemaParse(schema_ctxt);
    xmlSchemaFreeParserCtxt(schema_ctxt);
    BOOST_TEST(schema != NULL);

    std::string format_xml_filename
        (prepend_xslt_directory(cs.xslt_format_xml_filename())
        );
    xmlDocPtr format_xml = xmlParseFile(format_xml_filename.c_str());
    BOOST_TEST(validate_xml_doc_against_schema(format_xml, schema));

    // Must validate
    typedef std::vector<std::string> str_vector_t;
    LedgerOutput ledger0;
    ledger0
        .set("string_scalar", "AvName", "", "Account")
        .set("double_scalar", "InitAnnGenAcctInt", "run_curr_basis", "6.00%")
        .set("double_scalar", "Age", "", "45")
        .set("double_vector", "Outlay", "", str_vector_t(10, "20,000"))
        .set("string_vector", "DBOpt", "", str_vector_t(10, "A"))
        ;
    BOOST_TEST(validate_ledger_against_schema(ledger0, schema));

    // Must fail: scalar node is not defined
    LedgerOutput ledger1 = ledger0;
    ledger1.set("scalar", "name", "", "45");
    BOOST_TEST( !validate_ledger_against_schema(ledger1, schema));

    // Must fail: 'basis' is unknown basis value
    LedgerOutput ledger2 = ledger0;
    ledger2.set("string_scalar", "Age", "basis", "45");
    BOOST_TEST( !validate_ledger_against_schema(ledger2, schema));

    // Must fail: name 'Age' has to be unique across all types of values
    LedgerOutput ledger3 = ledger0;
    ledger3.set("string_scalar", "Age", "", "10");
    BOOST_TEST( !validate_ledger_against_schema(ledger3, schema));

    // Must fail: invalid numeric value '10.000,00' supplied for 'Age'
    LedgerOutput ledger4 = ledger0;
    ledger4.set("string_scalar", "Age", "", "10.000,00");
    BOOST_TEST( !validate_ledger_against_schema(ledger4, schema));

    // Must fail: node 'Age' is string_vector, but has a scalar value
    LedgerOutput ledger5 = ledger0;
    ledger5.set("string_vector", "Age", "", "45");
    BOOST_TEST( !validate_ledger_against_schema(ledger5, schema));

    // Must fail: node 'Age' is string_scalar, but has a vector value
    LedgerOutput ledger6 = ledger0;
    ledger6.set("string_scalar", "Age", "", str_vector_t(1, "45"));
    BOOST_TEST( !validate_ledger_against_schema(ledger6, schema));

    // take a valid simple xml output and test xsl templates on it
    std::string output_xml = ledger0.output();
    xmlDocPtr output_doc = xmlParseMemory(output_xml.c_str(), output_xml.size());

    // test html.xsl
    BOOST_TEST( apply_xslt_to_doc(cs.xslt_html_filename(), output_doc) );

    // test tab_delimited.xsl on output.xml
    BOOST_TEST( apply_xslt_to_doc(cs.xslt_tab_delimited_filename(), output_doc) );

    // test <xsl-fo>.xsl files on output.xml
    // BOOST_TEST( apply_xslt_to_doc(cs.xslt_html_filename(), output_doc) );

    return EXIT_SUCCESS;
}

