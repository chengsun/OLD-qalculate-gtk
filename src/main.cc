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

Manager *mngr;
Variable *vans, *vAns;
GtkWidget *functions_window;
string selected_function_category;
Function *selected_function;
GtkWidget *variables_window;
string selected_variable_category;
Variable *selected_variable;
string result_text;
GtkWidget *units_window;
string selected_unit_category;
Unit *selected_unit, *selected_to_unit;
bool load_global_defs, fetch_exchange_rates_at_startup, first_time;
GtkWidget *omToUnit_menu;
GladeXML *glade_xml;
GdkPixmap *pixmap_result;
extern bool b_busy;
extern vector<string> recent_objects_pre;

int main (int argc, char **argv) {

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_init (&argc, &argv);
	glade_init ();

	b_busy = false;

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
	new Calculator();

	//load application specific preferences
	load_preferences();

	mngr = new Manager();

	if(fetch_exchange_rates_at_startup) {
		CALCULATOR->fetchExchangeRates();
	}
	CALCULATOR->loadExchangeRates();

	//load global definitions
	if(load_global_defs && !CALCULATOR->loadGlobalDefinitions()) {
		g_print(_("Failed to load global definitions!\n"));
	}

	//load local definitions
	CALCULATOR->loadLocalDefinitions();

	//get ans variable objects or create if they do not exist
	vans = CALCULATOR->getVariable(_("ans"));
	vAns = CALCULATOR->getVariable(_("Ans"));
	if(!vans) {
		vans = CALCULATOR->addVariable(new Variable(_("Temporary"), _("ans"), mngr, _("Answer"), false));
	} else {
		vans->set(mngr);
	}
	if(!vAns) {
		vAns = CALCULATOR->addVariable(new Variable(_("Temporary"), _("Ans"), mngr, _("Answer"), false));
	} else {
		vAns->set(mngr);
	}	

	//reset
	functions_window = NULL;
	selected_function_category = _("All");
	selected_function = NULL;
	variables_window = NULL;
	selected_variable_category = _("All");
	selected_variable = NULL;
	units_window = NULL;
	selected_unit_category = _("All");
	selected_unit = NULL;
	selected_to_unit = NULL;
	omToUnit_menu = NULL;
	result_text = "0";
	pixmap_result = NULL;

	//check for calculation errros regularly
	g_timeout_add(100, on_display_errors_timeout, NULL);

	//create main window
	create_main_window();
	
	for(int i = recent_objects_pre.size() - 1; i >= 0; i--) {
		object_inserted(CALCULATOR->getExpressionItem(recent_objects_pre[i]));
	}
	
	//create dynamic menus
	generate_units_tree_struct();
	generate_functions_tree_struct();
	generate_variables_tree_struct();
	create_fmenu();
	create_vmenu();	
	create_umenu();
	create_pmenu();	
	create_umenu2();
	create_pmenu2();			
	
	gtk_main ();
	return 0;
}

