#
# src/calclib/MAkefile.am for qalculate
#

SUBDIRS = data

noinst_LTLIBRARIES = libcalculate.la

libcalculate_la_SOURCES = \
	Function.cc	Function.h \
	Calculator.cc	Calculator.h \
	DataCollection.cc	DataCollection.h \
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

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@