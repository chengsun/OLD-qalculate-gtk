#
# data/Makefile.am for qalculate
#

config_DATA = qalculate.cfg
configdir = $(datadir)/$(PACKAGE)

desktop_IN_FILES = qalculate.desktop.in
desktop_DATA = $(desktop_IN_FILES:.desktop.in=.desktop)
desktopdir = $(datadir)/applications
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = qalculate.cfg

