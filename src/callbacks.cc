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
#include <glade/glade.h>

#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "main.h"

bool changing_in_nbases_dialog;

extern GladeXML *glade_xml;

extern GtkWidget *expression;
extern GtkWidget *result;
extern GtkWidget *label_equals;
extern GtkWidget *f_menu, *v_menu, *u_menu, *u_menu2;
extern Variable *vans, *vAns;
extern GtkWidget *tFunctions, *tFunctionCategories;
extern GtkListStore *tFunctions_store;
extern GtkTreeStore *tFunctionCategories_store;
extern GtkWidget *tVariables, *tVariableCategories;
extern GtkListStore *tVariables_store;
extern GtkTreeStore *tVariableCategories_store;
extern GtkWidget *tUnits, *tUnitCategories;
extern GtkListStore *tUnits_store;
extern GtkTreeStore *tUnitCategories_store;
extern GtkAccelGroup *accel_group;
GtkWidget *u_enable_item, *f_enable_item, *v_enable_item, *uv_enable_item, *v_calcvar_item;
extern GtkWidget *functions_window;
extern string selected_function_category;
extern string selected_function;
extern GtkWidget *variables_window;
extern string selected_variable_category;
extern string selected_variable;
extern GtkWidget *units_window;
extern string selected_unit_category;
extern Unit *selected_unit;
extern Unit *selected_to_unit;
int saved_deci_mode, saved_decimals, saved_precision, saved_display_mode, saved_number_base, saved_angle_unit;
bool use_short_units, use_unicode_signs;
bool saved_functions_enabled, saved_variables_enabled, saved_unknownvariables_enabled, saved_units_enabled, saved_donot_calcvars;
bool save_mode_on_exit;
bool save_defs_on_exit;
bool fraction_is_on, saved_fraction_is_on;
bool hyp_is_on, saved_hyp_is_on;
int deci_mode, decimals, display_mode, number_base;
bool show_more, show_buttons;
extern bool load_global_defs;
extern GtkWidget *omToUnit_menu;
bool block_unit_convert;
extern Manager *mngr;

struct tree_struct {
	string item;
	list<tree_struct> items;
	vector<void*> objects;	
	bool operator < (tree_struct &s1) const {
		return item < s1.item;	
	}	
};
list<tree_struct> unit_cats, variable_cats, function_cats;
vector<void*> uc_units, uc_variables, uc_functions;	

void wrap_expression_selection() {
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {			
		gtk_editable_select_region(GTK_EDITABLE(expression), end, end);
		gtk_editable_insert_text(GTK_EDITABLE(expression), LEFT_BRACKET_STR, 1, &start);
		end++;
		gtk_editable_insert_text(GTK_EDITABLE(expression), RIGHT_BRACKET_STR, 1, &end);				
		gtk_editable_set_position(GTK_EDITABLE(expression), end);				
	}
}

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


/*
	display errors generated under calculation
*/
void display_errors() {
	if(!CALCULATOR->error())
		return;
	bool critical = false; bool b = false;
	GtkWidget *edialog;
	string str = "";
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "history")));
	GtkTextIter iter, iter_s;
	while(true) {
		if(CALCULATOR->error()->critical())
			critical = true;
		if(b)
			str += "\n";
		else
			b = true;
		str += CALCULATOR->error()->message();
		gtk_text_buffer_get_start_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, CALCULATOR->error()->message().c_str(), -1);
		gtk_text_buffer_get_start_iter(tb, &iter_s);
		if(CALCULATOR->error()->critical())
			gtk_text_buffer_apply_tag_by_name(tb, "red_foreground", &iter_s, &iter);
		else
			gtk_text_buffer_apply_tag_by_name(tb, "blue_foreground", &iter_s, &iter);
		gtk_text_buffer_insert(tb, &iter, "\n", -1);
		gtk_text_buffer_place_cursor(tb, &iter);
		if(!CALCULATOR->nextError()) break;
	}
	if(!str.empty())
	{
		if(critical)
		{
			edialog = gtk_message_dialog_new(
					GTK_WINDOW(
						glade_xml_get_widget (glade_xml, "main_window")
					),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					str.c_str());
		}
		else
		{
			edialog = gtk_message_dialog_new(
					GTK_WINDOW(
						glade_xml_get_widget (glade_xml, "main_window")
					),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					str.c_str());
		}

		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
}

gboolean on_display_errors_timeout(gpointer data) {
	display_errors();
	return true;
}

/*
	set focus on expression entry without losing selection
*/
void focus_keeping_selection() {
	gint start = 0, end = 0;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end);
	gtk_widget_grab_focus(expression);
	gtk_editable_select_region(GTK_EDITABLE(expression), start, end);
}

/*
	return the Function object that corresponds to the selected function name
*/
Function *get_selected_function() {
	for(int i = 0; i < CALCULATOR->functions.size(); i++) {
		if(CALCULATOR->functions[i]->name() == selected_function) {
			return CALCULATOR->functions[i];
		}
	}
	return NULL;
}

/*
	return the Variable object that corresponds to the selected variable name
*/
Variable *get_selected_variable() {
	for(int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(CALCULATOR->variables[i]->name() == selected_variable) {
			return CALCULATOR->variables[i];
		}
	}
	return NULL;
}

/*
	return the Unit object that corresponds to the selected unit name (expression menu)
*/
Unit *get_selected_unit() {
/*	for(int i = 0; i < CALCULATOR->units.size(); i++) {
		if(CALCULATOR->units[i]->name() == selected_unit) {
			return CALCULATOR->units[i];
		}
	}
	return NULL;*/
	return selected_unit;
}

/*
	return the Unit object that corresponds to the selected unit name (result menu)
*/
Unit *get_selected_to_unit() {
/*	for(int i = 0; i < CALCULATOR->units.size(); i++) {
		if(CALCULATOR->units[i]->name() == selected_to_unit) {
			return CALCULATOR->units[i];
		}
	}
	return NULL;*/
	return selected_to_unit;
}

void generate_units_tree_struct() {
	tree_struct *p_cat;
	int i2; bool b;
	bool no_cat = false;	
	string str;
	unit_cats.clear();
	uc_units.clear();
	Unit *u;
	list<tree_struct>::iterator it;	
	for(int i = 0; i < CALCULATOR->units.size(); i++) {
		if(CALCULATOR->units[i]->category().empty()) {
			b = false;
			for(int i3 = 0; i3 < uc_units.size(); i3++) {
				u = (Unit*) uc_units[i3];
				if(CALCULATOR->units[i]->title() < u->title()) {
					b = true;
					uc_units.insert(uc_units.begin() + i3, (void*) CALCULATOR->units[i]);
					break;
				}
			}
			if(!b) uc_units.push_back((void*) CALCULATOR->units[i]);			
		} else {
			b = false;
			//add category if not present
			if((i2 = CALCULATOR->units[i]->category().find("/")) == string::npos) { 	
				for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
					if(it->item == CALCULATOR->units[i]->category()) {
						for(int i3 = 0; i3 < it->objects.size(); i3++) {
							u = (Unit*) it->objects[i3];
							if(CALCULATOR->units[i]->title() < u->title()) {
								b = true;
								it->objects.insert(it->objects.begin() + i3, (void*) CALCULATOR->units[i]);
								break;
							}
						}
						if(!b) it->objects.push_back((void*) CALCULATOR->units[i]);					
						b = true;
					}
				}
				if(!b) {
					tree_struct cat;		
					cat.item = CALCULATOR->units[i]->category();
					cat.objects.push_back((void*) CALCULATOR->units[i]);
					unit_cats.push_back(cat);
				}
			} else {
				str = CALCULATOR->units[i]->category().substr(0, i2);
				for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
					if(it->item == str) {
						p_cat = &(*it);
						b = true;
					}
				}
				if(!b) {
					tree_struct cat;		
					cat.item = str;
					unit_cats.push_back(cat);				
					it = unit_cats.end();
					--it;
					p_cat = &(*it);
				}				
				b = false;
				str = CALCULATOR->units[i]->category().substr(i2 + 1, CALCULATOR->units[i]->category().length() - i2 - 1);
				for(it = p_cat->items.begin(); it != p_cat->items.end(); ++it) {
					if(it->item == str) {
						for(int i3 = 0; i3 < it->objects.size(); i3++) {
							u = (Unit*) it->objects[i3];
							if(CALCULATOR->units[i]->title() < u->title()) {
								b = true;
								it->objects.insert(it->objects.begin() + i3, (void*) CALCULATOR->units[i]);
								break;
							}
						}
						if(!b) it->objects.push_back((void*) CALCULATOR->units[i]);
						b = true;
					}
				}	
				if(!b) {
					tree_struct cat;		
					cat.item = str;
					cat.objects.push_back((void*) CALCULATOR->units[i]);
					p_cat->items.push_back(cat);				
				}			
			}
		}
	}
	
	unit_cats.sort();
	for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
		it->items.sort();
	}
}
void generate_variables_tree_struct() {
	tree_struct *p_cat;
	int i2; bool b;
	bool no_cat = false;	
	string str;
	Variable *v;
	variable_cats.clear();
	uc_variables.clear();
	list<tree_struct>::iterator it;	
	for(int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(CALCULATOR->variables[i]->category().empty()) {
			//uncategorized variable
			b = false;
			for(int i3 = 0; i3 < uc_variables.size(); i3++) {
				v = (Variable*) uc_variables[i3];
				if(CALCULATOR->variables[i]->title() < v->title()) {
					b = true;
					uc_variables.insert(uc_variables.begin() + i3, (void*) CALCULATOR->variables[i]);
					break;
				}
			}
			if(!b) uc_variables.push_back((void*) CALCULATOR->variables[i]);								
		} else {
			b = false;
			//add category if not present
			if((i2 = CALCULATOR->variables[i]->category().find("/")) == string::npos) { 	
				for(it = variable_cats.begin(); it != variable_cats.end(); ++it) {
					if(it->item == CALCULATOR->variables[i]->category()) {
						for(int i3 = 0; i3 < it->objects.size(); i3++) {
							v = (Variable*) it->objects[i3];
							if(CALCULATOR->variables[i]->title() < v->title()) {
								b = true;
								it->objects.insert(it->objects.begin() + i3, (void*) CALCULATOR->variables[i]);
								break;
							}
						}
						if(!b) it->objects.push_back((void*) CALCULATOR->variables[i]);					
						b = true;
					}
				}
				if(!b) {
					tree_struct cat;		
					cat.item = CALCULATOR->variables[i]->category();
					cat.objects.push_back((void*) CALCULATOR->variables[i]);
					variable_cats.push_back(cat);
				}
			} else {
				str = CALCULATOR->variables[i]->category().substr(0, i2);
				for(it = variable_cats.begin(); it != variable_cats.end(); ++it) {
					if(it->item == str) {
						p_cat = &(*it);
						b = true;
					}
				}
				if(!b) {
					tree_struct cat;		
					cat.item = str;
					variable_cats.push_back(cat);				
					it = variable_cats.end();
					--it;
					p_cat = &(*it);
				}				
				b = false;
				str = CALCULATOR->variables[i]->category().substr(i2 + 1, CALCULATOR->variables[i]->category().length() - i2 - 1);
				for(it = p_cat->items.begin(); it != p_cat->items.end(); ++it) {
					if(it->item == str) {
						for(int i3 = 0; i3 < it->objects.size(); i3++) {
							v = (Variable*) it->objects[i3];
							if(CALCULATOR->variables[i]->title() < v->title()) {
								b = true;
								it->objects.insert(it->objects.begin() + i3, (void*) CALCULATOR->variables[i]);
								break;
							}
						}
						if(!b) it->objects.push_back((void*) CALCULATOR->variables[i]);
						b = true;
					}
				}	
				if(!b) {
					tree_struct cat;		
					cat.item = str;
					cat.objects.push_back((void*) CALCULATOR->variables[i]);
					p_cat->items.push_back(cat);				
				}			
			}
		}
	}
	
	variable_cats.sort();
	for(it = variable_cats.begin(); it != variable_cats.end(); ++it) {
		it->items.sort();
	}
}
void generate_functions_tree_struct() {
	tree_struct *p_cat;
	int i2; bool b;
	bool no_cat = false;	
	string str;
	Function *f;
	function_cats.clear();
	uc_functions.clear();
	list<tree_struct>::iterator it;
	for(int i = 0; i < CALCULATOR->functions.size(); i++) {
		if(CALCULATOR->functions[i]->category().empty()) {
			//uncategorized function
			b = false;
			for(int i3 = 0; i3 < uc_functions.size(); i3++) {
				f = (Function*) uc_functions[i3];
				if(CALCULATOR->functions[i]->title() < f->title()) {
					b = true;
					uc_functions.insert(uc_functions.begin() + i3, (void*) CALCULATOR->functions[i]);
					break;
				}
			}
			if(!b) uc_functions.push_back((void*) CALCULATOR->functions[i]);					
		} else {
			b = false;
			//add category if not present
			if((i2 = CALCULATOR->functions[i]->category().find("/")) == string::npos) { 	
				for(it = function_cats.begin(); it != function_cats.end(); ++it) {
					if(it->item == CALCULATOR->functions[i]->category()) {
						for(int i3 = 0; i3 < it->objects.size(); i3++) {
							f = (Function*) it->objects[i3];
							if(CALCULATOR->functions[i]->title() < f->title()) {
								b = true;
								it->objects.insert(it->objects.begin() + i3, (void*) CALCULATOR->functions[i]);
								break;
							}
						}
						if(!b) it->objects.push_back((void*) CALCULATOR->functions[i]);					
						b = true;
					}
				}
				if(!b) {
					tree_struct cat;		
					cat.item = CALCULATOR->functions[i]->category();
					cat.objects.push_back((void*) CALCULATOR->functions[i]);
					function_cats.push_back(cat);
				}
			} else {
				str = CALCULATOR->functions[i]->category().substr(0, i2);
				for(it = function_cats.begin(); it != function_cats.end(); ++it) {
					if(it->item == str) {
						p_cat = &(*it);
						b = true;
					}
				}
				if(!b) {
					tree_struct cat;		
					cat.item = str;
					function_cats.push_back(cat);				
					it = function_cats.end();
					--it;
					p_cat = &(*it);
				}				
				b = false;
				str = CALCULATOR->functions[i]->category().substr(i2 + 1, CALCULATOR->functions[i]->category().length() - i2 - 1);
				for(it = p_cat->items.begin(); it != p_cat->items.end(); ++it) {
					if(it->item == str) {
						it->objects.push_back((void*) CALCULATOR->functions[i]);
						b = true;
					}
				}	
				if(!b) {
					tree_struct cat;		
					cat.item = str;
					cat.objects.push_back((void*) CALCULATOR->functions[i]);
					p_cat->items.push_back(cat);				
				}			
			}
		}
	}
	
	function_cats.sort();
	for(it = function_cats.begin(); it != function_cats.end(); ++it) {
		it->items.sort();
	}
}

/*
	generate the function categories tree in manage functions dialog
*/
void update_functions_tree(GtkWidget *fwin) {
	if(!fwin)
		return;
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tFunctionCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_tree_store_clear(tFunctionCategories_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_tree_store_append(tFunctionCategories_store, &iter3, NULL);
	gtk_tree_store_set(tFunctionCategories_store, &iter3, 0, _("All"), 1, _("All"), -1);
	string str;
	list<tree_struct>::iterator it, it2;
	for(it = function_cats.begin(); it != function_cats.end(); ++it) {
		gtk_tree_store_append(tFunctionCategories_store, &iter, &iter3);
		if(!it->items.empty()) {
			str = "/";
			str += it->item;
		} else {
			str = it->item;
		}
		gtk_tree_store_set(tFunctionCategories_store, &iter, 0, it->item.c_str(), 1, str.c_str(), -1);
		if(str == selected_function_category) {
			EXPAND_TO_ITER(model, tFunctionCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter);
		}
		if(!it->objects.empty() && !it->items.empty()) {
			gtk_tree_store_append(tFunctionCategories_store, &iter2, &iter);
			gtk_tree_store_set(tFunctionCategories_store, &iter2, 0, it->item.c_str(), 1, it->item.c_str(), -1);						
			if(it->item == selected_function_category) {
				EXPAND_TO_ITER(model, tFunctionCategories, iter2)
				gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter2);
			}		
		}
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			str = it->item;
			str += "/";
			str += it2->item;
			gtk_tree_store_append(tFunctionCategories_store, &iter2, &iter);
			gtk_tree_store_set(tFunctionCategories_store, &iter2, 0, it2->item.c_str(), 1, str.c_str(), -1);		
			if(str == selected_function_category) {
				EXPAND_TO_ITER(model, tFunctionCategories, iter2)
				gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter2);
			}
		}
	}
	if(!uc_functions.empty()) {
		//add "Uncategorized" category if there are functions without category
		gtk_tree_store_append(tFunctionCategories_store, &iter, &iter3);
		EXPAND_TO_ITER(model, tFunctionCategories, iter)
		gtk_tree_store_set(tFunctionCategories_store, &iter, 0, _("Uncategorized"), 1, _("Uncategorized"), -1);
		if(selected_function_category == _("Uncategorized")) {
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter);
		}
	}	
	if(!gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &model, &iter)) {
		//if no category has been selected (previously selected has been renamed/deleted), select "All"
		selected_function_category = _("All");
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tFunctionCategories_store), &iter);
		EXPAND_ITER(model, tFunctionCategories, iter)
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter);
	}
}

void setFunctionTreeItem(GtkTreeIter &iter2, Function *f) {
	gtk_list_store_append(tFunctions_store, &iter2);
	gtk_list_store_set(tFunctions_store, &iter2, 0, f->title(true).c_str(), 1, f->name().c_str(), -1);
	if(f->name() == selected_function) {
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &iter2);
	}		
}

/*
	generate the function tree in manage functions dialog when category selection has changed
*/
void on_tFunctionCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	bool no_cat = false, b_all = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tFunctions_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_delete"), FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_function_category = gstr;
		if(selected_function_category == _("All"))
			b_all = true;
		else if(selected_function_category == _("Uncategorized"))
			no_cat = true;
		if(!b_all && !no_cat && selected_function_category[0] == '/') {
			list<tree_struct>::iterator it, it2;
			string str = selected_function_category.substr(1, selected_function_category.length() - 1);
			for(it = function_cats.begin(); it != function_cats.end(); ++it) {
				if(str == it->item) {
					for(int i = 0; i < it->objects.size(); i++) {
						setFunctionTreeItem(iter2, (Function*) it->objects[i]);		
					}
					for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
						for(int i = 0; i < it2->objects.size(); i++) {
							setFunctionTreeItem(iter2, (Function*) it2->objects[i]);		
						}						
					}
					break;
				}
			}
		} else {			
			for(int i = 0; i < CALCULATOR->functions.size(); i++) {
				if(b_all || CALCULATOR->functions[i]->category().empty() && no_cat || CALCULATOR->functions[i]->category() == selected_function_category) {
					setFunctionTreeItem(iter2, CALCULATOR->functions[i]);
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

/*
	function selection has changed
*/
void on_tFunctions_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	bool no_cat = false, b_all = false;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		//remember the new selection
		selected_function = gstr;
		for(int i = 0; i < CALCULATOR->functions.size(); i++) {
			if(CALCULATOR->functions[i]->name() == selected_function) {
				gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "functions_textview_description"))), CALCULATOR->functions[i]->description().c_str(), -1);
				//disable editing of global functions until everything get sorted out, not
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_edit"), TRUE);
				//enable only delete button if function is defined in definitions file and not in source (builtin)
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_delete"), CALCULATOR->functions[i]->isUserFunction());
			}
		}
		g_free(gstr);
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_delete"), FALSE);
		selected_function = "";
	}
}

/*
	generate the variable categories tree in manage variables dialog
*/
void update_variables_tree(GtkWidget *fwin) {
	if(!fwin)
		return;
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tVariableCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_tree_store_clear(tVariableCategories_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_tree_store_append(tVariableCategories_store, &iter3, NULL);
	gtk_tree_store_set(tVariableCategories_store, &iter3, 0, _("All"), 1, _("All"), -1);
	string str;
	list<tree_struct>::iterator it, it2;
	for(it = variable_cats.begin(); it != variable_cats.end(); ++it) {
		gtk_tree_store_append(tVariableCategories_store, &iter, &iter3);
		if(!it->items.empty()) {
			str = "/";
			str += it->item;
		} else {
			str = it->item;
		}		
		gtk_tree_store_set(tVariableCategories_store, &iter, 0, it->item.c_str(), 1, str.c_str(), -1);
		if(str == selected_variable_category) {
			EXPAND_TO_ITER(model, tVariableCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter);
		}
		if(!it->objects.empty() && !it->items.empty()) {
			gtk_tree_store_append(tVariableCategories_store, &iter2, &iter);
			gtk_tree_store_set(tVariableCategories_store, &iter2, 0, it->item.c_str(), 1, it->item.c_str(), -1);						
			if(it->item == selected_variable_category) {
				EXPAND_TO_ITER(model, tVariableCategories, iter2)
				gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter2);
			}		
		}
		
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			str = it->item;
			str += "/";
			str += it2->item;
			gtk_tree_store_append(tVariableCategories_store, &iter2, &iter);
			gtk_tree_store_set(tVariableCategories_store, &iter2, 0, it2->item.c_str(), 1, str.c_str(), -1);		
			if(str == selected_variable_category) {
				EXPAND_TO_ITER(model, tVariableCategories, iter2)
				gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter2);
			}
		}
	}	
	
	if(!uc_variables.empty()) {
		//add "Uncategorized" category if there are variables without category
		gtk_tree_store_append(tVariableCategories_store, &iter, &iter3);
		EXPAND_TO_ITER(model, tVariableCategories, iter)
		gtk_tree_store_set(tVariableCategories_store, &iter, 0, _("Uncategorized"), 1, _("Uncategorized"), -1);
		if(selected_variable_category == _("Uncategorized")) {
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter);
		}
	}
	if(!gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &model, &iter)) {
		//if no category has been selected (previously selected has been renamed/deleted), select "All"
		selected_variable_category = _("All");
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tVariableCategories_store), &iter);
		EXPAND_ITER(model, tVariableCategories, iter)
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter);
	}
}

void setVariableTreeItem(GtkTreeIter &iter2, Variable *v) {
	gtk_list_store_append(tVariables_store, &iter2);
	gtk_list_store_set(tVariables_store, &iter2, 0, v->title(true).c_str(), 1, v->get()->print().c_str(), 2, v->name().c_str(), -1);
	if(v->name() == selected_variable) {
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &iter2);
	}
}

/*
	generate the variable tree in manage variables dialog when category selection has changed
*/
void on_tVariableCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	bool no_cat = false, b_all = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tVariables_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_delete"), FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_variable_category = gstr;		
		if(selected_variable_category == _("All"))
			b_all = true;
		else if(selected_variable_category == _("Uncategorized"))
			no_cat = true;
		if(!b_all && !no_cat && selected_variable_category[0] == '/') {
			list<tree_struct>::iterator it, it2;
			string str = selected_variable_category.substr(1, selected_variable_category.length() - 1);
			for(it = variable_cats.begin(); it != variable_cats.end(); ++it) {
				if(str == it->item) {
					for(int i = 0; i < it->objects.size(); i++) {
						setVariableTreeItem(iter2, (Variable*) it->objects[i]);		
					}
					for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
						for(int i = 0; i < it2->objects.size(); i++) {
							setVariableTreeItem(iter2, (Variable*) it2->objects[i]);		
						}						
					}
					break;
				}
			}
		} else {			
			for(int i = 0; i < CALCULATOR->variables.size(); i++) {
				if(b_all || CALCULATOR->variables[i]->category().empty() && no_cat || CALCULATOR->variables[i]->category() == selected_variable_category) {
					setVariableTreeItem(iter2, CALCULATOR->variables[i]);
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

/*
	variable selection has changed
*/
void on_tVariables_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	bool no_cat = false, b_all = false;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 2, &gstr, -1);
		//remember selection
		selected_variable = gstr;
		for(int i = 0; i < CALCULATOR->variables.size(); i++) {
			if(CALCULATOR->variables[i]->name() == selected_variable) {
				//disable editing of global variables until everything get sorted out, not
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_edit"), TRUE);
				//user is not allowed to delete some variables (pi and e)
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_delete"), CALCULATOR->variables[i]->isUserVariable());
			}
		}
		g_free(gstr);
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_delete"), FALSE);
		selected_variable = "";
	}
}


/*
	generate the unit categories tree in manage units dialog
*/
void update_units_tree(GtkWidget *fwin) {
	if(!fwin)
		return;		
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tUnitCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_tree_store_clear(tUnitCategories_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_tree_store_append(tUnitCategories_store, &iter3, NULL);
	gtk_tree_store_set(tUnitCategories_store, &iter3, 0, _("All"), 1, _("All"), -1);
	string str;
	list<tree_struct>::iterator it, it2;
	for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
		gtk_tree_store_append(tUnitCategories_store, &iter, &iter3);
		if(!it->items.empty()) {
			str = "/";
			str += it->item;
		} else {
			str = it->item;
		}
		gtk_tree_store_set(tUnitCategories_store, &iter, 0, it->item.c_str(), 1, str.c_str(), -1);		
		if(str == selected_unit_category) {
			EXPAND_TO_ITER(model, tUnitCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter);
		}
		if(!it->objects.empty() && !it->items.empty()) {
			gtk_tree_store_append(tUnitCategories_store, &iter2, &iter);
			gtk_tree_store_set(tUnitCategories_store, &iter2, 0, it->item.c_str(), 1, it->item.c_str(), -1);						
			if(it->item == selected_unit_category) {
				EXPAND_TO_ITER(model, tUnitCategories, iter2)
				gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter2);
			}		
		}
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			str = it->item;
			str += "/";
			str += it2->item;
			gtk_tree_store_append(tUnitCategories_store, &iter2, &iter);
			gtk_tree_store_set(tUnitCategories_store, &iter2, 0, it2->item.c_str(), 1, str.c_str(), -1);					
			if(str == selected_unit_category) {
				EXPAND_TO_ITER(model, tUnitCategories, iter2)
				gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter2);
			}
		}
	}
	if(!uc_units.empty()) {
		//add "Uncategorized" category if there are units without category
		gtk_tree_store_append(tUnitCategories_store, &iter, &iter3);
		gtk_tree_store_set(tUnitCategories_store, &iter, 0, _("Uncategorized"), 1, _("Uncategorized"), -1);
		if(selected_unit_category == _("Uncategorized")) {
			EXPAND_TO_ITER(model, tUnitCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter);
		}
	}	
	if(!gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &model, &iter)) {
		//if no category has been selected (previously selected has been renamed/deleted), select "All"
		selected_unit_category = _("All");
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnitCategories_store), &iter);
		EXPAND_ITER(model, tUnitCategories, iter)
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter);
	}
}

void setUnitTreeItem(GtkTreeIter &iter2, Unit *u) {
	gtk_list_store_append(tUnits_store, &iter2);
	string stype, snames, sbase;
	//display name, plural name and short name in the second column
	AliasUnit *au;	
	snames = u->name();
	if(!u->plural(false).empty()) {
		snames += "/";
		snames += u->plural();
	}
	if(!u->shortName(false).empty()) {
		snames += ": ";
		snames += u->shortName();
	}
	//depending on unit type display relation to base unit(s)
	switch(u->type()) {
		case 'D': {
			stype = _("COMPOSITE UNIT");
			snames = "";
			sbase = u->shortName(true);
			break;
		}
		case 'A': {
			stype = _("ALIAS");
			au = (AliasUnit*) u;
			if(use_short_units) {
				sbase = au->firstShortBaseExpName();
			} else {
				sbase = au->firstBaseExpName();
			}
			break;
		}
		case 'U': {
			stype = _("BASE UNIT");
			sbase = "";
			break;
		}
	}
	//display descriptive name (title), or name if no title defined
	gtk_list_store_set(tUnits_store, &iter2, UNITS_TITLE_COLUMN, u->title(true).c_str(), UNITS_NAME_COLUMN, u->name().c_str(), UNITS_TYPE_COLUMN, stype.c_str(), UNITS_NAMES_COLUMN, snames.c_str(), UNITS_BASE_COLUMN, sbase.c_str(), UNITS_POINTER_COLUMN, (gpointer) u, -1);
	if(u == selected_unit) {
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &iter2);
	}
}

/*
	generate the unit tree and units conversion menu in manage units dialog when category selection has changed
*/
void on_tUnitCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	//make sure that no unit conversion is done in the dialog until everthing is updated
	block_unit_convert = true;

	bool no_cat = false, b_all = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits));
	gulong s_handler = get_signal_handler(G_OBJECT(select));
	block_signal(G_OBJECT(select), s_handler);
	gtk_list_store_clear(tUnits_store);
	unblock_signal(G_OBJECT(select), s_handler);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_delete"), FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_unit_category = gstr;
		if(selected_unit_category == _("All"))
			b_all = true;
		else if(selected_unit_category == _("Uncategorized"))
			no_cat = true;
		if(!b_all && !no_cat && selected_unit_category[0] == '/') {
			list<tree_struct>::iterator it, it2;
			string str = selected_unit_category.substr(1, selected_unit_category.length() - 1);
			for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
				if(str == it->item) {
					for(int i = 0; i < it->objects.size(); i++) {
						setUnitTreeItem(iter2, (Unit*) it->objects[i]);		
					}
					for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
						for(int i = 0; i < it2->objects.size(); i++) {
							setUnitTreeItem(iter2, (Unit*) it2->objects[i]);		
						}						
					}
					break;
				}
			}
		} else {
			for(int i = 0; i < CALCULATOR->units.size(); i++) {
				if(b_all || CALCULATOR->units[i]->category().empty() && no_cat || CALCULATOR->units[i]->category() == selected_unit_category) {
					setUnitTreeItem(iter2, CALCULATOR->units[i]);			
				}
			}
		}
		if(!selected_unit || !gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &model2, &iter2)) {
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnits_store), &iter2);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &iter2);
		}
		g_free(gstr);
	} else {
		selected_unit_category = "";
	}
	//generate convert to menu
	GtkWidget *tmp_w = omToUnit_menu;
	if(tmp_w)
		gtk_widget_destroy(tmp_w);
	omToUnit_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "units_optionmenu_to_unit")), omToUnit_menu);
	GtkWidget *sub = omToUnit_menu;
	GtkWidget *item;
	int i = 0, h = -1;
	//add all units in units tree to menu
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnits_store), &iter2);
	Unit *u;
	while(b) {
		gchar *gstr;
		gtk_tree_model_get(GTK_TREE_MODEL(tUnits_store), &iter2, UNITS_TITLE_COLUMN, &gstr, UNITS_POINTER_COLUMN, &u, -1);
		if(!selected_to_unit)
			selected_to_unit = u;
		if(u) {
			MENU_ITEM_WITH_POINTER(gstr, on_omToUnit_menu_activate, u)
			if(selected_to_unit == u)
				h = i;
		}
		b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tUnits_store), &iter2);
		i++;
		g_free(gstr);
	}
	//if no items were added to the menu, reset selected unit
	if(i == 0)
		selected_to_unit = NULL;
	else {
		//if no menu item was selected, select the first
		if(h < 0) {
			h = 0;
			b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnits_store), &iter2);
			if(b) {
				gtk_tree_model_get(GTK_TREE_MODEL(tUnits_store), &iter2, UNITS_POINTER_COLUMN, &u, -1);
				selected_to_unit = u;
			}
		}
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "units_optionmenu_to_unit")), h);
	}
	block_unit_convert = false;		
	//update conversion display
	convert_in_wUnits();
}

/*
	unit selection has changed
*/
void on_tUnits_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	bool no_cat = false, b_all = false;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Unit *u;
		gtk_tree_model_get(model, &iter, UNITS_POINTER_COLUMN, &u, -1);
		selected_unit = u;
		for(int i = 0; i < CALCULATOR->units.size(); i++) {
			if(CALCULATOR->units[i] == selected_unit) {
				if(use_short_units)
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "units_label_from_unit")), CALCULATOR->units[i]->shortName(true).c_str());
				else
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "units_label_from_unit")), CALCULATOR->units[i]->plural(true).c_str());
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_delete"), CALCULATOR->units[i]->isUserUnit());
				//disable editing of global units until everything get sorted out, not
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_edit"), TRUE);
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_delete"), FALSE);
		selected_unit = NULL;
	}
	if(!block_unit_convert) convert_in_wUnits();
}

/*
	generate unit submenu in expression menu
	menus are not sorted yet
*/
void create_umenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Units"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression"))), 5)
	u_menu = item;
	sub2 = sub;
	Unit *u;
	list<tree_struct>::iterator it, it2;
	for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			u = (Unit*) it->objects[i];
			MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				u = (Unit*) it2->objects[i];
				MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
			}			
		}
	}		
	sub = sub2;
	for(int i = 0; i < uc_units.size(); i++) {
		u = (Unit*) uc_units[i];
		MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
	}	
	MENU_SEPARATOR	
	MENU_ITEM(_("Create new unit"), new_unit);
	MENU_ITEM(_("Manage units"), manage_units);
	MENU_ITEM_SET_ACCEL(GDK_u);
	if(CALCULATOR->unitsEnabled()) {
		MENU_ITEM(_("Disable units"), set_units_enabled)
	} else {
		MENU_ITEM(_("Enable units"), set_units_enabled)
	}
	u_enable_item = item;
}

/*
	generate unit submenu in result menu
*/
void create_umenu2() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Convert to unit"), gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result"))), 2)
	u_menu2 = item;
	sub2 = sub;
	Unit *u;
	list<tree_struct>::iterator it, it2;
	for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			u = (Unit*) it->objects[i];
			MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				u = (Unit*) it2->objects[i];
				MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
			}			
		}
	}	
	sub = sub2;
	for(int i = 0; i < uc_units.size(); i++) {
		u = (Unit*) uc_units[i];
		MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
	}	
	MENU_SEPARATOR	
	MENU_ITEM(_("Enter custom unit"), convert_to_custom_unit);
	MENU_ITEM(_("Create new unit"), new_unit);
	MENU_ITEM(_("Manage units"), manage_units);
}

/*
	recreate unit menus and update unit manager (when units have changed)
*/
void update_umenus() {
	gtk_widget_destroy(u_menu);
	gtk_widget_destroy(u_menu2);
	generate_units_tree_struct();
	create_umenu();
	create_umenu2();
	update_units_tree(units_window);
}

/*
	generate variables submenu in expression menu
*/
void create_vmenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Variables"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression"))), 3)
	v_menu = item;
	sub2 = sub;
	Variable *v;
	list<tree_struct>::iterator it, it2;
	for(it = variable_cats.begin(); it != variable_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			v = (Variable*) it->objects[i];
			MENU_ITEM_WITH_STRING(v->title(true).c_str(), insert_variable, v->name().c_str());
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				v = (Variable*) it2->objects[i];
				MENU_ITEM_WITH_STRING(v->title(true).c_str(), insert_variable, v->name().c_str());
			}			
		}
	}		
	sub = sub2;
	for(int i = 0; i < uc_variables.size(); i++) {
		v = (Variable*) uc_variables[i];
		MENU_ITEM_WITH_STRING(v->title(true).c_str(), insert_variable, v->name().c_str());
	}	
	MENU_SEPARATOR	
	MENU_ITEM(_("Create new variable"), new_variable);
	MENU_ITEM(_("Manage variables"), manage_variables);
	MENU_ITEM_SET_ACCEL(GDK_m);
	if(CALCULATOR->variablesEnabled()) {
		MENU_ITEM(_("Disable variables"), set_variables_enabled)
	} else {
		MENU_ITEM(_("Enable variables"), set_variables_enabled)
	}
	v_enable_item = item;
	if(CALCULATOR->donotCalculateVariables()) {
		MENU_ITEM(_("Calculate variables"), set_donot_calcvars)
	} else {
		MENU_ITEM(_("Do not calculate variables"), set_donot_calcvars)
	}
	v_calcvar_item = item;	
	if(CALCULATOR->unknownVariablesEnabled()) {
		MENU_ITEM(_("Disable unknown variables"), set_unknownvariables_enabled)
	} else {
		MENU_ITEM(_("Enable unknown variables"), set_unknownvariables_enabled)
	}
	uv_enable_item = item;
}

/*
	generate prefixes submenu in expression menu
*/
void create_pmenu() {
	GtkWidget *item, *item2;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Prefixes"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression"))), 4)
	int index = 0;
	Prefix *p = CALCULATOR->getPrefix(index);
	while(p) {
		gchar *gstr = g_strdup_printf("%s (10<sup>%li</sup>)", p->name(false).c_str(), p->exponent());
		MENU_ITEM_WITH_POINTER(gstr, insert_prefix, p)
		gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child(GTK_BIN(item))), TRUE);
		g_free(gstr);			
		index++;
		p = CALCULATOR->getPrefix(index);
	}
}

/*
	generate prefixes submenu in result menu
*/
void create_pmenu2() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Set prefix"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result"))), 2)
	int index = 0;
	Prefix *p = CALCULATOR->getPrefix(index);
	while(p) {
		gchar *gstr = g_strdup_printf("%s (10<sup>%li</sup>)", p->name(false).c_str(), p->exponent());
		MENU_ITEM_WITH_POINTER(gstr, set_prefix, p)
		gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child(GTK_BIN(item))), TRUE);
		g_free(gstr);			
		index++;
		p = CALCULATOR->getPrefix(index);
	}	
}

/*
	recreate variables menu and update variable manager (when variables have changed)
*/
void update_vmenu() {
	gtk_widget_destroy(v_menu);
	generate_variables_tree_struct();
	create_vmenu();
	update_variables_tree(variables_window);
}

/*
	generate functions submenu in expression menu
*/
void create_fmenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Functions"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression"))), 2)
	f_menu = item;
	sub2 = sub;
	Function *f;
	list<tree_struct>::iterator it, it2;
	for(it = function_cats.begin(); it != function_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			f = (Function*) it->objects[i];
			MENU_ITEM_WITH_STRING(f->title(true).c_str(), insert_function, f->name().c_str())
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				f = (Function*) it2->objects[i];
				MENU_ITEM_WITH_STRING(f->title(true).c_str(), insert_function, f->name().c_str())
			}			
		}
	}		
	sub = sub2;
	for(int i = 0; i < uc_functions.size(); i++) {
		f = (Function*) uc_functions[i];
		MENU_ITEM_WITH_STRING(f->title(true).c_str(), insert_function, f->name().c_str())
	}		
	MENU_SEPARATOR
	MENU_ITEM(_("Create new function"), new_function);
	MENU_ITEM(_("Manage functions"), manage_functions);
	MENU_ITEM_SET_ACCEL(GDK_f);
	if(CALCULATOR->functionsEnabled()) {
		MENU_ITEM(_("Disable functions"), set_functions_enabled)
	} else {
		MENU_ITEM(_("Enable functions"), set_functions_enabled)
	}
	f_enable_item = item;
}

/*
	recreate functions menu and update function manager (when functions have changed)
*/
void update_fmenu() {
	gtk_widget_destroy(f_menu);
	generate_functions_tree_struct();
	create_fmenu();
	update_functions_tree(functions_window);
}


/*
	return a customized result string
	set rlabel to true for result label (nicer look)
*/
string get_value_string(Manager *mngr_, bool rlabel = false, Prefix *prefix = NULL, bool *in_exact = NULL) {
	int displayflags = 0;
	displayflags = displayflags | DISPLAY_FORMAT_BEAUTIFY;
	if(fraction_is_on) displayflags = displayflags | DISPLAY_FORMAT_FRACTION;	
	else displayflags = displayflags | DISPLAY_FORMAT_DECIMAL_ONLY;		
	if(use_short_units) displayflags = displayflags | DISPLAY_FORMAT_SHORT_UNITS;
	else displayflags = displayflags | DISPLAY_FORMAT_LONG_UNITS;
	if(use_unicode_signs) displayflags = displayflags | DISPLAY_FORMAT_NONASCII;	
	if(rlabel) {
		displayflags = displayflags | DISPLAY_FORMAT_TAGS;
		displayflags = displayflags | DISPLAY_FORMAT_ALLOW_NOT_USABLE;
	}
	NumberFormat numberformat;
	if(number_base == BASE_OCTAL) numberformat = NUMBER_FORMAT_OCTAL;
	else if(number_base == BASE_HEX) numberformat = NUMBER_FORMAT_HEX;
	else if(number_base == BASE_BIN) numberformat = NUMBER_FORMAT_BIN;
	else {
		if(prefix) {
			numberformat = NUMBER_FORMAT_PREFIX;
		} else {
			switch(display_mode) {
				case MODE_SCIENTIFIC: {numberformat = NUMBER_FORMAT_EXP_PURE; displayflags = displayflags | DISPLAY_FORMAT_SCIENTIFIC; break;}
				case MODE_PREFIXES: {numberformat = NUMBER_FORMAT_PREFIX; break;}
				case MODE_DECIMALS: {numberformat = NUMBER_FORMAT_DECIMALS; break;}
				default: {numberformat = NUMBER_FORMAT_NORMAL;}
			}
		}
	}
	return mngr_->print(numberformat, displayflags, decimals, true, deci_mode == DECI_FIXED, in_exact, NULL, prefix);
}

/*
	set result in result widget and add to history widget
*/
void setResult(const gchar *expr, Prefix *prefix = NULL) {
	GtkTextIter iter;
	GtkTextBuffer *tb;
	string str = expr, str2;

	//update "ans" variables
	vans->set(mngr);
	vAns->set(mngr);

	bool in_exact = false;

	str2 = get_value_string(mngr, true, prefix, &in_exact);
	
	gtk_label_set_selectable(GTK_LABEL(result), true);
	gtk_widget_set_size_request(result, -1, -1);	
//	pango_parse_markup(str2.c_str(), -1, 0, NULL, NULL, NULL, NULL);
	if(str2.length() > 150) {
		show_message("The result is too large to resonably fit the window.", glade_xml_get_widget (glade_xml, "main_window"));
		str2 = "does not fit window";
	}
	gtk_label_set_text(GTK_LABEL(result), str2.c_str());
	gtk_label_set_use_markup(GTK_LABEL(result), TRUE);
	str2 = "<big>";
	if(in_exact) {
		str2 += SIGN_ALMOST_EQUAL;	
	} else {
		str2 += "=";
	}
	str2 += "</big>";
	gtk_label_set_text(GTK_LABEL(label_equals), str2.c_str());
	gtk_label_set_use_markup(GTK_LABEL(label_equals), TRUE);

	//mark the whole expression
	gtk_editable_select_region(GTK_EDITABLE(expression), 0, -1);
	display_errors();
	tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "history")));
	gtk_text_buffer_get_start_iter(tb, &iter);
	gtk_text_buffer_insert(tb, &iter, str.c_str(), -1);
	gtk_text_buffer_insert(tb, &iter, " ", -1);
	if(mngr->isPrecise()) gtk_text_buffer_insert(tb, &iter, "=", -1);	
	else gtk_text_buffer_insert(tb, &iter, SIGN_ALMOST_EQUAL, -1);		
	gtk_text_buffer_insert(tb, &iter, " ", -1);	
	gtk_text_buffer_insert(tb, &iter, get_value_string(mngr, false, prefix).c_str(), -1);
	gtk_text_buffer_insert(tb, &iter, "\n", -1);
	gtk_text_buffer_place_cursor(tb, &iter);
	while(CALCULATOR->error()) {
		CALCULATOR->nextError();
	}
}

void set_prefix(GtkMenuItem *w, gpointer user_data) {
	setResult(gtk_label_get_text(GTK_LABEL(result)), (Prefix*) user_data);
	gtk_widget_grab_focus(expression);	
}

/*
	calculate entered expression and display result
*/
void execute_expression() {
	string str = gtk_entry_get_text(GTK_ENTRY(expression));
	//unreference previous result (deletes the object if it is not used anywhere else
	mngr->unref();
	mngr = CALCULATOR->calculate(str);
	display_errors();
	setResult(gtk_entry_get_text(GTK_ENTRY(expression)));
	gtk_widget_grab_focus(expression);
	//gtk_editable_set_position(GTK_EDITABLE(expression), -1);
}



/*
	calculate position of expression menu
*/
void menu_e_posfunc(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
	GtkWidget *window = glade_xml_get_widget (glade_xml, "main_window");
	gint root_x = 0, root_y = 0, size_x = 0, size_y = 0;
	GdkRectangle rect;
	gdk_window_get_frame_extents(window->window, &rect);
	gtk_window_get_position(GTK_WINDOW(window), &root_x, &root_y);
	gtk_window_get_size(GTK_WINDOW(window), &size_x, &size_y);
	*x = root_x + (rect.width - size_x) / 2 + glade_xml_get_widget (glade_xml, "togglebutton_expression")->allocation.x + glade_xml_get_widget (glade_xml, "togglebutton_expression")->allocation.width;
	*y = root_y + (rect.height - size_y) / 2 + glade_xml_get_widget (glade_xml, "togglebutton_expression")->allocation.y;
	*push_in = false;
}

/*
	calculate position of result menu
*/
void menu_r_posfunc(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
	GtkWidget *window = glade_xml_get_widget (glade_xml, "main_window");
	gint root_x = 0, root_y = 0, size_x = 0, size_y = 0;
	GdkRectangle rect;
	gdk_window_get_frame_extents(window->window, &rect);
	gtk_window_get_position(GTK_WINDOW(window), &root_x, &root_y);
	gtk_window_get_size(GTK_WINDOW(window), &size_x, &size_y);
	*x = root_x + (rect.width - size_x) / 2 + glade_xml_get_widget (glade_xml, "togglebutton_result")->allocation.x + glade_xml_get_widget (glade_xml, "togglebutton_result")->allocation.width;
	*y = root_y + (rect.height - size_y) / 2 + glade_xml_get_widget (glade_xml, "togglebutton_result")->allocation.y;
	*push_in = false;
}


/*
	general function used to insert text in expression entry
*/
void insert_text(const gchar *name) {
	gint position;
	gint start = 0, end = 0;
	//overwrite selection
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {
		gtk_editable_set_position(GTK_EDITABLE(expression), start);
		gtk_editable_delete_text(GTK_EDITABLE(expression), start, end);
	}
	//insert the text at current position
	position = gtk_editable_get_position(GTK_EDITABLE(expression));
	gtk_editable_insert_text(GTK_EDITABLE(expression), name, strlen(name), &position);
	gtk_editable_set_position(GTK_EDITABLE(expression), position);
	gtk_widget_grab_focus(expression);
	//unselect
	gtk_editable_select_region(GTK_EDITABLE(expression), position, position);
}


/*
	insert function
	pops up an argument entry dialog and inserts function into expression entry
	parent is parent window
*/
void insert_function(Function *f, GtkWidget *parent = NULL) {
	if(!f)
		return;
	gint start = 0, end = 0;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end);
	GtkWidget *dialog;
	int args = f->args();
	//if function takes no arguments, do not display dialog and insert function directly
	if(args == 0) {
		string str = f->name() + "()";
		gchar *gstr = g_strdup(str.c_str());
		insert_text(gstr);
		g_free(gstr);
		return;
	}
	string f_title = f->title(true);
	dialog = gtk_dialog_new_with_buttons(f_title.c_str(), GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, GTK_STOCK_EXECUTE, GTK_RESPONSE_APPLY, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);	
	GtkWidget *vbox_pre = gtk_vbox_new(false, 12);
	GtkWidget *vbox = gtk_vbox_new(false, 6);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_pre), 12);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox_pre);
	f_title.insert(0, "<b>");
	f_title += "</b>";
	GtkWidget *title_label = gtk_label_new(f_title.c_str());
	gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
	gtk_misc_set_alignment(GTK_MISC(title_label), 0.0, 0.5);
	gtk_container_add(GTK_CONTAINER(vbox_pre), title_label);	
	gtk_container_add(GTK_CONTAINER(vbox_pre), vbox);	
	GtkWidget *label[args];
	GtkWidget *entry[args];
	GtkWidget *descr, *entry1, *label1;
	gchar *title[args];
	gchar *title1;
	//create argument entries
	for(int i = 0; i < args; i++) {
		if(f->argName(i + 1).empty())
			title[i] = g_strdup_printf(_("Argument %i"), i + 1);
		else
			title[i] = g_strdup(f->argName(i + 1).c_str());
		label[i] = gtk_label_new(title[i]);
		entry[i] = gtk_entry_new();
		//insert selection in expression entry into the first argument entry
		if(i == 0) {
			gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
			gtk_entry_set_text(GTK_ENTRY(entry[i]), gstr);
			g_free(gstr);
		}
		GtkWidget *hbox = gtk_hbox_new(TRUE, 6);
		gtk_container_add(GTK_CONTAINER(vbox), hbox);		
		gtk_container_add(GTK_CONTAINER(hbox), label[i]);
		gtk_container_add(GTK_CONTAINER(hbox), entry[i]);
		gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
	}
	//arguments is defined as less than zero, the function requests an vector of undefined length
	if(args < 0) {
		if(f->argName(1).empty())
			title1 = g_strdup(_("Vector (1,2,3,4...)"));
		else
			title1 = g_strdup(f->argName(1).c_str());
		label1 = gtk_label_new(title1);
		entry1 = gtk_entry_new();
		GtkWidget *hbox = gtk_hbox_new(TRUE, 6);
		gtk_container_add(GTK_CONTAINER(vbox), hbox);		
		gtk_container_add(GTK_CONTAINER(hbox), label1);
		gtk_container_add(GTK_CONTAINER(hbox), entry1);
		gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	}
	//display function description
	if(!f->description().empty()) {
		descr = gtk_label_new(f->description().c_str());
		gtk_label_set_line_wrap(GTK_LABEL(descr), TRUE);
		gtk_container_add(GTK_CONTAINER(vbox), descr);
		gtk_misc_set_alignment(GTK_MISC(descr), 1, 0.5);
	}
	gtk_widget_show_all(dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_ACCEPT || response == GTK_RESPONSE_APPLY) {
		string str = f->name() + LEFT_BRACKET_STR, str2;
		for(int i = 0; i < args; i++) {
			str2 = gtk_entry_get_text(GTK_ENTRY(entry[i]));

			//if the minimum number of function arguments have been filled, do not add anymore if entry is empty
			if(i >= f->minargs()) {
				remove_blank_ends(str2);
				if(str2.empty()) break;
			}

			if(i > 0)
				str += CALCULATOR->getComma();
			str += str2;
		}
		if(args < 0)
			str += gtk_entry_get_text(GTK_ENTRY(entry1));
		str += RIGHT_BRACKET_STR;
		//redo selection if "OK" was clicked, clear expression entry "Execute"
		if(response == GTK_RESPONSE_ACCEPT)
			gtk_editable_select_region(GTK_EDITABLE(expression), start, end);
		else
			gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
		gchar *gstr = g_strdup(str.c_str());
		insert_text(gstr);
		g_free(gstr);
		//Calculate directly when "Execute" was clicked
		if(response == GTK_RESPONSE_APPLY)
			execute_expression();
	}
	for(int i = 0; i < args; i++)
		g_free(title[i]);
	if(args < 0)
		g_free(title1);
	gtk_widget_destroy(dialog);
}

/*
	called from function menu
*/
void insert_function(GtkMenuItem *w, gpointer user_data) {
	gchar *name = (gchar*) user_data;
	insert_function(
			CALCULATOR->getFunction(name),
			glade_xml_get_widget (glade_xml, "main_window"));
}

/*
	called from variable menu
	just insert text data stored in menu item
*/
void insert_variable(GtkMenuItem *w, gpointer user_data) {
	insert_text((gchar*) user_data);
}
//from prefix menu
void insert_prefix(GtkMenuItem *w, gpointer user_data) {
	insert_text(((Prefix*) user_data)->name(false).c_str());
}
//from unit menu
void insert_unit(GtkMenuItem *w, gpointer user_data) {
	insert_text(((Unit*) user_data)->shortName(true, true).c_str());
}

/*
	display edit/new unit dialog
	creates new unit if u == NULL, win is parent window
*/
void
edit_unit(const char *category = "", Unit *u = NULL, GtkWidget *win = NULL)
{
	GtkWidget *dialog = create_unit_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	
	if(u) {
		if(u->isUserUnit())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unit"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unit (read only)"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Unit"));
	}

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category")), category);

	//clear entries
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")), "");	
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), "");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_spinbutton_exp")), 1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_reversed")), "");

	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "unit_edit_button_ok"), !u || u->isUserUnit());

	if(u) {
		//fill in original parameters
		if(u->type() == 'U')
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), BASE_UNIT);
		else if(u->type() == 'A')
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), ALIAS_UNIT);
		else if(u->type() == 'D')
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), COMPOSITE_UNIT);

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")), u->name().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")), u->name().c_str());		

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural")), u->plural(false).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short")), u->shortName(false).c_str());

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category")), u->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc")), u->title(false).c_str());
		
/*		if(!u->isUserUnit()) {
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"), FALSE);
		}*/

		switch(u->type()) {
			case 'A': {
				AliasUnit *au = (AliasUnit*) u;
				if(au->firstBaseUnit()->type() == 'D') {
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), ((CompositeUnit*) (au->firstBaseUnit()))->internalName().c_str());
				} else {
					if(use_short_units)
						gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), au->firstShortBaseName().c_str());
					else
						gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), au->firstBaseName().c_str());	
				}		
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_spinbutton_exp")), au->firstBaseExp());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation")), au->expression().c_str());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_reversed")), au->reverseExpression().c_str());
				break;
			}
			case 'D': {
				if(use_short_units)
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), u->shortName(true).c_str());
				else
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), u->name().c_str());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")), ((CompositeUnit*) u)->internalName().c_str());			
			}
		}

	} else {
		//default values
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), ALIAS_UNIT);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation")), "1");
	}

run_unit_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		int type = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")));		
		string str;
		if(type == COMPOSITE_UNIT) str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")));
		else str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name given
			if(type == COMPOSITE_UNIT) {
				show_message(_("Empty internal name field."), dialog);
				goto run_unit_edit_dialog;			
			} else {
				str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short")));
				remove_blank_ends(str);
				if(str.empty()) {
					//no short unit name either -- open dialog again
					show_message(_("Empty name field."), dialog);
					goto run_unit_edit_dialog;
				} else {
					//switch short name and name
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short"))));
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short")), "");
				}
			}
		}

		//unit with the same name exists -- overwrite or open the dialog again
		if(type == COMPOSITE_UNIT) {
			if(CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal"))), u) && !ask_question(_("A unit with the same internal name already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}		
		} else {
			if(CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))), u) && !ask_question(_("A unit with the same name already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}
			if(CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))), u) && !ask_question(_("A unit with the same plural name already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}
			if(CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short"))), u) && !ask_question(_("A unit with the same short format already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}
		}
		if(u) {
			//edited an existing unit -- update unit
			gint i1 = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")));
			switch(u->type()) {
				case 'A': {
					if(i1 != ALIAS_UNIT) {
						CALCULATOR->delUnit(u);
						u = NULL;
						break;
					}
					AliasUnit *au = (AliasUnit*) u;
					Unit *bu = CALCULATOR->getUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));
					if(!bu) bu = CALCULATOR->getCompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));
					if(!bu) {
						show_message(_("Base unit does not exist."), dialog);
						goto run_unit_edit_dialog;
					}
					au->setBaseUnit(bu);
					au->setExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation"))));
					au->setReverseExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_reversed"))));
					au->setExponent(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_spinbutton_exp"))));
					break;
				}
				case 'D': {
					if(i1 != COMPOSITE_UNIT) {
						CALCULATOR->delUnit(u);
						u = NULL;
						break;
					}
					u->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal"))));
					((CompositeUnit*) u)->setBaseExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));					
					break;
				}
				case 'U': {
					if(i1 != BASE_UNIT) {
						CALCULATOR->delUnit(u);
						u = NULL;
						break;
					}
					break;
				}
			}
			if(u && u->type() != 'D') {
				u->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))));
				u->setPlural(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))));
				u->setShortName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short"))));
			}
			u->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))));
			u->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))));			
			u->setUserUnit(true);
		}
		if(!u) {
			//new unit
			switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")))) {
				case ALIAS_UNIT: {
					Unit *bu = CALCULATOR->getUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));
					if(!bu) bu = CALCULATOR->getCompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));
					if(!bu) {
						show_message(_("Base unit does not exist."), dialog);
						goto run_unit_edit_dialog;
					}
					u = new AliasUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))), bu, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation"))), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_spinbutton_exp"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_reversed"))), true);
					break;
				}
				case COMPOSITE_UNIT: {
					CompositeUnit *cu = new CompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))), true);
					u = cu;
					break;
				}
				default: {
					u = new Unit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_short"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))), true);
					break;
				}
			}
			if(u)
				CALCULATOR->addUnit(u);
		}
		//select the new unit
		selected_unit = u;
		selected_unit_category = u->category();
		if(selected_unit_category.empty())
			selected_unit_category = _("Uncategorized");
		update_umenus();
	}
	gtk_widget_hide(dialog);
}

/*
	display edit/new function dialog
	creates new function if f == NULL, win is parent window
*/
void edit_function(const char *category = "", Function *f = NULL, GtkWidget *win = NULL) {

	GtkWidget *dialog = create_function_edit_dialog();	
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	
	if(f) {
		if(f->isUserFunction() && !f->isBuiltinFunction())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Function"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Function (read only)"));		
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Function"));
	}

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "function_edit_textview_description")));

	//clear entries
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_expression")), "");
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_entry_name"), TRUE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_entry_expression"), TRUE);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category")), category);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc")), "");
	gtk_text_buffer_set_text(buffer, "", -1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_arguments")), "");

	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_ok"), !f || (!f->isBuiltinFunction() && f->isUserFunction()));	

	if(f) {
		//fill in original paramaters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name")), f->name().c_str());
		if(!f->isBuiltinFunction())
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_expression")), ((UserFunction*) f)->equation().c_str());
//		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_entry_name"), !f->isBuiltinFunction() && f->isUserFunction());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_entry_name"), !f->isBuiltinFunction());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_entry_expression"), !f->isBuiltinFunction());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category")), f->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc")), f->title(false).c_str());
		gtk_text_buffer_set_text(buffer, f->description().c_str(), -1);
		string str;
		for(int i = 1; !f->argName(i).empty(); i++) {
			if(i == 1)
				str = f->argName(i);
			else {
				str += CALCULATOR->getComma();
				str += " ";
				str += f->argName(i);
			}
		}
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_arguments")), str.c_str());
	}

run_function_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name -- open dialog again
			show_message(_("Empty name field."), dialog);
			goto run_function_edit_dialog;
		}
		str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_expression")));
		remove_blank_ends(str);
		if(!(f && f->isBuiltinFunction()) && str.empty()) {
			//no expression/relation -- open dialog again
			show_message(_("Empty expression field."), dialog);
			goto run_function_edit_dialog;
		}
		GtkTextIter iter_s, iter_e;
		gtk_text_buffer_get_start_iter(buffer, &iter_s);
		gtk_text_buffer_get_end_iter(buffer, &iter_e);
		//function with the same name exists -- overwrite or open the dialog again
		if(CALCULATOR->nameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name"))), (void*) f) && !ask_question(_("A function or variable with the same name already exists.\nOverwrite function/variable?"), dialog)) {
			goto run_function_edit_dialog;
		}	
		if(f) {
			//edited an existing function
			if(!f->isBuiltinFunction())
				f->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name"))));
			f->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category"))));
			f->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc"))));
			f->setDescription(gtk_text_buffer_get_text(buffer, &iter_s, &iter_e, FALSE));
			if(!f->isBuiltinFunction())
				((UserFunction*) f)->setEquation(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_expression"))));
			str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_arguments")));
			string str2;
			unsigned int i = 0, i2 = 0;
			f->clearArgNames();
			while(true) {
				i = str.find_first_of(COMMA_S, i2);
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
			f->setUserFunction(true);
		} else {
			//new function
			f = new UserFunction(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_expression"))), true, -1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc"))), gtk_text_buffer_get_text(buffer, &iter_s, &iter_e, FALSE));
			str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_arguments")));
			string str2;
			unsigned int i = 0, i2 = 0;
			while(true) {
				i = str.find_first_of(COMMA_S, i2);
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
			CALCULATOR->addFunction(f);
		}
		//select the new function
		selected_function = f->name();
		selected_function_category = f->category();
		if(selected_function_category.empty())
			selected_function_category = _("Uncategorized");
		update_fmenu();	
	}
	gtk_widget_hide(dialog);
}

/*
	"New function" menu item selected
*/
void new_function(GtkMenuItem *w, gpointer user_data)
{
	edit_function(
			"",
			NULL,
			glade_xml_get_widget (glade_xml, "main_window"));
}
/*
	"New unit" menu item selected
*/
void new_unit(GtkMenuItem *w, gpointer user_data)
{
	edit_unit(
			"",
			NULL,
			glade_xml_get_widget (glade_xml, "main_window"));
}

/*
	a unit selected in result menu, convert result
*/
void convert_to_unit(GtkMenuItem *w, gpointer user_data)
{
	GtkWidget *edialog;
	Unit *u = (Unit*) user_data;
	if(!u) {
		edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (glade_xml, "main_window")
				),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Unit does not exist"));
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
	//result is stored in Manager *mngr
//	mngr->convert(u);
//	mngr->finalize();
	CALCULATOR->convert(mngr, u);
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}

void convert_to_custom_unit(GtkMenuItem *w, gpointer user_data)
{
/*	GtkWidget *dialog = gtk_dialog_new_with_buttons(
			_("Convert to custom unit"),
			GTK_WINDOW(
				glade_xml_get_widget (glade_xml, "main_window")
			),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL,
			GTK_RESPONSE_REJECT,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	GtkWidget *vbox = gtk_vbox_new(false, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	GtkWidget *label1 = gtk_label_new(_("Unit expression:"));
	GtkWidget *entry1 = gtk_entry_new();
	gtk_misc_set_alignment(GTK_MISC(label1), 0, 0.5);
	gtk_container_add(GTK_CONTAINER(vbox), label1);
	gtk_container_add(GTK_CONTAINER(vbox), entry1);
	gtk_widget_show_all(dialog);*/
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "unit_dialog");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	int i;
	while((i = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_OK || i == GTK_RESPONSE_APPLY) {
//		mngr->convert(gtk_entry_get_text(GTK_ENTRY(entry1)));
//		mngr->finalize();
		CALCULATOR->convert(mngr, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_dialog_entry_unit"))));
		setResult(gtk_label_get_text(GTK_LABEL(result)));
		if(i == GTK_RESPONSE_OK) break;
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	display edit/new variable dialog
	creates new variable if v == NULL, mngr_ is forced value, win is parent window
*/
void edit_variable(const char *category = "", Variable *v = NULL, Manager *mngr_ = NULL, GtkWidget *win = NULL) {

	GtkWidget *dialog = create_variable_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	if(v) {
		if(v->isUserVariable() && !v->isBuiltinVariable())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Variable"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Variable (read only)"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Variable"));
	}
		
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_button_ok"), !v || (!v->isBuiltinVariable() && v->isUserVariable()));		

	if(v) {
		//fill in original parameters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_name")), v->name().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_value")), get_value_string(v->get()).c_str());
		//can only change name and value of user variable
//		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_name"), !v->isBuiltinVariable() && v->isUserVariable());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_name"), !v->isBuiltinVariable());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_value"), !v->isBuiltinVariable());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc")), v->title(false).c_str());
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_name"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_value"), TRUE);
	
		//fill in default values
		string v_name = "x";
		if(CALCULATOR->nameTaken("x")) {
			if(!CALCULATOR->nameTaken("y"))
				v_name = "y";
			else if(!CALCULATOR->nameTaken("z"))
				v_name = "z";
			else
				v_name = CALCULATOR->getName();
		}
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_name")), v_name.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_value")), get_value_string(mngr).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc")), "");		
	}
	if(mngr_) {
		//forced value
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_value"), false);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_value")), get_value_string(mngr_).c_str());
	}

run_variable_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_name")));
		string str2 = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_value")));
		remove_blank_ends(str);
		remove_blank_ends(str2);
		if(str.empty()) {
			//no name -- open dialog again
			show_message(_("Empty name field."), dialog);
			goto run_variable_edit_dialog;
		}
		if(str2.empty()) {
			//no value -- open dialog again
			show_message(_("Empty value field."), dialog);
			goto run_variable_edit_dialog;
		}

		//variable with the same name exists -- overwrite or open dialog again
		if(CALCULATOR->nameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_name"))), (void*) v) && !ask_question(_("A function or variable with the same name already exists.\nOverwrite function/variable?"), dialog)) {
			goto run_variable_edit_dialog;
		}
		if(!v) {
			//no need to create a new variable when a variable with the same name exists
			v = CALCULATOR->getVariable(str);
		}
		if(v) {
			//update existing variable
			if(!v->isBuiltinVariable()) {
				if(mngr_) v->set(mngr_);
				else {
					mngr_ = CALCULATOR->calculate(str2);
					v->set(mngr_);
					mngr_->unref();
				}
				v->setName(str);
			}
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc"))));
			v->setUserVariable(true);
		} else {
			//new variable
			if(mngr_) {
				//forced value
				v = CALCULATOR->addVariable(new Variable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category"))), str, mngr_, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc"))), true));
			} else {
				mngr_ = CALCULATOR->calculate(str2);
				v = CALCULATOR->addVariable(new Variable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category"))), str, mngr_, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc"))), true));
				mngr_->unref();
			}
		}
		//select the new variable
		selected_variable = v->name();
		selected_variable_category = v->category();
		if(selected_variable_category.empty())
			selected_variable_category = _("Uncategorized");
		update_vmenu();
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	add a new variable (from menu) with the value of result
*/
void add_as_variable()
{
	edit_variable(
			_("Temporary"),
			NULL,
			mngr,
			glade_xml_get_widget (glade_xml, "main_window"));
}

/*
	add a new variable (from menu)
*/
void new_variable(GtkMenuItem *w, gpointer user_data)
{
	edit_variable(
			_("Temporary"),
			NULL,
			NULL,
			glade_xml_get_widget (glade_xml, "main_window"));
}

/*
	insert one-argument function when button clicked
*/
void insertButtonFunction(gchar *text) {
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {
		//set selection as argument
		gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
		gchar *gstr2 = g_strdup_printf("%s(%s)", text, gstr);
		insert_text(gstr2);
		g_free(gstr);
		g_free(gstr2);
	} else {
		//one-argument functions do not need parenthesis
		gchar *gstr2 = g_strdup_printf("%s ", text);
//		gchar *gstr2 = g_strdup_printf("%s()", text);
		insert_text(gstr2);
//		gtk_editable_set_position(GTK_EDITABLE(expression), gtk_editable_get_position(GTK_EDITABLE(expression)) - 1);
		g_free(gstr2);
	}

}

/*
	Button clicked -- insert text (1,2,3,... +,-,...)
*/
void button_pressed(GtkButton *w, gpointer user_data) {
	insert_text((gchar*) user_data);
}

/*
	Button clicked -- insert corresponding function
*/
void button_function_pressed(GtkButton *w, gpointer user_data) {
	insertButtonFunction((gchar*) user_data);
}

/*
	Update angle menu
*/
void set_angle_item() {
	GtkWidget *mi = NULL;
	switch(CALCULATOR->angleMode()) {
	case RADIANS: {
			mi = glade_xml_get_widget (glade_xml, "menu_item_radians");
			break;
		}
	case GRADIANS: {
			mi = glade_xml_get_widget (glade_xml, "menu_item_gradians");
			break;
		}
	case DEGREES: {
			mi = glade_xml_get_widget (glade_xml, "menu_item_degrees");
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

/*
	Update angle radio buttons
*/
void set_angle_button() {
	GtkWidget *tb = NULL;
	switch(CALCULATOR->angleMode()) {
	case RADIANS: {
			tb = glade_xml_get_widget (glade_xml, "radiobutton_radians");
			break;
		}
	case GRADIANS: {
			tb = glade_xml_get_widget (glade_xml, "radiobutton_gradians");
			break;
		}
	case DEGREES: {
			tb = glade_xml_get_widget (glade_xml, "radiobutton_degrees");
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

/*
	variables, functions and units enabled/disabled from menu
*/
void set_clean_mode(GtkMenuItem *w, gpointer user_data) {
	gboolean b = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	CALCULATOR->setFunctionsEnabled(!b);
	CALCULATOR->setVariablesEnabled(!b);
	CALCULATOR->setUnitsEnabled(!b);
}

/*
	functions enabled/disabled from menu
*/
void set_functions_enabled(GtkMenuItem *w, gpointer user_data) {
	if(CALCULATOR->functionsEnabled()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(f_enable_item))), _("Enable functions"));
		CALCULATOR->setFunctionsEnabled(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(f_enable_item))), _("Disable functions"));
		CALCULATOR->setFunctionsEnabled(true);
	}
}

/*
	variables enabled/disabled from menu
*/
void set_variables_enabled(GtkMenuItem *w, gpointer user_data) {
	if(CALCULATOR->variablesEnabled()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(v_enable_item))), _("Enable variables"));
		CALCULATOR->setVariablesEnabled(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(v_enable_item))), _("Disable variables"));
		CALCULATOR->setVariablesEnabled(true);
	}
}

void set_donot_calcvars(GtkMenuItem *w, gpointer user_data) {
	if(CALCULATOR->donotCalculateVariables()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(v_calcvar_item))), _("Do not calculate variables"));
		CALCULATOR->setDonotCalculateVariables(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(v_calcvar_item))), _("Calculate variables"));
		CALCULATOR->setDonotCalculateVariables(true);
	}
	execute_expression();
}

/*
	unknown variables enabled/disabled from menu
*/
void set_unknownvariables_enabled(GtkMenuItem *w, gpointer user_data) {
	if(CALCULATOR->unknownVariablesEnabled()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(uv_enable_item))), _("Enable unknown variables"));
		CALCULATOR->setUnknownVariablesEnabled(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(uv_enable_item))), _("Disable unknown variables"));
		CALCULATOR->setUnknownVariablesEnabled(true);
	}
}

/*
	units enabled/disabled from menu
*/
void set_units_enabled(GtkMenuItem *w, gpointer user_data) {
	if(CALCULATOR->unitsEnabled()) {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(u_enable_item))), _("Enable units"));
		CALCULATOR->setUnitsEnabled(false);
	} else {
		gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(GTK_BIN(u_enable_item))), _("Disable units"));
		CALCULATOR->setUnitsEnabled(true);
	}
}

/*
	Open variable manager
*/
void manage_variables(GtkMenuItem *w, gpointer user_data) {
	if(!variables_window) {
		//if not previously created, do so now
		variables_window = create_variables_dialog();
		gtk_widget_show(variables_window);
	} else {
		gtk_widget_show(variables_window);
		gtk_window_present(GTK_WINDOW(variables_window));
	}
}

/*
	Open function manager
*/
void
manage_functions(GtkMenuItem *w, gpointer user_data)
{
	if(!functions_window) {
		//if not previously created, do so now
		functions_window = create_functions_dialog();
	} else {
		gtk_window_present(GTK_WINDOW(functions_window));
	}
	gtk_widget_show(functions_window);
}

/*
	Open unit manager
*/
void manage_units(GtkMenuItem *w, gpointer user_data) {
	if(!units_window) {
		//if not previously created, do so now
		units_window = create_units_dialog();
		gtk_widget_show(units_window);
	} else {
		gtk_widget_show(units_window);
		gtk_window_present(GTK_WINDOW(units_window));
	}
}
/*
	selected item in unit conversion menu in unit manager has changed -- update conversion
*/
void on_omToUnit_menu_activate(GtkMenuItem *item, gpointer user_data) {
	selected_to_unit = (Unit*) user_data;
	convert_in_wUnits();
}
/*
	update menu button when menu is deactivated
*/
void
on_menu_e_deactivate                   (GtkMenuShell       *menushell,
                                        gpointer         user_data) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "togglebutton_expression")), FALSE);
}

/*
	update menu button when menu is deactivated
*/
void
on_menu_r_deactivate                   (GtkMenuShell       *menushell,
                                        gpointer         user_data) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "togglebutton_result")), FALSE);
}

/*
	do the conversion in unit manager
*/
void convert_in_wUnits(int toFrom) {
	//units
	Unit *uFrom = get_selected_unit();
	Unit *uTo = get_selected_to_unit();
	if(uFrom && uTo) {
		//values
		const gchar *fromValue = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_from_val")));
		const gchar *toValue = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_to_val")));
		//determine conversion direction
		if(toFrom > 0) {
			Manager *mngr = CALCULATOR->convert(toValue, uTo, uFrom);
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_from_val")), mngr->print().c_str());
			mngr->unref();
		} else {
			Manager *mngr = CALCULATOR->convert(fromValue, uFrom, uTo);
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_to_val")), mngr->print().c_str());
			mngr->unref();
		}
	}
}

/*
	save definitions to ~/.qalculate/qalculate.cfg
	the hard work is done in the Calculator class
*/
void save_defs() {
	gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", NULL);
	mkdir(gstr, S_IRWXU);
	g_free(gstr);
	gchar *gstr2 = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate.cfg", NULL);
	if(!CALCULATOR->save(gstr2)) {
		GtkWidget *edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (glade_xml, "main_window")
				),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Couldn't write definitions to\n%s"),
				gstr2);
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
	g_free(gstr2);
}

/*
	save mode to file
*/
void save_mode() {
	save_preferences(true);
}

/*
	remember current mode
*/
void set_saved_mode() {
	saved_deci_mode = deci_mode;
	saved_decimals = decimals;
	saved_precision = CALCULATOR->getPrecision();
	saved_display_mode = display_mode;
	saved_number_base = number_base;
	saved_angle_unit = CALCULATOR->angleMode();
	saved_functions_enabled = CALCULATOR->functionsEnabled();
	saved_variables_enabled = CALCULATOR->variablesEnabled();
	saved_donot_calcvars = CALCULATOR->donotCalculateVariables();
	saved_unknownvariables_enabled = CALCULATOR->unknownVariablesEnabled();
	saved_units_enabled = CALCULATOR->unitsEnabled();
	saved_hyp_is_on = hyp_is_on;
	saved_fraction_is_on = fraction_is_on;
}

/*
	load preferences from ~/.qalculate/qalculate-gtk.cfg
*/
void load_preferences() {

	deci_mode = DECI_LEAST;
	decimals = 0;
	display_mode = MODE_NORMAL;
	number_base = BASE_DECI;
	save_mode_on_exit = true;
	save_defs_on_exit = true;
	hyp_is_on = false;
	fraction_is_on = false;
	show_more = false;
	show_buttons = false;
	use_short_units = true;
	use_unicode_signs = true;
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
				else if(svar == "save_definitions_on_exit")
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
					CALCULATOR->setPrecision(v);
				else if(svar == "display_mode")
					display_mode = v;
				else if(svar == "number_base")
					number_base = v;
				else if(svar == "angle_unit")
					CALCULATOR->angleMode(v);
				else if(svar == "hyp_is_on")
					hyp_is_on = v;
				else if(svar == "fraction_is_on")
					fraction_is_on = v;					
				else if(svar == "functions_enabled")
					CALCULATOR->setFunctionsEnabled(v);
				else if(svar == "variables_enabled")
					CALCULATOR->setVariablesEnabled(v);
				else if(svar == "donot_calculate_variables")
					CALCULATOR->setDonotCalculateVariables(v);					
				else if(svar == "unknownvariables_enabled")
					CALCULATOR->setUnknownVariablesEnabled(v);
				else if(svar == "units_enabled")
					CALCULATOR->setUnitsEnabled(v);
				else if(svar == "use_short_units")
					use_short_units = v;
				else if(svar == "use_unicode_signs")
					use_unicode_signs = v;					
			}
		}
	}
	//remember start mode for when we save preferences
	set_saved_mode();
}

/*
	save preferences to ~/.qalculate/qalculate-gtk.cfg
	set mode to true to save current calculator mode
*/
void save_preferences(bool mode)
{
	FILE *file = NULL;
	gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", NULL);
	mkdir(gstr, S_IRWXU);
	g_free(gstr);
	gchar *gstr2 = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate-gtk.cfg", NULL);
	file = fopen(gstr2, "w+");
	if(file == NULL) {
		GtkWidget *edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (glade_xml, "main_window")
				),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Couldn't write preferences to\n%s"),
				gstr2);
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
		g_free(gstr2);
		return;
	}
	g_free(gstr2);
	fprintf(file, "\n[General]\n");
	fprintf(file, "version=%s\n", VERSION);	
	fprintf(file, "save_mode_on_exit=%i\n", save_mode_on_exit);
	fprintf(file, "save_definitions_on_exit=%i\n", save_defs_on_exit);
	fprintf(file, "load_global_definitions=%i\n", load_global_defs);
	fprintf(file, "show_more=%i\n", GTK_WIDGET_VISIBLE(glade_xml_get_widget (glade_xml, "notebook")));
	fprintf(file, "show_buttons=%i\n", gtk_notebook_get_current_page(GTK_NOTEBOOK(glade_xml_get_widget (glade_xml, "notebook"))) == 1);
	fprintf(file, "use_short_units=%i\n", use_short_units);
	fprintf(file, "use_unicode_signs=%i\n", use_unicode_signs);	
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
	fprintf(file, "fraction_is_on=%i\n", saved_fraction_is_on);	
	fprintf(file, "functions_enabled=%i\n", saved_functions_enabled);
	fprintf(file, "variables_enabled=%i\n", saved_variables_enabled);
	fprintf(file, "donot_calculate_variables=%i\n", saved_donot_calcvars);	
	fprintf(file, "unknownvariables_enabled=%i\n", saved_unknownvariables_enabled);
	fprintf(file, "units_enabled=%i\n", saved_units_enabled);
	fclose(file);
}

/*
	tree text sort function
*/
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

/*
	tree sort function for number strings
*/
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
/*
	display preferences dialog
*/
void edit_preferences() {
	GtkWidget *dialog = create_preferences_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	save_preferences();
	gtk_widget_grab_focus(expression);
}

extern "C" {

void on_menu_item_quit_activate(GtkMenuItem *w, gpointer user_data) {
	gtk_main_quit();
}

/*
	change preferences
*/
void on_preferences_checkbutton_short_units_toggled(GtkToggleButton *w, gpointer user_data) {
	use_short_units = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_unicode_signs_toggled(GtkToggleButton *w, gpointer user_data) {
	use_unicode_signs = gtk_toggle_button_get_active(w);
	if(use_unicode_signs) {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sub")), SIGN_MINUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_add")), SIGN_PLUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_times")), SIGN_MULTIPLICATION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_divide")), SIGN_DIVISION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sqrt")), SIGN_SQRT);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_dot")), SIGN_MULTIDOT);	
	} else {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sub")), MINUS_STR);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_add")), PLUS_STR);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_times")), MULTIPLICATION_STR);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_divide")), DIVISION_STR);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sqrt")), "SQRT");	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_dot")), DOT_STR);	
	}
	setResult(gtk_label_get_text(GTK_LABEL(result)));
}
void on_preferences_checkbutton_save_defs_toggled(GtkToggleButton *w, gpointer user_data) {
	save_defs_on_exit = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_save_mode_toggled(GtkToggleButton *w, gpointer user_data) {
	save_mode_on_exit = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_load_defs_toggled(GtkToggleButton *w, gpointer user_data) {
	load_global_defs = gtk_toggle_button_get_active(w);
}
/*
	hide unit manager when "Close" clicked
*/
void on_units_button_close_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(units_window);
}

/*
	change conversion direction in unit manager on user request
*/
void on_units_toggle_button_from_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "units_toggle_button_to")), FALSE);
		convert_in_wUnits();
	}
}

/*
	convert button clicked
*/
void on_units_button_convert_clicked(GtkButton *button, gpointer user_data) {
	convert_in_wUnits();
}

/*
	change conversion direction in unit manager on user request
*/
void on_units_toggle_button_to_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "units_toggle_button_from")), FALSE);
		convert_in_wUnits();
	}
}

/*
	enter in conversion field
*/
void on_units_entry_from_val_activate(GtkEntry *entry, gpointer user_data) {
	convert_in_wUnits(0);
}
void on_units_entry_to_val_activate(GtkEntry *entry, gpointer user_data) {
	convert_in_wUnits(1);
}

gboolean on_units_entry_from_val_focus_out_event(GtkEntry *entry, GdkEventFocus *event, gpointer user_data) {
	convert_in_wUnits(0);
	return FALSE;
}
gboolean on_units_entry_to_val_focus_out_event(GtkEntry *entry, GdkEventFocus *event, gpointer user_data) {
	convert_in_wUnits(1);
	return FALSE;
}

/*
	do not actually destroy the unit manager, only hide it so we need not recreate it later
*/
gboolean on_units_dialog_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(units_window);
	return TRUE;
}
gboolean on_units_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(units_window);
	return TRUE;
}

/*
	"Close" clicked -- quit
*/
void on_button_close_clicked(GtkButton *w, gpointer user_data) {
	on_gcalc_exit(NULL, NULL, user_data);
}

/*
	angle mode radio buttons toggled
*/
void on_radiobutton_radians_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		CALCULATOR->angleMode(RADIANS);
		set_angle_item();
	}
}
void on_radiobutton_degrees_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		CALCULATOR->angleMode(DEGREES);
		set_angle_item();
	}
}
void on_radiobutton_gradians_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		CALCULATOR->angleMode(GRADIANS);
		set_angle_item();
	}
}
/*
	enter in expression entry does the same as clicking "Execute" button
*/
void
on_expression_activate                 (GtkEntry        *entry,
                                        gpointer         user_data) {
	execute_expression();
}


/*
	more/less button clicked
	hide/show history/buttons
*/
void
on_button_less_more_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget	* window = glade_xml_get_widget (glade_xml, "main_window");
	gint w = 0, h = 0, hh = 150;

	if(GTK_WIDGET_VISIBLE(glade_xml_get_widget (glade_xml, "notebook"))) {
		hh = glade_xml_get_widget (glade_xml, "notebook")->allocation.height;
		gtk_widget_hide(glade_xml_get_widget (glade_xml, "notebook"));
		gtk_button_set_label(button, _("More >>"));
		//the extra widgets increased the window height with 150 pixels, decrease again
		gtk_window_get_size(GTK_WINDOW(window), &w, &h);
		gtk_window_resize(GTK_WINDOW(window), w, h - hh);
	} else {
		gtk_widget_show(glade_xml_get_widget (glade_xml, "notebook"));
		gtk_button_set_label(button, _("<< Less"));
	}
	focus_keeping_selection();
}
/*
	"Execute" clicked
*/
void
on_button_execute_clicked                        (GtkButton       *button,
                                        gpointer         user_data) {
	execute_expression();
}
/*
	save preferences, mode and definitions and then quit
*/
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


/*
	DEL button clicked -- delete in expression entry
*/
void on_button_del_clicked(GtkButton *w, gpointer user_data) {
	gint position = gtk_editable_get_position(GTK_EDITABLE(expression));
	//delete selection or one character
	if(g_utf8_strlen(gtk_entry_get_text(GTK_ENTRY(expression)), -1) == position)
		gtk_editable_delete_text(GTK_EDITABLE(expression), position - 1, position);
	else
		gtk_editable_delete_text(GTK_EDITABLE(expression), position, position + 1);
	gtk_widget_grab_focus(expression);
	gtk_editable_select_region(GTK_EDITABLE(expression), position, position);
}

/*
	AC button clicked -- clear expression entry
*/
void on_button_ac_clicked(GtkButton *w, gpointer user_data) {
	gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
	gtk_widget_grab_focus(expression);
}

/*
	HYP button toggled -- enable/disable hyperbolic functions
*/
void on_button_hyp_toggled(GtkToggleButton *w, gpointer user_data) {
	hyp_is_on = gtk_toggle_button_get_active(w);
	focus_keeping_selection();
}

/*
	a b/c fraction button toggled -- enable/disable fractional display
*/
void on_button_fraction_toggled(GtkToggleButton *w, gpointer user_data) {
	fraction_is_on = gtk_toggle_button_get_active(w);
	setResult(gtk_label_get_text(GTK_LABEL(result)));	
	gtk_widget_grab_focus(expression);	
}

/*
	Tan/Sin/Cos button clicked -- insert corresponding function
*/
void on_button_tan_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("tanh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "button_hyp")), FALSE);
	} else
		insertButtonFunction("tan");
}
void on_button_sine_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("sinh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "button_hyp")), FALSE);
	} else
		insertButtonFunction("sin");
}
void on_button_cosine_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("cosh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "button_hyp")), FALSE);
	} else
		insertButtonFunction("cos");
}

/*
	STO button clicked -- store result
*/
void on_button_store_clicked(GtkButton *w, gpointer user_data) {
	add_as_variable();
}

/*
	expression menu button clicked -- popup or hide the menu
*/
void
on_togglebutton_expression_toggled                      (GtkToggleButton       *button,
                                        gpointer         user_data) {
	if(gtk_toggle_button_get_active(button)) {
		gtk_menu_popup(GTK_MENU(gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression")))), NULL, NULL, menu_e_posfunc, NULL, 0, 0);
	} else {
		gtk_menu_popdown(GTK_MENU(gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression")))));
	}
}

/*
	result menu button clicked -- popup or hide the menu
*/
void
on_togglebutton_result_toggled                      (GtkToggleButton       *button,
                                        gpointer         user_data) {
	if(gtk_toggle_button_get_active(button)) {
		gtk_menu_popup(GTK_MENU(gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result")))), NULL, NULL, menu_r_posfunc, NULL, 0, 0);
	} else {
		gtk_menu_popdown(GTK_MENU(gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result")))));
	}
}

/*
	clear the displayed result when expression changes
*/
void on_expression_changed(GtkEditable *w, gpointer user_data) {
	if(strlen(gtk_label_get_label(GTK_LABEL(result))) < 1) return;
	gtk_widget_set_size_request(result, -1, result->allocation.height);	
	gtk_label_set_text(GTK_LABEL(result), "");
	gtk_label_set_text(GTK_LABEL(label_equals), "");	
}

/*
	Button clicked -- insert text (1,2,3,... +,-,...)
*/
void on_button_zero_clicked(GtkButton *w, gpointer user_data) {
	insert_text("0");
}
void on_button_one_clicked(GtkButton *w, gpointer user_data) {
	insert_text("1");
}
void on_button_two_clicked(GtkButton *w, gpointer user_data) {
	insert_text("2");
}
void on_button_three_clicked(GtkButton *w, gpointer user_data) {
	insert_text("3");
}
void on_button_four_clicked(GtkButton *w, gpointer user_data) {
	insert_text("4");
}
void on_button_five_clicked(GtkButton *w, gpointer user_data) {
	insert_text("5");
}
void on_button_six_clicked(GtkButton *w, gpointer user_data) {
	insert_text("6");
}
void on_button_seven_clicked(GtkButton *w, gpointer user_data) {
	insert_text("7");
}
void on_button_eight_clicked(GtkButton *w, gpointer user_data) {
	insert_text("8");
}
void on_button_nine_clicked(GtkButton *w, gpointer user_data) {
	insert_text("9");
}
void on_button_dot_clicked(GtkButton *w, gpointer user_data) {
	insert_text(DOT_STR);
}
void on_button_brace_open_clicked(GtkButton *w, gpointer user_data) {
	insert_text(LEFT_BRACKET_STR);
}
void on_button_brace_close_clicked(GtkButton *w, gpointer user_data) {
	insert_text(RIGHT_BRACKET_STR);
}
void on_button_times_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	if(use_unicode_signs) insert_text(SIGN_MULTIDOT);
	else insert_text(MULTIPLICATION_STR);
}
void on_button_add_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	if(use_unicode_signs) insert_text(SIGN_PLUS);
	else insert_text(PLUS_STR);
}
void on_button_sub_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	if(use_unicode_signs) insert_text(SIGN_MINUS);
	else insert_text(MINUS_STR);
}
void on_button_divide_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
//	if(use_unicode_signs) insert_text(SIGN_DIVISION);
//	else 
	insert_text(DIVISION_STR);
}
void on_button_ans_clicked(GtkButton *w, gpointer user_data) {
	insert_text("Ans");
}
void on_button_exp_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text(EXP_STR);
}
void on_button_xy_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text(POWER_STR);
}
void on_button_square_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text(POWER_STR);
	insert_text("2");
}

/*
	Button clicked -- insert corresponding function
*/
void on_button_sqrt_clicked(GtkButton *w, gpointer user_data) {
	if(use_unicode_signs) insert_text(SIGN_SQRT);
	else insertButtonFunction("sqrt");
}
void on_button_log_clicked(GtkButton *w, gpointer user_data) {
	insertButtonFunction("log");
}
void on_button_ln_clicked(GtkButton *w, gpointer user_data) {
	insertButtonFunction("ln");
}

void on_menu_item_addition_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(PLUS_STR);
}
void on_menu_item_subtraction_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(MINUS_STR);
}
void on_menu_item_multiplication_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(MULTIPLICATION_STR);
}
void on_menu_item_division_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(DIVISION_STR);
}
void on_menu_item_power_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(POWER_STR);
}
void on_menu_item_exponent_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(EXP_STR);
}
void on_menu_item_save_defs_activate(GtkMenuItem *w, gpointer user_data) {
	save_defs();
}
void on_menu_item_save_mode_activate(GtkMenuItem *w, gpointer user_data) {
	save_mode();
}
void on_menu_item_edit_prefs_activate(GtkMenuItem *w, gpointer user_data) {
	edit_preferences();
}
void on_menu_item_degrees_activate(GtkMenuItem *w, gpointer user_data) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		gint i = DEGREES;
		CALCULATOR->angleMode(i);
		set_angle_button();
	}
}
void on_menu_item_radians_activate(GtkMenuItem *w, gpointer user_data) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		gint i = RADIANS;
		CALCULATOR->angleMode(i);
		set_angle_button();
	}
}
void on_menu_item_gradians_activate(GtkMenuItem *w, gpointer user_data) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		gint i = GRADIANS;
		CALCULATOR->angleMode(i);
		set_angle_button();
	}
}
void on_menu_item_binary_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = BASE_BIN;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_octal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = BASE_OCTAL;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_decimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = BASE_DECI;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_hexadecimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = BASE_HEX;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_convert_number_bases_activate(GtkMenuItem *w, gpointer user_data) {
	changing_in_nbases_dialog = false;
	create_nbases_dialog();
}
void on_menu_item_display_normal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = MODE_NORMAL;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = MODE_SCIENTIFIC;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_non_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = MODE_DECIMALS;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = MODE_PREFIXES;
	setResult(gtk_label_get_text(GTK_LABEL(result)));
	gtk_widget_grab_focus(expression);
}
void on_menu_item_save_activate(GtkMenuItem *w, gpointer user_data) {
	add_as_variable();
}
void on_menu_item_precision_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "precision_dialog");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "precision_dialog_spinbutton_precision")), PRECISION);	
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}
void on_menu_item_decimals_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "decimals_dialog");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	if(deci_mode == DECI_LEAST)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "decimals_dialog_radiobutton_least")), TRUE);
	else if(deci_mode == DECI_FIXED)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "decimals_dialog_radiobutton_always")), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_decimals")), decimals);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	check if entered unit name is valid, if not modify
*/
void on_unit_edit_entry_name_changed(GtkEditable *editable, gpointer user_data) {
	if(!CALCULATOR->unitNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		gulong sh = get_signal_handler(G_OBJECT(editable));
		block_signal(G_OBJECT(editable), sh);
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidUnitName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		unblock_signal(G_OBJECT(editable), sh);
	}
}
/*
	selected unit type in edit/new unit dialog has changed
*/
void on_unit_edit_optionmenu_class_changed(GtkOptionMenu *om, gpointer user_data)
{

	gchar *composite[7] = {
		"unit_edit_label_plural",
		"unit_edit_entry_plural",
		"unit_edit_label_short",
		"unit_edit_entry_short",
		"unit_edit_label_singular",
		"unit_edit_entry_singular",
		NULL
	};

	gchar *composite2[3] = {
		"unit_edit_label_internal",
		"unit_edit_entry_internal",
		NULL
	};

	gchar *alias[9] = {
		"unit_edit_label_exp",
		"unit_edit_spinbutton_exp",
		"unit_edit_label_relation",
		"unit_edit_entry_relation",
		"unit_edit_label_reversed",
		"unit_edit_entry_reversed",
		"unit_edit_label_info1",
		"unit_edit_label_info2",
		NULL
	};

	gchar *base[4] = {
		"unit_edit_label_relation_title",
		"unit_edit_label_base",
		"unit_edit_entry_base",
		NULL
	};

	gchar **pointer;

	/* making the non-composite widgets (un)sensitive */
	for (pointer = composite; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (glade_xml, *pointer),
				(gtk_option_menu_get_history(om) == COMPOSITE_UNIT) ? FALSE : TRUE
				);
	}

	/* making the composite widgets (un)sensitive */
	for (pointer = composite2; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (glade_xml, *pointer),
				(gtk_option_menu_get_history(om) == COMPOSITE_UNIT) ? TRUE : FALSE
				);
	}

	/* making the alias widgets (un)sensitive */
	for (pointer = alias; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (glade_xml, *pointer),
				(gtk_option_menu_get_history(om) == ALIAS_UNIT) ? TRUE : FALSE
				);
	}

	/* making the non-base widgets (un)sensitive */
	for (pointer = base; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (glade_xml, *pointer),
				(gtk_option_menu_get_history(om) == BASE_UNIT) ? FALSE : TRUE
				);
	}

	
	
}
/*
	"New" button clicked in unit manager -- open new unit dialog
*/
void on_units_button_new_clicked(GtkButton *button, gpointer user_data) {
	if(selected_unit_category == _("All") || selected_unit_category == _("Uncategorized")) {
		edit_unit("", NULL, units_window);
	} else {
		//fill in category field with selected category
		if(selected_unit_category[0] == '/') {
			string str = selected_unit_category.substr(1, selected_unit_category.length() - 1);
			edit_unit(str.c_str(), NULL, units_window);
		} else {
			edit_unit(selected_unit_category.c_str(), NULL, units_window);
		}
	}
}

/*
	"Edit" button clicked in unit manager -- open edit unit dialog for selected unit
*/
void on_units_button_edit_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		edit_unit("", u, units_window);
	}
}

/*
	"Insert" button clicked in unit manager -- insert selected unit in expression entry
*/
void on_units_button_insert_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		gchar *gstr;
		if(use_short_units)
			gstr = g_strdup(u->shortName(true).c_str());
		else
			gstr = g_strdup(u->name().c_str());
		insert_text(gstr);
		g_free(gstr);
	}
}

/*
	"Convert" button clicked in unit manager -- convert result to selected unit
*/
void on_units_button_convert_to_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		CALCULATOR->convert(mngr, u);
		setResult(gtk_label_get_text(GTK_LABEL(result)));
		gtk_widget_grab_focus(expression);
	}
}

/*
	deletion of unit requested
*/
void on_units_button_delete_clicked(GtkButton *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Unit *u = get_selected_unit();
	if(u) {
		if(u->isUsedByOtherUnits()) {
			//do not delete units that are used by other units
			show_message(_("Cannot delete unit as it is needed by other units."), units_window);
			return;
		}
		//ensure that all references to the unit is removed in Calculator
		CALCULATOR->delUnit(u);
		delete u;
		//update menus and trees
		if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits)), &model, &iter)) {
			//reselect selected unit category
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

/*
	"New" button clicked in variable manager -- open new variable dialog
*/
void on_variables_button_new_clicked(GtkButton *button, gpointer user_data) {
	if(selected_variable_category == _("All") || selected_variable_category == _("Uncategorized")) {
		edit_variable("", NULL, NULL, variables_window);
	} else {
		//fill in category field with selected category
		edit_variable(selected_variable_category.c_str(), NULL, NULL, variables_window);
	}
}

/*
	"Edit" button clicked in variable manager -- open edit dialog for selected variable
*/
void on_variables_button_edit_clicked(GtkButton *button, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v) {
		edit_variable("", v, NULL, variables_window);
	}
}

/*
	"Insert" button clicked in variable manager -- insert variable name in expression entry
*/
void on_variables_button_insert_clicked(GtkButton *button, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v) {
		gchar *gstr = g_strdup(v->name().c_str());
		insert_text(gstr);
		g_free(gstr);
	}
}

/*
	"Delete" button clicked in variable manager -- deletion of selected variable requested
*/
void on_variables_button_delete_clicked(GtkButton *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Variable *v = get_selected_variable();
	if(v && !v->isBuiltinVariable()) {
		//ensure that all references are removed in Calculator
		CALCULATOR->delVariable(v);
		delete v;
		//update menus and trees
		if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &model, &iter)) {
			//reselect selected variable category
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

/*
	"Close" button clicked in variable manager -- hide
*/
void on_variables_button_close_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(variables_window);
}

/*
	do not actually destroy the variable manager, only hide it so we need not recreate it later
*/
gboolean on_variables_dialog_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(variables_window);
	return TRUE;
}
gboolean on_variables_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(variables_window);
	return TRUE;
}


/*
	"New" button clicked in function manager -- open new function dialog
*/
void on_functions_button_new_clicked(GtkButton *button, gpointer user_data) {
	if(selected_function_category == _("All") || selected_function_category == _("Uncategorized")) {
		edit_function("", NULL, functions_window);
	} else {
		//fill in category field with selected category
		edit_function(selected_function_category.c_str(), NULL, functions_window);
	}
}

/*
	"Edit" button clicked in function manager -- open edit function dialog for selected function
*/
void on_functions_button_edit_clicked(GtkButton *button, gpointer user_data) {
	Function *f = get_selected_function();
	if(f) {
		edit_function("", f, functions_window);
	}
}

/*
	"Insert" button clicked in function manager -- open dialog for insertion of function in expression entry
*/
void on_functions_button_insert_clicked(GtkButton *button, gpointer user_data) {
	insert_function(get_selected_function(), functions_window);
}

/*
	"Delete" button clicked in function manager -- deletion of selected function requested
*/
void on_functions_button_delete_clicked(GtkButton *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Function *f = get_selected_function();
	if(f && !f->isBuiltinFunction()) {
		//ensure removal of all references in Calculator
		CALCULATOR->delFunction(f);
		delete f;
		//update menus and trees
		if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &model, &iter)) {
			//reselected selected function category
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

/*
	"Close" button clicked in function manager -- hide
*/
void on_functions_button_close_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(functions_window);
}

/*
	do not actually destroy the function manager, only hide it so we need not recreate it later
*/
gboolean on_functions_dialog_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(functions_window);
	return TRUE;
}
gboolean on_functions_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(functions_window);
	return TRUE;
}
/*
	check if entered function name is valid, if not modify
*/
void on_function_edit_entry_name_changed(GtkEditable *editable, gpointer user_data) {
	if(!CALCULATOR->functionNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		gulong sh = get_signal_handler(G_OBJECT(editable));
		block_signal(G_OBJECT(editable), sh);
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidFunctionName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		unblock_signal(G_OBJECT(editable), sh);
	}
}
/*
	check if entered variable name is valid, if not modify
*/
void on_variable_edit_entry_name_changed(GtkEditable *editable, gpointer user_data) {
	if(!CALCULATOR->variableNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		gulong sh = get_signal_handler(G_OBJECT(editable));
		block_signal(G_OBJECT(editable), sh);
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidVariableName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		unblock_signal(G_OBJECT(editable), sh);
	}
}

void update_nbases_entries(Manager *value, NumberFormat nrformat) {
	GtkWidget *w_dec, *w_bin, *w_oct, *w_hex;
	w_dec = glade_xml_get_widget (glade_xml, "nbases_entry_decimal");
	w_bin = glade_xml_get_widget (glade_xml, "nbases_entry_binary");	
	w_oct = glade_xml_get_widget (glade_xml, "nbases_entry_octal");	
	w_hex = glade_xml_get_widget (glade_xml, "nbases_entry_hexadecimal");	
	gulong s_dec = get_signal_handler(w_dec);
	gulong s_bin = get_signal_handler(w_bin);	
	gulong s_oct = get_signal_handler(w_oct);	
	gulong s_hex = get_signal_handler(w_hex);	
	block_signal(w_dec, s_dec);
	block_signal(w_bin, s_bin);	
	block_signal(w_oct, s_oct);	
	block_signal(w_hex, s_hex);	
	if(nrformat != NUMBER_FORMAT_NORMAL) gtk_entry_set_text(GTK_ENTRY(w_dec), value->print().c_str());
	if(nrformat != NUMBER_FORMAT_BIN) gtk_entry_set_text(GTK_ENTRY(w_bin), value->print(NUMBER_FORMAT_BIN).c_str());	
	if(nrformat != NUMBER_FORMAT_OCTAL) gtk_entry_set_text(GTK_ENTRY(w_oct), value->print(NUMBER_FORMAT_OCTAL).c_str());	
	if(nrformat != NUMBER_FORMAT_HEX) gtk_entry_set_text(GTK_ENTRY(w_hex), value->print(NUMBER_FORMAT_HEX).c_str());	
	unblock_signal(w_dec, s_dec);
	unblock_signal(w_bin, s_bin);	
	unblock_signal(w_oct, s_oct);	
	unblock_signal(w_hex, s_hex);		
}
void on_nbases_button_close_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(glade_xml_get_widget (glade_xml, "nbases_dialog"));
}
void on_nbases_entry_decimal_changed(GtkEditable *editable, gpointer user_data) {	
	if(changing_in_nbases_dialog) return;
	Manager *value;
	Function *func = CALCULATOR->getFunction("round");	
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;
	changing_in_nbases_dialog = true;	
	if(func) value = func->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	else value = CALCULATOR->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	update_nbases_entries(value, NUMBER_FORMAT_NORMAL);
	value->unref();
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_binary_changed(GtkEditable *editable, gpointer user_data) {
	if(changing_in_nbases_dialog) return;
	Manager *value;
	Function *func = CALCULATOR->getFunction("BIN");	
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;
	changing_in_nbases_dialog = true;	
	if(func) value = func->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	else value = CALCULATOR->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	update_nbases_entries(value, NUMBER_FORMAT_BIN);
	value->unref();
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_octal_changed(GtkEditable *editable, gpointer user_data) {
	if(changing_in_nbases_dialog) return;
	Manager *value;
	Function *func = CALCULATOR->getFunction("OCT");	
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;	
	changing_in_nbases_dialog = true;	
	if(func) value = func->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	else value = CALCULATOR->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	update_nbases_entries(value, NUMBER_FORMAT_OCTAL);
	value->unref();
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_hexadecimal_changed(GtkEditable *editable, gpointer user_data) {
	if(changing_in_nbases_dialog) return;
	Manager *value;
	Function *func = CALCULATOR->getFunction("HEX");	
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	changing_in_nbases_dialog = true;	
	if(str.empty()) return;	
	if(func) value = func->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	else value = CALCULATOR->calculate(gtk_entry_get_text(GTK_ENTRY(editable)));
	update_nbases_entries(value, NUMBER_FORMAT_HEX);
	value->unref();
	changing_in_nbases_dialog = false;	
}

void on_button_functions_clicked(GtkButton *button, gpointer user_data) {
	manage_functions(NULL, user_data);
}
void on_button_variables_clicked(GtkButton *button, gpointer user_data) {
	manage_variables(NULL, user_data);
}
void on_button_units_clicked(GtkButton *button, gpointer user_data) {
	manage_units(NULL, user_data);
}
void on_button_convert_clicked(GtkButton *button, gpointer user_data) {
	convert_to_custom_unit(NULL, user_data);
}

void on_menu_item_about_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "about_dialog");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	precision has changed in precision dialog
*/
void on_precision_dialog_spinbutton_precision_value_changed(GtkSpinButton *w, gpointer user_data) {
	CALCULATOR->setPrecision(gtk_spin_button_get_value_as_int(w));
	setResult(gtk_label_get_text(GTK_LABEL(result)));
}

/*
	decimals or decimal mode has changed in decimals dialog
*/
void on_decimals_dialog_spinbutton_decimals_value_changed(GtkSpinButton *w, gpointer user_data) {
	decimals = gtk_spin_button_get_value_as_int(w);
	setResult(gtk_label_get_text(GTK_LABEL(result)));
}
void on_decimals_dialog_radiobutton_least_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		deci_mode = DECI_LEAST;
		setResult(gtk_label_get_text(GTK_LABEL(result)));
	}
}
void on_decimals_dialog_radiobutton_always_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		deci_mode = DECI_FIXED;
		setResult(gtk_label_get_text(GTK_LABEL(result)));
	}
}

gboolean on_expression_key_press_event(GtkWidget *w, GdkEventKey *event, gpointer user_data) {
	switch(event->keyval) {
		case GDK_dead_circumflex: {
			gint end = 0;
			wrap_expression_selection();
			end = gtk_editable_get_position(GTK_EDITABLE(expression));
			gtk_editable_insert_text(GTK_EDITABLE(expression), POWER_STR, strlen(POWER_STR), &end);				
			gtk_editable_set_position(GTK_EDITABLE(expression), end);							
			return TRUE;
		}
		case GDK_KP_Divide: {}		
		case GDK_KP_Multiply: {}				
		case GDK_KP_Add: {}				
		case GDK_KP_Subtract: {}				
		case GDK_slash: {}		
		case GDK_asterisk: {}				
		case GDK_plus: {}				
		case GDK_minus: {
			wrap_expression_selection();
			break;
		}						
	}
	if(use_unicode_signs) {
		gint pos = gtk_editable_get_position(GTK_EDITABLE(expression));
		switch(event->keyval) {
			case GDK_KP_Divide: {}		
			case GDK_slash: {
//				gtk_editable_insert_text(GTK_EDITABLE(expression), " " SIGN_DIVISION " ", strlen(SIGN_DIVISION) + 2, &pos);
//				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
//				return TRUE;			
				break;
			}					
			case GDK_KP_Multiply: {}			
			case GDK_asterisk: {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_MULTIDOT, -1, &pos);
				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
				return TRUE;			
			}								
			case GDK_KP_Add: {}				
			case GDK_plus: {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_PLUS, -1, &pos);
				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
				return TRUE;			
			}			
			case GDK_KP_Subtract: {}								
			case GDK_minus: {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_MINUS, -1, &pos);
				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
				return TRUE;
			}						
		}	
	}
	return FALSE;
}

}
