## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-I./calclib \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = qalculate-gtk

qalculate_gtk_SOURCES = \
	calclib/Function.cc calclib/Function.h \
	calclib/Calculator.cc calclib/Calculator.h \
	calclib/Variable.cc calclib/Variable.h \
	calclib/EqItem.cc calclib/EqItem.h \
	calclib/Error.cc calclib/Error.h \
	calclib/Manager.cc calclib/Manager.h \
	calclib/util.cc calclib/util.h \
	calclib/includes.h \
	calclib/Unit.cc calclib/Unit.h \
	calclib/BuiltinFunctions.cc calclib/BuiltinFunctions.h \
	main.cc \
	support.c support.h \
	interface.cc interface.h \
	callbacks.cc callbacks.h

qalculate_gtk_LDADD = @PACKAGE_LIBS@
