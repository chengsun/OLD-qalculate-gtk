#
# src/Makefile.am for qalculate
#

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GNOME_UTILS_CFLAGS@ \
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
	@GNOME_UTILS_LIBS@ \
	@QALCULATE_LIBS@

install-exec-local:
	cd $(DESTDIR)$(bindir) && rm -f qalculate; $(LN_S) @LN_QALCULATE@ qalculate

