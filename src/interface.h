/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#define MENU_ITEM_WITH_INT(x,y,z)	item = gtk_menu_item_new_with_label(x); gtk_widget_show (item); gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC(y), GINT_TO_POINTER (z)); gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
#define MENU_ITEM_WITH_STRING(x,y,z)	item = gtk_menu_item_new_with_label(x); gtk_widget_show (item); gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC(y), (gpointer) z); gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
#define MENU_ITEM_WITH_POINTER(x,y,z)	item = gtk_menu_item_new_with_label(x); gtk_widget_show (item); gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC(y), (gpointer) z); gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
#define MENU_ITEM(x,y)			item = gtk_menu_item_new_with_label(x); gtk_widget_show (item); gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC(y), NULL); gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
#define CHECK_MENU_ITEM(x,y)		item = gtk_check_menu_item_new_with_label(x); gtk_widget_show (item); gtk_signal_connect (GTK_OBJECT (item), "activate", GTK_SIGNAL_FUNC(y), NULL); gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
#define MENU_ITEM_SET_ACCEL(a)		gtk_widget_add_accelerator(item, "activate", accel_group, a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
#define MENU_TEAROFF			item = gtk_tearoff_menu_item_new(); gtk_widget_show (item); gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
#define RADIO_MENU_ITEM_WITH_INT_1(x,w)	item = gtk_radio_menu_item_new_with_label(w, x); gtk_widget_show (item); gtk_menu_shell_append(GTK_MENU_SHELL(sub), item); 
#define RADIO_MENU_ITEM_WITH_INT_2(x,y,z)	gtk_signal_connect (GTK_OBJECT (x), "activate", GTK_SIGNAL_FUNC(y), GINT_TO_POINTER (z));
#define SUBMENU_ITEM(x,y)		item = gtk_menu_item_new_with_label(x); gtk_widget_show (item); gtk_menu_shell_append(GTK_MENU_SHELL(y), item); sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);   
#define SUBMENU_ITEM_INSERT(x,y,i)		item = gtk_menu_item_new_with_label(x); gtk_widget_show (item); gtk_menu_shell_insert(GTK_MENU_SHELL(y), item, i); sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);   

enum {
	UNITS_TITLE_COLUMN,
	UNITS_NAMES_COLUMN,
	UNITS_BASE_COLUMN,
	UNITS_TYPE_COLUMN,
	UNITS_NAME_COLUMN,
	UNITS_N_COLUMNS
};

enum {
	BASE_UNIT,
	ALIAS_UNIT,	
	COMPOSITE_UNIT
};

#ifndef INTERFACE_H
#define INTERFACE_H

GtkWidget* create_window (void);
GtkWidget* create_wFunctions (void);
GtkWidget* create_wVariables (void);
GtkWidget* create_wUnits (void);
GtkWidget* create_wPreferences (void);
GtkWidget* create_wEditUnit (void);
GtkWidget *create_InfoWidget(const gchar *text);

#endif
