#
# data/Makefile.am for qalculate
#

xmldir = $(datadir)/qalculate

xml_in_files = datasets.xml.in elements.xml.in functions.xml.in planets.xml.in prefixes.xml.in units.xml.in variables.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST = \
	$(xml_in_files) $(xml_DATA)

