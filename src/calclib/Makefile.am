#
# src/calclib/MAkefile.am for qalculate
#

SUBDIRS = data

noinst_LTLIBRARIES = libcalculate.la

libcalculate_la_SOURCES = \
	Function.cc	Function.h \
	Calculator.cc	Calculator.h \
	Variable.cc	Variable.h \
	EqItem.cc	EqItem.h \
	Error.cc	Error.h \
	ExpressionItem.cc	ExpressionItem.h \
	Fraction.cc	Fraction.h \
	Integer.cc	Integer.h \
	Manager.cc	Manager.h \
	Matrix.cc	Matrix.h \
	Prefix.cc	Prefix.h \
	util.cc		util.h \
	includes.h \
	Unit.cc		Unit.h \
	BuiltinFunctions.cc BuiltinFunctions.h

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@