#
# data/Makefile.am for qalculate
#

glade_DATA = qalculate.glade
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
	$(glade_DATA)

