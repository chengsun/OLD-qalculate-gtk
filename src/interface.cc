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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "main.h"
#include "qalculate.h"
#include "data/icon.xpm"

extern GladeXML *main_glade, *about_glade, *argumentrules_glade, *csvimport_glade, *csvexport_glade, *nbexpression_glade, *decimals_glade;
extern GladeXML *functionedit_glade, *functions_glade, *matrixedit_glade, *nbases_glade, *plot_glade, *precision_glade;
extern GladeXML *preferences_glade, *unit_glade, *unitedit_glade, *units_glade, *unknownedit_glade, *variableedit_glade, *variables_glade;

GtkWidget *tFunctionCategories;
GtkWidget *tFunctions;
GtkListStore *tFunctions_store;
GtkTreeStore *tFunctionCategories_store;

GtkWidget *tVariableCategories;
GtkWidget *tVariables;
GtkListStore *tVariables_store;
GtkTreeStore *tVariableCategories_store;

GtkWidget *tUnitCategories;
GtkWidget *tUnits;
GtkListStore *tUnits_store;
GtkTreeStore *tUnitCategories_store;

#if GTK_MINOR_VERSION >= 3
GtkWidget *expander;
GtkEntryCompletion *completion;
GtkListStore *completion_store;
#endif

GtkWidget *tFunctionArguments;
GtkListStore *tFunctionArguments_store;
GtkWidget *tSubfunctions;
GtkListStore *tSubfunctions_store;

GtkWidget *tPlotFunctions;
GtkListStore *tPlotFunctions_store;

GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;

GtkWidget *expression;
GtkWidget *resultview;
GtkWidget *f_menu ,*v_menu, *u_menu, *u_menu2, *recent_menu;
GtkAccelGroup *accel_group;

extern bool show_buttons;
extern bool save_mode_on_exit, save_defs_on_exit, load_global_defs, hyp_is_on, fetch_exchange_rates_at_startup;
extern bool use_custom_result_font, use_custom_expression_font;
extern string custom_result_font, custom_expression_font;
extern string multi_sign;

extern PrintOptions printops, saved_printops;
extern EvaluationOptions evalops, saved_evalops;

extern vector<vector<GtkWidget*> > element_entries;
extern vector<string> initial_history;

gint compare_categories(gconstpointer a, gconstpointer b) {
	return strcasecmp((const char*) a, (const char*) b);
}

void
create_main_window (void)
{
	
	gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "main.glade", NULL);
	main_glade = glade_xml_new(gstr, NULL, NULL);
	g_assert(main_glade != NULL);
	g_free(gstr);
	
	/* make sure we get a valid main window */
	g_assert (glade_xml_get_widget (main_glade, "main_window") != NULL);

	expression = glade_xml_get_widget (main_glade, "expression");
	resultview = glade_xml_get_widget (main_glade, "resultview");
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "gray_foreground", "foreground", "gray40", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "red_foreground", "foreground", "red", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "blue_foreground", "foreground", "blue", NULL);

	/* the function table */
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (main_glade, "button_xy")))),
			TRUE);
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (main_glade, "button_fraction")))),
			TRUE);
			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "button_hyp")), hyp_is_on);			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "button_fraction")), printops.number_fraction_format == FRACTION_FRACTIONAL);

	switch(evalops.approximation) {
		case APPROXIMATION_EXACT: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_always_exact")), TRUE);
			break;
		}
		case APPROXIMATION_TRY_EXACT: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_try_exact")), TRUE);
			break;
		}
		case APPROXIMATION_APPROXIMATE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_approximate")), TRUE);
			break;
		}
	}
	
	switch(evalops.auto_post_conversion) {
		case POST_CONVERSION_BEST: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_post_conversion_best")), TRUE);
			break;
		}
		case POST_CONVERSION_BASE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_post_conversion_base")), TRUE);
			break;
		}
		default: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_post_conversion_none")), TRUE);
			break;
		}
	}

	accel_group = gtk_accel_group_new();


	switch (CALCULATOR->angleMode()) {
		case DEGREES: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_degrees")), TRUE);
			break;
		}
		case RADIANS: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_radians")), TRUE);
			break;
		}
		case GRADIANS: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_gradians")), TRUE);
			break;
		}
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_rpn_mode")), evalops.parse_options.rpn);

	switch(printops.base) {
		case BASE_BINARY: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_binary")), TRUE);
			break;
		}
		case BASE_OCTAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_octal")), TRUE);
			break;
		}
		case BASE_DECIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_decimal")), TRUE);
			break;
		}
		case BASE_HEXADECIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_hexadecimal")), TRUE);
			break;
		}
		case BASE_ROMAN_NUMERALS: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_roman")), TRUE);
			break;
		}
		case BASE_SEXAGESIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_sexagesimal")), TRUE);
			break;
		}
		case BASE_TIME: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_time_format")), TRUE);
			break;
		}
		default: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_custom_base")), TRUE);
		}
	}
	if(printops.base >= 2 && printops.base <= 36) {
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (main_glade, "number_base_spinbutton_base")), printops.base);
	}
	
	switch(printops.min_exp) {
		case EXP_PRECISION: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_normal")), TRUE);
			break;
		}
		case EXP_SCIENTIFIC: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_scientific")), TRUE);
			break;
		}
		case EXP_PURE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_purely_scientific")), TRUE);
			break;
		}
		case EXP_NONE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_non_scientific")), TRUE);
			break;
		}
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_indicate_infinite_series")), printops.indicate_infinite_series);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_round_halfway_to_even")), printops.round_halfway_to_even);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_prefixes")), printops.use_unit_prefixes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_all_prefixes")), printops.use_all_prefixes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_denominator_prefixes")), printops.use_denominator_prefix);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_place_units_separately")), printops.place_units_separately);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_abbreviate_names")), printops.abbreviate_names);
			
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_variables")), evalops.parse_options.variables_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_functions")), evalops.parse_options.functions_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_units")), evalops.parse_options.units_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_unknown_variables")), evalops.parse_options.unknowns_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_calculate_variables")), evalops.calculate_variables);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_allow_complex")), evalops.allow_complex);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_allow_infinite")), evalops.allow_infinite);

	switch (printops.number_fraction_format) {
		case FRACTION_DECIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_decimal")), TRUE);
			break;
		}
		case FRACTION_DECIMAL_EXACT: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_decimal_exact")), TRUE);
			break;
		}
		case FRACTION_COMBINED: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_combined")), TRUE);
			break;		
		}
		case FRACTION_FRACTIONAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_fraction")), TRUE);
			break;		
		}
	}

	switch(CALCULATOR->angleMode()) {
		case RADIANS: {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "radiobutton_radians")), TRUE);
			break;
		}
		case DEGREES: {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "radiobutton_degrees")), TRUE);
			break;
		}
		case GRADIANS: {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "radiobutton_gradians")), TRUE);
			break;
		}
	}
	switch(CALCULATOR->defaultAssumptions()->sign()) {
		case ASSUMPTION_SIGN_POSITIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_positive")), TRUE); break;}
		case ASSUMPTION_SIGN_NONPOSITIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_nonpositive")), TRUE); break;}
		case ASSUMPTION_SIGN_NEGATIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_negative")), TRUE); break;}
		case ASSUMPTION_SIGN_NONNEGATIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_nonnegative")), TRUE); break;}
		case ASSUMPTION_SIGN_NONZERO: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_nonzero")), TRUE); break;}
		default: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_unknown")), TRUE);}
	}
	switch(CALCULATOR->defaultAssumptions()->numberType()) {
		case ASSUMPTION_NUMBER_INTEGER: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_integer")), TRUE); break;}
		case ASSUMPTION_NUMBER_RATIONAL: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_rational")), TRUE); break;}
		case ASSUMPTION_NUMBER_REAL: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_real")), TRUE); break;}
		case ASSUMPTION_NUMBER_COMPLEX: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_complex")), TRUE); break;}
		case ASSUMPTION_NUMBER_NUMBER: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_number")), TRUE); break;}
		default: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_none")), TRUE);}
	}

#if GTK_MINOR_VERSION < 3
	if(show_buttons) {
		gtk_widget_show (glade_xml_get_widget (main_glade, "buttons"));
		gtk_button_set_label (GTK_BUTTON(glade_xml_get_widget (main_glade, "button_less_more")), _("Hide keypad"));
	} else {
		gtk_widget_hide (glade_xml_get_widget (main_glade, "buttons"));
		gtk_button_set_label (GTK_BUTTON(glade_xml_get_widget (main_glade, "button_less_more")), _("Show keypad"));
	}
#endif

	if(printops.use_unicode_signs) {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sub")), SIGN_MINUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_add")), SIGN_PLUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_times")), SIGN_MULTIPLICATION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_divide")), SIGN_DIVISION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sqrt")), SIGN_SQRT);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_dot")), SIGN_MULTIDOT);	
//		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_inexact")), SIGN_APPROXIMATELY_EQUAL);			
	}

	if(use_custom_result_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_result_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
	} else {
/*		PangoFontDescription *font = pango_font_description_copy(resultview->style->font_desc);
//		pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);	*/	
		if(custom_result_font.empty()) {
			custom_result_font = pango_font_description_to_string(resultview->style->font_desc);
		}		
	}
	if(use_custom_expression_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_expression_font.c_str());
		gtk_widget_modify_font(expression, font);
		pango_font_description_free(font);
	} else {
		if(custom_expression_font.empty()) {
			custom_expression_font = pango_font_description_to_string(expression->style->font_desc);
		}		
	}
	
	gtk_window_add_accel_group (GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), accel_group);

	gtk_widget_grab_focus(expression);
	GTK_WIDGET_SET_FLAGS(expression, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(expression);

	glade_xml_signal_autoconnect(main_glade);

//	gtk_widget_modify_bg(resultview, GTK_STATE_NORMAL, &glade_xml_get_widget(main_glade, "history")->style->base[GTK_WIDGET_STATE(glade_xml_get_widget(main_glade, "history"))]);	

	GtkTextIter iter;
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history")));
	for(unsigned int i = 0; i < initial_history.size(); i++) {
		gtk_text_buffer_get_end_iter(tb, &iter);
		if(i == 0 && initial_history[i] != "-----------------------") {
			gtk_text_buffer_insert(tb, &iter, "-----------------------\n", -1);
		}
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, initial_history[i].c_str(), -1);
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, "\n", -1);
	}
	initial_history.clear();

#ifndef HAVE_LIBGNOME
	gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_help"), FALSE);
#endif

#if GTK_MINOR_VERSION >= 3

/*	Expander	*/
	gtk_widget_hide(glade_xml_get_widget(main_glade, "buttonbox_bottom"));
	expander = gtk_expander_new(_("Keypad"));
	g_object_ref(glade_xml_get_widget(main_glade, "buttons"));
	gtk_container_remove(GTK_CONTAINER(glade_xml_get_widget(main_glade, "main_vbox")), glade_xml_get_widget(main_glade, "buttons"));
	gtk_box_pack_end(GTK_BOX(glade_xml_get_widget(main_glade, "main_vbox")), expander, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(expander), glade_xml_get_widget(main_glade, "buttons"));
	g_object_unref(glade_xml_get_widget(main_glade, "buttons"));
	gtk_expander_set_expanded(GTK_EXPANDER(expander), show_buttons);
	gtk_widget_show(expander);
	
/*	Completion	*/	
	completion = gtk_entry_completion_new();
	gtk_entry_set_completion(GTK_ENTRY(expression), completion);
	completion_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(completion_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(completion_store), 0, GTK_SORT_ASCENDING);
	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(completion_store));
	g_object_unref(completion_store);
	GtkCellRenderer *cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(completion), cell, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(completion), cell, "text", 0);	
	cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(completion), cell, FALSE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(completion), cell, "text", 1);
	gtk_entry_completion_set_match_func(completion, &completion_match_func, NULL, NULL);
	g_signal_connect((gpointer) completion, "match-selected", G_CALLBACK(on_completion_match_selected), NULL);
#endif
	//gtk_widget_hide(glade_xml_get_widget(main_glade, "menu_item_multiple_roots"));

	gtk_widget_show (glade_xml_get_widget (main_glade, "main_window"));

	GtkStyle *style;
	GdkBitmap *bitmap;
	GdkPixmap *pixmap;
	GdkColormap *colormap;
	GdkColor wait_color={0,0,0,0};
	GtkWidget *toplevel = glade_xml_get_widget (main_glade, "main_window");

	style=gtk_widget_get_style(GTK_WIDGET(toplevel));
	pixmap=gdk_pixmap_create_from_xpm_d(GTK_WIDGET(toplevel)->window,
		&bitmap, &style->bg[GTK_STATE_NORMAL], icon_xpm);
	colormap = gtk_widget_get_colormap(GTK_WIDGET(toplevel));
	gdk_color_alloc (colormap, &wait_color);
	
	gdk_window_set_icon(GTK_WIDGET(toplevel)->window, (GdkWindow *)NULL,  pixmap, bitmap);			
	
}

GtkWidget*
get_functions_dialog (void)
{

	if(!functions_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "functions.glade", NULL);
		functions_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(functions_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (functions_glade, "functions_dialog") != NULL);
	
		tFunctionCategories = glade_xml_get_widget (functions_glade, "functions_treeview_category");
		tFunctions	= glade_xml_get_widget (functions_glade, "functions_treeview_function");

		tFunctions_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tFunctions), GTK_TREE_MODEL(tFunctions_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Function"), renderer, "text", 0, NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tFunctions), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tFunctions_selection_changed), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tFunctions_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tFunctions_store), 0, GTK_SORT_ASCENDING);

		tFunctionCategories_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tFunctionCategories), GTK_TREE_MODEL(tFunctionCategories_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Category"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tFunctionCategories), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tFunctionCategories_selection_changed), NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tFunctionCategories_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tFunctionCategories_store), 0, GTK_SORT_ASCENDING);

		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (functions_glade, "functions_textview_description")));
		gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
		gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);

		glade_xml_signal_autoconnect(functions_glade);

		update_functions_tree();
	}

	return glade_xml_get_widget (functions_glade, "functions_dialog");
}

GtkWidget*
get_variables_dialog (void)
{
	if(!variables_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "variables.glade", NULL);
		variables_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(variables_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (variables_glade, "variables_dialog") != NULL);

		tVariableCategories = glade_xml_get_widget (variables_glade, "variables_treeview_category");
		tVariables = glade_xml_get_widget (variables_glade, "variables_treeview_variable");

		tVariables_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tVariables), GTK_TREE_MODEL(tVariables_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Variable"), renderer, "text", 0, NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tVariables), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Value"), renderer, "text", 1, NULL);
		gtk_tree_view_column_set_sort_column_id(column, 1);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tVariables), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tVariables_selection_changed), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tVariables_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tVariables_store), 1, int_string_sort_func, GINT_TO_POINTER(1), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tVariables_store), 0, GTK_SORT_ASCENDING);

		gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tVariables), TRUE);

		tVariableCategories_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tVariableCategories), GTK_TREE_MODEL(tVariableCategories_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Category"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tVariableCategories), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tVariableCategories_selection_changed), NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tVariableCategories_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tVariableCategories_store), 0, GTK_SORT_ASCENDING);

		glade_xml_signal_autoconnect(variables_glade);

		update_variables_tree();

	}
	
	return glade_xml_get_widget (variables_glade, "variables_dialog");
}

GtkWidget*
get_units_dialog (void)
{

	if(!units_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "units.glade", NULL);
		units_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(units_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (units_glade, "units_dialog") != NULL);
	
		tUnitCategories = glade_xml_get_widget (units_glade, "units_treeview_category");
		tUnits		= glade_xml_get_widget (units_glade, "units_treeview_unit");

		tUnits_store = gtk_list_store_new(UNITS_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tUnits), GTK_TREE_MODEL(tUnits_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", UNITS_TITLE_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, UNITS_TITLE_COLUMN);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Unit"), renderer, "text", UNITS_NAMES_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, UNITS_NAMES_COLUMN);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Unit"), renderer, "text", UNITS_BASE_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, UNITS_BASE_COLUMN);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tUnits_selection_changed), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_TITLE_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_TITLE_COLUMN), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_NAMES_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_NAMES_COLUMN), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_BASE_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_BASE_COLUMN), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tUnits_store), UNITS_TITLE_COLUMN, GTK_SORT_ASCENDING);

		gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tUnits), TRUE);

		tUnitCategories_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tUnitCategories), GTK_TREE_MODEL(tUnitCategories_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Category"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tUnitCategories), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tUnitCategories_selection_changed), NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnitCategories_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tUnitCategories_store), 0, GTK_SORT_ASCENDING);

		glade_xml_signal_autoconnect(units_glade);

		update_units_tree();
	
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (units_glade, "units_entry_from_val")), "1");	
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (units_glade, "units_entry_to_val")), "1");		
	
	}
	
	return glade_xml_get_widget (units_glade, "units_dialog");
}

GtkWidget*
get_preferences_dialog (void)
{
	if(!preferences_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "preferences.glade", NULL);
		preferences_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(preferences_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (preferences_glade, "preferences_dialog") != NULL);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_fetch_exchange_rates")), fetch_exchange_rates_at_startup);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_save_mode")), save_mode_on_exit);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_unicode_signs")), printops.use_unicode_signs);	
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_lower_case_numbers")), printops.lower_case_numbers);	
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_save_defs")), save_defs_on_exit);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_custom_result_font")), use_custom_result_font);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_custom_expression_font")), use_custom_expression_font);		
		gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_result_font"), use_custom_result_font);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_button_result_font")), custom_result_font.c_str());
		gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_expression_font"), use_custom_expression_font);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_button_expression_font")), custom_expression_font.c_str());
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot")), SIGN_MULTIDOT);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_ex")), SIGN_MULTIPLICATION);
		if(multi_sign == SIGN_MULTIDOT) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot")), TRUE);
		} else if(multi_sign == SIGN_MULTIPLICATION) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_ex")), TRUE);
		} else if(multi_sign == "*") {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_asterisk")), TRUE);
		}
		
		glade_xml_signal_autoconnect(preferences_glade);
		
	}

	return glade_xml_get_widget (preferences_glade, "preferences_dialog");
}

GtkWidget*
get_unit_edit_dialog (void)
{

	if(!unitedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "unitedit.glade", NULL);
		unitedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(unitedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (unitedit_glade, "unit_edit_dialog") != NULL);
		
		glade_xml_signal_autoconnect(unitedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(unsigned int i = 0; i < CALCULATOR->units.size(); i++) {
		if(!CALCULATOR->units[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->units[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->units[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->units[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (unitedit_glade, "unit_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	return glade_xml_get_widget (unitedit_glade, "unit_edit_dialog");
}

GtkWidget*
get_function_edit_dialog (void)
{

	if(!functionedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "functionedit.glade", NULL);
		functionedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(functionedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (functionedit_glade, "function_edit_dialog") != NULL);
		
		tFunctionArguments = glade_xml_get_widget (functionedit_glade, "function_edit_treeview_arguments");
		tFunctionArguments_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tFunctionArguments), GTK_TREE_MODEL(tFunctionArguments_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionArguments));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tFunctionArguments), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Type"), renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tFunctionArguments), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tFunctionArguments_selection_changed), NULL);
		
		tSubfunctions = glade_xml_get_widget (functionedit_glade, "function_edit_treeview_subfunctions");
		tSubfunctions_store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_BOOLEAN);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tSubfunctions), GTK_TREE_MODEL(tSubfunctions_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tSubfunctions));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Reference"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tSubfunctions), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Expression"), renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tSubfunctions), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Precalculate"), renderer, "text", 2, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tSubfunctions), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tSubfunctions_selection_changed), NULL);
		
		glade_xml_signal_autoconnect(functionedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(unsigned int i = 0; i < CALCULATOR->functions.size(); i++) {
		if(!CALCULATOR->functions[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->functions[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->functions[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->functions[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (functionedit_glade, "function_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);
	
	return glade_xml_get_widget (functionedit_glade, "function_edit_dialog");
}
GtkWidget*
get_variable_edit_dialog (void)
{
	
	if(!variableedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "variableedit.glade", NULL);
		variableedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(variableedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (variableedit_glade, "variable_edit_dialog") != NULL);
		
		glade_xml_signal_autoconnect(variableedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->variables[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (variableedit_glade, "variable_edit_combo_category")), items);
	g_hash_table_destroy(hash);
	g_list_free(items);

	return glade_xml_get_widget (variableedit_glade, "variable_edit_dialog");
}

GtkWidget*
get_unknown_edit_dialog (void)
{
	
	if(!unknownedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "unknownedit.glade", NULL);
		unknownedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(unknownedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (unknownedit_glade, "unknown_edit_dialog") != NULL);
		
		glade_xml_signal_autoconnect(unknownedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->variables[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (unknownedit_glade, "unknown_edit_combo_category")), items);
	g_hash_table_destroy(hash);
	g_list_free(items);

	return glade_xml_get_widget (unknownedit_glade, "unknown_edit_dialog");
}

GtkWidget*
get_matrix_edit_dialog (void)
{
	if(!matrixedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "matrixedit.glade", NULL);
		matrixedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(matrixedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (matrixedit_glade, "matrix_edit_dialog") != NULL);
		
		glade_xml_signal_autoconnect(matrixedit_glade);

		if(element_entries.size() == 0) {
			element_entries.resize(1);
			element_entries[0].push_back(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_1x1"));
		}
	
	}

	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->variables[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (matrixedit_glade, "matrix_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);


	return glade_xml_get_widget (matrixedit_glade, "matrix_edit_dialog");
}
GtkWidget*
get_csv_import_dialog (void)
{

	if(!csvimport_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "csvimport.glade", NULL);
		csvimport_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(csvimport_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (csvimport_glade, "csv_import_dialog") != NULL);
		
		glade_xml_signal_autoconnect(csvimport_glade);
	
	}
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_append(items, (gpointer) CALCULATOR->variables[i]->category().c_str());
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (csvimport_glade, "csv_import_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	return glade_xml_get_widget (csvimport_glade, "csv_import_dialog");
}

GtkWidget*
get_csv_export_dialog (void)
{

	if(!csvexport_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "csvexport.glade", NULL);
		csvexport_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(csvexport_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (csvexport_glade, "csv_export_dialog") != NULL);
		
		glade_xml_signal_autoconnect(csvexport_glade);
	
	}
	
	return glade_xml_get_widget (csvexport_glade, "csv_export_dialog");
	
}

GtkWidget* get_number_base_expression_dialog (void) {
	if(!nbexpression_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "nbexpression.glade", NULL);
		nbexpression_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(nbexpression_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (nbexpression_glade, "number_base_expression_dialog") != NULL);
		
		switch(evalops.parse_options.base) {
			case BASE_BINARY: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_radiobutton_binary")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
				break;
			}
			case BASE_OCTAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_radiobutton_octal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
				break;
			}
			case BASE_DECIMAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_radiobutton_decimal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
				break;
			}
			case BASE_HEXADECIMAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_radiobutton_hexadecimal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
				break;
			}
			case BASE_ROMAN_NUMERALS: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_radiobutton_roman")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
				break;
			}
			default: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_radiobutton_custom_base")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), TRUE);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base")), evalops.parse_options.base);
			}
		}
		
		glade_xml_signal_autoconnect(nbexpression_glade);
	
	}

	return glade_xml_get_widget (nbexpression_glade, "number_base_expression_dialog");
}

GtkWidget*
get_nbases_dialog (void)
{
	if(!nbases_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "nbases.glade", NULL);
		nbases_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(nbases_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (nbases_glade, "nbases_dialog") != NULL);
		
		glade_xml_signal_autoconnect(nbases_glade);
	
	}

	return glade_xml_get_widget (nbases_glade, "nbases_dialog");
}

GtkWidget *create_InfoWidget(const gchar *text) {

	GtkWidget *alignment, *hbox, *image, *infolabel;

	alignment = gtk_alignment_new(0, 0.5, 0, 1);
	gtk_widget_show(alignment);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(alignment), hbox);

	image = gtk_image_new_from_stock("gtk-dialog-info", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show(image);
	gtk_box_pack_start (GTK_BOX(hbox), image, FALSE, TRUE, 0);

	infolabel = gtk_label_new(text);
	gtk_widget_show(infolabel);
	gtk_box_pack_start(GTK_BOX(hbox), infolabel, FALSE, FALSE, 0);
	gtk_label_set_justify(GTK_LABEL(infolabel), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap(GTK_LABEL(infolabel), TRUE);

	return alignment;
}

GtkWidget* get_about_dialog (void) {
	if(!about_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "about.glade", NULL);
		about_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(about_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (about_glade, "about_dialog") != NULL);
		
		glade_xml_signal_autoconnect(about_glade);
	
	}

	return glade_xml_get_widget (about_glade, "about_dialog");
}
GtkWidget* get_argument_rules_dialog (void) {
	
	if(!argumentrules_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "argumentrules.glade", NULL);
		argumentrules_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(argumentrules_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (argumentrules_glade, "argument_rules_dialog") != NULL);
		
		glade_xml_signal_autoconnect(argumentrules_glade);
	
	}

	return glade_xml_get_widget (argumentrules_glade, "argument_rules_dialog");	
}
GtkWidget* get_decimals_dialog (void) {
	if(!decimals_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "decimals.glade", NULL);
		decimals_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(decimals_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (decimals_glade, "decimals_dialog") != NULL);
		
		glade_xml_signal_autoconnect(decimals_glade);
	
	}

	return glade_xml_get_widget (decimals_glade, "decimals_dialog");
}
GtkWidget* get_plot_dialog (void) {
	if(!plot_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "plot.glade", NULL);
		plot_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(plot_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (plot_glade, "plot_dialog") != NULL);
		
		tPlotFunctions = glade_xml_get_widget (plot_glade, "plot_treeview_data");
		tPlotFunctions_store = gtk_list_store_new(7, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tPlotFunctions), GTK_TREE_MODEL(tPlotFunctions_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Title"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tPlotFunctions), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Expression"), renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tPlotFunctions), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tPlotFunctions_selection_changed), NULL);

		
		glade_xml_signal_autoconnect(plot_glade);
	
	}
		
	return glade_xml_get_widget (plot_glade, "plot_dialog");		
}
GtkWidget* get_precision_dialog (void) {
	if(!precision_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "precision.glade", NULL);
		precision_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(precision_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (precision_glade, "precision_dialog") != NULL);
		
		glade_xml_signal_autoconnect(precision_glade);
	
	}

	return glade_xml_get_widget (precision_glade, "precision_dialog");
}
GtkWidget* get_unit_dialog (void) {
	if(!unit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "unit.glade", NULL);
		unit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(unit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (unit_glade, "unit_dialog") != NULL);
		
		glade_xml_signal_autoconnect(unit_glade);
	
	}

	return glade_xml_get_widget (unit_glade, "unit_dialog");
}

