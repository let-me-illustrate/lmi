// Generate group premium quote PDF file.
//
// Copyright (C) 2015, 2016 Gregory W. Chicares.
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

#include "group_quote_pdf_gen.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"            // jdn_t()
#include "data_directory.hpp"           // AddDataDir()
#include "force_linking.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "ledger_text_formats.hpp"      // ledger_format()
#include "mc_enum_types_aux.hpp"        // is_subject_to_ill_reg()
#include "miscellany.hpp"               // split_into_lines()
#include "oecumenic_enumerations.hpp"   // oenum_format_style
#include "path_utility.hpp"             // fs::path inserter
#include "version.hpp"
#include "wx_table_generator.hpp"
#include "wx_utility.hpp"               // ConvertDateToWx()

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>

#include <wx/datetime.h>
#include <wx/html/htmlcell.h>
#include <wx/html/winpars.h>
#include <wx/image.h>
#include <wx/pdfdc.h>

#include <limits>
#include <stdexcept>
#include <utility>                      // std::pair
#include <vector>

LMI_FORCE_LINKING_IN_SITU(group_quote_pdf_generator_wx)

namespace
{

enum enum_output_mode
    {e_output_normal
    ,e_output_measure_only
    };

/// Escape special XML characters in the given string, ensuring that it appears
/// correctly inside HTML element contents. Notice that we don't need to escape
/// quotes here as we never use the result of this function inside an HTML
/// attribute, only inside HTML elements.

wxString escape_for_html_elem(std::string const& s)
{
    wxString const u = wxString::FromUTF8(s.c_str());

    wxString z;
    z.reserve(u.length());
    for(wxString::const_iterator i = u.begin(); i != u.end(); ++i)
        {
        switch((*i).GetValue())
            {
            case '<': z += "&lt;" ; break;
            case '>': z += "&gt;" ; break;
            case '&': z += "&amp;"; break;
            default : z += *i     ;
            }
        }
    return z;
}

/// Namespace for helpers used for HTML generation.

namespace html
{

/// Namespace for the support HTML tags.
///
/// Tags are only used as template arguments, so they don't need to be defined,
/// just declared -- and tag_info below specialized for them.

namespace tag
{

struct b;
struct br;

} // namespace tag

template<typename T>
struct tag_info;

template<>
struct tag_info<tag::b>
{
    static char const* get_name() { return "b"; }
    static bool has_end() { return true; }
};

template<>
struct tag_info<tag::br>
{
    static char const* get_name() { return "br"; }
    static bool has_end() { return false; }
};

} // namespace html

/// Wrap the given text in an HTML tag if it is not empty, otherwise just
/// return an empty string.
///
/// For the tags without matching closing tags, such as e.g. "<br>", wrapping
/// the text means just prepending the tag to it. This is still done only if
/// the text is not empty.

template<typename T>
wxString wrap_if_not_empty(wxString const& html)
{
    wxString result;
    if(!html.empty())
        {
        result << '<' << html::tag_info<T>::get_name() << '>' << html;
        if(html::tag_info<T>::has_end())
            {
            result << "</" << html::tag_info<T>::get_name() << '>';
            }
        }

    return result;
}

/// Transform 'html' -> '<br><br>html', but return empty string unchanged.

wxString brbr(std::string const& html)
{
    return
        wrap_if_not_empty<html::tag::br>
            (wrap_if_not_empty<html::tag::br>
                (escape_for_html_elem(html)
                )
            );
}

/// Transform 'html' -> '<br><br><b>html</b>', but return empty string unchanged.

wxString brbrb(std::string const& html)
{
    return
        wrap_if_not_empty<html::tag::br>
            (wrap_if_not_empty<html::tag::br>
                (wrap_if_not_empty<html::tag::b>
                    (escape_for_html_elem(html)
                    )
                )
            );
}

/// Generate HTML representation of a field name and value in an HTML table and
/// append it to the specified string, defining the HTML.
///
/// The HTML fragment generated by this function contains two <td> tags with
/// the given contents.

void append_name_value_to_html_table
    (wxString& html_table
    ,std::string const& name
    ,std::string const& value
    )
{
    html_table += wxString::Format
        ("<td nowrap align=\"right\"><b>%s%s&nbsp;&nbsp;</b></td>"
         "<td>%s&nbsp;&nbsp;&nbsp;&nbsp;</td>"
        ,escape_for_html_elem(name)
        ,(value.empty() ? "" : ":")
        ,escape_for_html_elem(value)
        );
}

/// Append the given tag to the string and ensure that the matching closing tag
/// will be appended to it later.
///
/// This helper class helps with not forgetting the closing tags in the
/// generated HTML. It doesn't actually make it impossible to forget them (this
/// would require much heavier API and seems not to be worth it), but makes it
/// less likely.

class open_and_ensure_closing_tag
    :private lmi::uncopyable<open_and_ensure_closing_tag>
{
  public:
    open_and_ensure_closing_tag(wxString& html, char const* tag)
        :html_(html)
        ,tag_(tag)
    {
        html_ << "<" << tag_ << ">";
    }

    ~open_and_ensure_closing_tag()
    {
        html_ << "</" << tag_ << ">";
    }

  private:
    wxString& html_;
    wxString const tag_;
};

/// Simple description of a custom field, consisting of a non-empty name and a
/// possibly empty string value.
///
/// Objects of this class have value semantics.

struct extra_summary_field
{
    std::string name;
    std::string value;
};

/// Function parsing a multiline string of the form "name: value" as an array
/// of extra summary fields.

std::vector<extra_summary_field> parse_extra_report_fields(std::string const& s)
{
    std::vector<std::string> const lines = split_into_lines(s);

    std::vector<extra_summary_field> fields;
    fields.reserve(lines.size());

    typedef std::vector<std::string>::const_iterator vsci;
    for(vsci i = lines.begin(); i != lines.end(); ++i)
        {
        // Ignore the empty or blank lines, they could be added for readability
        // reasons and this also deals with the problem of split_into_lines()
        // returning a vector of a single empty line even if the source string
        // is entirely empty.
        if(i->find_first_not_of(' ') == std::string::npos)
            {
            continue;
            }

        extra_summary_field field;

        std::string::size_type const pos_colon = i->find(':');

        // Notice that substr() call is correct even if there is no colon in
        // this line, i.e. pos_colon == npos.
        field.name = i->substr(0, pos_colon);

        if(pos_colon != std::string::npos)
            {
            // Skip any spaces after the colon as this is what would be
            // normally expected by the user.
            std::string::size_type const
                pos_value = i->find_first_not_of(' ', pos_colon + 1);

            if(pos_value != std::string::npos)
                {
                field.value = i->substr(pos_value);
                }
            }
        // else: If there is no colon or nothing but space after it, just leave
        // the value empty, this is unusual, but not considered to be an error.

        fields.push_back(field);
        }

    return fields;
}

/// Load the image from the given file.
///
/// Look for the file in the current working directory, or, if that
/// fails, in lmi's data directory. Warn if it's not found in either
/// of those locations, or if it's found but cannot be loaded.
///
/// Diagnosed failures are presented merely as warnings so that quotes
/// can be produced even with a generic system built from svn only,
/// with no (proprietary) images.

wxImage load_image(char const* file)
{
    fs::path image_path(file);
    if(!fs::exists(image_path))
        {
        image_path = AddDataDir(file);
        }
    if(!fs::exists(image_path))
        {
        warning()
            << "Unable to find image '"
            << image_path
            << "'. Try reinstalling."
            << "\nA blank image will be used instead."
            << LMI_FLUSH
            ;
        return wxImage();
        }

    wxImage image(image_path.string().c_str(), wxBITMAP_TYPE_PNG);
    if(!image.IsOk())
        {
        warning()
            << "Unable to load image '"
            << image_path
            << "'. Try reinstalling."
            << "\nA blank image will be used instead."
            << LMI_FLUSH
            ;
        return wxImage();
        }

    return image;
}

/// Output an image at the given scale into the PDF.
///
/// The scale specifies how many times the image should be shrunk, i.e. scale >
/// 1 makes the image smaller while scale < 1 makes it larger.
///
/// Updates pos_y by increasing it by the height of the specified image at the
/// given scale.

void output_image
    (wxPdfDC&         pdf_dc
    ,wxImage const&   image
    ,char const*      image_name
    ,double           scale
    ,int              x
    ,int*             pos_y
    ,enum_output_mode output_mode = e_output_normal
    )
{
    int const y = wxRound(image.GetHeight() / scale);

    switch(output_mode)
        {
        case e_output_normal:
            {
            // Use wxPdfDocument API directly as wxDC doesn't provide a way to
            // set the image scale at PDF level and also because passing via
            // wxDC wastefully converts wxImage to wxBitmap only to convert it
            // back to wxImage when embedding it into the PDF.
            wxPdfDocument* const pdf_doc = pdf_dc.GetPdfDocument();
            LMI_ASSERT(pdf_doc);

            pdf_doc->SetImageScale(scale);
            pdf_doc->Image(image_name, image, x, *pos_y);
            pdf_doc->SetImageScale(1);
            }
            break;
        case e_output_measure_only:
            // Do nothing.
            break;
        default:
            {
            fatal_error() << "Case " << output_mode << " not found." << LMI_FLUSH;
            }
        }

    *pos_y += y;
}

/// Render, or just pretend rendering in order to measure it, the given HTML
/// contents at the specified position wrapping it at the given width.
/// Return the height of the output (using this width).

int output_html
    (wxHtmlWinParser& html_parser
    ,int x
    ,int y
    ,int width
    ,wxString const& html
    ,enum_output_mode output_mode = e_output_normal
    )
{
    boost::scoped_ptr<wxHtmlContainerCell> const cell
        (static_cast<wxHtmlContainerCell*>(html_parser.Parse(html))
        );
    LMI_ASSERT(cell);

    cell->Layout(width);
    switch(output_mode)
        {
        case e_output_normal:
            {
            wxHtmlRenderingInfo rendering_info;
            cell->Draw
                (*html_parser.GetDC()
                ,x
                ,y
                ,0
                ,std::numeric_limits<int>::max()
                ,rendering_info
                );
            }
            break;
        case e_output_measure_only:
            // Do nothing.
            break;
        default:
            {
            fatal_error() << "Case " << output_mode << " not found." << LMI_FLUSH;
            }
        }

    return cell->GetHeight();
}

enum enum_group_quote_columns
    {e_col_number
    ,e_col_name
    ,e_col_age
    ,e_col_dob
    ,e_col_basic_face_amount
    ,e_col_basic_premium
    ,e_col_supplemental_face_amount
    ,e_col_additional_premium
    ,e_col_total_face_amount
    ,e_col_total_premium
    ,e_col_max
    };

enum_group_quote_columns const e_first_totalled_column = e_col_basic_face_amount;

struct column_definition
{
    char const* header_;
    char const* widest_text_; // Empty string means variable width.
};

// Headers of premium columns include dynamically-determined payment
// mode, so they're actually format strings.

column_definition const column_definitions[] =
    {{"Part#"                          ,            "99999"   } // e_col_number
    ,{"Participant"                    ,                 ""   } // e_col_name
    ,{"Issue Age"                      ,              "999"   } // e_col_age
    ,{"Date of Birth"                  ,       "9999-99-99"   } // e_col_dob
    ,{"Basic\nFace Amount"             , "$999,999,999,999"   } // e_col_basic_face_amount
    ,{"Basic\n%s\nPremium"             ,   "$9,999,999,999.00"} // e_col_basic_premium
    ,{"Supplemental\nFace Amount"      , "$999,999,999,999"   } // e_col_supplemental_face_amount
    ,{"Additional\n%s\nPremium"        ,   "$9,999,999,999.00"} // e_col_additional_premium
    ,{"Total\nFace Amount"             , "$999,999,999,999"   } // e_col_total_face_amount
    ,{"Total\n%s\nPremium"             ,   "$9,999,999,999.00"} // e_col_total_premium
    };

BOOST_STATIC_ASSERT(sizeof column_definitions / sizeof(column_definitions[0]) == e_col_max);

class group_quote_pdf_generator_wx
    :public group_quote_pdf_generator
{
  public:
    static boost::shared_ptr<group_quote_pdf_generator> do_create()
        {
        return boost::shared_ptr<group_quote_pdf_generator>
                (new group_quote_pdf_generator_wx()
                );
        }

    virtual void add_ledger(Ledger const& ledger);
    virtual void save(std::string const& output_filename);

  private:
    // These margins are arbitrary and can be changed to conform to subjective
    // preferences.
    static int const horz_margin = 24;
    static int const vert_margin = 36;
    static int const vert_skip   = 12;

    // Ctor is private as it is only used by do_create().
    group_quote_pdf_generator_wx();

    // Generate the PDF once we have all the data.
    void do_generate_pdf(wxPdfDC& pdf_dc);

    // Compute the number of pages needed by the table rows in the output given
    // the space remaining on the first page, the heights of the header, one
    // table row and the footer and the last row position.
    // Remaining space contains the space on the first page on input and is
    // updated with the space remaining on the last page on output.
    int compute_pages_for_table_rows
        (int* remaining_space
        ,int  header_height
        ,int  row_height
        ,int  last_row_y
        );

    void output_page_number_and_version
        (wxPdfDC& pdf_dc
        ,int      total_pages
        ,int      current_page
        );
    void output_image_header
        (wxPdfDC& pdf_dc
        ,int*     pos_y
        );
    void output_document_header
        (wxPdfDC&         pdf_dc
        ,wxHtmlWinParser& html_parser
        ,int*             pos_y
        );
    void output_table_totals
        (wxPdfDC&            pdf_dc
        ,wx_table_generator& table_gen
        ,int*                pos_y
        );
    void output_footer
        (wxPdfDC&         pdf_dc
        ,wxHtmlWinParser& html_parser
        ,int*             pos_y
        ,enum_output_mode output_mode = e_output_normal
        );

    struct global_report_data
        {
        // Extract header and footer fields from a ledger.
        void fill_global_report_data(Ledger const& ledger);

        // Fixed fields that are always defined.
        std::string company_;
        std::string prepared_by_;
        std::string product_;
        std::string short_product_;
        std::string available_riders_;
        std::string premium_mode_;
        std::string contract_state_;
        std::string effective_date_;
        std::string footer_;

        // Optional supplementary fields.
        std::vector<extra_summary_field> extra_fields_;
        };
    global_report_data report_data_;

    struct row_data
        {
        std::string values[e_col_max];
        };
    std::vector<row_data> rows_;

    class totals_data
    {
      public:
        totals_data()
            {
            for(int col = e_first_totalled_column; col < e_col_max; ++col)
                {
                value(col) = 0.0;
                }
            }

        void total(int col, double d)
            {
            value(col) = d;
            }

        double total(int col) const
            {
            return const_cast<totals_data*>(this)->value(col);
            }

      private:
        double& value(int col) { return values_[col - e_first_totalled_column]; }

        double values_[e_col_max - e_first_totalled_column];
    };
    totals_data totals_;

    struct page_metrics
        {
        page_metrics()
            :width_(0)
            {
            }

        void initialize(wxDC const& dc)
            {
            total_size_ = dc.GetSize();
            width_ = total_size_.x - 2 * horz_margin;
            }

        wxSize total_size_;
        int width_;
        };
    page_metrics page_;

    int row_num_;
};

group_quote_pdf_generator_wx::group_quote_pdf_generator_wx()
    :row_num_(0)
{
}

void assert_nonblank(std::string const& value, std::string const& name)
{
    if(std::string::npos == value.find_first_not_of(" \f\n\r\t\v"))
        {
        fatal_error() << name << " must not be blank." << LMI_FLUSH;
        }
}

/// Copy global report data from ledger.
///
/// All ledger data used here must be checked for consistency upstream
/// by assert_okay_to_run_group_quote(); therefore, any changes in the
/// set of data used here should be reflected there.

void group_quote_pdf_generator_wx::global_report_data::fill_global_report_data
    (Ledger const& ledger
    )
{
    LedgerInvariant const& invar = ledger.GetLedgerInvariant();

    company_          = invar.CorpName;
    prepared_by_      = invar.ProducerName;
    product_          = invar.PolicyMktgName;
    short_product_    = invar.GroupQuoteShortProductName;
    available_riders_ = invar.GroupQuoteRidersHeader;
    premium_mode_     = invar.InitErMode;
    contract_state_   = invar.GetStatePostalAbbrev();
    jdn_t eff_date    = jdn_t(static_cast<int>(invar.EffDateJdn));
    effective_date_   = ConvertDateToWx(eff_date).FormatDate().ToStdString();
    // Deliberately begin the footer with <br> tags, to separate it
    // from the logo right above it.
    footer_ =
          brbr (invar.GroupQuoteIsNotAnOffer)
        + brbr (invar.GroupQuoteRidersFooter)
        + brbr (invar.GroupQuotePolicyFormId)
        + brbr (invar.GroupQuoteStateVariations)
        + brbr (invar.MarketingNameFootnote)
        + brbrb(invar.GroupQuoteProspectus)
        + brbr (invar.GroupQuoteUnderwriter)
        + brbr (invar.GroupQuoteBrokerDealer)
        ;

    assert_nonblank(company_         , "Sponsor");
    assert_nonblank(prepared_by_     , "Agent");
    assert_nonblank(product_         , "Product name");
    assert_nonblank(short_product_   , "Product ID");
    assert_nonblank(available_riders_, "Available riders"); // If none, should say "none".
    assert_nonblank(premium_mode_    , "Mode");
    assert_nonblank(contract_state_  , "State");
    assert_nonblank(effective_date_  , "Effective date");

    assert_nonblank(invar.GroupQuoteIsNotAnOffer   , "First footnote");
    assert_nonblank(invar.GroupQuoteRidersFooter   , "Second footnote");
    assert_nonblank(invar.GroupQuotePolicyFormId   , "Third footnote");
    assert_nonblank(invar.GroupQuoteStateVariations, "Fourth footnote");
    assert_nonblank(invar.MarketingNameFootnote    , "Fifth footnote");
    // Somewhat casually, assume a contract is variable if it's not
    // subject to the NAIC illustration reg.
    if(!is_subject_to_ill_reg(ledger.ledger_type()))
        {
        assert_nonblank(invar.GroupQuoteProspectus  , "Sixth footnote");
        assert_nonblank(invar.GroupQuoteUnderwriter , "Seventh footnote");
        assert_nonblank(invar.GroupQuoteBrokerDealer, "Eighth footnote");
        }

    extra_fields_     = parse_extra_report_fields(invar.Comments);
}

void group_quote_pdf_generator_wx::add_ledger(Ledger const& ledger)
{
    if(0 == ledger.GetCurrFull().LapseYear)
        {
        fatal_error() << "Lapsed during first year." << LMI_FLUSH;
        }

    LedgerInvariant const& invar = ledger.GetLedgerInvariant();

    int const year = 0;

    std::pair<int, oenum_format_style> const f0(0, oe_format_normal);
    std::pair<int, oenum_format_style> const f2(2, oe_format_normal);

    bool const is_composite = ledger.is_composite();

    row_data rd;
    for(int col = 0; col < e_col_max; ++col)
        {
        // The cast is only used to ensure that if any new elements are added
        // to the enum, the compiler would warn about their values not being
        // present in this switch.
        switch(static_cast<enum_group_quote_columns>(col))
            {
            case e_col_number:
                {
                // Row numbers shown to human beings should be 1-based.
                rd.values[col] = wxString::Format("%d", row_num_ + 1).ToStdString();
                }
                break;
            case e_col_name:
                {
                rd.values[col] = invar.Insured1;
                }
                break;
            case e_col_age:
                {
                rd.values[col] = wxString::Format("%.0f", invar.Age).ToStdString();
                }
                break;
            case e_col_dob:
                {
                rd.values[col] = ConvertDateToWx
                    (jdn_t(static_cast<int>(invar.DateOfBirthJdn))
                    ).FormatDate();
                }
                break;
            case e_col_basic_face_amount:
                {
                double const z = invar.SpecAmt.at(year);
                rd.values[col] = '$' + ledger_format(z, f0);
                if(is_composite)
                    {
                    totals_.total(col, z);
                    }
                }
                break;
            case e_col_basic_premium:
                {
                double const z = invar.ErModalMinimumPremium.at(year);
                rd.values[col] = '$' + ledger_format(z, f2);
                if(is_composite)
                    {
                    totals_.total(col, z);
                    }
                }
                break;
            case e_col_supplemental_face_amount:
                {
                double const z = invar.TermSpecAmt.at(year);
                rd.values[col] = '$' + ledger_format(z, f0);
                if(is_composite)
                    {
                    totals_.total(col, z);
                    }
                }
                break;
            case e_col_additional_premium:
                {
                double const z = invar.EeModalMinimumPremium.at(year) + invar.InitMinDumpin;
                rd.values[col] = '$' + ledger_format(z, f2);
                if(is_composite)
                    {
                    totals_.total(col, z);
                    }
                }
                break;
            case e_col_total_face_amount:
                {
                double const z = invar.SpecAmt.at(year) + invar.TermSpecAmt.at(year);
                rd.values[col] = '$' + ledger_format(z, f0);
                if(is_composite)
                    {
                    totals_.total(col, z);
                    }
                }
                break;
            case e_col_total_premium:
                {
                double const z = invar.ModalMinimumPremium.at(year) + invar.InitMinDumpin;
                rd.values[col] = '$' + ledger_format(z, f2);
                if(is_composite)
                    {
                    totals_.total(col, z);
                    }
                }
                break;
            case e_col_max:
                {
                fatal_error() << "Unreachable." << LMI_FLUSH;
                }
                break;
            default:
                {
                fatal_error() << "Case " << col << " not found." << LMI_FLUSH;
                }
            }
        }

    // The composite ledger arrives last. It is used only for global
    // data (which have already been asserted, upstream, not to vary
    // by cell) and for totals. It is neither shown in the main table
    // nor counted as a row. Only at this point, after all the other
    // ledgers have been seen, can all-zero columns (and corresponding
    // total columns) be suppressed.
    if(is_composite)
        {
        report_data_.fill_global_report_data(ledger);
        }
    else
        {
        rows_.push_back(rd);
        row_num_++;
        }
}

void group_quote_pdf_generator_wx::save(std::string const& output_filename)
{
    // Create a wxPrintData object just to describe the paper to use.
    wxPrintData print_data;
    print_data.SetOrientation(wxLANDSCAPE);
    print_data.SetPaperId(wxPAPER_LETTER);
    print_data.SetFilename(output_filename);

    wxPdfDC pdf_dc(print_data);
    page_.initialize(pdf_dc);
    do_generate_pdf(pdf_dc);
    pdf_dc.EndDoc();
}

void group_quote_pdf_generator_wx::do_generate_pdf(wxPdfDC& pdf_dc)
{
    // Ensure that the output is independent of the current display resolution:
    // it seems that this is only the case with the PDF map mode and wxDC mode
    // different from wxMM_TEXT.
    pdf_dc.SetMapModeStyle(wxPDF_MAPMODESTYLE_PDF);

    // For simplicity, use points for everything: font sizers are expressed in
    // them anyhow, so it's convenient to use them for everything else too.
    pdf_dc.SetMapMode(wxMM_POINTS);

    pdf_dc.StartDoc(wxString()); // Argument is not used.
    pdf_dc.StartPage();

    // Use a standard PDF Helvetica font (without embedding any custom fonts in
    // the generated file, the only other realistic choice is Times New Roman).
    pdf_dc.SetFont
        (wxFontInfo(8).Family(wxFONTFAMILY_SWISS).FaceName("Helvetica")
        );

    // Create an HTML parser to allow easily adding HTML contents to the output.
    wxHtmlWinParser html_parser(NULL);
    html_parser.SetDC(&pdf_dc);
    html_parser.SetStandardFonts
        (pdf_dc.GetFont().GetPointSize()
        ,"Helvetica"
        ,"Courier"
        );

    int pos_y = 0;

    output_image_header(pdf_dc, &pos_y);
    pos_y += 2 * vert_skip;

    output_document_header(pdf_dc, html_parser, &pos_y);
    pos_y += 2 * vert_skip;

    wx_table_generator table_gen
        (pdf_dc
        ,horz_margin
        ,page_.width_
        );

    for(int col = 0; col < e_col_max; ++col)
        {
        column_definition const& cd = column_definitions[col];
        std::string header(cd.header_);

        // The cast is only used to ensure that if any new elements are added
        // to the enum, the compiler would warn about their values not being
        // present in this switch.
        switch(static_cast<enum_group_quote_columns>(col))
            {
            case e_col_number:
            case e_col_name:
            case e_col_age:
            case e_col_dob:
            case e_col_basic_face_amount:
            case e_col_supplemental_face_amount:
            case e_col_total_face_amount:
                // Nothing to do for these columns: their labels are literal.
                break;
            case e_col_basic_premium:
            case e_col_additional_premium:
            case e_col_total_premium:
                {
                // Labels of these columns are format strings as they need to
                // be constructed dynamically.
                LMI_ASSERT(header.find("%s") != std::string::npos);
                header = wxString::Format
                    (wxString(header), report_data_.premium_mode_
                    ).ToStdString();
                }
                break;
            case e_col_max:
                {
                fatal_error() << "Unreachable." << LMI_FLUSH;
                }
                break;
            default:
                {
                fatal_error() << "Case " << col << " not found." << LMI_FLUSH;
                }
            }

        table_gen.add_column(header.c_str(), cd.widest_text_);
        }

    output_table_totals(pdf_dc, table_gen, &pos_y);

    int const y_before_header = pos_y;
    table_gen.output_header(&pos_y);
    int const header_height = pos_y - y_before_header;

    int y_after_footer = pos_y;
    output_footer(pdf_dc, html_parser, &y_after_footer, e_output_measure_only);
    int const footer_height = y_after_footer - pos_y;

    int const last_row_y = page_.total_size_.y - vert_margin;
    int remaining_space = last_row_y - pos_y;

    int total_pages = compute_pages_for_table_rows
        (&remaining_space
        ,header_height
        ,table_gen.row_height()
        ,last_row_y
        );

    // Check if the footer fits into the same page or if it needs a new one (we
    // never want to have a page break in the footer).
    bool const footer_on_its_own_page
        = remaining_space < (footer_height + 2 * vert_skip);
    if(footer_on_its_own_page)
        {
        total_pages++;
        }

    int current_page = 1;

    typedef std::vector<row_data>::const_iterator rdci;
    for(rdci i = rows_.begin(); i != rows_.end(); ++i)
        {
        table_gen.output_row(&pos_y, i->values);

        if(pos_y >= last_row_y)
            {
            output_page_number_and_version(pdf_dc, total_pages, current_page);

            current_page++;
            pdf_dc.StartPage();

            pos_y = vert_margin;
            table_gen.output_header(&pos_y);
            }
        }

    if(footer_on_its_own_page)
        {
        output_page_number_and_version(pdf_dc, total_pages, current_page);

        current_page++;
        pdf_dc.StartPage();

        pos_y = vert_margin;
        }
    else
        {
        pos_y += 2 * vert_skip;
        }

    output_footer(pdf_dc, html_parser, &pos_y);

    LMI_ASSERT(current_page == total_pages);
    output_page_number_and_version(pdf_dc, total_pages, current_page);
}

int group_quote_pdf_generator_wx::compute_pages_for_table_rows
    (int* remaining_space
    ,int header_height
    ,int row_height
    ,int last_row_y
    )
{
    int total_pages = 1;

    int const max_rows_on_first_page = (*remaining_space) / row_height;
    int remaining_rows = static_cast<int>(rows_.size());
    if(max_rows_on_first_page < remaining_rows)
        {
        // All rows don't fit on the first page, so add enough pages for the
        // rest of them.
        remaining_rows -= max_rows_on_first_page;

        int const page_area_y = last_row_y - vert_margin - header_height;
        int const rows_per_page = page_area_y / row_height;
        total_pages += (remaining_rows + rows_per_page - 1) / rows_per_page;
        *remaining_space = page_area_y;
        remaining_rows %= rows_per_page;
        }

    *remaining_space -= remaining_rows * row_height;

    return total_pages;
}

void group_quote_pdf_generator_wx::output_page_number_and_version
    (wxPdfDC& pdf_dc
    ,int total_pages
    ,int current_page
    )
{
    wxRect const footer_area
        (horz_margin
        ,page_.total_size_.y - vert_margin
        ,page_.width_
        ,vert_margin
        );

    pdf_dc.DrawLabel
        (wxString::Format("System version: %s", LMI_VERSION)
        ,footer_area
        ,wxALIGN_LEFT | wxALIGN_BOTTOM
        );

    pdf_dc.DrawLabel
        (wxString::Format("Page %d of %d", current_page, total_pages)
        ,footer_area
        ,wxALIGN_RIGHT | wxALIGN_BOTTOM
        );
}

void group_quote_pdf_generator_wx::output_image_header
    (wxPdfDC& pdf_dc
    ,int* pos_y
    )
{
    wxImage banner_image(load_image("group_quote_banner.png"));
    if(!banner_image.IsOk())
        {
        return;
        }

    // Set the scale to fit the image to the document width.
    double const
        scale = static_cast<double>(banner_image.GetWidth()) / page_.total_size_.x;
    int const pos_top = *pos_y;
    output_image(pdf_dc, banner_image, "banner", scale, 0, pos_y);

    wxDCFontChanger set_bigger_font(pdf_dc, pdf_dc.GetFont().Scaled(1.5));
    wxDCTextColourChanger set_white_text(pdf_dc, *wxWHITE);

    // Don't use escape_for_html_elem() here: instead, call
    // wxString::FromUTF8() directly, e.g., to preserve literal '&'.
    wxString const image_text
        (wxString::FromUTF8(report_data_.short_product_.c_str())
         + "\nPremium & Benefit Summary"
        );

    pdf_dc.DrawLabel
        (image_text
        ,wxRect
            (wxPoint(horz_margin, (pos_top + *pos_y) / 2),
             pdf_dc.GetMultiLineTextExtent(image_text)
            )
        ,wxALIGN_CENTER_HORIZONTAL
        );
}

void group_quote_pdf_generator_wx::output_document_header
    (wxPdfDC& pdf_dc
    ,wxHtmlWinParser& html_parser
    ,int* pos_y
    )
{
    wxString const title_html = wxString::Format
        ("<table width=\"100%%\">"
         "<tr>"
         "<td align=\"center\"><i><font size=\"+1\">%s</font></i></td>"
         "</tr>"
         "<tr>"
         "<td align=\"center\"><i>Prepared Date: %s</i></td>"
         "</tr>"
         "<tr>"
         "<td align=\"center\"><i>Prepared By: %s</i></td>"
         "</tr>"
         "</table>"
        ,escape_for_html_elem(report_data_.company_)
        ,wxDateTime::Today().FormatDate()
        ,escape_for_html_elem(report_data_.prepared_by_)
        );

    output_html(html_parser, horz_margin, *pos_y, page_.width_ / 2, title_html);

    // Build the summary table with all the mandatory fields.
    wxString summary_html =
         "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
         // This extra top empty row works around a bug in wxHTML
         // table positioning code: it uses the provided ordinate
         // coordinate as a base line of the first table line and
         // not as its top, as it ought to, so without this line
         // the rectangle drawn below wouldn't contain the header.
         "<tr>"
         "<td align=\"center\" colspan=\"4\">&nbsp;</td>"
         "</tr>"
         "<tr>"
         "<td align=\"center\" colspan=\"4\"><font size=\"+1\">Plan Details Summary</font></td>"
         "</tr>"
         ;

    {
    open_and_ensure_closing_tag tag_tr(summary_html, "tr");
    append_name_value_to_html_table
        (summary_html
        ,"Product"
        ,report_data_.product_
        );
    append_name_value_to_html_table
        (summary_html
        ,"Effective Date"
        ,report_data_.effective_date_
        );
    }
    {
    open_and_ensure_closing_tag tag_tr(summary_html, "tr");
    append_name_value_to_html_table
        (summary_html
        ,"Available Riders"
        ,report_data_.available_riders_
        );
    append_name_value_to_html_table
        (summary_html
        ,"Contract State"
        ,report_data_.contract_state_
        );
    }
    {
    open_and_ensure_closing_tag tag_tr(summary_html, "tr");
    append_name_value_to_html_table
        (summary_html
        ,"Number of Participants"
        ,wxString::Format("%d", row_num_).ToStdString()
        );
    append_name_value_to_html_table
        (summary_html
        ,"Premium Mode"
        ,report_data_.premium_mode_
        );
    }

    // Then add any additional fields in left-to-right then top-to-bottom order.
    std::vector<extra_summary_field> const& fields = report_data_.extra_fields_;

    typedef std::vector<extra_summary_field>::const_iterator esfci;
    for(esfci i = fields.begin(); i != fields.end();)
        {
        // Start a new table row and ensure it will be closed.
        open_and_ensure_closing_tag tag_tr(summary_html, "tr");

        // Add one (if there is only one) or two fields to this new row.
        for(int col = 0; col < 2; ++col)
            {
            append_name_value_to_html_table(summary_html, i->name, i->value);

            if(++i == fields.end())
                {
                break;
                }
            }
        }

    // Finally close the summary table.
    summary_html += "</table>";

    int const summary_height = output_html
        (html_parser
        ,horz_margin + page_.width_ / 2
        ,*pos_y
        ,page_.width_ / 2
        ,summary_html
        );

    // wxHTML tables don't support "frame" attribute, so draw the border around
    // the table manually.
    pdf_dc.SetBrush(*wxTRANSPARENT_BRUSH);
    pdf_dc.DrawRectangle
        (horz_margin + page_.width_ / 2
        ,*pos_y
        ,page_.width_ / 2
        ,summary_height
        );

    *pos_y += summary_height;
}

void group_quote_pdf_generator_wx::output_table_totals
    (wxPdfDC& pdf_dc
    ,wx_table_generator& table_gen
    ,int* pos_y
    )
{
    int& y = *pos_y;

    table_gen.output_horz_separator(e_first_totalled_column, e_col_max, y);
    table_gen.output_vert_separator(e_first_totalled_column, y);
    table_gen.output_vert_separator(e_col_max, y);

    y += table_gen.row_height();

    table_gen.output_vert_separator(e_col_number, y);

    int const cell_margin_x = pdf_dc.GetCharWidth();
    int const y_text = y + pdf_dc.GetCharHeight();

    // Render "Census" in bold.
    wxDCFontChanger set_bold_font(pdf_dc, pdf_dc.GetFont().Bold());
    pdf_dc.DrawLabel
        ("Census"
        ,table_gen.cell_rect(e_col_name, y_text).Deflate(cell_margin_x, 0)
        ,wxALIGN_LEFT
        );

    // And the totals in bold italic: notice that there is no need to create
    // another wxDCFontChanger here, the original font will be restored by the
    // one just above anyhow.
    pdf_dc.SetFont(pdf_dc.GetFont().Italic());

    LMI_ASSERT(0 < e_first_totalled_column);
    pdf_dc.DrawLabel
        ("Totals:"
        ,table_gen.cell_rect(e_first_totalled_column - 1, y_text).Deflate(cell_margin_x, 0)
        ,wxALIGN_RIGHT
        );

    for(int col = e_first_totalled_column; col < e_col_max; ++col)
        {
        int const num_dec =
            ((e_col_basic_face_amount           == col) ? 0
            :(e_col_basic_premium               == col) ? 2
            :(e_col_supplemental_face_amount    == col) ? 0
            :(e_col_additional_premium          == col) ? 2
            :(e_col_total_face_amount           == col) ? 0
            :(e_col_total_premium               == col) ? 2
            :throw std::logic_error("Invalid column type.")
            );
        std::pair<int, oenum_format_style> const f(num_dec, oe_format_normal);

        wxRect const cell_rect = table_gen.cell_rect(col, y);
            {
            wxDCPenChanger set_transparent_pen(pdf_dc, *wxTRANSPARENT_PEN);
            wxDCBrushChanger set_grey_brush(pdf_dc, *wxLIGHT_GREY_BRUSH);
            pdf_dc.DrawRectangle(cell_rect);
            }

        wxRect const text_rect
            (cell_rect.x + cell_margin_x
            ,y_text
            ,cell_rect.width - 2 * cell_margin_x
            ,cell_rect.height
            );

        pdf_dc.DrawLabel
            ("$"
            ,text_rect
            ,wxALIGN_LEFT
            );
        pdf_dc.DrawLabel
            (ledger_format(totals_.total(col), f)
            ,text_rect
            ,wxALIGN_RIGHT
            );

        table_gen.output_vert_separator(col, y);
        }

    table_gen.output_vert_separator(e_col_max, y);
    table_gen.output_horz_separator(e_col_number, e_col_max, y);

    y += table_gen.row_height();
}

void group_quote_pdf_generator_wx::output_footer
    (wxPdfDC& pdf_dc
    ,wxHtmlWinParser& html_parser
    ,int* pos_y
    ,enum_output_mode output_mode
    )
{
    wxImage logo_image(load_image("company_logo.png"));
    if(logo_image.IsOk())
        {
        // Arbitrarily scale down the logo by a factor of 2 to avoid making it
        // too big.
        output_image(pdf_dc, logo_image, "company_logo", 2.0, horz_margin, pos_y, output_mode);

        *pos_y += vert_skip;
        }

    wxString const footer_html = "<p>" + report_data_.footer_ + "</p>";

    *pos_y += output_html
        (html_parser
        ,horz_margin
        ,*pos_y
        ,page_.width_
        ,footer_html
        ,output_mode
        );
}

volatile bool ensure_setup = group_quote_pdf_generator_wx::set_creator
    (group_quote_pdf_generator_wx::do_create
    );

} // Unnamed namespace.

