#
# data/Makefile.am for qalculate
#

glade_DATA = about.glade argumentrules.glade csvexport.glade csvimport.glade datasets.glade decimals.glade \
	functionedit.glade functions.glade main.glade matrixedit.glade namesedit.glade nbases.glade \
	nbexpression.glade periodictable.glade plot.glade precision.glade preferences.glade unit.glade unitedit.glade \
	units.glade unknownedit.glade variableedit.glade variables.glade
gladedir = $(datadir)/$(PACKAGE)/glade

# this is for gnome and kde 3.2+

desktop_IN_FILES = qalculate.desktop.in
desktop_DATA = $(desktop_IN_FILES:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
@INTLTOOL_DESKTOP_RULE@

# this is for older kde-versions
applnk_DATA = $(desktop_DATA)
applnkdir = $(datadir)/applnk/Utilities

EXTRA_DIST = \
	$(desktop_IN_FILES)	\
	$(glade_DATA)	\
	icon.xpm

