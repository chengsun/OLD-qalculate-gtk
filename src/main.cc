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
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glade/glade.h>
#ifdef HAVE_LIBGNOME
#include <libgnome/libgnome.h>
#endif
#include <unistd.h>

#include "support.h"
#include "interface.h"
#include "callbacks.h"
#include "main.h"

MathStructure *mstruct, *parsed_mstruct;
string *parsed_to_str;
KnownVariable *vans;
GtkWidget *functions_window;
string selected_function_category;
Function *selected_function;
GtkWidget *variables_window;
string selected_variable_category;
Variable *selected_variable;
string result_text, parsed_text;
GtkWidget *units_window;
string selected_unit_category;
Unit *selected_unit, *selected_to_unit;
bool load_global_defs, fetch_exchange_rates_at_startup, first_time;
GtkWidget *omToUnit_menu;
GdkPixmap *pixmap_result;
GdkPixbuf *pixbuf_result;
extern bool b_busy;
extern vector<string> recent_functions_pre;
extern vector<string> recent_variables_pre;
extern vector<string> recent_units_pre;
extern GtkWidget *expression;

GladeXML *main_glade, *about_glade, *argumentrules_glade, *csvimport_glade, *csvexport_glade, *nbexpression_glade, *decimals_glade;
GladeXML *functionedit_glade, *functions_glade, *matrixedit_glade, *nbases_glade, *plot_glade, *precision_glade;
GladeXML *preferences_glade, *unit_glade, *unitedit_glade, *units_glade, *unknownedit_glade, *variableedit_glade, *variables_glade;

FILE *view_pipe_r, *view_pipe_w;
pthread_t view_thread;
pthread_attr_t view_thread_attr;

bool do_timeout;

int main (int argc, char **argv) {

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

#ifdef HAVE_LIBGNOME
	gnome_program_init(PACKAGE, VERSION, LIBGNOME_MODULE, argc, argv, NULL, NULL, NULL);
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
	unitedit_glade = NULL; units_glade = NULL; unknownedit_glade = NULL; variableedit_glade = NULL; 
	variables_glade = NULL;	csvexport_glade = NULL; nbexpression_glade = NULL;

	//create the almighty Calculator object
	new Calculator();

	//load application specific preferences
	load_preferences();

	mstruct = new MathStructure();
	parsed_mstruct = new MathStructure();
	parsed_to_str = new string;

	/*gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", "tmp", "messages", NULL);
	freopen(gstr, "w+", stdout);
	g_free(gstr);
	gstr = g_build_filename(g_get_home_dir(), ".qalculate", "tmp", "errors", NULL);
	freopen(gstr, "w+", stderr);
	g_free(gstr);*/

	bool canplot = CALCULATOR->canPlot();
	bool canfetch = CALCULATOR->canFetch();

	//create main window
	create_main_window();

	while(gtk_events_pending()) gtk_main_iteration();

	if(!calc_arg.empty()) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(main_glade, "expression")), calc_arg.c_str());
	} else if(first_time) {
		PangoLayout *layout = gtk_widget_create_pango_layout(glade_xml_get_widget(main_glade, "resultview"), NULL);
		gint w = 0, h = 0;
		pango_layout_set_markup(layout, _("Enter a mathematical expression above.\nEx. 5 + 2 / 3"), -1);
		pango_layout_get_pixel_size(layout, &w, &h);
		gdk_draw_layout(GDK_DRAWABLE(glade_xml_get_widget(main_glade, "resultview")->window), glade_xml_get_widget(main_glade, "resultview")->style->fg_gc[GTK_WIDGET_STATE(glade_xml_get_widget(main_glade, "expression"))], 0, 0, layout);	
		g_object_unref(layout);
	}

	while(gtk_events_pending()) gtk_main_iteration();

	//exchange rates
	if((fetch_exchange_rates_at_startup || first_time) && canfetch) {
		fetch_exchange_rates(5);
		while(gtk_events_pending()) gtk_main_iteration();
	}
	
	CALCULATOR->loadExchangeRates();

	//load global definitions
	if(load_global_defs && !CALCULATOR->loadGlobalDefinitions()) {
		g_print(_("Failed to load global definitions!\n"));
	}
	
	/*CALCULATOR->savePrefixes("prefixes.xml.new", true);
	CALCULATOR->saveUnits("units.xml.new", true);
	CALCULATOR->saveVariables("variables.xml.new", true);
	CALCULATOR->saveFunctions("functions.xml.new", true);*/

	//load local definitions
	CALCULATOR->loadLocalDefinitions();

	//get ans variable objects or create if they do not exist
	vans = (KnownVariable*) CALCULATOR->getVariable(_("ans"));
	if(!vans) {
		vans = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), _("ans"), *mstruct, _("Answer"), false));
	} else {
		vans->set(*mstruct);
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
	parsed_text = "0";
	pixmap_result = NULL;
	pixbuf_result = NULL;

	//check for calculation errros regularly
	do_timeout = true;
	g_timeout_add(100, on_display_errors_timeout, NULL);
	
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "menu_item_plot_functions"), canplot);
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "menu_item_fetch_exchange_rates"), canfetch);
		
	//create dynamic menus
	generate_units_tree_struct();
	generate_functions_tree_struct();
	generate_variables_tree_struct();
	create_fmenu();
	create_vmenu();	
	create_umenu();
	//create_pmenu();	
	create_umenu2();
	create_pmenu2();			

	for(int i = recent_functions_pre.size() - 1; i >= 0; i--) {
		function_inserted(CALCULATOR->getFunction(recent_functions_pre[i]));
	}
	for(int i = recent_variables_pre.size() - 1; i >= 0; i--) {
		variable_inserted(CALCULATOR->getVariable(recent_variables_pre[i]));
	}
	for(int i = recent_units_pre.size() - 1; i >= 0; i--) {
		unit_inserted(CALCULATOR->getUnit(recent_units_pre[i]));
	}
	
	update_completion();
	
	int pipe_wr[] = {0, 0};
	pipe(pipe_wr);
	view_pipe_r = fdopen(pipe_wr[0], "r");
	view_pipe_w = fdopen(pipe_wr[1], "w");
	pthread_attr_init(&view_thread_attr);
	pthread_create(&view_thread, &view_thread_attr, view_proc, view_pipe_r);
	
	if(!calc_arg.empty()) {
		execute_expression();
	}
	
	gtk_main();
	return 0;
}

