#
# src/calclib/MAkefile.am for qalculate
#

SUBDIRS = data

lib_LTLIBRARIES = libqalculate.la

libqalculate_la_SOURCES = \
	Function.cc	Function.h \
	Calculator.cc	Calculator.h \
	DataSet.cc	DataSet.h \
	Variable.cc	Variable.h \
	ExpressionItem.cc	ExpressionItem.h \
	Number.cc	Number.h \
	MathStructure.cc	MathStructure.h \
	Prefix.cc	Prefix.h \
	util.cc		util.h \
	includes.h \
	Unit.cc		Unit.h \
	BuiltinFunctions.cc BuiltinFunctions.h \
	qalculate.h

libqalculate_la_LDFLAGS = -version-info $(QALCULATE_CURRENT):$(QALCULATE_REVISION):$(QALCULATE_AGE) -no-undefined

libqalculate_la_LIBADD = $(INTLLIBS) $(DEPENDENT_LIBS)

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GLIB_CFLAGS@