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
#include "Calculator.h"

/* from main.cc */
extern GladeXML *glade_xml;

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

GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;

GtkWidget *expression;
GtkWidget *resultview;
GtkWidget *f_menu ,*v_menu, *u_menu, *u_menu2;
GtkAccelGroup *accel_group;

extern int display_mode, number_base, fractional_mode;
extern bool show_more, show_buttons;
extern bool use_short_units, save_mode_on_exit, save_defs_on_exit, load_global_defs, use_unicode_signs, hyp_is_on, fraction_is_on, use_prefixes;
extern bool use_custom_font, indicate_infinite_series;
extern string custom_font;

extern vector<vector<GtkWidget*> > element_entries;

void
create_main_window (void)
{
	/* make sure we get a valid main window */
	g_assert (NULL != glade_xml_get_widget (glade_xml, "main_window"));

	expression = glade_xml_get_widget (glade_xml, "expression");
	resultview = glade_xml_get_widget (glade_xml, "resultview");
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "history"))), "red_foreground", "foreground", "red", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "history"))), "blue_foreground", "foreground", "blue", NULL);

	/* the function table */
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (glade_xml, "button_xy")))),
			TRUE);
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (glade_xml, "button_fraction")))),
			TRUE);
			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "button_hyp")), hyp_is_on);			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "button_fraction")), fractional_mode == FRACTIONAL_MODE_FRACTION);
//	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "button_inexact")), !CALCULATOR->alwaysExact());					
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_exact_mode")), CALCULATOR->alwaysExact());


	accel_group = gtk_accel_group_new ();


	switch (CALCULATOR->angleMode())
	{
	case DEGREES:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_degrees")), TRUE);
		break;
	case RADIANS:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_radians")), TRUE);
		break;
	case GRADIANS:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_gradians")), TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	switch (number_base)
	{
	case BASE_OCTAL:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_octal")
				),
				TRUE);
		break;
	case BASE_DECI:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_decimal")
				),
				TRUE);
		break;
	case BASE_HEX:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_hexadecimal")
				),
				TRUE);
		break;
	case BASE_BIN:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_binary")
				),
				TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	switch (display_mode)
	{
	case MODE_NORMAL:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_display_normal")
					),
				TRUE);
		break;
	case MODE_SCIENTIFIC:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_display_scientific")
					),
				TRUE);
		break;
	case MODE_SCIENTIFIC_PURE:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_display_purely_scientific")
					),
				TRUE);
		break;
	case MODE_DECIMALS:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_display_non_scientific")
					),
				TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	gtk_check_menu_item_set_active(
			GTK_CHECK_MENU_ITEM(
				glade_xml_get_widget (glade_xml, "menu_item_display_prefixes")
				),
			use_prefixes);

	gtk_check_menu_item_set_active(
			GTK_CHECK_MENU_ITEM(
				glade_xml_get_widget (glade_xml, "menu_item_indicate_infinite_series")
				),
			indicate_infinite_series);

	switch (fractional_mode)
	{
	case FRACTIONAL_MODE_DECIMAL:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_fraction_decimal")
					),
				TRUE);
		break;
	case FRACTIONAL_MODE_COMBINED:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_fraction_combined")
					),
				TRUE);
		break;		
	case FRACTIONAL_MODE_FRACTION:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_fraction_fraction")
					),
				TRUE);
		break;		
	default:
		g_assert_not_reached ();
		break;
	}

	if(show_more)
	{
		gtk_widget_show (glade_xml_get_widget (glade_xml, "notebook"));
	}
	else
	{
		gtk_widget_hide (glade_xml_get_widget (glade_xml, "notebook"));
	}

	switch (CALCULATOR->angleMode())
	{
	case RADIANS:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "radiobutton_radians")), TRUE);
		break;
	case DEGREES:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "radiobutton_degrees")), TRUE);
		break;
	case GRADIANS:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "radiobutton_gradians")), TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	if(show_buttons)
	{
		gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (glade_xml, "notebook")), 1);
	}

	if(show_more)
	{
		gtk_button_set_label (
				GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_less_more")),
				_("<< Less"));
	}
	else
	{
		gtk_button_set_label (
				GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_less_more")),
				_("More >>"));
	}

	if(use_unicode_signs) {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sub")), SIGN_MINUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_add")), SIGN_PLUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_times")), SIGN_MULTIPLICATION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_divide")), SIGN_DIVISION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sqrt")), SIGN_SQRT);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_dot")), SIGN_MULTIDOT);	
//		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_inexact")), SIGN_APPROXIMATELY_EQUAL);			
	}

	if(use_custom_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
	} else {
		PangoFontDescription *font = pango_font_description_copy(resultview->style->font_desc);
//		pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);		
		if(custom_font.empty()) {
			custom_font = pango_font_description_to_string(resultview->style->font_desc);
		}		
	}
	
	g_signal_connect (G_OBJECT (gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression")))), "deactivate",
	                  G_CALLBACK (on_menu_e_deactivate),
	                  NULL);
	g_signal_connect (G_OBJECT (gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result")))), "deactivate",
	                  G_CALLBACK (on_menu_r_deactivate),
	                  NULL);

	gtk_window_add_accel_group (
			GTK_WINDOW (
				glade_xml_get_widget (glade_xml, "main_window")
			),
			accel_group);
	gtk_widget_grab_focus(expression);
	GTK_WIDGET_SET_FLAGS(expression, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(expression);

	glade_xml_signal_autoconnect(glade_xml);
	
//	gtk_widget_modify_bg(resultview, GTK_STATE_NORMAL, &glade_xml_get_widget(glade_xml, "history")->style->base[GTK_WIDGET_STATE(glade_xml_get_widget(glade_xml, "history"))]);	

	gtk_widget_show (glade_xml_get_widget (glade_xml, "main_window"));
	
}

GtkWidget*
create_functions_dialog (void)
{

	tFunctionCategories = glade_xml_get_widget (glade_xml, "tFunctionCategories");
	tFunctions	= glade_xml_get_widget (glade_xml, "tFunctions");



	tFunctions_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
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


	update_functions_tree(glade_xml_get_widget (glade_xml, "functions_dialog"));

	return glade_xml_get_widget (glade_xml, "functions_dialog");
}

GtkWidget*
create_variables_dialog (void)
{

	tVariableCategories = glade_xml_get_widget (glade_xml, "variables_tree_view1");
	tVariables = glade_xml_get_widget (glade_xml, "variables_tree_view2");

	tVariables_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
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

	update_variables_tree(glade_xml_get_widget (glade_xml, "variables_dialog"));

	return glade_xml_get_widget (glade_xml, "variables_dialog");
}

GtkWidget*
create_units_dialog (void)
{

	tUnitCategories = glade_xml_get_widget (glade_xml, "units_tree_view1");
	tUnits		= glade_xml_get_widget (glade_xml, "units_tree_view2");

	tUnits_store = gtk_list_store_new(UNITS_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
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
	column = gtk_tree_view_column_new_with_attributes(_("Base unit"), renderer, "text", UNITS_BASE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, UNITS_BASE_COLUMN);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Type"), renderer, "text", UNITS_TYPE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, UNITS_TYPE_COLUMN);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);
	g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tUnits_selection_changed), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_TITLE_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_TITLE_COLUMN), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_NAMES_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_NAMES_COLUMN), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_BASE_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_BASE_COLUMN), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_TYPE_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_TYPE_COLUMN), NULL);
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

	update_units_tree(glade_xml_get_widget (glade_xml, "units_dialog"));
	
	gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (glade_xml, "units_entry_from_val")), "1");	
	gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (glade_xml, "units_entry_to_val")), "1");		
	
	return glade_xml_get_widget (glade_xml, "units_dialog");
}

GtkWidget*
create_preferences_dialog (void)
{

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_load_defs")), load_global_defs);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_save_mode")), save_mode_on_exit);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_save_defs")), save_defs_on_exit);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_short_units")), use_short_units);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_unicode_signs")), use_unicode_signs);	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_custom_font")), use_custom_font);		
	gtk_widget_set_sensitive(glade_xml_get_widget(glade_xml, "preferences_button_font"), use_custom_font);	
	gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "preferences_button_font")), custom_font.c_str());			

	return glade_xml_get_widget (glade_xml, "preferences_dialog");;
}

GtkWidget*
create_unit_edit_dialog (void)
{
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(int i = 0; i < CALCULATOR->units.size(); i++) {
		if(!CALCULATOR->units[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->units[i]->category().c_str()) == NULL) {
				items = g_list_append(items, (gpointer) CALCULATOR->units[i]->category().c_str());
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->units[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (glade_xml, "unit_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	return glade_xml_get_widget (glade_xml, "unit_edit_dialog");
}

GtkWidget*
create_function_edit_dialog (void)
{
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(int i = 0; i < CALCULATOR->functions.size(); i++) {
		if(!CALCULATOR->functions[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->functions[i]->category().c_str()) == NULL) {
				items = g_list_append(items, (gpointer) CALCULATOR->functions[i]->category().c_str());
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->functions[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (glade_xml, "function_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	return glade_xml_get_widget (glade_xml, "function_edit_dialog");
}
GtkWidget*
create_variable_edit_dialog (void)
{
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_append(items, (gpointer) CALCULATOR->variables[i]->category().c_str());
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (glade_xml, "variable_edit_combo_category")), items);
	g_hash_table_destroy(hash);
	g_list_free(items);

	return glade_xml_get_widget (glade_xml, "variable_edit_dialog");
}
GtkWidget*
create_matrix_edit_dialog (void)
{
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_append(items, (gpointer) CALCULATOR->variables[i]->category().c_str());
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (glade_xml, "matrix_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	if(element_entries.size() == 0) {
		element_entries.resize(1);
		element_entries[0].push_back(glade_xml_get_widget (glade_xml, "matrix_edit_entry_1x1"));
	}

	return glade_xml_get_widget (glade_xml, "matrix_edit_dialog");
}
GtkWidget*
create_csv_import_dialog (void)
{
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_append(items, (gpointer) CALCULATOR->variables[i]->category().c_str());
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (glade_xml, "csv_import_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	return glade_xml_get_widget (glade_xml, "csv_import_dialog");
}

GtkWidget*
create_nbases_dialog (void)
{
	gtk_widget_show(glade_xml_get_widget (glade_xml, "nbases_dialog"));
	return glade_xml_get_widget (glade_xml, "nbases_dialog");
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

