/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#include "support.h"
#include "interface.h"
#include "callbacks.h"
#include "main.h"

Calculator *calc;
Manager *mngr;
Variable *vans, *vAns;
GtkWidget *functions_window;
string selected_function_category;
string selected_function;
GtkWidget *variables_window;
string selected_variable_category;
string selected_variable;
GtkWidget *units_window;
string selected_unit_category;
string selected_unit, selected_to_unit;
bool load_global_defs;
GtkWidget *omToUnit_menu;

int main (int argc, char *argv[]) {

	GtkWidget *window;

#ifdef ENABLE_NLS

	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_set_locale ();
	gtk_init (&argc, &argv);
	calc = new Calculator();

	load_preferences();

	gchar *gstr = g_build_filename(PACKAGE_DATA_DIR, PACKAGE, "qalculate.cfg", NULL);
	if(load_global_defs && !calc->load(gstr))
		g_print("%s not found!\n", gstr);
	g_free(gstr);

	gstr = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate.cfg", NULL);
	calc->load(gstr);
	g_free(gstr);

	mngr = new Manager(calc);

	vans = calc->getVariable("ans");
	vAns = calc->getVariable("Ans");

	if(!vans) {
		vans = calc->addVariable(new Variable(calc, "Utility", "ans", 0.0, "Answer", false));
		calc->addVariable(vans);		
	} else {
		vans->value(0);
	}
	if(!vAns) {
		vAns = calc->addVariable(new Variable(calc, "Utility", "Ans", 0.0, "Answer", false));
		calc->addVariable(vAns);
	} else {
		vAns->value(0);
	}	

	functions_window = NULL;
	selected_function_category = "All";
	selected_function = "";
	variables_window = NULL;
	selected_variable_category = "All";
	selected_variable = "";
	units_window = NULL;
	selected_unit_category = "All";
	selected_unit = "";
	selected_to_unit = "";
	omToUnit_menu = NULL;

	g_timeout_add(100, on_display_errors_timeout, NULL);

	window = create_window ();
	gtk_widget_show (window);

	gtk_main ();
	return 0;
}

