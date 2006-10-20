<?xml version="1.0"?>
<!--
    Xsl template used to render illustration data into html.

    Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    http://savannah.nongnu.org/projects/lmi
    email: <chicares@cox.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

    $Id: html.xsl,v 1.1.2.1 2006-10-20 17:46:02 etarassov Exp $

    Uses format.xml - column titles, number-formatting and other information.
-->
<xsl:stylesheet version="1.0"
	      xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="html" encoding="iso-8859-1" indent="yes" />

<xsl:include href="common.xsl" />

<xsl:variable name="show_columns_xml">
    <column name="Outlay" />
    <column name="AcctVal"     basis="run_guar_basis" />
    <column name="CSVNet"      basis="run_guar_basis" />
    <column name="EOYDeathBft" basis="run_guar_basis" />
    <column name="AcctVal"     basis="run_curr_basis" />
    <column name="CSVNet"      basis="run_curr_basis" />
    <column name="EOYDeathBft" basis="run_curr_basis" />
</xsl:variable>

<xsl:variable name="show_columns" select="document('')/xsl:stylesheet/xsl:variable[@name='show_columns_xml']/column" />

<xsl:variable name="illustration" select="/illustration" />

<xsl:template name="replace_space_by_line_breaks">
    <xsl:variable name="br"><br /></xsl:variable>
    <xsl:param name="sentence" />
    <xsl:choose>
        <xsl:when test="contains($sentence, ' ')">
            <xsl:call-template name="replace_space_by_line_breaks">
                <xsl:with-param name="sentence" select="substring-before($sentence, ' ')" />
            </xsl:call-template>
            <br />
            <xsl:call-template name="replace_space_by_line_breaks">
                <xsl:with-param name="sentence" select="substring-after($sentence, ' ')" />
            </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="$sentence" />
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<xsl:template match="/illustration">

<html>
    <head>
        <title>Let me illustrate...</title>
    </head>
    <body>
        Calculation summary for<br />
        <!-- blockquote -->
            <xsl:choose>
                <xsl:when test="double_scalar[@name='Composite']='1'">
                    composite<br />
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="string_scalar[@name='Insured1']" /><br />

                    <xsl:value-of select="string_scalar[@name='Gender']" />,
                    <xsl:value-of select="string_scalar[@name='Smoker']" />,
                    age <xsl:value-of select="double_scalar[@name='Age']" />
                    <br />

                    <xsl:if test="double_scalar[@name='IsSubjectToIllustrationReg']='1'">
                        
                        <xsl:value-of select="double_scalar[@name='GuarPrem']" />
                        guaranteed premium
                        <br />
                    </xsl:if>
                    <br />

                    <xsl:value-of select="double_scalar[@name='InitGLP']" />
                    initial guideline level premium
                    <br />

                    <xsl:value-of select="double_scalar[@name='InitGSP']" />
                    initial guideline single premium
                    <br />

                    <xsl:value-of select="double_scalar[@name='InitSevenPayPrem']" />
                    initial seven-pay premium
                    <br />

                    <xsl:choose>
                        <xsl:when test="double_scalar[@name='IsMec']='1'">
                            MEC
                        </xsl:when>
                        <xsl:otherwise>
                            Non-MEC
                        </xsl:otherwise>
                    </xsl:choose>
                    <br /><br />

                    <xsl:value-of select="double_scalar[@name='InitTgtPrem']" />
                    initial target premium
                    <br />

                    <xsl:value-of select="double_scalar[@name='InitBaseSpecAmt']" />
                    initial base specified amount
                    <br />

                    <xsl:value-of select="double_scalar[@name='InitTermSpecAmt']" />
                    initial term specified amount
                    <br />
                    
                    <xsl:value-of select="double_scalar[@name='InitTotalSA']" />
                    initial total specified amount
                    <br />

                    <xsl:value-of select="string_scalar[@name='StatePostalAbbrev']" />
                    state of jurisdiction
                    <br />
                </xsl:otherwise>
            </xsl:choose>
        <!-- /blockquote -->

        <hr />
    
        <table border="1" cellpadding="4" cellspacing="0" width="100%">
        <tr align="right">
            <th>Age</th>
        <xsl:for-each select="$show_columns">
            <th>
            <xsl:variable name="tmp">
                <xsl:call-template name="title">
                    <xsl:with-param name="name" select="@name" />
                    <xsl:with-param name="basis" select="@basis" />
                </xsl:call-template>
            </xsl:variable>
            <xsl:call-template name="replace_space_by_line_breaks">
                <xsl:with-param name="sentence" select="normalize-space($tmp)" />
            </xsl:call-template>
            </th>
        </xsl:for-each>
        </tr>
        <!--
            We know that all the columns have the same length. Let's pick one for iteration.
            TODO use 'Age' and 'EndtAge' double_scalars for that purpose.
        -->
        <xsl:variable name="age" select="number(double_scalar[@name='Age'])" />
        <xsl:variable name="loop_variable" select="'Outlay'" />
        <xsl:for-each select="double_vector[@name=$loop_variable]/duration">
            <xsl:variable name="position" select="position()" />
            <tr align="right">
                <td>
                    <xsl:value-of select="$age + $position - 1" />
                </td>
                <xsl:for-each select="$show_columns">
                <td>
                    <xsl:variable name="name" select="@name" />
                    <xsl:variable name="basis" select="@basis" />
                    <xsl:if test="@basis">
                        <xsl:value-of select="$illustration/*[@name=$name][@basis=$basis]/duration[$position]"/>
                    </xsl:if>
                    <xsl:if test="not(@basis)">
                        <xsl:value-of select="$illustration/*[@name=$name]/duration[$position]"/>
                    </xsl:if>
                </td>
                </xsl:for-each>
            </tr>
        </xsl:for-each>
        </table>
    </body>
</html>

</xsl:template>

</xsl:stylesheet>
