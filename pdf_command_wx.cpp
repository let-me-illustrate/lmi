// Create a PDF file from a ledger--wx interface.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "pdf_command.hpp"

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
#include "miscellany.hpp"               // lmi_tolower()
#include "oecumenic_enumerations.hpp"
#include "pdf_writer_wx.hpp"
#include "report_table.hpp"             // paginator
#include "safely_dereference_as.hpp"
#include "ssize_lmi.hpp"
#include "wx_new.hpp"
#include "wx_table_generator.hpp"

#include <wx/pdfdc.h>

#include <wx/utils.h>                   // wxBusyCursor

#include <wx/html/m_templ.h>

#include <array>
#include <cstdint>                      // ULONG_MAX
#include <cstdlib>                      // strtoul()
#include <cstring>                      // strchr(), strlen()
#include <exception>                    // uncaught_exceptions()
#include <fstream>
#include <map>
#include <memory>                       // make_unique(), unique_ptr
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>                      // forward(), move()
#include <vector>

LMI_FORCE_LINKING_IN_SITU(pdf_command_wx)

namespace
{

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
        (std::string      const& s
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

    static std::string reprocess(std::string const& raw_text)
    {
        std::string z = raw_text;

        z = std::regex_replace(z, std::regex("¶"), "<br>");
        z = std::regex_replace(z, std::regex("«"), "<strong>");
        z = std::regex_replace(z, std::regex("»"), "</strong>");

        std::regex const empty_paragraph("< *[Pp] *>[[:space:]]*< */[Pp] *>");
        z = std::regex_replace(z, empty_paragraph, "");

        return z;
    }

    // A function which can be used to interpolate an HTML string containing
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
        std::string z =
            interpolate_string
                (s
                ,[this]
                    (std::string const& str
                    ,interpolate_lookup_kind kind
                    )
                    {
                        return interpolation_func(str, kind);
                    }
                )
            ;
        return html::text::from_html
            (interpolate_string
                (reprocess(z).c_str()
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
        return evaluator_.value(name);
    }

    // Return a single value of a vector variable.
    std::string evaluate(std::string const& name, int index) const
    {
        return evaluator_.value(name, index);
    }

    // Interpolate the contents of the given external template.
    //
    // This is exactly the same as interpolating "{{>template_name}}" string
    // but a bit more convenient to use and simpler to read.
    html::text expand_template(std::string const& template_name) const
    {
        return (*this)("{{>" + template_name + "}}");
    }

    // PDF !! Retrofitting this accessor seems to suggest that
    // encapsulating the accessed object here may have been
    // premature.
    ledger_evaluator const& evaluator() {return evaluator_;}

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
            // and also check for overflow (notice that index == ULONG_MAX
            // doesn't, in theory, need to indicate overflow, but in practice
            // we're never going to have valid indices close to this number).
            if(stop != s.c_str() + s.length() - 1 || ULONG_MAX == index)
                {
                throw std::runtime_error
                    ("Index of vector variable '" + s + "' is not a valid number"
                    );
                }

            // Cast below is valid because of the check for overflow above.
            return html::text::from
                (evaluator_.value
                    (s.substr(0, open_pos)
                    ,bourn_cast<int>(index)
                    )
                );
            }

        return html::text::from(evaluator_.value(s));
    }

    std::string load_partial_from_file(std::string const& file) const
    {
        std::ifstream ifs(AddDataDir(file + ".xst"));
        if(!ifs)
            {
            alarum()
                << "Template file \""
                << file
                << ".xst\" not found."
                << std::flush
                ;
            }
        std::string partial;
        istream_to_string(ifs, partial);
        for(auto& i : partial) i = static_cast<unsigned char>(i ^ 0xff);
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
class table_mixin
{
  protected:
    table_mixin
        (Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        )
        :mixin_ledger_       {ledger}
        ,mixin_writer_       {writer}
        ,mixin_interpolator_ {interpolator}
    {
    }

    virtual ~table_mixin() = default;

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
    virtual bool should_hide_column(int) const // argument: column index
    {
        return false;
    }

    std::vector<std::string> visible_values(int year)
    {
        std::vector<std::string> v;
        auto const& columns = get_table_columns();
        for(int j = 0; j < lmi::ssize(columns); ++j)
            {
            columns[j].visibility = should_hide_column(j) ? oe_hidden : oe_shown;

            if(oe_shown == columns[j].visibility)
                {
                std::string output_value;
                if(columns[j].variable_name.empty())
                    {
                    ; // Separator column: use empty string.
                    }
                else
                    {
                    output_value = mixin_interpolator_.evaluate
                        (columns[j].variable_name
                        ,year
                        );
                    }

                v.push_back(output_value);
                }
            }
        return v;
    }

    // Useful helper for creating the table generator using the columns defined
    // by the separate (and simpler to implement) get_table_columns() pure
    // virtual function.
    wx_table_generator create_table_generator() const
    {
        std::vector<column_parameters> vc;
        std::vector<int> indices;
        int column = 0;
        for(auto const& i : get_table_columns())
            {
            indices.push_back(lmi::ssize(vc));
            if(!should_hide_column(column))
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
        auto& pdf_dc = mixin_writer_.dc();
        auto font = pdf_dc.GetFont();
        font.SetPointSize(9);
        pdf_dc.SetFont(font);

        return wx_table_generator
            (illustration_style_tag{}
            ,vc
            ,indices
            ,mixin_writer_.dc()
            ,mixin_writer_.get_horz_margin()
            ,mixin_writer_.get_page_width()
            );
    }

    // Names of data members have a distinctive prefix to prevent
    // collision with unprefixed names in other classes.

    Ledger            const& mixin_ledger_;

  private:
    pdf_writer_wx          & mixin_writer_;
    html_interpolator const& mixin_interpolator_;
};

// Custom handler for the HTML <header> tag not natively recognized by wxHTML.
// It also allows to find the container cell corresponding to the header
// contents later by assigning a unique ID to it.
static char const* const header_cell_id = "_lmi_page_header_id";

TAG_HANDLER_BEGIN(page_header, "HEADER")
    TAG_HANDLER_PROC(tag)
    {
        // As usual, reuse the current container if empty. That's unlikely:
        // although the header typically occurs at the very beginning of the
        // HTML template, it doesn't mean that the current container is empty,
        // quite on the contrary, it typically isn't because it contains the
        // cells setting the initial colors and font for the HTML body and we
        // must not make these cells part of the header cell as otherwise they
        // would be removed from the containing HTML document later and it
        // would use default font instead of the one set by pdf_writer_wx.
        auto container = m_WParser->GetContainer();
        if(container->GetFirstChild())
            {
            // It isn't, so we need to open a new one, which we will mark as
            // being the actual header cell.
            m_WParser->CloseContainer();
            container = m_WParser->OpenContainer();
            }

        // Set a unique ID for this container to allow finding it later.
        container->SetId(header_cell_id);

        // Use a nested container so that nested tags that close and reopen a
        // container again close this one, but still remain inside the outer
        // "header" container which will be detached from the main page HTML in
        // its entirety.
        m_WParser->OpenContainer();

        ParseInner(tag);

        // Close both the inner and the outer containers and reopen the
        // new current one.
        m_WParser->CloseContainer();
        m_WParser->CloseContainer();
        m_WParser->OpenContainer();

        // Return true to indicate that we've parsed the entire tag contents.
        return true;
    }
TAG_HANDLER_END(page_header)

// Base class for our custom HTML cells providing a way to pass them
// information about the PDF document being generated and the ledger used to
// generate it.
class html_cell_for_pdf_output : public wxHtmlCell
{
    friend class pdf_context_setter;

  public:
    // Before using this class a pdf_context_setter object needs to be
    // instantiated (and remain alive for as long as this class is used).
    class pdf_context_setter
    {
      public:
        // References passed to the ctor must have lifetime greater than that
        // of this object itself.
        explicit pdf_context_setter
            (Ledger            const& ledger
            ,pdf_writer_wx          & writer
            ,html_interpolator const& interpolator
            )
        {
            html_cell_for_pdf_output::pdf_context_for_html_output.set
                (&ledger
                ,&writer
                ,&interpolator
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
        pdf_context()
            :ledger_       {nullptr}
            ,writer_       {nullptr}
            ,interpolator_ {nullptr}
            {}

        void set
            (Ledger            const* ledger
            ,pdf_writer_wx          * writer
            ,html_interpolator const* interpolator
            )
        {
            ledger_       = ledger;
            writer_       = writer;
            interpolator_ = interpolator;
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

        html_interpolator const& interpolator() const
        {
            LMI_ASSERT(interpolator_);
            return *interpolator_;
        }

      private:
        Ledger            const* ledger_;
        pdf_writer_wx          * writer_;
        html_interpolator const* interpolator_;
    };

    // Small helper to check that we're using the expected DC and, also, acting
    // as a sink for the never used parameters of Draw().
    void draw_check_precondition
        (wxDC               & dc
        // There is no need to optimize drawing by restricting it to the
        // currently shown positions, we always render the cell entirely.
        ,int                  // view_y1
        ,int                  // view_y2
        // We don't care about rendering state as we don't support interactive
        // selection anyhow.
        ,wxHtmlRenderingInfo& // info
        )
    {
        // The DC passed to this function is supposed to be the same as the one
        // associated with the writer we will use for rendering, but check that
        // this is really so in order to avoid unexpectedly drawing the table
        // on something else.
        LMI_ASSERT(&dc == &pdf_context_for_html_output.writer().dc());
    }

    static inline pdf_context pdf_context_for_html_output = {};
};

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
        (wxImage  const& image
        ,wxString const& src
        ,double          scale_factor
        )
        :image_        {image}
        ,src_          {src}
        ,scale_factor_ {scale_factor}
    {
        m_Width  = wxRound(image.GetWidth () / scale_factor);
        m_Height = wxRound(image.GetHeight() / scale_factor);
    }

    // Override the base class function to actually render the image.
    void Draw
        (wxDC&                dc
        ,int                  x
        ,int                  y
        ,int                  view_y1
        ,int                  view_y2
        ,wxHtmlRenderingInfo& info
        ) override
    {
        draw_check_precondition(dc, view_y1, view_y2, info);

        auto& writer = pdf_context_for_html_output.writer();

        x += m_PosX;

        int pos_y = y + m_PosY;
        writer.output_image(image_, src_.utf8_str(), scale_factor_, x, pos_y);
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
                (new(wx) scaled_image_cell(image, src, scale_factor)
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
// if it contains a table not fitting on one page, although it may often
// correspond to a single physical page of the resulting illustration.
class logical_page
{
  public:
    // When a page is created, it must be associated with an illustration and
    // provided the HTML interpolator object which can be used for obtaining
    // the HTML text of the page contents from an external template.
    logical_page
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        )
        :illustration_ {illustration}
        ,ledger_       {ledger}
        ,writer_       {writer}
        ,interpolator_ {interpolator}
    {
    }

    // Pages are not value-like objects, so prohibit copying them.
    logical_page(logical_page const&) = delete;
    logical_page& operator=(logical_page const&) = delete;

    virtual ~logical_page() = default;

    // Called before rendering any pages to prepare for doing this, e.g. by
    // computing the number of pages needed.
    //
    // This function must not draw anything on the wxDC, it is provided only for
    // measurement purposes.
    virtual void pre_render() {}

    // Render this page contents.
    virtual void render() = 0;

  protected:
    pdf_illustration  const& illustration_;
    Ledger            const& ledger_;
    pdf_writer_wx          & writer_;
    html_interpolator const& interpolator_;
};

// Base class for the different kinds of illustrations.
//
// This object contains pages, added to it using its add() function, as well as
// illustration-global data registered as variables with html_interpolator and
// so available for the pages when expanding the external templates defining
// their contents.
class pdf_illustration : protected html_interpolator, protected pdf_writer_wx
{
  public:
    explicit pdf_illustration
        (Ledger   const& ledger
        ,fs::path const& pdf_out_file
        )
        :html_interpolator (ledger.make_evaluator())
        ,pdf_writer_wx     (pdf_out_file.string(), wxPORTRAIT, font_sizes_)
        ,ledger_           {ledger}
        ,pdf_out_file_     {pdf_out_file}
    {
        init_variables();
    }

    virtual ~pdf_illustration() = default;

    // Add a page.
    //
    // This is a template just in order to save on writing std::make_unique<>()
    // in the code using it and passing the illustration object and the
    // interpolator to the page ctor to make its creation shorter.
    template<typename T, typename... Args>
    void add(Args&&... args)
    {
        pages_.emplace_back
            (std::make_unique<T>
                (*this
                ,ledger_
                ,get_writer()
                ,get_interpolator()
                ,std::forward<Args>(args)...
                )
            );
    }

    // Render all pages to the specified PDF file.
    void render_all()
    {
        evaluator().write_tsv(pdf_out_file_);

        html_cell_for_pdf_output::pdf_context_setter the_pdf_context
            {ledger_
            ,get_writer()
            ,get_interpolator()
            };

        for(auto const& i : pages_)
            {
            i->pre_render();
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
                get_writer().next_page();
                }

            i->render();
            }

        get_writer().save();
    }

    // Functions to be implemented by the derived classes to indicate which
    // templates should be used for the upper (above the separating line) and
    // the lower parts of the footer. The upper template name may be empty if
    // it is not used at all.
    //
    // Notice that the upper footer template name can be overridden at the page
    // level, the functions here define the default for all illustration pages.
    //
    // These functions are used by the pages deriving from page_with_marginals.
    // The "lower" one formerly varied, but no longer does as of 2019-06; it's
    // kept virtual just in case it ever needs to be overridden again.
    virtual std::string get_upper_footer_template_name() const = 0;
    virtual std::string get_lower_footer_template_name() const
    {
        return "footer_lower";
    }

  protected:
    // Explicitly retrieve references to base classes.
    html_interpolator const& get_interpolator() const {return *this;}
    pdf_writer_wx          & get_writer      ()       {return *this;}

    // Helper for abbreviating a string to at most the given length (in bytes).
    static std::string abbreviate_if_necessary(std::string s, int len)
    {
        LMI_ASSERT(3 < len && len < 1000); // Implausible otherwise.
        if(len < lmi::ssize(s) && 3 < len)
            {
            s.replace(len - 3, std::string::npos, "...");
            }

        return s;
    }

    // Helper for creating abbreviated variables in the derived classes: such
    // variables have the name based on the name of the original variable with
    // "Abbrev" and "len" appended to it and their value is at most "len" bytes
    // long.
    void add_abbreviated_variable(std::string const& var, int len)
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

        // PDF !! Abbreviations of different lengths are used in some
        // derived classes; can uniform lengths be used everywhere?
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

        int const full_abbrev_length = 30;
        add_abbreviated_variable("MasterContractNumber", full_abbrev_length);
        add_abbreviated_variable("MasterContractNumber", full_abbrev_length / 2);
        add_abbreviated_variable("ContractNumber", full_abbrev_length);
        add_abbreviated_variable("ContractNumber", full_abbrev_length / 2);

        add_variable
            ("HasScaleUnit"
            ,!invar.scale_unit().empty()
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

        auto const& state_of_jurisdiction = invar.StateOfJurisdiction;
        add_variable
            ("StateIsNorthOrSouthCarolina"
            ,state_of_jurisdiction == "NC" || state_of_jurisdiction == "SC"
            );

        add_variable
            ("StateIsMaryland"
            ,state_of_jurisdiction == "MD"
            );
    }

    Ledger   const& ledger_;
    fs::path const& pdf_out_file_;

    // These font sizes are more suitable for illustrations than
    // the builtin wxHTML defaults. See:
    //   https://lists.nongnu.org/archive/html/lmi/2018-07/msg00033.html
    static inline html_font_sizes font_sizes_ {8, 9, 10, 12, 14, 18, 20};

    // All the pages of this illustration.
    std::vector<std::unique_ptr<logical_page>> pages_;
};

// Base class for all pages with a footer and/or header, collectively called
// "marginals".
class page_with_marginals : public logical_page
{
  public:
    using logical_page::logical_page;

    // Override pre_render() to compute page_top_ and footer_top_ which are
    // needed in the derived classes' overridden get_extra_pages_needed().
    void pre_render() override
    {
        auto const frame_horz_margin = writer_.get_horz_margin();
        auto const frame_width       = writer_.get_page_width();

        page_top_ = writer_.get_vert_margin();

        // Pre-render the header, if any.
        if
            (wxHtmlContainerCell* header_html = get_header_html()
            ;nullptr != header_html
            )
            {
            page_top_ += writer_.output_html
                (frame_horz_margin
                ,0
                ,frame_width
                ,*header_html
                ,oe_only_measure
                );
            }

        // We implicitly assume here that get_footer_lower_html() result
        // doesn't materially depend on the exact value of the page number as
        // we don't know its definitive value here yet. In theory, this doesn't
        // need to be true, e.g. we may later discover that 10 pages are needed
        // instead of 9 and the extra digit might result in a line wrapping on
        // a new line and this increasing the footer height, but in practice
        // this doesn't risk happening and taking into account this possibility
        // wouldn't be simple at all, so just ignore this possibility.
        auto footer_height = writer_.output_html
            (frame_horz_margin
            ,0
            ,frame_width
            ,get_footer_lower_html()
            ,oe_only_measure
            );

        if
            (auto const& upper_template = get_upper_footer_template_name()
            ;!upper_template.empty()
            )
            {
            footer_height += writer_.output_html
                (frame_horz_margin
                ,0
                ,frame_width
                ,interpolator_.expand_template(upper_template)
                ,oe_only_measure
                );

            // Leave a gap between the upper part of the footer and the main
            // page contents to separate them in absence of a separator line
            // which delimits the lower part.
            footer_height += writer_.dc().GetCharHeight();
            }

        footer_top_ = writer_.get_page_bottom() - footer_height;
    }

    void render() override
    {
        auto const frame_horz_margin = writer_.get_horz_margin();
        auto const frame_width       = writer_.get_page_width();

        auto& pdf_dc = writer_.dc();

        // Render the header, if any.
        if
            (wxHtmlContainerCell* header_html = get_header_html()
            ;nullptr != header_html
            )
            {
            writer_.output_html
                (frame_horz_margin
                ,writer_.get_vert_margin()
                ,frame_width
                ,*header_html
                );
            }

        // Render the footer, consisting of an optional upper and always
        // present lower part.
        auto y = footer_top_;

        if
            (auto const& upper_template = get_upper_footer_template_name()
            ;!upper_template.empty()
            )
            {
            y += pdf_dc.GetCharHeight();

            y += writer_.output_html
                (frame_horz_margin
                ,y
                ,frame_width
                ,interpolator_.expand_template(upper_template)
                );
            }

        writer_.output_html
            (frame_horz_margin
            ,y
            ,frame_width
            ,get_footer_lower_html()
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
    // Notice that it can only be used after calling our pre_render() function
    // as this is where it is computed.
    int get_footer_top() const
    {
        LMI_ASSERT(footer_top_ != 0);

        return footer_top_;
    }

    int get_page_body_top() const
    {
        return page_top_;
    }

    int get_page_body_height() const
    {
        return get_footer_top() - get_page_body_top();
    }

  private:
    // Function to be overridden in the base class which should actually return
    // the page number or equivalent string (e.g. "Appendix").
    virtual std::string get_page_number() const = 0;

    // Return non-owning pointer to the cell representing the header contents.
    // Returns null by default as most pages don't have a header or, at least,
    // not a header which needs to be repeated on the continuation physical
    // pages and so one that can't be treated as just being the beginning of
    // the main page body.
    virtual wxHtmlContainerCell* get_header_html() const
    {
        return nullptr;
    }

    // This function forwards to the illustration by default, but can be
    // overridden to define a page-specific footer if necessary.
    virtual std::string get_upper_footer_template_name() const
    {
        return illustration_.get_upper_footer_template_name();
    }

    // This function uses get_page_number() and returns the HTML wrapping it
    // and other fixed information appearing in the lower part of the footer.
    html::text get_footer_lower_html() const
    {
        auto const page_number_str = get_page_number();

        auto const templ = illustration_.get_lower_footer_template_name();

        // Use our own interpolation function to handle the special
        // "page_number" variable that is replaced with the actual
        // (possibly dynamic) page number.
        // PDF !! This member is already a const reference, so why
        // declare a const reference to it?
        auto const& z = interpolator_;
        return html::text::from_html
            (interpolate_string
                (("{{>" + templ + "}}").c_str()
                ,[page_number_str, z]
                    (std::string const& s
                    ,interpolate_lookup_kind kind
                    ) -> std::string
                    {
                    if(s == "page_number")
                        {
                        return page_number_str;
                        }

                    return z.interpolation_func(s, kind);
                    }
                )
            );
    }

    int page_top_   = 0;
    int footer_top_ = 0;
};

// Base class for all pages showing the page number in the footer.
//
// In addition to actually providing page_with_marginals with the correct
// string to show in the footer, this class implicitly handles the page count
// by incrementing it whenever a new object of this class is pre-rendered.
class numbered_page : public page_with_marginals
{
  public:
    // Must be called before creating the first numbered page.
    static void start_numbering()
    {
        last_page_number_ = 0;
    }

    numbered_page
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        )
        :page_with_marginals (illustration, ledger, writer, interpolator)
    {
        // This assert would fail if start_numbering() hadn't been called
        // before creating a numbered page, as it should be.
        LMI_ASSERT(0 <= last_page_number_);
    }

    void pre_render() override
    {
        page_with_marginals::pre_render();

        this_page_number_ = ++last_page_number_;

        extra_pages_ = get_extra_pages_needed();

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
    void next_page()
    {
        // This function may only be called if we had reserved enough physical
        // pages for these logical pages by overriding get_extra_pages_needed().
        LMI_ASSERT(0 < extra_pages_);

        writer_.next_page();

        ++this_page_number_;
        --extra_pages_;
    }

  private:
    virtual int get_extra_pages_needed() = 0;

    std::string get_page_number() const override
    {
        std::ostringstream oss;
        oss << "Page " << this_page_number_ << " of " << last_page_number_;
        return oss.str();
    }

    // "-1" is invalid; use start_numbering() to change it.
    static inline int last_page_number_ {-1};
    int               this_page_number_ {0};
    int               extra_pages_      {0};
};

// Simplest possible page which is entirely defined by its external template
// whose name must be specified when constructing it.
class standard_page : public numbered_page
{
  public:
    // Accept only string literals as template names, there should be no need
    // to use anything else.
    template<int N>
    explicit standard_page
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        ,char const               (&page_template_name)[N]
        )
        :numbered_page       (illustration, ledger, writer, interpolator)
        ,page_template_name_ {page_template_name}
    {
    }

    void pre_render() override
    {
        // Before calling the base class version, parse the HTML to initialize
        // page_body_cell_ and header_cell_.
        parse_page_html(writer_);

        numbered_page::pre_render();
    }

    void render() override
    {
        int last_page_break = 0;
        for(auto const& page_break : page_break_positions_)
            {
            if(last_page_break != 0)
                {
                next_page();
                }

            numbered_page::render();

            writer_.output_html
                (writer_.get_horz_margin()
                ,get_page_body_top()
                ,writer_.get_page_width()
                ,*page_body_cell_
                ,last_page_break
                ,page_break
                );

            last_page_break = page_break;
            }
    }

  protected:
    int get_extra_pages_needed() override
    {
        page_break_positions_ = writer_.paginate_html
            (writer_.get_page_width()
            ,get_page_body_height()
            ,*page_body_cell_
            );

        // The cast is safe, we're never going to have more than INT_MAX
        // pages and if we, somehow, do, the caller checks that this function
        // returns a positive value.
        return static_cast<int>(page_break_positions_.size()) - 1;
    }

  private:
    // Parse HTML page contents once and store the result in page_body_cell_
    // and header_cell_ member variables.
    //
    // Throws if parsing fails.
    void parse_page_html(pdf_writer_wx& writer)
    {
        // We should be called once and only once.
        LMI_ASSERT(!page_body_cell_);

        page_body_cell_ = writer.parse_html
            (interpolator_.expand_template(page_template_name_)
            );

        if(!page_body_cell_)
            {
            throw std::runtime_error
                ("failed to parse template '" + std::string{page_template_name_} + "'"
                );
            }

        // Check if the page has a header tag and extract it from it in this
        // case. It is not an error if there is no header in this page.
        for(auto cell = page_body_cell_->GetFirstChild(); cell; cell = cell->GetNext())
            {
            if(cell->GetId() == header_cell_id)
                {
                // Detach the cell from the tree to prevent it from being
                // rendered as part of the page body.
                page_body_cell_->Detach(cell);

                // And convert it to self-containing HTML document representing
                // just the header contents.
                //
                // Note that we can't just use this cell on its own, we
                // must let wxHtmlWinParser build the usual structure as
                // wxHTML relies on having extra cells in its DOM, notably
                // the wxHtmlFontCell setting the initial document font.
                header_cell_ = writer.make_html_from(cell);

                break;
                }
            }
    }

    wxHtmlContainerCell* get_header_html() const override
    {
        return header_cell_.get();
    }

    char const* const                    page_template_name_;
    std::unique_ptr<wxHtmlContainerCell> page_body_cell_;
    std::unique_ptr<wxHtmlContainerCell> header_cell_;
    std::vector<int>                     page_break_positions_;
};

/// Generic cover page for most ledger types.
///
/// See discussion here:
///   https://lists.nongnu.org/archive/html/lmi/2019-04/msg00024.html

class cover_page : public standard_page
{
  public:
    cover_page
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        )
        :standard_page
            (illustration
            ,ledger
            ,writer
            ,interpolator
            ,"cover"
            )
    {
    }

  private:
    int get_extra_pages_needed() override
    {
        int const extra = standard_page::get_extra_pages_needed();
        if(0 != extra)
            warning() << "Cover page will overflow." << LMI_FLUSH;
        return extra;
    }

    // Only the lower part of the footer is wanted here.
    std::string get_upper_footer_template_name() const override
    {
        return std::string {};
    }
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
    ,private table_mixin
{
  public:
    numeric_summary_table_cell()
        :table_mixin
            (pdf_context_for_html_output.ledger()
            ,pdf_context_for_html_output.writer()
            ,pdf_context_for_html_output.interpolator()
            )
    {
        m_Height = render_or_measure(0, oe_only_measure);
    }

    /// Override the base class function to actually render the table.
    ///
    /// PDF !! Ideally, the 'x' argument would be passed to
    /// render_or_measure(), which in turn would pass it to
    /// create_table_generator(), which would use it instead of
    /// always using get_horz_margin(). This cannot be asserted:
    ///   LMI_ASSERT(x == mixin_writer_.get_horz_margin());
    /// because mixin_writer_ is private in this context.

    void Draw
        (wxDC               & dc
        ,int                  // x
        ,int                  y
        ,int                  view_y1
        ,int                  view_y2
        ,wxHtmlRenderingInfo& info
        ) override
    {
        draw_check_precondition(dc, view_y1, view_y2, info);
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
        wx_table_generator table_gen {create_table_generator()};

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

        auto const& invar = mixin_ledger_.GetLedgerInvariant();

        int const year_max = mixin_ledger_.greatest_lapse_dur();

        int const summary_age = 70;
        // Other rows are for given durations, but the
        // last row is for a given age (typically 70).
        std::ostringstream oss;
        oss << "Age " << summary_age;
        std::string const summary_age_string = oss.str();

        std::array<int, 4> const summary_years =
            // "- 1": 70 (e.g.) is end-of-year age.
            {{4, 9, 19, summary_age - bourn_cast<int>(invar.Age) - 1}
            };
        for(auto const& year : summary_years)
            {
            // Skip row if it doesn't exist. For instance, if the issue
            // age is 85 and the contract remains in force until age 100,
            // then there is no twentieth duration and no age-70 row.
            if(!(0 <= year && year < year_max))
                {
                continue;
                }

            // Last row, showing the values for "Age 70" normally, needs to be
            // handled specially.
            bool const is_last_row = &year == &summary_years.back();

            // For composite ledgers, "Age" doesn't make sense and so this row
            // should be just skipped for them.
            if(is_last_row && mixin_ledger_.is_composite())
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
                    auto v = visible_values(year);
                    if(is_last_row)
                        {
                        v.at(column_policy_year) = summary_age_string;
                        }

                    table_gen.output_row(pos_y, v);
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
        // it, but currently we don't--so cast its address to void to prevent
        // the compiler from admonishing us.
        (void)&(tag);

        m_WParser->GetContainer()->InsertCell(new(wx) numeric_summary_table_cell());

        // This tag isn't supposed to have any inner contents, so return true
        // to not even try parsing it.
        return true;
    }
TAG_HANDLER_END(numeric_summary_table)

// Custom handler for <p> tags preventing the page breaks inside them.
TAG_HANDLER_BEGIN(unbreakable_paragraph, "P")
    TAG_HANDLER_PROC(tag)
    {
        // Note: this code mimics what TAG_HANDLER_PROC()s for "div" and "p"
        // tags in wxHTML itself do by copying their code because there is
        // unfortunately no way to delegate to them currently.

        // As usual, reuse the current container if it's empty.
        auto container = m_WParser->GetContainer();
        if(container->GetFirstChild())
            {
            // It isn't, we need to open a new one.
            m_WParser->CloseContainer();
            container = m_WParser->OpenContainer();
            }

        // This is the reason for this handler existence: mark the container
        // used for the paragraph contents as being unbreakable.
        container->SetCanLiveOnPagebreak(false);

        // Use a nested container so that nested tags that close and reopen a
        // container again close this one, but still remain inside the outer
        // "unbreakable" container.
        container = m_WParser->OpenContainer();

        // This code reproduces what the standard "P" handler does.
        // Unfortunately there is no way to just delegate to it from here.
        container->SetIndent(m_WParser->GetCharHeight(), wxHTML_INDENT_TOP);
        container->SetAlign(tag);

        ParseInner(tag);

        // Close both the inner and the outer containers and reopen the
        // new current one.
        m_WParser->CloseContainer();
        m_WParser->CloseContainer();
        m_WParser->OpenContainer();

        // Return true to indicate that we've parsed the entire tag contents.
        return true;
    }
TAG_HANDLER_END(unbreakable_paragraph)

TAGS_MODULE_BEGIN(lmi_illustration)
    TAGS_MODULE_ADD(page_header)
    TAGS_MODULE_ADD(scaled_image)
    TAGS_MODULE_ADD(numeric_summary_table)
    TAGS_MODULE_ADD(unbreakable_paragraph)
TAGS_MODULE_END(lmi_illustration)

class ill_reg_numeric_summary_page : public standard_page
{
  public:
    ill_reg_numeric_summary_page
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        )
        :standard_page
            (illustration
            ,ledger
            ,writer
            ,interpolator
            ,"ill_reg_numeric_summary"
            )
    {
    }
};

// Helper base class for pages showing a table displaying values for all
// contract years after some fixed content.
class page_with_tabular_report
    :public numbered_page
    ,protected table_mixin
    ,private paginator
{
  public:
    page_with_tabular_report
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        )
        :numbered_page (illustration, ledger, writer, interpolator)
        ,table_mixin   (              ledger, writer, interpolator)
        ,offset_    {bourn_cast<int>(ledger_.GetLedgerInvariant().InforceYear)}
        ,year_      {0}
        ,pos_y_     {}
    {
    }

    /// Initialize a wx_table_generator.
    ///
    /// This cannot be done in the ctor, where the virtual function
    /// get_table_columns() is still pure; yet it is wasteful to
    /// recreate inside every member function that uses it; therefore,
    /// create it OAOO, here--because this is apparently the first
    /// function called after the derived-class ctors have run.
    ///
    /// Create the wx_table_generator before calling the base-class
    /// implementation, which calls get_extra_pages_needed(), which
    /// uses the object initialized here.

    void pre_render() override
    {
        table_gen_.reset(new(wx) wx_table_generator {create_table_generator()});
        numbered_page::pre_render();
    }

    void render() override
    {
        paginator::print();
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
        (wx_table_generator           & // table_gen
        ,int                          & // pos_y
        ,oenum_render_or_only_measure   // output_mode
        ) const
    {
    }

  private:
    wx_table_generator& table_gen() const
    {
        return safely_dereference_as<wx_table_generator>(table_gen_.get());
    }

    // Render (only if output_mode is oe_render) the fixed page part and
    // (in any case) return the vertical coordinate of its bottom, where the
    // tabular report starts.
    int render_or_measure_fixed_page_part
        (oenum_render_or_only_measure output_mode
        ) const
    {
        int pos_y = writer_.get_vert_margin();

        pos_y += writer_.output_html
            (writer_.get_horz_margin()
            ,pos_y
            ,writer_.get_page_width()
            ,interpolator_.expand_template(get_fixed_page_contents_template_name())
            ,output_mode
            );

        render_or_measure_extra_headers
            (table_gen()
            ,pos_y
            ,output_mode
            );

        table_gen().output_headers(pos_y, output_mode);

        auto const ncols = lmi::ssize(get_table_columns());
        table_gen().output_horz_separator(0, ncols, pos_y, output_mode);
        pos_y += table_gen().separator_line_height();

        return pos_y;
    }

    // Override the base class function as the table may overflow onto the next
    // page(s).
    int get_extra_pages_needed() override
    {
        int const pos_y = render_or_measure_fixed_page_part(oe_only_measure);

        int const max_lines_per_page = (get_footer_top() - pos_y) / table_gen().row_height();

        int const rows_per_group = wx_table_generator::rows_per_group;

        if(max_lines_per_page < rows_per_group)
            {
            // We can't afford to continue in this case as we can never output
            // the table as the template simply doesn't leave enough space for
            // it on the page.
            throw std::runtime_error("no space left for tabular report");
            }

        // "-1 +": return the number of *extra* pages.
        return -1 + paginator::init
            (ledger_.greatest_lapse_dur() - offset_
            ,wx_table_generator::rows_per_group
            ,max_lines_per_page
            );
    }

    void prelude          () override {}

    void open_page        () override
        {
            // "if": next_page() has already been called once, which
            // is perfect for logical pages that fit on one physical
            // page. See:
            //   https://lists.nongnu.org/archive/html/lmi/2018-09/msg00022.html
            if(0 != year_) next_page();
            numbered_page::render();
            pos_y_ = render_or_measure_fixed_page_part(oe_render);
        }

    void print_a_data_row () override
        {
            auto const v = visible_values(year_ + offset_);
            table_gen().output_row(pos_y_, v);
            ++year_;
        }

    void print_a_separator() override
        {
            pos_y_ += table_gen().row_height();
        }

    void close_page       () override {}

    void postlude         () override {}

    std::unique_ptr<wx_table_generator>        table_gen_;
    int                                 const  offset_;
    int                                        year_;
    int                                        pos_y_;
};

class ill_reg_tabular_detail_page : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

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
        (wx_table_generator           & table_gen
        ,int                          & pos_y
        ,oenum_render_or_only_measure   output_mode
        ) const override
    {
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

    bool should_hide_column(int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger_.is_composite() && column == column_end_of_year_age;
    }
};

class ill_reg_tabular_detail2_page : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

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

    bool should_hide_column(int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger_.is_composite() && column == column_end_of_year_age;
    }
};

// Class for pages showing supplemental report after the fixed template
// contents. It can be either used directly or further derived from, e.g. to
// override some of its inherited virtual functions such as
// get_upper_footer_template_name() as done below.
class standard_supplemental_report : public page_with_tabular_report
{
  public:
    explicit standard_supplemental_report
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        ,std::string       const& page_template
        )
        :page_with_tabular_report (illustration, ledger, writer, interpolator)
        ,columns_       {build_columns(interpolator)}
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
    static illustration_table_columns build_columns
        (html_interpolator const& interpolator
        )
    {
        constexpr int max_columns = 12;
        std::string const empty_column_name("[none]");

        illustration_table_columns columns;
        for(int i = 0; i < max_columns; ++i)
            {
            auto name = interpolator.evaluate("SupplementalReportColumnsNames", i);
            if(name != empty_column_name)
                {
                columns.emplace_back
                    (illustration_table_column
                        {std::move(name)
                        ,interpolator.evaluate("SupplementalReportColumnsTitles", i)
                        ,interpolator.evaluate("SupplementalReportColumnsMasks" , i)
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
    explicit ill_reg_supplemental_report
        (pdf_illustration  const& illustration
        ,Ledger            const& ledger
        ,pdf_writer_wx          & writer
        ,html_interpolator const& interpolator
        )
        :standard_supplemental_report
            (illustration
            ,ledger
            ,writer
            ,interpolator
            ,"ill_reg_supp_report"
            )
    {
    }

  private:
    std::string get_upper_footer_template_name() const override
    {
        return "ill_reg_footer_disclaimer";
    }
};

/// Illustration subject to NAIC illustration reg.

class pdf_illustration_naic : public pdf_illustration
{
  public:
    explicit pdf_illustration_naic
        (Ledger   const& ledger
        ,fs::path const& pdf_out_file
        )
        :pdf_illustration{ledger, pdf_out_file}
    {
        auto const& invar = ledger.GetLedgerInvariant();
        auto const& state_of_jurisdiction = invar.StateOfJurisdiction;

        // Define variables specific to this illustration.

        add_abbreviated_variable("CorpName", 50);
        add_abbreviated_variable("Insured1", 50);

        auto const is_single_premium = bourn_cast<int>(invar.IsSinglePremium);
        add_variable
            ("SinglePremium"
            ,     oe_plain_single_premium     == is_single_premium
               || oe_modified_single_premium  == is_single_premium
               || oe_limited_flexible_premium == is_single_premium
            );

        add_variable
            ("PlainSinglePremium"
            ,oe_plain_single_premium     == is_single_premium
            );

        add_variable
            ("ModifiedSinglePremium"
            ,oe_modified_single_premium  == is_single_premium
            );

        add_variable
            ("LimitedFlexiblePremium"
            ,oe_limited_flexible_premium == is_single_premium
            );

        // Lowercase initial payment mode with prepended indefinite
        // article: e.g., "an annual" or "a monthly".
        std::string eemode0 = invar.InitEeMode;
        eemode0[0] = lmi_tolower(eemode0[0]);
        add_variable
            ("InitEeModeWithIndefiniteArticle"
            ,(std::strchr("aeiou", eemode0[0]) ? "an " : "a ") + eemode0
            );
        std::string ermode0 = invar.InitErMode;
        ermode0[0] = lmi_tolower(ermode0[0]);
        add_variable
            ("InitErModeWithIndefiniteArticle"
            ,(std::strchr("aeiou", ermode0[0]) ? "an " : "a ") + ermode0
            );

        add_variable
            ("HasInterestDisclaimer"
            ,!invar.InterestDisclaimer.empty()
            );

        add_variable
            ("StateIsIllinois"
            ,state_of_jurisdiction == "IL"
            );

        add_variable
            ("StateIsTexas"
            ,state_of_jurisdiction == "TX"
            );

        add_variable
            ("StateIsIllinoisOrTexas"
            ,state_of_jurisdiction == "IL" || state_of_jurisdiction == "TX"
            );

        int const inforce_year = bourn_cast<int>(invar.InforceYear);
        add_variable
            ("PresentCreditingRate"
            ,evaluate("AnnGAIntRate_Current", inforce_year)
            );

        add_variable
            ("DuringFirstYear"
            ,0 == inforce_year
            );

        add_variable
            ("InforceYearLT5"
            ,inforce_year < 5
            );

        auto const max_duration = invar.EndtAge - invar.Age;
        auto const lapse_year_guar = ledger.GetGuarFull().LapseYear;
        auto const lapse_year_mdpt = ledger.GetMdptFull().LapseYear;
        auto const lapse_year_curr = ledger.GetCurrFull().LapseYear;

        add_variable
            ("LapseYear_Guar_LT_MaxDur"
            ,lapse_year_guar < max_duration
            );

        add_variable
            ("LapseYear_Guar_Plus1"
            ,bourn_cast<int>(lapse_year_guar) + 1
            );

        add_variable
            ("LapseYear_Mdpt_LT_MaxDur"
            ,lapse_year_mdpt < max_duration
            );

        add_variable
            ("LapseYear_Mdpt_Plus1"
            ,bourn_cast<int>(lapse_year_mdpt) + 1
            );

        add_variable
            ("LapseYear_Curr_LT_MaxDur"
            ,lapse_year_curr < max_duration
            );

        add_variable
            ("LapseYear_Curr_Plus1"
            ,bourn_cast<int>(lapse_year_curr) + 1
            );

        // Add all the pages.
        numbered_page::start_numbering();
        add<cover_page>();
        add<standard_page>("ill_reg_narr_summary");
        add<standard_page>("ill_reg_narr_summary2");
        add<standard_page>("ill_reg_column_headings");
        // The condition is not '!invar.IsInforce': the question is
        // whether the first anniversary has been reached, not whether
        // the issue date has been passed. See the documentation for
        // AccountValue::SetGuarPrem(). In MST files, the variable
        // "DuringFirstYear" represents this same condition.
        if(0 == inforce_year)
            {
            add<ill_reg_numeric_summary_page>();
            }
        add<ill_reg_tabular_detail_page>();
        add<ill_reg_tabular_detail2_page>();
        if(invar.SupplementalReport)
            {
            add<ill_reg_supplemental_report>();
            }
        // Add this again, even though it was already added above.
        // Notionally, the purchaser detaches this duplicate paper
        // page and mails it physically to the insurer. Someday a
        // more modern alternative might be chosen.
        if(0 == inforce_year)
            {
            add<ill_reg_numeric_summary_page>();
            }
    }

    std::string get_upper_footer_template_name() const override
        { return {}; }
};

// Common base class for basic illustration pages using the same columns in
// both FINRA and private group placement illustrations.
class page_with_basic_tabular_report : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

  private:
    // This function must be overridden to return the text of the super-header
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

    bool should_hide_column(int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger_.is_composite() && column == column_end_of_year_age;
    }

    void render_or_measure_extra_headers
        (wx_table_generator           & table_gen
        ,int                          & pos_y
        ,oenum_render_or_only_measure   output_mode
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
        // PDF !! This member is already a const reference, so why
        // declare a const reference to it?
        auto const& z = interpolator_;
        auto const output_two_column_super_header = [this,pos_y,output_mode,z,&table_gen]
            (basis         guar_or_curr
            ,interest_rate zero_or_not
            ,int           begin_column
            ) -> int
            {
                int end_column = begin_column + 2;
                LMI_ASSERT(end_column <= column_max);

                auto y = pos_y;

                auto const header = get_two_column_header
                    (guar_or_curr
                    ,zero_or_not
                    );
                table_gen.output_super_header
                    (z(header).as_html()
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

class finra_basic : public page_with_basic_tabular_report
{
  public:
    using page_with_basic_tabular_report::page_with_basic_tabular_report;

  private:
    std::string get_fixed_page_contents_template_name() const override
    {
        return "finra_basic";
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
            << "Assumed"
            << "\nSep Acct Gross"
            << "\nRate* "
            << "({{InitAnnSepAcctNetInt_"
            << basis_suffix(guar_or_curr)
            << ir_suffix(zero_or_not)
            << "}} net)"
            << "\n{{InitAnnGenAcctInt_"
            << basis_suffix(guar_or_curr)
            << "}} GPA rate"
            ;
        return oss.str();
    }
};

class finra_supplemental : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

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
        return "finra_supp";
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

    bool should_hide_column(int column) const override
    {
        auto const& invar = ledger_.GetLedgerInvariant();

        // The supplemental page in FINRA illustrations exists in two versions:
        // default one and one with split premiums. Hide columns that are not
        // needed for the current illustration.
        switch(column)
            {
            case column_end_of_year_age:
                // This column doesn't make sense for composite ledgers.
                return ledger_.is_composite();

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

class finra_split_fund_report : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

  private:
    enum
        {column_policy_year
        ,column_end_of_year_age
        ,column_er_gross_payment
        ,column_ee_gross_payment
        ,column_premium_outlay
        ,column_guar0_av_gen
        ,column_guar0_av_sep
        ,column_guar0_account_value
        ,column_guar0_cash_surr_value
        ,column_curr0_av_gen
        ,column_curr0_av_sep
        ,column_curr0_account_value
        ,column_curr0_cash_surr_value
        ,column_max
        };

    std::string get_fixed_page_contents_template_name() const override
    {
        // Note that we reuse the template for another report because we only
        // need our template to define the standard header, as this one already
        // does, and all the rest, i.e. the report itself, is generated
        // dynamically by this class.
        return "finra_supp";
    }

    illustration_table_columns const& get_table_columns() const override
    {
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "ErGrossPmt"                 , "ER Gross\nPayment"           ,  "99,999,999" }
            ,{ "EeGrossPmt"                 , "EE Gross\nPayment"           ,  "99,999,999" }
            ,{ "GrossPmt"                   , "Premium\nOutlay"             , "999,999,999" }
            ,{ "AVGenAcct_GuaranteedZero"   , "Guar\nCharges\n"
                                              "Acct Value\n"
                                              "Gen Acct"                    , "999,999,999" }
            ,{ "AVSepAcct_GuaranteedZero"   , "Guar\nCharges\n"
                                              "0% Rate\n"
                                              "Acct Value\n"
                                              "Sep Acct"                    , "999,999,999" }
            ,{ "AcctVal_GuaranteedZero"     , "Guar\nCharges\n"
                                              "0% Rate\n"
                                              "Acct Value"                  , "999,999,999" }
            ,{ "CSVNet_GuaranteedZero"      , "Guar\nCharges\n"
                                              "0% Rate\n"
                                              "Cash Surr\nValue"            , "999,999,999" }
            ,{ "AVGenAcct_CurrentZero"      , "Curr\nCharges\n"
                                              "Acct Value\n"
                                              "Gen Acct"                    , "999,999,999" }
            ,{ "AVSepAcct_CurrentZero"      , "Curr\nCharges\n"
                                              "0% Rate\n"
                                              "Acct Value\n"
                                              "Sep Acct"                    , "999,999,999" }
            ,{ "AcctVal_CurrentZero"        , "Curr\nCharges\n"
                                              "0% Rate\n"
                                              "Acct Value"                  , "999,999,999" }
            ,{ "CSVNet_Current"             , "Curr\nCharges\n"
                                              "0% Rate\n"
                                              "Cash Surr\nValue"            , "999,999,999" }
            };

        return columns;
    }

    bool should_hide_column(int column) const override
    {
        auto const& invar = ledger_.GetLedgerInvariant();

        // This report shows either ER and EE gross payment columns in split
        // premiums case or a single premium outlay column otherwise.
        switch(column)
            {
            case column_end_of_year_age:
                // This column doesn't make sense for composite ledgers.
                return ledger_.is_composite();

            case column_er_gross_payment:
            case column_ee_gross_payment:
                // These columns appear only in this case (which,
                // weirdly, differs from the SplitMinPrem case
                // that governs elsewhere)...
                return !invar.ErNotionallyPaysTerm;

            case column_premium_outlay:
                // ...while this one replaces them otherwise.
                return invar.ErNotionallyPaysTerm;

            case column_policy_year:
            case column_guar0_cash_surr_value:
            case column_curr0_cash_surr_value:
            case column_guar0_account_value:
            case column_curr0_account_value:
            case column_guar0_av_sep:
            case column_curr0_av_sep:
            case column_guar0_av_gen:
            case column_curr0_av_gen:
            case column_max:
                // These columns are common to both cases and never hidden.
                break;
            }

        return false;
    }
};

class finra_assumption_detail : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

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
        return "finra_assumption_detail";
    }

    illustration_table_columns const& get_table_columns() const override
    {
    // PDF !! AnnLoanDueRate: "Loan\nInt Rate" would be a better
    // heading. "Assumed\nLoan Interest" suggests a dollar amount,
    // but this column is actually an annual rate. Because VLR is not
    // implemented, this is just the contractual fixed loan rate, for
    // which the word "Assumed" is misleading.
        static illustration_table_columns const columns =
            {{ "PolicyYear"                 , "Policy\nYear"                ,         "999" }
            ,{ "AttainedAge"                , "End of\nYear\nAge"           ,         "999" }
            ,{ "AnnSAIntRate_Current"       , "Sep Acct Net\nInt Rate"      ,      "99.99%" }
            ,{ "AnnGAIntRate_Current"       , "Gen Acct\nCurrent Rate"      ,      "99.99%" }
            ,{ "CurrMandE"                  , "M&E"                         ,      "99.99%" }
            ,{ "EeMode"                     , "Indiv\nPmt Mode"             ,  "Semiannual" }
            ,{ "ErMode"                     , "Corp\nPmt Mode"              ,  "Semiannual" }
            ,{ "AnnLoanDueRate"             , "Assumed\nLoan Interest"      ,      "99.99%" }
            };

        return columns;
    }

    // Notice that there is no need to override should_hide_column() in this
    // class as this page is not included in composite illustrations and hence
    // all of its columns, including the "AttainedAge" one, are always shown.
};

/// Illustration subject to FINRA regulation.

class pdf_illustration_finra : public pdf_illustration
{
  public:
    explicit pdf_illustration_finra
        (Ledger   const& ledger
        ,fs::path const& pdf_out_file
        )
        :pdf_illustration{ledger, pdf_out_file}
    {
        auto const& invar = ledger.GetLedgerInvariant();

        // Define variables specific to this illustration.

        auto const& state_of_jurisdiction = invar.StateOfJurisdiction;

        add_variable
            ("CallGuarUwSubstd"
            ,    invar.TxCallsGuarUwSubstd
              && invar.UWType == "Guaranteed issue"
              && state_of_jurisdiction == "TX"
            );

        add_variable
            ("HasTermOrSupplSpecAmt"
            ,test_variable("HasTerm") || test_variable("HasSupplSpecAmt")
            );

        // Add all the pages.
        numbered_page::start_numbering();
        add<cover_page>();
        add<finra_basic>();
        add<finra_supplemental>();
        add<standard_page>("finra_column_headings");
        add<standard_page>("finra_notes1");
        add<standard_page>("finra_notes2");
        if(!ledger.is_composite())
            {
            add<finra_assumption_detail>();
            }
        if(invar.SplitFundAllocation)
            {
            add<finra_split_fund_report>();
            }
        if(invar.SupplementalReport)
            {
            add<standard_supplemental_report>("finra_supp_report");
            }
    }

    std::string get_upper_footer_template_name() const override
    {
        return "finra_footer_upper";
    }
};

// Basic illustration page of the private group placement illustration.
class reg_d_group_basic : public page_with_basic_tabular_report
{
  public:
    using page_with_basic_tabular_report::page_with_basic_tabular_report;

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
            << "Hypothetical"
            << "\nGross Return"
            << "\n({{InitAnnSepAcctNetInt_"
            << basis_suffix(guar_or_curr)
            << ir_suffix(zero_or_not)
            << "}} net)"
            ;
        return oss.str();
    }
};

/// Group private placement illustration subject to Reg D.

class pdf_illustration_reg_d_group : public pdf_illustration
{
  public:
    explicit pdf_illustration_reg_d_group
        (Ledger   const& ledger
        ,fs::path const& pdf_out_file
        )
        :pdf_illustration{ledger, pdf_out_file}
    {
        // Define variables specific to this illustration.

        auto const& invar = ledger.GetLedgerInvariant();

        // Add all the pages.
        numbered_page::start_numbering();
        add<cover_page>();
        add<reg_d_group_basic>();
        add<standard_page>("reg_d_group_column_headings");
        add<standard_page>("reg_d_group_narr_summary");
        add<standard_page>("reg_d_group_narr_summary2");
        if(invar.SupplementalReport)
            {
            add<standard_supplemental_report>("reg_d_group_supp_report");
            }
    }

    std::string get_upper_footer_template_name() const override
    {
        return "reg_d_group_footer_upper";
    }
};

// This page exists in two almost identical versions, one using guaranteed and
// the other one using current values, use a base class to share the common
// parts.
class reg_d_indiv_irr_base : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

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

    bool should_hide_column(int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger_.is_composite() && column == column_end_of_year_age;
    }

    void render_or_measure_extra_headers
        (wx_table_generator           & table_gen
        ,int                          & pos_y
        ,oenum_render_or_only_measure   output_mode
        ) const override
    {
        std::ostringstream header_zero;
        header_zero
            << "{{InitAnnSepAcctGrossInt_"
            << basis_suffix(get_basis())
            << ir_suffix(interest_rate::zero)
            << "}} Hypothetical Rate of Return*"
            ;

        auto pos_y_copy = pos_y;
        table_gen.output_super_header
            (interpolator_(header_zero.str()).as_html()
            ,column_zero_cash_surr_value
            ,column_separator
            ,pos_y_copy
            ,output_mode
            );

        std::ostringstream header_nonzero;
        header_nonzero
            << "{{InitAnnSepAcctGrossInt_"
            << basis_suffix(get_basis())
            << ir_suffix(interest_rate::non_zero)
            << "}} Hypothetical Rate of Return*"
            ;

        table_gen.output_super_header
            (interpolator_(header_nonzero.str()).as_html()
            ,column_nonzero_cash_surr_value
            ,column_max
            ,pos_y
            ,output_mode
            );

        table_gen.output_horz_separator
            (column_zero_cash_surr_value
            ,column_separator
            ,pos_y
            ,output_mode
            );
        table_gen.output_horz_separator
            (column_nonzero_cash_surr_value
            ,column_max
            ,pos_y
            ,output_mode
            );
        pos_y += table_gen.separator_line_height();
    }
};

class reg_d_indiv_guar_irr : public reg_d_indiv_irr_base
{
  public:
    using reg_d_indiv_irr_base::reg_d_indiv_irr_base;

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

class reg_d_indiv_curr_irr : public reg_d_indiv_irr_base
{
  public:
    using reg_d_indiv_irr_base::reg_d_indiv_irr_base;

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

class reg_d_indiv_curr : public page_with_tabular_report
{
  public:
    using page_with_tabular_report::page_with_tabular_report;

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
            ,{ "PremiumLoad"                , "Premium\nLoads"              , "999,999,999" }
            ,{ "MiscCharges"                , "Admin\nCharges"              , "999,999,999" }
            ,{ "COICharge_Current"          , "Mortality\nCharges"          , "999,999,999" }
            ,{ "SepAcctCharges_Current"     , "Asset\nCharges"              , "999,999,999" }
            ,{ "GrossIntCredited_Current"   , "Investment\nIncome"          , "999,999,999" }
            ,{ "AcctVal_Current"            , "Account\nValue"              , "999,999,999" }
            ,{ "CSVNet_Current"             , "Cash\nSurr Value"            , "999,999,999" }
            ,{ "EOYDeathBft_Current"        , "Death\nBenefit"              , "999,999,999" }
            };

        return columns;
    }

    bool should_hide_column(int column) const override
    {
        // Don't show AttainedAge on a composite.
        return ledger_.is_composite() && column == column_end_of_year_age;
    }

    void render_or_measure_extra_headers
        (wx_table_generator           & table_gen
        ,int                          & pos_y
        ,oenum_render_or_only_measure   output_mode
        ) const override
    {
        table_gen.output_super_header
            (interpolator_
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

/// Individual private placement illustration subject to Reg D.

class pdf_illustration_reg_d_indiv : public pdf_illustration
{
  public:
    explicit pdf_illustration_reg_d_indiv
        (Ledger   const& ledger
        ,fs::path const& pdf_out_file
        )
        :pdf_illustration{ledger, pdf_out_file}
    {
        auto const& invar = ledger.GetLedgerInvariant();

        // Define variables specific to this illustration.

        add_abbreviated_variable("CorpName", 140);
        add_abbreviated_variable("Insured1", 140);

        // Add all the pages.
        numbered_page::start_numbering();
        add<standard_page>("reg_d_indiv_cover_page");
        add<reg_d_indiv_guar_irr>();
        add<reg_d_indiv_curr_irr>();
        add<reg_d_indiv_curr>();
        add<standard_page>("reg_d_indiv_notes1");
        add<standard_page>("reg_d_indiv_notes2");
        if(invar.SupplementalReport)
            {
            add<standard_supplemental_report>("reg_d_indiv_supp_report");
            }
    }

    std::string get_upper_footer_template_name() const override
    {
        return "reg_d_indiv_footer_upper";
    }
};
} // Unnamed namespace.

// Implementing this function in a GUI module lets wxPdfDoc be used.
// Currently, no other interface writes PDF files.

namespace
{
void concrete_pdf_command(Ledger const& ledger, fs::path const& pdf_out_file)
{
    wxBusyCursor reverie;

    switch(ledger.ledger_type())
        {
        case mce_ill_reg:
            pdf_illustration_naic        (ledger, pdf_out_file).render_all();
            break;
        case mce_prospectus_abeyed: // fall through
        case mce_finra:
            pdf_illustration_finra       (ledger, pdf_out_file).render_all();
            break;
        case mce_group_private_placement:
            pdf_illustration_reg_d_group (ledger, pdf_out_file).render_all();
            break;
        case mce_individual_private_placement:
            pdf_illustration_reg_d_indiv (ledger, pdf_out_file).render_all();
            break;
        case mce_offshore_private_placement_obsolete: // fall through
        case mce_ill_reg_private_placement_obsolete:  // fall through
        case mce_variable_annuity_obsolete:
            alarum() << "Unsupported ledger type." << LMI_FLUSH;
        }
}

bool volatile ensure_setup = pdf_command_initialize(concrete_pdf_command);
} // Unnamed namespace.
