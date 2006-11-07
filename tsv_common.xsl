<?xml version="1.0"?>
<!--
    Xsl template used by other templates to render a TSV output.

    Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.

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

    $Id$

    Uses format.xml - column titles, number-formatting and other information.
-->
<!DOCTYPE xsl:stylesheet [
<!ENTITY tab "&#x9;">
<!ENTITY nl "&#xA;">
]>
<xsl:stylesheet xmlns:lmi="http://savannah.nongnu.org/projects/lmi" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xsi:schemaLocation="http://savannah.nongnu.org/projects/lmi schema.xsd">

<xsl:output method="text"/>

<xsl:include href="common.xsl"/>

<!-- Print a non-empty value, and puts '0' for an empty (non-existing) value. -->
<xsl:template name="print_value">
    <xsl:param name="value"/>
    <xsl:choose>
        <xsl:when test="not($value)">
            <xsl:text>0</xsl:text>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="$value"/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<!--
    The template to be called from 'data_table'. See 'data_table' for parameter
    description.
    It generates the tab delimited table of values.
-->
<xsl:template name="do_data_table">
    <xsl:param name="headers"/>
    <xsl:param name="vectors"/>
    <!--
        Table: Headers
    -->
    <xsl:call-template name="do_data_table_pre_headers"/>
    <xsl:for-each select="$headers">
        <xsl:choose>
            <!-- a spacer -->
            <xsl:when test="not(@name)">
                <!-- leave the cell empty for a spacer column -->
            </xsl:when>
            <!-- if the title is specified directly, then use it -->
            <xsl:when test="./text()">
                <xsl:value-of select="./text()"/>
            </xsl:when>
            <!-- otherwise get it from 'format.xml' -->
            <xsl:otherwise>
                <xsl:call-template name="title">
                    <xsl:with-param name="name" select="@name"/>
                    <xsl:with-param name="basis" select="@basis"/>
                    <xsl:with-param name="column" select="."/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
        <xsl:text>&tab;</xsl:text>
    </xsl:for-each>
    <xsl:text>&nl;</xsl:text>
    <!--
        Table: Data
    -->
    <xsl:variable name="is_inforce" select="number($illustration/double_scalar[@name='IsInforce'])"/>
        <xsl:for-each select="$vectors[1]/duration">
            <xsl:variable name="position" select="number(position())"/>
            <xsl:call-template name="do_data_table_pre_data">
                <xsl:with-param name="position" select="$position"/>
            </xsl:call-template>
            <xsl:for-each select="$headers">
                <xsl:variable name="name" select="@name"/>
                <xsl:variable name="basis" select="@basis"/>
                <xsl:choose>
                    <!-- deal with uncommon column cases in here -->
                    <xsl:when test="$is_inforce &gt; 0 and ($name='IrrOnSurrender' or $name='IrrOnDeath')">
                        <xsl:text>(inforce)</xsl:text>
                    </xsl:when>
                    <xsl:when test="$name='InforceLives'">
                        <xsl:value-of select="$vectors[@name=$name]/duration[$position + 1]"/>
                    </xsl:when>
                    <!-- the general case -->
                    <xsl:otherwise>
                        <xsl:choose>
                            <xsl:when test="not($name)">
                                <!-- just an empty cell for a spacer column -->
                            </xsl:when>
                            <xsl:when test="not($basis)">
                                <xsl:call-template name="print_value">
                                    <xsl:with-param name="value" select="$vectors[@name=$name]/duration[$position]"/>
                                </xsl:call-template>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:call-template name="print_value">
                                    <xsl:with-param name="value" select="$vectors[@name=$name][@basis=$basis]/duration[$position]"/>
                                </xsl:call-template>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:otherwise>
                </xsl:choose>
                <xsl:text>&tab;</xsl:text>
            </xsl:for-each>
            <xsl:text>&nl;</xsl:text>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
