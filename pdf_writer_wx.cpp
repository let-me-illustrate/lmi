// PDF generation helpers.
//
// Copyright (C) 2017 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "pdf_writer_wx.hpp"

namespace
{

wxPrintData make_print_data
    (wxString const&    output_filename
    ,wxPrintOrientation orientation
    )
{
    wxPrintData print_data;
    print_data.SetPaperId(wxPAPER_LETTER);
    print_data.SetFilename(output_filename);
    print_data.SetOrientation(orientation);
    return print_data;
}

} // Unnamed namespace.

pdf_writer_wx::pdf_writer_wx
    (wxString const&    output_filename
    ,wxPrintOrientation orientation
    )
    :print_data_    {make_print_data(output_filename, orientation)}
    ,pdf_dc_        {print_data_}
    ,html_parser_   {nullptr}
{
    // Ensure that the output is independent of the current display resolution:
    // it seems that this is only the case with the PDF map mode and wxDC mode
    // different from wxMM_TEXT.
    pdf_dc_.SetMapModeStyle(wxPDF_MAPMODESTYLE_PDF);

    // For simplicity, use points for everything: font sizers are expressed in
    // them anyhow, so it's convenient to use them for everything else too.
    pdf_dc_.SetMapMode(wxMM_POINTS);

    pdf_dc_.StartDoc(wxString()); // Argument is not used.
    pdf_dc_.StartPage();

    // Use a standard PDF Helvetica font (without embedding any custom fonts in
    // the generated file, the only other realistic choice is Times New Roman).
    pdf_dc_.SetFont
        (wxFontInfo(8).Family(wxFONTFAMILY_SWISS).FaceName("Helvetica")
        );

    // Create an HTML parser to allow easily adding HTML contents to the output.
    html_parser_.SetDC(&pdf_dc_);
    html_parser_.SetStandardFonts
        (pdf_dc_.GetFont().GetPointSize()
        ,"Helvetica"
        ,"Courier"
        );

}

pdf_writer_wx::~pdf_writer_wx()
{
    // This will finally generate the PDF file.
    pdf_dc_.EndDoc();
}
