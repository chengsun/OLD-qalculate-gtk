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
#include <glade/glade.h>

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
Unit *selected_unit, *selected_to_unit;
bool load_global_defs;
GtkWidget *omToUnit_menu;
GladeXML * glade_xml;

int main (int argc, char **argv) {

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_init (&argc, &argv);
	glade_init ();

	/* load the glade file into the object and die if that doesn't work */
	gchar *gstr = g_build_filename (
			PACKAGE_DATA_DIR,
			PACKAGE,
			"glade",
			"qalculate.glade",
			NULL);
	glade_xml = glade_xml_new (
			gstr,
			NULL,
			NULL);
	g_assert (glade_xml != NULL);
	g_free (gstr);
	
	//create the almighty Calculator object
	calc = new Calculator();

	//load application specific preferences
	load_preferences();

	//load global definitions
	gstr = g_build_filename(PACKAGE_DATA_DIR, PACKAGE, "qalculate.cfg", NULL);
	if(load_global_defs && !calc->load(gstr)) {
		g_print(_("%s not found!\n"), gstr);
	}
	g_free(gstr);

	//load local definitions
	gstr = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate.cfg", NULL);
	calc->load(gstr);
	g_free(gstr);

	mngr = new Manager(calc);


	//get ans variable objects or create if they do not exist
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

	//reset
	functions_window = NULL;
	selected_function_category = _("All");
	selected_function = "";
	variables_window = NULL;
	selected_variable_category = _("All");
	selected_variable = "";
	units_window = NULL;
	selected_unit_category = _("All");
	selected_unit = NULL;
	selected_to_unit = NULL;
	omToUnit_menu = NULL;

	//check for calculation errros regularly
	g_timeout_add(100, on_display_errors_timeout, NULL);

	//create main window
	create_main_window();
	
	//create dynamic menus
	create_fmenu();
	create_vmenu();	
	create_pmenu();	
	create_umenu();
	create_umenu2();
	
	gtk_main ();
	return 0;
}

