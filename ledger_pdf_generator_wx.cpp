// Generate PDF files with ledger data using wxPdfDocument library.
//
// Copyright (C) 2017, 2018 Gregory W. Chicares.
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
#include "bourn_cast.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "force_linking.hpp"
#include "html.hpp"
#include "icon_monger.hpp"              // load_image()
#include "interpolate_string.hpp"
#include "istream_to_string.hpp"
#include "ledger.hpp"
#include "ledger_evaluator.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"               // lmi_tolower(), page_count()
#include "oecumenic_enumerations.hpp"
#include "pdf_writer_wx.hpp"
#include "ssize_lmi.hpp"
#include "wx_table_generator.hpp"

#include <wx/pdfdc.h>

#include <wx/utils.h>                   // wxBusyCursor

#include <wx/html/m_templ.h>

#include <array>
#include <cstddef>                      // size_t
#include <cstdint>                      // SIZE_MAX
#include <cstdlib>                      // strtoul()
#include <exception>                    // uncaught_exceptions()
#include <fstream>
#include <map>
#include <memory>                       // make_unique(), shared_ptr, unique_ptr
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>                      // forward(), move()
#include <vector>

LMI_FORCE_LINKING_IN_SITU(ledger_pdf_generator_wx)

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

// Helper enums identifying the possible {Guaranteed,Current}{Zero,}
// combinations.
enum class basis
    {guaranteed
    ,current
    };

enum class interest_rate
    {zero
    ,non_zero
    };

// And functions to retrieve their string representation.
std::string basis_suffix(basis guar_or_curr)
{
    switch(guar_or_curr)
        {
        case basis::guaranteed: return "Guaranteed";
        case basis::current:    return "Current"   ;
        }
    throw "Unreachable--unknown basis value";
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
        :evaluator_ {evaluator}
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
    //
    // The variable names recognized by this function are either those defined
    // by ledger_evaluator, i.e. scalar and vector fields of the ledger, or any
    // variables explicitly defined by add_variable() calls.
    html::text operator()(char const* s) const
    {
        return html::text::from_html
            (interpolate_string
                (s
                ,[this]
                    (std::string const& str
                    ,interpolate_lookup_kind kind
                    )
                    {
                        return interpolation_func(str, kind);
                    }
                )
            );
    }

    html::text operator()(std::string const& s) const
    {
        return (*this)(s.c_str());
    }

    // Add a variable, providing either its raw text or already escaped HTML
    // representation. Boolean values are converted to strings "0" or "1" as
    // expected.
    void add_variable(std::string const& name, html::text const& value)
    {
        vars_[name] = value;
    }

    void add_variable(std::string const& name, std::string const& value)
    {
        add_variable(name, html::text::from(value));
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

    // Interpolate the contents of the given external template.
    //
    // This is exactly the same as interpolating "{{>template_name}}" string
    // but a bit more convenient to use and simpler to read.
    html::text expand_template(std::string const& template_name) const
    {
        return (*this)("{{>" + template_name + "}}");
    }

  private:
    // The expansion function used with interpolate_string().
    html::text expand_html(std::string const& s) const
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
            if(stop != s.c_str() + s.length() - 1 || SIZE_MAX <= index)
                {
                throw std::runtime_error
                    ("Index of vector variable '" + s + "' is not a valid number"
                    );
                }

            // Cast below is valid because of the check for overflow above.
            return html::text::from
                (evaluator_
                    (s.substr(0, open_pos)
                    ,static_cast<std::size_t>(index)
                    )
                );
            }

        return html::text::from(evaluator_(s));
    }

    std::string load_partial_from_file(std::string const& file) const
    {
        std::ifstream ifs(AddDataDir(file + ".mst"));
        if(!ifs)
            {
            alarum()
                << "Template file \""
                << file
                << ".mst\" not found."
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
    std::map<std::string, html::text> vars_;
};

// A helper mix-in class for pages using tables which is also reused by the
// custom wxHtmlCell showing a table.
//
// Derived classes must provide get_table_columns() and may also override
// should_hide_column() to hide some of these columns dynamically and then can
// use create_table_generator() to obtain the generator object that can be used
// to render a table with the specified columns.
class using_illustration_table
{
  protected:
    // Description of a single table column.
    struct illustration_table_column
    {
        std::string const        variable_name;
        std::string const        header;
        std::string const        widest_text;
        mutable oenum_visibility visibility {oe_shown};
    };

    using illustration_table_columns = std::vector<illustration_table_column>;

    // Must be overridden to return the description of the table columns.
    virtual illustration_table_columns const& get_table_columns() const = 0;

    // PDF !! Most overrides have exactly this function body:
    //    {
    //    // Don't show AttainedAge on a composite.
    //    return ledger.is_composite() && column == column_end_of_year_age;
    //    }
    // However, that cannot be written here, once and only once,
    // because 'column_end_of_year_age' is an enumerator whose value
    // may differ in each derived class.
    virtual bool should_hide_column(Ledger const& ledger, int column) const
    {
        stifle_warning_for_unused_value(ledger);
        stifle_warning_for_unused_value(column);
        return false;
    }

    // Useful helper for creating the table generator using the columns defined
    // by the separate (and simpler to implement) get_table_columns() pure
    // virtual method.
    wx_table_generator create_table_generator
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ) const
    {
        std::vector<column_parameters> vc;
        std::vector<int> indices;
        int column = 0;
        for(auto const& i : get_table_columns())
            {
            indices.push_back(lmi::ssize(vc));
            if(!should_hide_column(ledger, column))
                {
                vc.push_back({i.header, i.widest_text, oe_right, oe_inelastic});
                }
            ++column;
            }
        // Add a one-past-the-end index equal to the last value, because
        // some member functions of class wx_table_generator expect it.
        indices.push_back(lmi::ssize(vc));

        // Arguably, should_hide_column() should return an enumerator--see:
        //   https://lists.nongnu.org/archive/html/lmi/2018-05/msg00026.html

        // Set the smaller font used for all tables before creating the table
        // generator which uses the DC font for its measurements.
        auto& pdf_dc = writer.dc();
        auto font = pdf_dc.GetFont();
        font.SetPointSize(9);
        pdf_dc.SetFont(font);

        return wx_table_generator
            (illustration_style_tag{}
            ,vc
            ,indices
            ,writer.dc()
            ,writer.get_horz_margin()
            ,writer.get_page_width()
            );
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
        ,int view_y2
        ,wxHtmlRenderingInfo& info
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

// Define scaffolding for a custom HTML "img" tag which must be used
// instead of the standard one in order to allow specifying the scaling factor
// that we want to use for the image in the PDF. Unfortunately this can't be
// achieved by simply using "width" and/or "height" attributes of the "img" tag
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
        :image_        {image}
        ,src_          {src}
        ,scale_factor_ {scale_factor}
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
        ,int view_y2
        ,wxHtmlRenderingInfo& info
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

// Note that defining this handler replaces the standard <img> tag handler
// defined in wxHTML itself, which also handles <map> and <area> tags, but as
// we don't use either of those and all our images are scaled, this is fine.
TAG_HANDLER_BEGIN(scaled_image, "IMG")
    TAG_HANDLER_PROC(tag)
    {
        wxString src;
        if(!tag.GetParamAsString("SRC", &src))
            {
            throw std::runtime_error
                ("missing mandatory \"src\" attribute of \"img\" tag"
                );
            }

        // The scale factor is optional.
        double scale_factor = 1.0;

        // But if it is given, we currently specify its inverse in HTML just
        // because it so happens that for the scale factors we use the inverse
        // can be expressed exactly in decimal notation, while the factor
        // itself can't. In principle, the converse could also happen and we
        // might add support for "factor" attribute too in this case. Or we
        // could use separate "numerator" and "denominator" attributes. But for
        // now implement just the bare minimum of what we need.
        wxString inv_factor_str;
        if(tag.GetParamAsString("INV_FACTOR", &inv_factor_str))
            {
            double inv_factor = 0.0;
            if(!inv_factor_str.ToCDouble(&inv_factor) || inv_factor == 0.0)
                {
                throw std::runtime_error
                    ( "invalid value for \"inv_factor\" attribute of "
                      "\"img\" tag: \""
                    + inv_factor_str.ToStdString()
                    + "\""
                    );
                }

            scale_factor = 1.0 / inv_factor;
            }

        wxImage image(load_image(src.c_str()));
        if(image.IsOk())
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

// Base class for all logical illustration pages.
//
// A single logical page may result in multiple physical pages of output, e.g.
// if it contains a table not fitting on one page, but mostly these page
// objects correspond to a single physical page of the resulting illustration.
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
    // illustration. This method is supposed to be called only once and only by
    // pdf_illustration this page is being added to.
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
    // The associated illustration, which will be non-null by the time our
    // virtual methods such as pre_render() and render() are called.
    pdf_illustration const* illustration_ = nullptr;
};

// Base class for the different kinds of illustrations.
//
// This object contains pages, added to it using its add() method, as well as
// illustration-global data registered as variables with html_interpolator and
// so available for the pages when expanding the external templates defining
// their contents.
class pdf_illustration : protected html_interpolator
{
  public:
    explicit pdf_illustration(Ledger const& ledger)
        :html_interpolator {ledger.make_evaluator()}
        ,ledger_           {ledger}
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

    // Render all pages to the specified PDF file.
    void render_all(fs::path const& output)
    {
        // Use non-default font sizes that are used to make the new
        // illustrations more similar to the previously existing ones.
        pdf_writer_wx writer
            (output.string()
            ,wxPORTRAIT
            ,{8, 9, 10, 12, 14, 18, 20}
            );

        html_cell_for_pdf_output::pdf_context_setter
            set_pdf_context(ledger_, writer, *this);

        for(auto const& i : pages_)
            {
            i->pre_render(ledger_, writer, *this);
            }

        bool first = true;
        for(auto const& i : pages_)
            {
            if(first)
                {
                // We shouldn't start a new page before the very first one.
                first = false;
                }
            else
                {
                // Do start a new physical page before rendering all the
                // subsequent pages (notice that a page is also free to call
                // next_page() from its render()).
                writer.next_page();
                }

            i->render(ledger_, writer, *this);
            }

        writer.save();
    }

    // Methods to be implemented by the derived classes to indicate which
    // templates should be used for the upper (above the separating line) and
    // the lower parts of the footer. The upper template name may be empty if
    // it is not used at all.
    //
    // Notice that the upper footer template name can be overridden at the page
    // level, the methods here define the default for all illustration pages.
    //
    // These methods are used by the pages deriving from page_with_footer.
    virtual std::string get_upper_footer_template_name() const = 0;
    virtual std::string get_lower_footer_template_name() const = 0;

  protected:
    // Explicitly retrieve the base class.
    html_interpolator const& get_interpolator() const {return *this;}

    // Helper for abbreviating a string to at most the given length (in bytes).
    static std::string abbreviate_if_necessary(std::string s, std::size_t len)
    {
        if(len < s.length() && 3 < len)
            {
            s.replace(len - 3, std::string::npos, "...");
            }

        return s;
    }

    // Helper for creating abbreviated variables in the derived classes: such
    // variables have the name based on the name of the original variable with
    // "Abbrev" and "len" appended to it and their value is at most "len" bytes
    // long.
    void add_abbreviated_variable(std::string const& var, std::size_t len)
    {
        add_variable
            (var + "Abbrev" + std::to_string(len)
            ,abbreviate_if_necessary(evaluate(var), len)
            );
    }

  private:
    // Define variables that can be used when interpolating pages contents.
    void init_variables()
    {
        // The variables defined here are used by all, or at least more than
        // one, illustration kinds. Variables only used in the templates of a
        // single illustration type should be defined in the corresponding
        // derived pdf_illustration_xxx class instead.

        add_variable
            ("date_prepared"
            , html::text::from(evaluate("PrepMonth"))
            + html::text::nbsp()
            + html::text::from(evaluate("PrepDay"))
            + html::text::from(", ")
            + html::text::from(evaluate("PrepYear"))
            );

        auto indent = html::text::nbsp();
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

        std::size_t const full_abbrev_length = 30;
        add_abbreviated_variable("MasterContractNumber", full_abbrev_length);
        add_abbreviated_variable("MasterContractNumber", full_abbrev_length / 2);
        add_abbreviated_variable("ContractNumber", full_abbrev_length);
        add_abbreviated_variable("ContractNumber", full_abbrev_length / 2);

        add_variable
            ("HasComplianceTrackingNumber"
            ,expand_template("imprimatur")
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

        // PDF !! Conditions of this ilk should become distinct entities in
        // the product files.
        auto const& policy_name = invar.PolicyLegalName;
        add_variable
            ("GroupCarveout"
            ,    policy_name == "Group Flexible Premium Adjustable Life Insurance Certificate"
              || policy_name == "Group Flexible Premium Variable Adjustable Life Insurance Certificate"
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

    // Source of the data.
    Ledger const& ledger_;

    // All the pages of this illustration.
    std::vector<std::unique_ptr<page>> pages_;
};

// Cover page used by several different illustration kinds.
class cover_page : public page
{
  public:
    void render
        (Ledger const& /* ledger */
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        int const height_contents = writer.output_html
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,interpolate_html.expand_template("cover")
            );

        // There is no way to draw a border around the page contents in wxHTML
        // currently, so do it manually.
        auto& pdf_dc = writer.dc();

        pdf_dc.SetPen(wxPen(illustration_rule_color, 2));
        pdf_dc.SetBrush(*wxTRANSPARENT_BRUSH);

        pdf_dc.DrawRectangle
            (writer.get_horz_margin()
            ,writer.get_vert_margin()
            ,writer.get_page_width()
            ,height_contents
            );
    }
};

// Base class for all pages with a footer.
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
            ,oe_only_measure
            );

        auto const& upper_template = get_upper_footer_template_name();
        if(!upper_template.empty())
            {
            footer_height += writer.output_html
                (frame_horz_margin
                ,0
                ,frame_width
                ,interpolate_html.expand_template(upper_template)
                ,oe_only_measure
                );

            // Leave a gap between the upper part of the footer and the main
            // page contents to separate them in absence of a separator line
            // which delimits the lower part.
            footer_height += writer.dc().GetCharHeight();
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

        auto& pdf_dc = writer.dc();

        auto y = footer_top_;

        auto const& upper_template = get_upper_footer_template_name();
        if(!upper_template.empty())
            {
            y += pdf_dc.GetCharHeight();

            y += writer.output_html
                (frame_horz_margin
                ,y
                ,frame_width
                ,interpolate_html.expand_template(upper_template)
                );
            }

        writer.output_html
            (frame_horz_margin
            ,y
            ,frame_width
            ,get_footer_lower_html(interpolate_html)
            );

        pdf_dc.SetPen(illustration_rule_color);
        pdf_dc.DrawLine
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

    // This method forwards to the illustration by default, but can be
    // overridden to define a page-specific footer if necessary.
    virtual std::string get_upper_footer_template_name() const
    {
        return illustration_->get_upper_footer_template_name();
    }

    // This method uses get_page_number() and returns the HTML wrapping it
    // and other fixed information appearing in the lower part of the footer.
    html::text get_footer_lower_html(html_interpolator const& interpolate_html) const
    {
        auto const page_number_str = get_page_number();

        auto const templ = illustration_->get_lower_footer_template_name();

        // Use our own interpolation function to handle the special
        // "page_number" variable that is replaced with the actual
        // (possibly dynamic) page number.
        return html::text::from_html
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

// Base class for all pages showing the page number in the footer.
//
// In addition to actually providing page_with_footer with the correct string
// to show in the footer, this class implicitly handles the page count by
// incrementing it whenever a new object of this class is pre-rendered.
class numbered_page : public page_with_footer
{
  public:
    // Must be called before creating the first numbered page.
    static void start_numbering()
    {
        last_page_number_ = 0;
    }

    numbered_page()
    {
        // This assert would fail if start_numbering() hadn't been called
        // before creating a numbered page, as it should be.
        LMI_ASSERT(0 <= last_page_number_);
    }

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

        LMI_ASSERT(0 <= extra_pages_);

        last_page_number_ += extra_pages_;
    }

    ~numbered_page() override
    {
        // Check that next_page() was called the expected number of times,
        // unless we're unwinding the stack due to some other error, in which
        // case it is normal that extra pages haven't been generated.
        //
        // Notice that we shouldn't use LMI_ASSERT() in the dtor by default,
        // and it's better to use warning() instead of using noexcept(false).
        if(extra_pages_ && !std::uncaught_exceptions())
            {
            warning()
                << "Logic error: there should have been "
                << extra_pages_
                << " more page(s) after the page "
                << this_page_number_
                << LMI_FLUSH
                ;
            }
    }

  protected:
    void next_page(pdf_writer_wx& writer)
    {
        // This method may only be called if we had reserved enough physical
        // pages for this logical pages by overriding get_extra_pages_needed().
        LMI_ASSERT(0 < extra_pages_);

        writer.next_page();

        ++this_page_number_;
        --extra_pages_;
    }

  private:
    // Derived classes may override this method if they may need more than one
    // physical page to show their contents.
    virtual int get_extra_pages_needed
        (Ledger const&              ledger
        ,pdf_writer_wx&             writer
        ,html_interpolator const&   interpolate_html
        ) = 0;

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

// Initial value is invalid, use start_numbering() to change it.
int numbered_page::last_page_number_ = -1;

// Simplest possible page which is entirely defined by its external template
// whose name must be specified when constructing it.
class standard_page : public numbered_page
{
  public:
    // Accept only string literals as template names, there should be no need
    // to use anything else.
    template<int N>
    explicit standard_page(char const (&page_template_name)[N])
        :page_template_name_ {page_template_name}
    {
    }

    void render
        (Ledger const& ledger
        ,pdf_writer_wx& writer
        ,html_interpolator const& interpolate_html
        ) override
    {
        // Page HTML must have been already set by get_extra_pages_needed().
        LMI_ASSERT(!page_html_.empty());

        int last_page_break = 0;
        for(auto const& page_break : page_break_positions_)
            {
            if(last_page_break != 0)
                {
                next_page(writer);
                }

            numbered_page::render(ledger, writer, interpolate_html);

            writer.output_html
                (writer.get_horz_margin()
                ,writer.get_vert_margin()
                ,writer.get_page_width()
                ,page_html_
                ,last_page_break
                ,page_break
                );

            last_page_break = page_break;
            }
    }

  private:
    int get_extra_pages_needed
        (Ledger const&              /* ledger */
        ,pdf_writer_wx&             writer
        ,html_interpolator const&   interpolate_html
        ) override
    {
        page_html_ = wxString::FromUTF8
            (interpolate_html.expand_template(page_template_name_).as_html()
            );

        page_break_positions_ = writer.paginate_html
            (writer.get_page_width()
            ,get_footer_top() - writer.get_vert_margin()
            ,page_html_
            );

        // The cast is safe, we're never going to have more than INT_MAX
        // pages and if we, somehow, do, the caller checks that this function
        // returns a positive value.
        return static_cast<int>(page_break_positions_.size()) - 1;
    }

    char const* const page_template_name_;
    wxString          page_html_;
    std::vector<int>  page_break_positions_;
};

// Helper classes used to show the numeric summary table. The approach used
// here is to define a custom HTML tag (<numeric_summary_table>) and use the
// existing wx_table_generator to replace it with the actual table when
// rendering.
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
        m_Height = render_or_measure(0, oe_only_measure);
    }

    // Override the base class method to actually render the table.
    void Draw
        (wxDC& dc
        ,int x
        ,int y
        ,int view_y1
        ,int view_y2
        ,wxHtmlRenderingInfo& info
        ) override
    {
        draw_check_precondition(dc, view_y1, view_y2, info);

        // We ignore the horizontal coordinate which is always 0 for this cell
        // anyhow.
        stifle_warning_for_unused_value(x);

        render_or_measure(y + m_PosY, oe_render);
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
        ,column_separator_mid_curr
        ,column_curr_account_value
        ,column_curr_cash_surr_value
        ,column_curr_death_benefit
        ,column_max
        };

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "AcctVal_Guaranteed"         , "Account\nValue"              , "999,999,999" }
            ,{ "CSVNet_Guaranteed"          , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Guaranteed"     , "Death\nBenefit"              , "999,999,999" }
            ,{ ""                           , " "                           ,           "-" }
            ,{ "AcctVal_Midpoint"           , "Account\nValue"              , "999,999,999" }
            ,{ "CSVNet_Midpoint"            , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Midpoint"       , "Death\nBenefit"              , "999,999,999" }
            ,{ ""                           , " "                           ,           "-" }
            ,{ "AcctVal_Current"            , "Account\nValue"              , "999,999,999" }
            ,{ "CSVNet_Current"             , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"              , "999,999,999" }
            };

        return columns;
    }

    int render_or_measure(int pos_y, oenum_render_or_only_measure output_mode)
    {
        auto const& ledger = pdf_context_for_html_output.ledger();
        auto& writer = pdf_context_for_html_output.writer();

        wx_table_generator table_gen{create_table_generator(ledger, writer)};

        // Output multiple rows of headers.

        // Make a copy because we want pos_y to be modified only once, not
        // twice, by both output_super_header() calls.
        auto pos_y_copy = pos_y;
        table_gen.output_super_header
            ("Guaranteed Values"
            ,column_guar_account_value
            ,column_separator_guar_non_guar
            ,pos_y_copy
            ,output_mode
            );
        table_gen.output_super_header
            ("Non-Guaranteed Values"
            ,column_mid_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_guar_account_value
            ,column_separator_guar_non_guar
            ,pos_y
            ,output_mode
            );
        table_gen.output_horz_separator
            (column_mid_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );
        pos_y += table_gen.separator_line_height();

        pos_y_copy = pos_y;
        table_gen.output_super_header
            ("Midpoint Values"
            ,column_mid_account_value
            ,column_separator_mid_curr
            ,pos_y_copy
            ,output_mode
            );

        table_gen.output_super_header
            ("Current Values"
            ,column_curr_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_mid_account_value
            ,column_separator_mid_curr
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_curr_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );
        pos_y += table_gen.separator_line_height();

        table_gen.output_headers(pos_y, output_mode);

        table_gen.output_horz_separator(0, column_max, pos_y, output_mode);
        pos_y += table_gen.separator_line_height();

        // And now the table values themselves.
        auto const& columns = get_table_columns();

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
                case oe_only_measure:
                    {
                    pos_y += table_gen.row_height();
                    }
                    break;

                case oe_render:
                    {
                    std::vector<std::string> visible_values;
                    for(int j = 0; j < lmi::ssize(columns); ++j)
                        {
                        columns[j].visibility =
                            should_hide_column(ledger, j)
                            ? oe_hidden
                            : oe_shown
                            ;

                        if(oe_shown == columns[j].visibility)
                            {
                            std::string output_value;
                            if(is_last_row && column_policy_year == j)
                                {
                                // Other rows are for given durations, but the
                                // last row is for a given age (typically 70).
                                std::ostringstream oss;
                                oss << "Age " << age_last;
                                output_value = oss.str();
                                }
                            else if(columns[j].variable_name.empty())
                                {
                                ; // Separator column: use empty string.
                                }
                            else
                                {
                                output_value = interpolate_html.evaluate
                                    (columns[j].variable_name
                                    ,year - 1
                                    );
                                }

                            visible_values.push_back(output_value);
                            }
                        }

                    table_gen.output_row(pos_y, visible_values);
                    }
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

// Custom handler for <p> tags preventing the page breaks inside them.
TAG_HANDLER_BEGIN(unbreakable_paragraph, "P")
    TAG_HANDLER_PROC(tag)
    {
        m_WParser->CloseContainer();
        auto const container = m_WParser->OpenContainer();

        // This is the reason for this handler existence: mark the container
        // used for the paragraph contents as being unbreakable.
        container->SetCanLiveOnPagebreak(false);

        // This code reproduces what the standard "P" handler does.
        // Unfortunately there is no way to just delegate to it from here.
        container->SetIndent(m_WParser->GetCharHeight(), wxHTML_INDENT_TOP);
        container->SetAlign(tag);

        // Don't stop parsing, continue with the tag contents.
        return false;
    }
TAG_HANDLER_END(unbreakable_paragraph)

TAGS_MODULE_BEGIN(lmi_illustration)
    TAGS_MODULE_ADD(scaled_image)
    TAGS_MODULE_ADD(numeric_summary_table)
    TAGS_MODULE_ADD(unbreakable_paragraph)
TAGS_MODULE_END(lmi_illustration)

// Numeric summary page is used on its own, as a regular page, but also as the
// base class for ill_reg_numeric_summary_attachment below, which is exactly
// the same page, but appearing as an attachment at the end of the document.
class ill_reg_numeric_summary_page : public standard_page
{
  public:
    ill_reg_numeric_summary_page()
        :standard_page("ill_reg_numeric_summary")
    {
    }
};

class ill_reg_numeric_summary_attachment : public ill_reg_numeric_summary_page
{
  private:
    std::string get_page_number() const override
    {
        return "Attachment";
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

        wx_table_generator table_gen{create_table_generator(ledger, writer)};

        auto const& columns = get_table_columns();

        // Just some cached values used inside the loop below.
        auto const row_height = table_gen.row_height();
        auto const page_bottom = get_footer_top();
        auto const rows_per_group = wx_table_generator::rows_per_group;

        // The table may need several pages, loop over them.
        int const year_max = ledger.GetMaxLength();
        for(int year = 0; year < year_max; )
            {
            int pos_y = render_or_measure_fixed_page_part
                (table_gen
                ,writer
                ,interpolate_html
                ,oe_render
                );

            for(;;)
                {
                std::vector<std::string> visible_values;
                for(int j = 0; j < lmi::ssize(columns); ++j)
                    {
                    columns[j].visibility =
                        should_hide_column(ledger, j)
                        ? oe_hidden
                        : oe_shown
                        ;

                    if(oe_shown == columns[j].visibility)
                        {
                        std::string output_value;
                        if(columns[j].variable_name.empty())
                            {
                            ; // Separator column: use empty string.
                            }
                        else
                            {
                            output_value = interpolate_html.evaluate
                                (columns[j].variable_name
                                ,year
                                );
                            }

                        visible_values.push_back(output_value);
                        }
                    }

                table_gen.output_row(pos_y, visible_values);

                ++year;
                if(year == year_max)
                    {
                    // We will also leave the outer loop.
                    break;
                    }

                if(year % rows_per_group == 0)
                    {
                    // We need a group break.
                    pos_y += row_height;

                    // And possibly a page break, which will be necessary if we
                    // don't have enough space for another group because we
                    // don't want to have page breaks in the middle of a group.
                    auto rows_in_next_group = rows_per_group;
                    if(year_max - year < rows_per_group)
                        {
                        // The next group is the last one and will be incomplete.
                        rows_in_next_group = year_max - year;
                        }

                    if(page_bottom - rows_in_next_group * row_height < pos_y)
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
    // Must be overridden to return the template containing the fixed page part.
    virtual std::string get_fixed_page_contents_template_name() const = 0;

    // May be overridden to render (only if output_mode is oe_render)
    // the extra headers just above the regular table headers.
    //
    // If this function does anything, it must show the first super-header at
    // pos_y and update it to account for the added lines. The base class
    // version does nothing.
    virtual void render_or_measure_extra_headers
        (wx_table_generator&          table_gen
        ,html_interpolator const&     interpolate_html
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode
        ) const
    {
        stifle_warning_for_unused_value(table_gen);
        stifle_warning_for_unused_value(interpolate_html);
        stifle_warning_for_unused_value(pos_y);
        stifle_warning_for_unused_value(output_mode);
    }

  private:
    // Render (only if output_mode is oe_render) the fixed page part and
    // (in any case) return the vertical coordinate of its bottom, where the
    // tabular report starts.
    int render_or_measure_fixed_page_part
        (wx_table_generator&          table_gen
        ,pdf_writer_wx&               writer
        ,html_interpolator const&     interpolate_html
        ,oenum_render_or_only_measure output_mode
        ) const
    {
        int pos_y = writer.get_vert_margin();

        pos_y += writer.output_html
            (writer.get_horz_margin()
            ,pos_y
            ,writer.get_page_width()
            ,interpolate_html.expand_template
                (get_fixed_page_contents_template_name()
                )
            ,output_mode
            );

        render_or_measure_extra_headers
            (table_gen
            ,interpolate_html
            ,pos_y
            ,output_mode
            );

        table_gen.output_headers(pos_y, output_mode);

        auto const ncols = get_table_columns().size();
        table_gen.output_horz_separator(0, ncols, pos_y, output_mode);
        pos_y += table_gen.separator_line_height();

        return pos_y;
    }

    // Override the base class method as the table may overflow onto the next
    // page(s).
    int get_extra_pages_needed
        (Ledger const&              ledger
        ,pdf_writer_wx&             writer
        ,html_interpolator const&   interpolate_html
        ) override
    {
        wx_table_generator table_gen{create_table_generator(ledger, writer)};

        int const pos_y = render_or_measure_fixed_page_part
            (table_gen
            ,writer
            ,interpolate_html
            ,oe_only_measure
            );

        int const rows_per_page = (get_footer_top() - pos_y) / table_gen.row_height();

        int const rows_per_group = wx_table_generator::rows_per_group;

        if(rows_per_page < rows_per_group)
            {
            // We can't afford to continue in this case as we can never output
            // the table as the template simply doesn't leave enough space for
            // it on the page.
            throw std::runtime_error("no space left for tabular report");
            }

        // We return the number of extra pages only, hence -1.
        return page_count
            (ledger.GetMaxLength()
            ,rows_per_group
            ,rows_per_page
            ) - 1;
    }
};

class ill_reg_tabular_detail_page : public page_with_tabular_report
{
  private:
    // PDF !! This derived class and its siblings each contain an enum
    // like the following. Most of the enumerators are unused. They
    // index the container returned by get_table_columns(), and must
    // be maintained in parallel with it so that the two lists match
    // perfectly.
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_premium_outlay
        ,column_guar_account_value
        ,column_guar_cash_surr_value
        ,column_guar_death_benefit
        ,column_dummy_separator
        ,column_curr_account_value
        ,column_curr_cash_surr_value
        ,column_curr_death_benefit
        ,column_max
        };

    std::string get_fixed_page_contents_template_name() const override
    {
        return "ill_reg_tabular_details";
    }

    std::string get_upper_footer_template_name() const override
    {
        return "ill_reg_footer_disclaimer";
    }

    void render_or_measure_extra_headers
        (wx_table_generator&          table_gen
        ,html_interpolator const&     interpolate_html
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode
        ) const override
    {
        stifle_warning_for_unused_value(interpolate_html);

        // Make a copy because we want the real pos_y to be modified only once,
        // not twice, by both output_super_header() calls.
        auto pos_y_copy = pos_y;
        table_gen.output_super_header
            ("Guaranteed Values"
            ,column_guar_account_value
            ,column_dummy_separator
            ,pos_y_copy
            ,output_mode
            );
        table_gen.output_super_header
            ("Non-Guaranteed Values"
            ,column_curr_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_guar_account_value
            ,column_dummy_separator
            ,pos_y
            ,output_mode
            );
        table_gen.output_horz_separator
            (column_curr_account_value
            ,column_max
            ,pos_y
            ,output_mode
            );
        pos_y += table_gen.separator_line_height();
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "AcctVal_Guaranteed"         , "Account\nValue"              , "999,999,999" }
            ,{ "CSVNet_Guaranteed"          , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Guaranteed"     , "Death\nBenefit"              , "999,999,999" }
            ,{ ""                           , " "                           ,        "----" }
            ,{ "AcctVal_Current"            , "Account\nValue"              , "999,999,999" }
            ,{ "CSVNet_Current"             , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"              , "999,999,999" }
            };

        return columns;
    }

    bool should_hide_column(Ledger const& ledger, int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger.is_composite() && column == column_end_of_year_age;
    }
};

class ill_reg_tabular_detail2_page : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_ill_crediting_rate
        ,column_selected_face_amount
        ,column_max
        };

    std::string get_fixed_page_contents_template_name() const override
    {
        return "ill_reg_tabular_details2";
    }

    std::string get_upper_footer_template_name() const override
    {
        return "ill_reg_footer_disclaimer";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "AnnGAIntRate_Current"       , "Illustrated\nCrediting Rate" ,      "99.99%" }
            ,{ "SpecAmt"                    , "Selected\nFace Amount"       , "999,999,999" }
            };

        return columns;
    }

    bool should_hide_column(Ledger const& ledger, int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger.is_composite() && column == column_end_of_year_age;
    }
};

// Class for pages showing supplemental report after the fixed template
// contents. It can be either used directly or further derived from, e.g. to
// override some of its inherited virtual methods such as
// get_upper_footer_template_name() as done below.
class standard_supplemental_report : public page_with_tabular_report
{
  public:
    explicit standard_supplemental_report
        (html_interpolator const& interpolate_html
        ,std::string       const& page_template
        )
        :columns_       {build_columns(interpolate_html)}
        ,page_template_ {page_template}
    {
    }

  private:
    illustration_table_columns const& get_table_columns() const override
    {
        return columns_;
    }

    std::string get_fixed_page_contents_template_name() const override
    {
        return page_template_;
    }

    // Helper function used by the ctor to initialize the const columns_ field.
    illustration_table_columns build_columns
        (html_interpolator const& interpolate_html
        )
    {
        constexpr std::size_t max_columns = 12;
        std::string const empty_column_name("[none]");

        illustration_table_columns columns;
        for(std::size_t i = 0; i < max_columns; ++i)
            {
            auto name = interpolate_html.evaluate("SupplementalReportColumnsNames", i);
            if(name != empty_column_name)
                {
                columns.emplace_back
                    (illustration_table_column
                        {std::move(name)
                        ,interpolate_html.evaluate("SupplementalReportColumnsTitles", i)
                        ,interpolate_html.evaluate("SupplementalReportColumnsMasks" , i)
                        }
                    );
                }
            }

        return columns;
    }

    illustration_table_columns const columns_      ;
    std::string                const page_template_;
};

class ill_reg_supplemental_report : public standard_supplemental_report
{
  public:
    explicit ill_reg_supplemental_report(html_interpolator const& interpolate_html)
        :standard_supplemental_report(interpolate_html, "ill_reg_supp_report")
    {
    }

  private:
    std::string get_upper_footer_template_name() const override
    {
        return "ill_reg_footer_disclaimer";
    }
};

// Regular illustration.
class pdf_illustration_regular : public pdf_illustration
{
  public:
    explicit pdf_illustration_regular(Ledger const& ledger)
        :pdf_illustration{ledger}
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
        // appropriate indefinite article preceding it, e.g. "an annual"
        // or "a monthly".
        std::string mode0 = invar.InitErMode;
        if(!mode0.empty())
            {
            mode0[0] = lmi_tolower(mode0[0]);
            add_variable
                ("ErModeLCWithArticle"
                ,(strchr("aeiou", mode0[0]) ? "an " : "a ") + mode0
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

        int const inforce_year = bourn_cast<int>(invar.InforceYear);
        add_variable
            ("UltimateInterestRate"
            ,evaluate("AnnGAIntRate_Current", inforce_year + 1)
            );

        add_variable
            ("InforceYearEq0"
            ,inforce_year == 0
            );

        add_variable
            ("InforceYearLE4"
            ,inforce_year < 4
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
        numbered_page::start_numbering();
        add<standard_page>("ill_reg_narr_summary");
        add<standard_page>("ill_reg_narr_summary2");
        add<standard_page>("ill_reg_column_headings");
        if(!invar.IsInforce)
            {
            add<ill_reg_numeric_summary_page>();
            }
        add<ill_reg_tabular_detail_page>();
        add<ill_reg_tabular_detail2_page>();
        if(invar.SupplementalReport)
            {
            add<ill_reg_supplemental_report>(get_interpolator());
            }
        if(!invar.IsInforce)
            {
            add<ill_reg_numeric_summary_attachment>();
            }
    }

    std::string get_upper_footer_template_name() const override
        { return {}; }
    std::string get_lower_footer_template_name() const override
        { return "ill_reg_footer"; }
};

// Common base class for basic illustration pages using the same columns in
// both NASD and private group placement illustrations.
class page_with_basic_tabular_report : public page_with_tabular_report
{
  private:
    // This method must be overridden to return the text of the super-header
    // used for all pairs of "cash surrender value" and "death benefit"
    // columns. The return value is subject to HTML interpolation and so may
    // contain {{variables}} and also can be multiline but, if so, it must have
    // the same number of lines for all input arguments.
    //
    // The basis and interest_rate arguments can be used to construct the full
    // name of the variable appropriate for the current column pair, with the
    // help of basis_suffix() and ir_suffix() functions.
    virtual std::string get_two_column_header
        (basis         guar_or_curr
        ,interest_rate zero_or_not
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
        ,column_separator_guar_curr0
        ,column_curr0_cash_surr_value
        ,column_curr0_death_benefit
        ,column_separator_curr0_curr
        ,column_curr_cash_surr_value
        ,column_curr_death_benefit
        ,column_max
        };

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "CSVNet_GuaranteedZero"      , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_GuaranteedZero" , "Death\nBenefit"              , "999,999,999" }
            ,{ ""                           , " "                           ,           "-" }
            ,{ "CSVNet_Guaranteed"          , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Guaranteed"     , "Death\nBenefit"              , "999,999,999" }
            ,{ ""                           , " "                           ,           "-" }
            ,{ "CSVNet_CurrentZero"         , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_CurrentZero"    , "Death\nBenefit"              , "999,999,999" }
            ,{ ""                           , " "                           ,           "-" }
            ,{ "CSVNet_Current"             , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"              , "999,999,999" }
            };

        return columns;
    }

    bool should_hide_column(Ledger const& ledger, int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger.is_composite() && column == column_end_of_year_age;
    }

    void render_or_measure_extra_headers
        (wx_table_generator&          table_gen
        ,html_interpolator const&     interpolate_html
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode
        ) const override
    {
        // Output the first super header row.

        auto pos_y_copy = pos_y;
        table_gen.output_super_header
            ("Using guaranteed charges"
            ,column_guar0_cash_surr_value
            ,column_separator_guar_curr0
            ,pos_y_copy
            ,output_mode
            );

        table_gen.output_super_header
            ("Using current charges"
            ,column_curr0_cash_surr_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_guar0_cash_surr_value
            ,column_separator_guar_curr0
            ,pos_y
            ,output_mode
            );
        table_gen.output_horz_separator
            (column_curr0_cash_surr_value
            ,column_max
            ,pos_y
            ,output_mode
            );
        pos_y += table_gen.separator_line_height();

        // Output the second super header row which is composed of three
        // physical lines.

        // This function outputs all lines of a single header, corresponding to
        // the "Guaranteed" or "Current", "Zero" or not, column and returns the
        // vertical position below the header.
        auto const output_two_column_super_header = [=,&table_gen]
            (basis          guar_or_curr
            ,interest_rate  zero_or_not
            ,std::size_t    begin_column
            ) -> int
            {
                std::size_t end_column = begin_column + 2;
                LMI_ASSERT(end_column <= column_max);

                auto y = pos_y;

                auto const header = get_two_column_header
                    (guar_or_curr
                    ,zero_or_not
                    );
                table_gen.output_super_header
                    (interpolate_html(header).as_html()
                    ,begin_column
                    ,end_column
                    ,y
                    ,output_mode
                    );

                table_gen.output_horz_separator
                    (begin_column
                    ,end_column
                    ,y
                    ,output_mode
                    );
                y += table_gen.separator_line_height();

                return y;
            };

        output_two_column_super_header
            (basis::guaranteed
            ,interest_rate::zero
            ,column_guar0_cash_surr_value
            );

        output_two_column_super_header
            (basis::guaranteed
            ,interest_rate::non_zero
            ,column_guar_cash_surr_value
            );

        output_two_column_super_header
            (basis::current
            ,interest_rate::zero
            ,column_curr0_cash_surr_value
            );

        pos_y = output_two_column_super_header
            (basis::current
            ,interest_rate::non_zero
            ,column_curr_cash_surr_value
            );
    }
};

class nasd_basic : public page_with_basic_tabular_report
{
  private:
    std::string get_fixed_page_contents_template_name() const override
    {
        return "nasd_basic";
    }

    std::string get_two_column_header
        (basis         guar_or_curr
        ,interest_rate zero_or_not
        ) const override
    {
        std::ostringstream oss;
        oss
            << "{{InitAnnSepAcctGrossInt_"
            << basis_suffix(guar_or_curr)
            << ir_suffix(zero_or_not)
            << "}} "
            << "Assumed Sep Acct\n"
            << "Gross Rate* "
            << "({{InitAnnSepAcctNetInt_"
            << basis_suffix(guar_or_curr)
            << ir_suffix(zero_or_not)
            << "}} net)\n"
            << "{{InitAnnGenAcctInt_"
            << basis_suffix(guar_or_curr)
            << "}} GPA rate"
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
        ,column_curr_account_value
        ,column_curr_cash_surr_value
        ,column_curr_death_benefit
        ,column_max
        };

    std::string get_fixed_page_contents_template_name() const override
    {
        return "nasd_supp";
    }

    // When invar.SplitMinPrem is true, this report has twelve columns
    // rather than eleven, and it's not generally possible to fit all
    // twelve. Ideally the net-premium column would be omitted in this
    // case because it's just not useful. Instead, for now at least,
    // the columns that are unique to this case are narrowed on the
    // assumption that premiums won't reach $100M even for composites.

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "ErGrossPmt"                 , "ER Gross\nPayment"           ,  "99,999,999" }
            ,{ "EeGrossPmt"                 , "EE Gross\nPayment"           ,  "99,999,999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "PolicyFee_Current"          , "Admin\nCharge"               , "999,999,999" }
            ,{ "PremTaxLoad_Current"        , "Premium\nTax Load"           , "999,999,999" }
            ,{ "DacTaxLoad_Current"         , "DAC\nTax Load"               , "999,999,999" }
            ,{ "ErModalMinimumPremium"      , "ER Modal\nMinimum\nPremium"  ,  "99,999,999" }
            ,{ "EeModalMinimumPremium"      , "EE Modal\nMinimum\nPremium"  ,  "99,999,999" }
            ,{ "NetPmt_Current"             , "Net\nPremium"                , "999,999,999" }
            ,{ "COICharge_Current"          , "Cost of\nInsurance\nCharges" , "999,999,999" }
            ,{ "AcctVal_Current"            , "Current\nAccount\nValue"     , "999,999,999" }
            ,{ "CSVNet_Current"             , "Current\nCash Surr\nValue"   , "999,999,999" }
            ,{ "EOYDeathBft_Current"        , "Current\nDeath\nBenefit"     , "999,999,999" }
            };

        return columns;
    }

    bool should_hide_column(Ledger const& ledger, int column) const override
    {
        auto const& invar = ledger.GetLedgerInvariant();

        // The supplemental page in NASD illustrations exists in two versions:
        // default one and one with split premiums. Hide columns that are not
        // needed for the current illustration.
        switch(column)
            {
            case column_end_of_year_age:
                // This column doesn't make sense for composite ledgers.
                return ledger.is_composite();

            case column_admin_charge:
            case column_premium_tax_load:
            case column_dac_tax_load:
                // These columns only appear in non-split premiums case.
                return invar.SplitMinPrem;

            case column_er_gross_payment:
            case column_ee_gross_payment:
            case column_er_min_premium:
            case column_ee_min_premium:
                // While those only appear in split premiums case.
                return !invar.SplitMinPrem;

            case column_policy_year:
            case column_premium_outlay:
            case column_net_premium:
            case column_cost_of_insurance_charges:
            case column_curr_account_value:
            case column_curr_cash_surr_value:
            case column_curr_death_benefit:
            case column_max:
                // These columns are common to both cases and never hidden.
                break;
            }

        return false;
    }
};

class nasd_assumption_detail : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_sep_acct_crediting_rate
        ,column_gen_acct_crediting_rate
        ,column_m_and_e
        ,column_ee_payment_mode
        ,column_er_payment_mode
        ,column_assumed_loan_interest
        ,column_max
        };

    std::string get_fixed_page_contents_template_name() const override
    {
        return "nasd_assumption_detail";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "AnnSAIntRate_Current"       , "Sep Acct Net\nInt Rate"      ,      "99.99%" }
            ,{ "AnnGAIntRate_Current"       , "Gen Acct\nCurrent Rate"      ,      "99.99%" }
            ,{ "CurrMandE"                  , "M&E"                         ,      "99.99%" }
            ,{ "EeMode"                     , "Indiv\nPmt Mode"             ,  "Semiannual" }
            ,{ "ErMode"                     , "Corp\nPmt Mode"              ,  "Semiannual" }
            ,{ "InitAnnLoanDueRate"         , "Assumed\nLoan Interest"      ,      "99.99%" }
            };

        return columns;
    }

    // Notice that there is no need to override should_hide_column() in this
    // class as this page is not included in composite illustrations and hence
    // all of its columns, including the "AttainedAge" one, are always shown.
};

// NASD illustration.
class pdf_illustration_nasd : public pdf_illustration
{
  public:
    explicit pdf_illustration_nasd(Ledger const& ledger)
        :pdf_illustration{ledger}
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

        auto const& policy_name = invar.PolicyLegalName;
        auto const& state_abbrev = invar.GetStatePostalAbbrev();

        add_variable
            ("UWTypeIsGuaranteedIssueInTexasWithFootnote"
            ,    invar.UWType == "Guaranteed issue"
              && policy_name == "Flexible Premium Variable Adjustable Life Insurance Policy"
              && state_abbrev == "TX"
            );

        add_variable
            ("HasTermOrSupplSpecAmt"
            ,test_variable("HasTerm") || test_variable("HasSupplSpecAmt")
            );

        add_variable
            ("StateIsNewYork"
            ,state_abbrev == "NY"
            );

        // Add all the pages.
        add<cover_page>();
        numbered_page::start_numbering();
        add<nasd_basic>();
        add<nasd_supplemental>();
        add<standard_page>("nasd_column_headings");
        add<standard_page>("nasd_notes1");
        add<standard_page>("nasd_notes2");
        if(!ledger.is_composite())
            {
            add<nasd_assumption_detail>();
            }
        if(invar.SupplementalReport)
            {
            add<standard_supplemental_report>
                (get_interpolator()
                ,"nasd_supp_report"
                );
            }
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
    std::string get_fixed_page_contents_template_name() const override
    {
        return "reg_d_group_basic";
    }

    std::string get_two_column_header
        (basis         guar_or_curr
        ,interest_rate zero_or_not
        ) const override
    {
        std::ostringstream oss;
        oss
            << "{{InitAnnSepAcctGrossInt_"
            << basis_suffix(guar_or_curr)
            << ir_suffix(zero_or_not)
            << "}} "
            << "Hypothetical Gross\n"
            << "Return ({{InitAnnSepAcctNetInt_"
            << basis_suffix(guar_or_curr)
            << ir_suffix(zero_or_not)
            << "}} net)"
            ;
        return oss.str();
    }
};

// Private group placement illustration.
class pdf_illustration_reg_d_group : public pdf_illustration
{
  public:
    explicit pdf_illustration_reg_d_group(Ledger const& ledger)
        :pdf_illustration{ledger}
    {
        // Define variables specific to this illustration.
        auto const& invar = ledger.GetLedgerInvariant();

        add_variable
            ("MecYearIs0"
            ,invar.MecYear == 0
            );

        // Add all the pages.
        add<cover_page>();
        numbered_page::start_numbering();
        add<reg_d_group_basic>();
        add<standard_page>("reg_d_group_column_headings");
        add<standard_page>("reg_d_group_narr_summary");
        add<standard_page>("reg_d_group_narr_summary2");
        if(invar.SupplementalReport)
            {
            add<standard_supplemental_report>
                (get_interpolator()
                ,"reg_d_group_supp_report"
                );
            }
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

    // Must be overridden to return the basis being used.
    virtual basis get_basis() const = 0;

    bool should_hide_column(Ledger const& ledger, int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger.is_composite() && column == column_end_of_year_age;
    }

    void render_or_measure_extra_headers
        (wx_table_generator&          table_gen
        ,html_interpolator const&     interpolate_html
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode
        ) const override
    {
        std::ostringstream header_zero;
        header_zero
            << "{{InitAnnSepAcctGrossInt_"
            << basis_suffix(get_basis())
            << ir_suffix(interest_rate::zero)
            << "}} Hypothetical Rate of\n"
            << "Return*"
            ;

        auto pos_y_copy = pos_y;
        table_gen.output_super_header
            (interpolate_html(header_zero.str()).as_html()
            ,column_zero_cash_surr_value
            ,column_zero_irr_surr_value
            ,pos_y_copy
            ,output_mode
            );

        std::ostringstream header_nonzero;
        header_nonzero
            << "{{InitAnnSepAcctGrossInt_"
            << basis_suffix(get_basis())
            << ir_suffix(interest_rate::non_zero)
            << "}} Hypothetical Rate of\n"
            << "Return*"
            ;

        table_gen.output_super_header
            (interpolate_html(header_nonzero.str()).as_html()
            ,column_nonzero_cash_surr_value
            ,column_nonzero_irr_surr_value
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_zero_cash_surr_value
            ,column_zero_irr_surr_value
            ,pos_y
            ,output_mode
            );
        table_gen.output_horz_separator
            (column_nonzero_cash_surr_value
            ,column_nonzero_irr_surr_value
            ,pos_y
            ,output_mode
            );
        pos_y += table_gen.separator_line_height();
    }
};

class reg_d_individual_guar_irr : public reg_d_individual_irr_base
{
  private:
    basis get_basis() const override
    {
        return basis::guaranteed;
    }

    std::string get_fixed_page_contents_template_name() const override
    {
        return "reg_d_indiv_guar_irr";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "CSVNet_GuaranteedZero"      , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_GuaranteedZero" , "Death\nBenefit"              , "999,999,999" }
            ,{ "IrrCsv_GuaranteedZero"      , "IRR on\nSurr Value"          ,  "100000.00%" }
            ,{ "IrrDb_GuaranteedZero"       , "IRR on\nDeath Bft"           ,  "100000.00%" }
            ,{ ""                           , " "                           ,           "-" }
            ,{ "CSVNet_Guaranteed"          , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Guaranteed"     , "Death\nBenefit"              , "999,999,999" }
            ,{ "IrrCsv_Guaranteed"          , "IRR on\nSurr Value"          ,  "100000.00%" }
            ,{ "IrrDb_Guaranteed"           , "IRR on\nDeath Bft"           ,  "100000.00%" }
            };

        return columns;
    }
};

class reg_d_individual_curr_irr : public reg_d_individual_irr_base
{
  private:
    basis get_basis() const override
    {
        return basis::current;
    }

    std::string get_fixed_page_contents_template_name() const override
    {
        return "reg_d_indiv_curr_irr";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "CSVNet_CurrentZero"         , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_CurrentZero"    , "Death\nBenefit"              , "999,999,999" }
            ,{ "IrrCsv_CurrentZero"         , "IRR on\nSurr Value"          ,  "100000.00%" }
            ,{ "IrrDb_CurrentZero"          , "IRR on\nDeath Bft"           ,  "100000.00%" }
            ,{ ""                           , " "                           ,           "-" }
            ,{ "CSVNet_Current"             , "Cash Surr\nValue"            , "999,999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"              , "999,999,999" }
            ,{ "IrrCsv_Current"             , "IRR on\nSurr Value"          ,  "100000.00%" }
            ,{ "IrrDb_Current"              , "IRR on\nDeath Bft"           ,  "100000.00%" }
            };

        return columns;
    }
};

class reg_d_individual_curr : public page_with_tabular_report
{
  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_premium_outlay
        ,column_premium_loads
        ,column_admin_charges
        ,column_curr_mortality_charges
        ,column_curr_asset_charges
        ,column_curr_investment_income
        ,column_curr_account_value
        ,column_curr_cash_surr_value
        ,column_curr_death_benefit
        ,column_max
        };

    std::string get_fixed_page_contents_template_name() const override
    {
        return "reg_d_indiv_curr";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "PremiumLoads"               , "Premium\nLoads"              , "999,999,999" }
            ,{ "AdminCharges"               , "Admin\nCharges"              , "999,999,999" }
            ,{ "COICharge_Current"          , "Mortality\nCharges"          , "999,999,999" }
            ,{ "SepAcctCharges_Current"     , "Asset\nCharges"              , "999,999,999" }
            ,{ "GrossIntCredited_Current"   , "Investment\nIncome"          , "999,999,999" }
            ,{ "AcctVal_Current"            , "Account\nValue"              , "999,999,999" }
            ,{ "CSVNet_Current"             , "Cash\nSurr Value"            , "999,999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"              , "999,999,999" }
            };

        return columns;
    }

    bool should_hide_column(Ledger const& ledger, int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger.is_composite() && column == column_end_of_year_age;
    }

    void render_or_measure_extra_headers
        (wx_table_generator&          table_gen
        ,html_interpolator const&     interpolate_html
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode
        ) const override
    {
        table_gen.output_super_header
            (interpolate_html
                ("{{InitAnnSepAcctGrossInt_Guaranteed}} Hypothetical Rate of Return*"
                ).as_html()
            ,column_curr_investment_income
            ,column_max
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_curr_investment_income
            ,column_max
            ,pos_y
            ,output_mode
            );
        pos_y += table_gen.separator_line_height();
    }
};

// Private individual placement illustration.
class pdf_illustration_reg_d_individual : public pdf_illustration
{
  public:
    explicit pdf_illustration_reg_d_individual(Ledger const& ledger)
        :pdf_illustration{ledger}
    {
        auto const& invar = ledger.GetLedgerInvariant();

        // Define variables specific to this illustration.
        add_abbreviated_variable("CorpName", 140);
        add_abbreviated_variable("Insured1", 140);

        // Add all the pages.
        numbered_page::start_numbering();
        add<standard_page>("reg_d_indiv_cover_page");
        add<reg_d_individual_guar_irr>();
        add<reg_d_individual_curr_irr>();
        add<reg_d_individual_curr>();
        add<standard_page>("reg_d_indiv_notes1");
        add<standard_page>("reg_d_indiv_notes2");
        if(invar.SupplementalReport)
            {
            add<standard_supplemental_report>
                (get_interpolator()
                ,"reg_d_indiv_supp_report"
                );
            }
    }

    std::string get_upper_footer_template_name() const override
    {
        return "reg_d_indiv_footer_upper";
    }

    std::string get_lower_footer_template_name() const override
    {
        return "reg_d_indiv_footer_lower";
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
    wxBusyCursor reverie;

    switch(ledger.ledger_type())
        {
        case mce_ill_reg:
            pdf_illustration_regular         (ledger).render_all(output);
            break;
        case mce_nasd:
            pdf_illustration_nasd            (ledger).render_all(output);
            break;
        case mce_group_private_placement:
            pdf_illustration_reg_d_group     (ledger).render_all(output);
            break;
        case mce_individual_private_placement:
            pdf_illustration_reg_d_individual(ledger).render_all(output);
            break;
        case mce_prospectus_obsolete:                 // fall through
        case mce_offshore_private_placement_obsolete: // fall through
        case mce_ill_reg_private_placement_obsolete:  // fall through
        case mce_variable_annuity_obsolete:
            alarum() << "Unsupported ledger type." << LMI_FLUSH;
        }
}

volatile bool ensure_setup = ledger_pdf_generator::set_creator
    (ledger_pdf_generator_wx::do_create
    );

} // Unnamed namespace.
