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

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <sys/stat.h>

#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "main.h"

extern GtkWidget *bEditFunction;
extern GtkWidget *bDeleteFunction;
extern GtkWidget *bEditVariable;
extern GtkWidget *bDeleteVariable;
extern GtkWidget *bEditUnit;
extern GtkWidget *bConvertToUnit;
extern GtkWidget *bDeleteUnit;
extern GtkWidget *eFromValue;
extern GtkWidget *lFromUnit;
extern GtkWidget *tbToFrom;
extern GtkWidget *tbToTo;
extern GtkWidget *bConvertUnits;
extern GtkWidget *eToValue;
extern GtkWidget *omToUnit;
extern GtkWidget *lFunctionDescription;
extern GtkWidget *history_scrolled;
extern GtkWidget *tabs;
extern GtkWidget *window;
extern GtkWidget *history;
extern GtkWidget *expression;
extern GtkWidget *result;
extern GtkWidget *bEXE, *bMenuE, *bMenuR;
extern GtkWidget *bHistory;
extern GtkWidget *sep, *menu_e, *menu_r, *f_menu, *v_menu, *u_menu, *u_menu2;
extern Calculator *calc;
extern GtkWidget *bHyp;
extern Variable *vans, *vAns;
extern GtkWidget *rRad;
extern GtkWidget *rDeg;
extern GtkWidget *rGra;
extern GtkWidget *mRad;
extern GtkWidget *mDeg;
extern GtkWidget *mGra;
extern GtkWidget *tFunctions, *tFunctionCategories;
extern GtkListStore *tFunctions_store;
extern GtkListStore *tFunctionCategories_store;
extern GtkWidget *tVariables, *tVariableCategories;
extern GtkListStore *tVariables_store;
extern GtkListStore *tVariableCategories_store;
extern GtkWidget *tUnits, *tUnitCategories;
extern GtkListStore *tUnits_store;
extern GtkListStore *tUnitCategories_store;
extern GtkAccelGroup *accel_group;
GtkWidget *u_enable_item, *f_enable_item, *v_enable_item;
extern GtkWidget *functions_window;
extern string selected_function_category;
extern string selected_function;
extern GtkWidget *variables_window;
extern string selected_variable_category;
extern string selected_variable;
extern GtkWidget *units_window;
extern string selected_unit_category;
extern string selected_unit;
extern string selected_to_unit;
int saved_deci_mode, saved_decimals, saved_precision, saved_display_mode, saved_number_base, saved_angle_unit;
bool use_short_units;
bool saved_functions_enabled, saved_variables_enabled, saved_units_enabled;
bool save_mode_on_exit;
bool save_defs_on_exit;
bool hyp_is_on, saved_hyp_is_on;
int deci_mode, decimals, precision, display_mode, number_base;
bool show_more, show_buttons;
extern GtkWidget *wEditUnit;
extern GtkWidget *lUnitType;
extern GtkWidget *omUnitType;
extern GtkWidget *lUnitName;
extern GtkWidget *eUnitName;
extern GtkWidget *lUnitPlural;
extern GtkWidget *eUnitPlural;
extern GtkWidget *lShortUnitFormat;
extern GtkWidget *eShortUnitFormat;
extern GtkWidget *boxAlias;
extern GtkWidget *lBaseUnit;
extern GtkWidget *eBaseUnit;
extern GtkObject *sbBaseExp_adj;
extern GtkWidget *sbBaseExp;
extern GtkWidget *lRelation;
extern GtkWidget *eRelation;
extern GtkWidget *lReverse;
extern GtkWidget *eReverse;
extern GtkWidget *lUnitCat;
extern GtkWidget *eUnitCat;
extern GtkWidget *lDescrUnitName;
extern GtkWidget *eDescrUnitName;
extern bool load_global_defs;
extern GtkWidget *omToUnit_menu;
bool block_unit_convert;
extern Manager *mngr;

gulong get_signal_handler(GtkWidget *w) {
	return g_signal_handler_find((gpointer) w, G_SIGNAL_MATCH_UNBLOCKED, 0, 0, NULL, NULL, NULL);
}
void block_signal(GtkWidget *w, gulong s_handler) {
	g_signal_handler_block((gpointer) w, s_handler);
}
void unblock_signal(GtkWidget *w, gulong s_handler) {
	g_signal_handler_unblock((gpointer) w, s_handler);
}
gulong get_signal_handler(GObject *w) {
	return g_signal_handler_find((gpointer) w, G_SIGNAL_MATCH_UNBLOCKED, 0, 0, NULL, NULL, NULL);
}
void block_signal(GObject *w, gulong s_handler) {
	g_signal_handler_block((gpointer) w, s_handler);
}
void unblock_signal(GObject *w, gulong s_handler) {
	g_signal_handler_unblock((gpointer) w, s_handler);
}
void show_message(const gchar *text, GtkWidget *win) {
	GtkWidget *edialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, text);
	gtk_dialog_run(GTK_DIALOG(edialog));
	gtk_widget_destroy(edialog);
}
bool ask_question(const gchar *text, GtkWidget *win) {
	GtkWidget *edialog = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_YES_NO, text);
	int question_answer = gtk_dialog_run(GTK_DIALOG(edialog));
	gtk_widget_destroy(edialog);
	return question_answer == GTK_RESPONSE_YES;
}

void on_unit_name_entry_changed(GtkEditable *editable, gpointer user_data) {
	if(!calc->unitNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		gulong sh = get_signal_handler(G_OBJECT(editable));
		block_signal(G_OBJECT(editable), sh);
		gtk_entry_set_text(GTK_ENTRY(editable), calc->convertToValidUnitName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		unblock_signal(G_OBJECT(editable), sh);
	}
}
void on_function_name_entry_changed(GtkEditable *editable, gpointer user_data) {
	if(!calc->functionNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		gulong sh = get_signal_handler(G_OBJECT(editable));
		block_signal(G_OBJECT(editable), sh);
		gtk_entry_set_text(GTK_ENTRY(editable), calc->convertToValidFunctionName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		unblock_signal(G_OBJECT(editable), sh);
	}
}
void on_variable_name_entry_changed(GtkEditable *editable, gpointer user_data) {
	if(!calc->variableNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		gulong sh = get_signal_handler(G_OBJECT(editable));
		block_signal(G_OBJECT(editable), sh);
		gtk_entry_set_text(GTK_ENTRY(editable), calc->convertToValidVariableName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		unblock_signal(G_OBJECT(editable), sh);
	}
}

void display_errors() {
	if(!calc->error())
		return;
	bool critical = calc->error()->critical();
	GtkWidget *edialog;
	string str = calc->error()->message();
	while(calc->nextError()) {
		if(calc->error()->critical())
			critical = true;
		str += "\n";
		str += calc->error()->message();
	}
	if(!str.empty()) {
		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(history));
		GtkTextIter iter, iter_s;
		gtk_text_buffer_get_start_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, str.c_str(), -1);
		gtk_text_buffer_get_start_iter(tb, &iter_s);
		if(critical)
			gtk_text_buffer_apply_tag_by_name(tb, "red_foreground", &iter_s, &iter);
		else
			gtk_text_buffer_apply_tag_by_name(tb, "blue_foreground", &iter_s, &iter);
		gtk_text_buffer_insert(tb, &iter, "\n", -1);
		gtk_text_buffer_place_cursor(tb, &iter);
		if(critical)
			edialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, str.c_str());
		else
			edialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, str.c_str());
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
}

gboolean on_display_errors_timeout(gpointer data) {
	display_errors();
	return true;
}

void focus_keeping_selection() {
	gint start = 0, end = 0;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end);
	gtk_widget_grab_focus(expression);
	gtk_editable_select_region(GTK_EDITABLE(expression), start, end);
}

Function *get_selected_function() {
	for(int i = 0; i < calc->functions.size(); i++) {
		if(calc->functions[i]->name() == selected_function) {
			return calc->functions[i];
		}
	}
	return NULL;
}
Variable *get_selected_variable() {
	for(int i = 0; i < calc->variables.size(); i++) {
		if(calc->variables[i]->name() == selected_variable) {
			return calc->variables[i];
		}
	}
	return NULL;
}
Unit *get_selected_unit() {
	for(int i = 0; i < calc->units.size(); i++) {
		if(calc->units[i]->name() == selected_unit) {
			return calc->units[i];
		}
	}
	return NULL;
}
Unit *get_selected_to_unit() {
	for(int i = 0; i < calc->units.size(); i++) {
		if(calc->units[i]->name() == selected_to_unit) {
			return calc->units[i];
		}
	}
	return NULL;
}

void update_functions_tree(GtkWidget *fwin) {
	if(!fwin)
		return;
	GHashTable *hash;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tFunctionCategories_store);
	unblock_signal(G_OBJECT(select), s_handler);
	bool no_cat = false;
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	gtk_list_store_append(tFunctionCategories_store, &iter);
	gtk_list_store_set(tFunctionCategories_store, &iter, 0, "All", -1);
	for(int i = 0; i < calc->functions.size(); i++) {
		if(calc->functions[i]->category().empty()) {
			no_cat = true;
		} else {
			if(g_hash_table_lookup(hash, (gconstpointer) calc->functions[i]->category().c_str()) == NULL) {
				gtk_list_store_append(tFunctionCategories_store, &iter);
				gtk_list_store_set(tFunctionCategories_store, &iter, 0, calc->functions[i]->category().c_str(), -1);
				if(calc->functions[i]->category() == selected_function_category) {
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter);
				}
				g_hash_table_insert(hash, (gpointer) calc->functions[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	if(no_cat) {
		gtk_list_store_append(tFunctionCategories_store, &iter);
		gtk_list_store_set(tFunctionCategories_store, &iter, 0, "Uncategorized", -1);
		if(selected_function_category == "Uncategorized") {
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter);
		}
	}
	if(!gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &model, &iter)) {
		selected_function_category = "All";
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tFunctionCategories_store), &iter);
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter);
	}
	g_hash_table_destroy(hash);
}
void on_tFunctionCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	bool no_cat = false, b_all = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tFunctions_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_widget_set_sensitive(bEditFunction, FALSE);
	gtk_widget_set_sensitive(bDeleteFunction, FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 0, &gstr, -1);
		selected_function_category = gstr;
		string str;
		if(selected_function_category == "All")
			b_all = true;
		else if(selected_function_category == "Uncategorized")
			no_cat = true;
		for(int i = 0; i < calc->functions.size(); i++) {
			if(b_all || calc->functions[i]->category().empty() && no_cat || calc->functions[i]->category() == selected_function_category) {
				gtk_list_store_append(tFunctions_store, &iter2);
				str = calc->functions[i]->title();
				if(str.empty())
					str = calc->functions[i]->name();
				gtk_list_store_set(tFunctions_store, &iter2, 0, str.c_str(), 1, calc->functions[i]->name().c_str(), -1);
				if(calc->functions[i]->name() == selected_function) {
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &iter2);
				}
			}
		}
		if(selected_function.empty() || !gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &model2, &iter2)) {
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tFunctions_store), &iter2);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &iter2);
		}
		g_free(gstr);
	} else {
		selected_function_category = "";
	}
}
void on_tFunctions_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	bool no_cat = false, b_all = false;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_function = gstr;
		for(int i = 0; i < calc->functions.size(); i++) {
			if(calc->functions[i]->name() == selected_function) {
				gtk_label_set_text(GTK_LABEL(lFunctionDescription), calc->functions[i]->description().c_str());
				gtk_widget_set_sensitive(bEditFunction, TRUE);
				gtk_widget_set_sensitive(bDeleteFunction, calc->functions[i]->isUserFunction());
			}
		}
		g_free(gstr);
	} else {
		gtk_widget_set_sensitive(bEditFunction, FALSE);
		gtk_widget_set_sensitive(bDeleteFunction, FALSE);
		selected_function = "";
	}
}

void update_variables_tree(GtkWidget *fwin) {
	if(!fwin)
		return;
	GHashTable *hash;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tVariableCategories_store);
	unblock_signal(G_OBJECT(select), s_handler);
	bool no_cat = false;
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	gtk_list_store_append(tVariableCategories_store, &iter);
	gtk_list_store_set(tVariableCategories_store, &iter, 0, "All", -1);
	for(int i = 0; i < calc->variables.size(); i++) {
		if(calc->variables[i]->category().empty()) {
			no_cat = true;
		} else {
			if(g_hash_table_lookup(hash, (gconstpointer) calc->variables[i]->category().c_str()) == NULL) {
				gtk_list_store_append(tVariableCategories_store, &iter);
				gtk_list_store_set(tVariableCategories_store, &iter, 0, calc->variables[i]->category().c_str(), -1);
				if(calc->variables[i]->category() == selected_variable_category) {
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter);
				}
				g_hash_table_insert(hash, (gpointer) calc->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	if(no_cat) {
		gtk_list_store_append(tVariableCategories_store, &iter);
		gtk_list_store_set(tVariableCategories_store, &iter, 0, "Uncategorized", -1);
		if(selected_variable_category == "Uncategorized") {
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter);
		}
	}
	if(!gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &model, &iter)) {
		selected_variable_category = "All";
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tVariableCategories_store), &iter);
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter);
	}
	g_hash_table_destroy(hash);
}
void on_tVariableCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	bool no_cat = false, b_all = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tVariables_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_widget_set_sensitive(bEditVariable, FALSE);
	gtk_widget_set_sensitive(bDeleteVariable, FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 0, &gstr, -1);
		selected_variable_category = gstr;
		string str, str2;
		if(selected_variable_category == "All")
			b_all = true;
		else if(selected_variable_category == "Uncategorized")
			no_cat = true;
		for(int i = 0; i < calc->variables.size(); i++) {
			if(b_all || calc->variables[i]->category().empty() && no_cat || calc->variables[i]->category() == selected_variable_category) {
				gtk_list_store_append(tVariables_store, &iter2);
				str = calc->variables[i]->name();
				str2 = calc->variables[i]->get()->print();
				gtk_list_store_set(tVariables_store, &iter2, 0, str.c_str(), 1, str2.c_str(), -1);
				if(str == selected_variable) {
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &iter2);
				}
			}
		}
		if(selected_variable.empty() || !gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &model2, &iter2)) {
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tVariables_store), &iter2);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &iter2);
		}
		g_free(gstr);
	} else {
		selected_variable_category = "";
	}
}
void on_tVariables_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	bool no_cat = false, b_all = false;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 0, &gstr, -1);
		selected_variable = gstr;
		for(int i = 0; i < calc->variables.size(); i++) {
			if(calc->variables[i]->name() == selected_variable) {
				gtk_widget_set_sensitive(bEditVariable, TRUE);
				gtk_widget_set_sensitive(bDeleteVariable, calc->variables[i]->isUserVariable());
			}
		}
		g_free(gstr);
	} else {
		gtk_widget_set_sensitive(bEditVariable, FALSE);
		gtk_widget_set_sensitive(bDeleteVariable, FALSE);
		selected_variable = "";
	}
}
void update_units_tree(GtkWidget *fwin) {
	if(!fwin)
		return;
	GHashTable *hash;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tUnitCategories_store);
	unblock_signal(G_OBJECT(select), s_handler);
	bool no_cat = false;
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	gtk_list_store_append(tUnitCategories_store, &iter);
	gtk_list_store_set(tUnitCategories_store, &iter, 0, "All", -1);
	for(int i = 0; i < calc->units.size(); i++) {
		if(calc->units[i]->category().empty()) {
			no_cat = true;
		} else {
			if(g_hash_table_lookup(hash, (gconstpointer) calc->units[i]->category().c_str()) == NULL) {
				gtk_list_store_append(tUnitCategories_store, &iter);
				gtk_list_store_set(tUnitCategories_store, &iter, 0, calc->units[i]->category().c_str(), -1);
				if(calc->units[i]->category() == selected_unit_category) {
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter);
				}
				g_hash_table_insert(hash, (gpointer) calc->units[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	if(no_cat) {
		gtk_list_store_append(tUnitCategories_store, &iter);
		gtk_list_store_set(tUnitCategories_store, &iter, 0, "Uncategorized", -1);
		if(selected_unit_category == "Uncategorized") {
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter);
		}
	}
	if(!gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &model, &iter)) {
		selected_unit_category = "All";
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnitCategories_store), &iter);
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter);
	}
	g_hash_table_destroy(hash);
}
void on_tUnitCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	block_unit_convert = true;
	bool no_cat = false, b_all = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tUnits_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_widget_set_sensitive(bEditUnit, FALSE);
	gtk_widget_set_sensitive(bDeleteUnit, FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		AliasUnit *au;
		gtk_tree_model_get(model, &iter, 0, &gstr, -1);
		selected_unit_category = gstr;
		if(selected_unit_category == "All")
			b_all = true;
		else if(selected_unit_category == "Uncategorized")
			no_cat = true;
		string stitle, stype, snames, sbase;
		for(int i = 0; i < calc->units.size(); i++) {
			if(b_all || calc->units[i]->category().empty() && no_cat || calc->units[i]->category() == selected_unit_category) {
				gtk_list_store_append(tUnits_store, &iter2);
				snames = calc->units[i]->name();
				if(calc->units[i]->hasPlural()) {
					snames += "/";
					snames += calc->units[i]->plural();
				}
				if(calc->units[i]->hasShortName()) {
					snames += ": ";
					snames += calc->units[i]->shortName();
				}
				switch(calc->units[i]->type()) {
				case 'D': {
						stype = "COMPOSITE UNIT";
						snames = "";
						sbase = calc->units[i]->shortName();
						break;
					}
				case 'A': {
						stype = "ALIAS";
						au = (AliasUnit*) calc->units[i];
						if(use_short_units) {
							sbase = au->firstShortBaseExpName();
						} else {
							sbase = au->firstBaseExpName();
						}
						break;
					}
				case 'U': {
						stype = "BASE UNIT";
						sbase = "";
						break;
					}
				}
				if(calc->units[i]->title().empty())
					stitle = calc->units[i]->name();
				else
					stitle = calc->units[i]->title();
				gtk_list_store_set(tUnits_store, &iter2, UNITS_TITLE_COLUMN, stitle.c_str(), UNITS_NAME_COLUMN, calc->units[i]->name().c_str(), UNITS_TYPE_COLUMN, stype.c_str(), UNITS_NAMES_COLUMN, snames.c_str(), UNITS_BASE_COLUMN, sbase.c_str(), -1);
				if(calc->units[i]->name() == selected_unit) {
					gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &iter2);
				}
			}
		}
		if(selected_unit.empty() || !gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &model2, &iter2)) {
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnits_store), &iter2);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &iter2);
		}
		g_free(gstr);
	} else {
		selected_unit_category = "";
	}
	GtkWidget *tmp_w = omToUnit_menu;
	if(tmp_w)
		gtk_widget_destroy(tmp_w);		
	omToUnit_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(omToUnit), omToUnit_menu);
	GtkWidget *sub = omToUnit_menu;
	GtkWidget *item;
	int i = 0, h = -1;
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnits_store), &iter2);
	Unit *u;
	while(b) {
		gchar *gstr2;
		gtk_tree_model_get(GTK_TREE_MODEL(tUnits_store), &iter2, UNITS_NAME_COLUMN, &gstr2, -1);
		if(selected_to_unit.empty())
			selected_to_unit = gstr2;
		u = calc->getUnit(gstr2);
		if(u)
			MENU_ITEM_WITH_STRING(u->plural().c_str(), on_omToUnit_menu_activate, u->name().c_str())
			if(selected_to_unit == gstr2)
				h = i;
		g_free(gstr2);
		b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tUnits_store), &iter2);
		i++;
	}
	if(i == 0)
		selected_to_unit = "";
	else {
		if(h < 0) {
			h = 0;
			b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnits_store), &iter2);
			if(b) {
				gchar *gstr;
				gtk_tree_model_get(GTK_TREE_MODEL(tUnits_store), &iter2, UNITS_NAME_COLUMN, &gstr, -1);
				selected_to_unit = gstr;
				g_free(gstr);
			}
		}
		gtk_option_menu_set_history(GTK_OPTION_MENU(omToUnit), h);
	}
	block_unit_convert = false;
	convert_in_wUnits();
}
void on_tUnits_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	bool no_cat = false, b_all = false;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, UNITS_NAME_COLUMN, &gstr, -1);
		selected_unit = gstr;
		for(int i = 0; i < calc->units.size(); i++) {
			if(calc->units[i]->name() == selected_unit) {
				if(use_short_units)
					gtk_label_set_text(GTK_LABEL(lFromUnit), calc->units[i]->shortName().c_str());
				else
					gtk_label_set_text(GTK_LABEL(lFromUnit), calc->units[i]->plural().c_str());
				gtk_widget_set_sensitive(bEditUnit, TRUE);
				gtk_widget_set_sensitive(bDeleteUnit, TRUE);
			}
		}
		g_free(gstr);
	} else {
		gtk_widget_set_sensitive(bEditUnit, FALSE);
		gtk_widget_set_sensitive(bDeleteUnit, FALSE);
		selected_unit = "";
	}
	if(!block_unit_convert) convert_in_wUnits();
}

void create_umenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT("Units", menu_e, 6)
	u_menu = item;
	MENU_TEAROFF
	sub2 = sub;
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	for(int i = 0; i < calc->units.size(); i++) {
		if(calc->units[i]->category().empty()) {
			sub = sub2;
		} else {
			if(g_hash_table_lookup(hash, (gconstpointer) calc->units[i]->category().c_str()) == NULL) {
				SUBMENU_ITEM(calc->units[i]->category().c_str(), sub2)
				MENU_TEAROFF
				g_hash_table_insert(hash, (gpointer) calc->units[i]->category().c_str(), (gpointer) sub);
			} else {
				sub = GTK_WIDGET(g_hash_table_lookup(hash, (gconstpointer) calc->units[i]->category().c_str()));
			}
		}
		if(calc->units[i]->title().empty()) {
			MENU_ITEM_WITH_POINTER(calc->units[i]->name().c_str(), insert_unit, calc->units[i])
		} else {
			MENU_ITEM_WITH_POINTER(calc->units[i]->title().c_str(), insert_unit, calc->units[i])
		}
	}
	g_hash_table_destroy(hash);
	sub = sub2;
	MENU_ITEM("New unit...", new_unit);
	MENU_ITEM("Manage units...", manage_units);
	MENU_ITEM_SET_ACCEL(GDK_u);
	if(calc->unitsEnabled()) {
		MENU_ITEM("Disable units", set_units_enabled)
	} else {
		MENU_ITEM("Enable units", set_units_enabled)
	}
	u_enable_item = item;
}

void create_umenu2() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT("Convert to unit", menu_r, 3)
	u_menu2 = item;
	MENU_TEAROFF
	sub2 = sub;
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	for(int i = 0; i < calc->units.size(); i++) {
		if(calc->units[i]->category().empty()) {
			sub = sub2;
		} else {
			if(g_hash_table_lookup(hash, (gconstpointer) calc->units[i]->category().c_str()) == NULL) {
				SUBMENU_ITEM(calc->units[i]->category().c_str(), sub2)
				MENU_TEAROFF
				g_hash_table_insert(hash, (gpointer) calc->units[i]->category().c_str(), (gpointer) sub);
			} else {
				sub = GTK_WIDGET(g_hash_table_lookup(hash, (gconstpointer) calc->units[i]->category().c_str()));
			}
		}
		if(calc->units[i]->title().empty()) {
			MENU_ITEM_WITH_POINTER(calc->units[i]->name().c_str(), convert_to_unit, calc->units[i])
		} else {
			MENU_ITEM_WITH_POINTER(calc->units[i]->name().c_str(), convert_to_unit, calc->units[i])
		}
	}
	g_hash_table_destroy(hash);
	sub = sub2;
	MENU_ITEM("Composite unit...", convert_to_custom_unit);
	MENU_ITEM("New unit...", new_unit);
	MENU_ITEM("Manage units...", manage_units);
}

void update_umenus() {
	gtk_widget_destroy(u_menu);
	gtk_widget_destroy(u_menu2);
	create_umenu();
	create_umenu2();
	update_units_tree(units_window);
}

void create_vmenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT("Variables", menu_e, 4)
	v_menu = item;
	MENU_TEAROFF
	sub2 = sub;
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	for(int i = 0; i < calc->variables.size(); i++) {
		if(calc->variables[i]->category().empty()) {
			sub = sub2;
		} else {
			if(g_hash_table_lookup(hash, (gconstpointer) calc->variables[i]->category().c_str()) == NULL) {
				SUBMENU_ITEM(calc->variables[i]->category().c_str(), sub2)
				MENU_TEAROFF
				g_hash_table_insert(hash, (gpointer) calc->variables[i]->category().c_str(), (gpointer) sub);
			} else {
				sub = GTK_WIDGET(g_hash_table_lookup(hash, (gconstpointer) calc->variables[i]->category().c_str()));
			}
		}
		MENU_ITEM_WITH_STRING(calc->variables[i]->name().c_str(), insert_variable, calc->variables[i]->name().c_str())
	}
	g_hash_table_destroy(hash);
	sub = sub2;
	MENU_ITEM("New variable...", new_variable);
	MENU_ITEM("Manage variables...", manage_variables);
	MENU_ITEM_SET_ACCEL(GDK_v);
	if(calc->variablesEnabled()) {
		MENU_ITEM("Disable variables", set_variables_enabled)
	} else {
		MENU_ITEM("Enable variables", set_variables_enabled)
	}
	v_enable_item = item;
}
void update_vmenu() {
	gtk_widget_destroy(v_menu);
	create_vmenu();
	update_variables_tree(variables_window);
}
void create_fmenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT("Functions", menu_e, 3)
	f_menu = item;
	MENU_TEAROFF
	sub2 = sub;
	hash = g_hash_table_new(g_str_hash, g_str_equal);
	for(int i = 0; i < calc->functions.size(); i++) {
		if(calc->functions[i]->category().empty()) {
			sub = sub2;
		} else {
			if(g_hash_table_lookup(hash, (gconstpointer) calc->functions[i]->category().c_str()) == NULL) {
				SUBMENU_ITEM(calc->functions[i]->category().c_str(), sub2)
				MENU_TEAROFF
				g_hash_table_insert(hash, (gpointer) calc->functions[i]->category().c_str(), (gpointer) sub);
			} else {
				sub = GTK_WIDGET(g_hash_table_lookup(hash, (gconstpointer) calc->functions[i]->category().c_str()));
			}
		}
		if(calc->functions[i]->title().empty()) {
			MENU_ITEM_WITH_STRING(calc->functions[i]->name().c_str(), insert_function, calc->functions[i]->name().c_str())
		} else {
			MENU_ITEM_WITH_STRING(calc->functions[i]->title().c_str(), insert_function, calc->functions[i]->name().c_str())
		}
	}
	g_hash_table_destroy(hash);
	sub = sub2;
	MENU_ITEM("New function...", new_function);
	MENU_ITEM("Manage functions...", manage_functions);
	MENU_ITEM_SET_ACCEL(GDK_f);
	if(calc->functionsEnabled()) {
		MENU_ITEM("Disable functions", set_functions_enabled)
	} else {
		MENU_ITEM("Enable functions", set_functions_enabled)
	}
	f_enable_item = item;
}
void update_fmenu() {
	gtk_widget_destroy(f_menu);
	create_fmenu();
	update_functions_tree(functions_window);
}

void
on_expression_activate                 (GtkEntry        *entry,
                                        gpointer         user_data) {
	on_bEXE_clicked(GTK_BUTTON(bEXE), NULL);
}


void
on_bHistory_clicked                    (GtkButton       *button,
                                        gpointer         user_data) {
	gint w = 0, h = 0, hh = 150;
	if(GTK_WIDGET_VISIBLE(tabs)) {
		hh = tabs->allocation.height;
		gtk_widget_hide(tabs);
		gtk_widget_show(sep);
		gtk_button_set_label(button, _("More >>"));
		gtk_window_get_size(GTK_WINDOW(window), &w, &h);
		gtk_window_resize(GTK_WINDOW(window), w, h - hh);
	} else {
		gtk_widget_show(tabs);
		gtk_widget_hide(sep);
		gtk_button_set_label(button, _("<< Less"));
	}
	focus_keeping_selection();
}
string get_value_string(Manager *mngr_, bool rlabel = false) {
	int unitflags = 0;
	unitflags = unitflags | UNIT_FORMAT_BEAUTIFY;
	unitflags = unitflags | UNIT_FORMAT_NONASCII;
	if(use_short_units) unitflags = unitflags | UNIT_FORMAT_SHORT;
	if(rlabel) {
		unitflags = unitflags | UNIT_FORMAT_TAGS;
		unitflags = unitflags | UNIT_FORMAT_ALLOW_NOT_USABLE;		
	}
	NumberFormat numberformat;
	if(number_base == BASE_OCTAL) numberformat = NUMBER_FORMAT_OCTAL;
	else if(number_base == BASE_HEX) numberformat = NUMBER_FORMAT_HEX;	
	else {
		switch(display_mode) {
			case MODE_SCIENTIFIC: {numberformat = NUMBER_FORMAT_EXP_PURE; unitflags = unitflags | UNIT_FORMAT_SCIENTIFIC; break;}
			case MODE_PREFIXES: {numberformat = NUMBER_FORMAT_PREFIX; break;}
			case MODE_DECIMALS: {numberformat = NUMBER_FORMAT_DECIMALS; break;}
			default: {numberformat = NUMBER_FORMAT_NORMAL;}
		}
	}
	return mngr_->print(numberformat, unitflags, precision, decimals, true, deci_mode == DECI_FIXED);
}
void setResult(const gchar *expr) {
	GtkTextIter iter;
	GtkTextBuffer *tb;
	string str = expr, str2;
	vans->set(mngr);
	vAns->set(mngr);
	str2 = get_value_string(mngr, true);
	bool useable = false;
	gtk_label_set_selectable(GTK_LABEL(result), useable);
	gtk_label_set_text(GTK_LABEL(result), str2.c_str());
	gtk_label_set_use_markup(GTK_LABEL(result), TRUE);
	gtk_editable_select_region(GTK_EDITABLE(expression), 0, -1);
	tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(history));
	gtk_text_buffer_get_start_iter(tb, &iter);
	gtk_text_buffer_insert(tb, &iter, str.c_str(), -1);
	gtk_text_buffer_insert(tb, &iter, " = ", -1);
	gtk_text_buffer_insert(tb, &iter, get_value_string(mngr).c_str(), -1);
	gtk_text_buffer_insert(tb, &iter, "\n", -1);
	gtk_text_buffer_place_cursor(tb, &iter);
}

void execute_expression() {
	string str = gtk_entry_get_text(GTK_ENTRY(expression));
	mngr->unref();
	mngr = calc->calculate(str);
	setResult(gtk_entry_get_text(GTK_ENTRY(expression)));
	gtk_widget_grab_focus(expression);
	//	gtk_editable_set_position(GTK_EDITABLE(expression), -1);
}

void
on_bEXE_clicked                        (GtkButton       *button,
                                        gpointer         user_data) {
	execute_expression();
}

void menu_e_posfunc(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data) {
	gint root_x = 0, root_y = 0, size_x = 0, size_y = 0;
	GdkRectangle rect;
	gdk_window_get_frame_extents(window->window, &rect);
	gtk_window_get_position(GTK_WINDOW(window), &root_x, &root_y);
	gtk_window_get_size(GTK_WINDOW(window), &size_x, &size_y);
	*x = root_x + (rect.width - size_x) / 2 + bMenuE->allocation.x + bMenuE->allocation.width;
	*y = root_y + (rect.height - size_y) / 2 + bMenuE->allocation.y;
	*push_in = false;
}

void menu_r_posfunc(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data) {
	gint root_x = 0, root_y = 0, size_x = 0, size_y = 0;
	GdkRectangle rect;
	gdk_window_get_frame_extents(window->window, &rect);
	gtk_window_get_position(GTK_WINDOW(window), &root_x, &root_y);
	gtk_window_get_size(GTK_WINDOW(window), &size_x, &size_y);
	*x = root_x + (rect.width - size_x) / 2 + bMenuR->allocation.x + bMenuR->allocation.width;
	*y = root_y + (rect.height - size_y) / 2 + bMenuR->allocation.y;
	*push_in = false;
}

void
on_bMenuE_toggled                      (GtkToggleButton       *button,
                                        gpointer         user_data) {
	if(gtk_toggle_button_get_active(button)) {
		gtk_menu_popup(GTK_MENU(menu_e), NULL, NULL, menu_e_posfunc, NULL, 0, 0);
	} else {
		gtk_menu_popdown(GTK_MENU(menu_e));
	}
}

void
on_bMenuR_toggled                      (GtkToggleButton       *button,
                                        gpointer         user_data) {
	if(gtk_toggle_button_get_active(button)) {
		gtk_menu_popup(GTK_MENU(menu_r), NULL, NULL, menu_r_posfunc, NULL, 0, 0);
	} else {
		gtk_menu_popdown(GTK_MENU(menu_r));
	}
}

void
on_menu_e_deactivate                   (GtkMenuShell       *menushell,
                                        gpointer         user_data) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bMenuE), FALSE);
}

void
on_menu_r_deactivate                   (GtkMenuShell       *menushell,
                                        gpointer         user_data) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bMenuR), FALSE);
}

void on_expression_changed(GtkEditable *w, gpointer user_data) {
	gtk_label_set_text(GTK_LABEL(result), "");
}
void insert_text(const gchar *name) {
	gint position;
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {
		gtk_editable_set_position(GTK_EDITABLE(expression), start);
		gtk_editable_delete_text(GTK_EDITABLE(expression), start, end);
	}
	position = gtk_editable_get_position(GTK_EDITABLE(expression));
	gtk_editable_insert_text(GTK_EDITABLE(expression), name, strlen(name), &position);
	gtk_editable_set_position(GTK_EDITABLE(expression), position);
	gtk_widget_grab_focus(expression);
	gtk_editable_select_region(GTK_EDITABLE(expression), position, position);
}
void insert_sign(GtkMenuItem *w, gpointer user_data) {
	gint i = GPOINTER_TO_INT(user_data);
	switch(i) {
	case '+': {
			insert_text(PLUS_STR);
			break;
		}
	case '-': {
			insert_text(MINUS_STR);
			break;
		}
	case '*': {
			insert_text(MULTIPLICATION_STR);
			break;
		}
	case '/': {
			insert_text(DIVISION_STR);
			break;
		}
	case '^': {
			insert_text(POWER_STR);
			break;
		}
	case 'E': {
			insert_text(EXP_STR);
			break;
		}
	}
}
void insert_function(Function *f, GtkWidget *parent = NULL) {
	if(!f)
		return;
	gint start = 0, end = 0;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end);
	GtkWidget *dialog;
	int args = f->args();
	if(args == 0) {
		string str = f->name() + "()";
		gchar *gstr = g_strdup(str.c_str());
		insert_text(gstr);
		g_free(gstr);
		return;
	}
	dialog = gtk_dialog_new_with_buttons(f->title().c_str(), GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_EXECUTE, GTK_RESPONSE_APPLY, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	GtkWidget *vbox = gtk_vbox_new(false, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	GtkWidget *label[args];
	GtkWidget *entry[args];
	GtkWidget *descr, *entry1, *label1;
	gchar *title[args];
	gchar *title1;
	for(int i = 0; i < args; i++) {
		if(f->argName(i + 1).empty())
			title[i] = g_strdup_printf("Variable %i", i + 1);
		else
			title[i] = g_strdup(f->argName(i + 1).c_str());
		label[i] = gtk_label_new(title[i]);
		entry[i] = gtk_entry_new();
		if(i == 0) {
			gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
			gtk_entry_set_text(GTK_ENTRY(entry[i]), gstr);
			g_free(gstr);
		}
		gtk_container_add(GTK_CONTAINER(vbox), label[i]);
		gtk_container_add(GTK_CONTAINER(vbox), entry[i]);
		gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	}
	if(args < 0) {
		if(f->argName(1).empty())
			title1 = g_strdup("Vector (1,2,3,4...)");
		else
			title1 = g_strdup(f->argName(1).c_str());
		label1 = gtk_label_new(title1);
		entry1 = gtk_entry_new();
		gtk_container_add(GTK_CONTAINER(vbox), label1);
		gtk_container_add(GTK_CONTAINER(vbox), entry1);
		gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	}
	if(!f->description().empty()) {
		descr = gtk_label_new(f->description().c_str());
		gtk_label_set_line_wrap(GTK_LABEL(descr), TRUE);
		gtk_container_add(GTK_CONTAINER(vbox), descr);
		gtk_misc_set_alignment(GTK_MISC(descr), 1, 0.5);
	}
	gtk_widget_show_all(dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_ACCEPT || response == GTK_RESPONSE_APPLY) {
		string str = f->name() + "(";
		for(int i = 0; i < args; i++) {
			str += gtk_entry_get_text(GTK_ENTRY(entry[i]));
			if(i + 1 < args)
				str += ",";
		}
		if(args < 0)
			str += gtk_entry_get_text(GTK_ENTRY(entry1));
		str += ")";
		if(response == GTK_RESPONSE_ACCEPT)
			gtk_editable_select_region(GTK_EDITABLE(expression), start, end);
		else
			gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
		gchar *gstr = g_strdup(str.c_str());
		insert_text(gstr);
		g_free(gstr);
		if(response == GTK_RESPONSE_APPLY)
			execute_expression();	
	}
	for(int i = 0; i < args; i++)
		g_free(title[i]);
	if(args < 0)
		g_free(title1);
	gtk_widget_destroy(dialog);
}
void insert_function(GtkMenuItem *w, gpointer user_data) {
	gchar *name = (gchar*) user_data;
	insert_function(calc->getFunction(name), window);
}
void insert_variable(GtkMenuItem *w, gpointer user_data) {
	insert_text((gchar*) user_data);
}
void insert_prefix(GtkMenuItem *w, gpointer user_data) {
	insert_variable(w, user_data);
}
void insert_unit(GtkMenuItem *w, gpointer user_data) {
	insert_text(((Unit*) user_data)->shortName(true).c_str());
}
void on_omUnitType_changed(GtkOptionMenu *om, gpointer user_data) {
	gtk_widget_hide(boxAlias);
	gtk_widget_set_sensitive(eUnitPlural, TRUE);
	gtk_widget_set_sensitive(eShortUnitFormat, TRUE);
	gtk_widget_set_sensitive(eUnitName, TRUE);
	switch(gtk_option_menu_get_history(om)) {
	case ALIAS_UNIT: {
			gtk_widget_show(boxAlias);
			break;
		}
	case COMPOSITE_UNIT: {
			gtk_widget_set_sensitive(eUnitPlural, FALSE);
			gtk_widget_set_sensitive(eShortUnitFormat, FALSE);
			gtk_widget_set_sensitive(eUnitName, FALSE);
			break;
		}
	}
}
void edit_unit(const char *category = "", Unit *u = NULL, GtkWidget *win = NULL) {
	GtkWidget *dialog = create_wEditUnit();
	if(u)
		gtk_window_set_title(GTK_WINDOW(dialog), "Edit unit");
	else
		gtk_window_set_title(GTK_WINDOW(dialog), "New unit");
	gtk_entry_set_text(GTK_ENTRY(eUnitCat), category);
	if(u) {
		if(u->type() == 'U')
			gtk_option_menu_set_history(GTK_OPTION_MENU(omUnitType), BASE_UNIT);
		else if(u->type() == 'A')
			gtk_option_menu_set_history(GTK_OPTION_MENU(omUnitType), ALIAS_UNIT);
		else if(u->type() == 'D')
			gtk_option_menu_set_history(GTK_OPTION_MENU(omUnitType), COMPOSITE_UNIT);
		gtk_entry_set_text(GTK_ENTRY(eUnitName), u->name().c_str());
		if(u->hasPlural())
			gtk_entry_set_text(GTK_ENTRY(eUnitPlural), u->plural().c_str());
		if(u->hasShortName())
			gtk_entry_set_text(GTK_ENTRY(eShortUnitFormat), u->shortName().c_str());
		gtk_entry_set_text(GTK_ENTRY(eUnitCat), u->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(eDescrUnitName), u->title().c_str());
		switch(u->type()) {
		case 'A': {
				AliasUnit *au = (AliasUnit*) u;
				if(use_short_units)
					gtk_entry_set_text(GTK_ENTRY(eBaseUnit), au->firstShortBaseName().c_str());
				else
					gtk_entry_set_text(GTK_ENTRY(eBaseUnit), au->firstBaseName().c_str());
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(sbBaseExp), au->firstBaseExp());
				gtk_entry_set_text(GTK_ENTRY(eRelation), au->expression().c_str());
				gtk_entry_set_text(GTK_ENTRY(eReverse), au->reverseExpression().c_str());
				break;
			}
		}
	} else {
		gtk_option_menu_set_history(GTK_OPTION_MENU(omUnitType), ALIAS_UNIT);
		gtk_entry_set_text(GTK_ENTRY(eRelation), "1");
	}
run_unit_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		string str = gtk_entry_get_text(GTK_ENTRY(eUnitName));
		remove_blank_ends(str);
		if(str.empty()) {
			str = gtk_entry_get_text(GTK_ENTRY(eShortUnitFormat));
			remove_blank_ends(str);
			if(str.empty()) {
				show_message("Empty name field.", dialog);
				goto run_unit_edit_dialog;
			} else {
				gtk_entry_set_text(GTK_ENTRY(eUnitName), gtk_entry_get_text(GTK_ENTRY(eShortUnitFormat)));
				gtk_entry_set_text(GTK_ENTRY(eShortUnitFormat), "");
			}
		}
		if(calc->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(eUnitName)), u) && !ask_question("A unit with the same name already exists.\nOverwrite unit?", dialog)) {
			goto run_unit_edit_dialog;
		}
		if(calc->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(eUnitPlural)), u) && !ask_question("A unit with the same plural name already exists.\nOverwrite unit?", dialog)) {
			goto run_unit_edit_dialog;
		}
		if(calc->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(eShortUnitFormat)), u) && !ask_question("A unit with the same short format already exists.\nOverwrite unit?", dialog)) {
			goto run_unit_edit_dialog;
		}
		if(u) {
			gint i1 = gtk_option_menu_get_history(GTK_OPTION_MENU(omUnitType));
			switch(u->type()) {
			case 'A': {
					if(i1 != ALIAS_UNIT) {
						calc->delUnit(u);
						u = NULL;
						break;
					}
					AliasUnit *au = (AliasUnit*) u;
					Unit *bu = calc->getUnit(gtk_entry_get_text(GTK_ENTRY(eBaseUnit)));
					if(!bu) {
						show_message("Base unit does not exist.", dialog);
						goto run_unit_edit_dialog;
					}
					au->baseUnit(bu);
					au->expression(gtk_entry_get_text(GTK_ENTRY(eRelation)));
					au->reverseExpression(gtk_entry_get_text(GTK_ENTRY(eReverse)));
					au->exp(gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbBaseExp)));
					break;
				}
			case 'D': {
					if(i1 != COMPOSITE_UNIT) {
						calc->delUnit(u);
						u = NULL;
						break;
					}
					break;
				}
			case 'U': {
					if(i1 != BASE_UNIT) {
						calc->delUnit(u);
						u = NULL;
						break;
					}
					break;
				}
			}
			if(u) {
				u->name(gtk_entry_get_text(GTK_ENTRY(eUnitName)));
				u->plural(gtk_entry_get_text(GTK_ENTRY(eUnitPlural)));
				u->shortName(gtk_entry_get_text(GTK_ENTRY(eShortUnitFormat)));
				u->title(gtk_entry_get_text(GTK_ENTRY(eDescrUnitName)));
				u->category(gtk_entry_get_text(GTK_ENTRY(eUnitCat)));
			}
		}
		if(!u) {
			switch(gtk_option_menu_get_history(GTK_OPTION_MENU(omUnitType))) {
			case ALIAS_UNIT: {
					Unit *bu = calc->getUnit(gtk_entry_get_text(GTK_ENTRY(eBaseUnit)));
					if(!bu) {
						show_message("Base unit does not exist.", dialog);
						goto run_unit_edit_dialog;
					}
					u = new AliasUnit(calc, gtk_entry_get_text(GTK_ENTRY(eUnitCat)), gtk_entry_get_text(GTK_ENTRY(eUnitName)), gtk_entry_get_text(GTK_ENTRY(eUnitPlural)), gtk_entry_get_text(GTK_ENTRY(eShortUnitFormat)), gtk_entry_get_text(GTK_ENTRY(eDescrUnitName)), bu, gtk_entry_get_text(GTK_ENTRY(eRelation)), gtk_spin_button_get_value(GTK_SPIN_BUTTON(sbBaseExp)), gtk_entry_get_text(GTK_ENTRY(eReverse)));
					break;
				}
			case COMPOSITE_UNIT: {
					CompositeUnit *cu = new CompositeUnit(calc, gtk_entry_get_text(GTK_ENTRY(eUnitCat)), gtk_entry_get_text(GTK_ENTRY(eDescrUnitName)));
					u = cu;
					break;
				}
			default: {
					u = new Unit(calc, gtk_entry_get_text(GTK_ENTRY(eUnitCat)), gtk_entry_get_text(GTK_ENTRY(eUnitName)), gtk_entry_get_text(GTK_ENTRY(eUnitPlural)), gtk_entry_get_text(GTK_ENTRY(eShortUnitFormat)), gtk_entry_get_text(GTK_ENTRY(eDescrUnitName)));
					break;
				}
			}
			if(u) {
				u->name(gtk_entry_get_text(GTK_ENTRY(eUnitName)));
				u->plural(gtk_entry_get_text(GTK_ENTRY(eUnitPlural)));
				u->shortName(gtk_entry_get_text(GTK_ENTRY(eShortUnitFormat)));
				u->title(gtk_entry_get_text(GTK_ENTRY(eDescrUnitName)));
				u->category(gtk_entry_get_text(GTK_ENTRY(eUnitCat)));
			}
			if(u)
				calc->addUnit(u);
		}
		selected_unit = u->name();
		selected_unit_category = u->category();
		if(selected_unit_category.empty())
			selected_unit_category = "Uncategorized";
		update_umenus();
	}
	gtk_widget_destroy(dialog);
}
void edit_function(const char *category = "", Function *f = NULL, GtkWidget *win = NULL) {
	GtkWidget *dialog;
	if(f)
		dialog = gtk_dialog_new_with_buttons("Edit function", GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	else
		dialog = gtk_dialog_new_with_buttons("New function", GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	GtkWidget *lName = gtk_label_new("* Name"), *eName = gtk_entry_new();
	gtk_misc_set_alignment(GTK_MISC(lName), 0, 0.5);
	g_signal_connect((gpointer) eName, "changed", G_CALLBACK(on_function_name_entry_changed), NULL);
	GtkWidget *lEq = gtk_label_new("* Expression"), *eEq = gtk_entry_new();
	gtk_misc_set_alignment(GTK_MISC(lEq), 0, 0.5);
	GtkWidget *lEqInfo = create_InfoWidget("insert '\\x' in the place of the 1st variable, '\\y' 2nd, '\\z' 3rd, '\\a' 4th, '\\b'...");
	GtkWidget *lVars = gtk_label_new("Variables (comma separated list)"), *eVars = gtk_entry_new();
	gtk_misc_set_alignment(GTK_MISC(lVars), 0, 0.5);
	GtkWidget *lCat = gtk_label_new("Category"), *eCat = gtk_entry_new();
	gtk_misc_set_alignment(GTK_MISC(lCat), 0, 0.5);
	gtk_entry_set_text(GTK_ENTRY(eCat), category);
	GtkWidget *lDescrName = gtk_label_new("Descriptive name"), *eDescrName = gtk_entry_new();
	gtk_misc_set_alignment(GTK_MISC(lDescrName), 0, 0.5);
	GtkWidget *lDescr = gtk_label_new("Description");
	gtk_misc_set_alignment(GTK_MISC(lDescr), 0, 0.5);
	GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow), GTK_SHADOW_IN);
	GtkWidget *tvDescr = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(tvDescr), GTK_WRAP_WORD);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), tvDescr);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tvDescr));
	GtkWidget *vbox = gtk_vbox_new(false, 5);
	if(f) {
		gtk_entry_set_text(GTK_ENTRY(eName), f->name().c_str());
		if(f->isUserFunction())
			gtk_entry_set_text(GTK_ENTRY(eEq), ((UserFunction*) f)->equation().c_str());
		gtk_widget_set_sensitive(eName, f->isUserFunction());
		gtk_widget_set_sensitive(eEq, f->isUserFunction());
		gtk_entry_set_text(GTK_ENTRY(eCat), f->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(eDescrName), f->title().c_str());
		gtk_text_buffer_set_text(buffer, f->description().c_str(), -1);
		string str;
		for(int i = 1; !f->argName(i).empty(); i++) {
			if(i == 1)
				str = f->argName(i);
			else {
				str += ", ";
				str += f->argName(i);
			}
		}
		gtk_entry_set_text(GTK_ENTRY(eVars), str.c_str());
	}
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	gtk_container_add(GTK_CONTAINER(vbox), lName);
	gtk_container_add(GTK_CONTAINER(vbox), eName);
	gtk_container_add(GTK_CONTAINER(vbox), lEq);
	gtk_container_add(GTK_CONTAINER(vbox), eEq);
	gtk_container_add(GTK_CONTAINER(vbox), lEqInfo);
	gtk_container_add(GTK_CONTAINER(vbox), lVars);
	gtk_container_add(GTK_CONTAINER(vbox), eVars);
	gtk_container_add(GTK_CONTAINER(vbox), lCat);
	gtk_container_add(GTK_CONTAINER(vbox), eCat);
	gtk_container_add(GTK_CONTAINER(vbox), lDescrName);
	gtk_container_add(GTK_CONTAINER(vbox), eDescrName);
	gtk_container_add(GTK_CONTAINER(vbox), lDescr);
	gtk_box_pack_end(GTK_BOX(vbox), scrolledwindow, TRUE, TRUE, 0);
	gtk_widget_show_all(dialog);
run_function_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		string str = gtk_entry_get_text(GTK_ENTRY(eName));
		remove_blank_ends(str);
		if(str.empty()) {
			show_message("Empty name field.", dialog);
			goto run_function_edit_dialog;
		}
		str = gtk_entry_get_text(GTK_ENTRY(eEq));
		remove_blank_ends(str);
		if(str.empty()) {
			show_message("Empty expression field.", dialog);
			goto run_function_edit_dialog;
		}

		GtkTextIter iter_s, iter_e;
		gtk_text_buffer_get_start_iter(buffer, &iter_s);
		gtk_text_buffer_get_end_iter(buffer, &iter_e);
		if(calc->nameTaken(gtk_entry_get_text(GTK_ENTRY(eName)), (void*) f) && !ask_question("A function or variable with the same name already exists.\nOverwrite function/variable?", dialog)) {
			goto run_function_edit_dialog;
		}
		if(f) {
			if(f->isUserFunction())
				f->name(gtk_entry_get_text(GTK_ENTRY(eName)));
			f->category(gtk_entry_get_text(GTK_ENTRY(eCat)));
			f->title(gtk_entry_get_text(GTK_ENTRY(eDescrName)));
			f->description(gtk_text_buffer_get_text(buffer, &iter_s, &iter_e, FALSE));
			if(f->isUserFunction())
				((UserFunction*) f)->equation(gtk_entry_get_text(GTK_ENTRY(eEq)));
			str = gtk_entry_get_text(GTK_ENTRY(eVars));
			string str2;
			unsigned int i = 0, i2 = 0;
			f->clearArgNames();
			while(true) {
				i = str.find(",", i2);
				if(i != string::npos) {
					str2 = str.substr(i2, i - i2);
					i2 = i + 1;
					remove_blank_ends(str2);
					f->addArgName(str2);
				} else {
					str2 = str.substr(i2, str.length() - i2);
					remove_blank_ends(str2);
					if(!str2.empty()) {
						f->addArgName(str2);
					}
					break;
				}
			}
		} else {
			f = new UserFunction(calc, gtk_entry_get_text(GTK_ENTRY(eCat)), gtk_entry_get_text(GTK_ENTRY(eName)), gtk_entry_get_text(GTK_ENTRY(eEq)), -1, gtk_entry_get_text(GTK_ENTRY(eDescrName)), gtk_text_buffer_get_text(buffer, &iter_s, &iter_e, FALSE));
			str = gtk_entry_get_text(GTK_ENTRY(eVars));
			string str2;
			unsigned int i = 0, i2 = 0;
			while(true) {
				i = str.find(",", i2);
				if(i != string::npos) {
					str2 = str.substr(i2, i - i2);
					i2 = i + 1;
					remove_blank_ends(str2);
					f->addArgName(str2);
				} else {
					str2 = str.substr(i2, str.length() - i2);
					remove_blank_ends(str2);
					if(!str2.empty())
						f->addArgName(str2);
					break;
				}
			}
			calc->addFunction(f);
		}
		selected_function = f->name();
		selected_function_category = f->category();
		if(selected_function_category.empty())
			selected_function_category = "Uncategorized";
		update_fmenu();
	}
	gtk_widget_destroy(dialog);
}
void new_function(GtkMenuItem *w, gpointer user_data) {
	edit_function("", NULL, window);
}
void new_unit(GtkMenuItem *w, gpointer user_data) {
	edit_unit("", NULL, window);
}


void convert_to_unit(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *edialog;
	Unit *u = (Unit*) user_data;
	if(!u) {
		edialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Unit does not exist");
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
	mngr->convert(u);
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}

void convert_to_custom_unit(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Convert to custom unit", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	GtkWidget *vbox = gtk_vbox_new(false, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	GtkWidget *label1 = gtk_label_new("Units");
	GtkWidget *entry1 = gtk_entry_new();
	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
//	if(use_short_units)
//		gtk_entry_set_text(GTK_ENTRY(entry1), unit->printShort(display_mode == MODE_SCIENTIFIC, true, true).c_str());
//	else
//		gtk_entry_set_text(GTK_ENTRY(entry1), unit->print(display_mode == MODE_SCIENTIFIC, true, true).c_str());
	gtk_container_add(GTK_CONTAINER(vbox), label1);
	gtk_container_add(GTK_CONTAINER(vbox), entry1);
	gtk_widget_show_all(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
//		UnitManager u(calc);
//		long double value = curvalue / calc->calculate(gtk_entry_get_text(GTK_ENTRY(entry1)), &u);
//		value = unit->convert(&u, value);
		setResult(gtk_label_get_text(GTK_LABEL(result)));
	}
	gtk_widget_destroy(dialog);
	gtk_widget_grab_focus(expression);
}
void edit_variable(const char *category = "", Variable *v = NULL, Manager *mngr_ = NULL, GtkWidget *win = NULL) {
	GtkWidget *dialog;
	if(v)
		dialog = gtk_dialog_new_with_buttons("Edit variable", GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	else
		dialog = gtk_dialog_new_with_buttons("New variable", GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	GtkWidget *label1 = gtk_label_new("* Name");
	GtkWidget *entry1 = gtk_entry_new();
	g_signal_connect((gpointer) entry1, "changed", G_CALLBACK(on_variable_name_entry_changed), NULL);
	GtkWidget *label2 = gtk_label_new("* Value");
	GtkWidget *entry2 = gtk_entry_new();
	GtkWidget *label3 = gtk_label_new("Category");
	GtkWidget *entry3 = gtk_entry_new();
	GtkWidget *label4 = gtk_label_new("Descriptive name");
	GtkWidget *entry4 = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry3), category);
	GtkWidget *vbox = gtk_vbox_new(false, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	gtk_misc_set_alignment(GTK_MISC(label2), 0, 0.5);
	gtk_misc_set_alignment(GTK_MISC(label3), 0, 0.5);
	gtk_misc_set_alignment(GTK_MISC(label4), 0, 0.5);
	if(v) {
		gtk_entry_set_text(GTK_ENTRY(entry1), v->name().c_str());
		gtk_entry_set_text(GTK_ENTRY(entry2), get_value_string(v->get()).c_str());
		gtk_widget_set_sensitive(entry1, v->isUserVariable());
		gtk_widget_set_sensitive(entry2, v->isUserVariable());
		gtk_entry_set_text(GTK_ENTRY(entry3), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(entry4), v->title().c_str());
	} else {
		string v_name = "x";
		if(calc->nameTaken("x")) {
			if(!calc->nameTaken("y"))
				v_name = "y";
			else if(!calc->nameTaken("z"))
				v_name = "z";
			else
				v_name = calc->getName();
		}
		gtk_entry_set_text(GTK_ENTRY(entry1), v_name.c_str());
		gtk_entry_set_text(GTK_ENTRY(entry2), get_value_string(mngr).c_str());
	}
	if(mngr_) {
		gtk_widget_set_sensitive(entry2, false);	
		gtk_entry_set_text(GTK_ENTRY(entry2), get_value_string(mngr_).c_str());				
	}
	gtk_container_add(GTK_CONTAINER(vbox), label1);
	gtk_container_add(GTK_CONTAINER(vbox), entry1);
	gtk_container_add(GTK_CONTAINER(vbox), label2);
	gtk_container_add(GTK_CONTAINER(vbox), entry2);
	gtk_container_add(GTK_CONTAINER(vbox), label3);
	gtk_container_add(GTK_CONTAINER(vbox), entry3);
	gtk_container_add(GTK_CONTAINER(vbox), label4);
	gtk_container_add(GTK_CONTAINER(vbox), entry4);
	gtk_widget_show_all(dialog);
run_variable_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		string str = gtk_entry_get_text(GTK_ENTRY(entry1));
		string str2 = gtk_entry_get_text(GTK_ENTRY(entry2));
		remove_blank_ends(str);
		remove_blank_ends(str2);
		if(str.empty()) {
			show_message("Empty name field.", dialog);
			goto run_variable_edit_dialog;
		}
		if(str2.empty()) {
			show_message("Empty value field.", dialog);
			goto run_variable_edit_dialog;
		}

		if(calc->nameTaken(gtk_entry_get_text(GTK_ENTRY(entry1)), (void*) v) && !ask_question("A function or variable with the same name already exists.\nOverwrite function/variable?", dialog)) {
			goto run_variable_edit_dialog;
		}
		if(!v)
			v = calc->getVariable(str);
		if(v) {
			if(v->isUserVariable()) {
				if(mngr_) v->set(mngr_);
				else {
					mngr_ = calc->calculate(str2);
					v->set(mngr_);
					mngr_->unref();
				}
			}
			v->category(gtk_entry_get_text(GTK_ENTRY(entry3)));
			v->title(gtk_entry_get_text(GTK_ENTRY(entry4)));
		} else {
			if(mngr_) {
				v = calc->addVariable(new Variable(calc, gtk_entry_get_text(GTK_ENTRY(entry3)), str, mngr_, gtk_entry_get_text(GTK_ENTRY(entry4))));
			} else {
				mngr_ = calc->calculate(str2);
				v = calc->addVariable(new Variable(calc, gtk_entry_get_text(GTK_ENTRY(entry3)), str, mngr_, gtk_entry_get_text(GTK_ENTRY(entry4))));
				mngr_->unref();				
			}
		}
		selected_variable = v->name();
		selected_variable_category = v->category();
		if(selected_variable_category.empty())
			selected_variable_category = "Uncategorized";
		update_vmenu();
	}
	gtk_widget_destroy(dialog);
	gtk_widget_grab_focus(expression);
}
void add_as_variable(GtkMenuItem *w, gpointer user_data) {
	edit_variable("Temporary", NULL, mngr, window);
}
void new_variable(GtkMenuItem *w, gpointer user_data) {
	edit_variable("Temporary", NULL, NULL, window);
}

void set_precision(GtkSpinButton *w, gpointer user_data) {
	precision = gtk_spin_button_get_value_as_int(w);
	setResult(gtk_label_get_text(GTK_LABEL(result)));
}
void set_decimals(GtkSpinButton *w, gpointer user_data) {
	decimals = gtk_spin_button_get_value_as_int(w);
	setResult(gtk_label_get_text(GTK_LABEL(result)));
}
void on_deci_least_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		deci_mode = DECI_LEAST;
		setResult(gtk_label_get_text(GTK_LABEL(result)));
	}
}
void on_deci_fixed_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		deci_mode = DECI_FIXED;
		setResult(gtk_label_get_text(GTK_LABEL(result)));
	}
}

void select_precision(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Precision", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	GtkWidget *label1 = gtk_label_new("Precision");
	GtkWidget *spin1 = gtk_spin_button_new_with_range(1, 25, 1);
	GtkWidget *vbox = gtk_vbox_new(false, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin1), precision);
	gtk_container_add(GTK_CONTAINER(vbox), label1);
	gtk_container_add(GTK_CONTAINER(vbox), spin1);
	g_signal_connect(G_OBJECT(spin1), "value-changed", G_CALLBACK(set_precision), NULL);
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	gtk_widget_grab_focus(expression);
}
void select_decimals(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons("Decimals", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	GtkWidget *label1 = gtk_label_new("Decimals");
	GtkWidget *spin1 = gtk_spin_button_new_with_range(0, 25, 1);
	GtkWidget *radio1, *radio2;
	GtkWidget *hbox1 = gtk_hbox_new(FALSE, 5);
	GtkWidget *vbox = gtk_vbox_new(false, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	radio1 = gtk_radio_button_new_with_mnemonic (NULL, _("Least"));
	gtk_box_pack_start (GTK_BOX (hbox1), radio1, FALSE, FALSE, 0);
	radio2 = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON(radio1), _("Always"));
	gtk_box_pack_start (GTK_BOX (hbox1), radio2, FALSE, FALSE, 0);
	if(deci_mode == DECI_LEAST)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio1), TRUE);
	else if(deci_mode == DECI_FIXED)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio2), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin1), decimals);
	gtk_container_add(GTK_CONTAINER(vbox), label1);
	gtk_container_add(GTK_CONTAINER(vbox), hbox1);
	gtk_container_add(GTK_CONTAINER(vbox), spin1);
	g_signal_connect(G_OBJECT(spin1), "value-changed", G_CALLBACK(set_decimals), NULL);
	g_signal_connect(G_OBJECT(radio1), "toggled", G_CALLBACK(on_deci_least_toggled), NULL);
	g_signal_connect(G_OBJECT(radio2), "toggled", G_CALLBACK(on_deci_fixed_toggled), NULL);
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	gtk_widget_grab_focus(expression);
}
void set_display_mode(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = GPOINTER_TO_INT(user_data);
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void set_number_base(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = GPOINTER_TO_INT(user_data);
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}

void insertButtonFunction(gchar *text) {
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {
		gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
		gchar *gstr2 = g_strdup_printf("%s(%s)", text, gstr);
		insert_text(gstr2);
		g_free(gstr);
		g_free(gstr2);
	} else {
		gchar *gstr2 = g_strdup_printf("%s ", text);
		//		gchar *gstr2 = g_strdup_printf("%s()", text);
		insert_text(gstr2);
		//		gtk_editable_set_position(GTK_EDITABLE(expression), gtk_editable_get_position(GTK_EDITABLE(expression)) - 1);
		g_free(gstr2);
	}

}
void button_pressed(GtkButton *w, gpointer user_data) {
	insert_text((gchar*) user_data);
}
void button_function_pressed(GtkButton *w, gpointer user_data) {
	insertButtonFunction((gchar*) user_data);
}
void on_bDEL_clicked(GtkButton *w, gpointer user_data) {
	gint position = gtk_editable_get_position(GTK_EDITABLE(expression));
	if(g_utf8_strlen(gtk_entry_get_text(GTK_ENTRY(expression)), -1) == position)
		gtk_editable_delete_text(GTK_EDITABLE(expression), position - 1, position);
	else
		gtk_editable_delete_text(GTK_EDITABLE(expression), position, position + 1);
	gtk_widget_grab_focus(expression);
	gtk_editable_select_region(GTK_EDITABLE(expression), position, position);
}
void on_bAC_clicked(GtkButton *w, gpointer user_data) {
	gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
	gtk_widget_grab_focus(expression);
}
void on_bHyp_clicked(GtkToggleButton *w, gpointer user_data) {
	hyp_is_on = gtk_toggle_button_get_active(w);
	focus_keeping_selection();
}
void on_bTan_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("tanh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bHyp), FALSE);
	} else
		insertButtonFunction("tan");
}
void on_bSin_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("sinh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bHyp), FALSE);
	} else
		insertButtonFunction("sin");
}
void on_bCos_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("cosh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bHyp), FALSE);
	} else
		insertButtonFunction("cos");
}
void on_bSTO_clicked(GtkButton *w, gpointer user_data) {
	add_as_variable(NULL, user_data);
}
void set_angle_item() {
	GtkWidget *mi = NULL;
	switch(calc->angleMode()) {
	case RADIANS: {
			mi = mRad;
			break;
		}
	case GRADIANS: {
			mi = mGra;
			break;
		}
	case DEGREES: {
			mi = mDeg;
			break;
		}
	}
	if(!mi)
		return;
	gulong sh = get_signal_handler(mi);
	block_signal(mi, sh);
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
	unblock_signal(mi, sh);
}
void set_angle_button() {
	GtkWidget *tb = NULL;
	switch(calc->angleMode()) {
	case RADIANS: {
			tb = rRad;
			break;
		}
	case GRADIANS: {
			tb = rGra;
			break;
		}
	case DEGREES: {
			tb = rDeg;
			break;
		}
	}
	if(!tb)
		return;
	gulong sh = get_signal_handler(tb);
	block_signal(tb, sh);
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
	unblock_signal(tb, sh);
}
void set_clean_mode(GtkMenuItem *w, gpointer user_data) {
	gboolean b = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	calc->setFunctionsEnabled(!b);
	calc->setVariablesEnabled(!b);
	calc->setUnitsEnabled(!b);
}
void set_functions_enabled(GtkMenuItem *w, gpointer user_data) {
	if(calc->functionsEnabled()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(f_enable_item))), "Enable functions");
		calc->setFunctionsEnabled(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(f_enable_item))), "Disable functions");
		calc->setFunctionsEnabled(true);
	}
}
void set_variables_enabled(GtkMenuItem *w, gpointer user_data) {
	if(calc->variablesEnabled()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(v_enable_item))), "Enable variables");
		calc->setVariablesEnabled(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(v_enable_item))), "Disable variables");
		calc->setVariablesEnabled(true);
	}
}
void set_units_enabled(GtkMenuItem *w, gpointer user_data) {
	if(calc->unitsEnabled()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(u_enable_item))), "Enable units");
		calc->setUnitsEnabled(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(u_enable_item))), "Disable units");
		calc->setUnitsEnabled(true);
	}
}
void set_angle_mode(GtkMenuItem *w, gpointer user_data) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		gint i = GPOINTER_TO_INT(user_data);
		calc->angleMode(i);
		set_angle_button();
	}
}
void on_rRad_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		calc->angleMode(RADIANS);
		set_angle_item();
	}
}
void on_rDeg_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		calc->angleMode(DEGREES);
		set_angle_item();
	}
}
void on_rGra_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		calc->angleMode(GRADIANS);
		set_angle_item();
	}
}
void manage_variables(GtkMenuItem *w, gpointer user_data) {
	if(!variables_window) {
		variables_window = create_wVariables();
		gtk_window_resize(GTK_WINDOW(variables_window), 500, 400);
		gtk_widget_show(variables_window);
	} else {
		gtk_widget_show(variables_window);
		gtk_window_present(GTK_WINDOW(variables_window));
	}
}
void manage_functions(GtkMenuItem *w, gpointer user_data) {
	if(!functions_window) {
		functions_window = create_wFunctions();
		gtk_window_resize(GTK_WINDOW(functions_window), 500, 400);
		gtk_widget_show(functions_window);
	} else {
		gtk_widget_show(functions_window);
		gtk_window_present(GTK_WINDOW(functions_window));
	}
}
void manage_units(GtkMenuItem *w, gpointer user_data) {
	if(!units_window) {
		units_window = create_wUnits();
		gtk_window_resize(GTK_WINDOW(units_window), 600, 400);
		gtk_widget_show(units_window);
	} else {
		gtk_widget_show(units_window);
		gtk_window_present(GTK_WINDOW(units_window));
	}
}
void on_bNewFunction_clicked(GtkButton *button, gpointer user_data) {
	if(selected_function_category == "All" || selected_function_category == "Uncategorized")
		edit_function("", NULL, functions_window);
	else
		edit_function(selected_function_category.c_str(), NULL, functions_window);
}
void on_bEditFunction_clicked(GtkButton *button, gpointer user_data) {
	Function *f = get_selected_function();
	if(f) {
		edit_function("", f, functions_window);
	}
}
void on_bInsertFunction_clicked(GtkButton *button, gpointer user_data) {
	insert_function(get_selected_function(), functions_window);
}
void on_bDeleteFunction_clicked(GtkButton *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Function *f = get_selected_function();
	if(f && f->isUserFunction()) {
		calc->delFunction(f);
		delete f;
		if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &model, &iter)) {
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
			string str = selected_function_category;
			update_fmenu();
			if(str == selected_function_category)
				gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), path);
			g_free(path);
		} else
			update_fmenu();
	}
}
void on_bCloseFunctions_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(functions_window);
}
gboolean on_wFunctions_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(functions_window);
	return TRUE;
}
gboolean on_wFunctions_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(functions_window);
	return TRUE;
}
void on_bNewVariable_clicked(GtkButton *button, gpointer user_data) {
	if(selected_variable_category == "All" || selected_variable_category == "Uncategorized")
		edit_variable("", NULL, NULL, variables_window);
	else
		edit_variable(selected_variable_category.c_str(), NULL, NULL, variables_window);
}
void on_bEditVariable_clicked(GtkButton *button, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v) {
		edit_variable("", v, NULL, variables_window);
	}
}
void on_bInsertVariable_clicked(GtkButton *button, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v) {
		gchar *gstr = g_strdup(v->name().c_str());
		insert_text(gstr);
		g_free(gstr);
	}
}
void on_bDeleteVariable_clicked(GtkButton *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Variable *v = get_selected_variable();
	if(v && v->isUserVariable()) {
		calc->delVariable(v);
		delete v;
		if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &model, &iter)) {
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
			string str = selected_variable_category;
			update_vmenu();
			if(str == selected_variable_category)
				gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), path);
			g_free(path);
		} else
			update_vmenu();
	}
}
void on_bCloseVariables_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(variables_window);
}
gboolean on_wVariables_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(variables_window);
	return TRUE;
}
gboolean on_wVariables_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(variables_window);
	return TRUE;
}
void on_bNewUnit_clicked(GtkButton *button, gpointer user_data) {
	if(selected_unit_category == "All" || selected_unit_category == "Uncategorized")
		edit_unit("", NULL, units_window);
	else
		edit_unit(selected_unit_category.c_str(), NULL, units_window);
}
void on_bEditUnit_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		edit_unit("", u, units_window);
	}
}
void on_bInsertUnit_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		gchar *gstr;
		if(u->hasShortName() && use_short_units)
			gstr = g_strdup(u->shortName().c_str());
		else
			gstr = g_strdup(u->name().c_str());
		insert_text(gstr);
		g_free(gstr);
	}
}
void on_bConvertToUnit_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		mngr->convert(u);
		setResult(gtk_label_get_text(GTK_LABEL(result)));
		gtk_widget_grab_focus(expression);
	}
}
void on_bDeleteUnit_clicked(GtkButton *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Unit *u = get_selected_unit();
	if(u) {
		if(u->isUsedByOtherUnits()) {
			show_message("Cannot delete unit as it is needed by other units.", units_window);
			return;
		}
		calc->delUnit(u);
		delete u;
		if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &model, &iter)) {
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
			string str = selected_unit_category;
			update_umenus();
			if(str == selected_unit_category)
				gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), path);
			g_free(path);
		} else
			update_umenus();
	}
}
void convert_in_wUnits(int toFrom) {
	Unit *uFrom = get_selected_unit();
	Unit *uTo = get_selected_to_unit();
	if(uFrom && uTo) {
/*		const gchar *fromValue = gtk_entry_get_text(GTK_ENTRY(eFromValue));
		const gchar *toValue = gtk_entry_get_text(GTK_ENTRY(eToValue));
		if(toFrom > 0 || (toFrom < 0 && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tbToFrom)))) {
			gtk_entry_set_text(GTK_ENTRY(eFromValue), d2s(calc->convert(toValue, uTo, uFrom), PRECISION).c_str());
		} else {
			gtk_entry_set_text(GTK_ENTRY(eToValue), d2s(calc->convert(fromValue, uFrom, uTo), PRECISION).c_str());
		}*/
	}
}
void on_bCloseUnits_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(units_window);
}
void on_tbToFrom_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tbToTo), FALSE);
		convert_in_wUnits();
	}
}
void on_bConvertUnits_clicked(GtkButton *button, gpointer user_data) {
	convert_in_wUnits();
}
void on_tbToTo_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tbToFrom), FALSE);
		convert_in_wUnits();
	}
}
void on_omToUnit_changed(GtkOptionMenu *om, gpointer user_data) {}
void on_omToUnit_menu_activate(GtkMenuItem *item, gpointer user_data) {
	gchar *name = (gchar*) user_data;
	selected_to_unit = name;
	convert_in_wUnits();
}
void on_eFromValue_activate(GtkEntry *entry, gpointer user_data) {
	convert_in_wUnits(0);
}
void on_eToValue_activate(GtkEntry *entry, gpointer user_data) {
	convert_in_wUnits(1);
}

gboolean on_wUnits_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(units_window);
	return TRUE;
}
gboolean on_wUnits_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(units_window);
	return TRUE;
}

void on_bClose_clicked(GtkButton *w, gpointer user_data) {
	on_gcalc_exit(NULL, NULL, user_data);
}
gboolean on_gcalc_exit(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	if(save_mode_on_exit)
		save_mode();
	else
		save_preferences();
	if(save_defs_on_exit)
		save_defs();
	gtk_main_quit();
	return FALSE;
}
void save_defs() {
	gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", NULL);
	mkdir(gstr, S_IRWXU);
	g_free(gstr);
	gchar *gstr2 = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate.cfg", NULL);
	if(!calc->save(gstr2)) {
		GtkWidget *edialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Couldn't write definitions to\n%s", gstr2);
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
	g_free(gstr2);
}
void save_mode() {
	save_preferences(true);
}
void set_saved_mode() {
	saved_deci_mode = deci_mode;
	saved_decimals = decimals;
	saved_precision = precision;
	saved_display_mode = display_mode;
	saved_number_base = number_base;
	saved_angle_unit = calc->angleMode();
	saved_functions_enabled = calc->functionsEnabled();
	saved_variables_enabled = calc->variablesEnabled();
	saved_units_enabled = calc->unitsEnabled();
	saved_hyp_is_on = hyp_is_on;
}
void load_preferences() {
	deci_mode = DECI_LEAST;
	decimals = 0;
	precision = PRECISION;
	display_mode = MODE_NORMAL;
	number_base = BASE_DECI;
	save_mode_on_exit = true;
	save_defs_on_exit = false;
	hyp_is_on = false;
	show_more = false;
	show_buttons = false;
	use_short_units = true;
	load_global_defs = true;
	FILE *file = NULL;
	gchar *gstr2 = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate-gtk.cfg", NULL);
	file = fopen(gstr2, "r");
	g_free(gstr2);
	if(file) {
		char line[10000];
		string stmp, svalue, svar;
		unsigned int i;
		int v;
		while(1) {
			if(fgets(line, 10000, file) == NULL)
				break;
			stmp = line;
			remove_blanks(stmp);
			if((i = stmp.find_first_of("=")) != string::npos) {
				svar = stmp.substr(0, i);
				svalue = stmp.substr(i + 1, stmp.length() - (i + 1));
				v = s2i(svalue);
				if(svar == "save_mode_on_exit")
					save_mode_on_exit = v;
				else if(svar == "save_deinitions_on_exit")
					save_defs_on_exit = v;
				else if(svar == "load_global_definitions")
					load_global_defs = v;
				else if(svar == "show_more")
					show_more = v;
				else if(svar == "show_buttons")
					show_buttons = v;
				else if(svar == "deci_mode")
					deci_mode = v;
				else if(svar == "decimals")
					decimals = v;
				else if(svar == "precision")
					precision = v;
				else if(svar == "display_mode")
					display_mode = v;
				else if(svar == "number_base")
					number_base = v;
				else if(svar == "angle_unit")
					calc->angleMode(v);
				else if(svar == "hyp_is_on")
					hyp_is_on = v;
				else if(svar == "functions_enabled")
					calc->setFunctionsEnabled(v);
				else if(svar == "variables_enabled")
					calc->setVariablesEnabled(v);
				else if(svar == "units_enabled")
					calc->setUnitsEnabled(v);
				else if(svar == "use_short_units")
					use_short_units = v;
			}
		}
	}
	set_saved_mode();
}
void save_preferences(bool mode) {
	FILE *file = NULL;
	gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", NULL);
	mkdir(gstr, S_IRWXU);
	g_free(gstr);
	gchar *gstr2 = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate-gtk.cfg", NULL);
	file = fopen(gstr2, "w+");
	if(file == NULL) {
		GtkWidget *edialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "Couldn't write preferences to\n%s", gstr2);
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
		g_free(gstr2);
		return;
	}
	g_free(gstr2);
	fprintf(file, "\n[General]\n");
	fprintf(file, "save_mode_on_exit=%i\n", save_mode_on_exit);
	fprintf(file, "save_definitions_on_exit=%i\n", save_defs_on_exit);
	fprintf(file, "load_global_definitions=%i\n", load_global_defs);
	fprintf(file, "show_more=%i\n", GTK_WIDGET_VISIBLE(tabs));
	fprintf(file, "show_buttons=%i\n", gtk_notebook_get_current_page(GTK_NOTEBOOK(tabs)) == 1);
	fprintf(file, "use_short_units=%i\n", use_short_units);
	if(mode)
		set_saved_mode();
	fprintf(file, "\n[Mode]\n");
	fprintf(file, "deci_mode=%i\n", saved_deci_mode);
	fprintf(file, "decimals=%i\n", saved_decimals);
	fprintf(file, "precision=%i\n", saved_precision);
	fprintf(file, "display_mode=%i\n", saved_display_mode);
	fprintf(file, "number_base=%i\n", saved_number_base);
	fprintf(file, "angle_unit=%i\n", saved_angle_unit);
	fprintf(file, "hyp_is_on=%i\n", saved_hyp_is_on);
	fprintf(file, "functions_enabled=%i\n", saved_functions_enabled);
	fprintf(file, "variables_enabled=%i\n", saved_variables_enabled);
	fprintf(file, "units_enabled=%i\n", saved_units_enabled);
	fclose(file);
}
void edit_preferences() {
	GtkWidget *dialog = create_wPreferences();
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	save_preferences();
	gtk_widget_grab_focus(expression);
}
void on_cbUseShortUnits_toggled(GtkToggleButton *w, gpointer user_data) {
	use_short_units = gtk_toggle_button_get_active(w);
}
void on_cbSaveDefsOnExit_toggled(GtkToggleButton *w, gpointer user_data) {
	save_defs_on_exit = gtk_toggle_button_get_active(w);
}
void on_cbSaveModeOnExit_toggled(GtkToggleButton *w, gpointer user_data) {
	save_mode_on_exit = gtk_toggle_button_get_active(w);
}
void on_cbLoadGlobalDefs_toggled(GtkToggleButton *w, gpointer user_data) {
	load_global_defs = gtk_toggle_button_get_active(w);
}

gint string_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data) {
	gint cid = GPOINTER_TO_INT(user_data);
	gchar *gstr1, *gstr2;
	gint retval;
	gtk_tree_model_get(model, a, cid, &gstr1, -1);
	gtk_tree_model_get(model, b, cid, &gstr2, -1);
	retval = g_ascii_strcasecmp(gstr1, gstr2);
	g_free(gstr1);
	g_free(gstr2);
	return retval;
}
gint int_string_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data) {
	gint cid = GPOINTER_TO_INT(user_data);
	gchar *gstr1, *gstr2;
	bool b1 = false, b2 = false;
	gint retval;
	gtk_tree_model_get(model, a, cid, &gstr1, -1);
	gtk_tree_model_get(model, b, cid, &gstr2, -1);
	if(gstr1[0] == '-') {
		b1 = true;
	}
	if(gstr2[0] == '-') {
		b2 = true;
	}
	if(b1 == b2)
		retval = g_ascii_strcasecmp(gstr1, gstr2);
	else if(b1)
		retval = -1;
	else
		retval = 1;
	g_free(gstr1);
	g_free(gstr2);
	return retval;
}

