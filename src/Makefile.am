#
# src/Makefile.am for qalculate
#

SUBDIRS = calclib

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I./calclib \
	@GNOME_UTILS_CFLAGS@ \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = qalculate-gtk qalc

qalculate_gtk_SOURCES = \
	main.cc		main.h		\
	support.c	support.h	\
	interface.cc	interface.h	\
	callbacks.cc	callbacks.h

qalculate_gtk_LDADD = \
	@PACKAGE_LIBS@ \
	@GNOME_UTILS_LIBS@ \
	calclib/libqalculate.la

qalc_SOURCES = qalc.cc

qalc_LDADD = \
	@PACKAGE_LIBS@ \
	@GNOME_UTILS_LIBS@ \
	calclib/libqalculate.la

install-exec-local:
	cd $(DESTDIR)$(bindir) && rm -f qalculate; $(LN_S) qalculate-gtk qalculate

