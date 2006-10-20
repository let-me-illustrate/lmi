<?xml version="1.0"?>
<!--
    The common part of Xsl templates used to render illustration data into various formats.

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

    $Id: common.xsl,v 1.1.2.1 2006-10-20 17:46:02 etarassov Exp $

    Uses format.xml - column titles, number-formatting and other information.
-->
<xsl:stylesheet version="1.0"
	      xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <!--
        Print this message if title is not found for a column.
        TODO: find a way to conditionally use it only in the debug mode
    -->
    <xsl:variable name="no_title_error" select="'Title is not defined for a column!'" />

    <!--
        Read the column information (title, forma-string, etc.) from format.xml file.
        Used in 'title' and 'get_column_format' templates.
    -->
    <xsl:variable name="columns" select="document('format.xml')/columns/column" />

    <!--
        Print column title.
        - 'column' - column name ('name_type', required)
        - 'basis' - column basis ('basis_type', optional)
        TODO: template is under construction
    -->
    <xsl:template name="title"
       ><xsl:param name="name"
      /><xsl:param name="basis"
      /><xsl:choose
           ><xsl:when test="$basis"
               ><xsl:call-template name="do_title"
                   ><xsl:with-param name="name" select="$name"
                  /><xsl:with-param name="basis" select="$basis"
                  /><!-- Basis parameter is specified
                 --><xsl:with-param name="title" select="$columns[@name=$name]/title[@basis=$basis]"
              /></xsl:call-template
           ></xsl:when
           ><xsl:otherwise
               ><xsl:call-template name="do_title"
                   ><xsl:with-param name="name" select="$name"
                  /><xsl:with-param name="basis" select="$basis"
                  /><!-- No basis parameter specified
                 --><xsl:with-param name="title" select="$columns[@name=$name]/title"
              /></xsl:call-template
           ></xsl:otherwise
       ></xsl:choose
   ></xsl:template>

    <xsl:template name="do_title"
       ><xsl:param name="name"
      /><xsl:param name="basis"
      /><xsl:param name="title"
      /><xsl:if test="$title!=''"
           ><xsl:value-of select="$title"
      /></xsl:if
       ><xsl:if test="$title=''"
           ><!-- no title, show error
         --><xsl:value-of select="$no_title_error"
          /><xsl:value-of select="$name" />_<xsl:value-of select="$basis"
      /></xsl:if
   ></xsl:template>

</xsl:stylesheet>
