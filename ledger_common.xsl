<?xml version="1.0"?>
<!--
    The common part of Xsl templates used to render illustration data into various formats.

    Copyright (C) 2006 Gregory W. Chicares.

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

    $Id: ledger_common.xsl,v 1.2 2006-11-30 05:10:33 chicares Exp $

-->

<xsl:stylesheet xmlns:lmi="http://savannah.nongnu.org/projects/lmi" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xsi:schemaLocation="http://savannah.nongnu.org/projects/lmi schema.xsd">

    <xsl:param name="debug"/>

    <!--
        Print this message if title is not found for a column.
        TODO ?? find a way to conditionally use it only in the debug mode
    -->
    <xsl:variable name="no_title_error">
        <xsl:if test="$debug">
            Title is not defined for :
        </xsl:if>
    </xsl:variable>

    <!--
        Read formatting specifications from a separate file.
        Used in 'title' and 'get_column_format' templates.
    -->
    <xsl:variable name="columns_format_info" select="document('format.xml')/lmi:columns/lmi:column"/>

    <!--
        An empty node. Pass it as initial empty nodeset to a template
        when recursivly constructing a nodeset value.
    -->
    <xsl:variable name="empty_nodeset" select="/a_dummy_nonexisten_node"/>

    <!--
        An empty string node. Use it when a dummy value should be passed.
    -->
    <xsl:variable name="dummy_node" select="''"/>

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

    <!--
        The calculation summary columns which come from 'configurable_settings.xml'.
    -->
    <xsl:variable name="calculation_summary_columns" select="/illustration/calculation_summary_columns/column"/>

    <!--
        The supplemental report columns. Keep it here so that whenever
        we decide to purge spacers or to sort columns, we do it in one place
        for all the xsl templates uniformly.
    -->
    <xsl:variable name="supplemental_columns" select="/illustration/supplemental_report/column | /illustration/supplemental_report/spacer"/>

    <!--
        Print column title.
        - 'name'   - column name ('name_type', required)
        - 'basis'  - column basis ('basis_type', optional)
        - 'column' - a node that comes from 'supplemental_report'
                     or from column list. If its content is not empty,
                     then it will be used as a title.
    -->
    <xsl:template name="title">
        <xsl:param name="name"/>
        <xsl:param name="basis"/>
        <xsl:param name="column"/>
        <xsl:choose>
            <xsl:when test="$column/text()">
                <xsl:value-of select="$column/text()"/>
            </xsl:when>
            <xsl:when test="not($name)">
                <!-- An empty column. Put nothing for a spacer. -->
            </xsl:when>
            <xsl:when test="$basis">
                <xsl:call-template name="do_title">
                    <xsl:with-param name="name" select="$name"/>
                    <xsl:with-param name="basis" select="$basis"/>
                    <xsl:with-param name="title" select="$columns_format_info[@name=$name]/lmi:title[@basis=$basis]/text()"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:call-template name="do_title">
                    <xsl:with-param name="name" select="$name"/>
                    <xsl:with-param name="basis" select="$basis"/>
                    <xsl:with-param name="title" select="$columns_format_info[@name=$name]/lmi:title/text()"/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!--
        Helper for "title" template.
        Shows an error message if '$no_title_error' is defined.
    -->
    <xsl:template name="do_title">
        <xsl:param name="name"/>
        <xsl:param name="basis"/>
        <xsl:param name="title"/>
        <xsl:if test="$title and $title!=''">
            <xsl:value-of select="$title"/>
        </xsl:if>
        <xsl:if test="(not($title) or $title='') and $debug">
            <!-- no title, show error -->
            <xsl:value-of select="$no_title_error"/>
            <xsl:text>[</xsl:text>
            <xsl:value-of select="$name"/>
            <xsl:if test="$basis">
                <xsl:text>_</xsl:text>
                <xsl:value-of select="$basis"/>
            </xsl:if>
            <xsl:text>]</xsl:text>
        </xsl:if>
    </xsl:template>

    <!--
        Calculate the maximum LapseYear. It determines the number of rows in the data table
        for the calculation summary.
    -->
    <xsl:variable name="lapse_year_nodes" select="$illustration/double_scalar[@name='LapseYear']"/>
    <xsl:template name="get_max_lapse_year">
        <xsl:param name="max_value"/>
        <xsl:param name="pos"/>
        <xsl:choose>
            <xsl:when test="$pos &gt; count($lapse_year_nodes)">
                <xsl:value-of select="$value"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="ly" select="number($lapse_year_nodes[$pos])"/>
                <xsl:choose>
                    <xsl:when test="$value &gt;= $ly">
                        <xsl:call-template name="get_max_lapse_year">
                            <xsl:with-param name="value" select="$value"/>
                            <xsl:with-param name="pos" select="$pos+1"/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:call-template name="get_max_lapse_year">
                            <xsl:with-param name="value" select="$ly"/>
                            <xsl:with-param name="pos" select="$pos+1"/>
                        </xsl:call-template>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <xsl:variable name="max_lapse_year_text">
        <xsl:call-template name="get_max_lapse_year">
            <xsl:with-param name="value" select="0"/>
            <xsl:with-param name="pos" select="1"/>
        </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="max_lapse_year" select="number($max_lapse_year_text)"/>

    <!--
        Double vector node carrying the first column values for the calculation
        summary table.
    -->
    <xsl:variable name="policy_year" select="$illustration/double_vector[@name='PolicyYear']"/>

    <!--
        The template wrapper used to prepare nodeset variable for a table data
        output. It prepares variables described below and calls a user-defined
        template 'do_data_table' which has to be defined in a user template.

        This template constructs two variables 'columns' and 'vectors'.
        It calls recursivly itself, using '$pos' as a loop counter, until
        '$pos > count($columns)'.
        - $pos     - loop counter
        - $columns - the list of all columns to be rendered
        - $headers - the nodeset of accepted solumns that will be rendered
        - $vectors - the nodeset of 'string_vector's and 'double_vectors'
                     corresponding exactly to the headers nodeset.

        When both lists are constructed the template calls 'do_data_table'
        template passing it newly constructed $headers and $vectors.

        'do_data_table' is implementation specific and has to be defined
        in the calling template.

        Note: the order of nodes in the resulting nodeset is arbitrary(!).
        Therefore additional efforts are needed in the calling template
        to find the correspondence between '$headers' and '$vectors'.
    -->
    <xsl:template name="data_table">
        <xsl:param name="pos"/>
        <xsl:param name="columns"/>
        <xsl:param name="headers"/>
        <xsl:param name="vectors"/>
        <xsl:choose>
            <xsl:when test="$pos &gt; count($columns)">
                <!-- Call the user-defined template 'do_data_table'. -->
                <xsl:call-template name="do_data_table">
                    <xsl:with-param name="headers" select="$headers"/>
                    <xsl:with-param name="vectors" select="$vectors"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:variable name="name" select="$columns[$pos]/@name"/>
                <xsl:variable name="basis" select="$columns[$pos]/@basis"/>
                <xsl:choose>
                    <!-- a spacer -->
                    <xsl:when test="not($name)">
                        <xsl:call-template name="data_table">
                            <xsl:with-param name="pos" select="$pos + 1"/>
                            <xsl:with-param name="columns" select="$columns"/>
                            <xsl:with-param name="headers" select="$headers | $columns[$pos]"/>
                            <!-- Use $columns[$pos] as a dummy node to push into '$vectors' -->
                            <xsl:with-param name="vectors" select="$vectors | $columns[$pos]"/>
                        </xsl:call-template>
                    </xsl:when>
                    <!-- column with without no basis -->
                    <xsl:when test="not($basis)">
                        <xsl:call-template name="data_table">
                            <xsl:with-param name="pos" select="$pos + 1"/>
                            <xsl:with-param name="columns" select="$columns"/>
                            <xsl:with-param name="headers" select="$headers | $columns[$pos]"/>
                            <xsl:with-param name="vectors" select="$vectors | $illustration/*[@name=$name]"/>
                        </xsl:call-template>
                    </xsl:when>
                    <!-- column name and basis are specified -->
                    <xsl:otherwise>
                        <xsl:call-template name="data_table">
                            <xsl:with-param name="pos" select="$pos + 1"/>
                            <xsl:with-param name="columns" select="$columns"/>
                            <xsl:with-param name="headers" select="$headers | $columns[$pos]"/>
                            <xsl:with-param name="vectors" select="$vectors | $illustration/*[@name=$name][@basis=$basis]"/>
                        </xsl:call-template>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>

