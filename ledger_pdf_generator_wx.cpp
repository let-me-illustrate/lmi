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
#include "bourn_cast.hpp"
#include "assert_lmi.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "data_directory.hpp"           // AddDataDir()
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
#include "version.hpp"
#include "wx_table_generator.hpp"

#include <wx/pdfdc.h>

#include <wx/image.h>
#include <wx/log.h>
#include <wx/html/m_templ.h>

#include <cstdint>                      // SIZE_MAX
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>                  // std::conditional
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

// Helper enums identifying the possible {Guaranteed,Current}{Zero,}
// combinations.
enum class base
    {guaranteed
    ,current
    };

enum class interest_rate
    {zero
    ,non_zero
    };

// And functions to retrieve their string representation.
std::string base_suffix(base guar_or_cur)
{
    switch(guar_or_cur)
        {
        case base::guaranteed: return "Guaranteed";
        case base::current:    return "Current"   ;
        }
    throw "Unreachable--unknown base value";
}

std::string ir_suffix(interest_rate zero_or_not)
{
    switch(zero_or_not)
        {
        case interest_rate::zero:     return "Zero";
        case interest_rate::non_zero: return ""    ;
        }
    throw "Unreachable--unknown interest_rate value";
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

    // This function is provided to be able to delegate to it in custom
    // interpolation functions, but usually shouldn't be called directly, just
    // use operator() below instead.
    std::string interpolation_func
        (std::string const& s
        ,interpolate_lookup_kind kind
        ) const
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
                    )
                    {
                        return interpolation_func(s, kind);
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

    void add_variable(std::string const& name, int value)
    {
        std::ostringstream oss;
        oss << value;
        add_variable(name, oss.str());
    }

    void add_variable(std::string const& name, bool value)
    {
        add_variable(name, std::string(value ? "1" : "0"));
    }

    // Detect, at compile-time, mistaken attempts to add floating point
    // variables: all those are only available from ledger_evaluator as they
    // must be formatted correctly.
    void add_variable(std::string const& name, double value) = delete;

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

    // Return the value of a single scalar variable.
    std::string evaluate(std::string const& name) const
    {
        return evaluator_(name);
    }

    // Return a single value of a vector variable.
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
        std::ifstream ifs(AddDataDir(file + ".mustache"));
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

// A slightly specialized table generator for the tables used in the
// illustrations.
class illustration_table_generator : public wx_table_generator
{
  public:
    static int const rows_per_group = 5;

    explicit illustration_table_generator(pdf_writer_wx& writer)
        :wx_table_generator
            (writer.dc()
            ,writer.get_horz_margin()
            ,writer.get_page_width()
            )
    {
        use_condensed_style();
        align_right();
    }

    // Return the amount of vertical space taken by separator lines in the
    // table headers.
    int get_separator_line_height() const
    {
        // This is completely arbitrary and chosen just because it seems to
        // look well.
        return row_height() / 2;
    }
};

// A helper mix-in class for pages using tables.
class using_illustration_table
{
  protected:
    // Description of a single table column.
    struct illustration_table_column
    {
        char const* variable_name;
        char const* label;
        char const* widest_text;
    };

    using illustration_table_columns = std::vector<illustration_table_column>;

    // Must be overridden to return the description of the table columns.
    virtual illustration_table_columns const& get_table_columns() const = 0;

    // May be overridden to return false if the given column shouldn't be shown
    // for the specific ledger values (currently used to exclude individual
    // columns from composite illustrations).
    virtual bool should_show_column(Ledger const& ledger, int column) const
    {
        stifle_warning_for_unused_value(ledger);
        stifle_warning_for_unused_value(column);
        return true;
    }

    // Useful helper for creating the table generator using the columns defined
    // by the separate (and simpler to implement) get_table_columns() pure
    // virtual method.
    illustration_table_generator create_table_generator
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ) const
    {
        // Set the smaller font used for all tables before creating the table
        // generator which uses the DC font for its measurements.
        auto& dc = writer.dc();
        auto font = dc.GetFont();
        font.SetPointSize(9);
        dc.SetFont(font);

        illustration_table_generator table(writer);

        // But set the highlight colour for drawing separator lines after
        // creating it to override its default pen.
        dc.SetPen(HIGHLIGHT_COL);

        int column = 0;
        for(auto const& i : get_table_columns())
            {
            std::string label;
            if(should_show_column(ledger, column++))
                {
                label = i.label;
                }
            //else: Leave the label empty to avoid showing the column.

            table.add_column(label, i.widest_text);
            }

        return table;
    }
};

// Base class for our custom HTML cells providing a way to pass them
// information about the PDF document being generated and the ledger used to
// generate it.
class html_cell_for_pdf_output : public wxHtmlCell
{
  public:
    // Before using this class a pdf_context_setter object needs to be
    // instantiated (and remain alive for as long as this class is used).
    class pdf_context_setter
    {
      public:
        // References passed to the ctor must have lifetime greater than that
        // of this object itself.
        explicit pdf_context_setter
            (Ledger const& ledger
            ,pdf_writer_wx& writer
            ,html_interpolator const& interpolate_html
            )
        {
            html_cell_for_pdf_output::pdf_context_for_html_output.set
                (&ledger
                ,&writer
                ,&interpolate_html
                );
        }

        ~pdf_context_setter()
        {
            html_cell_for_pdf_output::pdf_context_for_html_output.set
                (nullptr
                ,nullptr
                ,nullptr
                );
        }
    };

  protected:
    // This is ugly, but we have to use a global variable to make pdf_writer_wx
    // and wxDC objects used by the main code accessible to this cell class,
    // there is no way to pass them as parameters through wxHTML machinery.
    //
    // To at least make it a little bit safer to deal with this, the variable
    // itself is private and a public pdf_context_setter class is provided to
    // actually set it.
    class pdf_context
    {
      public:
        void set
            (Ledger const* ledger
            ,pdf_writer_wx* writer
            ,html_interpolator const* interpolate_html
            )
        {
            ledger_ = ledger;
            writer_ = writer;
            interpolate_html_ = interpolate_html;
        }

        Ledger const& ledger() const
        {
            LMI_ASSERT(ledger_);
            return *ledger_;
        }

        pdf_writer_wx& writer() const
        {
            LMI_ASSERT(writer_);
            return *writer_;
        }

        html_interpolator const& interpolate_html() const
        {
            LMI_ASSERT(interpolate_html_);
            return *interpolate_html_;
        }

      private:
        Ledger const* ledger_ = nullptr;
        pdf_writer_wx* writer_ = nullptr;
        html_interpolator const* interpolate_html_ = nullptr;
    };

    // Small helper to check that we're using the expected DC and, also, acting
    // as a sink for the never used parameters of Draw().
    void draw_check_precondition
        (wxDC& dc
        ,int view_y1
        ,int view_y2,
        wxHtmlRenderingInfo& info
        )
    {
        // The DC passed to this function is supposed to be the same as the one
        // associated with the writer we will use for rendering, but check that
        // this is really so in order to avoid unexpectedly drawing the table
        // on something else.
        LMI_ASSERT(&dc == &pdf_context_for_html_output.writer().dc());

        // There is no need to optimize drawing by restricting it to the
        // currently shown positions, we always render the cell entirely.
        stifle_warning_for_unused_value(view_y1);
        stifle_warning_for_unused_value(view_y2);

        // We don't care about rendering state as we don't support interactive
        // selection anyhow.
        stifle_warning_for_unused_value(info);
    }

    static pdf_context pdf_context_for_html_output;

    friend pdf_context_setter;
};

html_cell_for_pdf_output::pdf_context
html_cell_for_pdf_output::pdf_context_for_html_output;

// Define scaffolding for a custom HTML "scaled_image" tag which must be used
// instead of the standard "a" in order to allow specifying the scaling factor
// that we want to use for the image in the PDF. Unfortunately this can't be
// achieved by simply using "width" and/or "height" attributes of the "a" tag
// because their values can only be integers which is not precise enough to
// avoid (slightly but noticeably) distorting the image due to the aspect ratio
// being not quite right.

class scaled_image_cell : public html_cell_for_pdf_output
{
  public:
    scaled_image_cell
        (wxImage const& image
        ,wxString const& src
        ,double scale_factor
        )
        :image_(image)
        ,src_(src)
        ,scale_factor_(scale_factor)
    {
        m_Width  = wxRound(image.GetWidth () / scale_factor);
        m_Height = wxRound(image.GetHeight() / scale_factor);
    }

    // Override the base class method to actually render the image.
    void Draw
        (wxDC& dc
        ,int x
        ,int y
        ,int view_y1
        ,int view_y2,
        wxHtmlRenderingInfo& info
        ) override
    {
        draw_check_precondition(dc, view_y1, view_y2, info);

        auto& writer = pdf_context_for_html_output.writer();

        x += m_PosX;

        int pos_y = y + m_PosY;
        writer.output_image(image_, src_.utf8_str(), scale_factor_, x, &pos_y);
    }

  private:
    wxImage const image_;
    wxString const src_;
    double const scale_factor_;
};

TAG_HANDLER_BEGIN(scaled_image, "SCALED_IMAGE")
    TAG_HANDLER_PROC(tag)
    {
        wxString src;
        if (!tag.GetParamAsString("SRC", &src))
            {
            throw std::runtime_error
                ("missing mandatory \"src\" attribute of \"scaled_image\" tag"
                );
            }

        // The scale factor is optional.
        double scale_factor = 1.;

        // But if it is given, we currently specify its inverse in HTML just
        // because it so happens that for the scale factors we use the inverse
        // can be expressed exactly in decimal notation, while the factor
        // itself can't. In principle, the converse could also happen and we
        // might add support for "factor" attribute too in this case. Or we
        // could use separate "numerator" and "denominator" attributes. But for
        // now implement just the bare minimum of what we need.
        wxString inv_factor_str;
        if (tag.GetParamAsString("INV_FACTOR", &inv_factor_str))
            {
            double inv_factor = 0.;
            if (!inv_factor_str.ToCDouble(&inv_factor) || inv_factor == 0.)
                {
                throw std::runtime_error
                    ( "invalid value for \"inv_factor\" attribute of "
                      "\"scaled_image\" tag: \""
                    + inv_factor_str.ToStdString()
                    + "\""
                    );
                }

            scale_factor = 1./inv_factor;
            }

        wxImage image;
        // Disable error logging, we'll simply ignore the tag if the image is
        // not present.
            {
            wxLogNull noLog;
            image.LoadFile(src);
            }

        if (image.IsOk())
            {
            m_WParser->GetContainer()->InsertCell
                (new scaled_image_cell(image, src, scale_factor)
                );
            }

        // This tag isn't supposed to have any inner contents, so return true
        // to not even try parsing it.
        return true;
    }
TAG_HANDLER_END(scaled_image)

class pdf_illustration;

class page
{
  public:
    page() = default;

    // Pages are not value-like objects, so prohibit copying them.
    page(page const&) = delete;
    page& operator=(page const&) = delete;

    // Make base class dtor virtual.
    virtual ~page() = default;

    // Associate the illustration object using this page with it.
    //
    // This object is not passed as a ctor argument because it would be
    // redundant, instead it is associated with the page when it's added to an
    // illustration. This method is supposed to be called only once.
    void illustration(pdf_illustration const& illustration)
    {
        LMI_ASSERT(!illustration_);

        illustration_ = &illustration;
    }

    // Called before rendering any pages to prepare for doing this, e.g. by
    // computing the number of pages needed.
    //
    // This method must not draw anything on the wxDC, it is provided only for
    // measurement purposes.
    virtual void pre_render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        )
    {
        stifle_warning_for_unused_value(ledger);
        stifle_warning_for_unused_value(writer);
        stifle_warning_for_unused_value(interpolate_html);
    }

    // Render this page contents.
    virtual void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) = 0;

  protected:
    // Helper method for rendering the contents of the given external template,
    // which is expected to be found in the file with the provided name and
    // ".mustache" extension in the data directory.
    //
    // Return the height of the page contents.
    int render_page_template
        (std::string const& template_name
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        )
    {
        return writer.output_html
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,interpolate_html("{{>" + template_name + "}}")
            );
    }

    // The associated illustration, which will be non-null by the time our
    // virtual methods such as pre_render() and render() are called.
    pdf_illustration const* illustration_ = nullptr;
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
        ,ledger_(ledger)
    {
        init_variables();
    }

    // Make base class dtor virtual.
    virtual ~pdf_illustration() = default;

    // Add a page.
    //
    // This is a template just in order to save on writing std::make_unique<>()
    // in the code using it to make it slightly shorter.
    template<typename T, typename... Args>
    void add(Args&&... args)
    {
        auto page = std::make_unique<T>(std::forward<Args>(args)...);
        page->illustration(*this);
        pages_.emplace_back(std::move(page));
    }

    // Render all pages.
    void render_all()
    {
        html_cell_for_pdf_output::pdf_context_setter
            set_pdf_context(ledger_, writer_, *this);

        for(auto const& page : pages_)
            {
            page->pre_render(ledger_, writer_, *this);
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
                writer_.dc().StartPage();
                }

            page->render(ledger_, writer_, *this);
            }
    }

    // Methods to be implemented by the derived classes to indicate which
    // templates should be used for the upper (above the separating line) and
    // the lower parts of the footer. The upper template name may be empty if
    // it is not used at all.
    virtual std::string get_upper_footer_template_name() const = 0;
    virtual std::string get_lower_footer_template_name() const = 0;

  protected:
    // Helper for abbreviating a string to at most the given length (in bytes).
    static std::string abbreviate_if_necessary(std::string s, size_t len)
    {
        if(s.length() > len && len > 3)
            {
            s.replace(len - 3, std::string::npos, "...");
            }

        return s;
    }

    // Helper for creating abbreviated variables in the derived classes: such
    // variables have the name based on the name of the original variable with
    // "Abbrev" and "len" appended to it and their value is at most "len" bytes
    // long.
    void add_abbreviated_variable(std::string const& var, size_t len)
    {
        add_variable
            (var + "Abbrev" + std::to_string(len)
            ,abbreviate_if_necessary(evaluate(var), len)
            );
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

        auto const& invar = ledger_.GetLedgerInvariant();

        add_abbreviated_variable("CorpName", 60);
        add_abbreviated_variable("Insured1", 30);

        // Define the variables needed by contract_numbers template.
        add_variable
            ("HasMasterContract"
            ,!invar.MasterContractNumber.empty()
            );
        add_variable
            ("HasPolicyNumber"
            ,!invar.ContractNumber.empty()
            );

        size_t const full_abbrev_length = 30;
        add_abbreviated_variable("MasterContractNumber", full_abbrev_length);
        add_abbreviated_variable("MasterContractNumber", full_abbrev_length / 2);
        add_abbreviated_variable("ContractNumber", full_abbrev_length);
        add_abbreviated_variable("ContractNumber", full_abbrev_length / 2);

        add_variable
            ("HasComplianceTrackingNumber"
            ,(*this)("{{>compliance_tracking_number}}")
                .as_html().find_first_not_of(" \n")
                != std::string::npos
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
            ("MecYearPlus1"
            ,bourn_cast<int>(invar.MecYear) + 1
            );

        add_variable
            ("UWTypeIsMedical"
            ,invar.UWType == "Medical"
            );

        add_variable
            ("UWClassIsRated"
            ,invar.UWClass == "Rated"
            );

        auto const& policy_name = invar.PolicyLegalName;
        add_variable
            ("GroupCarveout"
            ,policy_name == "Group Flexible Premium Adjustable Life Insurance Certificate"
            );

        auto const& state_abbrev = invar.GetStatePostalAbbrev();
        add_variable
            ("StateIsCarolina"
            ,state_abbrev == "NC" || state_abbrev == "SC"
            );

        add_variable
            ("StateIsMaryland"
            ,state_abbrev == "MD"
            );
    }

    // Use non-default font sizes to make it simpler to replicate the existing
    // illustrations.
    static std::array<int, 7> const html_font_sizes;

    // Writer object used for the page metrics and higher level functions.
    pdf_writer_wx writer_;

    // Source of the data.
    Ledger const& ledger_;

    // All the pages of this illustration.
    std::vector<std::unique_ptr<page>> pages_;
};

std::array<int, 7> const pdf_illustration::html_font_sizes
    {
    { 8
    , 9
    ,10
    ,12
    ,14
    ,18
    ,20
    }
    };

class cover_page : public page
{
  public:
    void render
        (Ledger const& /* ledger */
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        int const height_contents = render_page_template
            ("cover"
            ,writer
            ,interpolate_html
            );

        // There is no way to draw a border around the page contents in wxHTML
        // currently, so do it manually.
        auto& dc = writer.dc();

        dc.SetPen(wxPen(HIGHLIGHT_COL, 2));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);

        dc.DrawRectangle
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,height_contents
            );
    }
};

/// Base class for all pages with a footer.
class page_with_footer : public page
{
  public:
    // Override pre_render() to compute footer_top_ which is needed in the
    // derived classes overridden get_extra_pages_needed().
    void pre_render
        (Ledger const& /* ledger */
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        auto const frame_horz_margin = writer.get_horz_margin();
        auto const frame_width       = writer.get_page_width();

        // We implicitly assume here that get_footer_lower_html() result
        // doesn't materially depend on the exact value of the page number as
        // we don't know its definitive value here yet. In theory, this doesn't
        // need to be true, e.g. we may later discover that 10 pages are needed
        // instead of 9 and the extra digit might result in a line wrapping on
        // a new line and this increasing the footer height, but in practice
        // this doesn't risk happening and taking into account this possibility
        // wouldn't be simple at all, so just ignore this possibility.
        auto footer_height = writer.output_html
            (frame_horz_margin
            ,0
            ,frame_width
            ,get_footer_lower_html(interpolate_html)
            ,e_output_measure_only
            );

        auto const&
            upper_template = illustration_->get_upper_footer_template_name();
        if(!upper_template.empty())
            {
            footer_height += writer.output_html
                (frame_horz_margin
                ,0
                ,frame_width
                ,interpolate_html("{{>" + upper_template + "}}")
                ,e_output_measure_only
                );
            }

        footer_top_ = writer.get_page_bottom() - footer_height;
    }

    void render
        (Ledger const& /* ledger */
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        auto const frame_horz_margin = writer.get_horz_margin();
        auto const frame_width       = writer.get_page_width();

        auto y = footer_top_;

        auto const&
            upper_template = illustration_->get_upper_footer_template_name();
        if(!upper_template.empty())
            {
            y += writer.output_html
                (frame_horz_margin
                ,footer_top_
                ,frame_width
                ,interpolate_html("{{>" + upper_template + "}}")
                );
            }

        writer.output_html
            (frame_horz_margin
            ,y
            ,frame_width
            ,get_footer_lower_html(interpolate_html)
            );

        auto& dc = writer.dc();

        dc.SetPen(HIGHLIGHT_COL);
        dc.DrawLine
            (frame_horz_margin
            ,y
            ,frame_width + frame_horz_margin
            ,y
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
    // the page number or equivalent string (e.g. "Appendix").
    virtual std::string get_page_number() const = 0;

    // This method uses get_page_number() and returns the HTML wrapping it
    // and other fixed information appearing in the lower part of the footer.
    text get_footer_lower_html(html_interpolator const& interpolate_html) const
    {
        auto const page_number_str = get_page_number();

        auto const templ = illustration_->get_lower_footer_template_name();

        // Use our own interpolation function to handle the special
        // "page_number" variable that is replaced with the actual
        // (possibly dynamic) page number.
        return text::from_html
            (interpolate_string
                (("{{>" + templ + "}}").c_str()
                ,[page_number_str, interpolate_html]
                    (std::string const& s
                    ,interpolate_lookup_kind kind
                    ) -> std::string
                    {
                    if(s == "page_number")
                        {
                        return page_number_str;
                        }

                    return interpolate_html.interpolation_func(s, kind);
                    }
                )
            );
    }

    int footer_top_ = 0;
};

/// Base class for attachment pages.
class attachment_page : public page_with_footer
{
  private:
    std::string get_page_number() const override
    {
        return "Attachment";
    }
};

/// Base class for all pages showing the page number in the footer.
///
/// In addition to actually providing page_with_footer with the correct string
/// to show in the footer, this class implicitly handles the page count by
/// incrementing it whenever a new object of this class is pre-rendered.
class numbered_page : public page_with_footer
{
  public:
    void pre_render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        page_with_footer::pre_render(ledger, writer, interpolate_html);

        this_page_number_ = ++last_page_number_;

        extra_pages_ = get_extra_pages_needed
            (ledger
            ,writer
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
    void next_page(pdf_writer_wx& writer)
    {
        // This method may only be called if we had reserved enough physical
        // pages for this logical pages by overriding get_extra_pages_needed().
        LMI_ASSERT(extra_pages_ > 0);

        writer.dc().StartPage();

        this_page_number_++;
        extra_pages_--;
    }

  private:
    // Derived classes may override this method if they may need more than one
    // physical page to show their contents.
    virtual int get_extra_pages_needed
        (Ledger const&              ledger
        ,pdf_writer_wx&             writer
        ,html_interpolator const&   interpolate_html
        ) const
    {
        stifle_warning_for_unused_value(ledger);
        stifle_warning_for_unused_value(writer);
        stifle_warning_for_unused_value(interpolate_html);

        return 0;
    }

    std::string get_page_number() const override
    {
        std::ostringstream oss;
        oss << "Page " << this_page_number_ << " of " << last_page_number_;
        return oss.str();
    }

    static int last_page_number_;
    int        this_page_number_     = 0;
    int        extra_pages_          = 0;
};

int numbered_page::last_page_number_ = 0;

// Simplest possible page which is entirely defined by its external template
// whose name must be specified when constructing it.
class standard_page : public numbered_page
{
  public:
    // Accept only string literals as template names, there should be no need
    // to use anything else.
    template<int N>
    explicit standard_page(char const (&page_template_name)[N])
        :page_template_name_(page_template_name)
    {
    }

    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        numbered_page::render(ledger, writer, interpolate_html);

        render_page_template(page_template_name_, writer, interpolate_html);
    }

  private:
    char const* const page_template_name_;
};

// Helper classes used to show the numeric summary table. The approach used
// here is to define a custom HTML tag (<numeric_summary_table>) and use the
// existing illustration_table_generator to replace it with the actual table
// when rendering.
//
// Notice that we currently make the simplifying assumption that this table is
// always short enough so that everything fits on the same page as it would be
// much more complicated to handle page breaks in the table in the middle of a
// page (page_with_tabular_report below handles them only for the table at the
// bottom of the page, after all the other contents, and this is already more
// complicated and can't be done with just a custom HTML tag as we do it here).

// An HTML cell showing the contents of the numeric summary table.
class numeric_summary_table_cell
    :public html_cell_for_pdf_output
    ,private using_illustration_table
{
  public:
    numeric_summary_table_cell()
    {
        m_Height = render_or_measure(0, e_output_measure_only);
    }

    // Override the base class method to actually render the table.
    void Draw
        (wxDC& dc
        ,int x
        ,int y
        ,int view_y1
        ,int view_y2,
        wxHtmlRenderingInfo& info
        ) override
    {
        draw_check_precondition(dc, view_y1, view_y2, info);

        // We ignore the horizontal coordinate which is always 0 for this cell
        // anyhow.
        stifle_warning_for_unused_value(x);

        render_or_measure(y + m_PosY, e_output_normal);
    }

  private:
    enum
        {column_policy_year
        ,column_premium_outlay
        ,column_guar_account_value
        ,column_guar_cash_surr_value
        ,column_guar_death_benefit
        ,column_separator_guar_non_guar
        ,column_mid_account_value
        ,column_mid_cash_surr_value
        ,column_mid_death_benefit
        ,column_separator_mid_cur
        ,column_cur_account_value
        ,column_cur_cash_surr_value
        ,column_cur_death_benefit
        ,column_max
        };

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"             , "Policy\nYear"       ,       "999" }
            ,{ "GrossPmt"               , "Premium\nOutlay"    ,   "999,999" }
            ,{ "AcctVal_Guaranteed"     , "Account\nValue"     ,   "999,999" }
            ,{ "CSVNet_Guaranteed"      , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_Guaranteed" , "Death\nBenefit"     , "9,999,999" }
            ,{ ""                       , " "                  ,         "-" }
            ,{ "AcctVal_Midpoint"       , "Account\nValue"     ,   "999,999" }
            ,{ "CSVNet_Midpoint"        , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_Midpoint"   , "Death\nBenefit"     , "9,999,999" }
            ,{ ""                       , " "                  ,         "-" }
            ,{ "AcctVal_Current"        , "Account\nValue"     ,   "999,999" }
            ,{ "CSVNet_Current"         , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_Current"    , "Death\nBenefit"     , "9,999,999" }
            };

        return columns;
    }

    int render_or_measure(int pos_y, enum_output_mode output_mode)
    {
        auto const& ledger = pdf_context_for_html_output.ledger();
        auto& writer = pdf_context_for_html_output.writer();

        illustration_table_generator
            table{create_table_generator(ledger, writer)};

        // Output multiple rows of headers.

        // Make a copy because we want pos_y to be modified only once, not
        // twice, by both output_super_header() calls.
        auto y_copy = pos_y;
        table.output_super_header
            ("Guaranteed Values"
            ,column_guar_account_value
            ,column_separator_guar_non_guar
            ,&y_copy
            ,output_mode
            );
        table.output_super_header
            ("Non-Guaranteed Values"
            ,column_mid_account_value
            ,column_max
            ,&pos_y
            ,output_mode
            );

        pos_y += table.get_separator_line_height();
        table.output_horz_separator
            (column_guar_account_value
            ,column_separator_guar_non_guar
            ,pos_y
            ,output_mode
            );
        table.output_horz_separator
            (column_mid_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        y_copy = pos_y;
        table.output_super_header
            ("Midpoint Values"
            ,column_mid_account_value
            ,column_separator_mid_cur
            ,&y_copy
            ,output_mode
            );

        table.output_super_header
            ("Current Values"
            ,column_cur_account_value
            ,column_max
            ,&pos_y
            ,output_mode
            );

        pos_y += table.get_separator_line_height();
        table.output_horz_separator
            (column_mid_account_value
            ,column_separator_mid_cur
            ,pos_y
            ,output_mode
            );

        table.output_horz_separator
            (column_cur_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        table.output_header(&pos_y, output_mode);

        pos_y += table.get_separator_line_height();
        table.output_horz_separator(0, column_max, pos_y, output_mode);

        // And now the table values themselves.
        auto const& columns = get_table_columns();
        std::vector<std::string> values(columns.size());

        auto const& invar = ledger.GetLedgerInvariant();
        auto const& interpolate_html = pdf_context_for_html_output.interpolate_html();

        int const year_max = pdf_context_for_html_output.ledger().GetMaxLength();
        int const age_last = 70;
        std::array<int, 4> const summary_years =
            {{5, 10, 20, age_last - bourn_cast<int>(invar.Age)}
            };
        for(auto const& year : summary_years)
            {
            // Skip row if it doesn't exist. For instance, if the issue
            // age is 85 and the contract remains in force until age 100,
            // then there is no twentieth duration and no age-70 row.
            if(!(0 < year && year <= year_max))
                {
                continue;
                }

            // Last row, showing the values for "Age 70" normally, needs to be
            // handled specially.
            bool const is_last_row = &year == &summary_years.back();

            // For composite ledgers, "Age" doesn't make sense and so this row
            // should be just skipped for them.
            if(is_last_row && ledger.is_composite())
                {
                continue;
                }

            switch(output_mode)
                {
                case e_output_measure_only:
                    pos_y += table.row_height();
                    break;

                case e_output_normal:
                    for(std::size_t col = 0; col < columns.size(); ++col)
                        {
                        std::string const variable_name = columns[col].variable_name;

                        // According to regulations, we need to replace the
                        // policy year in the last row with the age.
                        if(col == column_policy_year)
                            {
                            if(is_last_row)
                                {
                                std::ostringstream oss;
                                oss << "Age " << age_last;
                                values[col] = oss.str();
                                continue;
                                }
                            }

                        // Special hack for the dummy columns whose value is always
                        // empty as it's used only as separator.
                        values[col] = variable_name.empty()
                            ? std::string{}
                            : interpolate_html.evaluate(variable_name, year - 1)
                            ;
                        }

                    table.output_row(&pos_y, values.data());
                    break;
                }
            }

        return pos_y;
    }
};

// Custom tag which is replaced by the numeric summary table.
TAG_HANDLER_BEGIN(numeric_summary_table, "NUMERIC_SUMMARY_TABLE")
    TAG_HANDLER_PROC(tag)
    {
        // The tag argument would be useful if we defined any parameters for
        // it, but currently we don't.
        stifle_warning_for_unused_value(tag);

        m_WParser->GetContainer()->InsertCell(new numeric_summary_table_cell());

        // This tag isn't supposed to have any inner contents, so return true
        // to not even try parsing it.
        return true;
    }
TAG_HANDLER_END(numeric_summary_table)

// In wxWidgets versions prior to 3.1.1, there is an extra semicolon at the end
// of TAGS_MODULE_BEGIN() expansion resulting in a warning with -pedantic used
// by lmi, so suppress this warning here (this could be removed once 3.1.1 is
// required).
wxGCC_WARNING_SUPPRESS(pedantic)

TAGS_MODULE_BEGIN(lmi_illustration)
    TAGS_MODULE_ADD(scaled_image)
    TAGS_MODULE_ADD(numeric_summary_table)
TAGS_MODULE_END(lmi_illustration)

wxGCC_WARNING_RESTORE(pedantic)

// Numeric summary page appears twice, once as a normal page and once as an
// attachment, with the only difference being that the base class is different,
// so make it a template to avoid duplicating the code.

// Just a helper alias.
template<bool is_attachment>
using numbered_or_attachment_base = typename std::conditional
    <is_attachment
    ,attachment_page
    ,numbered_page
    >::type;

template<bool is_attachment>
class numeric_summary_or_attachment_page
    : public numbered_or_attachment_base<is_attachment>
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        numbered_or_attachment_base<is_attachment>::render
            (ledger
            ,writer
            ,interpolate_html
            );

        this->render_page_template("numeric_summary", writer, interpolate_html);
    }
};

// Helper base class for pages showing a table displaying values for all
// contract years after some fixed content.
class page_with_tabular_report
    :public numbered_page
    ,protected using_illustration_table
{
  public:
    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        numbered_page::render(ledger, writer, interpolate_html);

        illustration_table_generator
            table{create_table_generator(ledger, writer)};

        auto const& columns = get_table_columns();

        // Just some cached values used inside the loop below.
        auto const row_height = table.row_height();
        auto const page_bottom = get_footer_top();
        auto const rows_per_group = illustration_table_generator::rows_per_group;
        std::vector<std::string> values(columns.size());

        // The table may need several pages, loop over them.
        int const year_max = ledger.GetMaxLength();
        for(int year = 0; year < year_max; ++year)
            {
            int pos_y = render_or_measure_fixed_page_part
                (table
                ,writer
                ,interpolate_html
                ,e_output_normal
                );

            for(; year < year_max; ++year)
                {
                for(std::size_t col = 0; col < columns.size(); ++col)
                    {
                    std::string const variable_name = columns[col].variable_name;

                    // Special hack for the dummy columns used in some reports,
                    // whose value is always empty as it's used only as
                    // separator.
                    values[col] = variable_name.empty()
                        ? std::string{}
                        : interpolate_html.evaluate(variable_name, year)
                        ;
                    }

                table.output_row(&pos_y, values.data());

                if((year + 1) % rows_per_group == 0)
                    {
                    // We need a group break.
                    pos_y += row_height;

                    // And possibly a page break, which will be necessary if we don't
                    // have enough space for another full group because we don't want
                    // to have page breaks in the middle of a group.
                    if(pos_y >= page_bottom - rows_per_group*row_height)
                        {
                        next_page(writer);
                        numbered_page::render(ledger, writer, interpolate_html);
                        break;
                        }
                    }
                }
            }
    }

  protected:
    // Must be overridden to return the contents of the fixed page part. This
    // string will be interpolated and typically will be just "{{>template}}".
    virtual std::string get_fixed_page_contents() const = 0;

    // May be overridden to render (only if output_mode is e_output_normal)
    // the extra headers just above the regular table headers.
    //
    // If this function does anything, it must show the first super-header at
    // pos_y and update it to account for the added lines. The base class
    // version does nothing.
    virtual void render_or_measure_extra_headers
        (illustration_table_generator&  table
        ,html_interpolator const&       interpolate_html
        ,int*                           pos_y
        ,enum_output_mode               output_mode
        ) const
    {
        stifle_warning_for_unused_value(table);
        stifle_warning_for_unused_value(interpolate_html);
        stifle_warning_for_unused_value(pos_y);
        stifle_warning_for_unused_value(output_mode);
    }

  private:
    // Render (only if output_mode is e_output_normal) the fixed page part and
    // (in any case) return the vertical coordinate of its bottom, where the
    // tabular report starts.
    int render_or_measure_fixed_page_part
        (illustration_table_generator&  table
        ,pdf_writer_wx&                 writer
        ,html_interpolator const&       interpolate_html
        ,enum_output_mode               output_mode
        ) const
    {
        int pos_y = writer.get_vert_margin();

        pos_y += writer.output_html
            (writer.get_horz_margin()
            ,pos_y
            ,writer.get_page_width()
            ,interpolate_html(get_fixed_page_contents())
            ,output_mode
            );

        render_or_measure_extra_headers
            (table
            ,interpolate_html
            ,&pos_y,
            output_mode
            );

        table.output_header(&pos_y, output_mode);

        pos_y += table.get_separator_line_height();
        table.output_horz_separator
            (0
            ,table.columns_count()
            ,pos_y
            ,output_mode
            );

        return pos_y;
    }

    // Override the base class method as the table may overflow onto the next
    // page(s).
    int get_extra_pages_needed
        (Ledger const&              ledger
        ,pdf_writer_wx&             writer
        ,html_interpolator const&   interpolate_html
        ) const override
    {
        illustration_table_generator
            table{create_table_generator(ledger, writer)};

        int const pos_y = render_or_measure_fixed_page_part
            (table
            ,writer
            ,interpolate_html
            ,e_output_measure_only
            );

        int const rows_per_page = (get_footer_top() - pos_y) / table.row_height();

        int const rows_per_group = illustration_table_generator::rows_per_group;

        if(rows_per_page < rows_per_group)
            {
            // We can't afford to continue in this case as we can never output
            // the table as the template simply doesn't leave enough space for
            // it on the page.
            throw std::runtime_error("no space left for tabular report");
            }

        // Each group actually takes rows_per_group+1 rows because of the
        // separator row between groups, hence the second +1, but there is no
        // need for the separator after the last group, hence the first +1.
        int const groups_per_page = (rows_per_page + 1) / (rows_per_group + 1);

        // But we are actually interested in the number of years per page and
        // not the number of groups.
        int const years_per_page = groups_per_page * rows_per_group;

        // Finally determine how many pages we need to show all the years.
        return ledger.GetMaxLength() / years_per_page;
    }
};

class tabular_detail_page : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_premium_outlay
        ,column_guar_account_value
        ,column_guar_cash_surr_value
        ,column_guar_death_benefit
        ,column_dummy_separator
        ,column_cur_account_value
        ,column_cur_cash_surr_value
        ,column_cur_death_benefit
        ,column_max
        };

    std::string get_fixed_page_contents() const override
    {
        return "{{>tabular_details}}";
    }

    void render_or_measure_extra_headers
        (illustration_table_generator&  table
        ,html_interpolator const&       interpolate_html
        ,int*                           pos_y
        ,enum_output_mode               output_mode
        ) const override
    {
        stifle_warning_for_unused_value(interpolate_html);

        // Make a copy because we want the real pos_y to be modified only once,
        // not twice, by both output_super_header() calls.
        auto pos_y_copy = *pos_y;
        table.output_super_header
            ("Guaranteed Values"
            ,column_guar_account_value
            ,column_dummy_separator
            ,&pos_y_copy
            ,output_mode
            );
        table.output_super_header
            ("Non-Guaranteed Values"
            ,column_cur_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        *pos_y += table.get_separator_line_height();
        table.output_horz_separator
            (column_guar_account_value
            ,column_dummy_separator
            ,*pos_y
            ,output_mode
            );
        table.output_horz_separator
            (column_cur_account_value
            ,column_max
            ,*pos_y
            ,output_mode
            );
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"             , "Policy\nYear"       ,       "999" }
            ,{ "AttainedAge"            , "End of\nYear Age"   ,       "999" }
            ,{ "GrossPmt"               , "Premium\nOutlay"    ,   "999,999" }
            ,{ "AcctVal_Guaranteed"     , "Account\nValue"     ,   "999,999" }
            ,{ "CSVNet_Guaranteed"      , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_Guaranteed" , "Death\nBenefit"     , "9,999,999" }
            ,{ ""                       , " "                  ,      "----" }
            ,{ "AcctVal_Current"        , "Account\nValue"     ,   "999,999" }
            ,{ "CSVNet_Current"         , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_Current"    , "Death\nBenefit"     , "9,999,999" }
            };

        return columns;
    }

    bool should_show_column(Ledger const& ledger, int column) const override
    {
        // One column should be hidden for composite ledgers.
        return column != column_end_of_year_age || !ledger.is_composite();
    }
};

class tabular_detail2_page : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_ill_crediting_rate
        ,column_selected_face_amount
        ,column_max
        };

    std::string get_fixed_page_contents() const override
    {
        return "{{>tabular_details2}}";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"          , "Policy\nYear"               ,         "999" }
            ,{ "AttainedAge"         , "End of\nYear Age"           ,         "999" }
            ,{ "AnnGAIntRate_Current", "Illustrated\nCrediting Rate",      "99.99%" }
            ,{ "SpecAmt"             , "Selected\nFace Amount"      , "999,000,000" }
            };

        return columns;
    }

    bool should_show_column(Ledger const& ledger, int column) const override
    {
        // One column should be hidden for composite ledgers.
        return column != column_end_of_year_age || !ledger.is_composite();
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

        // Define variables specific to this illustration which doesn't use the
        // standard 60/30 lengths for whatever reason.
        add_abbreviated_variable("CorpName", 50);
        add_abbreviated_variable("Insured1", 50);

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
            ("GroupExperienceRating"
            ,policy_name == "Group Flexible Premium Adjustable Life Insurance Policy"
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
            ("HasInterestDisclaimer"
            ,!invar.InterestDisclaimer.empty()
            );

        add_variable
            ("HasGuarPrem"
            ,invar.GuarPrem != 0
            );

        add_variable
            ("StateIsIllinois"
            ,state_abbrev == "IL"
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
            ,bourn_cast<int>(lapse_year_guaruanteed) + 1
            );

        add_variable
            ("LapseYear_Midpoint_LT_MaxDuration"
            ,lapse_year_midpoint < max_duration
            );

        add_variable
            ("LapseYear_Midpoint_Plus1"
            ,bourn_cast<int>(lapse_year_midpoint) + 1
            );

        add_variable
            ("LapseYear_Current_LT_MaxDuration"
            ,lapse_year_current < max_duration
            );

        add_variable
            ("LapseYear_Current_Plus1"
            ,bourn_cast<int>(lapse_year_current) + 1
            );

        // Add all the pages.
        add<cover_page>();
        add<standard_page>("narrative_summary");
        add<standard_page>("narrative_summary_cont");
        add<standard_page>("column_headings");
        if(!invar.IsInforce)
            {
            add<numeric_summary_or_attachment_page<false>>();
            }
        add<tabular_detail_page>();
        add<tabular_detail2_page>();
        if(!invar.IsInforce)
            {
            add<numeric_summary_or_attachment_page<true>>();
            }
    }

    std::string get_upper_footer_template_name() const override { return {}; }
    std::string get_lower_footer_template_name() const override { return "footer"; }
};

// Common base class for basic illustration pages using the same columns in
// both NASD and private group placement illustrations.
class page_with_basic_tabular_report : public page_with_tabular_report
{
  private:
    // This method must be overridden to return the text of the super-header
    // used for all pairs of "cash surrogate value" and "death benefit"
    // columns. The return value is subject to HTML interpolation and so may
    // contain {{variables}} and also can be multiline but, if so, it must have
    // the same number of lines for all input arguments.
    //
    // The suffix_xxx arguments can be used to construct the full name of the
    // variable appropriate for the current column pair: the short one is
    // either "Guaranteed" or "Current" while the full one also includes "Zero"
    // for the column pairs using zero interest-rate assumption.
    virtual std::string get_two_column_header
        (std::string const& suffix_full
        ,std::string const& suffix_short
        ) const = 0;

    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_premium_outlay
        ,column_guar0_cash_surr_value
        ,column_guar0_death_benefit
        ,column_separator_guar0_guar
        ,column_guar_cash_surr_value
        ,column_guar_death_benefit
        ,column_separator_guar_cur0
        ,column_cur0_cash_surr_value
        ,column_cur0_death_benefit
        ,column_separator_cur0_cur
        ,column_cur_cash_surr_value
        ,column_cur_death_benefit
        ,column_max
        };

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"     ,       "999" }
            ,{ "AttainedAge"                , "End of\nYear Age" ,       "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"  ,   "999,999" }
            ,{ "CSVNet_GuaranteedZero"      , "Cash Surr\nValue" ,   "999,999" }
            ,{ "EOYDeathBft_GuaranteedZero" , "Death\nBenefit"   , "9,999,999" }
            ,{ ""                           , " "                ,         "-" }
            ,{ "CSVNet_Guaranteed"          , "Cash Surr\nValue" ,   "999,999" }
            ,{ "EOYDeathBft_Guaranteed"     , "Death\nBenefit"   , "9,999,999" }
            ,{ ""                           , " "                ,         "-" }
            ,{ "CSVNet_CurrentZero"         , "Cash Surr\nValue" ,   "999,999" }
            ,{ "EOYDeathBft_CurrentZero"    , "Death\nBenefit"   , "9,999,999" }
            ,{ ""                           , " "                ,         "-" }
            ,{ "CSVNet_Current"             , "Cash Surr\nValue" ,   "999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"   , "9,999,999" }
            };

        return columns;
    }

    bool should_show_column(Ledger const& ledger, int column) const override
    {
        // One column should be hidden for composite ledgers.
        return column != column_end_of_year_age || !ledger.is_composite();
    }

    void render_or_measure_extra_headers
        (illustration_table_generator&  table
        ,html_interpolator const&       interpolate_html
        ,int*                           pos_y
        ,enum_output_mode               output_mode
        ) const override
    {
        // Output the first super header row.

        auto pos_y_copy = *pos_y;
        table.output_super_header
            ("Using guaranteed charges"
            ,column_guar0_cash_surr_value
            ,column_separator_guar_cur0
            ,pos_y
            ,output_mode
            );

        *pos_y = pos_y_copy;
        table.output_super_header
            ("Using current charges"
            ,column_cur0_cash_surr_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        *pos_y += table.get_separator_line_height();
        table.output_horz_separator
            (column_guar0_cash_surr_value
            ,column_separator_guar_cur0
            ,*pos_y
            ,output_mode
            );
        table.output_horz_separator
            (column_cur0_cash_surr_value
            ,column_max
            ,*pos_y
            ,output_mode
            );

        // Output the second super header row which is composed of three
        // physical lines.

        // This function outputs all lines of a single header, corresponding to
        // the "Guaranteed" or "Current", "Zero" or not, column and returns the
        // vertical position below the header.
        auto const output_two_column_super_header = [=,&table]
            (base           guar_or_cur
            ,interest_rate  zero_or_not
            ,std::size_t    begin_column
            ) -> int
            {
                std::size_t end_column = begin_column + 2;
                LMI_ASSERT(end_column <= column_max);

                auto y = *pos_y;

                auto const header = get_two_column_header
                    (base_suffix(guar_or_cur) + ir_suffix(zero_or_not)
                    ,base_suffix(guar_or_cur)
                    )
                    ;
                table.output_super_header
                    (interpolate_html(header).as_html()
                    ,begin_column
                    ,end_column
                    ,&y
                    ,output_mode
                    );

                y += table.get_separator_line_height();
                table.output_horz_separator
                    (begin_column
                    ,end_column
                    ,y
                    ,output_mode
                    );

                return y;
            };

        output_two_column_super_header
            (base::guaranteed
            ,interest_rate::zero
            ,column_guar0_cash_surr_value
            );

        output_two_column_super_header
            (base::guaranteed
            ,interest_rate::non_zero
            ,column_guar_cash_surr_value
            );

        output_two_column_super_header
            (base::current
            ,interest_rate::zero
            ,column_cur0_cash_surr_value
            );

        *pos_y = output_two_column_super_header
            (base::current
            ,interest_rate::non_zero
            ,column_cur_cash_surr_value
            );
    }
};

class nasd_basic : public page_with_basic_tabular_report
{
  private:
    std::string get_fixed_page_contents() const override
    {
        return "{{>nasd_basic}}";
    }

    std::string get_two_column_header
        (std::string const& suffix_full
        ,std::string const& suffix_short
        ) const override
    {
        std::ostringstream oss;
        oss
            << "{{InitAnnSepAcctGrossInt_" << suffix_full << "}} "
            << "Assumed Sep Acct\n"
            << "Gross Rate* "
            << "({{InitAnnSepAcctNetInt_" << suffix_full << "}} net)\n"
            << "{{InitAnnGenAcctInt_" << suffix_short << "}} GPA rate"
            ;
        return oss.str();
    }
};

class nasd_supplemental : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_er_gross_payment
        ,column_ee_gross_payment
        ,column_premium_outlay
        ,column_admin_charge
        ,column_premium_tax_load
        ,column_dac_tax_load
        ,column_er_min_premium
        ,column_ee_min_premium
        ,column_net_premium
        ,column_cost_of_insurance_charges
        ,column_cur_account_value
        ,column_cur_cash_surr_value
        ,column_cur_death_benefit
        ,column_max
        };

    std::string get_fixed_page_contents() const override
    {
        return "{{>nasd_supplemental}}";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"           , "Policy\nYear"               ,       "999" }
            ,{ "AttainedAge"          , "End of\nYear Age"           ,       "999" }
            ,{ "ErGrossPmt"           , "ER Gross\nPayment"          ,   "999,999" }
            ,{ "EeGrossPmt"           , "EE Gross\nPayment"          ,   "999,999" }
            ,{ "GrossPmt"             , "Premium\nOutlay"            ,   "999,999" }
            ,{ "PolicyFee_Current"    , "Admin\nCharge"              ,   "999,999" }
            ,{ "PremTaxLoad_Current"  , "Premium\nTax Load"          ,   "999,999" }
            ,{ "DacTaxLoad_Current"   , "DAC\nTax Load"              ,   "999,999" }
            ,{ "ErModalMinimumPremium", "ER Modal\nMinimum\nPremium" ,   "999,999" }
            ,{ "EeModalMinimumPremium", "EE Modal\nMinimum\nPremium" ,   "999,999" }
            ,{ "NetPmt_Current"       , "Net\nPremium"               ,   "999,999" }
            ,{ "COICharge_Current"    , "Cost of\nInsurance\nCharges",   "999,999" }
            ,{ "AcctVal_Current"      , "Current\nAccount\nValue"    ,   "999,999" }
            ,{ "CSVNet_Current"       , "Current\nCash Surr\nValue"  ,   "999,999" }
            ,{ "EOYDeathBft_Current"  , "Current\nDeath\nBenefit"    , "9,999,999" }
            };

        return columns;
    }

    bool should_show_column(Ledger const& ledger, int column) const override
    {
        auto const& invar = ledger.GetLedgerInvariant();

        // The supplemental page in NASD illustrations exists in two versions:
        // default one and one with split premiums. Hide columns that are not
        // needed for the current illustration.
        switch(column)
            {
            case column_end_of_year_age:
                // This column doesn't make sense for composite ledgers.
                return !ledger.is_composite();

            case column_admin_charge:
            case column_premium_tax_load:
            case column_dac_tax_load:
                // These columns only appear in non-split premiums case.
                return invar.SplitMinPrem == 0.;

            case column_er_gross_payment:
            case column_ee_gross_payment:
            case column_er_min_premium:
            case column_ee_min_premium:
                // While those only appear in split premiums case.
                return invar.SplitMinPrem == 1.;

            case column_policy_year:
            case column_premium_outlay:
            case column_net_premium:
            case column_cost_of_insurance_charges:
            case column_cur_account_value:
            case column_cur_cash_surr_value:
            case column_cur_death_benefit:
            case column_max:
                // These columns are common to both cases and never hidden.
                break;
            }

        return true;
    }
};

class nasd_assumption_detail : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_withdrawal
        ,column_loan
        ,column_end_of_year_age
        ,column_sep_acct_crediting_rate
        ,column_gen_acct_crediting_rate
        ,column_m_and_e
        ,column_ee_payment_mode
        ,column_er_payment_mode
        ,column_assumed_loan_interest
        ,column_max
        };

    std::string get_fixed_page_contents() const override
    {
        return "{{>nasd_assumption_detail}}";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"          , "Policy\nYear"                ,        "999" }
            ,{ "NetWD"               , "Withdrawal"                  ,    "999,999" }
            ,{ "NewCashLoan"         , "Loan"                        ,    "999,999" }
            ,{ "AttainedAge"         , "End of\nYear Age"            ,        "999" }
            ,{ "AnnSAIntRate_Current", "Sep Acct Net\nInv Rate"      ,     "99.99%" }
            ,{ "AnnGAIntRate_Current", "Gen Acct\nCurrent Rate"      ,     "99.99%" }
            ,{ "CurrMandE"           , "M&E"                         ,     "99.99%" }
            ,{ "EeMode"              , "Indiv\nPmt Mode"             , "Semiannual" }
            ,{ "ErMode"              , "Corp\nPmt Mode"              , "Semiannual" }
            ,{ "InitAnnLoanDueRate"  , "Assumed\nLoan Interest"      ,     "99.99%" }
            };

        return columns;
    }

    bool should_show_column(Ledger const& ledger, int column) const override
    {
        // This table looks very differently for individual and composite
        // illustrations.
        switch(column)
            {
            case column_withdrawal:
            case column_loan:
                // Shown only for composite illustrations.
                return ledger.is_composite();

            case column_end_of_year_age:
            case column_sep_acct_crediting_rate:
            case column_gen_acct_crediting_rate:
            case column_m_and_e:
            case column_ee_payment_mode:
            case column_er_payment_mode:
            case column_assumed_loan_interest:
                // Shown only for individual ones.
                return !ledger.is_composite();

            case column_policy_year:
            case column_max:
                // Always shown.
                break;
            }

        return true;
    }
};

// NASD illustration.
class pdf_illustration_nasd : public pdf_illustration
{
  public:
    pdf_illustration_nasd
        (Ledger const& ledger
        ,fs::path const& output
        )
        :pdf_illustration(ledger, output)
    {
        auto const& invar = ledger.GetLedgerInvariant();

        // Define variables specific to this illustration.
        if(!invar.ContractName.empty())
            {
            std::string s = invar.ContractName;
            for(auto& c : s)
                {
                c = lmi_tolower(c);
                }
            s[0] = lmi_toupper(s[0]);

            add_variable("ContractNameCap", s);
            }

        add_variable
            ("UWTypeIsGuaranteedIssueInTexasWithFootnote"
            ,invar.UWType == "Guaranteed issue"
            );

        add_variable
            ("HasTermOrSupplSpecAmt"
            ,test_variable("HasTerm") || test_variable("HasSupplSpecAmt")
            );

        auto const& state_abbrev = invar.GetStatePostalAbbrev();
        add_variable
            ("StateIsNewYork"
            ,state_abbrev == "NY"
            );

        // Add all the pages.
        add<cover_page>();
        add<nasd_basic>();
        add<nasd_supplemental>();
        add<standard_page>("nasd_column_headings");
        add<standard_page>("nasd_notes1");
        add<standard_page>("nasd_notes2");
        add<nasd_assumption_detail>();
    }

    std::string get_upper_footer_template_name() const override
    {
        return "nasd_footer_upper";
    }

    std::string get_lower_footer_template_name() const override
    {
        return "nasd_footer_lower";
    }
};

// Basic illustration page of the private group placement illustration.
class reg_d_group_basic : public page_with_basic_tabular_report
{
  private:
    std::string get_fixed_page_contents() const override
    {
        return "{{>reg_d_group_basic}}";
    }

    std::string get_two_column_header
        (std::string const& suffix_full
        ,std::string const& suffix_short
        ) const override
    {
        std::ostringstream oss;
        oss
            << "{{InitAnnSepAcctGrossInt_" << suffix_full << "}} "
            << "Hypothetical Gross\n"
            << "Return ({{InitAnnSepAcctNetInt_" << suffix_full << "}} net)"
            ;
        return oss.str();
    }
};

// Private group placement illustration.
class pdf_illustration_reg_d_group : public pdf_illustration
{
  public:
    pdf_illustration_reg_d_group
        (Ledger const& ledger
        ,fs::path const& output
        )
        :pdf_illustration(ledger, output)
    {
        // Define variables specific to this illustration.
        auto const& invar = ledger.GetLedgerInvariant();

        add_variable
            ("MecYearIs0"
            ,invar.MecYear == 0
            );

        // Add all the pages.
        add<cover_page>();
        add<reg_d_group_basic>();
        add<standard_page>("reg_d_group_column_headings");
        add<standard_page>("reg_d_group_narrative_summary");
        add<standard_page>("reg_d_group_narrative_summary2");
    }

    std::string get_upper_footer_template_name() const override
    {
        return "reg_d_group_footer_upper";
    }

    std::string get_lower_footer_template_name() const override
    {
        return "reg_d_group_footer_lower";
    }
};

// This page exists in two almost identical versions, one using guaranteed and
// the other one using current values, use a base class to share the common
// parts.
class reg_d_individual_irr_base : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_premium_outlay
        ,column_zero_cash_surr_value
        ,column_zero_death_benefit
        ,column_zero_irr_surr_value
        ,column_zero_irr_death_benefit
        ,column_separator
        ,column_nonzero_cash_surr_value
        ,column_nonzero_death_benefit
        ,column_nonzero_irr_surr_value
        ,column_nonzero_irr_death_benefit
        ,column_max
        };

    // Must be overridden to return the base being used.
    virtual base get_base() const = 0;

    bool should_show_column(Ledger const& ledger, int column) const override
    {
        // One column should be hidden for composite ledgers.
        return column != column_end_of_year_age || !ledger.is_composite();
    }

    void render_or_measure_extra_headers
        (illustration_table_generator&  table
        ,html_interpolator const&       interpolate_html
        ,int*                           pos_y
        ,enum_output_mode               output_mode
        ) const override
    {
        std::ostringstream header_zero;
        header_zero
            << "{{InitAnnSepAcctGrossInt_"
            << base_suffix(get_base())
            << ir_suffix(interest_rate::zero)
            << "}} Hypothetical Rate of\n"
            << "Return*"
            ;

        auto pos_y_copy = *pos_y;
        table.output_super_header
            (interpolate_html(header_zero.str()).as_html()
            ,column_zero_cash_surr_value
            ,column_zero_irr_surr_value
            ,pos_y
            ,output_mode
            );

        std::ostringstream header_nonzero;
        header_nonzero
            << "{{InitAnnSepAcctGrossInt_"
            << base_suffix(get_base())
            << ir_suffix(interest_rate::non_zero)
            << "}} Hypothetical Rate of\n"
            << "Return*"
            ;

        *pos_y = pos_y_copy;
        table.output_super_header
            (interpolate_html(header_nonzero.str()).as_html()
            ,column_nonzero_cash_surr_value
            ,column_nonzero_irr_surr_value
            ,pos_y
            ,output_mode
            );

        *pos_y += table.get_separator_line_height();
        table.output_horz_separator
            (column_zero_cash_surr_value
            ,column_zero_irr_surr_value
            ,*pos_y
            ,output_mode
            );
        table.output_horz_separator
            (column_nonzero_cash_surr_value
            ,column_nonzero_irr_surr_value
            ,*pos_y
            ,output_mode
            );
    }
};

class reg_d_individual_guar_irr : public reg_d_individual_irr_base
{
  private:
    base get_base() const override
    {
        return base::guaranteed;
    }

    std::string get_fixed_page_contents() const override
    {
        return "{{>reg_d_individual_guar_irr}}";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"       ,       "999" }
            ,{ "AttainedAge"                , "End of\nYear Age"   ,       "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"    ,   "999,999" }
            ,{ "CSVNet_GuaranteedZero"      , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_GuaranteedZero" , "Death\nBenefit"     , "9,999,999" }
            ,{ "IrrCsv_GuaranteedZero"      , "IRR on\nSurr Value" ,    "99.99%" }
            ,{ "IrrDb_GuaranteedZero"       , "IRR on\nDeath Bft"  ,    "99.99%" }
            ,{ ""                           , " "                  ,         "-" }
            ,{ "CSVNet_Guaranteed"          , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_Guaranteed"     , "Death\nBenefit"     , "9,999,999" }
            ,{ "IrrCsv_Guaranteed"          , "IRR on\nSurr Value" ,    "99.99%" }
            ,{ "IrrDb_Guaranteed"           , "IRR on\nDeath Bft"  ,    "99.99%" }
            };

        return columns;
    }
};

class reg_d_individual_cur_irr : public reg_d_individual_irr_base
{
  private:
    base get_base() const override
    {
        return base::current;
    }

    std::string get_fixed_page_contents() const override
    {
        return "{{>reg_d_individual_cur_irr}}";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"       ,       "999" }
            ,{ "AttainedAge"                , "End of\nYear Age"   ,       "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"    ,   "999,999" }
            ,{ "CSVNet_CurrentZero"         , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_CurrentZero"    , "Death\nBenefit"     , "9,999,999" }
            ,{ "IrrCsv_CurrentZero"         , "IRR on\nSurr Value" ,    "99.99%" }
            ,{ "IrrDb_CurrentZero"          , "IRR on\nDeath Bft"  ,    "99.99%" }
            ,{ ""                           , " "                  ,         "-" }
            ,{ "CSVNet_Current"             , "Cash Surr\nValue"   ,   "999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"     , "9,999,999" }
            ,{ "IrrCsv_Current"             , "IRR on\nSurr Value" ,    "99.99%" }
            ,{ "IrrDb_Current"              , "IRR on\nDeath Bft"  ,    "99.99%" }
            };

        return columns;
    }
};

// Private individual placement illustration.
class pdf_illustration_reg_d_individual : public pdf_illustration
{
  public:
    pdf_illustration_reg_d_individual
        (Ledger const& ledger
        ,fs::path const& output
        )
        :pdf_illustration(ledger, output)
    {
        // Define variables specific to this illustration.
        add_abbreviated_variable("CorpName", 140);
        add_abbreviated_variable("Insured1", 140);

        // Add all the pages.
        add<standard_page>("reg_d_individual_cover_page");
        add<reg_d_individual_guar_irr>();
        add<reg_d_individual_cur_irr>();
    }

    std::string get_upper_footer_template_name() const override
    {
        return "reg_d_individual_footer_upper";
    }

    std::string get_lower_footer_template_name() const override
    {
        return "reg_d_individual_footer_lower";
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
            pdf_ill = std::make_unique<pdf_illustration_regular>(ledger, output);
            break;
        case mce_nasd:
            pdf_ill = std::make_unique<pdf_illustration_nasd>(ledger, output);
            break;
        case mce_group_private_placement:
            pdf_ill = std::make_unique<pdf_illustration_reg_d_group>(ledger, output);
            break;
        case mce_individual_private_placement:
            pdf_ill = std::make_unique<pdf_illustration_reg_d_individual>(ledger, output);
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
