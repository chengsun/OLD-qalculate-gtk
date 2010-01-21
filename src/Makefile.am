#
# src/Makefile.am for qalculate
#

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@LIBGNOME_CFLAGS@ \
	@GTK_CFLAGS@ \
	@GLIB_CFLAGS@ \
	@QALCULATE_CFLAGS@

bin_PROGRAMS = @QALCULATE_GTK@
EXTRA_PROGRAMS = qalculate-gtk

qalculate_gtk_SOURCES = \
	main.cc		main.h		\
	interface.cc	interface.h	\
	callbacks.cc	callbacks.h	\
	support.h

qalculate_gtk_LDADD = \
	@GLIB_LIBS@ \
	@GTK_LIBS@ \
	@LIBGNOME_LIBS@ \
	@QALCULATE_LIBS@

