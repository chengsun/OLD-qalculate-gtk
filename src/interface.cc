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

#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "main.h"
#include "Calculator.h"

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


GtkWidget *window;
GtkWidget *tableT;
GtkWidget *bClose;
GtkWidget *history_scrolled;
GtkWidget *history;
GtkWidget *expression;
GtkWidget *result;
GtkWidget *bEXE;
GtkWidget *bHistory;
GtkWidget *menu_e, *menu_r, *f_menu ,*v_menu, *u_menu, *u_menu2;
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
GSList *amode_group = NULL;
GSList *dmode_group = NULL;
GSList *bmode_group = NULL;
GtkWidget *arrow1, *arrow2;
GtkAccelGroup *accel_group;
GtkWidget *sep;

extern int display_mode, number_base;
extern bool show_more, show_buttons;
extern Calculator *calc;
extern bool use_short_units, save_mode_on_exit, save_defs_on_exit, load_global_defs;


GtkWidget*
create_window (void) {
	GtkWidget *tab1label;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox1;
	GtkWidget *hbox3;
	GtkWidget *hbuttonbox2;
	GtkWidget *hbuttonbox1;
	GtkWidget *hbox2;
	GtkWidget *vbox3;
	GtkWidget *table2;
	GtkWidget *tab2label;
	GtkWidget *table1;

	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;

	accel_group = gtk_accel_group_new ();

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), _("Qalculate!"));

	menu_e = gtk_menu_new();
	gtk_widget_show (menu_e);
	sub = menu_e;

	MENU_TEAROFF

	SUBMENU_ITEM("Angle unit", menu_e)
	MENU_TEAROFF
	RADIO_MENU_ITEM_WITH_INT_1("Degrees", amode_group)
	amode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(calc->angleMode() == DEGREES)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	mDeg = item;
	RADIO_MENU_ITEM_WITH_INT_1("Radians", amode_group)
	amode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(calc->angleMode() == RADIANS)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	mRad = item;
	RADIO_MENU_ITEM_WITH_INT_1("Gradians", amode_group)
	amode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(calc->angleMode() == GRADIANS)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	mGra = item;
	RADIO_MENU_ITEM_WITH_INT_2(mDeg, set_angle_mode, DEGREES)
	RADIO_MENU_ITEM_WITH_INT_2(mRad, set_angle_mode, RADIANS)
	RADIO_MENU_ITEM_WITH_INT_2(mGra, set_angle_mode, GRADIANS)

	SUBMENU_ITEM("Signs", menu_e)
	MENU_TEAROFF
	MENU_ITEM_WITH_INT("+ (addition)", insert_sign, '+')
	MENU_ITEM_WITH_INT("- (subtraction)", insert_sign, '-')
	MENU_ITEM_WITH_INT("* (multiplication)", insert_sign, '*')
	MENU_ITEM_WITH_INT("/ (division)", insert_sign, '/')
	MENU_ITEM_WITH_INT("^ (power)", insert_sign, '^')
	MENU_ITEM_WITH_INT("E (exponent)", insert_sign, 'E')

	create_fmenu();
	create_vmenu();

	SUBMENU_ITEM("Prefixes", menu_e)
	MENU_TEAROFF
	vector<l_type::iterator> its;
	bool no_larger = false;
	l_type::iterator it1;
	for(it1 = calc->l_prefix.begin(); it1 != calc->l_prefix.end(); ++it1) {
		no_larger = true;
		for(vector<l_type::iterator>::iterator it2 = its.begin(); it2 != its.end(); ++it2) {
			if(it1->second < (*it2)->second) {
				its.insert(it2, it1);
				no_larger = false;
				break;
			}
		}
		if(no_larger)
			its.push_back(it1);
	}
	for(vector<l_type::iterator>::iterator it = its.begin(); it != its.end(); ++it) {
		gchar *gstr = g_strdup_printf("%s (10<sup>%i</sup>)", (*it)->first, (int) log10((*it)->second));
		MENU_ITEM_WITH_STRING(gstr, insert_prefix, (*it)->first)
		gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child(GTK_BIN(item))), TRUE);
		g_free(gstr);
	}
	create_umenu();
	sub = menu_e;

	//  CHECK_MENU_ITEM("Clean mode", set_clean_mode);
	MENU_ITEM("Save definitions", save_defs);
	MENU_ITEM("Save mode", save_mode);
	MENU_ITEM("Preferences", edit_preferences);

	menu_r = gtk_menu_new();
	gtk_widget_show (menu_r);
	sub = menu_r;

	MENU_TEAROFF

	SUBMENU_ITEM("Number base", menu_r)
	MENU_TEAROFF
	RADIO_MENU_ITEM_WITH_INT_1("Octal", bmode_group);
	bmode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(number_base == BASE_OCTAL)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	item2 = item;
	RADIO_MENU_ITEM_WITH_INT_1("Decimal", bmode_group);
	bmode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(number_base == BASE_DECI)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	item3 = item;
	RADIO_MENU_ITEM_WITH_INT_1("Hexadecimal", bmode_group);
	bmode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(number_base == BASE_HEX)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	RADIO_MENU_ITEM_WITH_INT_2(item, set_number_base, BASE_HEX);
	RADIO_MENU_ITEM_WITH_INT_2(item2, set_number_base, BASE_OCTAL);
	RADIO_MENU_ITEM_WITH_INT_2(item3, set_number_base, BASE_DECI);

	SUBMENU_ITEM("Display mode", menu_r)
	MENU_TEAROFF
	RADIO_MENU_ITEM_WITH_INT_1("Normal", dmode_group);
	dmode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(display_mode == MODE_NORMAL)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	item2 = item;
	RADIO_MENU_ITEM_WITH_INT_1("Scientific", dmode_group);
	dmode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(display_mode == MODE_SCIENTIFIC)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	item3 = item;
	RADIO_MENU_ITEM_WITH_INT_1("Non-scientific", dmode_group);
	dmode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(display_mode == MODE_DECIMALS)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	item4 = item;
	RADIO_MENU_ITEM_WITH_INT_1("Use prefixes", dmode_group);
	dmode_group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
	if(display_mode == MODE_PREFIXES)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
	RADIO_MENU_ITEM_WITH_INT_2(item, set_display_mode, MODE_PREFIXES);
	RADIO_MENU_ITEM_WITH_INT_2(item2, set_display_mode, MODE_NORMAL);
	RADIO_MENU_ITEM_WITH_INT_2(item3, set_display_mode, MODE_SCIENTIFIC);
	RADIO_MENU_ITEM_WITH_INT_2(item4, set_display_mode, MODE_DECIMALS);

	create_umenu2();

	sub = menu_r;
	MENU_ITEM("Store result...", add_as_variable);
	MENU_ITEM_SET_ACCEL(GDK_s);
	MENU_ITEM("Precision...", select_precision);
	MENU_ITEM("Decimals...", select_decimals);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (window), vbox1);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);
	gtk_box_set_spacing (GTK_BOX (vbox1), 5);

	tableT = gtk_table_new (2, 2, TRUE);
	gtk_widget_show (tableT);
	gtk_box_pack_start (GTK_BOX (vbox1), tableT, TRUE, TRUE, 0);
	gtk_table_set_row_spacings (GTK_TABLE (tableT), 5);
	gtk_table_set_col_spacings (GTK_TABLE (tableT), 5);
	gtk_table_set_homogeneous(GTK_TABLE(tableT), FALSE);

	bMenuE = gtk_toggle_button_new();
	arrow1 = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_IN);
	gtk_widget_show(arrow1);
	gtk_container_add(GTK_CONTAINER(bMenuE), arrow1);
	gtk_widget_show (bMenuE);
	gtk_table_attach (GTK_TABLE (tableT), bMenuE, 1, 2, 0, 1, GTK_SHRINK, GTK_FILL, 0, 0);
	bMenuR = gtk_toggle_button_new();
	arrow2 = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_IN);
	gtk_widget_show(arrow2);
	gtk_container_add(GTK_CONTAINER(bMenuR), arrow2) ;
	gtk_widget_show (bMenuR);
	gtk_table_attach (GTK_TABLE (tableT), bMenuR, 1, 2, 1, 2, GTK_SHRINK, GTK_FILL, 0, 0);

	expression = gtk_entry_new ();
	gtk_widget_show (expression);
	gtk_table_attach_defaults (GTK_TABLE (tableT), expression, 0, 1, 0, 1);

	result = gtk_label_new("<b><big>0</big></b>");
	gtk_widget_show (result);
	gtk_table_attach_defaults (GTK_TABLE (tableT), result, 0, 1, 1, 2);
	gtk_label_set_justify(GTK_LABEL(result), GTK_JUSTIFY_RIGHT);
	gtk_label_set_selectable(GTK_LABEL(result), TRUE);
	gtk_label_set_use_markup(GTK_LABEL(result), TRUE);
	gtk_misc_set_alignment (GTK_MISC (result), 0.98, 0.5);

	sep = gtk_hseparator_new();
	gtk_widget_show (sep);
	gtk_box_pack_start (GTK_BOX (vbox1), sep, FALSE, FALSE, 0);

	tabs = gtk_notebook_new ();
	if(show_more)
		gtk_widget_show(tabs);
	gtk_box_pack_start (GTK_BOX (vbox1), tabs, TRUE, TRUE, 0);

	history_scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (history_scrolled);
	gtk_container_add (GTK_CONTAINER (tabs), history_scrolled);
	gtk_container_set_border_width (GTK_CONTAINER (history_scrolled), 5);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (history_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (history_scrolled), GTK_SHADOW_IN);

	history = gtk_text_view_new ();
	gtk_widget_show (history);
	gtk_container_add (GTK_CONTAINER (history_scrolled), history);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (history), FALSE);
	gtk_text_view_set_left_margin (GTK_TEXT_VIEW (history), 5);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (history), 5);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(history)), "red_foreground", "foreground", "red", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(history)), "blue_foreground", "foreground", "blue", NULL);

	tab1label = gtk_label_new (_("History"));
	gtk_widget_show (tab1label);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (tabs), gtk_notebook_get_nth_page (GTK_NOTEBOOK (tabs), 0), tab1label);
	gtk_label_set_justify (GTK_LABEL (tab1label), GTK_JUSTIFY_LEFT);

	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox2);
	gtk_container_add (GTK_CONTAINER (tabs), hbox2);

	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_box_pack_start (GTK_BOX (hbox2), vbox3, TRUE, TRUE, 0);

	table2 = gtk_table_new (4, 3, TRUE);
	gtk_widget_show (table2);
	gtk_box_pack_start (GTK_BOX (vbox3), table2, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table2), 10);

	bSQRT = gtk_button_new_with_mnemonic (_("SQRT"));
	gtk_widget_show (bSQRT);
	gtk_table_attach (GTK_TABLE (table2), bSQRT, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bXY = gtk_button_new_with_mnemonic (_("x<sup>y</sup>"));
	gtk_widget_show (bXY);
	gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child(GTK_BIN(bXY))), TRUE);
	gtk_table_attach (GTK_TABLE (table2), bXY, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bLog = gtk_button_new_with_mnemonic (_("log"));
	gtk_widget_show (bLog);
	gtk_table_attach (GTK_TABLE (table2), bLog, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bLn = gtk_button_new_with_mnemonic (_("ln"));
	gtk_widget_show (bLn);
	gtk_table_attach (GTK_TABLE (table2), bLn, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bSin = gtk_button_new_with_mnemonic (_("sin"));
	gtk_widget_show (bSin);
	gtk_table_attach (GTK_TABLE (table2), bSin, 0, 1, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bCos = gtk_button_new_with_mnemonic (_("cos"));
	gtk_widget_show (bCos);
	gtk_table_attach (GTK_TABLE (table2), bCos, 1, 2, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bTan = gtk_button_new_with_mnemonic (_("tan"));
	gtk_widget_show (bTan);
	gtk_table_attach (GTK_TABLE (table2), bTan, 2, 3, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bSTO = gtk_button_new_with_mnemonic (_("STO"));
	gtk_widget_show (bSTO);
	gtk_table_attach (GTK_TABLE (table2), bSTO, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bLeftP = gtk_button_new_with_mnemonic (_("("));
	gtk_widget_show (bLeftP);
	gtk_table_attach (GTK_TABLE (table2), bLeftP, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bRightP = gtk_button_new_with_mnemonic (_(")"));
	gtk_widget_show (bRightP);
	gtk_table_attach (GTK_TABLE (table2), bRightP, 2, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bX2 = gtk_button_new_with_mnemonic (_("x<sup>2</sup>"));
	gtk_widget_show (bX2);
	gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child(GTK_BIN(bX2))), TRUE);
	gtk_table_attach (GTK_TABLE (table2), bX2, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bHyp = gtk_toggle_button_new_with_mnemonic (_("hyp"));
	gtk_widget_show (bHyp);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bHyp), FALSE);
	gtk_table_attach (GTK_TABLE (table2), bHyp, 2, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	hbox3 = gtk_hbox_new (TRUE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (vbox3), hbox3, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox3), 6);

	rDeg = gtk_radio_button_new_with_mnemonic (NULL, _("Deg"));
	gtk_widget_show (rDeg);
	gtk_box_pack_start (GTK_BOX (hbox3), rDeg, FALSE, FALSE, 0);

	rRad = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON(rDeg), _("Rad"));
	gtk_widget_show (rRad);
	gtk_box_pack_start (GTK_BOX (hbox3), rRad, FALSE, FALSE, 0);

	rGra = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON(rDeg), _("Gra"));
	gtk_widget_show (rGra);
	gtk_box_pack_start (GTK_BOX (hbox3), rGra, FALSE, FALSE, 0);

	if(calc->angleMode() == RADIANS)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rRad), TRUE);
	else if(calc->angleMode() == DEGREES)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rDeg), TRUE);
	else if(calc->angleMode() == GRADIANS)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rGra), TRUE);

	table1 = gtk_table_new (4, 5, TRUE);
	gtk_widget_show (table1);
	gtk_box_pack_start (GTK_BOX (hbox2), table1, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (table1), 10);
	gtk_table_set_row_spacings (GTK_TABLE (table1), 10);
	gtk_table_set_col_spacings (GTK_TABLE (table1), 5);

	b7 = gtk_button_new_with_mnemonic (_("7"));
	gtk_widget_show (b7);
	gtk_table_attach (GTK_TABLE (table1), b7, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b4 = gtk_button_new_with_mnemonic (_("4"));
	gtk_widget_show (b4);
	gtk_table_attach (GTK_TABLE (table1), b4, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b1 = gtk_button_new_with_mnemonic (_("1"));
	gtk_widget_show (b1);
	gtk_table_attach (GTK_TABLE (table1), b1, 0, 1, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b8 = gtk_button_new_with_mnemonic (_("8"));
	gtk_widget_show (b8);
	gtk_table_attach (GTK_TABLE (table1), b8, 1, 2, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b2 = gtk_button_new_with_mnemonic (_("2"));
	gtk_widget_show (b2);
	gtk_table_attach (GTK_TABLE (table1), b2, 1, 2, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b5 = gtk_button_new_with_mnemonic (_("5"));
	gtk_widget_show (b5);
	gtk_table_attach (GTK_TABLE (table1), b5, 1, 2, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b9 = gtk_button_new_with_mnemonic (_("9"));
	gtk_widget_show (b9);
	gtk_table_attach (GTK_TABLE (table1), b9, 2, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b6 = gtk_button_new_with_mnemonic (_("6"));
	gtk_widget_show (b6);
	gtk_table_attach (GTK_TABLE (table1), b6, 2, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b3 = gtk_button_new_with_mnemonic (_("3"));
	gtk_widget_show (b3);
	gtk_table_attach (GTK_TABLE (table1), b3, 2, 3, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	b0 = gtk_button_new_with_mnemonic (_("0"));
	gtk_widget_show (b0);
	gtk_table_attach (GTK_TABLE (table1), b0, 0, 1, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bDot = gtk_button_new_with_mnemonic (_("."));
	gtk_widget_show (bDot);
	gtk_table_attach (GTK_TABLE (table1), bDot, 1, 2, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bEXP = gtk_button_new_with_mnemonic (_("EXP"));
	gtk_widget_show (bEXP);
	gtk_table_attach (GTK_TABLE (table1), bEXP, 2, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bDEL = gtk_button_new_with_mnemonic (_("DEL"));
	gtk_widget_show (bDEL);
	gtk_table_attach (GTK_TABLE (table1), bDEL, 3, 4, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bAC = gtk_button_new_with_mnemonic (_("AC"));
	gtk_widget_show (bAC);
	gtk_table_attach (GTK_TABLE (table1), bAC, 4, 5, 0, 1,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bMulti = gtk_button_new_with_mnemonic (_("x"));
	gtk_widget_show (bMulti);
	gtk_table_attach (GTK_TABLE (table1), bMulti, 3, 4, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bDivi = gtk_button_new_with_mnemonic (_("/"));
	gtk_widget_show (bDivi);
	gtk_table_attach (GTK_TABLE (table1), bDivi, 4, 5, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bPlus = gtk_button_new_with_mnemonic (_("+"));
	gtk_widget_show (bPlus);
	gtk_table_attach (GTK_TABLE (table1), bPlus, 3, 4, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bMinus = gtk_button_new_with_mnemonic (_("-"));
	gtk_widget_show (bMinus);
	gtk_table_attach (GTK_TABLE (table1), bMinus, 4, 5, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bAns = gtk_button_new_with_mnemonic (_("Ans"));
	gtk_widget_show (bAns);
	gtk_table_attach (GTK_TABLE (table1), bAns, 3, 4, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	bEquals = gtk_button_new_with_mnemonic (_("="));
	gtk_widget_show (bEquals);
	gtk_table_attach (GTK_TABLE (table1), bEquals, 4, 5, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	tab2label = gtk_label_new (_("Buttons"));
	gtk_widget_show (tab2label);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (tabs), gtk_notebook_get_nth_page (GTK_NOTEBOOK (tabs), 1), tab2label);
	gtk_label_set_justify (GTK_LABEL (tab2label), GTK_JUSTIFY_LEFT);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, TRUE, 0);
	gtk_box_set_spacing (GTK_BOX (hbox1), 50);

	hbuttonbox2 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox2);
	gtk_box_pack_start (GTK_BOX (hbox1), hbuttonbox2, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox2), GTK_BUTTONBOX_START);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox2), 10);

	if(show_buttons)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), 1);

	if(show_more)
		bHistory = gtk_button_new_with_mnemonic (_("<< Less"));
	else
		bHistory = gtk_button_new_with_mnemonic (_("More >>"));
	gtk_widget_show (bHistory);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), bHistory);
	GTK_WIDGET_SET_FLAGS (bHistory, GTK_CAN_DEFAULT);

	hbuttonbox1 = gtk_hbutton_box_new ();
	gtk_widget_show (hbuttonbox1);
	gtk_box_pack_start (GTK_BOX (hbox1), hbuttonbox1, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox1), 10);

	bClose = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (bClose);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), bClose);
	GTK_WIDGET_SET_FLAGS (bClose, GTK_CAN_DEFAULT);

	bEXE = gtk_button_new_from_stock ("gtk-execute");
	gtk_widget_show (bEXE);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), bEXE);
	GTK_WIDGET_SET_FLAGS (bEXE, GTK_CAN_DEFAULT);

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


	g_signal_connect ((gpointer) window, "delete_event",
	                  G_CALLBACK (on_gcalc_exit),
	                  NULL);
	g_signal_connect ((gpointer) window, "destroy_event",
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
	g_signal_connect (G_OBJECT (menu_e), "deactivate",
	                  G_CALLBACK (on_menu_e_deactivate),
	                  NULL);
	g_signal_connect (G_OBJECT (menu_r), "deactivate",
	                  G_CALLBACK (on_menu_r_deactivate),
	                  NULL);
	g_signal_connect (G_OBJECT (expression), "changed",
	                  G_CALLBACK (on_expression_changed),
	                  NULL);

	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	gtk_widget_grab_focus(expression);
	GTK_WIDGET_SET_FLAGS(expression, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(expression);

	return window;
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

	wFunctions = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (wFunctions), _("Functions"));

	dialog_vbox1_f = GTK_DIALOG (wFunctions)->vbox;
	gtk_widget_show (dialog_vbox1_f);

	vbox1_f = gtk_vbox_new (FALSE, 5);
	gtk_widget_show (vbox1_f);
	gtk_box_pack_start (GTK_BOX (dialog_vbox1_f), vbox1_f, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox1_f), 5);

	hbox1_f = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (hbox1_f);
	gtk_box_pack_start (GTK_BOX (vbox1_f), hbox1_f, TRUE, TRUE, 0);

	scrolledwindow1_f = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (scrolledwindow1_f, 160, 1);
	gtk_widget_show (scrolledwindow1_f);
	gtk_box_pack_start (GTK_BOX (hbox1_f), scrolledwindow1_f, FALSE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1_f), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1_f), GTK_SHADOW_IN);

	tFunctionCategories = gtk_tree_view_new ();
	gtk_widget_show (tFunctionCategories);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1_f), tFunctionCategories);

	scrolledwindow2_f = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (scrolledwindow2_f);
	gtk_box_pack_start (GTK_BOX (hbox1_f), scrolledwindow2_f, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2_f), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2_f), GTK_SHADOW_IN);

	tFunctions = gtk_tree_view_new ();
	gtk_widget_show (tFunctions);
	gtk_container_add (GTK_CONTAINER (scrolledwindow2_f), tFunctions);

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

	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tFunctions), TRUE);

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

	vbuttonbox1_f = gtk_vbutton_box_new ();
	gtk_widget_show (vbuttonbox1_f);
	gtk_box_pack_start (GTK_BOX (hbox1_f), vbuttonbox1_f, FALSE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1_f), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (vbuttonbox1_f), 5);

	bNewFunction = gtk_button_new_from_stock ("gtk-new");
	gtk_widget_show (bNewFunction);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_f), bNewFunction);
	GTK_WIDGET_SET_FLAGS (bNewFunction, GTK_CAN_DEFAULT);

	bEditFunction = gtk_button_new ();
	gtk_widget_show (bEditFunction);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_f), bEditFunction);
	GTK_WIDGET_SET_FLAGS (bEditFunction, GTK_CAN_DEFAULT);

	alignment2_f = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment2_f);
	gtk_container_add (GTK_CONTAINER (bEditFunction), alignment2_f);

	hbox3_f = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox3_f);
	gtk_container_add (GTK_CONTAINER (alignment2_f), hbox3_f);

	image2_f = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image2_f);
	gtk_box_pack_start (GTK_BOX (hbox3_f), image2_f, FALSE, FALSE, 0);

	label3_f = gtk_label_new_with_mnemonic (_("_Edit"));
	gtk_widget_show (label3_f);
	gtk_box_pack_start (GTK_BOX (hbox3_f), label3_f, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label3_f), GTK_JUSTIFY_LEFT);

	bInsertFunction = gtk_button_new ();
	gtk_widget_show (bInsertFunction);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_f), bInsertFunction);
	GTK_WIDGET_SET_FLAGS (bInsertFunction, GTK_CAN_DEFAULT);

	alignment1_f = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_widget_show (alignment1_f);
	gtk_container_add (GTK_CONTAINER (bInsertFunction), alignment1_f);

	hbox2_f = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox2_f);
	gtk_container_add (GTK_CONTAINER (alignment1_f), hbox2_f);

	image1_f = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image1_f);
	gtk_box_pack_start (GTK_BOX (hbox2_f), image1_f, FALSE, FALSE, 0);

	label2_f = gtk_label_new_with_mnemonic (_("_Insert"));
	gtk_widget_show (label2_f);
	gtk_box_pack_start (GTK_BOX (hbox2_f), label2_f, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label2_f), GTK_JUSTIFY_LEFT);

	bDeleteFunction = gtk_button_new_from_stock ("gtk-delete");
	gtk_widget_show (bDeleteFunction);
	gtk_container_add (GTK_CONTAINER (vbuttonbox1_f), bDeleteFunction);
	GTK_WIDGET_SET_FLAGS (bDeleteFunction, GTK_CAN_DEFAULT);

	frame1_f = gtk_frame_new (NULL);
	gtk_widget_show (frame1_f);
	gtk_box_pack_start (GTK_BOX (vbox1_f), frame1_f, FALSE, TRUE, 0);
	gtk_widget_set_size_request (frame1_f, 10, 150);

	lFunctionDescription = gtk_label_new (_("no description"));
	gtk_widget_show (lFunctionDescription);
	gtk_container_add (GTK_CONTAINER (frame1_f), lFunctionDescription);
	gtk_label_set_justify (GTK_LABEL (lFunctionDescription), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (lFunctionDescription), TRUE);
	gtk_misc_set_alignment (GTK_MISC (lFunctionDescription), 0, 0);
	gtk_misc_set_padding (GTK_MISC (lFunctionDescription), 5, 5);

	label1_f = gtk_label_new (_("Description"));
	gtk_widget_show (label1_f);
	gtk_frame_set_label_widget (GTK_FRAME (frame1_f), label1_f);
	gtk_label_set_justify (GTK_LABEL (label1_f), GTK_JUSTIFY_LEFT);

	dialog_action_area1_f = GTK_DIALOG (wFunctions)->action_area;
	gtk_widget_show (dialog_action_area1_f);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1_f), GTK_BUTTONBOX_END);

	bCloseFunctions = gtk_button_new_from_stock ("gtk-close");
	gtk_widget_show (bCloseFunctions);
	gtk_dialog_add_action_widget (GTK_DIALOG (wFunctions), bCloseFunctions, GTK_RESPONSE_CLOSE);
	GTK_WIDGET_SET_FLAGS (bCloseFunctions, GTK_CAN_DEFAULT);

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

	GtkWidget *label1_v;
	GtkWidget *dialog_action_area1_v;
	GtkWidget *frame1_v;
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

	GtkWidget *label1_u;
	GtkWidget *dialog_action_area1_u;
	GtkWidget *frame1_u;
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
	GtkWidget *item18;
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

