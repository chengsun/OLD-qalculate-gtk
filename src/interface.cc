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
GtkListStore *tFunctionCategories_store;

GtkWidget *tVariableCategories;
GtkWidget *tVariables;
GtkListStore *tVariables_store;
GtkListStore *tVariableCategories_store;

GtkWidget *tUnitCategories;
GtkWidget *tUnits;
GtkListStore *tUnits_store;
GtkListStore *tUnitCategories_store;

GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;

GtkWidget *expression;
GtkWidget *result;
GtkWidget *f_menu ,*v_menu, *u_menu, *u_menu2;
GtkAccelGroup *accel_group;

extern int display_mode, number_base;
extern bool show_more, show_buttons;
extern Calculator *calc;
extern bool use_short_units, save_mode_on_exit, save_defs_on_exit, load_global_defs;


void
create_main_window (void)
{
	/* make sure we get a valid main window */
	g_assert (NULL != glade_xml_get_widget (glade_xml, "main_window"));

	expression = glade_xml_get_widget (glade_xml, "expression");
	result = glade_xml_get_widget (glade_xml, "result");
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "history"))), "red_foreground", "foreground", "red", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "history"))), "blue_foreground", "foreground", "blue", NULL);

	/* the function table */
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (glade_xml, "button_xy")))),
			TRUE);
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (glade_xml, "button_square")))),
			TRUE);

	accel_group = gtk_accel_group_new ();


	switch (calc->angleMode())
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
	case MODE_DECIMALS:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_display_non_scientific")
					),
				TRUE);
		break;
	case MODE_PREFIXES:
		gtk_check_menu_item_set_active(
				GTK_CHECK_MENU_ITEM(
					glade_xml_get_widget (glade_xml, "menu_item_display_prefixes")
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

	switch (calc->angleMode())
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



	tFunctionCategories_store = gtk_list_store_new(1, G_TYPE_STRING);
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

	tVariables_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
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

	tVariableCategories_store = gtk_list_store_new(1, G_TYPE_STRING);
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

	tUnits_store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
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

	tUnitCategories_store = gtk_list_store_new(1, G_TYPE_STRING);
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

	return glade_xml_get_widget (glade_xml, "units_dialog");
}

GtkWidget*
create_preferences_dialog (void)
{

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_load_defs")), load_global_defs);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_save_mode")), save_mode_on_exit);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_save_defs")), save_defs_on_exit);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "preferences_checkbutton_short_units")), use_short_units);

	return glade_xml_get_widget (glade_xml, "preferences_dialog");;
}

GtkWidget*
create_unit_edit_dialog (void)
{
	/* FIXME populate the combo menus */

	return glade_xml_get_widget (glade_xml, "unit_edit_dialog");
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

