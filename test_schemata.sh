#!/bin/sh

# Test xml schemata.

# Copyright (C) 2012 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# http://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id$

echo "Testing schemata."

# Directory where this script resides.

src_dir=$(dirname $(readlink --canonicalize $0))

# Directory where 'jing.jar' and 'trang.jar' reside, along with their
# support files--extracted from:
#   http://jing-trang.googlecode.com/files/jing-20091111.zip
#   http://jing-trang.googlecode.com/files/trang-20091111.zip

jar_dir=/opt/lmi/third_party/rng

# Data for testing.

cp --preserve $src_dir/sample.cns $src_dir/sample.ill .

# Primary schemata (RNC: RELAX NG, compact syntax).
#
# RNC is primary because it's far more readable than other formats.
# However, as this is written in 2012, only one tool implements it
# with good diagnostics.
#
# Only RNC is to be edited; XSD and RNG are generated from it.

cp --preserve $src_dir/cell.rnc $src_dir/multiple_cell_document.rnc $src_dir/single_cell_document.rnc .

# Test RNC files with 'jing'.

java -jar $jar_dir/jing.jar -c multiple_cell_document.rnc sample.cns
java -jar $jar_dir/jing.jar -c single_cell_document.rnc   sample.ill

# Secondary schemata (XSD: W3C XML Schema).
#
# XSD is secondary because it's widely used and more than one tool
# implements it with good diagnostics. However, it's far less readable
# than RNC.
#
# XSD, generated from RNG, is stored in the repository because it's
# widely used.

cp --preserve $src_dir/cell.xsd $src_dir/multiple_cell_document.xsd $src_dir/single_cell_document.xsd .

# Test XSD files with 'jing'.

java -jar $jar_dir/jing.jar multiple_cell_document.xsd sample.cns
java -jar $jar_dir/jing.jar single_cell_document.xsd   sample.ill

# Test XSD files with 'xmllint'.

xmllint --noout --schema multiple_cell_document.xsd sample.cns
xmllint --noout --schema single_cell_document.xsd   sample.ill

# Tertiary schemata (RNG: RELAX NG, xml syntax).
#
# RNG is tertiary because it has the disadvantages of RNC and XSD
# without the advantages. As seen in the "invalid input" tests below,
# 'xmllint' implements it, but with poor diagnostics.
#
# RNG is generated extemporaneously for testing only, against the day
# when better implementations may come into wider use.

# Generate RNG from RNC with 'trang'.
#
# Alternatively, 'jing -sc' might be used, but would result in
# excessive duplication.

java -jar $jar_dir/trang.jar multiple_cell_document.rnc multiple_cell_document.rng
java -jar $jar_dir/trang.jar single_cell_document.rnc   single_cell_document.rng

# Test RNG files with 'jing'.

java -jar $jar_dir/jing.jar multiple_cell_document.rng sample.cns
java -jar $jar_dir/jing.jar single_cell_document.rng   sample.ill

# Test RNG files with 'xmllint'.

xmllint --noout --relaxng multiple_cell_document.rng sample.cns
xmllint --noout --relaxng single_cell_document.rng   sample.ill

# Test invalid input.

cat >touchstone.eraseme <<EOF
  invalid input, jing, .rnc:
character content of element "InforceDcv" invalid; must be a floating-point number greater than or equal to 0
character content of element "PremiumTaxState" invalid; must be equal to "AK", "AL", "AR", "AZ", "CA", "CO", "CT", "DC", "DE", "FL", "GA", "HI", "IA", "ID", "IL", "IN", "KS", "KY", "LA", "MA", "MD", "ME", "MI", "MN", "MO", "MS", "MT", "NC", "ND", "NE", "NH", "NJ", "NM", "NV", "NY", "OH", "OK", "OR", "PA", "PR", "RI", "SC", "SD", "TN", "TX", "UT", "VA", "VT", "WA", "WI", "WV", "WY" or "XX"
element "StateOfJurisdictionMangledTag" not allowed anywhere; expected element "StateOfJurisdiction"
element "SubstandardTable" not allowed yet; missing required element "StateOfJurisdiction"
  invalid input, jing, .xsd:
http://www.w3.org/TR/xml-schema-1#cvc-minInclusive-valid?-12345.67&0.0E1&nonnegative_double
http://www.w3.org/TR/xml-schema-1#cvc-type.3.1.3?InforceDcv&-12345.67
http://www.w3.org/TR/xml-schema-1#cvc-enumeration-valid?FC&[AL, AK, AZ, AR, CA, CO, CT, DE, DC, FL, GA, HI, ID, IL, IN, IA, KS, KY, LA, ME, MD, MA, MI, MN, MS, MO, MT, NE, NV, NH, NJ, NM, NY, NC, ND, OH, OK, OR, PA, PR, RI, SC, SD, TN, TX, UT, VT, VA, WA, WV, WI, WY, XX]
http://www.w3.org/TR/xml-schema-1#cvc-type.3.1.3?PremiumTaxState&FC
http://www.w3.org/TR/xml-schema-1#cvc-complex-type.2.4.a?StateOfJurisdictionMangledTag&{StateOfJurisdiction}
  invalid input, xmllint, .xsd:
Element 'InforceDcv': [facet 'minInclusive'] The value '-12345.67' is less than the minimum value allowed ('0').
Element 'InforceDcv': '-12345.67' is not a valid value of the atomic type 'nonnegative_double'.
Element 'PremiumTaxState': [facet 'enumeration'] The value 'FC' is not an element of the set {'AL', 'AK', 'AZ', 'AR', 'CA', 'CO', 'CT', 'DE', 'DC', 'FL', 'GA', 'HI', 'ID', 'IL', 'IN', 'IA', 'KS', 'KY', 'LA', 'ME', 'MD', 'MA', 'MI', 'MN', 'MS', 'MO', 'MT', 'NE', 'NV', 'NH', 'NJ', 'NM', 'NY', 'NC', 'ND', 'OH', 'OK', 'OR', 'PA', 'PR', 'RI', 'SC', 'SD', 'TN', 'TX', 'UT', 'VT', 'VA', 'WA', 'WV', 'WI', 'WY', 'XX'}.
Element 'PremiumTaxState': 'FC' is not a valid value of the atomic type 'state'.
Element 'StateOfJurisdictionMangledTag': This element is not expected. Expected is ( StateOfJurisdiction ).
sample_bad fails to validate
  invalid input, jing, .rng:
character content of element "InforceDcv" invalid; must be a floating-point number greater than or equal to 0
character content of element "PremiumTaxState" invalid; must be equal to "AK", "AL", "AR", "AZ", "CA", "CO", "CT", "DC", "DE", "FL", "GA", "HI", "IA", "ID", "IL", "IN", "KS", "KY", "LA", "MA", "MD", "ME", "MI", "MN", "MO", "MS", "MT", "NC", "ND", "NE", "NH", "NJ", "NM", "NV", "NY", "OH", "OK", "OR", "PA", "PR", "RI", "SC", "SD", "TN", "TX", "UT", "VA", "VT", "WA", "WI", "WV", "WY" or "XX"
element "StateOfJurisdictionMangledTag" not allowed anywhere; expected element "StateOfJurisdiction"
element "SubstandardTable" not allowed yet; missing required element "StateOfJurisdiction"
  invalid input, xmllint, .rng:
Error validating datatype double
Element InforceDcv failed to validate content
Error validating value
Element PremiumTaxState failed to validate content
Did not expect element StateOfJurisdictionMangledTag there
sample_bad fails to validate
EOF

cat >eraseme.sed <<EOF
1,/<\/cell>/ {
  # Negative InforceDcv not allowed.
  /InforceDcv/s|>.*<|>-12345.67<|
  # 'FC' is not a valid state abbreviation.
  /PremiumTaxState/s|>.*<|>FC<|
  # Mangled tagname.
  /StateOfJurisdiction/s|StateOfJurisdiction|StateOfJurisdictionMangledTag|g
  }
EOF

# Test invalid input: '.cns'.
<sample.cns >sample_bad.cns sed --file=eraseme.sed
rm --force cns.eraseme
echo "  invalid input, jing, .rnc:"                                      >> cns.eraseme 2>&1
java -jar $jar_dir/jing.jar -c multiple_cell_document.rnc sample_bad.cns >> cns.eraseme 2>&1
echo "  invalid input, jing, .xsd:"                                      >> cns.eraseme 2>&1
java -jar $jar_dir/jing.jar multiple_cell_document.xsd    sample_bad.cns >> cns.eraseme 2>&1
echo "  invalid input, xmllint, .xsd:"                                   >> cns.eraseme 2>&1
xmllint --noout --schema multiple_cell_document.xsd       sample_bad.cns >> cns.eraseme 2>&1
echo "  invalid input, jing, .rng:"                                      >> cns.eraseme 2>&1
java -jar $jar_dir/jing.jar multiple_cell_document.rng    sample_bad.cns >> cns.eraseme 2>&1
echo "  invalid input, xmllint, .rng:"                                   >> cns.eraseme 2>&1
xmllint --noout --relaxng multiple_cell_document.rng      sample_bad.cns >> cns.eraseme 2>&1
sed -e 's/^.*error: //;s/\.cns fails/ fails/;s/  *$//' -i cns.eraseme
diff --unified=0 touchstone.eraseme cns.eraseme

# Test invalid input: '.ill'.
<sample.ill >sample_bad.ill sed --file=eraseme.sed
rm --force ill.eraseme
echo "  invalid input, jing, .rnc:"                                      >> ill.eraseme 2>&1
java -jar $jar_dir/jing.jar -c single_cell_document.rnc   sample_bad.ill >> ill.eraseme 2>&1
echo "  invalid input, jing, .xsd:"                                      >> ill.eraseme 2>&1
java -jar $jar_dir/jing.jar single_cell_document.xsd      sample_bad.ill >> ill.eraseme 2>&1
echo "  invalid input, xmllint, .xsd:"                                   >> ill.eraseme 2>&1
xmllint --noout --schema single_cell_document.xsd         sample_bad.ill >> ill.eraseme 2>&1
echo "  invalid input, jing, .rng:"                                      >> ill.eraseme 2>&1
java -jar $jar_dir/jing.jar single_cell_document.rng      sample_bad.ill >> ill.eraseme 2>&1
echo "  invalid input, xmllint, .rng:"                                   >> ill.eraseme 2>&1
xmllint --noout --relaxng single_cell_document.rng        sample_bad.ill >> ill.eraseme 2>&1
sed -e 's/^.*error: //;s/\.ill fails/ fails/;s/  *$//' -i ill.eraseme
diff --unified=0 touchstone.eraseme ill.eraseme

# Regenerate XSD files as they should appear in the repository.
#
# Reversing the order of the 'trang' commands produces a different
# 'cell.xsd', which lacks <xs:complexType name="cell_element">.

java -jar $jar_dir/trang.jar multiple_cell_document.rnc multiple_cell_document.xsd
java -jar $jar_dir/trang.jar single_cell_document.rnc   single_cell_document.xsd
sed -e 's/  *$//' -i *.xsd
diff --unified=0 --ignore-matching-lines='<!-- [$]Id:.*[$] -->' --from-file=$src_dir *.xsd \
    || echo "Dubious '*.xsd' in repository."

echo "Done."

