// Ledger xsl operations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ledger_xsl.cpp,v 1.26 2008-01-01 18:29:47 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_xsl.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "dev_null_stream_buffer.hpp" // "idiosyncrasy_spreadsheet" workaround
#include "global_settings.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"       // "idiosyncrasy_spreadsheet" workaround
#include "ledger_formatter.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"
#include "system_command.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <ios>
#include <ostream>                    // "idiosyncrasy_spreadsheet" workaround
#include <sstream>

// Define this macro here, after including all the headers that it
// could potentially affect.

#define LMI_USE_NEW_REPORTS

namespace
{
/// File path for xsl-fo file appropriate for the given ledger.

fs::path xsl_filepath(Ledger const& ledger)
{
    std::string xsl_name = ledger.GetLedgerType().str() + ".xsl";
    fs::path xsl_file(global_settings::instance().data_directory() / xsl_name);
    if(!fs::exists(xsl_file))
        {
        fatal_error()
            << "Unable to read file '"
            << xsl_file.string()
            << "' required for ledger type '"
            << ledger.GetLedgerType()
            << "'."
            << LMI_FLUSH
            ;
        }
    return xsl_file;
}
} // Unnamed namespace.

void experiment0(fs::path const&, fs::path const&, fs::path const&, fs::path const&); // EVGENIY !! EXPERIMENTAL

std::string write_ledger_as_pdf(Ledger const& ledger, fs::path const& filepath, bool experimental)
{
    fs::path print_dir(configurable_settings::instance().print_directory());

    // Ensure that the output filename is portable. Apache fop rejects
    // some names for '-xml' that it accepts for pdf output, without
    // documenting what names it considers valid, so using the boost
    // conventions here is just a guess. Nonportable names that apache
    // fop accepts for pdf output seem not to work well with the most
    // popular msw pdf viewer, at least in a msw 'dde conversation',
    // so use a similar portable name for pdf output.
    //
    // SOMEDAY !! It might be nicer to transform a nonportable name to
    // a portable one, preserving as many original characters as
    // possible--for instance, by filtering out everything but
    // [.-_A-Za-z0-9] and forbidding a terminal period '.'. OTOH, some
    // users might find that more astonishing than the behavior
    // implemented here.
    //
    // USER !! This filename change should be documented for users.
    // Ultimately, for fop, the reason why their nonportable msw
    // filenames must be transformed is that apache fop is java, and
    // java is "portable".
    //
    fs::path real_filepath
        (fs::portable_name(filepath.string())
        ?   filepath
        :   fs::path("output")
        );

    // EXPERIMENTAL This variable should be renamed, e.g., to 'xml_fo_file'.
    fs::path xml_out_file = unique_filepath(print_dir / real_filepath, ".fo.xml");

    fs::ofstream ofs(xml_out_file, ios_out_trunc_binary());
    // Scale a copy of the 'ledger' argument. The original must not be
    // modified because scaling is not reentrant. TODO ?? However,
    // that problem is not avoided here, because what is scaled is
    // actually not copied due to use of shared_ptr; see description
    // of problems in the ledger-class implementation.
    Ledger scaled_ledger(ledger);
    scaled_ledger.AutoScale();
#if defined LMI_USE_NEW_REPORTS
    LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
    LedgerFormatter formatter(factory.CreateFormatter(scaled_ledger));
    formatter.FormatAsXslFo(ofs);
    // TODO ?? Eliminate the problem that this works around.
    if
        (   ledger.GetIsComposite()
        &&  std::string::npos != ledger.GetLedgerInvariant().Comments.find("idiosyncrasy_spreadsheet")
        )
        {
        static dev_null_stream_buffer<char> no_output;
        std::ostream null_stream(&no_output);
        scaled_ledger.write(null_stream);
        }
#else  // !defined LMI_USE_NEW_REPORTS
    scaled_ledger.write(ofs);
#endif // !defined LMI_USE_NEW_REPORTS
    ofs.close();

    fs::path xsl_file = xsl_filepath(ledger);

    fs::path pdf_out_file = unique_filepath(print_dir / real_filepath, ".pdf");

    if(experimental)
        {
        experiment0(filepath, xsl_file, xml_out_file, pdf_out_file);
        }

    std::ostringstream oss;
#if defined LMI_USE_NEW_REPORTS
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -fo "  << '"' << xml_out_file.string() << '"'
        << " -pdf " << '"' << pdf_out_file.string() << '"'
        ;
#else  // !defined LMI_USE_NEW_REPORTS
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -xsl "  << '"' << xsl_file.string()     << '"'
        << " -xml "  << '"' << xml_out_file.string() << '"'
        << " "       << '"' << pdf_out_file.string() << '"'
        ;
#endif // !defined LMI_USE_NEW_REPORTS

    system_command(oss.str());
    return pdf_out_file.string();
}

// EVGENIY !! EXPERIMENTAL section begins.
//
// If we're going to do a lot of things like this, then we should
// consider scaling the ledger only once--perhaps in 'emit_ledger.cpp'
// if we can make that the only place that uses a scaled ledger.
//
// Anyway, what I see here is that
//  - write_ledger_as_xml() does the right thing, but
//  - write_ledger_as_fo_xml() doesn't create the same output that
//      xsltproc does: maybe it doesn't have the full xml dataset?

#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>

#include <iostream>
#include <ostream>

void write_ledger_as_xml(Ledger const& ledger, fs::path const& filepath)
{
    fs::path out_file(filepath.string() + ".xml");
    fs::ofstream ofs(out_file, ios_out_trunc_binary());

    Ledger scaled_ledger(ledger);
    scaled_ledger.AutoScale();

    scaled_ledger.write(ofs);
}

void write_ledger_as_xml2(Ledger const& ledger, fs::path const& filepath)
{
    fs::path input_file(filepath.string() + ".v2.xml");
    fs::ofstream ifs(input_file, ios_out_trunc_binary());

    Ledger scaled_ledger(ledger);
    scaled_ledger.AutoScale();

    LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
    LedgerFormatter formatter(factory.CreateFormatter(scaled_ledger));
    formatter.FormatAsXml(ifs, e_xml_full);
}

void write_ledger_as_fo_xml(Ledger const& ledger, fs::path const& filepath)
{
    fs::path out_file(filepath.string() + ".fo.xml");
    fs::ofstream ofs(out_file, ios_out_trunc_binary());

    Ledger scaled_ledger(ledger);
    scaled_ledger.AutoScale();

    LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
    LedgerFormatter formatter(factory.CreateFormatter(scaled_ledger));
    formatter.FormatAsXslFo(ofs);
}

// EVGENIY !! Here's my testcase:
//
// C:/var/opt/lmi/spool[0]$/opt/lmi/bin/lmi_cli_shared --accept --data_path=/opt/lmi/data --cnsfile=sample.cns --emit=emit_pdf_file,emit_quietly,emit_timings,emit_composite_only 2>&1 |less
//
// I've copied 'sample.cns' from cvs to that directory, in order to
// avoid problems with 'xalan'.
//
// I get four different pdf files as follows:
//   xml -->                       (fop) --> pdf
//   xml --> (xalan)    --> fo --> (fop) --> pdf
//   xml --> (libxslt)  --> fo --> (fop) --> pdf ***
//   xml --> (xsltproc) --> fo --> (fop) --> pdf
// and the one marked "***" looks wrong, while the others seem to
// differ only in minor details, like slightly different alignment
// of the logo on the first page.

void experiment0
    (fs::path const& original_filepath
    ,fs::path const& xsl_file
    ,fs::path const& xml_out_file
    ,fs::path const& pdf_out_file
    )
{
#if 0
// Expunge: use libxslt instead.
    // Use apache tools to produce intermediate xsl-fo output.
    //
    // EVGENIY !! This is strange: apache 'xalan' fails unless I
    // patch the stylesheet, e.g., as follows:
    //   - <xsl:include href="fo_common.xsl"/>
    //   + <xsl:include href="file:/opt/lmi/data/fo_common.xsl"/>
    // which is of course a horribly fragile technique. Are they
    // correct that this is really necessary? It seems very odd that
    // apache 'xalan' seems to need such a change, but apache 'fop'
    // does not--in fact, if I make that change, 'fop' warns about it.
    {
    Timer timer;
    fs::path const fo_out_file = fs::change_extension(pdf_out_file, ".xalan.fo");
    std::string command = configurable_settings::instance().xsl_fo_command();
    // Assume that string is like this: "CMD /c /fop-0.20.5/fop".
    std::string::size_type z = command.rfind("fop");
    LMI_ASSERT(std::string::npos != z);
    command.replace(z, 3, "xalan");
    std::ostringstream oss;
    // Here's another weird apache 'xalan' thing: its '-in' and '-out'
    // don't seem to like full paths.
    oss
        << command
        << " -xsl "  << '"' << xsl_file.string()           << '"'
        << " -in  "  << '"' << xml_out_file.leaf()         << '"'
        << " -out "  << '"' << fo_out_file.leaf()          << '"'
        ;
    std::cout << "Executing command:\n" << "  " << oss.str() << std::endl;
    system_command(oss.str());
    std::cout
        << "...wrote '" << fo_out_file.string() << "'.\n"
        << "  time: " << timer.stop().elapsed_msec_str()
        << std::endl
        ;
    }
#endif // 0

    // apache "area tree" output.
    {
    Timer timer;
    fs::path const at_out_file = fs::change_extension(pdf_out_file, ".at");
    std::ostringstream oss;
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -xsl "  << '"' << xsl_file.string()           << '"'
        << " -xml "  << '"' << xml_out_file.string()       << '"'
        << " -at "   << '"' << at_out_file.string()        << '"'
        ;
    std::cout << "Executing command:\n" << "  " << oss.str() << std::endl;
    system_command(oss.str());
    std::cout
        << "...wrote '" << at_out_file.string() << "'.\n"
        << "  time: " << timer.stop().elapsed_msec_str()
        << std::endl
        ;
    }

    // apache 'fop' call to produce pdf output: same as production,
    // except that here the time is measured.
    {
    Timer timer;
    fs::path pdf_out_file0 = unique_filepath(pdf_out_file, ".pdf0");
    std::ostringstream oss;
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -xsl "  << '"' << xsl_file.string()           << '"'
        << " -xml "  << '"' << xml_out_file.string()       << '"'
        << " "       << '"' << pdf_out_file0.string()      << '"'
        ;
    std::cout << "Executing command:\n" << "  " << oss.str() << std::endl;
    system_command(oss.str());
    std::cout
        << "...wrote '" << pdf_out_file0.string() << "' as usual.\n"
        << "  time: " << timer.stop().elapsed_msec_str()
        << std::endl
        ;
    }

#if 0
// Expunge: use libxslt instead.
    // apache 'fop' call to produce pdf output: differs from the
    // preceding call--this one uses already-transformed fo output
    // from apache 'xalan'.
    {
    Timer timer;
    fs::path const fo_out_file = fs::change_extension(pdf_out_file, ".xalan.fo");
    fs::path pdf_out_file1 = unique_filepath(pdf_out_file, ".pdf1");
    std::ostringstream oss;
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -fo "   << '"' << fo_out_file.string()        << '"'
        << " "       << '"' << pdf_out_file1.string()      << '"'
        ;
    std::cout << "Executing command:\n" << "  " << oss.str() << std::endl;
    system_command(oss.str());
    std::cout
        << "...wrote '" << pdf_out_file1.string() << "' using pre-tranformed fo output from 'xalan'.\n"
        << "  time: " << timer.stop().elapsed_msec_str()
        << std::endl
        ;
    }
#endif // 0

    // apache 'fop' call to produce pdf output: differs from the
    // preceding call--this one uses already-transformed fo output
    // from 'libxslt'.
    //
    // EVGENIY !! You might have to find the file and adjust its
    // name--it comes from a different set of functions that use
    // different filenames, and I haven't had time to work that
    // out yet. But what's really interesting is that this
    // procedure produces a different and apparently incorrect pdf.
    // However, replace the ".fo.xml" file here with output of
    // 'xsltproc', e.g.
    //   /var/opt/lmi/spool[0]$xsltproc -o sample.000000000.fo.xml /lmi/src/lmi/illustration_reg.xsl sample.cns.000000000.xml
    // and the pdf file will be almost identical to the others.
    {
    Timer timer;
    fs::path const fo_out_file = fs::change_extension(original_filepath, ".fo.xml");
    fs::path pdf_out_file2 = unique_filepath(pdf_out_file, ".pdf2");
    std::ostringstream oss;
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -fo "   << '"' << fo_out_file.string()        << '"'
        << " "       << '"' << pdf_out_file2.string()      << '"'
        ;
    std::cout << "Executing command:\n" << "  " << oss.str() << std::endl;
    system_command(oss.str());
    std::cout
        << "...wrote '" << pdf_out_file2.string() << "' using pre-tranformed fo output from 'libxslt'.\n"
        << "  time: " << timer.stop().elapsed_msec_str()
        << std::endl
        ;
    }
}

