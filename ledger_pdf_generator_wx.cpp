// Generate PDF files with ledger data using wxPdfDocument library.
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

#include "ledger_pdf_generator.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "force_linking.hpp"
#include "html.hpp"
#include "interpolate_string.hpp"
#include "istream_to_string.hpp"
#include "ledger.hpp"
#include "ledger_evaluator.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"               // lmi_tolower()
#include "pdf_writer_wx.hpp"
#include "value_cast.hpp"
#include "version.hpp"

#include <wx/pdfdc.h>

#include <cstdint>                      // SIZE_MAX
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

LMI_FORCE_LINKING_IN_SITU(ledger_pdf_generator_wx)

using namespace html;

namespace
{

// Colour used for lines and border in the generated illustrations.
const wxColour HIGHLIGHT_COL(0x00, 0x2f, 0x6c);

// This function is also provided in <boost/algorithm/string/predicate.hpp>,
// but it's arguably not worth adding dependency on this Boost library just for
// this function.
inline
bool starts_with(std::string const& s, char const* prefix)
{
    return s.compare(0, strlen(prefix), prefix) == 0;
}

// Helper class grouping functions for dealing with interpolating strings
// containing variable references.
class html_interpolator
{
  public:
    // Ctor takes the object used to interpolate the variables not explicitly
    // defined using add_variable().
    explicit html_interpolator(ledger_evaluator&& evaluator)
        :evaluator_(evaluator)
    {
    }

    // A method which can be used to interpolate an HTML string containing
    // references to the variables defined for this illustration. The general
    // syntax is the same as in the global interpolate_string() function, i.e.
    // variables are of the form "{{name}}" and section of the form
    // "{{#name}}..{{/name}}" or "{{^name}}..{{/name}}" are also allowed and
    // their contents is included in the expansion if and only if the variable
    // with the given name has value "1" for the former or "0" for the latter.
    // The variable names understood by this function are:
    //  - Scalar fields of GetLedgerInvariant().
    //  - Special variables defined in this class, such as "lmi_version" and
    //    "date_prepared".
    //  - Any additional fields defined in the derived classes.
    text operator()(char const* s) const
    {
        return text::from_html
            (interpolate_string
                (s
                ,[this]
                    (std::string const& s
                    ,interpolate_lookup_kind kind
                    ) -> std::string
                    {
                    switch(kind)
                        {
                        case interpolate_lookup_kind::variable:
                        case interpolate_lookup_kind::section:
                            return expand_html(s).as_html();

                        case interpolate_lookup_kind::partial:
                            return load_partial_from_file(s);
                        }

                    throw std::runtime_error("invalid lookup kind");
                    }
                )
            );
    }

    text operator()(std::string const& s) const
    {
        return (*this)(s.c_str());
    }

    // Add a variable, providing either its raw text or already escaped HTML
    // representation. Boolean values are converted to strings "0" or "1" as
    // expected.
    void add_variable(std::string const& name, text const& value)
    {
        vars_[name] = value;
    }

    void add_variable(std::string const& name, std::string const& value)
    {
        add_variable(name, text::from(value));
    }

    void add_variable(std::string const& name, bool value)
    {
        add_variable(name, std::string(value ? "1" : "0"));
    }

    // Test a boolean variable: the value must be "0" or "1", which is mapped
    // to false or true respectively. Anything else results in an exception.
    bool test_variable(std::string const& name) const
    {
        auto const z = expand_html(name).as_html();
        return
              z == "1" ? true
            : z == "0" ? false
            : throw std::runtime_error
                ("Variable '" + name + "' has non-boolean value '" + z + "'"
                )
            ;
    }

  protected:
    // Used by derived classes to define variables based on the existing
    // variables values.
    std::string evaluate(std::string const& name) const
    {
        return evaluator_(name);
    }

    std::string evaluate(std::string const& name, std::size_t index) const
    {
        return evaluator_(name, index);
    }

  private:
    // The expansion function used with interpolate_string().
    text expand_html(std::string const& s) const
    {
        // Check our own variables first:
        auto const it = vars_.find(s);
        if(it != vars_.end())
            {
            return it->second;
            }

        // Then look in the ledger, either as a scalar or a vector depending on
        // whether it has "[index]" part or not.
        if(!s.empty() && *s.rbegin() == ']')
            {
            auto const open_pos = s.find('[');
            if(open_pos == std::string::npos)
                {
                throw std::runtime_error
                    ("Variable '" + s + "' doesn't have the expected '['"
                    );
                }

            char* stop = nullptr;
            auto const index = std::strtoul(s.c_str() + open_pos + 1, &stop, 10);

            // Conversion must have stopped at the closing bracket character
            // and also check for overflow (notice that index == SIZE_MAX
            // doesn't, in theory, need to indicate overflow, but in practice
            // we're never going to have valid indices close to this number).
            if(stop != s.c_str() + s.length() - 1 || index >= SIZE_MAX)
                {
                throw std::runtime_error
                    ("Index of vector variable '" + s + "' is not a valid number"
                    );
                }

            // Cast below is valid because of the check for overflow above.
            return text::from
                (evaluator_
                    (s.substr(0, open_pos)
                    ,static_cast<std::size_t>(index)
                    )
                );
            }

        return text::from(evaluator_(s));
    }

    std::string load_partial_from_file(std::string const& file) const
    {
        std::ifstream ifs(file + ".mustache");
        if(!ifs)
            {
            alarum()
                << "Template file \""
                << file
                << ".mustache\" not found."
                << std::flush
                ;
            }
        std::string partial;
        istream_to_string(ifs, partial);
        return partial;
    }

    // Object used for variables expansion.
    ledger_evaluator const evaluator_;

    // Variables defined for all pages of this illustration.
    std::map<std::string, text> vars_;
};

class page
{
  public:
    page() = default;

    // Pages are not value-like objects, so prohibit copying them.
    page(page const&) = delete;
    page& operator=(page const&) = delete;

    // Pages are never used polymorphically currently, but still give them a
    // virtual dtor, if only to avoid gcc warnings about not having it.
    virtual ~page() = default;

    // Called before rendering any pages to prepare for doing this, e.g. by
    // computing the number of pages needed.
    //
    // This method must not draw anything on the wxDC, it is provided only for
    // measurement purposes.
    virtual void pre_render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        )
    {
        stifle_warning_for_unused_value(ledger);
        stifle_warning_for_unused_value(writer);
        stifle_warning_for_unused_value(dc);
        stifle_warning_for_unused_value(interpolate_html);
    }

    // Render this page contents.
    virtual void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) = 0;
};

// This is just a container for the illustration-global data.
class pdf_illustration : protected html_interpolator
{
  public:
    pdf_illustration(Ledger const& ledger
                    ,fs::path const& output
                    )
        :html_interpolator(ledger.make_evaluator())
        ,writer_(output.string(), wxPORTRAIT, &html_font_sizes)
        ,dc_(writer_.dc())
        ,ledger_(ledger)
    {
        init_variables();
    }

    // Add a page.
    template<typename T>
    void add()
    {
        pages_.emplace_back(new T());
    }

    // Render all pages.
    void render_all()
    {
        for(auto const& page : pages_)
            {
            page->pre_render(ledger_, writer_, dc_, *this);
            }

        bool first = true;
        for(auto const& page : pages_)
            {
            if(first)
                {
                first = false;
                }
            else
                {
                dc_.StartPage();
                }

            page->render(ledger_, writer_, dc_, *this);
            }
    }

  private:
    // Initialize the variables that can be interpolated later.
    void init_variables()
    {
        add_variable
            ("date_prepared"
            , text::from(evaluate("PrepMonth"))
            + text::nbsp()
            + text::from(evaluate("PrepDay"))
            + text::from(", ")
            + text::from(evaluate("PrepYear"))
            );

        auto indent = text::nbsp();
        add_variable("Space1", indent);

        indent += indent;
        add_variable("Space2", indent);

        indent += indent;
        add_variable("Space4", indent);

        indent += indent;
        add_variable("Space8", indent);

        indent += indent;
        add_variable("Space16", indent);

        indent += indent;
        add_variable("Space32", indent);

        indent += indent;
        add_variable("Space64", indent);

        auto const abbreviate_if_necessary = [](std::string s, size_t len)
            {
            if(s.length() > len)
                {
                s.replace(len - 3, std::string::npos, "...");
                }
            return s;
            };

        add_variable
            ("CorpNameAbbrev50"
            ,abbreviate_if_necessary(evaluate("CorpName"), 50)
            );

        add_variable
            ("Insured1Abbrev50"
            ,abbreviate_if_necessary(evaluate("Insured1"), 50)
            );
    }

    // Use non-default font sizes to make it simpler to replicate the existing
    // illustrations.
    static std::array<int, 7> const html_font_sizes;

    // Writer object used for the page metrics and higher level functions.
    pdf_writer_wx writer_;

    // The DC associated with the writer, it could be accessed via it, but as
    // it's needed often, provide a shorter alias for it.
    wxDC& dc_;

    // Source of the data.
    Ledger const& ledger_;

    // All the pages of this illustration.
    std::vector<std::unique_ptr<page>> pages_;
};

std::array<int, 7> const pdf_illustration::html_font_sizes =
    { 8
    , 9
    ,10
    ,12
    ,14
    ,18
    ,20
    };

class cover_page : public page
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        dc.SetPen(wxPen(HIGHLIGHT_COL, 2));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);

        auto const frame_horz_margin = writer.get_horz_margin();
        auto const frame_vert_margin = writer.get_vert_margin();
        auto const frame_width       = writer.get_page_width();
        auto const frame_height      = writer.get_page_height();

        dc.DrawRectangle
            (frame_horz_margin
            ,frame_vert_margin
            ,frame_width
            ,frame_height
            );

        auto const& invar = ledger.GetLedgerInvariant();

        // We use empty table cells to insert spaces into the table below.
        auto const space = tag::tr(tag::td(text::nbsp()));

        auto const cover_html =
            tag::table[attr::width("100%")]
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+4")]
                            (tag::b(text::from(invar.PolicyMktgName))
                            )
                        )
                    )
                )
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+4")]
                            (tag::b
                                (text::from
                                    (invar.IsInforce
                                        ? "In Force Life Insurance Illustration"
                                        : "Life Insurance Illustration"
                                    )
                                )
                            )
                        )
                    )
                )
                (space)
                (space)
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+2")]
                            (tag::b
                                (text::from("Prepared for:")
                                )
                            )
                        )
                    )
                )
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+2")]
                            (text::from
                                (ledger.is_composite()
                                    ? invar.CorpName
                                    : invar.Insured1
                                )
                            )
                        )
                    )
                )
                (space)
                (space)
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+2")]
                            (tag::b
                                (text::from("Presented by:")
                                )
                            )
                        )
                    )
                )
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+2")]
                            (text::from(invar.ProducerName)
                            )
                        )
                    )
                )
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+2")]
                            (text::from(invar.ProducerStreet)
                            )
                        )
                    )
                )
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+2")]
                            (text::from(invar.ProducerCity)
                            )
                        )
                    )
                )
                (space)
                (tag::tr
                    (tag::td[attr::align("center")]
                        (tag::font[attr::size("+2")]
                            (interpolate_html("{{date_prepared}}")
                            )
                        )
                    )
                )
                ;

        auto const text_horz_margin = 2*frame_horz_margin;
        auto const text_width       = frame_width - 2*frame_horz_margin;
        writer.output_html
            (text_horz_margin
            ,4*frame_vert_margin
            ,text_width
            ,cover_html
            );

        auto const footer_html = tag::p[attr::align("center")]
            (tag::font[attr::size("-1")]
                (interpolate_html
                    (R"(
{{InsCoShortName}} Financial Group is a marketing
name for {{InsCoName}} ({{InsCoShortName}}) and its
affiliated company and sales representatives, {{InsCoAddr}}.
)"
                    )
                )
            );

        // Compute the footer height (which depends on how long it is, as it
        // can be wrapped to take more than one line)...
        int const footer_height = writer.output_html
            (text_horz_margin
            ,0
            ,text_width
            ,footer_html
            ,e_output_measure_only
            );

        // ... in order to be able to position it precisely at the bottom of
        // our blue frame.
        writer.output_html
            (text_horz_margin
            ,frame_vert_margin + frame_height - footer_height
            ,text_width
            ,footer_html
            );
    }
};

/// Base class for all with a footer.
class page_with_footer : public page
{
  public:
    // Override pre_render() to compute footer_top_ which is needed in the
    // derived classes overridden get_extra_pages_needed().
    void pre_render
        (Ledger const& /* ledger */
        ,pdf_writer_wx& writer
        ,wxDC& /* dc */
        ,html_interpolator const& interpolate_html
        ) override
    {
        // Note that we implicitly assume here that get_footer_html() result
        // doesn't materially depend on the exact value of last_page_number_ as
        // we don't know its definitive value here yet. In theory, this doesn't
        // need to be true, e.g. we may later discover that 10 pages are needed
        // instead of 9 and the extra digit might result in a line wrapping on
        // a new line and this increasing the footer height, but in practice
        // this doesn't risk happening and taking into account this possibility
        // wouldn't be simple at all, so just ignore this possibility.
        auto const footer_height = writer.output_html
            (writer.get_horz_margin()
            ,0
            ,writer.get_page_width()
            ,get_footer_html(interpolate_html)
            ,e_output_measure_only
            );

        footer_top_ = writer.get_page_bottom() - footer_height;
    }

    void render
        (Ledger const& /* ledger */
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        auto const frame_horz_margin = writer.get_horz_margin();
        auto const frame_width       = writer.get_page_width();

        writer.output_html
            (frame_horz_margin
            ,footer_top_
            ,frame_width
            ,get_footer_html(interpolate_html)
            );

        dc.SetPen(HIGHLIGHT_COL);
        dc.DrawLine
            (frame_horz_margin
            ,footer_top_
            ,frame_width + frame_horz_margin
            ,footer_top_
            );
    }

  protected:
    // Helper for the derived pages to get the vertical position of the footer.
    // Notice that it can only be used after calling our pre_render() method
    // as this is where it is computed.
    int get_footer_top() const
    {
        LMI_ASSERT(footer_top_ != 0);

        return footer_top_;
    }

  private:
    // Method to be overridden in the base class which should actually return
    // the contents of the (middle part of the) footer.
    virtual std::string get_footer_contents() const = 0;

    // This method uses get_footer_contents() and returns the HTML wrapping it
    // and other fixed information appearing in the footer.
    html::text get_footer_html(html_interpolator const& interpolate_html) const
    {
        return
            tag::font[attr::size("-2")]
                (tag::table[attr::width("100%")]
                          [attr::cellspacing("0")]
                          [attr::cellpadding("0")]
                    (tag::tr
                        (tag::td[attr::colspan("3")]
                            (text::nbsp())
                        )
                    )
                    (tag::tr
                        (tag::td
                            (interpolate_html
                                ("Date Prepared: {{date_prepared}}"
                                )
                            )
                        )
                        (tag::td[attr::align("center")]
                            (text::from(get_footer_contents())
                            )
                        )
                        (tag::td[attr::align("right")]
                            (interpolate_html
                                ("{{InsCoName}}"
                                )
                            )
                        )
                    )
                    (tag::tr
                        (tag::td
                            (interpolate_html
                                ("System Version: {{LmiVersion}}"
                                )
                            )
                        )
                        (tag::td
                            (text::nbsp()
                            )
                        )
                        (tag::td[attr::align("right")]
                            (interpolate_html
                                (R"(
    {{#IsInforce}}
        {{#Composite}}
            {{ImprimaturInforceComposite}}
        {{/Composite}}
        {{^Composite}}
            {{ImprimaturInforce}}
        {{/Composite}}
    {{/IsInforce}}
    {{^IsInforce}}
        {{#Composite}}
            {{ImprimaturPresaleComposite}}
        {{/Composite}}
        {{^Composite}}
            {{ImprimaturPresale}}
        {{/Composite}}
    {{/IsInforce}}
    )"
                                )
                            )
                        )
                    )
                );
    }

    int footer_top_ = 0;
};

/// Base class for all pages showing the page number in the footer.
///
/// In addition to actually providing page_with_footer with the correct string
/// to show in the footer, this class implicitly handles the page count by
/// incrementing it whenever a new object of this class is created.
class numbered_page : public page_with_footer
{
  public:
    numbered_page()
        :this_page_number_(++last_page_number_)
    {
    }

    void pre_render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        page_with_footer::pre_render(ledger, writer, dc, interpolate_html);

        extra_pages_ = get_extra_pages_needed
            (ledger
            ,writer
            ,dc
            ,interpolate_html
            );

        LMI_ASSERT(extra_pages_ >= 0);

        last_page_number_ += extra_pages_;
    }

    ~numbered_page() override
    {
        // Check that next_page() was called the expected number of times.
        // Unfortunately we can't use LMI_ASSERT() in the (noexcept) dtor, so
        // use warning() instead.
        if(extra_pages_)
            {
            warning()
                << "Logic error: "
                << extra_pages_
                << " missing extra pages."
                << LMI_FLUSH
                ;
            }
    }

  protected:
    void next_page(wxDC& dc)
    {
        // This method may only be called if we had reserved enough physical
        // pages for this logical pages by overriding get_extra_pages_needed().
        LMI_ASSERT(extra_pages_ > 0);

        dc.StartPage();

        this_page_number_++;
        extra_pages_--;
    }

  private:
    // Derived classes may override this method if they may need more than one
    // physical page to show their contents.
    virtual int get_extra_pages_needed
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) const
    {
        stifle_warning_for_unused_value(ledger);
        stifle_warning_for_unused_value(writer);
        stifle_warning_for_unused_value(dc);
        stifle_warning_for_unused_value(interpolate_html);

        return 0;
    }

    std::string get_footer_contents() const override
    {
        std::ostringstream oss;
        oss << "Page " << this_page_number_ << " of " << last_page_number_;
        return oss.str();
    }

    static int last_page_number_;
    int        this_page_number_;
    int        extra_pages_          = 0;
};

int numbered_page::last_page_number_ = 0;

// Wrap the given HTML in a paragraph tag using (smaller) body font size.
text add_body_paragraph_html(text const& t)
{
    return tag::p(tag::font[attr::size("-1")](t));
}

// Return HTML with the standard page header.
text get_header_html
    (LedgerInvariant const& invar
    ,html_interpolator const& interpolate_html
    )
{
    // Some convenient helpers for performing common operations.
    auto const add_line = [=](char const* s) -> text
        {
        return tag::br + interpolate_html(s);
        };

    text header_html = interpolate_html
        (R"(
{{#IsInforce}}
LIFE INSURANCE IN FORCE BASIC ILLUSTRATION
{{/IsInforce}}
{{^IsInforce}}
LIFE INSURANCE BASIC ILLUSTRATION
{{/IsInforce}}
)"
        );

    header_html += add_line("{{InsCoName}}");

    if(invar.ProducerName != "0")
        {
        header_html += add_line("Presented by: {{ProducerName}}");
        }

    if(invar.ProducerStreet != "0")
        {
        header_html += add_line("{{ProducerStreet}}");
        }

    if(invar.ProducerCity != "0")
        {
        header_html += add_line("{{ProducerCity}}");
        }

    // Construct the left-hand side of the summary table.
    text lhs_html = text::from("Prepared for:");

    lhs_html += add_line("{{Space8}}Group Name:{{CorpNameAbbrev50}}");

    lhs_html += add_line
        (R"(
{{#Composite}}
Composite Illustration
{{/Composite}}
{{^Composite}}
{{Space8}}Insured:{{Insured1Abbrev50}}
{{/Composite}}
)"
        );

    if(!interpolate_html.test_variable("Composite"))
        {
        lhs_html += add_line("{{Space8}}Age: {{Age}}");
        }

    lhs_html += add_line
        ("Product: {{PolicyForm}}{{Space1}}{{PolicyMktgName}}"
        );

    lhs_html += add_line
        (R"(
{{#ModifiedSinglePremium}}
Modified Single Premium Adjustable Life Insurance Policy
{{/ModifiedSinglePremium}}
{{^ModifiedSinglePremium}}
{{PolicyLegalName}}
{{/ModifiedSinglePremium}}
)"
        );

    if(!interpolate_html.test_variable("IsInforce"))
        {
        lhs_html += add_line
            (R"(
{{^SinglePremium}}
Initial Premium:
{{/SinglePremium}}
{{#SinglePremium}}
Single Premium:
{{/SinglePremium}}
{{Space1}}${{InitPrem}}
)"
            );
        }

    if(!interpolate_html.test_variable("Composite"))
        {
        lhs_html += add_line
            ("Initial Death Benefit Option: {{InitDBOpt}}"
            );
        }

    // Now the right-hand side.
    auto rhs_html = interpolate_html
        (R"(
Initial {{#HasTerm}}Total{{/HasTerm}}
Selected Face Amount: ${{InitTotalSA}}
)"
        );

    if(interpolate_html.test_variable("HasTerm"))
        {
        rhs_html += add_line
            ("Initial Base Face Amount: ${{InitBaseSpecAmt}}"
            );

        rhs_html += add_line
            ("Initial Term Face Amount: ${{InitTermSpecAmt}}"
            );
        }

    rhs_html += add_line
        ("Guaranteed Crediting Rate: {{InitAnnGenAcctInt_Guaranteed}}"
        );

    rhs_html += add_line
        (R"(
Current Illustrated Crediting Rate:
{{#InforceYear}}
{{UltimateInterestRate}}
{{/InforceYear}}
{{^InforceYear}}
{{InitAnnGenAcctInt_Current}}
{{/InforceYear}}
)"
        );

    if
        (   interpolate_html.test_variable("SinglePremium")
        &&  invar.InforceYear <= 4
        )
        {
        rhs_html += add_line
            (R"(
Ultimate Illustrated Crediting Rate:
{{#ModifiedSinglePremium0}}
{{AnnGAIntRate_Current[11]}}
{{/ModifiedSinglePremium0}}
{{^ModifiedSinglePremium0}}
{{AnnGAIntRate_Current[6]}}
{{/ModifiedSinglePremium0}}
)"
            );
        }

    if(!interpolate_html.test_variable("Composite"))
        {
        rhs_html += add_line
            (R"(
Underwriting Type:
{{#UWTypeIsMedical}}
Fully underwritten
{{/UWTypeIsMedical}}
{{^UWTypeIsMedical}}
{{UWType}}
{{/UWTypeIsMedical}}
)"
            );

        rhs_html += add_line
            (R"(
Rate Classification: {{UWClass}}, {{Smoker}}, {{Gender}}
)"
            );

        if(invar.UWClass == "Rated")
            {
            rhs_html += add_line
                ("{{Space2}}{{Space1}}Table Rating: {{SubstandardTable}}"
                );
            }
        }

    // Put everything together.
    return
        tag::font[attr::size("-1")]
            (tag::p[attr::align("center")]
                (header_html
                )
            )
            (tag::p
                (text::nbsp()
                )
            )
            (tag::table
                [attr::width("100%")]
                [attr::cellspacing("0")]
                [attr::cellpadding("0")]
                [attr::valign("top")]
                (tag::tr
                    (tag::td[attr::width("60%")](lhs_html))
                    (tag::td[attr::width("40%")](rhs_html))
                )
            )
        ;
}

class narrative_summary_page : public numbered_page
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        numbered_page::render(ledger, writer, dc, interpolate_html);

        text summary_html = get_header_html
            (ledger.GetLedgerInvariant()
            ,interpolate_html
            );

        summary_html += tag::p[attr::align("center")]
            (text::from("NARRATIVE SUMMARY")
            );

        // Just a convenient helper performing a common operation.
        auto const add_body_paragraph = [=](std::string const& s) -> text
            {
            return add_body_paragraph_html(interpolate_html(s));
            };

        std::string description;
        if(!interpolate_html.test_variable("SinglePremium"))
            {
            description = R"(
{{PolicyMktgName}} is a
{{#GroupExperienceRating}}group{{/GroupExperienceRating}}
{{#GroupCarveout}}group{{/GroupCarveout}}
flexible premium adjustable life insurance contract.
{{#GroupExperienceRating}}
It is a no-load policy and is intended for large case sales.
It is primarily marketed to financial institutions
to fund certain corporate liabilities.
{{/GroupExperienceRating}}
It features accumulating account values, adjustable benefits,
and flexible premiums.
)";
            }
        else if(  interpolate_html.test_variable("ModifiedSinglePremium")
               || interpolate_html.test_variable("ModifiedSinglePremium0")
               )
            {
            description = R"(
{{PolicyMktgName}}
is a modified single premium adjustable life
insurance contract. It features accumulating
account values, adjustable benefits, and single premium.
)";
            }
        else
            {
            description = R"(
{{PolicyMktgName}}
is a single premium adjustable life insurance contract.
It features accumulating account values,
adjustable benefits, and single premium.
)";
            }

        summary_html += add_body_paragraph(description);

        if(!interpolate_html.test_variable("IsInforce"))
            {
            summary_html += add_body_paragraph
                (R"(
Coverage may be available on a Guaranteed Standard Issue basis.
All proposals are based on case characteristics and must
be approved by the {{InsCoShortName}}
Home Office. For details regarding underwriting
and coverage limitations refer to your offer letter
or contact your {{InsCoShortName}} representative.
)"
                );
            }

        summary_html += add_body_paragraph_html
            ( interpolate_html("{{AvName}}")
            + text::nbsp()
            + interpolate_html("{{MonthlyChargesPaymentFootnote}}")
            );

        std::string premiums;
        if(!interpolate_html.test_variable("SinglePremium"))
            {
            premiums = R"(
Premiums are assumed to be paid on {{ErModeLCWithArticle}}
basis and received at the beginning of the contract year.
)";
            }
        else
            {
            premiums = R"(
The single premium is assumed to be paid at the beginning
of the contract year.
)";
            }

        premiums += R"(
{{AvName}} Values, {{CsvName}} Values,
and death benefits are illustrated as of the end
of the contract year. The method we use to allocate
overhead expenses is the fully allocated expense method.
)";

        summary_html += add_body_paragraph(premiums);

        if(!interpolate_html.test_variable("SinglePremium"))
            {
            summary_html += add_body_paragraph
                (R"(
In order to guarantee coverage to age
{{EndtAge}}, {{ErModeLCWithArticle}} premium
{{#HasGuarPrem}}
of ${{GuarPrem}} must be paid.
{{/HasGuarPrem}}
{{^HasGuarPrem}}
is defined.
{{/HasGuarPrem}}
This amount is based on the guaranteed monthly charges
and the guaranteed interest crediting rate.
{{#DefnLifeInsIsGPT}}
This premium may be in excess of the maximum premium allowable
in order to qualify this policy as life insurance.
{{/DefnLifeInsIsGPT}}
)"
                );
            }

        summary_html += add_body_paragraph
            (R"(
Loaned amounts of the {{AvName}}
Value will be credited a rate equal to the loan interest rate less
a spread, guaranteed not to exceed
{{#GroupCarveout}}
1.25%.
{{/GroupCarveout}}
{{^GroupCarveout}}
3.00%.
{{/GroupCarveout}}
)"
            );

        if(interpolate_html.test_variable("HasTerm"))
            {
            summary_html += add_body_paragraph
                (R"(
The term rider provides the option to purchase monthly
term insurance on the life of the insured. The term rider
selected face amount supplements the selected face amount
of the contract. If the term rider is attached, the policy
to which it is attached may have a lower annual cutoff premium
and, as a result, the lower overall sales loads paid may be
lower than a contract having the same total face amount,
but with no term rider.
{{#NoLapse}}
  Also, the lapse protection feature of the contract's
  {{NoLapseProvisionName}}
  does not apply to the term rider's selected face amount.
{{/NoLapse}}
)"
                );
            }

        if(interpolate_html.test_variable("HasWP"))
            {
            summary_html += add_body_paragraph
                (R"(
The Waiver of Monthly Charges Rider provides for waiver
of monthly charges in the event of the disability
of the insured that begins before attained age 65
and continues for at least 6 months, as described in the rider.
An additional charge is associated with this rider. Please refer
to your contract for specific provisions and a detailed schedule
of charges.
)"
                );
            }

        if(interpolate_html.test_variable("HasWP"))
            {
            summary_html += add_body_paragraph
                (R"(
The Accidental Death benefit provides an additional benefit
if death is due to accident. An additional charge is associated
with this rider. Please refer to your contract
for specific provisions and a detailed schedule of charges.
)"
                );
            }

        summary_html += add_body_paragraph
            (R"(
The definition of life insurance for this contract is the
{{#DefnLifeInsIsGPT}}
    guideline premium test. The guideline single premium
    is ${{InitGSP}}
    and the guideline level premium
    is ${{InitGLP}}
{{/DefnLifeInsIsGPT}}
{{^DefnLifeInsIsGPT}}
    cash value accumulation test.
{{/DefnLifeInsIsGPT}}
)"
            );

        summary_html += add_body_paragraph
            (R"(
This is an illustration only. An illustration is not intended
to predict actual performance. Interest rates
and values set forth in the illustration are not guaranteed.
)"
            );

        summary_html += add_body_paragraph
            (R"(
{{^StateIsTexas}}
This illustration assumes that the currently illustrated
non-guaranteed elements will continue unchanged
for all years shown. This is not likely to occur
and actual results may be more or less favorable than shown.
The non-guaranteed benefits and values are not guaranteed
and are based on assumptions such as interest credited
and current monthly charges, which are subject to change by
{{InsCoName}}.
{{/StateIsTexas}}
{{#StateIsTexas}}
This illustration is based on both non-guaranteed
and guaranteed assumptions. Non-guaranteed assumptions
include interest rates and monthly charges.
This illustration assumes that the currently illustrated
non-guaranteed elements will continue unchanged
for all years shown. This is not likely to occur
and actual results may be more or less favorable than shown.
Factors that may affect future policy performance include
the company's expectations for future mortality, investments,
persistency, profits and expenses.
{{/StateIsTexas}}
)"
            );

        writer.output_html
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,summary_html
            );
    }
};

class narrative_summary_cont_page : public numbered_page
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        numbered_page::render(ledger, writer, dc, interpolate_html);

        // Just a convenient helper performing a common operation.
        auto const add_body_paragraph = [=](std::string const& s) -> text
            {
            return add_body_paragraph_html(interpolate_html(s));
            };

        text summary_html = tag::p[attr::align("center")]
            (text::from("NARRATIVE SUMMARY (Continued)")
            );

        summary_html += tag::br;

        if(interpolate_html.test_variable("HasSalesLoadRefund"))
            {
            summary_html += add_body_paragraph
                (R"(
Sales Load Refund: We will refund a portion of the sales load
to you, as part of your {{CsvName}}
Value, if you surrender your contract within the first two
policy years. In policy year 1, we will refund
{{SalesLoadRefundRate0}}
of the first contract year sales load collected
and in contract year 2, we will refund
{{SalesLoadRefundRate1}}
of the first contract year sales load collected.
)"
                );
            }

        if(interpolate_html.test_variable("NoLapse"))
            {
            summary_html += add_body_paragraph
                (R"(
{{NoLapseProvisionName}}:
The contract will remain in force after the first premium
has been paid, even if there is insufficient
{{AvName}} Value
to cover the monthly charges provided that the insured
is not in a substandard rating class and the policy debt
does not exceed {{AvName}} Value.
)"
                );
            }

        summary_html += add_body_paragraph("{{GuarMortalityFootnote}}");

        summary_html += add_body_paragraph
            (R"(
This illustration assumes death of the insured at age {{EndtAge}}.
)"
            );

        summary_html += add_body_paragraph
            (R"(
 The loan interest rate is fixed at {{InitAnnLoanDueRate}} per year.
)"
            );

        summary_html += add_body_paragraph
            (R"(
The state of issue is {{StatePostalAbbrev}}.
)"
            );

        summary_html += add_body_paragraph
            (R"(
This illustration assumes an initial Selected Face Amount of
${{InitBaseSpecAmt}}. Selected Face
Amount reductions assumed in this illustration (if any) are shown
in the Tabular Detail.
)"
            );

        if(interpolate_html.test_variable("IsInforce"))
            {
            summary_html += add_body_paragraph
                (R"(
This illustration assumes a beginning account value of
${{InforceUnloanedAV}} as of the date of this illustration.
)"
                );

            summary_html += add_body_paragraph
                (R"(
This illustration assumes a beginning cost basis of
${{InforceTaxBasis}} as
of the date of this illustration; the actual cost basis
may be higher or lower. Consult the Home Office for cost
basis information.
)"
                );
            }

        if(interpolate_html.test_variable("Composite"))
            {
            summary_html += add_body_paragraph
                (R"(
Please see the attached census, listing the face amounts,
underwriting classes and issue ages for individual participants.
)"
                );
            }

        if(interpolate_html.test_variable("StateIsCarolina"))
            {
            summary_html += add_body_paragraph
                (R"(
In the states of North Carolina and South Carolina,
Guaranteed Issue Underwriting is referred
to as "Limited Underwriting" and Simplified
Issue Underwriting is referred to as "Simplified Underwriting".
)"
                );
            }

        if(interpolate_html.test_variable("StateIsMaryland"))
            {
            summary_html += add_body_paragraph
                (R"(
In the state of Maryland, Guaranteed Issue Underwriting
is referred to as "Nonstandard Limited Underwriting"
and Simplified Issue Underwriting is referred to as
"Nonstandard Simplified Underwriting".
)"
                );
            }

        if(interpolate_html.test_variable("GroupExperienceRating"))
            {
            summary_html += add_body_paragraph
                (R"(
We may assess a Market Value Adjustment upon a surrender
of the certificate when the surrender proceeds are intended
to be applied to an insurance policy issued by an insurer
unaffiliated with
{{InsCoShortName}}
with an intent to qualify the exchange as a tax free exchange
under IRC section 1035.
)"
                );

            if(!interpolate_html.test_variable("UseExperienceRating"))
                {
                summary_html += add_body_paragraph
                    ("This illustration does not reflect experience rating."
                    );
                }

            summary_html += add_body_paragraph
                (R"(
The guaranteed values reflect the maximum charges permitted
by the contract, which may include an Experience Rating
Risk Charge.
)"
                );

            summary_html += add_body_paragraph
                (R"(
No Experience Rating Risk Charge or a distribution
of an Experience Rating Reserve Credit is reflected
in the current, non-guaranteed values. Actual charges
and credits will be based on the actual experience of the group.
)"
                );
            }

        if(interpolate_html.test_variable("Has1035ExchCharge"))
            {
            summary_html += add_body_paragraph
                (R"(
{{^SinglePremium}}
    Upon surrender of this policy, where the surrender
    proceeds are intended to be applied to an insurance policy
    or certificate issued in conjunction with an intent
    to qualify the exchange as a tax free exchange
    under Section 1035 of the Internal Revenue Code,
    we may assess an Exchange Charge. The Exchange Charge
    is the greater of the Market Value Adjustment Charge
    and the Percentage of Premium Charge. In the states
    of Florida or Indiana, the Exchange charge
    (referred to as Assignment Charge in Florida)
    will be the Percentage of Premium Charge only.
    The Exchange Charge will potentially reduce
    the surrender proceeds, but will never increase
    the surrender proceeds. Please refer to your policy
    for details.
{{/SinglePremium}}
{{#SinglePremium}}
        Upon surrender of this policy, where the surrender proceeds
        are intended to be applied to an insurance policy
        or certificate issued in conjunction with an intent
        to qualify the exchange as a tax free exchange
        under Section 1035 of the Internal Revenue Code (1035
        Exchange), we may assess an Exchange Charge. The Exchange
        Charge will potentially reduce the surrender proceeds,
        but will never increase the surrender proceeds.
        Please refer to your policy for details.
{{/SinglePremium}}
)"
                );
            }

        if(interpolate_html.test_variable("HasSpouseRider"))
            {
            summary_html += add_body_paragraph
                (R"(
The ${{SpouseRiderAmount}} Spouse
rider provides term life insurance on the spouse
(issue age {{SpouseIssueAge}})
for a limited duration, for an extra charge.
Please refer to your contract for specific provisions
and a detailed schedule of charges.
)"
                );
            }

        if(interpolate_html.test_variable("HasChildRider"))
            {
            summary_html += add_body_paragraph
                (R"(
The ${{ChildRiderAmount}} Child
rider provides term life insurance on the insured's children
for a limited duration, for an extra charge. Please refer
to your contract for specific provisions
and a detailed schedule of charges.
)"
                );
            }

        summary_html += add_body_paragraph("{{SurrenderFootnote}}");

        summary_html += add_body_paragraph("{{FundRateFootnote}}");

        summary_html += add_body_paragraph("{{Space2}}{{FundRateFootnote0}}");

        summary_html += add_body_paragraph("{{Space2}}{{FundRateFootnote1}}");

        summary_html += tag::p[attr::align("center")]
            (tag::b
                (text::from("IMPORTANT TAX DISCLOSURE")
                )
            );

        summary_html += add_body_paragraph
            (R"(
As illustrated, this contract
{{#IsMec}}
  becomes
{{/IsMec}}
{{^IsMec}}
  would not become
{{/IsMec}}
a Modified Endowment Contract (MEC)
under the Internal Revenue Code
{{#IsMec}}
{{MecYearPlus1}}
{{/IsMec}}
To the extent of gain in the contract, loans, distributions
and withdrawals from a MEC are subject to income tax
and may also trigger a penalty tax.
)"
            );

        if
            (   !interpolate_html.test_variable("SinglePremium")
            &&  !interpolate_html.test_variable("IsInforce")
            )
            {
            summary_html += add_body_paragraph
                (R"(
The initial 7-pay premium limit is ${{InitSevenPayPrem}}.
)"
                );
            }

        summary_html += tag::p
            (tag::font[attr::size("-1")]
                (tag::b
                    (interpolate_html
                        (R"(
The information contained in this illustration is not written
or intended as tax or legal advice, and may not be relied upon
for purposes of avoiding any federal tax penalties.
Neither {{InsCoShortName}} nor any
of its employees or representatives are authorized to give tax
or legal advice. For more information pertaining
to the tax consequences of purchasing or owning this policy,
consult with your own independent tax or legal counsel.
)"
                        )
                    )
                )
            );

        writer.output_html
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,summary_html
            );
    }
};

class columns_headings_page : public numbered_page
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        numbered_page::render(ledger, writer, dc, interpolate_html);

        writer.output_html
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,interpolate_html("{{>column_headings}}")
            );
    }
};

class numeric_summary_page : public numbered_page
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        numbered_page::render(ledger, writer, dc, interpolate_html);

        writer.output_html
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,interpolate_html("{{>numeric_summary}}")
            );
    }
};

// Regular illustration.
class pdf_illustration_regular : public pdf_illustration
{
  public:
    pdf_illustration_regular(Ledger const& ledger
                            ,fs::path const& output
                            )
        :pdf_illustration(ledger, output)
    {
        auto const& invar = ledger.GetLedgerInvariant();
        auto const& policy_name = invar.PolicyLegalName;
        auto const& state_abbrev = invar.GetStatePostalAbbrev();

        // Define variables specific to this illustration.
        add_variable
            ("ModifiedSinglePremium"
            ,starts_with(policy_name, "Single") && state_abbrev == "MA"
            );

        add_variable
            ("ModifiedSinglePremium0"
            ,starts_with(policy_name, "Modified")
            );

        add_variable
            ("ModifiedSinglePremiumOrModifiedSinglePremium0"
            , test_variable("ModifiedSinglePremium")
            ||test_variable("ModifiedSinglePremium0")
            );

        add_variable
            ("SinglePremium"
            ,starts_with(policy_name, "Single") || starts_with(policy_name, "Modified")
            );

        add_variable
            ("GroupCarveout"
            ,policy_name == "Group Flexible Premium Adjustable Life Insurance Certificate"
            );

        add_variable
            ("GroupExperienceRating"
            ,policy_name == "Group Flexible Premium Adjustable Life Insurance Policy"
            );

        add_variable
            ("MecYearPlus1"
            ,invar.MecYear + 1
            );

        // Variable representing the premium payment frequency with the
        // appropriate indefinite article preceding it, e.g. "an annual" or "a
        // monthly".
        auto const er_mode = invar.ErMode[0].str();
        if(!er_mode.empty())
            {
            auto const er_mode_first = lmi_tolower(er_mode[0]);
            add_variable
                ("ErModeLCWithArticle"
                ,(strchr("aeiou", er_mode_first) ? "an" : "a") + er_mode.substr(1)
                );
            }

        add_variable
            ("HasProducerCity"
            ,invar.ProducerCity != "0"
            );

        add_variable
            ("HasGuarPrem"
            ,invar.GuarPrem != 0
            );

        add_variable
            ("HasScaleUnit"
            ,!invar.ScaleUnit().empty()
            );

        add_variable
            ("DefnLifeInsIsGPT"
            ,invar.DefnLifeIns == "GPT"
            );

        add_variable
            ("StateIsCarolina"
            ,state_abbrev == "NC" || state_abbrev == "SC"
            );

        add_variable
            ("StateIsIllinois"
            ,state_abbrev == "IL"
            );

        add_variable
            ("StateIsMaryland"
            ,state_abbrev == "MD"
            );

        add_variable
            ("StateIsTexas"
            ,state_abbrev == "TX"
            );

        add_variable
            ("StateIsIllinoisOrTexas"
            ,state_abbrev == "IL" || state_abbrev == "TX"
            );

        add_variable
            ("UWTypeIsMedical"
            ,invar.UWType == "Medical"
            );

        add_variable
            ("UltimateInterestRate"
            ,evaluate("AnnGAIntRate_Current", invar.InforceYear + 1)
            );

        auto const max_duration = invar.EndtAge - invar.Age;
        auto const lapse_year_guaruanteed = ledger.GetGuarFull().LapseYear;
        auto const lapse_year_midpoint = ledger.GetMdptFull().LapseYear;
        auto const lapse_year_current = ledger.GetCurrFull().LapseYear;

        add_variable
            ("LapseYear_Guaranteed_LT_MaxDuration"
            ,lapse_year_guaruanteed < max_duration
            );

        add_variable
            ("LapseYear_Guaranteed_Plus1"
            ,lapse_year_guaruanteed + 1
            );

        add_variable
            ("LapseYear_Midpoint_LT_MaxDuration"
            ,lapse_year_midpoint < max_duration
            );

        add_variable
            ("LapseYear_Midpoint_Plus1"
            ,lapse_year_midpoint + 1
            );

        add_variable
            ("LapseYear_Current_LT_MaxDuration"
            ,lapse_year_current < max_duration
            );

        add_variable
            ("LapseYear_Current_Plus1"
            ,lapse_year_current + 1
            );

        // Add all the pages.
        add<cover_page>();
        add<narrative_summary_page>();
        add<narrative_summary_cont_page>();
        add<columns_headings_page>();
        if(!invar.IsInforce)
            {
            add<numeric_summary_page>();
            }
    }
};

class ledger_pdf_generator_wx : public ledger_pdf_generator
{
  public:
    static std::shared_ptr<ledger_pdf_generator> do_create()
        {
        return std::make_shared<ledger_pdf_generator_wx>();
        }

    ledger_pdf_generator_wx() = default;
    ledger_pdf_generator_wx(ledger_pdf_generator_wx const&) = delete;
    ledger_pdf_generator_wx& operator=(ledger_pdf_generator_wx const&) = delete;

    void write(Ledger const& ledger, fs::path const& output) override;

  private:
};

void ledger_pdf_generator_wx::write
    (Ledger const& ledger
    ,fs::path const& output
    )
{
    std::unique_ptr<pdf_illustration> pdf_ill;

    auto const z = ledger.ledger_type();
    switch(z)
        {
        case mce_ill_reg:
            pdf_ill.reset(new pdf_illustration_regular(ledger, output));
            break;
        case mce_nasd:
        case mce_group_private_placement:
        case mce_individual_private_placement:
        case mce_variable_annuity:
            // TODO
            alarum() << "Illustrating ledger type '" << z << "' not implemented yet" << LMI_FLUSH;
            break;
        default:
            alarum() << "Unknown ledger type '" << z << "'." << LMI_FLUSH;
        }

    pdf_ill->render_all();
}

volatile bool ensure_setup = ledger_pdf_generator::set_creator
    (ledger_pdf_generator_wx::do_create
    );

} // Unnamed namespace.
