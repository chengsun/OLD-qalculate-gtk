/*
    Qalculate    

    Copyright (C) 2003-2007  Niklas Knutsson (nq@altern.org)

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

MathStructure *mstruct, *matrix_mstruct, *parsed_mstruct, *parsed_tostruct;
bool prev_result_approx;
string *parsed_to_str;
KnownVariable *vans[5];
GtkWidget *functions_window;
string selected_function_category;
MathFunction *selected_function;
GtkWidget *variables_window;
string selected_variable_category;
Variable *selected_variable;
string result_text, parsed_text;
GtkWidget *units_window;
string selected_unit_category;
string selected_unit_selector_category;
Unit *selected_unit, *selected_to_unit;
bool load_global_defs, fetch_exchange_rates_at_startup, first_time, first_qalculate_run;
GtkWidget *omToUnit_menu;
GdkPixmap *pixmap_result;
GdkPixbuf *pixbuf_result;
extern bool b_busy;
extern vector<string> recent_functions_pre;
extern vector<string> recent_variables_pre;
extern vector<string> recent_units_pre;
extern GtkWidget *expression;
extern PrintOptions printops;

GladeXML *main_glade, *about_glade, *argumentrules_glade, *csvimport_glade, *csvexport_glade, *setbase_glade, *datasetedit_glade, *datasets_glade, *decimals_glade;
GladeXML *functionedit_glade, *functions_glade, *matrixedit_glade, *matrix_glade, *namesedit_glade, *nbases_glade, *plot_glade, *precision_glade;
GladeXML *preferences_glade, *unit_glade, *unitedit_glade, *units_glade, *unknownedit_glade, *variableedit_glade, *variables_glade;
GladeXML *periodictable_glade;

FILE *view_pipe_r, *view_pipe_w, *command_pipe_r, *command_pipe_w;
pthread_t view_thread, command_thread;
pthread_attr_t view_thread_attr, command_thread_attr;
bool command_thread_started;

bool do_timeout, check_expression_position;
gint expression_position;

#ifdef HAVE_LIBGNOME
static poptContext pctx;
static struct poptOption options[] = {
	{NULL, '\0', 0, NULL, 0, NULL, NULL}
};
#endif

int main (int argc, char **argv) {

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

#ifdef HAVE_LIBGNOME

	GnomeProgram *program = gnome_program_init("qalculate-gtk", VERSION, LIBGNOME_MODULE, argc, argv, GNOME_PARAM_POPT_TABLE, options, GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR, NULL);

	char *icon = gnome_program_locate_file(program, GNOME_FILE_DOMAIN_APP_PIXMAP, "qalculate.png", TRUE, NULL);

	if(icon) {
		gtk_window_set_default_icon_from_file(icon, NULL);
		g_free(icon);
	}
	
	gtk_init(&argc, &argv);

	g_object_get(G_OBJECT(program), GNOME_PARAM_POPT_CONTEXT, &pctx, NULL);


#else


	gtk_init(&argc, &argv);

	gtk_window_set_default_icon_from_file(PACKAGE_DATA_DIR "/pixmaps/qalculate.png", NULL);

#endif

	glade_init();

	string calc_arg;
#ifdef HAVE_LIBGNOME
	const char **args = poptGetArgs (pctx);
	for(int i = 0; args && args[i]; i++) {
		if(i > 1) {
			calc_arg += " ";
		}
		if(strlen(args[i]) >= 2 && ((args[i][0] == '\"' && args[i][strlen(args[i]) - 1] == '\"') || (args[i][0] == '\'' && args[i][strlen(args[i]) - 1] == '\''))) {
			calc_arg += args[i] + 1;
			calc_arg.erase(calc_arg.length() - 1);
		} else {
			calc_arg += args[i];
		}
	}
	poptFreeContext (pctx);
#else	
	for(int i = 1; i < argc; i++) {
		if(i > 1) {
			calc_arg += " ";
		}
		if(strlen(argv[i]) >= 2 && ((argv[i][0] == '\"' && argv[i][strlen(argv[i]) - 1] == '\"') || (argv[i][0] == '\'' && argv[i][strlen(argv[i]) - 1] == '\''))) {
			calc_arg += argv[i] + 1;
			calc_arg.erase(calc_arg.length() - 1);
		} else {
			calc_arg += argv[i];
		}
	}
#endif
	b_busy = false;

	main_glade = NULL; about_glade = NULL; argumentrules_glade = NULL; 
	csvimport_glade = NULL; datasetedit_glade = NULL; datasets_glade = NULL; decimals_glade = NULL; functionedit_glade = NULL; 
	functions_glade = NULL; matrixedit_glade = NULL; matrix_glade = NULL; namesedit_glade = NULL; nbases_glade = NULL; plot_glade = NULL; 
	precision_glade = NULL; preferences_glade = NULL; unit_glade = NULL; 
	unitedit_glade = NULL; units_glade = NULL; unknownedit_glade = NULL; variableedit_glade = NULL; 
	variables_glade = NULL;	csvexport_glade = NULL; setbase_glade = NULL; periodictable_glade = NULL;

	//create the almighty Calculator object
	new Calculator();

	//load application specific preferences
	load_preferences();

	mstruct = new MathStructure();
	parsed_mstruct = new MathStructure();
	parsed_tostruct = new MathStructure();
	parsed_tostruct->setUndefined();
	matrix_mstruct = new MathStructure();
	prev_result_approx = false;
	parsed_to_str = new string;

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
		pango_layout_set_markup(layout, _("Enter a mathematical expression above. Ex. 5 + 2 / 3"), -1);
		pango_layout_get_pixel_size(layout, &w, &h);
		gdk_draw_layout(GDK_DRAWABLE(glade_xml_get_widget(main_glade, "resultview")->window), glade_xml_get_widget(main_glade, "resultview")->style->fg_gc[GTK_WIDGET_STATE(glade_xml_get_widget(main_glade, "expression"))], 0, 0, layout);	
		g_object_unref(layout);
	}

	while(gtk_events_pending()) gtk_main_iteration();

	//exchange rates
	if(first_qalculate_run && canfetch) {
		GtkWidget *edialog = gtk_message_dialog_new(GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, _("You need to download exchange rates to be able to convert between different currencies. You can later get current exchange rates by selecting \"Update Exchange Rates\" under the File menu.\n\nDo you want to fetch exchange rates now from the Internet?"));
		int question_answer = gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
		while(gtk_events_pending()) gtk_main_iteration();
		if(question_answer == GTK_RESPONSE_YES) {
			fetch_exchange_rates(5);
			while(gtk_events_pending()) gtk_main_iteration();
		}
	} else if(fetch_exchange_rates_at_startup && canfetch) {
		fetch_exchange_rates(5);
		while(gtk_events_pending()) gtk_main_iteration();
	}

	CALCULATOR->loadExchangeRates();

	string ans_str = _("ans");
	vans[0] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str, m_undefined, _("Last Answer"), false));
	vans[0]->addName(_("answer"));
	vans[0]->addName(ans_str + "1");
	vans[1] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "2", m_undefined, _("Answer 2"), false));
	vans[2] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "3", m_undefined, _("Answer 3"), false));
	vans[3] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "4", m_undefined, _("Answer 4"), false));
	vans[4] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "5", m_undefined, _("Answer 5"), false));

	//load global definitions
	if(load_global_defs && !CALCULATOR->loadGlobalDefinitions()) {
		g_print(_("Failed to load global definitions!\n"));
	}

	//load local definitions
	CALCULATOR->loadLocalDefinitions();

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
	
	check_expression_position = true;
	expression_position = 1;
	g_timeout_add(50, on_check_expression_position_timeout, NULL);
	
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

	for(int i = ((int) recent_functions_pre.size()) - 1; i >= 0; i--) {
		function_inserted(CALCULATOR->getActiveFunction(recent_functions_pre[i]));
	}
	for(int i = ((int) recent_variables_pre.size()) - 1; i >= 0; i--) {
		variable_inserted(CALCULATOR->getActiveVariable(recent_variables_pre[i]));
	}
	for(int i = ((int) recent_units_pre.size()) - 1; i >= 0; i--) {
		Unit *u = CALCULATOR->getActiveUnit(recent_units_pre[i]);
		if(!u) u = CALCULATOR->getCompositeUnit(recent_units_pre[i]);
		unit_inserted(u);
	}

	update_completion();
	
	int pipe_wr[] = {0, 0};
	pipe(pipe_wr);
	view_pipe_r = fdopen(pipe_wr[0], "r");
	view_pipe_w = fdopen(pipe_wr[1], "w");
	pthread_attr_init(&view_thread_attr);
	pthread_create(&view_thread, &view_thread_attr, view_proc, view_pipe_r);
	
	int pipe_wr2[] = {0, 0};
	pipe(pipe_wr2);
	command_pipe_r = fdopen(pipe_wr2[0], "r");
	command_pipe_w = fdopen(pipe_wr2[1], "w");
	pthread_attr_init(&command_thread_attr);
	command_thread_started = false;
	
	if(!calc_arg.empty()) {
		execute_expression();
	} else if(!first_time) {
		int base = printops.base;
		printops.base = 10;
		setResult(NULL, false, false, false);
		printops.base = base;
	}

	gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", "accelmap", NULL);
	gtk_accel_map_load(gstr);
	g_free(gstr);

	gtk_main();

	return 0;
}
