<?xml version="1.0" encoding="UTF-8"?>
<!--
    Common part of various illustrations.

    Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA

    http://savannah.nongnu.org/projects/lmi
    email: <gchicares@sbcglobal.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
-->
<!DOCTYPE stylesheet [
<!ENTITY nbsp "&#xA0;">
]>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">

  <!--
  Used in an expression like translate($sometext, $lcletters, $ucletters)
  to transform $sometext into uppercase.
  -->
  <xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>

  <!--
  Strip commas from numbers:
  translate($string_Representation_of_a_number, $numberswc, $numberswoc).
  -->
  <xsl:variable name="numberswoc">0123456789</xsl:variable>
  <xsl:variable name="numberswc">0123456789,</xsl:variable>

  <!--
  Global parameter, that controls the output media size.
  Refer to http://en.wikipedia.org/wiki/Paper_size for details.
  North American paper size standards are accepted:
  'letter', 'legal', 'emperor'.
  Any unknown page size keyword is treated as 'letter'.
  -->
  <xsl:param name="page-type" select="'letter'"/>
  <!-- Page orientation has to be 'portrait' or 'landscape' -->
  <xsl:param name="page-orientation" select="'portrait'"/>

  <!--
  Define this parameter to true to exclude company logo. This should be useful
  for text output where any graphics is replaced by a grid of '#' symbols.
  -->
  <xsl:param name="hide-company-logo" select="boolean(0)"/>

  <!--
  In some xsl:for-each loop we could iterate over a node set from some
  other document (not the current document being transformed, for example
  we could iterate over a list of columns from the separate format file
  to generate a list of available columns).

  In such a case inside the loop the root node '/' binds to the root node
  of that external document and we cannot use '/illustration' no longer
  to access our current xml data.
  Use the global variable '$illustration' to access the data.
  -->
  <xsl:variable name="illustration" select="/illustration"/>

  <!--
  Prepare to use new format of XML input files:
  The $scalars variable below should become
  "/illustration/string_scalar | /illustration/double_scalar"
  -->
  <xsl:variable name="scalars" select="/illustration/scalar"/>

  <!--
  Prepare to use new format of XML input files:
  The $vectors variable below should become
  "/illustration/string_vector | /illustration/double_vector"
  -->
  <xsl:variable name="vectors" select="/illustration/data/newcolumn/column"/>

  <xsl:variable name="max-lapse-year-text">
    <xsl:call-template name="get-max-lapse-year"/>
  </xsl:variable>
  <xsl:variable name="max-lapse-year" select="number($max-lapse-year-text)"/>

  <!--
  Prepare to use new format of XML input files:
  The $supplemental_report variable below should become
  "/illustration/supplementalreport"
  -->
  <xsl:variable name="supplemental_report" select="$illustration/supplementalreport"/>
  <xsl:variable name="NO_SUPPLEMENTAL_COLUMN" select="'[none]'"/>

  <!--
  Prepare to use new format of XML input files:
  The $supplemental_report_columns variable below should become
  "$supplemental_report/column"
  -->
  <xsl:variable name="supplemental_report_columns" select="$supplemental_report/columns"/>

  <xsl:variable name="has_supplemental_report" select="boolean($scalars/SupplementalReport='1')"/>

  <!--
  Frequently used value. For a composite case the variable is_composite is true.
  -->
  <xsl:variable name="is_composite" select="boolean($scalars/Composite='1')"/>

  <!-- Page size in a form of 'width x height' (portrait orientation) -->
  <xsl:variable name="page-size">
    <xsl:choose>
      <xsl:when test="$page-type='legal'">
        8.5in x 14in
      </xsl:when>
      <xsl:when test="$page-type='emperor'">
        48in x 72in
      </xsl:when>
      <xsl:otherwise><!-- fall back to 'letter' -->
        8.5in x 11in
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <!-- These variables should be used in every 'simple-page-master'. -->
  <xsl:variable name="portrait-width" select="normalize-space(substring-before($page-size, 'x'))"/>
  <xsl:variable name="portrait-height" select="normalize-space(substring-after($page-size, 'x'))"/>
  <!-- Account for page orientation in a form 'horizontal x vertical' -->
  <xsl:variable name="page-geometry">
    <xsl:choose>
      <xsl:when test="$page-orientation='landscape'">
        <xsl:value-of select="concat($portrait-height,' x ',$portrait-width)"/>
      </xsl:when>
      <xsl:otherwise><!-- fall back to 'portrait' -->
        <xsl:value-of select="concat($portrait-width,' x ',$portrait-height)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <!-- These variables should be used in every 'simple-page-master'. -->
  <xsl:variable name="page-width" select="normalize-space(substring-before($page-geometry, 'x'))"/>
  <xsl:variable name="page-height" select="normalize-space(substring-after($page-geometry, 'x'))"/>

  <!--
  Helper template to be used in 'fo:simple-page-master' to improve readability.
  Use template parameters 'width' and 'height' to override default page size.
  -->
  <xsl:template name="set-page-size">
    <xsl:param name="width" select="$page-width"/>
    <xsl:param name="height" select="$page-height"/>
    <xsl:attribute name="page-width"><xsl:value-of select="$width"/></xsl:attribute>
    <xsl:attribute name="page-height"><xsl:value-of select="$height"/></xsl:attribute>
    <xsl:attribute name="margin">.25in .25in .1in</xsl:attribute>
  </xsl:template>

  <xsl:variable name="compliance_tracking_string">
    <xsl:choose>
      <xsl:when test="$scalars/IsInforce!='1'">
        <xsl:if test="$scalars/Composite='1'">
          <xsl:value-of select="$scalars/ImprimaturPresaleComposite"/>
        </xsl:if>
        <xsl:if test="$scalars/Composite!='1'">
          <xsl:value-of select="$scalars/ImprimaturPresale"/>
        </xsl:if>
      </xsl:when>
      <xsl:otherwise>
        <xsl:if test="$scalars/Composite='1'">
          <xsl:value-of select="$scalars/ImprimaturInforceComposite"/>
        </xsl:if>
        <xsl:if test="$scalars/Composite!='1'">
          <xsl:value-of select="$scalars/ImprimaturInforce"/>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <!-- Suppose that the number does not contain leading/trailing spaces -->
  <xsl:variable name="compliance_tracking_number" select="normalize-space($compliance_tracking_string)"/>

  <!--
  This may appear to be a clumsy way to get the max value but there is no clean way
  to do this with the available XSL functionality and the values being distinct
  entities. The way it is done in Calculation Summary and all examples to be found
  have the values grouped in a common node within the XML. Simplified example:
  <values>
    <value>55</value>
    <value>45</value>
    <value>50</value>
    <value>40</value>
  </values>
  Note: illustration_reg cases do not currently have LapseYear_CurrentZero and
  LapseYear_GuaranteedZero in their XML. We rely on the fact that an inexistent
  node evaluates to zero in a comparison expression.
  -->
  <xsl:template name="get-max-lapse-year">
    <xsl:call-template name="max-comparison">
      <xsl:with-param name="value1">
        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="$scalars/LapseYear_Current"/>
          <xsl:with-param name="value2" select="$scalars/LapseYear_Guaranteed"/>
        </xsl:call-template>
      </xsl:with-param>
      <xsl:with-param name="value2">
        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="$scalars/LapseYear_CurrentZero"/>
          <xsl:with-param name="value2" select="$scalars/LapseYear_GuaranteedZero"/>
        </xsl:call-template>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <!-- Return the maximum of two values -->
  <xsl:template name="max-comparison">
    <xsl:param name="value1"/>
    <xsl:param name="value2"/>
    <xsl:choose>
      <xsl:when test="$value1 &lt; $value2">
        <xsl:value-of select="$value2"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$value1"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
    Print Dollar Units
    This template is common to all the illustration xsl templates.
    Originally it has appeared in files with minor differences such as:
      * explicitly specified text alignment, font size or not specified;
      * parentheses added around the text or omitted;
      * an extra space added just before the text.
  -->
  <xsl:template name="dollar-units">
    <fo:block text-align="center" font-size="9pt" padding-top="1em">
      <xsl:text>Values shown are in </xsl:text>
      <xsl:choose>
        <xsl:when test="$scalars/ScaleUnit=''">
          <xsl:text>dollars</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="$scalars/ScaleUnit"/>
          <xsl:text>s of dollars</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </fo:block>
  </xsl:template>

  <!-- Generate widely used text snippet 'PrepMonth PrepDay, PrepYear' -->
  <xsl:template name="date-prepared">
    <xsl:value-of select="$scalars/PrepMonth"/>
    <xsl:text>&nbsp;</xsl:text>
    <xsl:value-of select="$scalars/PrepDay"/>
    <xsl:text>, </xsl:text>
    <xsl:value-of select="$scalars/PrepYear"/>
  </xsl:template>

  <!--
  Helper 'attributes' template for a table cell.
  Included inside a fo:table-cell, this template adds common cell padding,
  text alignment (align text toward bottom-right cell corner).
  Also set (possible) bottom border properties.
  -->
  <xsl:template name="header-cell">
    <xsl:attribute name="display-align">after</xsl:attribute>
    <xsl:attribute name="padding-top">4pt</xsl:attribute>
    <xsl:attribute name="padding-bottom">2pt</xsl:attribute>
    <xsl:attribute name="text-align">right</xsl:attribute>
    <xsl:attribute name="border-bottom-style">solid</xsl:attribute>
    <xsl:attribute name="border-bottom-color">#002F6C</xsl:attribute>
  </xsl:template>
  <!--
  Same as 'header-cell', but add a bottom border of 1pt.
  -->
  <xsl:template name="header-cell-with-border">
    <xsl:call-template name="header-cell"/>
    <xsl:attribute name="border-bottom-width">1pt</xsl:attribute>
  </xsl:template>

  <!-- Generate supplemental report data table -->
  <xsl:template name="supplemental-report-body">
    <xsl:if test="count($supplemental_report_columns) &gt; 0">
      <fo:flow flow-name="xsl-region-body">
        <fo:block font-size="9pt" font-family="serif">
          <fo:table table-layout="fixed" width="100%">
            <xsl:call-template name="generate-table-columns">
              <xsl:with-param name="columns" select="$supplemental_report_columns"/>
            </xsl:call-template>
            <!--
            TODO ?? Use 'generate-table-header' generic template
            to print table column headers.
            -->
            <fo:table-header>
              <fo:table-row>
                <xsl:for-each select="$supplemental_report_columns">
                  <fo:table-cell>
                    <xsl:call-template name="header-cell-with-border"/>
                    <fo:block>
                      <xsl:call-template name="text-word-wrap">
                        <xsl:with-param name="text" select="./title"/>
                      </xsl:call-template>
                    </fo:block>
                  </fo:table-cell>
                </xsl:for-each>
              </fo:table-row>
              <!-- Additional empty row to add spacing after the header -->
              <fo:table-row>
                <fo:table-cell padding="2pt">
                  <fo:block/>
                </fo:table-cell>
              </fo:table-row>
            </fo:table-header>
            <!-- Supplemental report table body -->
            <fo:table-body>
              <xsl:call-template name="generate-table-values">
                <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
                <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
                <xsl:with-param name="columns" select="$supplemental_report_columns"/>
                <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              </xsl:call-template>
            </fo:table-body>
          </fo:table>
        </fo:block>
        <xsl:if test="$has_supplemental_report">
          <fo:block id="endofdoc"/>
        </xsl:if>
      </fo:flow>
    </xsl:if>
  </xsl:template>

  <!--
  Generate table columns list.
  If a column has no name attribute (or an empty one) treat it
  as a small separator. Also treat a <spacer/> node as a wide separator.
  -->
  <xsl:template name="generate-table-columns">
    <xsl:param name="columns"/>
    <xsl:for-each select="$columns">
      <xsl:variable name="empty_column" select="boolean(not(./name) and not(@name) and not(@scalar) and not(@special))"/>
      <xsl:variable name="is_spacer" select="name() = 'spacer'"/>
      <fo:table-column>
        <xsl:choose>
          <xsl:when test="$empty_column and not($is_spacer)">
            <xsl:attribute name="column-width">proportional-column-width(33)</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="column-width">proportional-column-width(100)</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
      </fo:table-column>
    </xsl:for-each>
    <fo:table-column column-width="proportional-column-width(1)"/>
  </xsl:template>

  <!--
  Determine the minimum number of rows needed to correctly show column headers.
  Every column can spawn over multiple rows. This template return the maximum
  number of rows.
  -->
  <xsl:template name="get-max-header-rows">
    <xsl:param name="columns"/>
    <xsl:param name="column"/>
    <xsl:param name="row"/>
    <xsl:choose>
      <xsl:when test="count($columns) = 0">0</xsl:when>
      <xsl:when test="($row = 1) and ($column = count($columns))">1</xsl:when>
      <xsl:otherwise>
        <xsl:variable name="text">
          <xsl:call-template name="get-text-nth-line">
            <xsl:with-param name="text" select="$columns[$column]"/>
            <xsl:with-param name="n" select="$row"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="this_result">
          <xsl:if test="$text=''">0</xsl:if>
          <xsl:if test="$text!=''"><xsl:value-of select="$row"/></xsl:if>
        </xsl:variable>
        <xsl:variable name="recursive_result">
          <xsl:choose>
            <xsl:when test="($column != count($columns))">
              <xsl:call-template name="get-max-header-rows">
                <xsl:with-param name="columns" select="$columns"/>
                <xsl:with-param name="column" select="($column + 1)"/>
                <xsl:with-param name="row" select="$row"/>
              </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
              <xsl:call-template name="get-max-header-rows">
                <xsl:with-param name="columns" select="$columns"/>
                <xsl:with-param name="column" select="1"/>
                <xsl:with-param name="row" select="($row - 1)"/>
              </xsl:call-template>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:variable>

        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="number(normalize-space($this_result))"/>
          <xsl:with-param name="value2" select="number(normalize-space($recursive_result))"/>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
  A generic template that generates data-table header rows.
  Simply calls 'generate-table-header-row' with initial values.
  Additionally adds a blank header row separating the header cells from
  value cells.
  -->
  <xsl:template name="generate-table-headers">
    <xsl:param name="columns"/>
    <xsl:variable name="max-rows">
      <xsl:call-template name="get-max-header-rows">
        <xsl:with-param name="columns" select="$columns"/>
        <xsl:with-param name="column" select="1"/>
        <xsl:with-param name="row" select="10"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:call-template name="generate-table-header-row">
      <xsl:with-param name="columns" select="$columns"/>
      <xsl:with-param name="row" select="1"/>
      <xsl:with-param name="max-rows" select="number(normalize-space($max-rows))"/>
    </xsl:call-template>
    <!--
    TODO ?? This monstrosity adds an extra space between table headers
    and table data.
    -->
    <fo:table-row>
      <fo:table-cell padding="2pt">
        <fo:block/>
      </fo:table-cell>
    </fo:table-row>
  </xsl:template>

  <!--
  Output data table header row and recursively call itself until all rows
  are produced.
  Table header rows are printed using 'generate-table-header-cell' which
  does merging of the header-cells when the adjacent cells have the same text.
  See 'generate-table-header-cell' description for an explanation.
  -->
  <xsl:template name="generate-table-header-row">
    <xsl:param name="columns"/>
    <xsl:param name="row"/>
    <xsl:param name="max-rows"/>
    <xsl:if test="count($columns) &gt; 0">
      <fo:table-row>
        <xsl:call-template name="generate-table-header-cell">
          <xsl:with-param name="columns" select="$columns"/>
          <xsl:with-param name="row" select="$row"/>
          <xsl:with-param name="last_row" select="boolean($row = $max-rows)"/>
          <xsl:with-param name="cell" select="1"/>
        </xsl:call-template>
      </fo:table-row>
      <xsl:if test="$row &lt; $max-rows">
        <xsl:call-template name="generate-table-header-row">
          <xsl:with-param name="columns" select="$columns"/>
          <xsl:with-param name="row" select="$row + 1"/>
          <xsl:with-param name="max-rows" select="$max-rows"/>
        </xsl:call-template>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <!--
  Output cell text (and recursively continue).
  The main complexity comes from rendering the common header cell prefix
  spawned over multiple columns. Example:
    <column>Policy |Year</column>
    <column>Benefit|Guaranteed</column>
    <column>Benefit|Current</column>
  will be rendered like so (ASCII graphics):
    /===============================\
    | Policy |       Benefit        |
    |========+======================|
    |   Year | Guaranteed | Current |
    |========+============+=========|
    |          [table data]         |
  Note how the common header cell 'Benefit' is spawned over two columns.
  -->
  <xsl:template name="generate-table-header-cell">
    <xsl:param name="columns"/>
    <xsl:param name="row"/>
    <xsl:param name="last_row"/>
    <xsl:param name="cell"/>
    <xsl:param name="spans" select="1"/>
    <xsl:variable name="cell_text">
      <xsl:call-template name="get-text-nth-line">
        <xsl:with-param name="text" select="string($columns[$cell])"/>
        <xsl:with-param name="n" select="$row"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="next_cell_text">
      <xsl:if test="$cell &lt; count($columns)">
        <xsl:call-template name="get-text-nth-line">
          <xsl:with-param name="text" select="string($columns[$cell + 1])"/>
          <xsl:with-param name="n" select="$row"/>
        </xsl:call-template>
      </xsl:if>
    </xsl:variable>
    <xsl:if test="($cell_text != $next_cell_text) or ($cell = count($columns))">
      <fo:table-cell number-columns-spanned="{$spans}">
        <xsl:call-template name="header-cell"/>
        <xsl:if test="$spans != 1">
          <xsl:attribute name="text-align">center</xsl:attribute>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="$last_row or (($spans &gt; 1) and ($cell_text != ''))">
            <xsl:attribute name="border-bottom-width">1pt</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
            <xsl:attribute name="border-bottom-width">0pt</xsl:attribute>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:call-template name="text-word-wrap">
          <xsl:with-param name="text" select="$cell_text"/>
        </xsl:call-template>
      </fo:table-cell>
    </xsl:if>
    <xsl:if test="$cell &lt; count($columns)">
      <xsl:choose>
        <xsl:when test="$cell_text = $next_cell_text">
          <xsl:call-template name="generate-table-header-cell">
            <xsl:with-param name="columns" select="$columns"/>
            <xsl:with-param name="row" select="$row"/>
            <xsl:with-param name="last_row" select="$last_row"/>
            <xsl:with-param name="cell" select="$cell + 1"/>
            <xsl:with-param name="spans" select="$spans + 1"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="generate-table-header-cell">
            <xsl:with-param name="columns" select="$columns"/>
            <xsl:with-param name="row" select="$row"/>
            <xsl:with-param name="last_row" select="$last_row"/>
            <xsl:with-param name="cell" select="$cell + 1"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:template>

  <!--
  A single character indicates that a column title has to be spawned over
  multiple cells in the table header. Example:
    <column>Policy |Year</column>
    <column>Benefit|Current</column>
  will be rendered like so (ASCII graphics):
    /==================\
    | Policy | Benefit |
    |========+=========|
    |   Year | Current |
    |========+=========|
    |   [table data]   |
  -->
  <xsl:variable name="CELL_WRAPPER" select="'|'"/>

  <xsl:template name="get-text-nth-line">
    <!-- The text to split into lines -->
    <xsl:param name="text"/>
    <!-- Index (starting from 1) of the line to return -->
    <xsl:param name="n"/>
    <xsl:choose>
      <xsl:when test="starts-with($text, ' ')">
        <!-- chop off the leading junk character -->
        <xsl:call-template name="get-text-nth-line">
          <xsl:with-param name="text" select="substring($text, 2)"/>
          <xsl:with-param name="n" select="$n"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="($n = 1) and contains($text, $CELL_WRAPPER)">
        <xsl:value-of select="normalize-space(substring-before($text, $CELL_WRAPPER))"/>
      </xsl:when>
      <xsl:when test="($n = 1)"><!-- and not(contains($text, $CELL_WRAPPER)) -->
        <xsl:value-of select="normalize-space($text)"/>
      </xsl:when>
      <xsl:when test="contains($text, $CELL_WRAPPER)"><!-- and ($n != 1) -->
        <xsl:call-template name="get-text-nth-line">
          <xsl:with-param name="text" select="substring-after($text, $CELL_WRAPPER)"/>
          <xsl:with-param name="n" select="($n - 1)"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise><!-- ($n != 1) and not(contains($text, $CELL_WRAPPER)) -->
        <!-- The text does not have nth line. Return nothing -->
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
  A single character that indicates that a column title has to be split
  in this place into two separate lines.
  For example if a column has title 'Policy _Year', then the output file
  will contain:
    <fo:block>Policy</fo:block>
    <fo:block>Year</fo:block>
  Which will result in two separate lines.
  Note: TEXT_LINE_WRAPPER must be exactly one symbol.
  -->
  <xsl:variable name="TEXT_LINE_WRAPPER" select="'_'"/>
  <!--
  Prepare the column title for a table-header cell - split it into lines using
  the special symbol $TEXT_LINE_WRAPPER (underscore by default), and wrap-up
  each line into <fo:block> so that FOP does not change word wrapping.
  This way we control the way the column titles are shown.
  -->
  <xsl:template name="text-word-wrap">
    <!-- The text to split into lines -->
    <xsl:param name="text"/>
    <!-- Do we treat the leading $TEXT_LINE_WRAPPERs as junk? -->
    <xsl:param name="is_dirty" select="boolean(0)"/>
    <xsl:choose>
      <xsl:when test="starts-with($text, ' ') or ($is_dirty and starts-with($text, $TEXT_LINE_WRAPPER))">
        <!-- chop off the leading junk character -->
        <xsl:call-template name="text-word-wrap">
          <xsl:with-param name="text" select="substring($text, 2)"/>
          <xsl:with-param name="is_dirty" select="boolean($is_dirty) and starts-with($text, $TEXT_LINE_WRAPPER)"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="contains($text, $TEXT_LINE_WRAPPER)">
        <xsl:call-template name="text-word-wrap">
          <xsl:with-param name="text" select="substring-before($text, $TEXT_LINE_WRAPPER)"/>
          <xsl:with-param name="is_dirty" select="boolean(1)"/>
        </xsl:call-template>
        <xsl:call-template name="text-word-wrap">
          <xsl:with-param name="text" select="substring-after($text, $TEXT_LINE_WRAPPER)"/>
          <xsl:with-param name="is_dirty" select="boolean(1)"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise><!-- not(contains($text, $TEXT_LINE_WRAPPER)) -->
        <!-- Finally wrap the line in the <fo:block> tags -->
        <fo:block><xsl:value-of select="normalize-space($text)"/></fo:block>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
  Print out table body. Each column from $columns defines the data source
  for the table column.
    - column/@name   - values are read from $vectors[@name=$column_name]
    - column/name    - same as above for compatibility with supplemental reports
    - column/@scalar - values are read from $scalars/*[name(.)=$scalar_name]
    - column/@special - call 'get-special-column-value' to get values - it is
                        a hook for any special case, i.e. a column
                        which is not present in the input XML, but is calculated
                        on the fly.
  Parameters:
    - columns       - nodeset containing table-columns meta information.
    - counter       - row counter (incrementing with each table data row)
    - max-counter   - upper limit for 'counter' (including)
    - inforceyear   - alignment parameter for 'counter'
    - special-param - an untouched value passed through
                      directly to 'get-special-column-value'
  -->
  <xsl:template name="generate-table-values">
    <xsl:param name="columns"/>
    <xsl:param name="counter"/>
    <xsl:param name="max-counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:param name="special-param" select="NaN"/>
    <xsl:param name="add-dummy-row-if-empty" select="boolean(1)"/>
    <xsl:if test="$counter &lt;= $max-counter">
      <fo:table-row>
        <xsl:for-each select="$columns">
          <fo:table-cell padding="1pt .5pt 0">
            <!-- Add some space after each 5th year -->
            <xsl:if test="($counter + $inforceyear) mod 5 = 0">
              <!-- but only if it's not the last row of the table -->
              <xsl:if test="$counter &lt; $max-counter">
                <xsl:attribute name="padding-bottom">8pt</xsl:attribute>
              </xsl:if>
            </xsl:if>
            <fo:block text-align="right">
              <xsl:choose>
                <xsl:when test="./name">
                  <xsl:variable name="column_name" select="string(./name)"/>
                  <xsl:value-of select="$vectors[@name=$column_name]/duration[$counter]/@column_value"/>
                </xsl:when>
                <xsl:when test="@name">
                  <xsl:variable name="column_name" select="@name"/>
                  <xsl:value-of select="$vectors[@name=$column_name]/duration[$counter]/@column_value"/>
                </xsl:when>
                <xsl:when test="@scalar">
                  <xsl:variable name="scalar_name" select="@scalar"/>
                  <xsl:value-of select="$scalars/*[name(.)=$scalar_name]"/>
                </xsl:when>
                <xsl:when test="@special">
                  <xsl:call-template name="get-special-column-value">
                    <xsl:with-param name="special" select="@special"/>
                    <xsl:with-param name="column" select="."/>
                    <xsl:with-param name="counter" select="$counter"/>
                    <xsl:with-param name="special-param" select="$special-param"/>
                  </xsl:call-template>
                </xsl:when>
              </xsl:choose>
            </fo:block>
          </fo:table-cell>
        </xsl:for-each>
        <!--
        Ensure that a group of 5 rows is never split across multiple pages.
        Add a dummy cell, that spans 5 rows. Since FOP avoids breaking cells,
        this cell remains on one page, so will the group of 5 rows.
        -->
        <xsl:if test="($counter + $inforceyear) mod 5 = 1">
          <fo:table-cell number-rows-spanned="5">
            <fo:block/>
          </fo:table-cell>
        </xsl:if>
      </fo:table-row>
      <xsl:call-template name="generate-table-values">
        <xsl:with-param name="columns" select="$columns"/>
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="max-counter" select="$max-counter"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
        <xsl:with-param name="special-param" select="$special-param"/>
      </xsl:call-template>
    </xsl:if>
    <!--
    If there is no data at all, then FOP would complain because
    the specification does not allow empty <fo:table-body/> tag.
    As a workaround add an empty row.
    -->
    <xsl:if test="not($max-counter) and $add-dummy-row-if-empty">
      <fo:table-row>
        <fo:table-cell>
          <fo:block/>
        </fo:table-cell>
      </fo:table-row>
    </xsl:if>
  </xsl:template>

  <xsl:variable name="TEXT_ELLIPSIS" select="'...'"/>
  <!--
  Truncate string $passString if its length is more than $length.
  -->
  <xsl:template name="limitstring">
    <xsl:param name="passString"/>
    <xsl:param name="length"/>
    <xsl:choose>
      <xsl:when test="string-length($passString) &lt;= $length">
        <xsl:value-of select="$passString"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="substring($passString, 1, $length)"/>
        <xsl:value-of select="$TEXT_ELLIPSIS"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="company-logo">
    <xsl:if test="not($hide-company-logo)">
      <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
    </xsl:if>
  </xsl:template>

  <!-- Generic cover page -->
  <xsl:template name="generic-cover">
    <fo:page-sequence master-reference="cover" force-page-count="no-force">
      <fo:flow flow-name="xsl-region-body">
        <fo:block border="2pt solid #002F6C" font-size="14pt" font-family="sans-serif" text-align="center">

          <fo:block font-size="20pt" font-weight="bold" padding-top="5em">
            <xsl:value-of select="$scalars/PolicyMktgName"/>
          </fo:block>
          <fo:block font-size="20pt" font-weight="bold">
            <xsl:choose>
              <xsl:when test="$scalars/IsInforce!='1'">
                Life Insurance Illustration
              </xsl:when>
              <xsl:otherwise>
                In Force Life Insurance Illustration
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>

          <fo:block font-weight="bold" padding-top="8em">
            Prepared for:
          </fo:block>
          <fo:block padding-top="1em" margin="0 .15in">
            <xsl:variable name="prepared-for-raw">
              <xsl:choose>
                <xsl:when test="not($is_composite)">
                  <xsl:value-of select="$scalars/Insured1"/>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:value-of select="$scalars/CorpName"/>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:variable>
            <xsl:variable name="prepared-for" select="normalize-space($prepared-for-raw)"/>
            <!-- Properly adjust for long user input strings: limit output to 140 characters for approximately two lines. -->
            <xsl:call-template name="limitstring">
              <xsl:with-param name="passString" select="$prepared-for"/>
              <xsl:with-param name="length" select="140"/>
            </xsl:call-template>
            <xsl:if test="string-length($prepared-for) &lt; 70">
              <fo:block padding=".5em"/>
            </xsl:if>
            <xsl:if test="string-length($prepared-for) = 0">
              <fo:block padding=".5em"/>
            </xsl:if>
          </fo:block>

          <fo:block text-align="center">
            <fo:block font-weight="bold" padding="4em 0 1em">
              Presented by:
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/ProducerName"/>
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/ProducerStreet"/>
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/ProducerCity"/>
            </fo:block>
            <fo:block padding-top="2em">
              <xsl:call-template name="date-prepared"/>
            </fo:block>
          </fo:block>

          <fo:block padding="9em 0 4em">
            <xsl:call-template name="company-logo"/>
          </fo:block>

          <fo:block font-size="9pt" padding-bottom="1em">
            <xsl:value-of select="$scalars/MarketingNameFootnote"/>
          </fo:block>
        </fo:block>
      </fo:flow>
    </fo:page-sequence>
  </xsl:template>

  <!--
  Helper template - specify page footer using less code.
  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
  |                 $top-block                 |
  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
  |               $subtop-block                |
  |============================================|
  | $left-block | $center-block | $right-block |
  \~~~~~~~~~~~-~+~~~~~~~~~~~~~~~+~~~~~~~~~~~~~~/
  -->
  <xsl:template name="generic-footer">
    <xsl:param name="top-block" select="''"/>
    <xsl:param name="subtop-block" select="''"/>
    <xsl:param name="left-block" select="''"/>
    <xsl:param name="center-block" select="''"/>
    <xsl:param name="right-block" select="''"/>
    <fo:static-content flow-name="xsl-region-after">
      <fo:block font-size="8pt" font-family="sans-serif" text-align="left">
        <xsl:if test="$top-block">
          <fo:block padding=".5em 0">
            <xsl:copy-of select="$top-block"/>
          </fo:block>
        </xsl:if>
        <fo:block padding-top=".5em" border-top-style="solid" border-top-width="1pt" border-top-color="#002F6C">
          <xsl:if test="$subtop-block">
            <fo:block padding=".5em 0">
              <xsl:copy-of select="$subtop-block"/>
            </fo:block>
          </xsl:if>
          <fo:table table-layout="fixed" width="100%">
            <fo:table-column column-width="proportional-column-width(1)"/>
            <fo:table-column column-width="proportional-column-width(1)"/>
            <fo:table-column column-width="proportional-column-width(1)"/>
            <fo:table-body>
              <fo:table-row>
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:copy-of select="$left-block"/>
                  </fo:block>
                </fo:table-cell>
                <fo:table-cell>
                  <fo:block text-align="center">
                    <xsl:copy-of select="$center-block"/>
                  </fo:block>
                </fo:table-cell>
                <fo:table-cell>
                  <fo:block text-align="right">
                    <xsl:copy-of select="$right-block"/>
                  </fo:block>
                </fo:table-cell>
              </fo:table-row>
            </fo:table-body>
          </fo:table>
        </fo:block>
      </fo:block>
    </fo:static-content>
  </xsl:template>

<xsl:template name="print-mastercontractnumber-and-contractnumber">
  <xsl:param name="omit_policynumber" select="$is_composite"/>
    <!--
    This monstrosity truncates 'MasterContractNumber' and 'ContractNumber' to:
    * 30 characters if both are present;
    * 15 characters if only one is present.
    -->
    <xsl:variable name="has_master_contract" select="number($scalars/MasterContractNumber!='')"/>
    <xsl:variable name="has_polnumber" select="number($scalars/ContractNumber!='' and not($omit_policynumber))"/>
    <xsl:variable name="contracts" select="$has_master_contract + $has_polnumber"/>
    <xsl:if test="$contracts">
      <xsl:variable name="number_length" select="floor(30 div $contracts)"/>
      <fo:block>
        <xsl:if test="$has_master_contract">
          Master contract:
          <xsl:call-template name="limitstring">
            <xsl:with-param name="passString" select="$scalars/MasterContractNumber"/>
            <xsl:with-param name="length" select="$number_length"/>
          </xsl:call-template>
        </xsl:if>
        <xsl:if test="$has_polnumber">
          Contract number:
          <xsl:call-template name="limitstring">
            <xsl:with-param name="passString" select="$scalars/ContractNumber"/>
            <xsl:with-param name="length" select="$number_length"/>
          </xsl:call-template>
        </xsl:if>
      </fo:block>
    </xsl:if>
  </xsl:template>

  <!-- Print 'Page n of N' text snippet. -->
  <xsl:template name="page-of">
    Page <fo:page-number/> of <fo:page-number-citation ref-id="endofdoc"/>
  </xsl:template>

  <!-- Empty stub for special table column values retrieval hook template -->
  <xsl:template name="get-special-column-value"/>

  <!--
  Custom table headers for 'illustration-assumption-report'.
  The special feature requested: the cell 'Net Crediting Rate'
  to be nicely centered over two cells.
  -->
  <xsl:template name="illustration-assumption-custom-headers">
    <fo:table-row>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>Policy</fo:block>
        <fo:block>Year</fo:block>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>End of</fo:block>
        <fo:block>Year Age</fo:block>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block/>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>Sep Acct Net</fo:block>
        <fo:block>Inv Rate</fo:block>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>Gen Acct</fo:block>
        <fo:block>Current Rate</fo:block>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>M&amp;E</fo:block>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>Indiv</fo:block>
        <fo:block>Pmt Mode</fo:block>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>Corp</fo:block>
        <fo:block>Pmt Mode</fo:block>
      </fo:table-cell>
      <fo:table-cell number-rows-spanned="2">
        <xsl:call-template name="header-cell-with-border"/>
        <fo:block>Assumed</fo:block>
        <fo:block>Loan Interest</fo:block>
      </fo:table-cell>
    </fo:table-row>
    <fo:table-row>
      <fo:table-cell padding="2pt">
        <fo:block/>
      </fo:table-cell>
    </fo:table-row>
  </xsl:template>

</xsl:stylesheet>
