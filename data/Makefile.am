#
# data/Makefile.am for qalculate
#

glade_DATA = about.glade argumentrules.glade csvexport.glade csvimport.glade datasetedit.glade datasets.glade decimals.glade \
	functionedit.glade functions.glade main.glade matrix.glade matrixedit.glade namesedit.glade nbases.glade \
	setbase.glade periodictable.glade plot.glade precision.glade preferences.glade unit.glade unitedit.glade \
	units.glade unknownedit.glade variableedit.glade variables.glade
gladedir = $(datadir)/$(PACKAGE)/glade

# this is for gnome

desktop_IN_FILES = qalculate-gtk.desktop.in
desktop_DATA = $(desktop_IN_FILES:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
@INTLTOOL_DESKTOP_RULE@

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = qalculate.png

EXTRA_DIST = \
	$(desktop_IN_FILES)	\
	$(glade_DATA)	\
	$(pixmaps_DATA)

