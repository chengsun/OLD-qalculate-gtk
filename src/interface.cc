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

GtkWidget *wFunctions;
GtkWidget *tFunctionCategories;
GtkWidget *tFunctions;
GtkWidget *bNewFunction;
GtkWidget *bInsertFunction;
GtkWidget *bEditFunction;
GtkWidget *bDeleteFunction;
GtkWidget *lFunctionDescription;
GtkWidget *bCloseFunctions;
GtkListStore *tFunctions_store;
GtkListStore *tFunctionCategories_store;

GtkWidget *wVariables;
GtkWidget *tVariableCategories;
GtkWidget *tVariables;
GtkWidget *bNewVariable;
GtkWidget *bInsertVariable;
GtkWidget *bEditVariable;
GtkWidget *bDeleteVariable;
GtkWidget *bCloseVariables;
GtkListStore *tVariables_store;
GtkListStore *tVariableCategories_store;

GtkWidget *wUnits;
GtkWidget *tUnitCategories;
GtkWidget *tUnits;
GtkWidget *bNewUnit;
GtkWidget *bInsertUnit;
GtkWidget *bConvertToUnit;
GtkWidget *bEditUnit;
GtkWidget *bDeleteUnit;
GtkWidget *bCloseUnits;
GtkWidget *eFromValue;
GtkWidget *lFromUnit;
GtkWidget *tbToFrom;
GtkWidget *tbToTo;
GtkWidget *bConvertUnits;
GtkWidget *eToValue;
GtkWidget *omToUnit;
GtkListStore *tUnits_store;
GtkListStore *tUnitCategories_store;

GtkWidget *wEditUnit;
GtkWidget *lUnitType;
GtkWidget *omUnitType;
GtkWidget *lUnitName;
GtkWidget *eUnitName;
GtkWidget *lUnitPlural;
GtkWidget *eUnitPlural;
GtkWidget *lShortUnitFormat;
GtkWidget *eShortUnitFormat;
GtkWidget *boxAlias;
GtkWidget *lBaseUnit;
GtkWidget *eBaseUnit;
GtkObject *sbBaseExp_adj;
GtkWidget *sbBaseExp;
GtkWidget *lRelation;
GtkWidget *eRelation;
GtkWidget *lReverse;
GtkWidget *eReverse;
GtkWidget *lUnitCat;
GtkWidget *eUnitCat;
GtkWidget *lDescrUnitName;
GtkWidget *eDescrUnitName;


GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;


GtkWidget *tableT;
GtkWidget *bClose;
GtkWidget *history_scrolled;
GtkWidget *history;
GtkWidget *expression;
GtkWidget *result;
GtkWidget *bEXE;
GtkWidget *bHistory;
GtkWidget *f_menu ,*v_menu, *u_menu, *u_menu2;
GtkWidget *bMenuE;
GtkWidget *bMenuR;
GtkWidget *tabs;
GtkWidget *bSQRT;
GtkWidget *bXY;
GtkWidget *bLog;
GtkWidget *bLn;
GtkWidget *bSin;
GtkWidget *bCos;
GtkWidget *bTan;
GtkWidget *bSTO;
GtkWidget *bLeftP;
GtkWidget *bRightP;
GtkWidget *bX2;
GtkWidget *bHyp;
GtkWidget *b7;
GtkWidget *b4;
GtkWidget *b1;
GtkWidget *b8;
GtkWidget *b2;
GtkWidget *b5;
GtkWidget *b9;
GtkWidget *b6;
GtkWidget *b3;
GtkWidget *b0;
GtkWidget *bDot;
GtkWidget *bEXP;
GtkWidget *bDEL;
GtkWidget *bAC;
GtkWidget *bMulti;
GtkWidget *bDivi;
GtkWidget *bPlus;
GtkWidget *bMinus;
GtkWidget *bAns;
GtkWidget *bEquals;
GtkWidget *rRad;
GtkWidget *rDeg;
GtkWidget *rGra;
GtkWidget *mRad;
GtkWidget *mDeg;
GtkWidget *mGra;
GSList *dmode_group = NULL;
GSList *bmode_group = NULL;
GtkWidget *arrow1, *arrow2;
GtkAccelGroup *accel_group;

extern int display_mode, number_base;
extern bool show_more, show_buttons;
extern Calculator *calc;
extern bool use_short_units, save_mode_on_exit, save_defs_on_exit, load_global_defs;


void
create_window (void)
{
	/* make sure we get a valid main window */
	g_assert (NULL != glade_xml_get_widget (glade_xml, "main_window"));

	accel_group = gtk_accel_group_new ();

	mDeg = glade_xml_get_widget (glade_xml, "menu_item_degrees");
	mRad = glade_xml_get_widget (glade_xml, "menu_item_radians");
	mGra = glade_xml_get_widget (glade_xml, "menu_item_gradians");

	switch (calc->angleMode())
	{
	case DEGREES:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mDeg), TRUE);
		break;
	case RADIANS:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mRad), TRUE);
		break;
	case GRADIANS:
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mGra), TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	g_signal_connect (G_OBJECT (mDeg), "activate", GTK_SIGNAL_FUNC(set_angle_mode), GINT_TO_POINTER (DEGREES));
	g_signal_connect (G_OBJECT (mRad), "activate", GTK_SIGNAL_FUNC(set_angle_mode), GINT_TO_POINTER (RADIANS));
	g_signal_connect (G_OBJECT (mGra), "activate", GTK_SIGNAL_FUNC(set_angle_mode), GINT_TO_POINTER (GRADIANS));

	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_addition")),
			"activate",
			G_CALLBACK(insert_sign),
			GINT_TO_POINTER ('+'));
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_subtraction")),
			"activate",
			G_CALLBACK(insert_sign),
			GINT_TO_POINTER ('-'));
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_multiplication")),
			"activate",
			G_CALLBACK(insert_sign),
			GINT_TO_POINTER ('*'));
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_division")),
			"activate",
			G_CALLBACK(insert_sign),
			GINT_TO_POINTER ('/'));
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_power")),
			"activate",
			G_CALLBACK(insert_sign),
			GINT_TO_POINTER ('^'));
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_exponent")),
			"activate",
			G_CALLBACK(insert_sign),
			GINT_TO_POINTER ('E'));
	
	//  CHECK_MENU_ITEM("Clean mode", set_clean_mode);
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_save_defs")),
			"activate",
			GTK_SIGNAL_FUNC(save_defs),
			NULL);
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_save_mode")),
			"activate",
			GTK_SIGNAL_FUNC(save_mode),
			NULL);
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_edit_prefs")),
			"activate",
			GTK_SIGNAL_FUNC(edit_preferences),
			NULL);

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
	default:
		g_assert_not_reached ();
		break;
	}

	g_signal_connect (G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_hexadecimal")), "activate", GTK_SIGNAL_FUNC(set_number_base), GINT_TO_POINTER (BASE_HEX));
	g_signal_connect (G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_octal")), "activate", GTK_SIGNAL_FUNC(set_number_base), GINT_TO_POINTER (BASE_OCTAL));
	g_signal_connect (G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_decimal")), "activate", GTK_SIGNAL_FUNC(set_number_base), GINT_TO_POINTER (BASE_DECI));

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
	
	g_signal_connect (G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_display_normal")), "activate", G_CALLBACK(set_display_mode), GINT_TO_POINTER (MODE_NORMAL));
	g_signal_connect (G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_display_scientific")), "activate", G_CALLBACK(set_display_mode), GINT_TO_POINTER (MODE_SCIENTIFIC));
	g_signal_connect (G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_display_non_scientific")), "activate", G_CALLBACK(set_display_mode), GINT_TO_POINTER (MODE_DECIMALS));
	g_signal_connect (G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_display_prefixes")), "activate", G_CALLBACK(set_display_mode), GINT_TO_POINTER (MODE_PREFIXES));


	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_save")),
			"activate",
			G_CALLBACK(add_as_variable),
			NULL);
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_precision")),
			"activate",
			G_CALLBACK(select_precision),
			NULL);
	g_signal_connect (
			G_OBJECT (glade_xml_get_widget (glade_xml, "menu_item_decimals")),
			"activate",
			G_CALLBACK(select_decimals),
			NULL);

	tableT = glade_xml_get_widget (glade_xml, "top_table");
	bMenuE = glade_xml_get_widget (glade_xml, "togglebutton_expression");
	arrow1 = glade_xml_get_widget (glade_xml, "togglebutton_expression_arrow");
	bMenuR = glade_xml_get_widget (glade_xml, "togglebutton_result");
	arrow2 = glade_xml_get_widget (glade_xml, "togglebutton_result_arrow");
	expression = glade_xml_get_widget (glade_xml, "expression");
	result = glade_xml_get_widget (glade_xml, "result");

	tabs = glade_xml_get_widget (glade_xml, "notebook");
	
	if(show_more)
	{
		gtk_widget_show (tabs);
	}
	else
	{
		gtk_widget_hide (tabs);
	}
	

	history_scrolled = glade_xml_get_widget (glade_xml, "history_page");
	history = glade_xml_get_widget (glade_xml, "history");
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(history)), "red_foreground", "foreground", "red", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(history)), "blue_foreground", "foreground", "blue", NULL);

	/* the function table */
	bSQRT	= glade_xml_get_widget (glade_xml, "button_sqrt");
	bXY	= glade_xml_get_widget (glade_xml, "button_xy");
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(bXY))),
			TRUE);
	bLog	= glade_xml_get_widget (glade_xml, "button_log");
	bLn	= glade_xml_get_widget (glade_xml, "button_ln");
	bSin	= glade_xml_get_widget (glade_xml, "button_sine");
	bCos	= glade_xml_get_widget (glade_xml, "button_cosine");
	bTan	= glade_xml_get_widget (glade_xml, "button_tan");
	bSTO	= glade_xml_get_widget (glade_xml, "button_store");
	bLeftP	= glade_xml_get_widget (glade_xml, "button_brace_open");
	bRightP = glade_xml_get_widget (glade_xml, "button_brace_close");
	bX2	= glade_xml_get_widget (glade_xml, "button_square");
	gtk_label_set_use_markup (
			GTK_LABEL (gtk_bin_get_child (GTK_BIN(bX2))),
			TRUE);
	bHyp	= glade_xml_get_widget (glade_xml, "button_hyp");

	rDeg	= glade_xml_get_widget (glade_xml, "radiobutton_degrees");
	rRad	= glade_xml_get_widget (glade_xml, "radiobutton_radians");
	rGra	= glade_xml_get_widget (glade_xml, "radiobutton_gradians");

	switch (calc->angleMode())
	{
	case RADIANS:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rRad), TRUE);
		break;
	case DEGREES:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rDeg), TRUE);
		break;
	case GRADIANS:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rGra), TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	/* the standard input table */
	b0	= glade_xml_get_widget (glade_xml, "button_zero");
	b1	= glade_xml_get_widget (glade_xml, "button_one");
	b2      = glade_xml_get_widget (glade_xml, "button_two");
	b3      = glade_xml_get_widget (glade_xml, "button_three");
	b4	= glade_xml_get_widget (glade_xml, "button_four");
	b5      = glade_xml_get_widget (glade_xml, "button_five");
	b6      = glade_xml_get_widget (glade_xml, "button_six");
	b7	= glade_xml_get_widget (glade_xml, "button_seven");
	b8	= glade_xml_get_widget (glade_xml, "button_eight");
	b9      = glade_xml_get_widget (glade_xml, "button_nine");
	bDot	= glade_xml_get_widget (glade_xml, "button_dot");
	bEXP	= glade_xml_get_widget (glade_xml, "button_exp");
	bDEL	= glade_xml_get_widget (glade_xml, "button_del");
	bAC	= glade_xml_get_widget (glade_xml, "button_ac");
	bMulti	= glade_xml_get_widget (glade_xml, "button_times");
	bDivi	= glade_xml_get_widget (glade_xml, "button_divide");
	bPlus	= glade_xml_get_widget (glade_xml, "button_add");
	bMinus	= glade_xml_get_widget (glade_xml, "button_sub");
	bAns	= glade_xml_get_widget (glade_xml, "button_ans");
	bEquals	= glade_xml_get_widget (glade_xml, "button_equals");

	if(show_buttons)
	{
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), 1);
	}

	bHistory = glade_xml_get_widget (glade_xml, "button_less_more");
	if(show_more)
	{
		gtk_button_set_label (
				GTK_BUTTON(bHistory),
				_("<< Less"));
	}
	else
	{		
		gtk_button_set_label (
				GTK_BUTTON(bHistory),
				_("More >>"));
	}
	
	bClose	= glade_xml_get_widget (glade_xml, "button_close");
	bEXE	= glade_xml_get_widget (glade_xml, "button_execute");

	g_signal_connect (G_OBJECT (b0), "clicked", G_CALLBACK (button_pressed), (gpointer) "0");
	g_signal_connect (G_OBJECT (b1), "clicked", G_CALLBACK (button_pressed), (gpointer) "1");
	g_signal_connect (G_OBJECT (b2), "clicked", G_CALLBACK (button_pressed), (gpointer) "2");
	g_signal_connect (G_OBJECT (b3), "clicked", G_CALLBACK (button_pressed), (gpointer) "3");
	g_signal_connect (G_OBJECT (b4), "clicked", G_CALLBACK (button_pressed), (gpointer) "4");
	g_signal_connect (G_OBJECT (b5), "clicked", G_CALLBACK (button_pressed), (gpointer) "5");
	g_signal_connect (G_OBJECT (b6), "clicked", G_CALLBACK (button_pressed), (gpointer) "6");
	g_signal_connect (G_OBJECT (b7), "clicked", G_CALLBACK (button_pressed), (gpointer) "7");
	g_signal_connect (G_OBJECT (b8), "clicked", G_CALLBACK (button_pressed), (gpointer) "8");
	g_signal_connect (G_OBJECT (b9), "clicked", G_CALLBACK (button_pressed), (gpointer) "9");
	g_signal_connect (G_OBJECT (bDot), "clicked", G_CALLBACK (button_pressed), (gpointer) ".");
	g_signal_connect (G_OBJECT (bEXP), "clicked", G_CALLBACK (button_pressed), (gpointer) "E");
	g_signal_connect (G_OBJECT (bMulti), "clicked", G_CALLBACK (button_pressed), (gpointer) "*");
	g_signal_connect (G_OBJECT (bDivi), "clicked", G_CALLBACK (button_pressed), (gpointer) "/");
	g_signal_connect (G_OBJECT (bPlus), "clicked", G_CALLBACK (button_pressed), (gpointer) "+");
	g_signal_connect (G_OBJECT (bMinus), "clicked", G_CALLBACK (button_pressed), (gpointer) "-");
	g_signal_connect (G_OBJECT (bAns), "clicked", G_CALLBACK (button_pressed), (gpointer) "Ans");
	g_signal_connect (G_OBJECT (bLog), "clicked", G_CALLBACK (button_function_pressed), (gpointer) "log");
	g_signal_connect (G_OBJECT (bLn), "clicked", G_CALLBACK (button_function_pressed), (gpointer) "ln");
	g_signal_connect (G_OBJECT (bXY), "clicked", G_CALLBACK (button_pressed), (gpointer) "^");
	g_signal_connect (G_OBJECT (bX2), "clicked", G_CALLBACK (button_pressed), (gpointer) "^2");
	g_signal_connect (G_OBJECT (bLeftP), "clicked", G_CALLBACK (button_pressed), (gpointer) "(");
	g_signal_connect (G_OBJECT (bRightP), "clicked", G_CALLBACK (button_pressed), (gpointer) ")");
	g_signal_connect (G_OBJECT (bSQRT), "clicked", G_CALLBACK (button_function_pressed), (gpointer) "sqrt");

	g_signal_connect (G_OBJECT (bEquals), "clicked", G_CALLBACK (on_bEXE_clicked), NULL);
	g_signal_connect (G_OBJECT (bAC), "clicked", G_CALLBACK (on_bAC_clicked), NULL);
	g_signal_connect (G_OBJECT (bDEL), "clicked", G_CALLBACK (on_bDEL_clicked), NULL);
	g_signal_connect (G_OBJECT (bHyp), "toggled", G_CALLBACK (on_bHyp_clicked), NULL);
	g_signal_connect (G_OBJECT (bTan), "clicked", G_CALLBACK (on_bTan_clicked), NULL);
	g_signal_connect (G_OBJECT (bSin), "clicked", G_CALLBACK (on_bSin_clicked), NULL);
	g_signal_connect (G_OBJECT (bCos), "clicked", G_CALLBACK (on_bCos_clicked), NULL);
	g_signal_connect (G_OBJECT (bSTO), "clicked", G_CALLBACK (on_bSTO_clicked), NULL);


	g_signal_connect (
			G_OBJECT (
				glade_xml_get_widget (glade_xml, "main_window")
			),
			"delete_event",
	                G_CALLBACK (on_gcalc_exit),
	                NULL);
	g_signal_connect (
			G_OBJECT (
				glade_xml_get_widget (glade_xml, "main_window")
			),
			"destroy_event",
	                G_CALLBACK (on_gcalc_exit),
	                NULL);
	g_signal_connect (G_OBJECT (rRad), "toggled",
	                  G_CALLBACK (on_rRad_toggled),
	                  NULL);
	g_signal_connect (G_OBJECT (rDeg), "toggled",
	                  G_CALLBACK (on_rDeg_toggled),
	                  NULL);
	g_signal_connect (G_OBJECT (rGra), "toggled",
	                  G_CALLBACK (on_rGra_toggled),
	                  NULL);
	g_signal_connect (G_OBJECT (expression), "activate",
	                  G_CALLBACK (on_expression_activate),
	                  NULL);
	g_signal_connect (G_OBJECT (bHistory), "clicked",
	                  G_CALLBACK (on_bHistory_clicked),
	                  NULL);
	g_signal_connect (G_OBJECT (bEXE), "clicked",
	                  G_CALLBACK (on_bEXE_clicked),
	                  NULL);
	g_signal_connect (G_OBJECT (bClose), "clicked",
	                  G_CALLBACK (on_bClose_clicked),
	                  NULL);
	g_signal_connect (G_OBJECT (bMenuE), "toggled",
	                  G_CALLBACK (on_bMenuE_toggled),
	                  NULL);
	g_signal_connect (G_OBJECT (bMenuR), "toggled",
	                  G_CALLBACK (on_bMenuR_toggled),
	                  NULL);
	g_signal_connect (G_OBJECT (gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression")))), "deactivate",
	                  G_CALLBACK (on_menu_e_deactivate),
	                  NULL);
	g_signal_connect (G_OBJECT (gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result")))), "deactivate",
	                  G_CALLBACK (on_menu_r_deactivate),
	                  NULL);
	g_signal_connect (G_OBJECT (expression), "changed",
	                  G_CALLBACK (on_expression_changed),
	                  NULL);

	gtk_window_add_accel_group (
			GTK_WINDOW (
				glade_xml_get_widget (glade_xml, "main_window")
			),
			accel_group);
	gtk_widget_grab_focus(expression);
	GTK_WIDGET_SET_FLAGS(expression, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(expression);

	gtk_widget_show (glade_xml_get_widget (glade_xml, "main_window"));
}

GtkWidget*
create_wFunctions (void)
{
	wFunctions	= glade_xml_get_widget (glade_xml, "wFunctions");
	tFunctionCategories = glade_xml_get_widget (glade_xml, "tFunctionCategories");
	tFunctions	= glade_xml_get_widget (glade_xml, "tFunctions");


	
	tFunctions_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tFunctions), GTK_TREE_MODEL(tFunctions_store));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Function", renderer, "text", 0, NULL);
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
	column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tFunctionCategories), column);
	g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tFunctionCategories_selection_changed), NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tFunctionCategories_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tFunctionCategories_store), 0, GTK_SORT_ASCENDING);



	bNewFunction	= glade_xml_get_widget (glade_xml, "bNewFunction");
	bEditFunction	= glade_xml_get_widget (glade_xml, "bEditFunction");
	bInsertFunction	= glade_xml_get_widget (glade_xml, "bInsertFunction");
	bDeleteFunction	= glade_xml_get_widget (glade_xml, "bDeleteFunction");
	lFunctionDescription = glade_xml_get_widget (glade_xml, "lFunctionDescription");
	bCloseFunctions	= glade_xml_get_widget (glade_xml, "bCloseFunctions");



	g_signal_connect ((gpointer) wFunctions, "delete_event",
	                  G_CALLBACK (on_wFunctions_delete_event),
	                  NULL);
	g_signal_connect ((gpointer) wFunctions, "destroy_event",
	                  G_CALLBACK (on_wFunctions_destroy_event),
	                  NULL);
	g_signal_connect ((gpointer) bNewFunction, "clicked",
	                  G_CALLBACK (on_bNewFunction_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bEditFunction, "clicked",
	                  G_CALLBACK (on_bEditFunction_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bInsertFunction, "clicked",
	                  G_CALLBACK (on_bInsertFunction_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bDeleteFunction, "clicked",
	                  G_CALLBACK (on_bDeleteFunction_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bCloseFunctions, "clicked",
	                  G_CALLBACK (on_bCloseFunctions_clicked),
	                  NULL);

	update_functions_tree(wFunctions);

	return wFunctions;
}

GtkWidget*
create_wVariables (void)
{
	wVariables = glade_xml_get_widget (glade_xml, "variables_dialog");
	tVariableCategories = glade_xml_get_widget (glade_xml, "variables_tree_view1");
	tVariables = glade_xml_get_widget (glade_xml, "variables_tree_view2");

	tVariables_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tVariables), GTK_TREE_MODEL(tVariables_store));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Variable", renderer, "text", 0, NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tVariables), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Value", renderer, "text", 1, NULL);
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
	column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tVariableCategories), column);
	g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tVariableCategories_selection_changed), NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tVariableCategories_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tVariableCategories_store), 0, GTK_SORT_ASCENDING);

	bNewVariable	= glade_xml_get_widget (glade_xml, "variables_button_new");
	bEditVariable	= glade_xml_get_widget (glade_xml, "variables_button_edit");
	bInsertVariable = glade_xml_get_widget (glade_xml, "variables_button_insert");
	bDeleteVariable	= glade_xml_get_widget (glade_xml, "variables_button_delete");
	bCloseVariables = glade_xml_get_widget (glade_xml, "variables_button_close");

	g_signal_connect ((gpointer) wVariables, "delete_event",
	                  G_CALLBACK (on_wVariables_delete_event),
	                  NULL);
	g_signal_connect ((gpointer) wVariables, "destroy_event",
	                  G_CALLBACK (on_wVariables_destroy_event),
	                  NULL);
	g_signal_connect ((gpointer) bNewVariable, "clicked",
	                  G_CALLBACK (on_bNewVariable_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bEditVariable, "clicked",
	                  G_CALLBACK (on_bEditVariable_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bInsertVariable, "clicked",
	                  G_CALLBACK (on_bInsertVariable_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bDeleteVariable, "clicked",
	                  G_CALLBACK (on_bDeleteVariable_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bCloseVariables, "clicked",
	                  G_CALLBACK (on_bCloseVariables_clicked),
	                  NULL);

	update_variables_tree(wVariables);

	return wVariables;
}

GtkWidget*
create_wUnits (void)
{
	wUnits		= glade_xml_get_widget (glade_xml, "units_dialog");
	tUnitCategories = glade_xml_get_widget (glade_xml, "units_tree_view1");
	tUnits		= glade_xml_get_widget (glade_xml, "units_tree_view2");

	tUnits_store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tUnits), GTK_TREE_MODEL(tUnits_store));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", UNITS_TITLE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, UNITS_TITLE_COLUMN);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Unit", renderer, "text", UNITS_NAMES_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, UNITS_NAMES_COLUMN);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Base unit", renderer, "text", UNITS_BASE_COLUMN, NULL);
	gtk_tree_view_column_set_sort_column_id(column, UNITS_BASE_COLUMN);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", UNITS_TYPE_COLUMN, NULL);
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
	column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tUnitCategories), column);
	g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tUnitCategories_selection_changed), NULL);
	gtk_tree_view_column_set_sort_column_id(column, 0);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnitCategories_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tUnitCategories_store), 0, GTK_SORT_ASCENDING);

	eFromValue	= glade_xml_get_widget (glade_xml, "units_entry_from_val");
	lFromUnit	= glade_xml_get_widget (glade_xml, "units_label_from_unit");
	tbToFrom	= glade_xml_get_widget (glade_xml, "units_toggle_button_from");

	bConvertUnits	= glade_xml_get_widget (glade_xml, "units_button_convert");

	tbToTo		= glade_xml_get_widget (glade_xml, "units_toggle_button_to");
	eToValue	= glade_xml_get_widget (glade_xml, "units_entry_to_val");
	omToUnit	= glade_xml_get_widget (glade_xml, "units_option_menu_to_unit");

	bNewUnit	= glade_xml_get_widget (glade_xml, "units_button_new");
	bEditUnit	= glade_xml_get_widget (glade_xml, "units_button_edit");
	bInsertUnit	= glade_xml_get_widget (glade_xml, "units_button_insert");
	bConvertToUnit	= glade_xml_get_widget (glade_xml, "units_button_convert_to");
	bDeleteUnit	= glade_xml_get_widget (glade_xml, "units_button_delete");

	bCloseUnits	= glade_xml_get_widget (glade_xml, "units_button_close");

	g_signal_connect ((gpointer) wUnits, "delete_event",
	                  G_CALLBACK (on_wUnits_delete_event),
	                  NULL);
	g_signal_connect ((gpointer) wUnits, "destroy_event",
	                  G_CALLBACK (on_wUnits_destroy_event),
	                  NULL);
	g_signal_connect ((gpointer) bNewUnit, "clicked",
	                  G_CALLBACK (on_bNewUnit_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bEditUnit, "clicked",
	                  G_CALLBACK (on_bEditUnit_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bInsertUnit, "clicked",
	                  G_CALLBACK (on_bInsertUnit_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bConvertToUnit, "clicked",
	                  G_CALLBACK (on_bConvertToUnit_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bDeleteUnit, "clicked",
	                  G_CALLBACK (on_bDeleteUnit_clicked),
	                  NULL);
	g_signal_connect ((gpointer) bCloseUnits, "clicked",
	                  G_CALLBACK (on_bCloseUnits_clicked),
	                  NULL);
	g_signal_connect ((gpointer) tbToFrom, "toggled",
	                  G_CALLBACK (on_tbToFrom_toggled),
	                  NULL);
	g_signal_connect ((gpointer) bConvertUnits, "clicked",
	                  G_CALLBACK (on_bConvertUnits_clicked),
	                  NULL);
	g_signal_connect ((gpointer) tbToTo, "toggled",
	                  G_CALLBACK (on_tbToTo_toggled),
	                  NULL);
	g_signal_connect ((gpointer) omToUnit, "changed",
	                  G_CALLBACK (on_omToUnit_changed),
	                  NULL);
	g_signal_connect ((gpointer) eFromValue, "activate",
	                  G_CALLBACK (on_eFromValue_activate),
	                  NULL);
	g_signal_connect ((gpointer) eToValue, "activate",
	                  G_CALLBACK (on_eToValue_activate),
	                  NULL);

	update_units_tree(wUnits);

	return wUnits;
}

GtkWidget*
create_wPreferences (void)
{
	GtkWidget *wPreferences;
	GtkWidget *dialog_vbox3;
	GtkWidget *vbox2_pr;
	GtkWidget *cbSaveModeOnExit;
	GtkWidget *cbSaveDefsOnExit;
	GtkWidget *cbLoadGlobalDefs;
	GtkWidget *hseparator1;
	GtkWidget *cbUseShortUnits;
	GtkWidget *dialog_action_area3;
	GtkWidget *bClose_pr;

	wPreferences	= glade_xml_get_widget (glade_xml, "preferences_dialog");
	dialog_vbox3 = GTK_DIALOG (wPreferences)->vbox;
	vbox2_pr	= glade_xml_get_widget (glade_xml, "preferences_vbox2");
	cbLoadGlobalDefs= glade_xml_get_widget (glade_xml, "preferences_checkbutton_load_defs");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbLoadGlobalDefs), load_global_defs);
	cbSaveModeOnExit= glade_xml_get_widget (glade_xml, "preferences_checkbutton_save_mode");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbSaveModeOnExit), save_mode_on_exit);
	cbSaveDefsOnExit= glade_xml_get_widget (glade_xml, "preferences_checkbutton_save_defs");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbSaveDefsOnExit), save_defs_on_exit);

	cbUseShortUnits	= glade_xml_get_widget (glade_xml, "preferences_checkbutton_short_units");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbUseShortUnits), use_short_units);

	bClose_pr	= glade_xml_get_widget (glade_xml, "preferences_button_close");

	g_signal_connect ((gpointer) cbLoadGlobalDefs, "toggled", G_CALLBACK (on_cbLoadGlobalDefs_toggled), NULL);
	g_signal_connect ((gpointer) cbSaveModeOnExit, "toggled", G_CALLBACK (on_cbSaveModeOnExit_toggled), NULL);
	g_signal_connect ((gpointer) cbSaveDefsOnExit, "toggled", G_CALLBACK (on_cbSaveDefsOnExit_toggled), NULL);
	g_signal_connect ((gpointer) cbUseShortUnits, "toggled", G_CALLBACK (on_cbUseShortUnits_toggled), NULL);

	return wPreferences;
}

GtkWidget*
create_wEditUnit (void) {
	GtkWidget *dialog_vbox4;
	GtkWidget *vbox3;
	GtkWidget *menu1;
	GtkWidget *item15;
	GtkWidget *item16;
	GtkWidget *item17;
	GtkWidget *hbox4;
	GtkWidget *dialog_action_area4;
	GtkWidget *cancelbutton1;
	GtkWidget *okbutton1;
	GtkWidget *infowidget1;
	GtkWidget *infowidget2;

	wEditUnit = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (wEditUnit), _("New unit"));
	gtk_window_set_resizable (GTK_WINDOW (wEditUnit), FALSE);

	dialog_vbox4 = GTK_DIALOG (wEditUnit)->vbox;
	gtk_widget_show (dialog_vbox4);

	vbox3 = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox3);
	gtk_box_pack_start (GTK_BOX (dialog_vbox4), vbox3, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox3), 5);

	lUnitType = gtk_label_new (_("Type"));
	gtk_widget_show (lUnitType);
	gtk_box_pack_start (GTK_BOX (vbox3), lUnitType, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lUnitType), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lUnitType), 0, 0.5);

	omUnitType = gtk_option_menu_new ();
	gtk_widget_show (omUnitType);
	gtk_box_pack_start (GTK_BOX (vbox3), omUnitType, FALSE, FALSE, 0);

	menu1 = gtk_menu_new ();

	item15 = gtk_menu_item_new_with_mnemonic (_("BASE UNIT"));
	gtk_widget_show (item15);
	gtk_container_add (GTK_CONTAINER (menu1), item15);

	item16 = gtk_menu_item_new_with_mnemonic (_("ALIAS"));
	gtk_widget_show (item16);
	gtk_container_add (GTK_CONTAINER (menu1), item16);

	item17 = gtk_menu_item_new_with_mnemonic (_("COMPOSITE UNIT"));
	gtk_widget_show (item17);
	gtk_container_add (GTK_CONTAINER (menu1), item17);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (omUnitType), menu1);

	lUnitName = gtk_label_new (_("* Name"));
	gtk_widget_show (lUnitName);
	gtk_box_pack_start (GTK_BOX (vbox3), lUnitName, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lUnitName), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lUnitName), 0, 0.5);

	eUnitName = gtk_entry_new ();
	gtk_widget_show (eUnitName);
	gtk_box_pack_start (GTK_BOX (vbox3), eUnitName, FALSE, FALSE, 0);

	lUnitPlural = gtk_label_new (_("Plural form"));
	gtk_widget_show (lUnitPlural);
	gtk_box_pack_start (GTK_BOX (vbox3), lUnitPlural, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lUnitPlural), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lUnitPlural), 0, 0.5);

	eUnitPlural = gtk_entry_new ();
	gtk_widget_show (eUnitPlural);
	gtk_box_pack_start (GTK_BOX (vbox3), eUnitPlural, FALSE, FALSE, 0);

	lShortUnitFormat = gtk_label_new (_("Short format"));
	gtk_widget_show (lShortUnitFormat);
	gtk_box_pack_start (GTK_BOX (vbox3), lShortUnitFormat, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lShortUnitFormat), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lShortUnitFormat), 0, 0.5);

	eShortUnitFormat = gtk_entry_new ();
	gtk_widget_show (eShortUnitFormat);
	gtk_box_pack_start (GTK_BOX (vbox3), eShortUnitFormat, FALSE, FALSE, 0);

	boxAlias = gtk_vbox_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (vbox3), boxAlias, TRUE, TRUE, 0);

	lBaseUnit = gtk_label_new (_("* Base unit (unit/exponent)"));
	gtk_widget_show (lBaseUnit);
	gtk_box_pack_start (GTK_BOX (boxAlias), lBaseUnit, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lBaseUnit), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lBaseUnit), 0, 0.5);

	hbox4 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox4);
	gtk_box_pack_start (GTK_BOX (boxAlias), hbox4, FALSE, FALSE, 0);

	eBaseUnit = gtk_entry_new ();
	gtk_widget_show (eBaseUnit);
	gtk_box_pack_start (GTK_BOX (hbox4), eBaseUnit, TRUE, TRUE, 0);

	sbBaseExp_adj = gtk_adjustment_new (1, -100, 100, 1, 10, 10);
	sbBaseExp = gtk_spin_button_new (GTK_ADJUSTMENT (sbBaseExp_adj), 1, 0);
	gtk_widget_show (sbBaseExp);
	gtk_box_pack_start (GTK_BOX (hbox4), sbBaseExp, TRUE, TRUE, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (sbBaseExp), TRUE);

	lRelation = gtk_label_new (_("Relation"));
	gtk_widget_show (lRelation);
	gtk_box_pack_start (GTK_BOX (boxAlias), lRelation, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lRelation), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lRelation), 0, 0.5);

	eRelation = gtk_entry_new ();
	gtk_widget_show (eRelation);
	gtk_box_pack_start (GTK_BOX (boxAlias), eRelation, FALSE, FALSE, 0);

	infowidget1 = create_InfoWidget(_("If non-fixed relation, replace value with \\x and exponent with \\y"));
	gtk_box_pack_start (GTK_BOX (boxAlias), infowidget1, FALSE, FALSE, 0);

	lReverse = gtk_label_new (_("Reversed relation"));
	gtk_widget_show (lReverse);
	gtk_box_pack_start (GTK_BOX (boxAlias), lReverse, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lReverse), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lReverse), 0, 0.5);

	eReverse = gtk_entry_new ();
	gtk_widget_show (eReverse);
	gtk_box_pack_start (GTK_BOX (boxAlias), eReverse, FALSE, FALSE, 0);

	infowidget2 = create_InfoWidget(_("Not needed for fixed relations."));
	gtk_box_pack_start (GTK_BOX (boxAlias), infowidget2, FALSE, FALSE, 0);

	lUnitCat = gtk_label_new (_("Category"));
	gtk_widget_show (lUnitCat);
	gtk_box_pack_start (GTK_BOX (vbox3), lUnitCat, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lUnitCat), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lUnitCat), 0, 0.5);

	eUnitCat = gtk_entry_new ();
	gtk_widget_show (eUnitCat);
	gtk_box_pack_start (GTK_BOX (vbox3), eUnitCat, FALSE, FALSE, 0);

	lDescrUnitName = gtk_label_new (_("Descriptive name"));
	gtk_widget_show (lDescrUnitName);
	gtk_box_pack_start (GTK_BOX (vbox3), lDescrUnitName, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lDescrUnitName), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (lDescrUnitName), 0, 0.5);

	eDescrUnitName = gtk_entry_new ();
	gtk_widget_show (eDescrUnitName);
	gtk_box_pack_start (GTK_BOX (vbox3), eDescrUnitName, FALSE, FALSE, 0);

	dialog_action_area4 = GTK_DIALOG (wEditUnit)->action_area;
	gtk_widget_show (dialog_action_area4);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area4), GTK_BUTTONBOX_END);

	cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
	gtk_widget_show (cancelbutton1);
	gtk_dialog_add_action_widget (GTK_DIALOG (wEditUnit), cancelbutton1, GTK_RESPONSE_CANCEL);
	GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

	okbutton1 = gtk_button_new_from_stock ("gtk-ok");
	gtk_widget_show (okbutton1);
	gtk_dialog_add_action_widget (GTK_DIALOG (wEditUnit), okbutton1, GTK_RESPONSE_OK);
	GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

	g_signal_connect ((gpointer) omUnitType, "changed",
	                  G_CALLBACK (on_omUnitType_changed),
	                  NULL);
	g_signal_connect ((gpointer) eUnitName, "changed",
	                  G_CALLBACK (on_unit_name_entry_changed),
	                  NULL);
	g_signal_connect ((gpointer) eUnitPlural, "changed",
	                  G_CALLBACK (on_unit_name_entry_changed),
	                  NULL);
	g_signal_connect ((gpointer) eShortUnitFormat, "changed",
	                  G_CALLBACK (on_unit_name_entry_changed),
	                  NULL);

	return wEditUnit;
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

