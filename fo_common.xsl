<?xml version="1.0" encoding="UTF-8"?>
<!--
    Common part of various illustrations.

    Copyright (C) 2007 Gregory W. Chicares.

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
    email: <chicares@cox.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

    $Id: fo_common.xsl,v 1.17 2007-05-30 17:28:42 etarassov Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:fo="http://www.w3.org/1999/XSL/Format">
  <!--
  In some xsl:for-each loop we could iterate over a node set from some
  other document (not the current document being transformed, for example
  we could iterate over a list of columns from the separate format file
  to generate a list of available columns).

  In such a case inside the loop the root node '/' binds to the root node
  of that external document and we cannot use '/illustration' no longer
  to acces our current xml data.
  Use the global variable '$illustration' to access the data.
  -->
  <xsl:variable name="illustration" select="/illustration"/>
  <xsl:variable name="max-lapse-year-text">
    <xsl:call-template name="get-max-lapse-year"/>
  </xsl:variable>
  <xsl:variable name="max-lapse-year" select="number($max-lapse-year-text)"/>
  <xsl:variable name="supplemental_report" select="$illustration/supplementalreport"/>
  <xsl:variable name="has_supplemental_report" select="boolean($illustration/scalar/SupplementalReport='1')"/>

  <!--
  Frequently used value. For a composite case the variable is_composite is true.
  -->
  <xsl:variable name="is_composite" select="boolean($illustration/scalar/Composite='1')"/>

  <!--
  The two strings below define how the special symbols in are escaped
  when the title text is written into the report.
  Currently only '_' (the underscore character) is translated - into a hard-space.
  For example a text '____Right_Aligned' would be translated into
  '&#xA0;&#xA0;&#xA0;&#xA0;Right&#xA0;Aligned' which will result in the text
  being padded to the right even if the text is aligned to the left.
  -->
  <xsl:variable name="SPECIAL_LETTERS">_</xsl:variable>
  <xsl:variable name="SPECIAL_LETTERS_ESCAPED">&#xA0;</xsl:variable>

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
  NOTE: illustration_reg cases do not currently have LapseYear_CurrentZero and
  LapseYear_GuaranteedZero in their XML. We rely on the fact that a inexistent
  node evaluates to zero in a comparison expression.
  -->

  <xsl:template name="get-max-lapse-year">
    <xsl:call-template name="max-comparison">
      <xsl:with-param name="value1">
        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="$illustration/scalar/LapseYear_Current"/>
          <xsl:with-param name="value2" select="$illustration/scalar/LapseYear_Guaranteed"/>
        </xsl:call-template>
      </xsl:with-param>
      <xsl:with-param name="value2">
        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="$illustration/scalar/LapseYear_CurrentZero"/>
          <xsl:with-param name="value2" select="$illustration/scalar/LapseYear_GuaranteedZero"/>
        </xsl:call-template>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

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
    Originally it has appeared in files with minor differencies such as:
      * explicitly specified text alignment, font size or not specified;
      * paranteses added around the text or omitted;
      * an extra space added just before the text.
  -->
  <xsl:template name="dollar-units">
    <xsl:choose>
      <xsl:when test="$illustration/scalar/ScaleUnit=''">
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in dollars)</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:otherwise>
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in </xsl:text>
          <xsl:value-of select="$illustration/scalar/ScaleUnit"/>
          <xsl:text>s of dollars)</xsl:text>
        </fo:block>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="supplemental-report-body">
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:for-each select="$supplemental_report/columns">
            <fo:table-column/>
          </xsl:for-each>
          <fo:table-header>
            <fo:table-row>
              <xsl:for-each select="$supplemental_report/columns">
                <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                  <fo:block text-align="right">
                    <xsl:value-of select="translate(./title,$SPECIAL_LETTERS,$SPECIAL_LETTERS_ESCAPED)"/>
                  </fo:block>
                </fo:table-cell>
              </xsl:for-each>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell padding="2pt">
                <fo:block/>
              </fo:table-cell>
            </fo:table-row>
          </fo:table-header>
          <!-- Create Supplemental Report Values -->
          <!-- make inforce illustration start in the inforce year -->
          <fo:table-body>
            <xsl:call-template name="supplemental-report-values">
              <xsl:with-param name="counter" select="$illustration/scalar/InforceYear + 1"/>
              <xsl:with-param name="inforceyear" select="0 - $illustration/scalar/InforceYear"/>
            </xsl:call-template>
          </fo:table-body>
        </fo:table>
      </fo:block>
      <xsl:if test="$has_supplemental_report">
        <fo:block id="endofdoc"/>
      </xsl:if>
    </fo:flow>
  </xsl:template>

  <!-- Create Supplemental Report Values -->
  <xsl:template name="supplemental-report-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <xsl:for-each select="$supplemental_report/columns">
          <xsl:variable name="column_name" select="string(./name)"/>
          <fo:table-cell padding=".2pt">
            <fo:block text-align="right">
              <xsl:value-of select="$illustration/data/newcolumn/column[@name=$column_name]/duration[$counter]/@column_value"/>
            </fo:block>
          </fo:table-cell>
        </xsl:for-each>
      </fo:table-row>
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="4pt">
            <fo:block text-align="right" />
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="supplemental-report-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <!--
  Generate table columns list.
  If a column has no name attribute (or an empty one) treat it as a separator.
  -->
  <xsl:template name="generate-table-columns">
    <xsl:param name="columns"/>
    <xsl:for-each select="$columns">
      <fo:table-column>
        <xsl:if test="not(@name) and not(@scalar)">
          <xsl:attribute name="column-width">2mm</xsl:attribute>
        </xsl:if>
      </fo:table-column>
    </xsl:for-each>
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
  A generic template that generates a data-table header rows.
  Simlpy calls 'generate-table-header-row' with initial values.
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
  Output data table header row and recursivly call itself until all rows
  are produces.
  Table header rows are printed using 'generate-table-header-cell' which
  does merging of the header-cells when the adjucent cells have the same text.
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
  Output cell text (and recursivly continue).
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
      <fo:table-cell display-align="after" padding-top="4pt" padding-bottom="2pt" border-bottom-style="solid" border-bottom-color="blue">
        <xsl:if test="$spans != 1">
          <xsl:attribute name="number-columns-spanned">
            <xsl:value-of select="$spans"/>
          </xsl:attribute>
        </xsl:if>
          <xsl:attribute name="text-align">
            <xsl:if test="$spans = 1">right</xsl:if>
            <xsl:if test="$spans != 1">center</xsl:if>
          </xsl:attribute>
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
  A single character that indicates that a column title has to be splitted
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

  <xsl:template name="generate-table-values">
    <xsl:param name="columns"/>
    <xsl:param name="counter"/>
    <xsl:param name="max-counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-counter">
      <fo:table-row>
        <xsl:for-each select="$columns">
          <fo:table-cell padding=".6pt">
            <!-- Add some space if it the first row and some space after each 5th year -->
            <xsl:if test="($counter + $inforceyear) mod 5 = 0">
              <xsl:attribute name="padding-bottom">8pt</xsl:attribute>
            </xsl:if>
            <fo:block text-align="right">
              <xsl:choose>
                <xsl:when test="@name">
                  <xsl:variable name="column_name" select="@name" />
                  <xsl:value-of select="$illustration/data/newcolumn/column[@name=$column_name]/duration[$counter]/@column_value"/>
                </xsl:when>
                <xsl:when test="@scalar">
                  <xsl:variable name="scalar_name" select="@scalar" />
                  <xsl:value-of select="$illustration/scalar/*[name(.)=$scalar_name]"/>
                </xsl:when>
              </xsl:choose>
            </fo:block>
          </fo:table-cell>
        </xsl:for-each>
      </fo:table-row>
      <xsl:call-template name="generate-table-values">
        <xsl:with-param name="columns" select="$columns"/>
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="max-counter" select="$max-counter"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
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
        <xsl:value-of select="substring($passString, 1, $length - string-length($TEXT_ELLIPSIS))"/>
        <xsl:value-of select="$TEXT_ELLIPSIS"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>