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
#ifdef HAVE_LIBGNOME
#include <libgnome/libgnome.h>
#endif

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
GdkPixmap *pixmap_result;
extern bool b_busy;
extern vector<string> recent_objects_pre;

GladeXML *main_glade, *about_glade, *argumentrules_glade, *csvimport_glade, *decimals_glade;
GladeXML *functionedit_glade, *functions_glade, *matrixedit_glade, *nbases_glade, *plot_glade, *precision_glade;
GladeXML *preferences_glade, *unit_glade, *unitedit_glade, *units_glade, *variableedit_glade, *variables_glade;

int main (int argc, char **argv) {

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

#ifdef HAVE_LIBGNOME
	GnomeProgram *app = gnome_program_init(PACKAGE, VERSION, LIBGNOME_MODULE, argc, argv, NULL, NULL, NULL);
#endif
	gtk_init(&argc, &argv);
	glade_init();
	
	string calc_arg;
	for(int i = 1; i < argc; i++) {
		if(i > 1) {
			calc_arg += " ";
		}
		if(strlen(argv[i]) >= 2 && ((argv[i][0] == '\"' && argv[i][strlen(argv[i] - 1)] == '\"') || (argv[i][0] == '\'' && argv[i][strlen(argv[i] - 1)] == '\''))) {
			calc_arg += argv[i] + 1;
			calc_arg.erase(calc_arg.length() - 1);
		} else {
			calc_arg += argv[i];
		}
	}

	b_busy = false;
	
	main_glade = NULL; about_glade = NULL; argumentrules_glade = NULL; 
	csvimport_glade = NULL; decimals_glade = NULL; functionedit_glade = NULL; 
	functions_glade = NULL; matrixedit_glade = NULL; nbases_glade = NULL; plot_glade = NULL; 
	precision_glade = NULL; preferences_glade = NULL; unit_glade = NULL; 
	unitedit_glade = NULL; units_glade = NULL; variableedit_glade = NULL; variables_glade = NULL;	

	//create the almighty Calculator object
	new Calculator();

	//load application specific preferences
	load_preferences();

	mngr = new Manager();

	bool canplot = CALCULATOR->canPlot();
	bool canfetch = CALCULATOR->canFetch();

	if(fetch_exchange_rates_at_startup && canfetch) {
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


	//create main window
	create_main_window();

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(main_glade, "expression")), calc_arg.c_str());

	//check for calculation errros regularly
	g_timeout_add(100, on_display_errors_timeout, NULL);
	
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "menu_item_plot_functions"), canplot);
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "menu_item_fetch_exchange_rates"), canfetch);
	
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
	
	if(!calc_arg.empty()) {
		execute_expression();
	}
	
	gtk_main();
	return 0;
}

