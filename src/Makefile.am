#
# src/Makefile.am for qalculate
#

SUBDIRS = libqalculate

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I./libqalculate \
	@GNOME_UTILS_CFLAGS@ \
	@GTK_CFLAGS@ \
	@GLIB_CFLAGS@

bin_PROGRAMS = @QALCULATE_GTK@ @QALCULATE_TEXT@
EXTRA_PROGRAMS = qalculate-gtk qalc

qalculate_gtk_SOURCES = \
	main.cc		main.h		\
	interface.cc	interface.h	\
	callbacks.cc	callbacks.h

qalculate_gtk_LDADD = \
	@GLIB_LIBS@ \
	@GTK_LIBS@ \
	@GNOME_UTILS_LIBS@ \
	libqalculate/libqalculate.la

qalc_SOURCES = qalc.cc

qalc_LDADD = \
	@GLIB_LIBS@ \
	@READLINE_LIBS@ \
	libqalculate/libqalculate.la

install-exec-local:
	cd $(DESTDIR)$(bindir) && rm -f qalculate; $(LN_S) @LN_QALCULATE@ qalculate

