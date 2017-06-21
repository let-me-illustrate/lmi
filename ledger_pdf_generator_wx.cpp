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
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "force_linking.hpp"
#include "global_settings.hpp"
#include "html.hpp"
#include "interpolate_string.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "pdf_writer_wx.hpp"
#include "value_cast.hpp"
#include "version.hpp"

#include <wx/pdfdc.h>

#include <map>
#include <stdexcept>

LMI_FORCE_LINKING_IN_SITU(ledger_pdf_generator_wx)

using namespace html;

namespace
{

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
    // defined using add_variable(). Its lifetime must be greater than that of
    // this object itself.
    explicit html_interpolator(LedgerInvariant const& invar)
        :invar_(invar)
    {
    }

    // A method which can be used to interpolate an HTML string containing
    // references to the variables defined for this illustration. The general
    // syntax is the same as in the global interpolate_string() function, i.e.
    // variables are anything of the form "${name}". The variable names
    // understood by this function are:
    //  - Scalar fields of GetLedgerInvariant().
    //  - Special variables defined in this class, such as "lmi_version" and
    //    "date_prepared".
    //  - Any additional fields defined in the derived classes.
    text operator()(char const* s) const
    {
        return text::from_html
            (interpolate_string
                (s
                ,[this](std::string const& s) { return expand_html(s).as_html(); }
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
        auto const z = expand_simple_html(name).as_html();
        return
              z == "1" ? true
            : z == "0" ? false
            : throw std::runtime_error
                ("Variable '" + name + "' has non-boolean value '" + z + "'"
                )
            ;
    }

  private:
    // Highest level variable expansion function.
    text expand_html(std::string const& s) const
    {
        // Check for the special "${var?only-if-set}" form:
        auto const pos_question = s.find('?');
        if(pos_question != std::string::npos)
            {
            return test_variable(s.substr(0, pos_question))
                    ? text::from(s.substr(pos_question + 1))
                    : text()
                    ;
            }

        return expand_simple_html(s);
    }

    // Simple expansion for just the variable name.
    text expand_simple_html(std::string const& s) const
    {
        // Check our own variables first:
        auto const it = vars_.find(s);
        if(it != vars_.end())
            {
            return it->second;
            }

        // Then look in the ledger.
        return text::from(invar_.value_str(s));
    }

    // Object used for variables expansion.
    LedgerInvariant const& invar_;

    // Variables defined for all pages of this illustration.
    std::map<std::string, text> vars_;
};

// This is just a container for the illustration-global data.
class pdf_illustration : protected html_interpolator
{
  public:
    pdf_illustration(Ledger const& ledger
                    ,fs::path const& output
                    )
        :html_interpolator(ledger.GetLedgerInvariant())
        ,writer_(output.string(), wxPORTRAIT, &html_font_sizes)
        ,dc_(writer_.dc())
        ,ledger_(ledger)
    {
        init_variables();
    }

    // Add a new page using its render() method.
    template<typename T>
    void add()
    {
        if(page_number_++)
            {
            dc_.StartPage();
            }

        T page;
        page.render(ledger_, writer_, dc_, *this);
    }

  private:
    // Initialize the variables that can be interpolated later.
    void init_variables()
    {
        std::string lmi_version;
        calendar_date prep_date;

        // Skip authentication for non-interactive regression testing.
        if(!global_settings::instance().regression_testing())
            {
            lmi_version = LMI_VERSION;
            authenticate_system();
            }
        else
            {
            // For regression tests,
            //   - use an invariant string as version
            //   - use EffDate as date prepared
            // in order to avoid gratuitous failures.
            lmi_version = "Regression testing";
            prep_date.julian_day_number
                (static_cast<int>(ledger_.GetLedgerInvariant().EffDateJdn)
                );
            }

        add_variable("lmi_version", lmi_version);
        add_variable
            ("date_prepared"
            , text::from(month_name(prep_date.month()))
            + text::nbsp()
            + text::from(value_cast<std::string>(prep_date.day()))
            + text::from(", ")
            + text::from(value_cast<std::string>(prep_date.year()))
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

    // Number of last added page.
    int page_number_{0};
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

    // Render this page contents.
    virtual void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) = 0;
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
        dc.SetPen(wxPen(*wxBLUE, 2));
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
                            (interpolate_html("${date_prepared}")
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
${InsCoShortName} Financial Group is a marketing
name for ${InsCoName} (${InsCoShortName}) and its
affiliated company and sales representatives, ${InsCoAddr}.
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

class narrative_summary_page : public page
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,html_interpolator const& interpolate_html
        ) override
    {
        auto const& invar = ledger.GetLedgerInvariant();

        text summary_html =
            tag::p[attr::align("center")]
                (text::from("NARRATIVE SUMMARY")
                )
            ;

        // Just a helper performing a common operation.
        auto const add_body_paragraph_html = [](text const& t) -> text
            {
            return tag::p(tag::font[attr::size("-1")](t));
            };

        auto const add_body_paragraph = [=](std::string const& s) -> text
            {
            return add_body_paragraph_html(interpolate_html(s));
            };

        std::string description;
        if(!interpolate_html.test_variable("SinglePremium"))
            {
            description = R"(
${PolicyMktgName} is a ${GroupExperienceRating?group}${GroupCarveout?group}
flexible premium adjustable life insurance contract.
${GroupExperienceRating?
It is a no-load policy and is intended for large case sales.
It is primarily marketed to financial institutions
to fund certain corporate liabilities.
}
It features accumulating account values, adjustable benefits,
and flexible premiums.
)";
            }
        else if(  interpolate_html.test_variable("ModifiedSinglePremium")
               || interpolate_html.test_variable("ModifiedSinglePremium0")
               )
            {
            description = R"(
${PolicyMktgName}
is a modified single premium adjustable life
insurance contract. It features accumulating
account values, adjustable benefits, and single premium.
)";
            }
        else
            {
            description = R"(
${PolicyMktgName}
is a single premium adjustable life insurance contract.
It features accumulating account values,
adjustable benefits, and single premium.
)";
            }

        summary_html += add_body_paragraph(description);

        if(!invar.IsInforce)
            {
            summary_html += add_body_paragraph
                (R"(
Coverage may be available on a Guaranteed Standard Issue basis.
All proposals are based on case characteristics and must
be approved by the ${InsCoShortName}
Home Office. For details regarding underwriting
and coverage limitations refer to your offer letter
or contact your ${InsCoShortName} representative.
)"
                );
            }

        writer.output_html
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,summary_html
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

        // Define variables specific to this illustration.
        add_variable
            ("ModifiedSinglePremium"
            ,starts_with(policy_name, "Single") && invar.GetStatePostalAbbrev() == "MA"
            );

        add_variable
            ("ModifiedSinglePremium0"
            ,starts_with(policy_name, "Modified")
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

        // Add all the pages.
        add<cover_page>();
        add<narrative_summary_page>();
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
    auto const z = ledger.ledger_type();
    switch(z)
        {
        case mce_ill_reg:
            {
            pdf_illustration_regular(ledger, output);
            }
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
}

volatile bool ensure_setup = ledger_pdf_generator::set_creator
    (ledger_pdf_generator_wx::do_create
    );

} // Unnamed namespace.
