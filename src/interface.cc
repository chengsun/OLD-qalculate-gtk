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
/*	gtk_radio_menu_item_set_group(
			GTK_RADIO_MENU_ITEM(mRad),
			gtk_radio_menu_item_get_group(
				GTK_RADIO_MENU_ITEM(mDeg)
			)
		);
*/	mGra = glade_xml_get_widget (glade_xml, "menu_item_gradians");
/*	gtk_radio_menu_item_set_group(
			GTK_RADIO_MENU_ITEM(mGra),
			gtk_radio_menu_item_get_group(
				GTK_RADIO_MENU_ITEM(mDeg)
			)
		);
*/
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

	/*gtk_radio_menu_item_set_group(
			GTK_RADIO_MENU_ITEM(
				glade_xml_get_widget (glade_xml, "menu_item_decimal")
				),
			gtk_radio_menu_item_get_group(
				GTK_RADIO_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_octal"))
				)
			);
	gtk_radio_menu_item_set_group(
			GTK_RADIO_MENU_ITEM(
				glade_xml_get_widget (glade_xml, "menu_item_hexadecimal")
				),
			gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_octal")) )
			);*/

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

/*	gtk_radio_menu_item_set_group (
			GTK_RADIO_MENU_ITEM (
				glade_xml_get_widget (glade_xml, "menu_item_display_scientific")
				),
			gtk_radio_menu_item_get_group (
				GTK_RADIO_MENU_ITEM (
					glade_xml_get_widget (glade_xml, "menu_item_display_normal")
					)
				)
			);
	gtk_radio_menu_item_set_group (
			GTK_RADIO_MENU_ITEM (
				glade_xml_get_widget (glade_xml, "menu_item_display_non_scientific")
				),
			gtk_radio_menu_item_get_group (
				GTK_RADIO_MENU_ITEM (
					glade_xml_get_widget (glade_xml, "menu_item_display_normal")
					)
				)
			);
	gtk_radio_menu_item_set_group (
			GTK_RADIO_MENU_ITEM (
				glade_xml_get_widget (glade_xml, "menu_item_display_prefixes")
				),
			gtk_radio_menu_item_get_group (
				GTK_RADIO_MENU_ITEM (
					glade_xml_get_widget (glade_xml, "menu_item_display_normal")
					)
				)
			);
*/
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
create_wFunctions (void) {

	GtkWidget *label1_f;
	GtkWidget *dialog_action_area1_f;
	GtkWidget *frame1_f;
	GtkWidget *vbox1_f;
	GtkWidget *hbox1_f;
	GtkWidget *scrolledwindow1_f;
	GtkWidget *scrolledwindow2_f;
	GtkWidget *vbuttonbox1_f;
	GtkWidget *alignment2_f;
	GtkWidget *hbox3_f;
	GtkWidget *image2_f;
	GtkWidget *label3_f;
	GtkWidget *alignment1_f;
	GtkWidget *hbox2_f;
	GtkWidget *image1_f;
	GtkWidget *label2_f;
	GtkWidget *dialog_vbox1_f;



	wFunctions	= glade_xml_get_widget (glade_xml, "wFunctions");
	dialog_vbox1_f	= glade_xml_get_widget (glade_xml, "dialog-vbox1");
	hbox1_f		= glade_xml_get_widget (glade_xml, "hbox1");
	scrolledwindow1_f = glade_xml_get_widget (glade_xml, "scrolledwindow1");
	tFunctionCategories = glade_xml_get_widget (glade_xml, "tFunctionCategories");
	scrolledwindow2_f = glade_xml_get_widget (glade_xml, "scrolledwindow1");
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



	vbuttonbox1_f	= glade_xml_get_widget (glade_xml, "vbuttonbox1");
	bNewFunction	= glade_xml_get_widget (glade_xml, "bNewFunction");
	bEditFunction	= glade_xml_get_widget (glade_xml, "bEditFunction");
	alignment2_f	= glade_xml_get_widget (glade_xml, "alignment2");
	hbox3_f		= glade_xml_get_widget (glade_xml, "hbox3");
	image2_f	= glade_xml_get_widget (glade_xml, "image2");
	label3_f	= glade_xml_get_widget (glade_xml, "label3");
	bInsertFunction	= glade_xml_get_widget (glade_xml, "bInsertFunction");
	alignment1_f	= glade_xml_get_widget (glade_xml, "alignment1");
	hbox2_f		= glade_xml_get_widget (glade_xml, "hbox2");
	image1_f	= glade_xml_get_widget (glade_xml, "image1");
	label2_f	= glade_xml_get_widget (glade_xml, "label2");
	bDeleteFunction	= glade_xml_get_widget (glade_xml, "bDeleteFunction");
	frame1_f	= glade_xml_get_widget (glade_xml, "frame1");
	lFunctionDescription = glade_xml_get_widget (glade_xml, "lFunctionDescription");
	label1_f	= glade_xml_get_widget (glade_xml, "label1");
	dialog_action_area1_f = GTK_DIALOG (wFunctions)->action_area;
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
create_wVariables (void) {

	GtkWidget *dialog_action_area1_v;
	GtkWidget *vbox1_v;
	GtkWidget *hbox1_v;
	GtkWidget *scrolledwindow1_v;
	GtkWidget *scrolledwindow2_v;
	GtkWidget *vbuttonbox1_v;
	GtkWidget *alignment2_v;
	GtkWidget *hbox3_v;
	GtkWidget *image2_v;
	GtkWidget *label3_v;
	GtkWidget *alignment1_v;
	GtkWidget *hbox2_v;
	GtkWidget *image1_v;
	GtkWidget *label2_v;
	GtkWidget *dialog_vbox1_v;

	wVariables = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (wVariables), _("Variables"));

	dialog_vbox1_v = GTK_DIALOG (wVariables)->vbox;
	gtk_widget_show (dialog_vbox1_v);

	vbox1_v = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox1_v);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1_v), vbox1_v, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1_v), 5);

	hbox1_v = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox1_v);
	gtk_box_pack_start (GTK_BOX (vbox1_v), hbox1_v, TRUE, TRUE, 0);

	scrolledwindow1_v = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (scrolledwindow1_v, 160, 1);
	gtk_widget_show (scrolledwindow1_v);
	gtk_box_pack_start (GTK_BOX (hbox1_v), scrolledwindow1_v, FALSE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1_v), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1_v), GTK_SHADOW_IN);

	tVariableCategories = gtk_tree_view_new ();
	gtk_widget_show (tVariableCategories);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1_v), tVariableCategories);

	scrolledwindow2_v = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2_v);
	gtk_box_pack_start (GTK_BOX (hbox1_v), scrolledwindow2_v, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2_v), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2_v), GTK_SHADOW_IN);

	tVariables = gtk_tree_view_new ();
	gtk_widget_show (tVariables);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2_v), tVariables);

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

	vbuttonbox1_v = gtk_vbutton_box_new ();
	gtk_widget_show (vbuttonbox1_v);
	gtk_box_pack_start (GTK_BOX (hbox1_v), vbuttonbox1_v, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1_v), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (vbuttonbox1_v), 5);

	bNewVariable = gtk_button_new_from_stock ("gtk-new");
	gtk_widget_show (bNewVariable);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_v), bNewVariable);
	GTK_WIDGET_SET_FLAGS (bNewVariable, GTK_CAN_DEFAULT);

	bEditVariable = gtk_button_new ();
	gtk_widget_show (bEditVariable);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_v), bEditVariable);
	GTK_WIDGET_SET_FLAGS (bEditVariable, GTK_CAN_DEFAULT);

	alignment2_v = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment2_v);
	gtk_container_add (GTK_CONTAINER (bEditVariable), alignment2_v);

	hbox3_v = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox3_v);
	gtk_container_add (GTK_CONTAINER (alignment2_v), hbox3_v);

	image2_v = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2_v);
	gtk_box_pack_start (GTK_BOX (hbox3_v), image2_v, FALSE, FALSE, 0);

	label3_v = gtk_label_new_with_mnemonic (_("_Edit"));
	gtk_widget_show (label3_v);
	gtk_box_pack_start (GTK_BOX (hbox3_v), label3_v, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label3_v), GTK_JUSTIFY_LEFT);

	bInsertVariable = gtk_button_new ();
	gtk_widget_show (bInsertVariable);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_v), bInsertVariable);
	GTK_WIDGET_SET_FLAGS (bInsertVariable, GTK_CAN_DEFAULT);

	alignment1_v = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment1_v);
	gtk_container_add (GTK_CONTAINER (bInsertVariable), alignment1_v);

	hbox2_v = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox2_v);
	gtk_container_add (GTK_CONTAINER (alignment1_v), hbox2_v);

	image1_v = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image1_v);
	gtk_box_pack_start (GTK_BOX (hbox2_v), image1_v, FALSE, FALSE, 0);

	label2_v = gtk_label_new_with_mnemonic (_("_Insert"));
	gtk_widget_show (label2_v);
	gtk_box_pack_start (GTK_BOX (hbox2_v), label2_v, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label2_v), GTK_JUSTIFY_LEFT);

	bDeleteVariable = gtk_button_new_from_stock ("gtk-delete");
	gtk_widget_show (bDeleteVariable);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_v), bDeleteVariable);
	GTK_WIDGET_SET_FLAGS (bDeleteVariable, GTK_CAN_DEFAULT);

	dialog_action_area1_v = GTK_DIALOG (wVariables)->action_area;
	gtk_widget_show (dialog_action_area1_v);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1_v), GTK_BUTTONBOX_END);

	bCloseVariables = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (bCloseVariables);
	gtk_dialog_add_action_widget (GTK_DIALOG (wVariables), bCloseVariables, GTK_RESPONSE_CLOSE);
	GTK_WIDGET_SET_FLAGS (bCloseVariables, GTK_CAN_DEFAULT);

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
create_wUnits (void) {

	GtkWidget *dialog_action_area1_u;
	GtkWidget *vbox1_u;
	GtkWidget *hbox1_u;
	GtkWidget *scrolledwindow1_u;
	GtkWidget *scrolledwindow2_u;
	GtkWidget *vbuttonbox1_u;
	GtkWidget *alignment2_u;
	GtkWidget *hbox3_u;
	GtkWidget *image2_u;
	GtkWidget *image3_u;
	GtkWidget *label3_u;
	GtkWidget *label4_u;
	GtkWidget *alignment1_u;
	GtkWidget *alignment3_u;
	GtkWidget *hbox2_u;
	GtkWidget *hbox4_u;
	GtkWidget *image1_u;
	GtkWidget *label2_u;
	GtkWidget *dialog_vbox1_u;
	GtkWidget *hbox6;
	GtkWidget *alignment4;
	GtkWidget *hbox7;
	GtkWidget *arrow1;
	GtkWidget *image4;
	GtkWidget *alignment5;
	GtkWidget *hbox8;
	GtkWidget *arrow2;
	GtkWidget *hbox9;
	GtkWidget *hbox10;

	wUnits = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (wUnits), _("Units"));

	dialog_vbox1_u = GTK_DIALOG (wUnits)->vbox;
	gtk_widget_show (dialog_vbox1_u);

	vbox1_u = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox1_u);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1_u), vbox1_u, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1_u), 5);

	hbox1_u = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox1_u);
	gtk_box_pack_start (GTK_BOX (vbox1_u), hbox1_u, TRUE, TRUE, 0);

	scrolledwindow1_u = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (scrolledwindow1_u, 160, 1);
	gtk_widget_show (scrolledwindow1_u);
	gtk_box_pack_start (GTK_BOX (hbox1_u), scrolledwindow1_u, FALSE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1_u), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1_u), GTK_SHADOW_IN);

	tUnitCategories = gtk_tree_view_new ();
	gtk_widget_show (tUnitCategories);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1_u), tUnitCategories);

	scrolledwindow2_u = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2_u);
	gtk_box_pack_start (GTK_BOX (hbox1_u), scrolledwindow2_u, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2_u), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2_u), GTK_SHADOW_IN);

	tUnits = gtk_tree_view_new ();
	gtk_widget_show (tUnits);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2_u), tUnits);

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

	hbox6 = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox6);
	gtk_box_pack_start (GTK_BOX (vbox1_u), hbox6, FALSE, FALSE, 0);
	gtk_box_set_homogeneous(GTK_BOX(hbox6), TRUE);

	hbox9 = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox9);
	gtk_box_pack_start (GTK_BOX (hbox6), hbox9, FALSE, TRUE, 0);

	hbox10 = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox10);
	gtk_box_pack_start (GTK_BOX (hbox6), hbox10, FALSE, TRUE, 0);

	eFromValue = gtk_entry_new ();
	gtk_widget_show (eFromValue);
	gtk_box_pack_start (GTK_BOX (hbox9), eFromValue, FALSE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(eFromValue), "1");

	lFromUnit = gtk_label_new (_("metres"));
	gtk_widget_show (lFromUnit);
	gtk_box_pack_start (GTK_BOX (hbox9), lFromUnit, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (lFromUnit), GTK_JUSTIFY_LEFT);

	tbToFrom = gtk_toggle_button_new ();
	gtk_widget_show (tbToFrom);
	gtk_box_pack_start (GTK_BOX (hbox9), tbToFrom, FALSE, FALSE, 0);

	alignment4 = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment4);
	gtk_container_add (GTK_CONTAINER (tbToFrom), alignment4);

	hbox7 = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox7);
	gtk_container_add (GTK_CONTAINER (alignment4), hbox7);

	arrow1 = gtk_arrow_new (GTK_ARROW_LEFT, GTK_SHADOW_OUT);
	gtk_widget_show (arrow1);
	gtk_box_pack_start (GTK_BOX (hbox7), arrow1, FALSE, FALSE, 0);

	bConvertUnits = gtk_button_new ();
	gtk_widget_show (bConvertUnits);
	gtk_box_pack_start (GTK_BOX (hbox10), bConvertUnits, FALSE, FALSE, 0);

	image4 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image4);
	gtk_container_add (GTK_CONTAINER (bConvertUnits), image4);

	tbToTo = gtk_toggle_button_new ();
	gtk_widget_show (tbToTo);
	gtk_box_pack_start (GTK_BOX (hbox10), tbToTo, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tbToTo), TRUE);

	alignment5 = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment5);
	gtk_container_add (GTK_CONTAINER (tbToTo), alignment5);

	hbox8 = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox8);
	gtk_container_add (GTK_CONTAINER (alignment5), hbox8);

	arrow2 = gtk_arrow_new (GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
	gtk_widget_show (arrow2);
	gtk_box_pack_start (GTK_BOX (hbox8), arrow2, FALSE, FALSE, 0);

	eToValue = gtk_entry_new ();
	gtk_widget_show (eToValue);
	gtk_box_pack_start (GTK_BOX (hbox10), eToValue, FALSE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(eToValue), "1");

	omToUnit = gtk_option_menu_new ();
	gtk_widget_show (omToUnit);
	gtk_box_pack_start (GTK_BOX (hbox10), omToUnit, FALSE, FALSE, 0);

	vbuttonbox1_u = gtk_vbutton_box_new ();
	gtk_widget_show (vbuttonbox1_u);
	gtk_box_pack_start (GTK_BOX (hbox1_u), vbuttonbox1_u, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1_u), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (vbuttonbox1_u), 5);

	bNewUnit = gtk_button_new_from_stock ("gtk-new");
	gtk_widget_show (bNewUnit);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_u), bNewUnit);
	GTK_WIDGET_SET_FLAGS (bNewUnit, GTK_CAN_DEFAULT);

	bEditUnit = gtk_button_new ();
	gtk_widget_show (bEditUnit);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_u), bEditUnit);
	GTK_WIDGET_SET_FLAGS (bEditUnit, GTK_CAN_DEFAULT);

	alignment2_u = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment2_u);
	gtk_container_add (GTK_CONTAINER (bEditUnit), alignment2_u);

	hbox3_u = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox3_u);
	gtk_container_add (GTK_CONTAINER (alignment2_u), hbox3_u);

	image2_u = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2_u);
	gtk_box_pack_start (GTK_BOX (hbox3_u), image2_u, FALSE, FALSE, 0);

	label3_u = gtk_label_new_with_mnemonic (_("_Edit"));
	gtk_widget_show (label3_u);
	gtk_box_pack_start (GTK_BOX (hbox3_u), label3_u, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label3_u), GTK_JUSTIFY_LEFT);

	bInsertUnit = gtk_button_new ();
	gtk_widget_show (bInsertUnit);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_u), bInsertUnit);
	GTK_WIDGET_SET_FLAGS (bInsertUnit, GTK_CAN_DEFAULT);

	alignment1_u = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment1_u);
	gtk_container_add (GTK_CONTAINER (bInsertUnit), alignment1_u);

	hbox2_u = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox2_u);
	gtk_container_add (GTK_CONTAINER (alignment1_u), hbox2_u);

	image1_u = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image1_u);
	gtk_box_pack_start (GTK_BOX (hbox2_u), image1_u, FALSE, FALSE, 0);

	label2_u = gtk_label_new_with_mnemonic (_("_Insert"));
	gtk_widget_show (label2_u);
	gtk_box_pack_start (GTK_BOX (hbox2_u), label2_u, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label2_u), GTK_JUSTIFY_LEFT);

	bConvertToUnit = gtk_button_new ();
	gtk_widget_show (bConvertToUnit);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_u), bConvertToUnit);
	GTK_WIDGET_SET_FLAGS (bConvertToUnit, GTK_CAN_DEFAULT);

	alignment3_u = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment3_u);
	gtk_container_add (GTK_CONTAINER (bConvertToUnit), alignment3_u);

	hbox4_u = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox4_u);
	gtk_container_add (GTK_CONTAINER (alignment3_u), hbox4_u);

	image3_u = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image3_u);
	gtk_box_pack_start (GTK_BOX (hbox4_u), image3_u, FALSE, FALSE, 0);

	label4_u = gtk_label_new_with_mnemonic (_("_Convert to"));
	gtk_widget_show (label4_u);
	gtk_box_pack_start (GTK_BOX (hbox4_u), label4_u, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label4_u), GTK_JUSTIFY_LEFT);

	bDeleteUnit = gtk_button_new_from_stock ("gtk-delete");
	gtk_widget_show (bDeleteUnit);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_u), bDeleteUnit);
	GTK_WIDGET_SET_FLAGS (bDeleteUnit, GTK_CAN_DEFAULT);

	dialog_action_area1_u = GTK_DIALOG (wUnits)->action_area;
	gtk_widget_show (dialog_action_area1_u);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1_u), GTK_BUTTONBOX_END);

	bCloseUnits = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (bCloseUnits);
	gtk_dialog_add_action_widget (GTK_DIALOG (wUnits), bCloseUnits, GTK_RESPONSE_CLOSE);
	GTK_WIDGET_SET_FLAGS (bCloseUnits, GTK_CAN_DEFAULT);

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
create_wPreferences (void) {
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

	wPreferences = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (wPreferences), _("Preferences"));
	gtk_window_set_resizable (GTK_WINDOW (wPreferences), FALSE);

	dialog_vbox3 = GTK_DIALOG (wPreferences)->vbox;
	gtk_widget_show (dialog_vbox3);

	vbox2_pr = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox2_pr);
	gtk_box_pack_start (GTK_BOX (dialog_vbox3), vbox2_pr, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2_pr), 5);

	cbLoadGlobalDefs = gtk_check_button_new_with_mnemonic (_("Load global definitions on start"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbLoadGlobalDefs), load_global_defs);
	gtk_widget_show (cbLoadGlobalDefs);
	gtk_box_pack_start (GTK_BOX (vbox2_pr), cbLoadGlobalDefs, FALSE, FALSE, 0);

	cbSaveModeOnExit = gtk_check_button_new_with_mnemonic (_("Save mode on exit"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbSaveModeOnExit), save_mode_on_exit);
	gtk_widget_show (cbSaveModeOnExit);
	gtk_box_pack_start (GTK_BOX (vbox2_pr), cbSaveModeOnExit, FALSE, FALSE, 0);

	cbSaveDefsOnExit = gtk_check_button_new_with_mnemonic (_("Save definitions on exit"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbSaveDefsOnExit), save_defs_on_exit);
	gtk_widget_show (cbSaveDefsOnExit);
	gtk_box_pack_start (GTK_BOX (vbox2_pr), cbSaveDefsOnExit, FALSE, FALSE, 0);

	hseparator1 = gtk_hseparator_new ();
	gtk_widget_show (hseparator1);
	gtk_box_pack_start (GTK_BOX (vbox2_pr), hseparator1, TRUE, TRUE, 0);

	cbUseShortUnits = gtk_check_button_new_with_mnemonic (_("Short unit format"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbUseShortUnits), use_short_units);
	gtk_widget_show (cbUseShortUnits);
	gtk_box_pack_start (GTK_BOX (vbox2_pr), cbUseShortUnits, FALSE, FALSE, 0);

	dialog_action_area3 = GTK_DIALOG (wPreferences)->action_area;
	gtk_widget_show (dialog_action_area3);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area3), GTK_BUTTONBOX_END);

	bClose_pr = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (bClose_pr);
	gtk_dialog_add_action_widget (GTK_DIALOG (wPreferences), bClose_pr, GTK_RESPONSE_CLOSE);
	GTK_WIDGET_SET_FLAGS (bClose_pr, GTK_CAN_DEFAULT);

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

