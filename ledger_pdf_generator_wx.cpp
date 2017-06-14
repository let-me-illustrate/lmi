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

#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "force_linking.hpp"
#include "global_settings.hpp"
#include "html.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "pdf_writer_wx.hpp"
#include "value_cast.hpp"
#include "version.hpp"

#include <wx/pdfdc.h>

LMI_FORCE_LINKING_IN_SITU(ledger_pdf_generator_wx)

using namespace html;

namespace
{

// This is just a container for the illustration-global data.
class pdf_illustration
{
  public:
    pdf_illustration(Ledger const& ledger
                    ,fs::path const& output
                    )
        :writer_(output.string(), wxPORTRAIT)
        ,dc_(writer_.dc())
        ,ledger_(ledger)
        ,properties_(properties::init(ledger))
    {
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
        page.render(ledger_, writer_, dc_, properties_);
    }

    // Global illustration properties.
    class properties
    {
      public:
        // Fields can be accessed directly by pages.
        text lmi_version_;
        text date_prepared_;

        static properties init(Ledger const& ledger)
        {
            std::string lmi_version(LMI_VERSION);
            calendar_date prep_date;

            // Skip authentication for non-interactive regression testing.
            if(!global_settings::instance().regression_testing())
                {
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
                    (static_cast<int>(ledger.GetLedgerInvariant().EffDateJdn)
                    );
                }

            properties p;
            p.lmi_version_ = text::from(lmi_version);
            p.date_prepared_ =
                  text::from(month_name(prep_date.month()))
                + text::nbsp()
                + text::from(value_cast<std::string>(prep_date.day()))
                + text::from(", ")
                + text::from(value_cast<std::string>(prep_date.year()))
                ;
            return p;
        }

      private:
        // Ctor is private, use init() to create objects of this type.
        properties() = default;
    };

  private:
    // Writer object used for the page metrics and higher level functions.
    pdf_writer_wx writer_;

    // The DC associated with the writer, it could be accessed via it, but as
    // it's needed often, provide a shorter alias for it.
    wxDC& dc_;

    // Source of the data.
    Ledger const& ledger_;

    // Properties initialized only once and used by pages via public accessor.
    properties const properties_;

    // Number of last added page.
    int page_number_{0};
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
        ,pdf_illustration::properties const& props
        ) = 0;
};

class cover_page : public page
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,wxDC& dc
        ,pdf_illustration::properties const& props
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
                            (props.date_prepared_
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
            (text::from
                (invar.InsCoShortName
                +"Financial Group is a marketing name for "
                +invar.InsCoName
                +"("
                +invar.InsCoShortName
                +") and its affiliated company and sales representatives, "
                +invar.InsCoAddr
                +"."
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
    pdf_illustration pdf_ill(ledger, output);
    pdf_ill.add<cover_page>();
}

volatile bool ensure_setup = ledger_pdf_generator::set_creator
    (ledger_pdf_generator_wx::do_create
    );

} // Unnamed namespace.
