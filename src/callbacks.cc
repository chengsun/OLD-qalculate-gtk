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
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <glade/glade.h>

#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "main.h"

bool changing_in_nbases_dialog;

extern GladeXML *glade_xml;

extern GtkWidget *expression;
extern GtkWidget *f_menu, *v_menu, *u_menu, *u_menu2;
extern Variable *vans, *vAns;
extern GtkWidget *tFunctionArguments;
extern GtkListStore *tFunctionArguments_store;
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
extern Function *selected_function;
Function *edited_function;
Argument *selected_argument;
Argument *edited_argument;
extern GtkWidget *variables_window;
extern string selected_variable_category;
extern Variable *selected_variable;
extern GtkWidget *units_window;
extern string selected_unit_category;
extern Unit *selected_unit;
extern Unit *selected_to_unit;
bool saved_use_min_deci, saved_use_max_deci;
int saved_min_deci, saved_max_deci, saved_precision, saved_display_mode, saved_number_base, saved_angle_unit;
bool use_short_units, use_unicode_signs, use_prefixes;
bool saved_functions_enabled, saved_variables_enabled, saved_unknownvariables_enabled, saved_units_enabled, saved_donot_calcvars, saved_use_prefixes;
bool saved_indicate_infinite_series, indicate_infinite_series;
bool save_mode_on_exit;
bool save_defs_on_exit;
bool saved_in_rpn_mode;
int fractional_mode, saved_fractional_mode;
bool use_custom_font;
string custom_font;
bool saved_always_exact;
bool hyp_is_on, saved_hyp_is_on;
bool use_min_deci, use_max_deci;
int min_deci, max_deci, display_mode, number_base;
bool show_more, show_buttons;
extern bool load_global_defs;
extern GtkWidget *omToUnit_menu;
bool block_unit_convert;
extern Manager *mngr;
extern string result_text;
extern GtkWidget *resultview;
extern GdkPixmap *pixmap_result;
vector<vector<GtkWidget*> > element_entries;
bool b_busy;
pthread_t view_thread;
Prefix *tmp_prefix;
bool tmp_in_exact;
GdkPixmap *tmp_pixmap;
bool expression_has_changed;

#define TEXT_TAGS			"<span size=\"xx-large\">"
#define TEXT_TAGS_END			"</span>"
#define TEXT_TAGS_SMALL			"<span size=\"large\">"
#define TEXT_TAGS_SMALL_END		"</span>"

#define MARKUP_STRING(str, text)	if(in_power) {str = TEXT_TAGS_SMALL;} else {str = TEXT_TAGS;} str += text; if(in_power) {str += TEXT_TAGS_SMALL_END;} else {str += TEXT_TAGS_END;}			
#define CALCULATE_SPACE_W		gint space_w, space_h; PangoLayout *layout_space = gtk_widget_create_pango_layout(resultview, NULL); if(in_power) {pango_layout_set_markup(layout_space, TEXT_TAGS_SMALL " " TEXT_TAGS_SMALL_END, -1);} else {pango_layout_set_markup(layout_space, TEXT_TAGS " " TEXT_TAGS_END, -1);} pango_layout_get_pixel_size(layout_space, &space_w, &space_h); g_object_unref(layout_space);


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
vector<void*> ia_units, ia_variables, ia_functions;	

void wrap_expression_selection() {
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {			
		gtk_editable_select_region(GTK_EDITABLE(expression), end, end);
		gtk_editable_insert_text(GTK_EDITABLE(expression), LEFT_BRACKET_STR.c_str(), 1, &start);
		end++;
		gtk_editable_insert_text(GTK_EDITABLE(expression), RIGHT_BRACKET_STR.c_str(), 1, &end);				
		gtk_editable_set_position(GTK_EDITABLE(expression), end);				
	}
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
	if(b_busy || !CALCULATOR->error())
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

Function *get_selected_function() {
	return selected_function;
}

Function *get_edited_function() {
	return edited_function;
}

Argument *get_edited_argument() {
	return edited_argument;
}
Argument *get_selected_argument() {
	return selected_argument;
}

Variable *get_selected_variable() {
	return selected_variable;
}

Unit *get_selected_unit() {
	return selected_unit;
}

Unit *get_selected_to_unit() {
	return selected_to_unit;
}

void generate_units_tree_struct() {
	tree_struct *p_cat;
	int i2; bool b;
	bool no_cat = false;	
	string str;
	unit_cats.clear();
	uc_units.clear();
	ia_units.clear();
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
		} else if(!CALCULATOR->units[i]->isActive()) {
			b = false;
			for(int i3 = 0; i3 < ia_units.size(); i3++) {
				u = (Unit*) ia_units[i3];
				if(CALCULATOR->units[i]->title() < u->title()) {
					b = true;
					ia_units.insert(ia_units.begin() + i3, (void*) CALCULATOR->units[i]);
					break;
				}
			}
			if(!b) ia_units.push_back((void*) CALCULATOR->units[i]);						
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
	ia_variables.clear();
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
		} else if(!CALCULATOR->variables[i]->isActive()) {
			//deactivated variable
			b = false;
			for(int i3 = 0; i3 < ia_variables.size(); i3++) {
				v = (Variable*) ia_variables[i3];
				if(CALCULATOR->variables[i]->title() < v->title()) {
					b = true;
					ia_variables.insert(ia_variables.begin() + i3, (void*) CALCULATOR->variables[i]);
					break;
				}
			}
			if(!b) ia_variables.push_back((void*) CALCULATOR->variables[i]);											
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
	ia_functions.clear();
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
		} else if(!CALCULATOR->functions[i]->isActive()) {
			//deactivated function
			b = false;
			for(int i3 = 0; i3 < ia_functions.size(); i3++) {
				f = (Function*) ia_functions[i3];
				if(CALCULATOR->functions[i]->title() < f->title()) {
					b = true;
					ia_functions.insert(ia_functions.begin() + i3, (void*) CALCULATOR->functions[i]);
					break;
				}
			}
			if(!b) ia_functions.push_back((void*) CALCULATOR->functions[i]);								
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
void update_functions_tree() {
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tFunctionCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tFunctionCategories_selection_changed, NULL);
	gtk_tree_store_clear(tFunctionCategories_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tFunctionCategories_selection_changed, NULL);
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
	if(!ia_functions.empty()) {
		//add "Inactive" category if there are inactive functions
		gtk_tree_store_append(tFunctionCategories_store, &iter, NULL);
		EXPAND_TO_ITER(model, tFunctionCategories, iter)
		gtk_tree_store_set(tFunctionCategories_store, &iter, 0, _("Inactive"), 1, _("Inactive"), -1);
		if(selected_function_category == _("Inactive")) {
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
	gtk_list_store_set(tFunctions_store, &iter2, 0, f->title(true).c_str(), 1, (gpointer) f, -1);
	if(f == selected_function) {
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &iter2);
	}		
}

/*
	generate the function tree in manage functions dialog when category selection has changed
*/
void on_tFunctionCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	bool no_cat = false, b_all = false, b_inactive = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tFunctions_selection_changed, NULL);
	gtk_list_store_clear(tFunctions_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tFunctions_selection_changed, NULL);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_insert"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_delete"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_deactivate"), FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_function_category = gstr;
		if(selected_function_category == _("All")) {
			b_all = true;
		} else if(selected_function_category == _("Uncategorized")) {
			no_cat = true;
		} else if(selected_function_category == _("Inactive")) {
			b_inactive = true;
		}
		if(!b_all && !no_cat && !b_inactive && selected_function_category[0] == '/') {
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
				if((b_inactive && !CALCULATOR->functions[i]->isActive()) || (CALCULATOR->functions[i]->isActive() && (b_all || (no_cat && CALCULATOR->functions[i]->category().empty()) || (!b_inactive && CALCULATOR->functions[i]->category() == selected_function_category)))) {
					setFunctionTreeItem(iter2, CALCULATOR->functions[i]);
				}
			}
		}
		if(!selected_function || !gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &model2, &iter2)) {
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
		Function *f;
		gtk_tree_model_get(model, &iter, 1, &f, -1);
		//remember the new selection
		selected_function = f;
		for(int i = 0; i < CALCULATOR->functions.size(); i++) {
			if(CALCULATOR->functions[i] == selected_function) {
				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "functions_textview_description")));
				gtk_text_buffer_set_text(buffer, "", -1);
				GtkTextIter iter;
				f = CALCULATOR->functions[i];
				Argument *arg;
				Argument default_arg;
				string str, str2;
				str += f->name();
				gtk_text_buffer_get_end_iter(buffer, &iter);
				gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "bold", "italic", NULL);
				str = "";
				int iargs = f->maxargs();
				if(iargs < 0) {
					iargs = f->minargs() + 1;
				}
				str += LEFT_BRACKET_STR;				
				if(iargs != 0) {
					for(int i2 = 1; i2 <= iargs; i2++) {	
						if(i2 > f->minargs()) {
							str += "[";
						}
						if(i2 > 1) {
							str += CALCULATOR->getComma();
							str += " ";
						}
						arg = f->getArgumentDefinition(i2);
						if(arg && !arg->name().empty()) {
							str2 = arg->name();
						} else {
							str2 = _("argument");
							str2 += " ";
							str2 += i2s(i2);
						}
						str += str2;
						if(i2 > f->minargs()) {
							str += "]";
						}
					}
					if(f->maxargs() < 0) {
						str += CALCULATOR->getComma();
						str += " ...";
					}
				}
				str += RIGHT_BRACKET_STR;				
				gtk_text_buffer_get_end_iter(buffer, &iter);
				gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "italic", NULL);
				str = "";
				str += "\n";
				if(!f->description().empty()) {
					str += "\n";
					str += f->description();
					str += "\n";
				}
				gtk_text_buffer_get_end_iter(buffer, &iter);
				gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
				if(iargs) {
					str = "\n";
					str += _("Arguments");
					str += "\n";
					gtk_text_buffer_get_end_iter(buffer, &iter);
					gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "bold", NULL);
					for(int i2 = 1; i2 <= iargs; i2++) {	
						arg = f->getArgumentDefinition(i2);
						if(arg && !arg->name().empty()) {
							str = arg->name();
						} else {
							str = i2s(i2);	
						}
						str += ": ";
						if(arg) {
							str2 = arg->printlong();
						} else {
							str2 = default_arg.printlong();
						}
						if(i2 > f->minargs()) {
							str2 += " (";
							str2 += _("optional");
							str2 += ")";
						}
						str2 += "\n";
						gtk_text_buffer_get_end_iter(buffer, &iter);
						gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
						gtk_text_buffer_get_end_iter(buffer, &iter);
						gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str2.c_str(), -1, "italic", NULL);
					}
				}
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_edit"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_deactivate"), TRUE);
				if(CALCULATOR->functions[i]->isActive()) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "functions_buttonlabel_deactivate")), _("Deactivate"));
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "functions_buttonlabel_deactivate")), _("Activate"));
				}
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_insert"), CALCULATOR->functions[i]->isActive());
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_delete"), CALCULATOR->functions[i]->isLocal());
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_insert"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_delete"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "functions_button_deactivate"), FALSE);
		selected_function = NULL;
	}
}

/*
	generate the variable categories tree in manage variables dialog
*/
void update_variables_tree() {
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tVariableCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tVariableCategories_selection_changed, NULL);
	gtk_tree_store_clear(tVariableCategories_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tVariableCategories_selection_changed, NULL);
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
	if(!ia_variables.empty()) {
		//add "Inactive" category if there are inactive variables
		gtk_tree_store_append(tVariableCategories_store, &iter, NULL);
		EXPAND_TO_ITER(model, tVariableCategories, iter)
		gtk_tree_store_set(tVariableCategories_store, &iter, 0, _("Inactive"), 1, _("Inactive"), -1);
		if(selected_variable_category == _("Inactive")) {
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
	string value = "";
	if(v->isExpression()) {
		value = v->expression();
		if(value.length() > 13) {
			value = value.substr(0, 10);
			value += "...";
		}
	} else {
		Manager *v_mngr = v->get();
		if(v_mngr->isMatrix()) {
			if(v_mngr->matrix()->isVector()) {
				value = "vector";
			} else {
				value = "matrix";
			}
		} else {
			value = CALCULATOR->printManagerTimeOut(v_mngr, 30000, NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY);
			//value = v_mngr->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY);
		}
	}
	gtk_list_store_set(tVariables_store, &iter2, 0, v->title(true).c_str(), 1, value.c_str(), 2, (gpointer) v, -1);
	if(v == selected_variable) {
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &iter2);
	}
}

/*
	generate the variable tree in manage variables dialog when category selection has changed
*/
void on_tVariableCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	bool no_cat = false, b_all = false, b_inactive = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tVariables_selection_changed, NULL);
	gtk_list_store_clear(tVariables_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tVariables_selection_changed, NULL);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_insert"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_delete"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_deactivate"), FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_variable_category = gstr;		
		if(selected_variable_category == _("All")) {
			b_all = true;
		} else if(selected_variable_category == _("Uncategorized")) {
			no_cat = true;
		} else if(selected_variable_category == _("Inactive")) {
			b_inactive = true;
		}
		if(!b_all && !no_cat && !b_inactive && selected_variable_category[0] == '/') {
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
				if((b_inactive && !CALCULATOR->variables[i]->isActive()) || (CALCULATOR->variables[i]->isActive() && (b_all || (no_cat && CALCULATOR->variables[i]->category().empty()) || (!b_inactive && CALCULATOR->variables[i]->category() == selected_variable_category)))) {
					setVariableTreeItem(iter2, CALCULATOR->variables[i]);
				}
			}
		}
		if(!selected_variable || !gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariables)), &model2, &iter2)) {
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
		Variable *v;
		gtk_tree_model_get(model, &iter, 2, &v, -1);
		//remember selection
		selected_variable = v;
		for(int i = 0; i < CALCULATOR->variables.size(); i++) {
			if(CALCULATOR->variables[i] == selected_variable) {
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_edit"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_insert"), CALCULATOR->variables[i]->isActive());
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_deactivate"), TRUE);
				if(CALCULATOR->variables[i]->isActive()) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "variables_buttonlabel_deactivate")), _("Deactivate"));
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "variables_buttonlabel_deactivate")), _("Activate"));
				}

				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_delete"), CALCULATOR->variables[i]->isLocal());
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_insert"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_delete"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variables_button_deactivate"), FALSE);
		selected_variable = NULL;
	}
}


/*
	generate the unit categories tree in manage units dialog
*/
void update_units_tree() {
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tUnitCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitCategories_selection_changed, NULL);
	gtk_tree_store_clear(tUnitCategories_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitCategories_selection_changed, NULL);
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
	if(!ia_units.empty()) {
		gtk_tree_store_append(tUnitCategories_store, &iter, NULL);
		gtk_tree_store_set(tUnitCategories_store, &iter, 0, _("Inactive"), 1, _("Inactive"), -1);
		if(selected_unit_category == _("Inactive")) {
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
	string snames, sbase;
	//display name, plural name and short name in the second column
	AliasUnit *au;	
	snames = u->name();
	if(!u->plural(false).empty()) {
		snames += " : ";
		snames += u->plural();
	}
	if(!u->singular(false).empty()) {
		if(u->plural(false).empty()) {
			snames += " : ";
		} else {
			snames += "/";
		}
		snames += u->singular();
	}
	//depending on unit type display relation to base unit(s)
	switch(u->unitType()) {
		case COMPOSITE_UNIT: {
			snames = "";
			sbase = u->shortName();
			break;
		}
		case ALIAS_UNIT: {
			au = (AliasUnit*) u;
			if(use_short_units) {
				sbase = au->firstBaseUnit()->shortName();
			} else {
				sbase = au->firstBaseUnit()->singular();
			}
			if(au->firstBaseExp() != 1) {
				sbase += POWER;
				sbase += li2s(au->firstBaseExp());
			}
			break;
		}
		case BASE_UNIT: {
			sbase = "";
			break;
		}
	}
	//display descriptive name (title), or name if no title defined
	gtk_list_store_set(tUnits_store, &iter2, UNITS_TITLE_COLUMN, u->title(true).c_str(), UNITS_NAMES_COLUMN, snames.c_str(), UNITS_BASE_COLUMN, sbase.c_str(), UNITS_POINTER_COLUMN, (gpointer) u, -1);
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

	bool no_cat = false, b_all = false, b_inactive = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnits));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnits_selection_changed, NULL);
	gtk_list_store_clear(tUnits_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnits_selection_changed, NULL);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_insert"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_delete"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_deactivate"), FALSE);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_unit_category = gstr;
		if(selected_unit_category == _("All")) {
			b_all = true;
		} else if(selected_unit_category == _("Uncategorized")) {
			no_cat = true;
		} else if(selected_unit_category == _("Inactive")) {
			b_inactive = true;
		}
		if(!b_all && !no_cat && !b_inactive && selected_unit_category[0] == '/') {
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
				if((b_inactive && !CALCULATOR->units[i]->isActive()) || (CALCULATOR->units[i]->isActive() && (b_all || (no_cat && CALCULATOR->units[i]->category().empty()) || (!b_inactive && CALCULATOR->units[i]->category() == selected_unit_category)))) {
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
		if(!selected_to_unit) {
			selected_to_unit = u;	
		}
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
				if(use_short_units) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "units_label_from_unit")), CALCULATOR->units[i]->shortName().c_str());
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "units_label_from_unit")), CALCULATOR->units[i]->plural(true).c_str());
				}
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_delete"), CALCULATOR->units[i]->isLocal());
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_insert"), CALCULATOR->units[i]->isActive());
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_edit"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_deactivate"), TRUE);
				if(CALCULATOR->units[i]->isActive()) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "units_buttonlabel_deactivate")), _("Deactivate"));
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "units_buttonlabel_deactivate")), _("Activate"));
				}
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_insert"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_delete"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "units_button_deactivate"), FALSE);
		selected_unit = NULL;
	}
	if(!block_unit_convert) convert_in_wUnits();
}

void on_tFunctionArguments_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	selected_argument = NULL;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Argument *arg;
		gtk_tree_model_get(model, &iter, 2, &arg, -1);
		selected_argument = arg;
		int menu_index = MENU_ARGUMENT_TYPE_FREE;
		if(selected_argument) {
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_argument_name")), selected_argument->name().c_str());
			switch(selected_argument->type()) {
				case ARGUMENT_TYPE_TEXT: {
					menu_index = MENU_ARGUMENT_TYPE_TEXT;
					break;
				}
				case ARGUMENT_TYPE_DATE: {
					menu_index = MENU_ARGUMENT_TYPE_DATE;				
					break;
				}
				case ARGUMENT_TYPE_INTEGER: {
					menu_index = MENU_ARGUMENT_TYPE_INTEGER;
					break;
				}
				case ARGUMENT_TYPE_FRACTION: {
					menu_index = MENU_ARGUMENT_TYPE_FRACTION;
					break;
				}
				case ARGUMENT_TYPE_VECTOR: {
					menu_index = MENU_ARGUMENT_TYPE_VECTOR;
					break;
				}
				case ARGUMENT_TYPE_MATRIX: {
					menu_index = MENU_ARGUMENT_TYPE_MATRIX;
					break;
				}
				case ARGUMENT_TYPE_EXPRESSION_ITEM: {
					menu_index = MENU_ARGUMENT_TYPE_EXPRESSION_ITEM;
					break;
				}					
				case ARGUMENT_TYPE_FUNCTION: {
					menu_index = MENU_ARGUMENT_TYPE_FUNCTION;
					break;
				}
				case ARGUMENT_TYPE_UNIT: {
					menu_index = MENU_ARGUMENT_TYPE_UNIT;
					break;
				}
				case ARGUMENT_TYPE_VARIABLE: {
					menu_index = MENU_ARGUMENT_TYPE_VARIABLE;
					break;
				}					
				case ARGUMENT_TYPE_BOOLEAN: {
					menu_index = MENU_ARGUMENT_TYPE_BOOLEAN;
					break;
				}						
			}			
		} else {
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_argument_name")), "");
		}
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "function_edit_optionmenu_argument_type")), menu_index);
		if(!(get_edited_function() && get_edited_function()->isBuiltin())) {
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_rules"), TRUE);	
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_remove_argument"), TRUE);
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_modify_argument"), TRUE);		
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_modify_argument"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_remove_argument"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_rules"), FALSE);	
	}
}
void update_function_arguments_list(Function *f) {
	selected_argument = NULL;
	gtk_list_store_clear(tFunctionArguments_store);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_modify_argument"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_remove_argument"), FALSE);	
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_rules"), FALSE);	
	if(f) {
		GtkTreeIter iter;
		Argument *arg;
		int args = f->maxargs();
		if(args < 0) {
			args = f->minargs() + 1;	
		}
		for(int i = 1; i <= args; i++) {
			gtk_list_store_append(tFunctionArguments_store, &iter);
			string str = "free", str2 = "";
			arg = f->getArgumentDefinition(i);
			if(arg) {
				arg = arg->copy();
				str = arg->printlong();
				str2 = arg->name();
			}			
			gtk_list_store_set(tFunctionArguments_store, &iter, 0, str2.c_str(), 1, str.c_str(), 2, (gpointer) arg, -1);
		}
	}
}


/*
	generate unit submenu in expression menu
	menus are not sorted yet
*/
void create_umenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Units"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression"))), 3)
	u_menu = item;
	sub2 = sub;
	Unit *u;
	list<tree_struct>::iterator it, it2;
	for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			u = (Unit*) it->objects[i];
			if(u->isActive()) {
				MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
			}
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				u = (Unit*) it2->objects[i];
				if(u->isActive()) {
					MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
				}
			}			
		}
	}		
	sub = sub2;
	for(int i = 0; i < uc_units.size(); i++) {
		u = (Unit*) uc_units[i];
		if(u->isActive()) {
			MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
		}
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
	SUBMENU_ITEM_INSERT(_("Convert to unit"), gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result"))), 3)
	u_menu2 = item;
	sub2 = sub;
	Unit *u;
	list<tree_struct>::iterator it, it2;
	for(it = unit_cats.begin(); it != unit_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			u = (Unit*) it->objects[i];
			if(u->isActive()) {
				MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
			}
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				u = (Unit*) it2->objects[i];
				if(u->isActive()) {
					MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
				}
			}			
		}
	}	
	sub = sub2;
	for(int i = 0; i < uc_units.size(); i++) {
		u = (Unit*) uc_units[i];
		if(u->isActive()) {
			MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
		}
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
	update_units_tree();
}

/*
	generate variables submenu in expression menu
*/
void create_vmenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Variables"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression"))), 2)
	v_menu = item;
	sub2 = sub;
	Variable *v;
	list<tree_struct>::iterator it, it2;
	for(it = variable_cats.begin(); it != variable_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			v = (Variable*) it->objects[i];
			if(v->isActive()) {
				MENU_ITEM_WITH_STRING(v->title(true).c_str(), insert_variable, v->name().c_str());
			}
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				v = (Variable*) it2->objects[i];
				if(v->isActive()) {
					MENU_ITEM_WITH_STRING(v->title(true).c_str(), insert_variable, v->name().c_str());
				}
			}			
		}
	}		
	sub = sub2;
	for(int i = 0; i < uc_variables.size(); i++) {
		v = (Variable*) uc_variables[i];
		if(v->isActive()) {
			MENU_ITEM_WITH_STRING(v->title(true).c_str(), insert_variable, v->name().c_str());
		}
	}	
	MENU_SEPARATOR	
	MENU_ITEM(_("Create new variable"), new_variable);
	MENU_ITEM(_("Create new matrix"), new_matrix);	
	MENU_ITEM(_("Create new vector"), new_vector);		
	MENU_ITEM(_("Import CSV file"), on_import_csv_file_activated);	
	MENU_SEPARATOR		
	MENU_ITEM(_("Manage variables"), manage_variables);
	MENU_SEPARATOR
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
	SUBMENU_ITEM_INSERT(_("Set prefix"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_result"))), 4)
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
	update_variables_tree();
}

/*
	generate functions submenu in expression menu
*/
void create_fmenu() {
	GtkWidget *item, *item2, *item3, *item4;
	GtkWidget *sub, *sub2;
	GHashTable *hash;
	SUBMENU_ITEM_INSERT(_("Functions"), gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (glade_xml, "menu_item_expression"))), 1)
	f_menu = item;
	sub2 = sub;
	Function *f;
	list<tree_struct>::iterator it, it2;
	for(it = function_cats.begin(); it != function_cats.end(); ++it) {
		SUBMENU_ITEM(it->item.c_str(), sub2)
		GtkWidget *sub3 = sub;
		for(int i = 0; i < it->objects.size(); i++) {
			f = (Function*) it->objects[i];
			if(f->isActive()) {
				MENU_ITEM_WITH_STRING(f->title(true).c_str(), insert_function, f->name().c_str())
			}
		}				
		for(it2 = it->items.begin(); it2 != it->items.end(); ++it2) {
			SUBMENU_ITEM(it2->item.c_str(), sub3)
			for(int i = 0; i < it2->objects.size(); i++) {
				f = (Function*) it2->objects[i];
				if(f->isActive()) {
					MENU_ITEM_WITH_STRING(f->title(true).c_str(), insert_function, f->name().c_str())
				}
			}			
		}
	}		
	sub = sub2;
	for(int i = 0; i < uc_functions.size(); i++) {
		f = (Function*) uc_functions[i];
		if(f->isActive()) {
			MENU_ITEM_WITH_STRING(f->title(true).c_str(), insert_function, f->name().c_str())
		}
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
	update_functions_tree();
}


/*
	return a customized result string
	set rlabel to true for result label (nicer look)
*/
void getFormat(NumberFormat &numberformat, int &displayflags, int &min_decimals, int &max_decimals, bool rlabel = false, Prefix *prefix = NULL) {
	displayflags = 0;
	displayflags = displayflags | DISPLAY_FORMAT_BEAUTIFY;
	if(fractional_mode == FRACTIONAL_MODE_COMBINED) displayflags = displayflags | DISPLAY_FORMAT_FRACTION;	
	else if(fractional_mode == FRACTIONAL_MODE_FRACTION) displayflags = displayflags | DISPLAY_FORMAT_FRACTIONAL_ONLY;		
	else displayflags = displayflags | DISPLAY_FORMAT_DECIMAL_ONLY;		
	if(use_short_units) displayflags = displayflags | DISPLAY_FORMAT_SHORT_UNITS;
	else displayflags = displayflags | DISPLAY_FORMAT_LONG_UNITS;
	if(use_unicode_signs) displayflags = displayflags | DISPLAY_FORMAT_NONASCII;	
	if(indicate_infinite_series) displayflags = displayflags | DISPLAY_FORMAT_INDICATE_INFINITE_SERIES;		
	if(use_prefixes || prefix) displayflags = displayflags | DISPLAY_FORMAT_USE_PREFIXES;	
	if(rlabel) {
		//displayflags = displayflags | DISPLAY_FORMAT_TAGS;
		displayflags = displayflags | DISPLAY_FORMAT_ALLOW_NOT_USABLE;
	}
	if(number_base == BASE_OCTAL) numberformat = NUMBER_FORMAT_OCTAL;
	else if(number_base == BASE_HEX) numberformat = NUMBER_FORMAT_HEX;
	else if(number_base == BASE_BIN) numberformat = NUMBER_FORMAT_BIN;
	else {
		switch(display_mode) {
			case MODE_SCIENTIFIC: {numberformat = NUMBER_FORMAT_EXP; displayflags = displayflags | DISPLAY_FORMAT_SCIENTIFIC; break;}
			case MODE_SCIENTIFIC_PURE: {numberformat = NUMBER_FORMAT_EXP_PURE; displayflags = displayflags | DISPLAY_FORMAT_SCIENTIFIC; break;}			
			case MODE_DECIMALS: {numberformat = NUMBER_FORMAT_DECIMALS; break;}
			default: {numberformat = NUMBER_FORMAT_NORMAL;}
		}
	}
	if(use_min_deci) {
		min_decimals = min_deci;
	} else {
		min_decimals = 0;
	}
	if(use_max_deci) {
		max_decimals = max_deci;
	} else {
		max_decimals = -1;
	}	
}
string get_value_string(Manager *mngr_, bool rlabel = false, Prefix *prefix = NULL, bool *in_exact = NULL) {
	int displayflags;
	NumberFormat numberformat;
	int min_decimals, max_decimals;
	getFormat(numberformat, displayflags, min_decimals, max_decimals, rlabel, prefix);
	return CALCULATOR->printManagerTimeOut(mngr_, 100000, numberformat, displayflags, min_decimals, max_decimals, in_exact, NULL, prefix);
//	return mngr_->print(numberformat, displayflags, min_decimals, max_decimals, in_exact, NULL, prefix);
}

GdkPixmap *draw_manager(Manager *m, NumberFormat nrformat = NUMBER_FORMAT_NORMAL, int displayflags = DISPLAY_FORMAT_DEFAULT, int min_decimals = 0, int max_decimals = -1, bool *in_exact = NULL, bool *usable = NULL, Prefix *prefix = NULL, bool toplevel = true, bool *plural = NULL, Integer *l_exp = NULL, bool in_composite = false, bool in_power = false, bool draw_minus = false, gint *point_central = NULL) {

	GdkPixmap *pixmap = NULL;
	gint w, h;
	if(toplevel && !in_exact) {
		bool bie = false;
		in_exact = &bie;
	}
	gint central_point = 0;
	if(in_exact && !m->isPrecise()) {
		*in_exact = true;
	}
	switch(m->type()) {
		case STRING_MANAGER: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(in_power) {
				str = "<i>" TEXT_TAGS_SMALL;
			} else {
				str = "<i>" TEXT_TAGS;
			}
			if(m->text() == "pi") str += SIGN_PI;
			else if(m->text() == "euler") str += SIGN_GAMMA;
			else if(m->text() == "apery") str += SIGN_ZETA LEFT_BRACKET "3" RIGHT_BRACKET;
			else if(m->text() == "pythagoras") str += SIGN_SQRT "2";
//			else if(m->text() == "catalan") str += "G";
			else if(m->text() == "golden") str += SIGN_PHI;
			else str += m->text();
			if(in_power) {
				str += TEXT_TAGS_SMALL_END "</i>";
			} else {
				str += TEXT_TAGS_END "</i>";
			}
			w += 1;					
			pango_layout_set_markup(layout, str.c_str(), -1);
			pango_layout_get_pixel_size(layout, &w, &h);
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			break;
		}
		case FUNCTION_MANAGER: {
			gint comma_w, comma_h, function_w, function_h, uh, dh, h, w, ctmp, htmp, wtmp, arc_w, arc_h;
			vector<GdkPixmap*> pixmap_args;
			vector<gint> hpa;
			vector<gint> cpa;			
			vector<gint> wpa;
			
			CALCULATE_SPACE_W
			PangoLayout *layout_comma = gtk_widget_create_pango_layout(resultview, NULL);
			string str, func_str;
			MARKUP_STRING(str, COMMA_STR)
			pango_layout_set_markup(layout_comma, str.c_str(), -1);		
			pango_layout_get_pixel_size(layout_comma, &comma_w, &comma_h);
			PangoLayout *layout_function = gtk_widget_create_pango_layout(resultview, NULL);
			func_str = m->function()->name();
			
			if(func_str == "zeta") func_str = SIGN_ZETA;
			else if(func_str == "sqrt") func_str = SIGN_SQRT;			

			MARKUP_STRING(str, func_str)
			pango_layout_set_markup(layout_function, str.c_str(), -1);			
			pango_layout_get_pixel_size(layout_function, &function_w, &function_h);
			w = function_w + 1;
			uh = function_h / 2 + function_h % 2;
			dh = function_h / 2;
			
			for(int index = 0; index < m->countChilds(); index++) {
				if(l_exp) delete l_exp;
				l_exp = NULL;			
				pixmap_args.push_back(draw_manager(m->getChild(index), nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power, true, &ctmp));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_args[index]), &wtmp, &htmp);
				hpa.push_back(htmp);
				cpa.push_back(ctmp);				
				wpa.push_back(wtmp);
				if(index > 0) {
					w += comma_w;
					w += space_w;
				}				
				w += wtmp;
				if(ctmp > dh) {
					dh = ctmp;
				}
				if(htmp - ctmp > uh) {
					uh = htmp - ctmp;
				}				
			}
			
			uh += 2;
			dh += 2;
			h = uh + dh;
			central_point = dh;
			arc_h = dh * 2;
			arc_w = arc_h / 6;
			w += arc_w * 2 + 2;
			w += 1;

			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
			
			w = 0;
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - function_h / 2 - function_h % 2, layout_function);	
			w += function_w + 1;
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 90 * 64, 180 * 64);			
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w + 1, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 90 * 64, 180 * 64);	
			w += arc_w + 1;
			for(int index = 0; index < m->countChilds(); index++) {
				if(index > 0) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - comma_h / 2 - comma_h % 2, layout_comma);	
					w += comma_w;
					w += space_w;
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_args[index]), 0, 0, w, uh - (hpa[index] - cpa[index]), -1, -1);
				w += wpa[index];
				g_object_unref(pixmap_args[index]);
			}	
			w += 1;
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - arc_w, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 270 * 64, 180 * 64);			
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - 1 - arc_w, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 270 * 64, 180 * 64);	
			w += arc_w;				
			
			g_object_unref(layout_comma);
			g_object_unref(layout_function);
			break;
		}			
		case UNIT_MANAGER: {
			if(m->unit()->unitType() == COMPOSITE_UNIT) {
				Manager *mngr_d = ((CompositeUnit*) m->unit())->generateManager(false);
				int displayflags_d = displayflags;
				if(!(displayflags_d & DISPLAY_FORMAT_USE_PREFIXES)) {
					displayflags_d = displayflags_d | DISPLAY_FORMAT_USE_PREFIXES;
				}
				pixmap = draw_manager(mngr_d, nrformat, displayflags_d, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, NULL, true, in_power, true, &central_point);		
				mngr_d->unref();
			} else {
				string str, str2;
				if(in_power) {
					str = TEXT_TAGS_SMALL;
				} else {
					str = TEXT_TAGS;
				}
				if(!in_composite && toplevel) {
					str2 = "1";
					if(min_decimals > 0) {
						str2 += DOT_STR;
						for(int i = 0; i < min_decimals; i++) {
							str2 += '0';
						}
					}
					str += str2; str += " ";
				}
				if(!(displayflags & DISPLAY_FORMAT_LONG_UNITS)) {
					if(displayflags & DISPLAY_FORMAT_NONASCII) {
						if(m->unit()->name() == "euro") str += SIGN_EURO;
						else if(m->unit()->name() == "oC") str += SIGN_POWER_0 "C";
						else if(m->unit()->name() == "oF") str += SIGN_POWER_0 "F";
						else if(m->unit()->name() == "oR") str += SIGN_POWER_0 "R";
						else str += m->unit()->name();
					} else {
						str += m->unit()->name();
					}
				} else if(plural && *plural) str += m->unit()->plural();
				else str += m->unit()->singular();
				if(in_power) {
					str += TEXT_TAGS_SMALL_END;
				} else {
					str += TEXT_TAGS_END;
				}			
				PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
				pango_layout_set_markup(layout, str.c_str(), -1);
				pango_layout_get_pixel_size(layout, &w, &h);
				central_point = h / 2;
				pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
				gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
				gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, 0, layout);	
				g_object_unref(layout);
			}
			break;
		}
		case FRACTION_MANAGER: {
			bool minus, exp_minus;
			string whole_, numerator_, denominator_, exponent_, prefix_;
			m->fraction()->getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags, min_decimals, max_decimals, prefix, in_exact, usable, false, NULL, l_exp, in_composite, in_power);
			PangoLayout *layout_whole = NULL, *layout_num = NULL, *layout_den = NULL, *layout_prefix = NULL, *layout_exp = NULL;
			gint pw = 0, ph = 0, wlw, hlw, wln, hln, wld, hld, wlp, hlp, hfr, wfr, wle, hle;
			CALCULATE_SPACE_W
			string str;
			if(!whole_.empty()) {
				if(whole_.length() > 1500) {
					whole_ = "(...)";
				}
				if(minus && (toplevel || draw_minus)) {
					whole_.insert(0, SIGN_MINUS);
				}
				MARKUP_STRING(str, whole_)
				if(!exponent_.empty()) {
					str += TEXT_TAGS_SMALL EXP TEXT_TAGS_SMALL_END;
					str += TEXT_TAGS;
					if(exp_minus) {
						str += SIGN_MINUS;
					}
					str += exponent_;
					str += TEXT_TAGS_END;
				}
				layout_whole = gtk_widget_create_pango_layout(resultview, NULL);
				pango_layout_set_markup(layout_whole, str.c_str(), -1);
				pango_layout_get_pixel_size(layout_whole, &wlw, &hlw);
				ph = hlw;
				pw = wlw;
			}
			if(!numerator_.empty()) {
				if(numerator_.length() > 1500) {
					numerator_ = "(...)";
				}
				if(denominator_.length() > 1500) {
					denominator_ = "(...)";
				}
				if(whole_.empty() && minus && (toplevel || draw_minus)) {
					layout_whole = gtk_widget_create_pango_layout(resultview, NULL);
					if(in_power) {
						pango_layout_set_markup(layout_whole, TEXT_TAGS_SMALL SIGN_MINUS TEXT_TAGS_SMALL_END, -1);				
						pango_layout_get_pixel_size(layout_whole, &wlw, &hlw);
					} else {
						pango_layout_set_markup(layout_whole, TEXT_TAGS SIGN_MINUS TEXT_TAGS_END, -1);				
						pango_layout_get_pixel_size(layout_whole, &wlw, &hlw);
					}
					ph = hlw;
					pw = wlw + 2;					
				}
				if(in_power) {
					layout_num = gtk_widget_create_pango_layout(resultview, NULL);
					string str = TEXT_TAGS_SMALL;
					str += numerator_;
					str += SIGN_DIVISION;
					str += denominator_;
					str += " " TEXT_TAGS_SMALL_END;
					pango_layout_set_markup(layout_num, str.c_str(), -1);
					pango_layout_get_pixel_size(layout_num, &wln, &hln);
					hfr = hln;
					wfr = wln;
				} else {
					numerator_.insert(0, TEXT_TAGS);
					numerator_ += TEXT_TAGS_END;
					if(!exponent_.empty() && !exp_minus) {
						numerator_ += TEXT_TAGS_SMALL EXP TEXT_TAGS_SMALL_END;
						numerator_ += TEXT_TAGS;
						numerator_ += exponent_;
						numerator_ += TEXT_TAGS_END;
					}
					layout_num = gtk_widget_create_pango_layout(resultview, NULL);
					pango_layout_set_markup(layout_num, numerator_.c_str(), -1);
					denominator_.insert(0, TEXT_TAGS);
					denominator_ += TEXT_TAGS_END;
					if(!exponent_.empty() && exp_minus) {
						denominator_ += TEXT_TAGS_SMALL EXP TEXT_TAGS_SMALL_END;
						denominator_ += TEXT_TAGS;
						denominator_ += exponent_;
						denominator_ += TEXT_TAGS_END;
					}					
					layout_den = gtk_widget_create_pango_layout(resultview, NULL);				
					pango_layout_set_markup(layout_den, denominator_.c_str(), -1);
					pango_layout_get_pixel_size(layout_den, &wld, &hld);
					pango_layout_get_pixel_size(layout_num, &wln, &hln);
					hfr = hln + hld + 6;
					if(wld > wln) {
						wfr = wld + 2;
					} else {
						wfr = wln + 2;
					}
				}
				if(hfr > ph) {
					ph = hfr;
				}
				if(!whole_.empty()) {
					pw += space_w;
				}
				pw += wfr;
			}
			if(!prefix_.empty()) {
				if(in_power) {
					prefix_.insert(0, TEXT_TAGS_SMALL);
					prefix_ += TEXT_TAGS_SMALL_END;				
				} else {
					prefix_.insert(0, TEXT_TAGS);
					prefix_ += TEXT_TAGS_END;			
				}
				layout_prefix = gtk_widget_create_pango_layout(resultview, NULL);
				pango_layout_set_markup(layout_prefix, prefix_.c_str(), -1);
				pango_layout_get_pixel_size(layout_prefix, &wlp, &hlp);
				if(hlp > ph) {
					ph = hlp;
				}
				if(pw > 0) {
					pw += space_w;
				}
				pw += wlp;
			}	
			central_point = ph / 2;
			pixmap = gdk_pixmap_new(resultview->window, pw, ph, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, pw, ph);	
			gint x = 0;
			if(layout_whole) {
				gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, (ph - hlw) / 2, layout_whole);	
				x += wlw;
			}
			if(layout_num) {
				if(!whole_.empty()) {
					x += space_w;
				} else if(x > 0) {
					x += 2;
				}
				if(layout_den) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], x + (wfr - wln) / 2, (ph - hfr) / 2, layout_num);
					gdk_draw_line(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], x, (ph - hfr) / 2 + hln + 3, x + wfr, (ph - hfr) / 2 + hln + 3);
					gdk_draw_line(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], x, (ph - hfr) / 2 + hln + 3 + 1, x + wfr, (ph - hfr) / 2 + hln + 3 + 1);
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], x + (wfr - wld) / 2, (ph - hfr) / 2 + hln + 6, layout_den);	
				} else {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], x, (ph - hfr) / 2, layout_num);
				}	
				x += wfr;
			}
			if(layout_exp) {
				gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], x, (ph - hle) / 2, layout_exp);
				x += wle;
			}			
			if(layout_prefix) {
				if(l_exp) l_exp->clear();
				if(x > 0) x += space_w;
				gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], x, (ph - hlp) / 2, layout_prefix);
			}
			if(layout_whole) {g_object_unref(layout_whole);}
			if(layout_num) g_object_unref(layout_num);
			if(layout_den) g_object_unref(layout_den);
			if(layout_exp) g_object_unref(layout_exp);			
			if(layout_prefix) g_object_unref(layout_prefix);
			break;
		}	
		case MATRIX_MANAGER: {
			gint wtmp, htmp, ctmp = 0, order_w, order_h, w = 0, h = 0;
			CALCULATE_SPACE_W
			vector<gint> col_w;
			vector<gint> row_h;
			vector<gint> row_uh;
			vector<gint> row_dh;									
			vector<vector<gint> > element_w;
			vector<vector<gint> > element_h;
			vector<vector<gint> > element_c;			
			vector<vector<GdkPixmap*> > pixmap_elements;
			element_w.resize(m->matrix()->rows());			
			element_h.resize(m->matrix()->rows());
			element_c.resize(m->matrix()->rows());						
			pixmap_elements.resize(m->matrix()->rows());
			for(int index_r = 0; index_r < m->matrix()->rows(); index_r++) {
				for(int index_c = 0; index_c < m->matrix()->columns(); index_c++) {
					ctmp = 0;
					pixmap_elements[index_r].push_back(draw_manager(m->matrix()->get(index_r + 1, index_c + 1), nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power, true, &ctmp));					
					gdk_drawable_get_size(GDK_DRAWABLE(pixmap_elements[index_r][index_c]), &wtmp, &htmp);
					element_w[index_r].push_back(wtmp);
					element_h[index_r].push_back(htmp);
					element_c[index_r].push_back(ctmp);					
					if(index_r == 0) {
						col_w.push_back(wtmp);
					} else if(wtmp > col_w[index_c]) {
						col_w[index_c] = wtmp;
					}
					if(index_c == 0) {
						row_uh.push_back(htmp - ctmp);
						row_dh.push_back(ctmp);
					} else {
						if(ctmp > row_dh[index_r]) {
							row_dh[index_r] = ctmp;
						}
						if(htmp - ctmp > row_uh[index_r]) {
							row_uh[index_r] = htmp - ctmp;
						}						
					}
				}
				row_h.push_back(row_uh[index_r] + row_dh[index_r]);
				h += row_h[index_r];
				if(index_r != 0) {
					h += 4;
				}
			}	
			h += 4;
			for(int i = 0; i < col_w.size(); i++) {
				w += col_w[i];
				w += space_w * 2;
			}
	
			gint wlr = h / 6, wll = h / 6;
			
			w += wlr + 1;
			w += wll + 1;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	

			w = 0;
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w, 0, wll * 2, h, 90 * 64, 180 * 64);			
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w + 1, 0, wll * 2, h, 90 * 64, 180 * 64);						
			h = 2;
			for(int index_r = 0; index_r < m->matrix()->rows(); index_r++) {
				w = wll + 1;
				for(int index_c = 0; index_c < m->matrix()->columns(); index_c++) {
					gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_elements[index_r][index_c]), 0, 0, w + (col_w[index_c] - element_w[index_r][index_c]) / 2, h + row_uh[index_r] - (element_h[index_r][index_c] - element_c[index_r][index_c]), -1, -1);				
					w += col_w[index_c];
					w += space_w * 2;
					g_object_unref(pixmap_elements[index_r][index_c]);
				}
				h += row_h[index_r];
				h += 4;
			}
			w -= space_w * 2;
			h -= 4;
			h += 2;
			w += 1;
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - wlr, 0, wlr * 2, h, 270 * 64, 180 * 64);
			gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - 1 - wlr, 0, wlr * 2, h, 270 * 64, 180 * 64);						
			break;
		}
		case COMPARISON_MANAGER: {}
		case AND_MANAGER: {}				
		case OR_MANAGER: {
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> wpt;
			vector<gint> cpt;
			gint sign_w, sign_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
			CALCULATE_SPACE_W
			PangoLayout *layout_sign = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(in_power) {
				str = TEXT_TAGS_SMALL;
			} else {
				str = TEXT_TAGS;
			}	
			if(m->type() == COMPARISON_MANAGER) {
				switch(m->comparisonType()) {
					case COMPARISON_EQUALS: {
						str += "==";
						break;
					}
					case COMPARISON_NOT_EQUALS: {
						str += SIGN_NOT_EQUAL;
						break;
					}
					case COMPARISON_GREATER: {
						str += ">";
						break;
					}
					case COMPARISON_LESS: {
						str += "<";
						break;
					}
					case COMPARISON_EQUALS_GREATER: {
						str += SIGN_GREATER_OR_EQUAL;
						break;
					}
					case COMPARISON_EQUALS_LESS: {
						str += SIGN_LESS_OR_EQUAL;
						break;
					}
				}
			} else if(m->type() == AND_MANAGER) {
				str += AND;
			} else if(m->type() == OR_MANAGER) {
				str += OR;
			}
			if(in_power) {
				str += TEXT_TAGS_SMALL_END;
			} else {
				str += TEXT_TAGS_END;
			}
			pango_layout_set_markup(layout_sign, str.c_str(), -1);
			pango_layout_get_pixel_size(layout_sign, &sign_w, &sign_h);
			for(int i = 0; i < m->countChilds(); i++) {
				if(l_exp) delete l_exp;
				l_exp = NULL;	
				hetmp = 0;		
				pixmap_terms.push_back(draw_manager(m->getChild(i), nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power, true, &hetmp));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[i]), &wtmp, &htmp);
				hpt.push_back(htmp);
				cpt.push_back(hetmp);
				wpt.push_back(wtmp);
				w += wtmp;
				if(i > 0) w += sign_w;
				if(sign_h / 2 > dh) {
					dh = sign_h / 2;
				}
				if(sign_h / 2 + sign_h % 2 > uh) {
					uh = sign_h / 2 + sign_h % 2;
				}					
				if(htmp - hetmp > uh) {
					uh = htmp - hetmp;
				}
				if(hetmp > dh) {
					dh = hetmp;
				}				
			}
			w += space_w * (pixmap_terms.size() - 1) * 2;
			central_point = dh;
			h = dh + uh;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);			
			w = 0;
			for(int i = 0; i < pixmap_terms.size(); i++) {
				if(i > 0) {
					w += space_w;
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - sign_h / 2 - sign_h % 2, layout_sign);
					w += sign_w;	
					w += space_w;			
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_terms[i]), 0, 0, w, uh - (hpt[i] - cpt[i]), -1, -1);
				w += wpt[i];
				g_object_unref(pixmap_terms[i]);
			}
			g_object_unref(layout_sign);
			break;
		}			
		case ADDITION_MANAGER: {
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> wpt;
			vector<gint> cpt;
			gint plus_w, plus_h, minus_w, minus_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
			CALCULATE_SPACE_W
			PangoLayout *layout_plus = gtk_widget_create_pango_layout(resultview, NULL);
			if(in_power) {
				pango_layout_set_markup(layout_plus, TEXT_TAGS_SMALL "+" TEXT_TAGS_SMALL_END, -1);
			} else {
				pango_layout_set_markup(layout_plus, TEXT_TAGS "+" TEXT_TAGS_END, -1);
			}
			pango_layout_get_pixel_size(layout_plus, &plus_w, &plus_h);
			PangoLayout *layout_minus = gtk_widget_create_pango_layout(resultview, NULL);
			if(in_power) {
				pango_layout_set_markup(layout_minus, TEXT_TAGS_SMALL SIGN_MINUS TEXT_TAGS_SMALL_END, -1);
			} else {
				pango_layout_set_markup(layout_minus, TEXT_TAGS SIGN_MINUS TEXT_TAGS_END, -1);
			}			
			pango_layout_get_pixel_size(layout_minus, &minus_w, &minus_h);
			for(int i = 0; i < m->countChilds(); i++) {
				if(l_exp) delete l_exp;
				l_exp = NULL;	
				hetmp = 0;		
				pixmap_terms.push_back(draw_manager(m->getChild(i), nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power, false, &hetmp));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[i]), &wtmp, &htmp);
				hpt.push_back(htmp);
				cpt.push_back(hetmp);
				wpt.push_back(wtmp);
				w += wtmp;
				if(m->getChild(i)->hasNegativeSign()) {
					w += minus_w;
					if(minus_h / 2 > dh) {
						dh = minus_h / 2;
					}
					if(minus_h / 2 + minus_h % 2 > uh) {
						uh = minus_h / 2 + minus_h % 2;
					}						
				} else {
					if(i > 0) w += plus_w;
					if(plus_h / 2 > dh) {
						dh = plus_h / 2;
					}
					if(plus_h / 2 + plus_h % 2 > uh) {
						uh = plus_h / 2 + plus_h % 2;
					}					
				}
				if(htmp - hetmp > uh) {
					uh = htmp - hetmp;
				}
				if(hetmp > dh) {
					dh = hetmp;
				}				
			}
			w += space_w * (pixmap_terms.size() - 1) * 2;
			central_point = dh;
			h = dh + uh;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);			
			w = 0;
			for(int i = 0; i < pixmap_terms.size(); i++) {
				if(i == 0) {
					if(m->getChild(i)->negative()) {
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - minus_h / 2 - minus_h % 2, layout_minus);
						w += minus_w;
					}
				} else {
					w += space_w;
					if(m->getChild(i)->negative()) {
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - minus_h / 2 - minus_h % 2, layout_minus);
						w += minus_w;
					} else {
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - plus_h / 2 - plus_h % 2, layout_plus);
						w += plus_w;
					}	
					w += space_w;			
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_terms[i]), 0, 0, w, uh - (hpt[i] - cpt[i]), -1, -1);
				w += wpt[i];
				g_object_unref(pixmap_terms[i]);
			}
			g_object_unref(layout_minus);
			g_object_unref(layout_plus);
			break;
		}	
		case POWER_MANAGER: {
			gint power_w, power_h, base_w, base_h, exp_w, exp_h, one_w, one_h, w = 0, h = 0, ctmp = 0;
			CALCULATE_SPACE_W
			GdkPixmap *pixmap_one = NULL;
			if(!in_composite && toplevel && m->base()->isUnit()) {
				Manager one_mngr(1, 1);
				pixmap_one = draw_manager(&one_mngr, nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, NULL, false, in_power, true);	
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_one), &one_w, &one_h);
			}
			bool wrap_base = false, wrap_exp = false;
			if((m->base()->countChilds() > 0 || m->base()->hasNegativeSign() || (m->base()->isFraction() && !m->base()->fraction()->isInteger() && ((displayflags & DISPLAY_FORMAT_FRACTION) || (displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY)))) && !m->base()->isFunction() && !in_composite) {
				wrap_base = true;
			}
			if(m->exponent()->countChilds() > 0) {
				wrap_exp = true;
			}			
			GdkPixmap *pixmap_base = draw_manager(m->base(), nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, NULL, in_composite, in_power, true, &central_point);			
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_base), &base_w, &base_h);
			
			PangoLayout *layout_power = NULL;
			if(in_power) {
				layout_power = gtk_widget_create_pango_layout(resultview, NULL);
				pango_layout_set_markup(layout_power, TEXT_TAGS_SMALL POWER TEXT_TAGS_SMALL_END, -1);
				pango_layout_get_pixel_size(layout_power, &power_w, &power_h);			
			}
			if(!(displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY) && (displayflags & DISPLAY_FORMAT_FRACTION)) {
				displayflags = displayflags | DISPLAY_FORMAT_FRACTIONAL_ONLY;
			}			
			GdkPixmap *pixmap_exp = draw_manager(m->exponent(), nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, NULL, in_composite, true, true, &ctmp);
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_exp), &exp_w, &exp_h);
			
			h = base_h;
			w = base_w;
			gint arc_base_w = base_h / 6;
			gint arc_exp_w = exp_h / 6;									
			if(wrap_base) {
				base_h += 4;
				central_point += 2;
				w += arc_base_w * 2 + 3;
			}						
			if(layout_power) {
				w += power_w; 
				if(exp_h > h) {
					h = exp_h;
				}
			} else {
				if(exp_h < h) {
					h += exp_h / 2;
				} else {
					h += exp_h - base_h / 2;
				}
			}
			w += exp_w;
			if(wrap_exp) {
				w += arc_exp_w * 2 + 3;
			}
			if(pixmap_one) {
				w += one_w;
				w += space_w;
				if(one_h > h) {
					h = one_h;
				}
			}			
			
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);			

			w = 0;
			if(pixmap_one) {
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_one), 0, 0, w, h - one_h, -1, -1);
				w += one_w;
				w += space_w;
			}
			if(wrap_base) {
				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w, h - base_h - 4, arc_base_w * 2, base_h + 4, 90 * 64, 180 * 64);			
				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w + 1, h - base_h - 4, arc_base_w * 2, base_h + 4, 90 * 64, 180 * 64);	
				w += arc_base_w + 1;
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_base), 0, 0, w, h - base_h - 2, -1, -1);
				w += base_w + 1;
				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - arc_base_w, h - base_h - 4, arc_base_w * 2, base_h + 4, 270 * 64, 180 * 64);			
				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - 1 - arc_base_w, h - base_h - 4, arc_base_w * 2, base_h + 4, 270 * 64, 180 * 64);	
				w += arc_base_w + 1;				
			} else {
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_base), 0, 0, w, h - base_h, -1, -1);
				w += base_w;
			}
			if(layout_power) {
				gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, 0, layout_power);
				w += power_w;
			}	
			if(wrap_exp) {
				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w, 0, arc_exp_w * 2, exp_h, 90 * 64, 180 * 64);			
//				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w + 1, 0, arc_exp_w * 2, exp_h, 90 * 64, 180 * 64);	
				w += arc_exp_w + 1;
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_exp), 0, 0, w, 0, -1, -1);
				w += exp_w + 1;
				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - arc_exp_w, 0, arc_exp_w * 2, exp_h, 270 * 64, 180 * 64);			
//				gdk_draw_arc(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], FALSE, w - 1 - arc_exp_w, 0, arc_exp_w * 2, exp_h, 270 * 64, 180 * 64);	
			} else {
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_exp), 0, 0, w, 0, -1, -1);
			}			
			
			g_object_unref(pixmap_base);
			g_object_unref(pixmap_exp);
			if(layout_power) g_object_unref(layout_power);
			if(pixmap_one) g_object_unref(pixmap_one);
			break;
		} 
		case MULTIPLICATION_MANAGER: {
		
			if(displayflags & DISPLAY_FORMAT_SCIENTIFIC) {
				m->sort(SORT_SCIENTIFIC);
			} else {
				m->sort(SORT_DEFAULT);
			}

			gint multi_w, multi_h, div_w, div_h, minus_w, minus_h, one_w, one_h, wtmp, htmp, hetmp, w = 0, w2 = 0, h = 0, h2 = 0, uh = 0, dh = 0, uh2 = 0, dh2 = 0, num_h = 0, num_w = 0, den_h = 0, den_w = 0, wfr = 0, cfr = 0, hfr = 0, num_dh = 0, num_uh = 0, den_dh = 0, den_uh = 0, dhfr = 0, uhfr = 0;
			CALCULATE_SPACE_W
			PangoLayout *layout_multi = gtk_widget_create_pango_layout(resultview, NULL);
			if(in_power) {
				pango_layout_set_markup(layout_multi, TEXT_TAGS_SMALL SIGN_MULTIDOT TEXT_TAGS_SMALL_END, -1);
			} else {
				pango_layout_set_markup(layout_multi, TEXT_TAGS SIGN_MULTIDOT TEXT_TAGS_END, -1);
			}
			pango_layout_get_pixel_size(layout_multi, &multi_w, &multi_h);
			PangoLayout *layout_div = gtk_widget_create_pango_layout(resultview, NULL);
			if(in_power) {
				pango_layout_set_markup(layout_div, TEXT_TAGS_SMALL SIGN_DIVISION TEXT_TAGS_SMALL_END, -1);
			} else {
				pango_layout_set_markup(layout_div, TEXT_TAGS SIGN_DIVISION TEXT_TAGS_END, -1);
			}
			pango_layout_get_pixel_size(layout_div, &div_w, &div_h);
			PangoLayout *layout_minus = gtk_widget_create_pango_layout(resultview, NULL);
			if(in_power) {
				pango_layout_set_markup(layout_minus, TEXT_TAGS_SMALL SIGN_MINUS TEXT_TAGS_SMALL_END, -1);
			} else {
				pango_layout_set_markup(layout_minus, TEXT_TAGS SIGN_MINUS TEXT_TAGS_END, -1);
			}			
			pango_layout_get_pixel_size(layout_minus, &minus_w, &minus_h);									
			
			Manager one_mngr(1, 1);
			GdkPixmap *pixmap_one = draw_manager(&one_mngr, nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, NULL, false, in_power, true);	
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_one), &one_w, &one_h);
						
			Manager *m_i;

			bool b = false, c = false, d = false;
			bool plural_ = true;
			int prefix_ = 0;
			bool had_unit = false, had_div_unit = false, is_unit = false;
			int unit_count = 0;
			bool had_non_unit_div = false;
			int div_count = 0;
			bool do_div = false;
			bool do_simple_unit_div = false;
			bool num_non_unit = false;
			bool prepend_one = false;
			bool first_is_minus = false;
			bool minus = false;
			
			bool prefix_fract = false;
			PangoLayout *layout_num = NULL;
			PangoLayout *layout_den = NULL;
			gint num_fract_h, num_fract_w, den_fract_h, den_fract_w;
			vector<GdkPixmap*> pixmap_factors;
			vector<GdkPixmap*> pixmap_factors_div;
			vector<gint> hpf;			
			vector<gint> cpf;			
			vector<gint> wpf;
			vector<gint> hpf_div;			
			vector<gint> cpf_div;						
			vector<gint> wpf_div;						
			vector<bool> f_is_unit;
			vector<bool> f_is_div;
			vector<bool> add_multi;
			vector<bool> do_wrap;
			vector<bool> f_needs_multi_space;
			vector<bool> f_has_prefix;			
			
			for(int i = 0; i < m->countChilds(); i++) {
				m_i = m->getChild(i);
				is_unit = false;
				hetmp = 0;
				if(i == 0 && m_i->isFraction() && m_i->fraction()->isMinusOne() && m->countChilds() > 1 && !m->getChild(1)->isUnit_exp()) {
					first_is_minus = true;
					pixmap_factors.push_back(NULL);
					pixmap_factors_div.push_back(NULL);
					f_is_unit.push_back(false);
					f_is_div.push_back(false);		
					f_needs_multi_space.push_back(false);			
					if(draw_minus || toplevel) {
						w = minus_w; w2 = minus_w;
						hpf.push_back(minus_h);
						wpf.push_back(minus_w);
						dh = minus_h / 2;
						uh = minus_h / 2 + minus_h % 2;
						dh2 = dh;
						uh2 = uh;
					} else {
						hpf.push_back(0);
						wpf.push_back(0);					
					}
					hpf_div.push_back(0);
					wpf_div.push_back(0);						
					cpf_div.push_back(0);
					cpf.push_back(dh);
/*					string str;
					layout_num = gtk_widget_create_pango_layout(resultview, NULL);
					MARKUP_STRING(str, TEXT_TAGS "1" TEXT_TAGS_END)
					pango_layout_set_markup(layout_num, str.c_str(), -1);
					pango_layout_get_pixel_size(layout_num, &num_fract_w, &num_fract_h);
					w2 += num_fract_w;
					if(num_fract_h / 2 > dh2) {
						dh2 = num_fract_h / 2;
					}
					if(num_fract_h / 2 + num_fract_h % 2 > uh2) {
						uh2 = num_fract_h / 2 + num_fract_h % 2;
					}*/													
					f_has_prefix.push_back(false);
					i++;
					m_i = m->getChild(i);
				}	

				if(m_i->isUnit() || (m_i->isPower() && m_i->base()->isUnit())) {
					is_unit = true;
					if(m_i->isUnit() && m_i->unit()->unitType() == COMPOSITE_UNIT) {
						unit_count += ((CompositeUnit*) m_i->unit())->countUnits();
					} else {
						unit_count++;
					}
					if(i == 0) {
						prepend_one = true;
						w += one_w;
						w2 += one_w;
						w += space_w;
						w2 += space_w;
					}
				} 
				f_is_unit.push_back(is_unit);
				if(l_exp) delete l_exp;
				l_exp = NULL;
				if(prefix_) prefix_--;
				if(m_i->isFraction() && m->countChilds() >= i + 2) {
					if(m->getChild(i + 1)->isPower()) {
						if(m->getChild(i + 1)->exponent()->isFraction() && m->getChild(i + 1)->base()->isUnit()) {
							if(m->getChild(i + 1)->exponent()->fraction()->isInteger()) {
								l_exp = new Integer(m->getChild(i + 1)->exponent()->fraction()->numerator());
							}
							prefix_ = 2;
						} 
					} else if(m->getChild(i + 1)->isUnit() && m->getChild(i + 1)->unit()->unitType() != COMPOSITE_UNIT) {
						l_exp = new Integer(1);
						prefix_ = 2;						
					}
				}
			
				if(m_i->isPower() && m_i->exponent()->hasNegativeSign()) {
					Manager mngr_i(m_i);
					mngr_i.addInteger(-1, OPERATION_RAISE);
					pixmap_factors_div.push_back(draw_manager(&mngr_i, nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power, true, &hetmp));
					gdk_drawable_get_size(GDK_DRAWABLE(pixmap_factors_div[i]), &wtmp, &htmp);
					w2 += wtmp;
					if(htmp - hetmp > uh2) {
						uh2 = htmp - hetmp;
					}
					if(hetmp > dh2) {
						dh2 = hetmp;
					}					
					f_is_div.push_back(true);
					hpf_div.push_back(htmp);
					wpf_div.push_back(wtmp);
					cpf_div.push_back(hetmp);					
					if(!is_unit) had_non_unit_div = true;
					if(div_count == 0) {
						if(i > 0 && f_needs_multi_space[i - 1]) {
							w2 -= space_w;
							w2 -= multi_w;
							w2 -= space_w;
						}
						w2 += space_w;
						w2 += div_w;
						w2 += space_w;
						if(div_h / 2 > dh2) {
							dh2 = div_h / 2;
						}
						if(div_h / 2 + div_h % 2 > uh2) {
							uh2 = div_h / 2 + div_h % 2;
						}						
					}
					div_count++;
				} else {		
					if(m_i->isFraction() && i == 0 && ((displayflags & DISPLAY_FORMAT_FRACTION) || (displayflags & DISPLAY_FORMAT_FRACTIONAL_ONLY))) {
						int displayflags_d = displayflags;
						if(!(displayflags_d & DISPLAY_FORMAT_FRACTIONAL_ONLY)) {
							displayflags_d = displayflags_d | DISPLAY_FORMAT_FRACTIONAL_ONLY;
						}
						bool exp_minus = false;
						string whole_, numerator_, denominator_, exponent_, prefix_;
						m_i->fraction()->getPrintObjects(minus, whole_, numerator_, denominator_, exp_minus, exponent_, prefix_, nrformat, displayflags_d, min_decimals, max_decimals, prefix, in_exact, usable, false, NULL, l_exp, in_composite, in_power);
						string str;						
						if(!denominator_.empty() || (exp_minus && !exponent_.empty())) {
							if(denominator_.empty() && exp_minus) denominator_ = "1";
							layout_den = gtk_widget_create_pango_layout(resultview, NULL);
							if(exp_minus && !exponent_.empty()) {
								exponent_.insert(0, TEXT_TAGS_END TEXT_TAGS_SMALL "E" TEXT_TAGS_SMALL_END TEXT_TAGS);
								denominator_ += exponent_;
							}
							MARKUP_STRING(str, denominator_)							
							pango_layout_set_markup(layout_den, str.c_str(), -1);
							pango_layout_get_pixel_size(layout_den, &den_fract_w, &den_fract_h);
							w2 += den_fract_w;
							if(den_fract_h / 2 > dh2) {
								dh2 = den_fract_h / 2;
							}
							if(den_fract_h / 2 + den_fract_h % 2 > uh2) {
								uh2 = den_fract_h / 2 + den_fract_h % 2;
							}
						}
						if(numerator_.empty()) {
							numerator_ = whole_;
						}
						if(numerator_ == "1") numerator_ = "";
						layout_num = gtk_widget_create_pango_layout(resultview, NULL);
						if(!exp_minus && !exponent_.empty()) {
							if(numerator_.empty()) numerator_ = "1";
							exponent_.insert(0, TEXT_TAGS_END TEXT_TAGS_SMALL "E" TEXT_TAGS_SMALL_END TEXT_TAGS);
							numerator_ += exponent_;
						}
						if(!prefix_.empty()) {
							prefix_fract = true;
							if(numerator_.empty() && !in_composite) numerator_ = "1";
							if(!numerator_.empty()) numerator_ += " ";
							numerator_ += prefix_;
						}
						MARKUP_STRING(str, numerator_)
						pango_layout_set_markup(layout_num, str.c_str(), -1);
						pango_layout_get_pixel_size(layout_num, &num_fract_w, &num_fract_h);
						if(minus) {
							w2 += minus_w;
						}							
						w2 += num_fract_w;
						if(num_fract_h / 2 > dh2) {
							dh2 = num_fract_h / 2;
						}
						if(num_fract_h / 2 + num_fract_h % 2 > uh2) {
							uh2 = num_fract_h / 2 + num_fract_h % 2;
						}											
					}
					pixmap_factors_div.push_back(NULL);
					f_is_div.push_back(false);			
					hpf_div.push_back(0);
					cpf_div.push_back(0);					
					wpf_div.push_back(0);
				}	
				hetmp = 0;
				pixmap_factors.push_back(draw_manager(m_i, nrformat, displayflags, min_decimals, max_decimals, in_exact, usable, prefix, false, NULL, l_exp, in_composite, in_power, true, &hetmp));
				f_has_prefix.push_back(prefix_ == 2 && l_exp && l_exp->isZero());
				if(m_i->isText() && (m_i->text().length() == 1 || m_i->text() == "pi" || m_i->text() == "euler" || m_i->text() == "golden") || (m_i->isFraction() && !(i < m->countChilds() && m->getChild(i + 1)->isPower() && m->getChild(i + 1)->base()->isFraction())) || (m_i->isPower() && m_i->base()->isText())) {
					f_needs_multi_space.push_back(false);
				} else {
					f_needs_multi_space.push_back(true);
					if(i > 0 && !f_has_prefix[i - 1] && !f_needs_multi_space[i - 1] && (i != 1 || !first_is_minus)) {
						w += space_w;
						w2 += space_w;
					}
					if(i + 1 < m->countChilds()) {
						w += space_w;
						w2 += space_w;
						w += multi_w;
						w2 += multi_w;
						if(multi_h / 2 > dh) {
							dh = multi_h / 2;
						}
						if(multi_h / 2 > dh2) {
							dh2 = multi_h / 2;
						}
						if(multi_h / 2 + multi_h % 2 > uh) {
							uh = multi_h / 2 + multi_h % 2;
						}
						if(multi_h / 2 + multi_h % 2 > uh2) {
							uh2 = multi_h / 2 + multi_h % 2;
						}												
						w += space_w;
						w2 += space_w;
					}
				}
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_factors[i]), &wtmp, &htmp);
				hpf.push_back(htmp);				
				cpf.push_back(hetmp);								
				wpf.push_back(wtmp);
				w += wtmp;
				if(!f_is_div[i] && !(layout_num && i == 0)) {
					w2 += wtmp;
					if(htmp - hetmp > uh2) {
						uh2 = htmp - hetmp;
					}
					if(hetmp > dh2) {
						dh2 = hetmp;
					}										
				}
				if(htmp - hetmp > uh) {
					uh = htmp - hetmp;
				}
				if(hetmp > dh) {
					dh = hetmp;
				}				
				if(l_exp) delete l_exp;
				l_exp = NULL;
			}
			bool layout_den_needs_multi_space = false;
			if(!(displayflags & DISPLAY_FORMAT_SCIENTIFIC)) {
				if(!in_power && (had_non_unit_div || div_count > 1 || (div_count > 0 && (unit_count > 2 || layout_den)))) {
					do_div = true;
					int index = 0;
					h = 0;
					uh = 0;
					dh = 0;
					w = 0;
					if(first_is_minus) {
						w = minus_w;
						w += 1;
						uh = minus_h / 2 + minus_h % 2;
						dh = minus_h;
						index = 1;
					}
					if(minus) {
						w = minus_w;
						w += 1;
						uh = minus_h / 2 + minus_h % 2;
						dh = minus_h;
					}
					bool last_num = false;
					bool last_den = false;
					if(layout_num) {
						num_uh = num_fract_h / 2 + num_fract_h % 2;
						num_dh = num_fract_h / 2;
						num_w = num_fract_w;
						index += 1;
						last_num = true;
					}	

					if(layout_den) {
						den_uh = den_fract_h / 2 + den_fract_h % 2;
						den_dh = den_fract_h / 2;
						den_w = den_fract_w;						
						last_den = true;
					}
					if(prepend_one) {
						num_w += one_w;
						if(one_h / 2 + one_h % 2 > num_uh) {
							num_uh = one_h / 2 + one_h % 2;
						}
						if(one_h / 2 > num_dh) {
							num_dh = one_h / 2;
						}						
					}
					for(; index < m->countChilds(); index++) {
						if(f_is_div[index]) {
							if(index == 0 || (index == 1 && (num_fract_w == 0 || first_is_minus))) {
								num_w = 0;
								prepend_one = false;
							}
							break;
						}
						if(last_num && num_fract_w == 0 && f_is_unit[index]) {
							prepend_one = true;						
							num_w += one_w;
							if(one_h / 2 + one_h % 2 > num_uh) {
								num_uh = one_h / 2 + one_h % 2;
							}
							if(one_h / 2 > num_dh) {
								num_dh = one_h / 2;
							}							
						}
						num_w += wpf[index];
						if(hpf[index] - cpf[index] > num_uh) {
							num_uh = hpf[index] - cpf[index];
						}
						if(cpf[index] > num_dh) {
							num_dh = cpf[index];
						}						
						if(f_needs_multi_space[index] && index > 0 && !f_needs_multi_space[index - 1] && (index != 1 || !first_is_minus)) {
							num_w += space_w;
						}
						if(f_needs_multi_space[index] && ((index + 1 < m->countChilds() && !f_is_div[index + 1]) || (last_num && index > 0 && f_needs_multi_space[index - 1]))) {
							num_w += space_w;
							num_w += multi_w;
							if(multi_h / 2 + multi_h % 2 > num_uh) {
								num_uh = multi_h / 2 + multi_h % 2;
							}
							if(multi_h / 2 > num_dh) {
								num_dh = multi_h / 2;
							}
							num_w += space_w;
						}
						last_num = false;						
					}
					if(num_w == 0) {
						num_w = one_w;
						num_dh = one_h / 2;
						num_uh = one_h / 2 + one_h % 2;
					}
					for(; index < m->countChilds(); index++) {
						if(last_den && m->getChild(index)->isPower() && m->getChild(index)->base()->isFraction()) {
							layout_den_needs_multi_space = true;
						}
						if(!f_is_div[index]) {
							break;
						}
						den_w += wpf_div[index];
						if(hpf_div[index] > den_h) {
							den_h = hpf_div[index];
						}
						if(hpf_div[index] - cpf_div[index] > den_uh) {
							den_uh = hpf_div[index] - cpf_div[index];
						}
						if(cpf_div[index] > den_dh) {
							den_dh = cpf_div[index];
						}						
						if(f_needs_multi_space[index] && ((index > 0 && !f_has_prefix[index - 1] && !f_needs_multi_space[index - 1] && f_is_div[index - 1]) || (last_den && !layout_den_needs_multi_space))) {
							den_w += space_w;
						}
						if(f_needs_multi_space[index] && ((index + 1 < m->countChilds() && f_is_div[index + 1]) || (last_den && layout_den_needs_multi_space))) {
							den_w += space_w;
							den_w += multi_w;
							if(multi_h / 2 + multi_h % 2 > den_uh) {
								den_uh = multi_h / 2 + multi_h % 2;
							}
							if(multi_h / 2 > den_dh) {
								den_dh = multi_h / 2;
							}							
							den_w += space_w;
						}		
						last_den = false;				
					}
					num_h = num_uh + num_dh;
					den_h = den_uh + den_dh;
					hfr = num_h + den_h + 6;
					dhfr = den_h + 3;
					uhfr = hfr - dhfr;
					if(den_w > num_w) {
						wfr = den_w;
					} else {
						wfr = num_w;
					}
					wfr += 2;
					w += wfr;
					if(uhfr > uh) {
						uh = uhfr;
					}
					if(dhfr > dh) {
						dh = dhfr;
					}
					for(; index < m->countChilds(); index++) {
						if(hpf[index] - cpf[index] > uh) {
							uh = hpf[index] - cpf[index];
						}
						if(cpf[index] > dh) {
							dh = cpf[index];
						}						
						w += wpf[index];
						if(f_is_div[index - 1]) {
							w += space_w;
							w += multi_w;
							if(multi_h / 2 + multi_h % 2 > uh) {
								uh = multi_h / 2 + multi_h % 2;
							}
							if(multi_h / 2 > dh) {
								dh = multi_h / 2;
							}															
							w += space_w;
						} else if(f_needs_multi_space[index] && index > 0 && !f_needs_multi_space[index - 1]) {
							w += space_w;
						}
						if(f_needs_multi_space[index] && index + 1 < m->countChilds()) {
							w += space_w;
							w += multi_w;
							if(multi_h / 2 + multi_h % 2 > uh) {
								uh = multi_h / 2 + multi_h % 2;
							}
							if(multi_h / 2 > dh) {
								dh = multi_h / 2;
							}							
							w += space_w;
						}						
					}
				} else if(div_count > 0) {
					uh = uh2;
					dh = dh2;
					w = w2;
					do_simple_unit_div = true;
				}
			}
			h = uh + dh;
			central_point = dh;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);			

			w = 0;
			bool in_den = false;
			bool in_num = true;
			int div_i = 0;
			gint w_first_fract = 0;
			gint line_y = uh;
			num_uh = uh - 2 - num_dh;
			den_uh = uh + 3 + den_uh;
			for(int i = 0; i < m->countChilds(); i++) {
				if(i == 0 && first_is_minus) {
					if(toplevel || draw_minus) {
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - minus_h / 2 - minus_h % 2, layout_minus);	
						w += minus_w;
						if(do_div) w += 1;
						w_first_fract = w;
					}	
				} else if(do_div) {
					if(i == 0) {
						if(minus && (toplevel || draw_minus)) {
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - minus_h / 2 - minus_h % 2, layout_minus);	
							w += minus_w;
							w += 1;
							w_first_fract = w;						
						}
						w = (wfr - num_w) / 2 + w_first_fract;
					}
					if(f_is_div[i]) {
						if(i == 0 || (i == 1 && (num_fract_w == 0 || first_is_minus))) {
							gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_one), 0, 0, w, num_uh - one_h / 2 - one_h % 2, -1, -1);								
							w += one_w;
						}													
						if(!in_den) {
							in_den = true;
							in_num = false;
							w = w_first_fract;
							gdk_draw_line(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, line_y, w + wfr, line_y);
							gdk_draw_line(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, line_y + 1, w + wfr, line_y + 1);
							w = (wfr - den_w) / 2 + w_first_fract;
						}
						if(div_i == 0 && layout_den) {						
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, den_uh - den_fract_h / 2 - den_fract_h % 2, layout_den);	
							w += den_fract_w;							
							if(layout_den_needs_multi_space) {
								w += space_w;
								gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, den_uh - multi_h / 2 - multi_h % 2, layout_multi);
								w += multi_w;
								w += space_w;
							}							
						}						
						if(f_needs_multi_space[i] && (((div_i == 0 || !f_is_div[i - 1]) && layout_den && !layout_den_needs_multi_space) || (div_i > 0 && !f_has_prefix[i - 1] && !f_needs_multi_space[i - 1] && f_is_div[i - 1]))) {
							w += space_w;							
						}												
						gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_factors_div[i]), 0, 0, w, den_uh - (hpf_div[i] - cpf_div[i]), -1, -1);
						w += wpf_div[i];
						if(f_needs_multi_space[i] && i + 1 < m->countChilds() && f_is_div[i + 1]) {
							w += space_w;
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, den_uh - multi_h / 2 - multi_h % 2, layout_multi);
							w += multi_w;
							w += space_w;
						}								
						div_i++;
					} else if(in_num) {
						if(i == 0 && prepend_one) {
							gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_one), 0, 0, w, num_uh - one_h / 2 - one_h % 2, -1, -1);								
							w += one_w;
						}					
						if(i > 0 && f_needs_multi_space[i] && !f_has_prefix[i - 1] && !f_needs_multi_space[i - 1] && !(i == 1 && (first_is_minus || prefix_fract))) {
							w += space_w;
						}
						if(i == 0 && layout_num) {						
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, num_uh - num_fract_h / 2 - num_fract_h % 2, layout_num);	
							w += num_fract_w;						
							if(f_needs_multi_space[i] && i + 1 < m->countChilds() && !f_is_div[i + 1]) {
								w += space_w;
								gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, num_uh - multi_h / 2 - multi_h % 2, layout_multi);
								w += multi_w;
								w += space_w;
							}							
						} else {						
							gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_factors[i]), 0, 0, w, num_uh - (hpf[i] - cpf[i]), -1, -1);
							w += wpf[i];
							if(f_needs_multi_space[i] && i + 1 < m->countChilds() && !f_is_div[i + 1]) {
								w += space_w;
								gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, num_uh - multi_h / 2 - multi_h % 2, layout_multi);
								w += multi_w;
								w += space_w;
							}							
						}	
					} else {
						i--;
						div_i = 1;
						in_den = false;
						do_div = false;
						w = wfr + w_first_fract;
						w += space_w;
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - multi_h / 2 - multi_h % 2, layout_multi);
						w += multi_w;
						w += space_w;						
					}
				} else {
					if(i == 0 && prepend_one) {
						gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_one), 0, 0, w, uh - one_h / 2 - one_h % 2, -1, -1);								
						w += one_w;
						w += space_w;
					}		
					if(div_i != 1 && div_i != 2 && i > 0 && f_needs_multi_space[i] && !f_has_prefix[i - 1] && !f_needs_multi_space[i - 1] && !(i == 1 && first_is_minus)) {
						w += space_w;
					}
					if(do_simple_unit_div && f_is_div[i]) {
						gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_factors_div[i]), 0, 0, w, uh - (hpf_div[i] - cpf_div[i]), -1, -1);							
						w += wpf_div[i];
					} else 	if(do_simple_unit_div && i == 0 && layout_num) {						
						if(minus) {
							if(toplevel || draw_minus) {
								gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - minus_h / 2 - minus_h % 2, layout_minus);	
								w += minus_w;
							}						
						}
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - num_fract_h / 2 - num_fract_h % 2, layout_num);	
						w += num_fract_w;
					} else {
						gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_factors[i]), 0, 0, w, uh - (hpf[i] - cpf[i]), -1, -1);
						w += wpf[i];
					}
					if(div_i == 2) div_i = 3;
					if(do_simple_unit_div && div_i != 2 && i + 1 < m->countChilds() && f_is_div[i + 1]) {		
						div_i = 2;
						w += space_w;
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - div_h / 2 - div_h % 2, layout_div);
						w += div_w;
						w += space_w;							
						if(layout_den) {
							div_i = 3;						
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - den_fract_h / 2 - den_fract_h % 2, layout_den);	
							w += den_fract_w;
						}
					} else if(f_needs_multi_space[i] && i + 1 < m->countChilds()) {
						w += space_w;
						gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - multi_h / 2 - multi_h % 2, layout_multi);
						w += multi_w;
						w += space_w;
					}
					if(div_i == 1) div_i = 0;					
				}
				if(pixmap_factors[i]) g_object_unref(pixmap_factors[i]);
				if(pixmap_factors_div[i]) g_object_unref(pixmap_factors_div[i]);	
			}
			if(pixmap_one) g_object_unref(pixmap_one);
			if(layout_multi) g_object_unref(layout_multi);
			if(layout_div) g_object_unref(layout_div);
			if(layout_minus) g_object_unref(layout_minus);
			if(layout_num) g_object_unref(layout_num);
			if(layout_den) g_object_unref(layout_den);
		
			break;
		}			
		case ALTERNATIVE_MANAGER: {
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> cpt;			
			vector<gint> hpth;			
			vector<gint> wpt;
			vector<bool> ie;
			gint or_w, or_h, wtmp, htmp, w = 0, h = 0, ctmp = 0, wle, hle, wlae, hlae;
			CALCULATE_SPACE_W
			PangoLayout *layout_almost_equals = gtk_widget_create_pango_layout(resultview, NULL);
			PangoLayout *layout_equals = gtk_widget_create_pango_layout(resultview, NULL);
			pango_layout_set_markup(layout_almost_equals, TEXT_TAGS SIGN_ALMOST_EQUAL " " TEXT_TAGS_END, -1);
			pango_layout_set_markup(layout_equals, TEXT_TAGS "= " TEXT_TAGS_END, -1);
			pango_layout_get_pixel_size(layout_almost_equals, &wlae, &hlae);			
			pango_layout_get_pixel_size(layout_equals, &wle, &hle);			
			PangoLayout *layout_or = gtk_widget_create_pango_layout(resultview, NULL);
			pango_layout_set_markup(layout_or, _(TEXT_TAGS "or" TEXT_TAGS_END), -1);
			pango_layout_get_pixel_size(layout_or, &or_w, &or_h);
			for(int i = 0; i < m->countChilds(); i++) {
				bool bie = false;
				ctmp = 0;
				pixmap_terms.push_back(draw_manager(m->getChild(i), nrformat, displayflags, min_decimals, max_decimals, &bie, usable, prefix, false, NULL, l_exp, in_composite, in_power, true, &ctmp));
				if(bie && in_exact) {
					*in_exact = true;
				}
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[i]), &wtmp, &htmp);
				hpt.push_back(htmp);
				cpt.push_back(ctmp);
				wpt.push_back(wtmp);
				if(bie) {
					wtmp += wlae;
				} else {
					wtmp += wle;
				}
				wtmp += space_w;
				if(wtmp > w) {
					w = 0;
					w += wtmp;
				}
				if(i > 0 && htmp < or_h) {
					htmp = or_h;
				}
				if(bie && hlae > htmp) {
					htmp = hlae;
				} else if(!bie && hle > htmp) {
					htmp = hle;
				}
				hpth.push_back(htmp);			
				ie.push_back(bie);
				h += htmp + 4;
			}
			w += or_w;
			w += space_w * 3;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);			
			h = 0;
			for(int i = 0; i < pixmap_terms.size(); i++) {
				if(i > 0) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, h + hpt[i] - cpt[i] - or_h / 2 - or_h % 2, layout_or);
				}				
				if(ie[i]) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], or_w + space_w * 3, h + hpt[i] - cpt[i] - hlae / 2 - hlae % 2, layout_almost_equals);
				} else {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], or_w + space_w * 3, h + hpt[i] - cpt[i] - hle / 2 - hlae % 2, layout_equals);
				}	
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_terms[i]), 0, 0, w - wpt[i], h, -1, -1);								
				g_object_unref(pixmap_terms[i]);
				h += hpth[i];
				h += 4;				
			}
			g_object_unref(layout_or);
			g_object_unref(layout_almost_equals);
			g_object_unref(layout_equals);
			toplevel = false;
		}		
	}
	if(toplevel && pixmap) {
		gint w, h, wle, hle, w_new, h_new;
		gdk_drawable_get_size(GDK_DRAWABLE(pixmap), &w, &h);			
		GdkPixmap *pixmap_old = pixmap;
		PangoLayout *layout_equals = gtk_widget_create_pango_layout(resultview, NULL);
		if((in_exact && *in_exact) || !m->isPrecise()) {
			pango_layout_set_markup(layout_equals, TEXT_TAGS SIGN_ALMOST_EQUAL " " TEXT_TAGS_END, -1);
		} else {
			pango_layout_set_markup(layout_equals, TEXT_TAGS "= " TEXT_TAGS_END, -1);
		}
		pango_layout_get_pixel_size(layout_equals, &wle, &hle);
		w_new = w + wle;
		h_new = h;
		pixmap = gdk_pixmap_new(resultview->window, w_new, h_new, -1);			
		gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w_new, h_new);	
		gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, h - central_point - hle / 2 - hle % 2, layout_equals);	
		gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_old), 0, 0, wle, 0, -1, -1);
		g_object_unref(pixmap_old);
		g_object_unref(layout_equals);
	}
	if(!pixmap) {
		pixmap = gdk_pixmap_new(resultview->window, 1, 1, -1);
	}
	if(point_central) *point_central = central_point;
	return pixmap;
}
void clearresult() {
	gdk_draw_rectangle(resultview->window, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, resultview->allocation.width, resultview->allocation.height);
}

void on_abort_display(GtkDialog *w, gint arg1, gpointer user_data) {
	while(pthread_cancel(view_thread) == 0) {
		usleep(100);
	}
	CALCULATOR->restoreState();
	CALCULATOR->clearBuffers();
	b_busy = false;
}

void *view_proc(void *x) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	

	int displayflags;
	NumberFormat numberformat;
	int min_decimals, max_decimals;
	
	getFormat(numberformat, displayflags, min_decimals, max_decimals, true, tmp_prefix);
	
	tmp_pixmap = draw_manager(mngr, numberformat, displayflags, min_decimals, max_decimals, &tmp_in_exact, NULL, tmp_prefix);
	
	getFormat(numberformat, displayflags, min_decimals, max_decimals, false, tmp_prefix);
	result_text = mngr->print(numberformat, displayflags, min_decimals, max_decimals, &tmp_in_exact, NULL, tmp_prefix);
	
	b_busy = false;
}
gboolean on_event(GtkWidget *w, GdkEvent *e, gpointer user_data) {
	if(e->type == GDK_EXPOSE || e->type == GDK_PROPERTY_NOTIFY || e->type == GDK_CONFIGURE) {
		return FALSE;
	}
	return TRUE;
}
/*
	set result in result widget and add to history widget
*/
void setResult(const gchar *expr, Prefix *prefix = NULL, bool update_history = true) {

	if(expression_has_changed) {
		execute_expression();
		if(!prefix) return;
	}

	GtkTextIter iter;
	GtkTextBuffer *tb;

	//update "ans" variables
	vans->set(mngr);
	vAns->set(mngr);
	
	pthread_attr_t view_thread_attr;
	pthread_attr_init(&view_thread_attr);
	
	b_busy = true;
	
	if(update_history) {
		result_text = expr;
		gtk_editable_select_region(GTK_EDITABLE(expression), 0, -1);
		display_errors();
		tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "history")));
		gtk_text_buffer_get_start_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, result_text.c_str(), -1);
		gtk_text_buffer_insert(tb, &iter, " ", -1);
		gtk_text_buffer_insert(tb, &iter, "\n", -1);
		gtk_text_buffer_get_iter_at_line_index(tb, &iter, 0, result_text.length() + 1);
		result_text = "?";
	}

	clearresult();

	if(pixmap_result) {
		g_object_unref(pixmap_result);
	}
	pixmap_result = NULL;

	tmp_prefix = prefix;
	tmp_pixmap = NULL;
	tmp_in_exact = false;
	
	CALCULATOR->saveState();
	
	while(gtk_events_pending()) gtk_main_iteration();
	gulong handler_id = g_signal_connect(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")), "event", G_CALLBACK(on_event), NULL);
//	g_object_freeze_notify(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")));
	while(gtk_events_pending()) gtk_main_iteration();
	while(!pthread_create(&view_thread, &view_thread_attr, view_proc, (void*) expr) == 0) {
		usleep(100);
	}	
	int i = 0;
	while(b_busy && i < 50) {
		usleep(10000);
		i++;
	}
	i = 0;
	GtkWidget *dialog = NULL;
	if(b_busy) {
		dialog = glade_xml_get_widget (glade_xml, "progress_dialog");
		gtk_window_set_title(GTK_WINDOW(dialog), "Processing...");
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "progress_label_message")), "Processing...");
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
		g_signal_connect(GTK_OBJECT(dialog), "response", G_CALLBACK(on_abort_display), NULL);
		gtk_widget_show(dialog);
	}
	while(b_busy) {
		while(gtk_events_pending()) gtk_main_iteration();
		usleep(100000);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(glade_xml_get_widget (glade_xml, "progress_progressbar")));
	}
	if(dialog) {
		gtk_widget_hide(dialog);
	}
//	g_object_thaw_notify(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")));
	g_signal_handler_disconnect(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")), handler_id);
	while(gtk_events_pending()) gtk_main_iteration();
	
	gint w = 0, wr = 0, h = 0, hr = 0, h_new, w_new;
	if(!tmp_pixmap) {
		PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
		pango_layout_set_markup(layout, TEXT_TAGS "?" TEXT_TAGS_END, -1);
		pango_layout_get_pixel_size(layout, &w, &h);
		tmp_pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
		gdk_draw_rectangle(tmp_pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
		gdk_draw_layout(GDK_DRAWABLE(tmp_pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
		g_object_unref(layout);
		tmp_in_exact = false;
	}
	gdk_drawable_get_size(GDK_DRAWABLE(tmp_pixmap), &w, &h);	
	gtk_widget_get_size_request(resultview, &wr, &hr);	
	if(h < 50) {
		h_new = 50;
	} else {
		h_new = h;
	}
	if(w < glade_xml_get_widget(glade_xml, "scrolled_result")->allocation.width - 20) {
		w_new = glade_xml_get_widget(glade_xml, "scrolled_result")->allocation.width - 20;
	} else {
		w_new = w;
	}	
	if(wr != w_new || hr != h_new) {
		if(h_new > 200) {
			if(w_new > glade_xml_get_widget(glade_xml, "scrolled_result")->allocation.width - 20) {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(glade_xml, "scrolled_result")), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);		
			} else {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(glade_xml, "scrolled_result")), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
			}
			gtk_widget_set_size_request(glade_xml_get_widget(glade_xml, "scrolled_result"), -1, 200);						
		} else {
			if(w_new > glade_xml_get_widget(glade_xml, "scrolled_result")->allocation.width - 20) {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(glade_xml, "scrolled_result")), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);		
			} else {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(glade_xml, "scrolled_result")), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			}	
			gtk_widget_set_size_request(glade_xml_get_widget(glade_xml, "scrolled_result"), -1, -1);					
		}							
		gtk_widget_set_size_request(resultview, w_new, h_new);
		while(gtk_events_pending()) gtk_main_iteration();
	}
	if(resultview->allocation.width > w) {
		gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(tmp_pixmap), 0, 0, resultview->allocation.width - w, (resultview->allocation.height - h) / 2, -1, -1);
	} else {
		gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(tmp_pixmap), 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1);
	}	
	pixmap_result = tmp_pixmap;
	
	if(update_history) {
		if(result_text.length() > 500000) {
			result_text = "(...)";
		}
		if(!tmp_in_exact) gtk_text_buffer_insert(tb, &iter, "=", -1);	
		else gtk_text_buffer_insert(tb, &iter, SIGN_ALMOST_EQUAL, -1);		
		gtk_text_buffer_insert(tb, &iter, " ", -1);	
		gtk_text_buffer_insert(tb, &iter, result_text.c_str(), -1);
		gtk_text_buffer_place_cursor(tb, &iter);
		while(CALCULATOR->error()) {
			CALCULATOR->nextError();
		}
	}
}

void viewresult(Prefix *prefix = NULL) {
	setResult(NULL, prefix, false);
}

void set_prefix(GtkMenuItem *w, gpointer user_data) {
	setResult(result_text.c_str(), (Prefix*) user_data);
	gtk_widget_grab_focus(expression);	
}

void on_abort_calculation(GtkDialog *w, gint arg1, gpointer user_data) {
	CALCULATOR->abort();
}

/*
	calculate entered expression and display result
*/
void execute_expression() {
	expression_has_changed = false;
	string str = gtk_entry_get_text(GTK_ENTRY(expression));
	//unreference previous result (deletes the object if it is not used anywhere else
	mngr->unref();
	while(gtk_events_pending()) gtk_main_iteration();
	gulong handler_id = g_signal_connect(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")), "event", G_CALLBACK(on_event), NULL);
//	g_object_freeze_notify(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")));
	while(gtk_events_pending()) gtk_main_iteration();
	mngr = CALCULATOR->calculate(str, true);
	int i = 0;
	while(CALCULATOR->busy() && i < 50) {
		usleep(10000);
		i++;
	}
	i = 0;
	GtkWidget *dialog = NULL;	
	if(CALCULATOR->busy()) {
		dialog = glade_xml_get_widget (glade_xml, "progress_dialog");
		gtk_window_set_title(GTK_WINDOW(dialog), "Calculating...");
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "progress_label_message")), "Calculating...");
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
		g_signal_connect(GTK_OBJECT(dialog), "response", G_CALLBACK(on_abort_calculation), NULL);
		gtk_widget_show(dialog);
	}
	while(CALCULATOR->busy()) {
		while(gtk_events_pending()) gtk_main_iteration();
		usleep(100000);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(glade_xml_get_widget (glade_xml, "progress_progressbar")));
	}
	if(dialog) {
		gtk_widget_hide(dialog);
	}
	while(gtk_events_pending()) gtk_main_iteration();
	display_errors();
	setResult(gtk_entry_get_text(GTK_ENTRY(expression)));
//	g_object_thaw_notify(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")));
	g_signal_handler_disconnect(G_OBJECT(glade_xml_get_widget (glade_xml, "main_window")), handler_id);
	while(gtk_events_pending()) gtk_main_iteration();
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
	if(!f) {
		return;
	}
	gint start = 0, end = 0;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end);
	GtkWidget *dialog;
	//if function takes no arguments, do not display dialog and insert function directly
	if(f->args() == 0) {
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
	GtkWidget *vbox_pre = gtk_vbox_new(FALSE, 12);
	gtk_container_set_border_width(GTK_CONTAINER(vbox_pre), 12);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox_pre);
	f_title.insert(0, "<b>");
	f_title += "</b>";
	GtkWidget *title_label = gtk_label_new(f_title.c_str());
	gtk_label_set_use_markup(GTK_LABEL(title_label), TRUE);
	gtk_misc_set_alignment(GTK_MISC(title_label), 0.0, 0.5);
	gtk_container_add(GTK_CONTAINER(vbox_pre), title_label);	
	int args;
	bool has_vector = false;
	if(f->args() > 0) {
		args = f->args();
	} else if(f->minargs() > 0) {
		args = f->minargs() + 1;
		has_vector = true;
	} else {
		args = 1;
		has_vector = true;
	}
	GtkWidget *table = gtk_table_new(3, args, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 6);
	gtk_table_set_col_spacings(GTK_TABLE(table), 6);
	gtk_container_add(GTK_CONTAINER(vbox_pre), table);	
	GtkWidget *label[args];
	GtkWidget *entry[args];
	GtkWidget *type_label[args];		
	GtkWidget *descr;
	string argstr, typestr; 
	string argtype;
	//create argument entries
	Argument *arg;
	for(int i = 0; i < args; i++) {
		arg = f->getArgumentDefinition(i + 1);
		if(!arg || arg->name().empty()) {
			argstr = _("Argument");
			argstr += " ";
			argstr += i2s(i + 1);
		} else {
			argstr = arg->name();
		}
		typestr = "";
		argtype = "";
		label[i] = gtk_label_new(argstr.c_str());
		gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
		if(arg)
		switch(arg->type()) {
			case ARGUMENT_TYPE_INTEGER: {
				IntegerArgument *iarg = (IntegerArgument*) arg;
				gdouble min = -1000000, max = 1000000;
				if(iarg->min()) {
					min = iarg->min()->getInt();
				}
				if(iarg->max()) {
					max = iarg->max()->getInt();
				}				
				entry[i] = gtk_spin_button_new_with_range(min, max, 1);
				gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(entry[i]), FALSE);
				if(!arg->zeroForbidden() && min <= 0 && max >= 0) {
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry[i]), 0);
				} else {
					if(max < 0) {
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry[i]), max);
					} else if(min <= 1) {
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry[i]), 1);
					} else {
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry[i]), min);
					}
				}
				break;
			}
			case ARGUMENT_TYPE_BOOLEAN: {
				entry[i] = gtk_spin_button_new_with_range(0, 1, 1);
				gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(entry[i]), FALSE);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry[i]), 0);
				break;
			}									
			default: {
				if(i >= f->minargs() && !has_vector) {
					typestr = "(";
					typestr += _("optional");
				}
				argtype = arg->print();		
				if(typestr.empty()) {
					typestr = "(";
				} else if(!argtype.empty()) {
					typestr += " ";
				}
				if(!argtype.empty()) {
					typestr += argtype;
				}
				typestr += ")";		
				if(typestr.length() == 2) {
					typestr = "";
				}
			
				entry[i] = gtk_entry_new();
			}
		}
		if(typestr.empty() && i >= f->minargs() && !has_vector) {
			typestr = "(";
			typestr += _("optional");
			typestr += ")";			
		}		
		if(arg)
		switch(arg->type()) {		
			case ARGUMENT_TYPE_DATE: {
				typestr = typestr.substr(1, typestr.length() - 2);
				type_label[i] = gtk_button_new_with_label(typestr.c_str());
				g_signal_connect((gpointer) type_label[i], "clicked", G_CALLBACK(on_type_label_date_clicked), (gpointer) entry[i]);
				break;
			}
			default: {
				type_label[i] = gtk_label_new(typestr.c_str());		
				gtk_misc_set_alignment(GTK_MISC(type_label[i]), 1, 0.5);
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry[i]), f->getDefaultValue(i + 1).c_str());
		//insert selection in expression entry into the first argument entry
		if(i == 0) {
			gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
			gtk_entry_set_text(GTK_ENTRY(entry[i]), gstr);
			g_free(gstr);
		}
		gtk_table_attach(GTK_TABLE(table), label[i], 0, 1, i, i + 1, (GtkAttachOptions) (GTK_FILL), GTK_FILL, 0, 0);
		gtk_table_attach(GTK_TABLE(table), entry[i], 1, 2, i, i + 1, (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		gtk_table_attach(GTK_TABLE(table), type_label[i], 2, 3, i, i + 1, (GtkAttachOptions) (GTK_FILL), GTK_FILL, 0, 0);
	}
	//display function description
	if(!f->description().empty()) {
		descr = gtk_label_new(f->description().c_str());
		gtk_label_set_line_wrap(GTK_LABEL(descr), TRUE);
		gtk_container_add(GTK_CONTAINER(vbox_pre), descr);
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
			}
			if(!(i >= f->minargs() && str2.empty()) && f->getArgumentDefinition(i + 1) && f->getArgumentDefinition(i + 1)->needQuotes()) {
				if(str2.length() < 1 || (str2[0] != '\"' && str[0] != '\'')) { 
					str2.insert(0, "\"");
					str2 += "\"";
				}
			}
			if(i > 0) {
				str += CALCULATOR->getComma();
			}
			str += str2;
		}
		str += RIGHT_BRACKET_STR;
		
		//redo selection if "OK" was clicked, clear expression entry "Execute"
		if(response == GTK_RESPONSE_ACCEPT) {
			gtk_editable_select_region(GTK_EDITABLE(expression), start, end);
		} else {
			gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
		}
		
		insert_text(str.c_str());
		//Calculate directly when "Execute" was clicked
		if(response == GTK_RESPONSE_APPLY)
			execute_expression();
	}
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
	if(use_short_units) {
		insert_text(((Unit*) user_data)->shortName().c_str());
	} else {
		insert_text(((Unit*) user_data)->plural().c_str());
	}
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
		if(u->isLocal())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unit"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unit (global)"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Unit"));
	}

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category")), category);

	//clear entries
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")), "");	
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), "");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_spinbutton_exp")), 1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_reversed")), "");

	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "unit_edit_button_ok"), TRUE);

	if(u) {
		//fill in original parameters
		if(u->unitType() == BASE_UNIT)
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), UNIT_CLASS_BASE_UNIT);
		else if(u->unitType() == ALIAS_UNIT)
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), UNIT_CLASS_ALIAS_UNIT);
		else if(u->unitType() == COMPOSITE_UNIT)
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), UNIT_CLASS_COMPOSITE_UNIT);

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name")), u->name().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")), u->name().c_str());		

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural")), u->plural(false).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")), u->singular(false).c_str());

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category")), u->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc")), u->title(false).c_str());

		switch(u->unitType()) {
			case ALIAS_UNIT: {
				AliasUnit *au = (AliasUnit*) u;
				if(au->firstBaseUnit()->unitType() == COMPOSITE_UNIT) {
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), ((CompositeUnit*) (au->firstBaseUnit()))->referenceName().c_str());
				} else {
					if(use_short_units)
						gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), au->firstBaseUnit()->shortName().c_str());
					else
						gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), au->firstBaseUnit()->singular().c_str());	
				}		
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_spinbutton_exp")), au->firstBaseExp());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation")), au->expression().c_str());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_reversed")), au->reverseExpression().c_str());
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_checkbutton_exact")), au->isPrecise());
				break;
			}
			case COMPOSITE_UNIT: {
				if(use_short_units) {
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), u->shortName().c_str());
				} else {
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base")), u->singular().c_str());
				}
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")), ((CompositeUnit*) u)->referenceName().c_str());			
			}
		}

	} else {
		//default values
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_checkbutton_exact")), TRUE);
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), UNIT_CLASS_ALIAS_UNIT);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation")), "1");
	}
	on_unit_edit_optionmenu_class_changed(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")), NULL);

run_unit_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		int type = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")));		
		string str;
		if(type == UNIT_CLASS_COMPOSITE_UNIT) str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")));
		else str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name given
			if(type == UNIT_CLASS_COMPOSITE_UNIT) {
				show_message(_("Empty internal name field."), dialog);
				goto run_unit_edit_dialog;			
			} else {
				str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")));
				remove_blank_ends(str);
				if(str.empty()) {
					//no singular unit name either -- open dialog again
					show_message(_("Empty name field."), dialog);
					goto run_unit_edit_dialog;
				} else {
					//switch singular name and name
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name")), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))));
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")), "");
				}
			}
		}

		//unit with the same name exists -- overwrite or open the dialog again
		if(type == UNIT_CLASS_COMPOSITE_UNIT) {
			if((!u || u->referenceName() != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal")))) && CALCULATOR->nameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal"))), u) && !ask_question(_("A unit with the same internal name already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}		
		} else {
			if((!u || u->name() != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name")))) && CALCULATOR->nameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name"))), u) && !ask_question(_("A unit with the same name already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}
			if((!u || u->plural(false) != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural")))) && CALCULATOR->nameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))), u) && !ask_question(_("A unit with the same plural name already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}
			if((!u || u->singular(false) != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular")))) && CALCULATOR->nameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))), u) && !ask_question(_("A unit with the same singular name already exists.\nOverwrite unit?"), dialog)) {
				goto run_unit_edit_dialog;
			}
		}
		if(u) {
			//edited an existing unit -- update unit
			u->setLocal(true);
			gint i1 = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")));
			switch(u->unitType()) {
				case ALIAS_UNIT: {
					if(i1 != UNIT_CLASS_ALIAS_UNIT) {
						u->destroy();
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
					au->setPrecise(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_checkbutton_exact"))));
					break;
				}
				case COMPOSITE_UNIT: {
					if(i1 != UNIT_CLASS_COMPOSITE_UNIT) {
						u->destroy();
						u = NULL;
						break;
					}
					u->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal"))));
					((CompositeUnit*) u)->setBaseExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));					
					break;
				}
				case BASE_UNIT: {
					if(i1 != UNIT_CLASS_BASE_UNIT) {
						u->destroy();
						u = NULL;
						break;
					}
					break;
				}
			}
			if(u && u->unitType() != COMPOSITE_UNIT) {
				u->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name"))));
				u->setPlural(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))));
				u->setSingular(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))));
			}
			u->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))));
			u->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))));			
		}
		if(!u) {
			//new unit
			switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "unit_edit_optionmenu_class")))) {
				case UNIT_CLASS_ALIAS_UNIT: {
					Unit *bu = CALCULATOR->getUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));
					if(!bu) bu = CALCULATOR->getCompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))));
					if(!bu) {
						show_message(_("Base unit does not exist."), dialog);
						goto run_unit_edit_dialog;
					}
					u = new AliasUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))), bu, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_relation"))), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_spinbutton_exp"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_reversed"))), true);
					((AliasUnit*) u)->setPrecise(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "unit_edit_checkbutton_exact"))));
					break;
				}
				case UNIT_CLASS_COMPOSITE_UNIT: {
					CompositeUnit *cu = new CompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_internal"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_base"))), true);
					u = cu;
					break;
				}
				default: {
					u = new Unit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_name"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_plural"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_singular"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_edit_entry_desc"))), true);
					break;
				}
			}
			if(u) {
				CALCULATOR->addUnit(u);
			}
		}
		//select the new unit
		selected_unit = u;
		selected_unit_category = u->category();
		if(selected_unit_category.empty()) {
			selected_unit_category = _("Uncategorized");
		}
		if(!u->isActive()) {
			selected_unit_category = _("Inactive");
		}
		update_umenus();
	}
	gtk_widget_hide(dialog);
}

void edit_argument(Argument *arg) {
	if(!arg) {
		arg = new Argument();
	}
	edited_argument = arg;
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "argument_rules_dialog");	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "function_edit_dialog")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_test")), arg->tests());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_allow_matrix")), arg->matrixAllowed());
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_allow_matrix"), TRUE);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "argument_rules_entry_condition")), arg->getCustomCondition().c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_condition")), !arg->getCustomCondition().empty());
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_entry_condition"), !arg->getCustomCondition().empty());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_forbid_zero")), arg->zeroForbidden());
	switch(arg->type()) {
		case ARGUMENT_TYPE_FRACTION: {
			FractionArgument *farg = (FractionArgument*) arg;
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_box_min"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_min")), farg->min() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_min_include_equals")), farg->includeEqualsMin());
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min"), farg->min() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_min_include_equals"), TRUE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min")), FALSE);
			if(farg->min()) {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min")), farg->min()->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY | DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT).c_str());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min")), 0);
			}
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_box_max"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_max")), farg->max() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_max_include_equals")), farg->includeEqualsMax());
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max"), farg->max() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_max_include_equals"), TRUE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max")), FALSE);
			if(farg->max()) {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max")), farg->max()->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY | DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT).c_str());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max")), 0);
			}			
			break;
		}
		case ARGUMENT_TYPE_INTEGER: {
			IntegerArgument *iarg = (IntegerArgument*) arg;
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_box_min"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_min")), iarg->min() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_min_include_equals")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min"), iarg->min() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_min_include_equals"), FALSE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min")), TRUE);
			if(iarg->min()) {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min")), iarg->min()->getInt());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min")), 0);
			}
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_box_max"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_max")), iarg->max() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_max_include_equals")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max"), iarg->max() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_max_include_equals"), FALSE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max")), TRUE);
			if(iarg->max()) {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max")), iarg->max()->getInt());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max")), 0);
			}	
			break;
		}
		case ARGUMENT_TYPE_FREE: {}
		case ARGUMENT_TYPE_MATRIX: {		
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_allow_matrix")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_allow_matrix"), FALSE);	
		}
		default: {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_min")), FALSE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_max")), FALSE);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min")), 0);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max")), 0);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_box_min"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_box_max"), FALSE);
		}
	}
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		arg->setTests(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_test"))));
		arg->setMatrixAllowed(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_allow_matrix"))));
		arg->setZeroForbidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_forbid_zero"))));
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_condition")))) {
			arg->setCustomCondition(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "argument_rules_entry_condition"))));
		} else {
			arg->setCustomCondition("");
		}
		if(arg->type() == ARGUMENT_TYPE_FRACTION) {
			FractionArgument *farg = (FractionArgument*) arg;
			farg->setIncludeEqualsMin(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_min_include_equals"))));
			farg->setIncludeEqualsMax(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_max_include_equals"))));
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_min")))) {
				Manager *mngr2 = CALCULATOR->calculate(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min"))));
				farg->setMin(mngr2->fraction());
				mngr2->unref();
			} else {
				farg->setMin(NULL);
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_max")))) {
				Manager *mngr2 = CALCULATOR->calculate(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max"))));
				farg->setMax(mngr2->fraction());
				mngr2->unref();
			} else {
				farg->setMax(NULL);
			}			
		} else if(arg->type() == ARGUMENT_TYPE_INTEGER) {
			IntegerArgument *iarg = (IntegerArgument*) arg;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_min")))) {
				Integer integ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min"))));
				iarg->setMin(&integ);
			} else {
				iarg->setMin(NULL);
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_checkbutton_enable_max")))) {
				Integer integ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max"))));
				iarg->setMax(&integ);
			} else {
				iarg->setMax(NULL);
			}			
		}
		GtkTreeModel *model;
		GtkTreeIter iter;
		GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionArguments));
		if(gtk_tree_selection_get_selected(select, &model, &iter)) {
			gtk_list_store_set(tFunctionArguments_store, &iter, 0, arg->name().c_str(), 1, arg->printlong().c_str(), 2, (gpointer) arg, -1);
		}
	}
	edited_argument = NULL;
	gtk_widget_hide(dialog);
}

/*
	display edit/new function dialog
	creates new function if f == NULL, win is parent window
*/
void edit_function(const char *category = "", Function *f = NULL, GtkWidget *win = NULL) {

	GtkWidget *dialog = create_function_edit_dialog();	
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	edited_function = f;
	
	if(f) {
		if(f->isLocal())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Function"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Function (global)"));		
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Function"));
	}

	GtkTextBuffer *description_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "function_edit_textview_description")));
	GtkTextBuffer *expression_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (glade_xml, "function_edit_textview_expression")));	

	//clear entries
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_condition")), "");
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_entry_name"), !f || !f->isBuiltin());
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_textview_expression"), !f || !f->isBuiltin());
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category")), category);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc")), "");
	gtk_text_buffer_set_text(description_buffer, "", -1);
	gtk_text_buffer_set_text(expression_buffer, "", -1);

	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_ok"), TRUE);	
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_optionmenu_argument_type"), !f || !f->isBuiltin());
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "function_edit_button_add_argument"), !f || !f->isBuiltin());

	if(f) {
		//fill in original paramaters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_name")), f->name().c_str());
		if(!f->isBuiltin()) {
			gtk_text_buffer_set_text(expression_buffer, ((UserFunction*) f)->equation().c_str(), -1);
		}
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category")), f->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc")), f->title(false).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_condition")), f->condition().c_str());
		gtk_text_buffer_set_text(description_buffer, f->description().c_str(), -1);
	}
	update_function_arguments_list(f);

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
		GtkTextIter e_iter_s, e_iter_e;
		gtk_text_buffer_get_start_iter(expression_buffer, &e_iter_s);
		gtk_text_buffer_get_end_iter(expression_buffer, &e_iter_e);		
		string str2 = gtk_text_buffer_get_text(expression_buffer, &e_iter_s, &e_iter_e, FALSE);
		remove_blank_ends(str2);
		gsub("\n", " ", str2);
		if(!(f && f->isBuiltin()) && str2.empty()) {
			//no expression/relation -- open dialog again
			show_message(_("Empty expression field."), dialog);
			goto run_function_edit_dialog;
		}
		GtkTextIter d_iter_s, d_iter_e;
		gtk_text_buffer_get_start_iter(description_buffer, &d_iter_s);
		gtk_text_buffer_get_end_iter(description_buffer, &d_iter_e);
		//function with the same name exists -- overwrite or open the dialog again
		if((!f || str != f->name()) && CALCULATOR->nameTaken(str, f) && !ask_question(_("A function or variable with the same name already exists.\nOverwrite function/variable?"), dialog)) {
			goto run_function_edit_dialog;
		}	
		if(f) {
			f->setLocal(true);
			//edited an existing function
			if(!f->isBuiltin()) {
				f->setName(str);
			}
			f->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category"))));
			f->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc"))));
			f->setDescription(gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE));
			if(!f->isBuiltin()) {
				((UserFunction*) f)->setEquation(str2);
				f->clearArgumentDefinitions();
			}	
		} else {
			//new function
			f = new UserFunction(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_category"))), str, str2, true, -1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_desc"))), gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE));
			CALCULATOR->addFunction(f);
		}
		if(f) {
			f->setCondition(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_condition"))));
			GtkTreeIter iter;
			bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tFunctionArguments_store), &iter);
			int i = 1;
			Argument *arg;
			while(b) {
				gtk_tree_model_get(GTK_TREE_MODEL(tFunctionArguments_store), &iter, 2, &arg, -1);
				if(arg && f->isBuiltin() && f->getArgumentDefinition(i)) {
					f->getArgumentDefinition(i)->setName(arg->name());
					delete arg;
				} else if(arg) {
					f->setArgumentDefinition(i, arg);
				}
				b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tFunctionArguments_store), &iter);
				i++;
			}		
		}
		//select the new function
		selected_function = f;
		selected_function_category = f->category();
		if(selected_function_category.empty()) {
			selected_function_category = _("Uncategorized");
		}
		if(!f->isActive()) {
			selected_function_category = _("Inactive");
		}		
		update_fmenu();	
	}
	edited_function = NULL;
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
	CALCULATOR->convert(mngr, u);
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}

void convert_to_custom_unit(GtkMenuItem *w, gpointer user_data)
{
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "unit_dialog");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	int i;
	while((i = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_OK || i == GTK_RESPONSE_APPLY) {
		CALCULATOR->convert(mngr, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "unit_dialog_entry_unit"))));
		setResult(result_text.c_str());
		if(i == GTK_RESPONSE_OK) break;
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	display edit/new variable dialog
	creates new variable if v == NULL, mngr_ is forced value, win is parent window
*/
void edit_variable(const char *category, Variable *v, Manager *mngr_, GtkWidget *win) {

	if((v != NULL && v->get()->isMatrix() && (!mngr_ || mngr_->isMatrix())) || (mngr_ && !v && mngr_->isMatrix())) {
		edit_matrix(category, v, mngr_, win);
		return;
	}

	GtkWidget *dialog = create_variable_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	if(v) {
		if(v->isLocal())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Variable"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Variable (global)"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Variable"));
	}
		
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_button_ok"), TRUE);		

	if(v) {
		//fill in original parameters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_name")), v->name().c_str());
		bool in_exact = false;
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_value")), get_value_string(v->get(), false, NULL, &in_exact).c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact")), !in_exact);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_name"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_value"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact"), !v->isBuiltin());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc")), v->title(false).c_str());
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_name"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_value"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact"), TRUE);
	
		//fill in default values
		string v_name = "x";
		if(CALCULATOR->nameTaken("x")) {
			if(!CALCULATOR->nameTaken("y")) {
				v_name = "y";
			} else if(!CALCULATOR->nameTaken("z")) {
				v_name = "z";
			} else {
				v_name = CALCULATOR->getName();
			}
		}
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_name")), v_name.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_value")), get_value_string(mngr).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc")), "");		
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact")), TRUE);				
	}
	if(mngr_) {
		//forced value
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_entry_value"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact"), FALSE);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_value")), get_value_string(mngr_).c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact")), mngr_->isPrecise());
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
		if((!v || str != v->name()) && CALCULATOR->nameTaken(str, v) && !ask_question(_("A function or variable with the same name already exists.\nOverwrite function/variable?"), dialog)) {
			goto run_variable_edit_dialog;
		}
		if(!v) {
			//no need to create a new variable when a variable with the same name exists
			v = CALCULATOR->getVariable(str);
		}
		if(v) {
			//update existing variable
			v->setLocal(true);
			if(!v->isBuiltin()) {
				if(mngr_) {
					v->set(mngr_);
				} else {
					v->set(str2);
				}
				v->setName(str);
				v->setPrecise(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact"))));
			}
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc"))));
		} else {
			//new variable
			if(mngr_) {
				//forced value
				v = CALCULATOR->addVariable(new Variable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category"))), str, mngr_, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc"))), true));
			} else {
				v = CALCULATOR->addVariable(new Variable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_category"))), str, str2, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "variable_edit_entry_desc"))), true));
				v->setPrecise(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "variable_edit_checkbutton_exact"))));
			}
		}
		//select the new variable
		selected_variable = v;
		selected_variable_category = v->category();
		if(selected_variable_category.empty()) {
			selected_variable_category = _("Uncategorized");
		}
		if(!v->isActive()) {
			selected_variable_category = _("Inactive");
		}		
		update_vmenu();
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	display edit/new matrix dialog
	creates new matrix if v == NULL, mngr_ is forced value, win is parent window
*/
void edit_matrix(const char *category, Variable *v, Manager *mngr_, GtkWidget *win, gboolean create_vector) {

	if((v && !v->get()->isMatrix()) || (mngr_ && !mngr_->isMatrix())) {
		edit_variable(category, v, mngr_, win);
		return;
	}

	GtkWidget *dialog = create_matrix_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	if(mngr_) {
		create_vector = mngr_->matrix()->isVector();
	} else if(v) {
		create_vector = v->get()->matrix()->isVector();	
	}
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_vector")), create_vector);				

	if(create_vector) {
		if(v) {
			if(v->isLocal())
				gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Vector"));
			else
				gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Vector (global)"));
		} else {
			gtk_window_set_title(GTK_WINDOW(dialog), _("New Vector"));
		}
	} else {
		if(v) {
			if(v->isLocal())
				gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Matrix"));
			else
				gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Matrix (global)"));
		} else {
			gtk_window_set_title(GTK_WINDOW(dialog), _("New Matrix"));
		}	
	}
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_button_ok"), TRUE);		

	int r = 4, c = 4;
	Vector *old_vctr = NULL;
	if(v) {
		if(create_vector) {
			old_vctr = (Vector*) v->get()->matrix();
		}	
		//fill in original parameters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_name")), v->name().c_str());
		//can only change name and value of user variable
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_entry_name"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_rows"), !v->isBuiltin());		
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_columns"), !v->isBuiltin());				
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_table_elements"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_matrix"), !v->isBuiltin());						
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_vector"), !v->isBuiltin());								
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_desc")), v->title(false).c_str());
		c = v->get()->matrix()->columns();
		r = v->get()->matrix()->rows();
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_entry_name"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_rows"), TRUE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_columns"), TRUE);				
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_table_elements"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_matrix"), TRUE);						
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_vector"), TRUE);								
	
		//fill in default values
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_name")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_desc")), "");		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_rows")), 3);		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_columns")), 3);						
	}
	if(mngr_) {
		//forced value
		if(create_vector) {
			old_vctr = (Vector*) mngr_->matrix();
		}
		c = mngr_->matrix()->columns();
		r = mngr_->matrix()->rows();				
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_rows"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_columns"), FALSE);				
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_table_elements"), FALSE);						
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_matrix"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_vector"), FALSE);		
	}

	if(create_vector) {
		if(old_vctr) {
			r = old_vctr->components();
			c = (int) ::sqrt(r) + 4;
			if(r % c > 0) {
				r = r / c + 1;
			} else {
				r = r / c;
			}
		} else {
			c = 8;
			r = 3;
		}
	}

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_rows")), r);		
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_columns")), c);					
	on_matrix_edit_spinbutton_columns_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_columns")), NULL);
	on_matrix_edit_spinbutton_rows_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_spinbutton_rows")), NULL);		

	while(gtk_events_pending()) gtk_main_iteration();
	for(int index_r = 0; index_r < element_entries.size(); index_r++) {
		for(int index_c = 0; index_c < element_entries[index_r].size(); index_c++) {
			if(create_vector) {
				if(old_vctr && index_r * element_entries[index_r].size() + index_c < old_vctr->components()) {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), old_vctr->get(index_r * element_entries[index_r].size() + index_c + 1)->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY | DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT).c_str());
				} else {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), "");
				}
			} else {
				if(v) {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), v->get()->matrix()->get(index_r + 1, index_c + 1)->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY | DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT).c_str());
				} else if(mngr_) {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), mngr_->matrix()->get(index_r + 1, index_c + 1)->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY | DISPLAY_FORMAT_ALWAYS_DISPLAY_EXACT).c_str());			
				} else {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), "0");
				}
			}
		}
	}		
run_matrix_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_name")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name -- open dialog again
			show_message(_("Empty name field."), dialog);
			goto run_matrix_edit_dialog;
		}

		//variable with the same name exists -- overwrite or open dialog again
		if((!v || str != v->name()) && CALCULATOR->nameTaken(str) && !ask_question(_("A function or variable with the same name already exists.\nOverwrite function/variable?"), dialog)) {
			goto run_matrix_edit_dialog;
		}
		if(!v) {
			//no need to create a new variable when a variable with the same name exists
			v = CALCULATOR->getVariable(str);
		}
		Matrix *mtrx;
		if(!mngr_) {
			Manager *mngr_v;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_vector")))) {
				Vector *vctr = new Vector();
				string str;
				for(int index_r = 0; index_r < element_entries.size(); index_r++) {
					for(int index_c = 0; index_c < element_entries[index_r].size(); index_c++) {
						str = gtk_entry_get_text(GTK_ENTRY(element_entries[index_r][index_c]));
						remove_blank_ends(str);
						if(!str.empty()) {
							mngr_v = CALCULATOR->calculate(str);
							if(index_r || index_c) vctr->addComponent();
							vctr->set(mngr_v, vctr->components());
							mngr_v->unref();
						}
					}
				}
				mtrx = vctr;
			} else {
				mtrx = new Matrix(element_entries.size(), element_entries[0].size());
				for(int index_r = 0; index_r < element_entries.size(); index_r++) {
					for(int index_c = 0; index_c < element_entries[index_r].size(); index_c++) {
						mngr_v = CALCULATOR->calculate(gtk_entry_get_text(GTK_ENTRY(element_entries[index_r][index_c])));
						mtrx->set(mngr_v, index_r + 1, index_c + 1);
						mngr_v->unref();
					}
				}
			}					
		}		
		if(v) {
			v->setLocal(true);
			//update existing variable
			if(!v->isBuiltin()) {
				if(mngr_) {
					v->set(mngr_);
				} else {
					mngr_ = new Manager(mtrx);
					v->set(mngr_);
					mngr_->unref();
					delete mtrx;
				}
				v->setName(str);
			}
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_desc"))));
		} else {
			//new variable
			if(mngr_) {
				//forced value
				v = CALCULATOR->addVariable(new Variable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_category"))), str, mngr_, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_desc"))), true));
			} else {
				mngr_ = new Manager(mtrx);
				v = CALCULATOR->addVariable(new Variable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_category"))), str, mngr_, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "matrix_edit_entry_desc"))), true));
				mngr_->unref();
				delete mtrx;
			}
		}
		//select the new variable
		selected_variable = v;
		selected_variable_category = v->category();
		if(selected_variable_category.empty()) {
			selected_variable_category = _("Uncategorized");
		}
		if(!v->isActive()) {
			selected_variable_category = _("Inactive");
		}		
		update_vmenu();
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

void import_csv_file(GtkWidget *win) {

	GtkWidget *dialog = create_csv_import_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_file")), "");	
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_desc")), "");

run_csv_import_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_file")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no filename -- open dialog again
			show_message(_("No file name entered."), dialog);
			goto run_csv_import_dialog;
		}
		string delimiter = "";
		switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "csv_import_optionmenu_delimiter")))) {
			case DELIMITER_COMMA: {
				delimiter = ",";
				break;
			}
			case DELIMITER_TABULATOR: {
				delimiter = "\t";
				break;
			}			
			case DELIMITER_SEMICOLON: {
				delimiter = ";";
				break;
			}		
			case DELIMITER_SPACE: {
				delimiter = " ";
				break;
			}				
			case DELIMITER_OTHER: {
				delimiter = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_delimiter_other")));
				break;
			}			
		}
		if(delimiter.empty()) {
			//no filename -- open dialog again
			show_message(_("No delimiter selected."), dialog);
			goto run_csv_import_dialog;
		}		
		if(!CALCULATOR->importCSV(str.c_str(), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "csv_import_spinbutton_first_row"))), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "csv_import_checkbutton_headers"))), delimiter, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "csv_import_radiobutton_matrix"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_name"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_desc"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_category"))))) {
			GtkWidget *edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (glade_xml, "main_window")
				),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Could not import from file \n%s"),
				str.c_str()
			);
			gtk_dialog_run(GTK_DIALOG(edialog));
			gtk_widget_destroy(edialog);
		}
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
	add a new matrix (from menu)
*/
void new_matrix(GtkMenuItem *w, gpointer user_data)
{
	edit_matrix(
			_("Matrices"),
			NULL,
			NULL,
			glade_xml_get_widget (glade_xml, "main_window"), FALSE);
}
/*
	add a new vector (from menu)
*/
void new_vector(GtkMenuItem *w, gpointer user_data)
{
	edit_matrix(
			_("Vectors"),
			NULL,
			NULL,
			glade_xml_get_widget (glade_xml, "main_window"), TRUE);
}

void on_import_csv_file_activated(GtkMenuItem *w, gpointer user_data) {
	import_csv_file(glade_xml_get_widget (glade_xml, "main_window"));
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
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_radians_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_radians_activate, NULL);						
			break;
		}
	case GRADIANS: {
			mi = glade_xml_get_widget (glade_xml, "menu_item_gradians");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_gradians_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_gradians_activate, NULL);						
			break;
		}
	case DEGREES: {
			mi = glade_xml_get_widget (glade_xml, "menu_item_degrees");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_degrees_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_degrees_activate, NULL);						
			break;
		}
	}
}

/*
	Update angle radio buttons
*/
void set_angle_button() {
	GtkWidget *tb = NULL;
	switch(CALCULATOR->angleMode()) {
	case RADIANS: {
			tb = glade_xml_get_widget (glade_xml, "radiobutton_radians");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_radians_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_radians_toggled, NULL);							
			break;
		}
	case GRADIANS: {
			tb = glade_xml_get_widget (glade_xml, "radiobutton_gradians");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_gradians_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_gradians_toggled, NULL);							
			break;
		}
	case DEGREES: {
			tb = glade_xml_get_widget (glade_xml, "radiobutton_degrees");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_degrees_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_degrees_toggled, NULL);							
			break;
		}
	}

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
	bool b_always_exact = CALCULATOR->alwaysExact();
	CALCULATOR->setAlwaysExact(false);
	if(uFrom && uTo) {
		//values
		const gchar *fromValue = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_from_val")));
		const gchar *toValue = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_to_val")));
		//determine conversion direction
		if(toFrom > 0) {
			Manager *v_mngr = CALCULATOR->convert(toValue, uTo, uFrom);
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_from_val")), v_mngr->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY).c_str());
			mngr->unref();
		} else {
			Manager *v_mngr = CALCULATOR->convert(fromValue, uFrom, uTo);
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "units_entry_to_val")), v_mngr->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY).c_str());
			mngr->unref();
		}
	}
	CALCULATOR->setAlwaysExact(b_always_exact);
}

/*
	save definitions to ~/.qalculate/qalculate.cfg
	the hard work is done in the Calculator class
*/
void save_defs() {
	if(!CALCULATOR->saveDefinitions()) {
		GtkWidget *edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (glade_xml, "main_window")
				),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Couldn't write definitions"));
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
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
	saved_use_min_deci = use_min_deci;
	saved_min_deci = min_deci;
	saved_use_max_deci = use_max_deci;
	saved_max_deci = max_deci;	
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
	saved_fractional_mode = fractional_mode;
	saved_use_prefixes = use_prefixes;
	saved_indicate_infinite_series = indicate_infinite_series;
	saved_always_exact = CALCULATOR->alwaysExact();
	saved_in_rpn_mode = CALCULATOR->inRPNMode();
}

/*
	load preferences from ~/.qalculate/qalculate-gtk.cfg
*/
void load_preferences() {

	use_min_deci = false;
	min_deci = 0;
	use_max_deci = false;
	max_deci = 2;	
	display_mode = MODE_NORMAL;
	number_base = BASE_DECI;
	save_mode_on_exit = true;
	save_defs_on_exit = true;
	indicate_infinite_series = false;
	hyp_is_on = false;
	fractional_mode = FRACTIONAL_MODE_DECIMAL;
	use_custom_font = false;
	custom_font = "";
	show_more = false;
	show_buttons = false;
	use_short_units = true;
	use_unicode_signs = true;
	use_prefixes = true;
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
			remove_blank_ends(stmp);
			if((i = stmp.find_first_of("=")) != string::npos) {
				svar = stmp.substr(0, i);
				remove_blank_ends(svar);
				svalue = stmp.substr(i + 1, stmp.length() - (i + 1));
				remove_blank_ends(svalue);
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
				else if(svar == "min_deci")
					min_deci = v;
				else if(svar == "use_min_deci")
					use_min_deci = v;
				else if(svar == "max_deci")
					max_deci = v;
				else if(svar == "use_max_deci")
					use_max_deci = v;					
				else if(svar == "precision")
					CALCULATOR->setPrecision(v);
				else if(svar == "display_mode")
					display_mode = v;
				else if(svar == "use_prefixes")
					use_prefixes = v;					
				else if(svar == "fractional_mode")
					fractional_mode = v;					
				else if(svar == "number_base")
					number_base = v;
				else if(svar == "angle_unit")
					CALCULATOR->angleMode(v);
				else if(svar == "hyp_is_on")
					hyp_is_on = v;
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
				else if(svar == "use_custom_font")
					use_custom_font = v;										
				else if(svar == "custom_font")
					custom_font = svalue;
				else if(svar == "indicate_infinite_series")
					indicate_infinite_series = v;											
				else if(svar == "always_exact")
					CALCULATOR->setAlwaysExact(v);					
				else if(svar == "in_rpn_mode")
					CALCULATOR->setRPNMode(v);										
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
	fprintf(file, "use_custom_font=%i\n", use_custom_font);	
	fprintf(file, "custom_font=%s\n", custom_font.c_str());		
	if(mode)
		set_saved_mode();
	fprintf(file, "\n[Mode]\n");
	fprintf(file, "min_deci=%i\n", saved_min_deci);
	fprintf(file, "use_min_deci=%i\n", saved_use_min_deci);
	fprintf(file, "max_deci=%i\n", saved_max_deci);
	fprintf(file, "use_max_deci=%i\n", saved_use_max_deci);	
	fprintf(file, "precision=%i\n", saved_precision);
	fprintf(file, "display_mode=%i\n", saved_display_mode);
	fprintf(file, "fractional_mode=%i\n", saved_fractional_mode);	
	fprintf(file, "use_prefixes=%i\n", saved_use_prefixes);	
	fprintf(file, "number_base=%i\n", saved_number_base);
	fprintf(file, "angle_unit=%i\n", saved_angle_unit);
	fprintf(file, "hyp_is_on=%i\n", saved_hyp_is_on);
	fprintf(file, "functions_enabled=%i\n", saved_functions_enabled);
	fprintf(file, "variables_enabled=%i\n", saved_variables_enabled);
	fprintf(file, "donot_calculate_variables=%i\n", saved_donot_calcvars);	
	fprintf(file, "unknownvariables_enabled=%i\n", saved_unknownvariables_enabled);
	fprintf(file, "units_enabled=%i\n", saved_units_enabled);
	fprintf(file, "indicate_infinite_series=%i\n", saved_indicate_infinite_series);	
	fprintf(file, "always_exact=%i\n", saved_always_exact);	
	fprintf(file, "in_rpn_mode=%i\n", saved_in_rpn_mode);		
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
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sub")), MINUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_add")), PLUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_times")), MULTIPLICATION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_divide")), DIVISION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_sqrt")), "SQRT");	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (glade_xml, "button_dot")), CALCULATOR->getDecimalPoint());	
	}
	setResult(result_text.c_str());
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
void on_preferences_checkbutton_custom_font_toggled(GtkToggleButton *w, gpointer user_data) {
	use_custom_font = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget(glade_xml, "preferences_button_font"), use_custom_font);
	if(use_custom_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
		viewresult(NULL);		
	} else {
		PangoFontDescription *font = pango_font_description_from_string("");
//		pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);		
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
		viewresult(NULL);			
	}
}
void on_preferences_button_font_clicked(GtkButton *w, gpointer user_data) {
	GtkWidget *d = gtk_font_selection_dialog_new(_("Select result font"));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		custom_font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(d));
		gtk_button_set_label(w, custom_font.c_str());
		PangoFontDescription *font = pango_font_description_from_string(custom_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
		viewresult(NULL);
	}
	gtk_widget_destroy(d);
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
	if(gtk_toggle_button_get_active(w)) {
		fractional_mode = FRACTIONAL_MODE_FRACTION;
		GtkWidget *w_fraction = glade_xml_get_widget (glade_xml, "menu_item_fraction_fraction");
		g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_combined_activate, NULL);		
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w_fraction), TRUE);		
		g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_combined_activate, NULL);		
	} else {
		fractional_mode = FRACTIONAL_MODE_DECIMAL;
		GtkWidget *w_fraction = glade_xml_get_widget (glade_xml, "menu_item_fraction_decimal");
		g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_decimal_activate, NULL);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w_fraction), TRUE);
		g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_decimal_activate, NULL);
	}
	setResult(result_text.c_str());	
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
	expression_has_changed = true;
	if(result_text.empty()) return;
	clearresult();
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
	insert_text(CALCULATOR->getDecimalPoint());
}
void on_button_brace_open_clicked(GtkButton *w, gpointer user_data) {
	insert_text(LEFT_BRACKET_STR.c_str());
}
void on_button_brace_close_clicked(GtkButton *w, gpointer user_data) {
	insert_text(RIGHT_BRACKET_STR.c_str());
}
void on_button_times_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	if(use_unicode_signs) insert_text(SIGN_MULTIDOT);
	else insert_text(MULTIPLICATION_STR.c_str());
}
void on_button_add_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
//	if(use_unicode_signs) insert_text(SIGN_PLUS);
//	else 
	insert_text(PLUS_STR.c_str());
}
void on_button_sub_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	if(use_unicode_signs) insert_text(SIGN_MINUS);
	else insert_text(MINUS_STR.c_str());
}
void on_button_divide_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
//	if(use_unicode_signs) insert_text(SIGN_DIVISION);
//	else 
	insert_text(DIVISION_STR.c_str());
}
void on_button_ans_clicked(GtkButton *w, gpointer user_data) {
	insert_text("Ans");
}
void on_button_exp_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text(EXP_STR.c_str());
}
void on_button_xy_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text(POWER_STR.c_str());
}
void on_button_square_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text(POWER_STR.c_str());
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
	insert_text(PLUS_STR.c_str());
}
void on_menu_item_subtraction_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(MINUS_STR.c_str());
}
void on_menu_item_multiplication_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(MULTIPLICATION_STR.c_str());
}
void on_menu_item_division_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(DIVISION_STR.c_str());
}
void on_menu_item_power_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(POWER_STR.c_str());
}
void on_menu_item_exponent_activate(GtkMenuItem *w, gpointer user_data) {
	insert_text(EXP_STR.c_str());
}
void on_menu_item_rpn_mode_activate(GtkMenuItem *w, gpointer user_data) {
	CALCULATOR->setRPNMode(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
	gtk_widget_grab_focus(expression);
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
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_octal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = BASE_OCTAL;
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_decimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = BASE_DECI;
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_hexadecimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	number_base = BASE_HEX;
	setResult(result_text.c_str());
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
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = MODE_SCIENTIFIC;
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_purely_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = MODE_SCIENTIFIC_PURE;
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_non_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	display_mode = MODE_DECIMALS;
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	use_prefixes = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_indicate_infinite_series_activate(GtkMenuItem *w, gpointer user_data) {
	indicate_infinite_series = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_exact_mode_activate(GtkMenuItem *w, gpointer user_data) {
	CALCULATOR->setAlwaysExact(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
	execute_expression();
}
void on_menu_item_fraction_decimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	fractional_mode = FRACTIONAL_MODE_DECIMAL;

	GtkWidget *w_fraction = glade_xml_get_widget (glade_xml, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_fraction_combined_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	fractional_mode = FRACTIONAL_MODE_COMBINED;

	GtkWidget *w_fraction = glade_xml_get_widget (glade_xml, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}
void on_menu_item_fraction_fraction_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	fractional_mode = FRACTIONAL_MODE_FRACTION;

	GtkWidget *w_fraction = glade_xml_get_widget (glade_xml, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), TRUE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	setResult(result_text.c_str());
	gtk_widget_grab_focus(expression);
}

void on_menu_item_save_activate(GtkMenuItem *w, gpointer user_data) {
	add_as_variable();
}
void on_menu_item_precision_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "precision_dialog");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (glade_xml, "precision_dialog_spinbutton_precision"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_precision_dialog_spinbutton_precision_value_changed, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "precision_dialog_spinbutton_precision")), PRECISION);	
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (glade_xml, "precision_dialog_spinbutton_precision"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_precision_dialog_spinbutton_precision_value_changed, NULL);
	gtk_widget_show(dialog);
}
void on_menu_item_decimals_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = glade_xml_get_widget (glade_xml, "decimals_dialog");
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (glade_xml, "main_window")));
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_checkbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_max_toggled, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_checkbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_min_toggled, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_max_value_changed, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_min_value_changed, NULL);	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "decimals_dialog_checkbutton_min")), use_min_deci);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (glade_xml, "decimals_dialog_checkbutton_max")), use_max_deci);	
	gtk_widget_set_sensitive (glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_min"), use_min_deci);
	gtk_widget_set_sensitive (glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_max"), use_max_deci);	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_min")), min_deci);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_max")), max_deci);	
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_checkbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_max_toggled, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_checkbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_min_toggled, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_max_value_changed, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_min_value_changed, NULL);	
	gtk_widget_show(dialog);
}

/*
	check if entered unit name is valid, if not modify
*/
void on_unit_edit_entry_name_changed(GtkEditable *editable, gpointer user_data) {
	if(!CALCULATOR->unitNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unit_edit_entry_name_changed, NULL);		
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidUnitName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unit_edit_entry_name_changed, NULL);		
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
		"unit_edit_entry_name",
		"unit_edit_label_singular",
		"unit_edit_entry_singular",
		NULL
	};

	gchar *composite2[3] = {
		"unit_edit_label_internal",
		"unit_edit_entry_internal",
		NULL
	};

	gchar *alias[8] = {
		"unit_edit_label_exp",
		"unit_edit_spinbutton_exp",
		"unit_edit_label_relation",
		"unit_edit_entry_relation",
		"unit_edit_checkbutton_exact",		
		"unit_edit_label_reversed",
		"unit_edit_entry_reversed",
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
				(gtk_option_menu_get_history(om) == UNIT_CLASS_COMPOSITE_UNIT) ? FALSE : TRUE
				);
	}

	/* making the composite widgets (un)sensitive */
	for (pointer = composite2; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (glade_xml, *pointer),
				(gtk_option_menu_get_history(om) == UNIT_CLASS_COMPOSITE_UNIT) ? TRUE : FALSE
				);
	}

	/* making the alias widgets (un)sensitive */
	for (pointer = alias; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (glade_xml, *pointer),
				(gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT) ? TRUE : FALSE
				);
	}

	/* making the non-base widgets (un)sensitive */
	for (pointer = base; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (glade_xml, *pointer),
				(gtk_option_menu_get_history(om) == UNIT_CLASS_BASE_UNIT) ? FALSE : TRUE
				);
	}

	
	
}
/*
	"New" button clicked in unit manager -- open new unit dialog
*/
void on_units_button_new_clicked(GtkButton *button, gpointer user_data) {
	if(selected_unit_category == _("All") || selected_unit_category == _("Uncategorized") || selected_unit_category == _("Inactive")) {
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
		if(use_short_units) {
			gstr = g_strdup(u->shortName().c_str());
		} else {
			gstr = g_strdup(u->plural(true).c_str());
		}
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
		setResult(result_text.c_str());
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
	if(u && u->isLocal()) {
		if(u->isUsedByOtherUnits()) {
			//do not delete units that are used by other units
			show_message(_("Cannot delete unit as it is needed by other units."), units_window);
			return;
		}
		//ensure that all references to the unit is removed in Calculator
		u->destroy();
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
	if(selected_variable_category == _("All") || selected_variable_category == _("Uncategorized") || selected_variable_category == _("Inactive")) {
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
	if(v && v->isLocal()) {
		//ensure that all references are removed in Calculator
		v->destroy();
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
	if(selected_function_category == _("All") || selected_function_category == _("Uncategorized") || selected_function_category == _("Inactive")) {
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
	if(f && f->isLocal()) {
		//ensure removal of all references in Calculator
		f->destroy();
		//update menus and trees
		if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &model, &iter)) {
			//reselected selected function category
			GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
			string str = selected_function_category;
			update_fmenu();
			if(str == selected_function_category) {
				gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), path);
			}
			g_free(path);
		} else {
			update_fmenu();
		}
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
		g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_function_edit_entry_name_changed, NULL);		
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidFunctionName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_function_edit_entry_name_changed, NULL);		
	}
}
/*
	check if entered variable name is valid, if not modify
*/
void on_variable_edit_entry_name_changed(GtkEditable *editable, gpointer user_data) {
	if(!CALCULATOR->variableNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_variable_edit_entry_name_changed, NULL);		
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidVariableName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_variable_edit_entry_name_changed, NULL);		
	}
}

void on_matrix_edit_spinbutton_columns_value_changed(GtkSpinButton *w, gpointer user_data) {
	GtkTable *table = GTK_TABLE(glade_xml_get_widget(glade_xml, "matrix_edit_table_elements")); 
	gint c = element_entries[0].size();
	gint r = element_entries.size();
	gint new_c = gtk_spin_button_get_value_as_int(w);	
	gtk_table_resize(table, r, new_c);
	if(new_c < c) {
		for(gint index_r = 0; index_r < r; index_r++) {
			for(gint index_c = new_c; index_c < c; index_c++) {
				gtk_widget_destroy(element_entries[index_r][index_c]);
			}
			element_entries[index_r].resize(new_c);		
		}	
	} else {
		for(gint index_c = c; index_c < new_c; index_c++) {
			for(gint index_r = 0; index_r < r; index_r++) {
				GtkWidget *entry = gtk_entry_new();
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_matrix")))) {
					gtk_entry_set_text(GTK_ENTRY(entry), "0");
				}
				gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
				gtk_table_attach(table, entry, index_c, index_c + 1, index_r, index_r + 1, (GtkAttachOptions) 0, GTK_FILL, 0, 0);
				gtk_widget_show(entry);			
				element_entries[index_r].push_back(entry);
			}
		}
	}
}
void on_matrix_edit_spinbutton_rows_value_changed(GtkSpinButton *w, gpointer user_data) {
	GtkTable *table = GTK_TABLE(glade_xml_get_widget(glade_xml, "matrix_edit_table_elements")); 
	gint c = element_entries[0].size();
	gint r = element_entries.size();
	gint new_r = gtk_spin_button_get_value_as_int(w);	
	gtk_table_resize(table, new_r, c);
	for(gint index_r = new_r; index_r < r; index_r++) {
		for(gint index_c = 0; index_c < c; index_c++) {
			gtk_widget_destroy(element_entries[index_r][index_c]);
		}
	}
	element_entries.resize(new_r);		
	for(gint index_r = r; index_r < new_r; index_r++) {
		for(gint index_c = 0; index_c < c; index_c++) {
			GtkWidget *entry = gtk_entry_new();
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (glade_xml, "matrix_edit_radiobutton_matrix")))) {
				gtk_entry_set_text(GTK_ENTRY(entry), "0");
			}		
			gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
			gtk_table_attach(table, entry, index_c, index_c + 1, index_r, index_r + 1, (GtkAttachOptions) 0, GTK_FILL, 0, 0);
			gtk_widget_show(entry);
			element_entries[index_r].push_back(entry);
		}
	}
}

void update_nbases_entries(Manager *value, NumberFormat nrformat) {
	GtkWidget *w_dec, *w_bin, *w_oct, *w_hex;
	w_dec = glade_xml_get_widget (glade_xml, "nbases_entry_decimal");
	w_bin = glade_xml_get_widget (glade_xml, "nbases_entry_binary");	
	w_oct = glade_xml_get_widget (glade_xml, "nbases_entry_octal");	
	w_hex = glade_xml_get_widget (glade_xml, "nbases_entry_hexadecimal");	
	g_signal_handlers_block_matched((gpointer) w_dec, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_decimal_changed, NULL);			
	g_signal_handlers_block_matched((gpointer) w_bin, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_binary_changed, NULL);
	g_signal_handlers_block_matched((gpointer) w_oct, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_octal_changed, NULL);
	g_signal_handlers_block_matched((gpointer) w_hex, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_hexadecimal_changed, NULL);
	if(nrformat != NUMBER_FORMAT_NORMAL) gtk_entry_set_text(GTK_ENTRY(w_dec), value->print(NUMBER_FORMAT_NORMAL, DISPLAY_FORMAT_DECIMAL_ONLY).c_str());
	if(nrformat != NUMBER_FORMAT_BIN) gtk_entry_set_text(GTK_ENTRY(w_bin), value->print(NUMBER_FORMAT_BIN).c_str());	
	if(nrformat != NUMBER_FORMAT_OCTAL) gtk_entry_set_text(GTK_ENTRY(w_oct), value->print(NUMBER_FORMAT_OCTAL).c_str());	
	if(nrformat != NUMBER_FORMAT_HEX) gtk_entry_set_text(GTK_ENTRY(w_hex), value->print(NUMBER_FORMAT_HEX).c_str());	
	g_signal_handlers_unblock_matched((gpointer) w_dec, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_decimal_changed, NULL);			
	g_signal_handlers_unblock_matched((gpointer) w_bin, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_binary_changed, NULL);
	g_signal_handlers_unblock_matched((gpointer) w_oct, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_octal_changed, NULL);
	g_signal_handlers_unblock_matched((gpointer) w_hex, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_hexadecimal_changed, NULL);
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
	execute_expression();
}


void on_decimals_dialog_spinbutton_max_value_changed(GtkSpinButton *w, gpointer user_data) {
	max_deci = gtk_spin_button_get_value_as_int(w);
	setResult(result_text.c_str());
}
void on_decimals_dialog_spinbutton_min_value_changed(GtkSpinButton *w, gpointer user_data) {
	min_deci = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
	setResult(result_text.c_str());
}
void on_decimals_dialog_checkbutton_max_toggled(GtkToggleButton *w, gpointer user_data) {
	use_max_deci = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_max"), use_max_deci);
	setResult(result_text.c_str());
}
void on_decimals_dialog_checkbutton_min_toggled(GtkToggleButton *w, gpointer user_data) {
	use_min_deci = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "decimals_dialog_spinbutton_min"), use_min_deci);
	setResult(result_text.c_str());
}

gboolean on_expression_key_press_event(GtkWidget *w, GdkEventKey *event, gpointer user_data) {
	switch(event->keyval) {
		case GDK_dead_circumflex: {
			gint end = 0;
			wrap_expression_selection();
			end = gtk_editable_get_position(GTK_EDITABLE(expression));
			gtk_editable_insert_text(GTK_EDITABLE(expression), POWER_STR.c_str(), -1, &end);				
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
//				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_PLUS, -1, &pos);
//				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
//				return TRUE;			
				break;
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

gboolean on_resultview_expose_event(GtkWidget *w, GdkEventExpose *event, gpointer user_data) {
	if(pixmap_result) {
		gint w = 0, h = 0;
		gdk_drawable_get_size(GDK_DRAWABLE(pixmap_result), &w, &h);	
		if(resultview->allocation.width > w) {
			gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_result), 0, 0, resultview->allocation.width - w, (resultview->allocation.height - h) / 2, -1, -1);
		} else {
			gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_result), 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1);
		}		
	}	
	return TRUE;
}
void on_matrix_edit_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "matrix_edit_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "matrix_edit_label_elements")), _("Components (in horizontal order)"));
	}
}
void on_matrix_edit_radiobutton_vector_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "matrix_edit_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (glade_xml, "matrix_edit_label_elements")), _("Components (in horizontal order)"));
	}
}
void on_csv_import_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer user_data) {
}
void on_csv_import_radiobutton_vectors_toggled(GtkToggleButton *w, gpointer user_data) {
}
void on_csv_import_optionmenu_delimiter_changed(GtkOptionMenu *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "csv_import_entry_delimiter_other"), gtk_option_menu_get_history(w) == DELIMITER_OTHER);
}
void on_csv_import_button_file_clicked(GtkButton *button, gpointer user_data) {
	GtkWidget *d = gtk_file_selection_new(_("Select file to import"));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "csv_import_entry_file")), gtk_file_selection_get_filename(GTK_FILE_SELECTION(d)));
	}
	gtk_widget_destroy(d);
}

void on_type_label_date_clicked(GtkButton *w, gpointer user_data) {
	GtkWidget *d = gtk_dialog_new_with_buttons(_("Select date"), GTK_WINDOW(gtk_widget_get_ancestor(GTK_WIDGET(w), GTK_TYPE_WINDOW)), (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	GtkWidget *date_w = gtk_calendar_new();
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(d)->vbox), date_w);
	gtk_widget_show_all(d);
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		guint year = 0, month = 0, day = 0;
		gtk_calendar_get_date(GTK_CALENDAR(date_w), &year, &month, &day);
		gchar *gstr = g_strdup_printf("%i-%02i-%02i", year, month + 1, day);
		gtk_entry_set_text(GTK_ENTRY(user_data), gstr);
		g_free(gstr);
	}
	gtk_widget_destroy(d);
}

void on_functions_button_deactivate_clicked(GtkButton *w, gpointer user_data) {
	Function *f = get_selected_function();
	if(f) {
		f->setActive(!f->isActive());
		update_fmenu();
	}
}
void on_variables_button_deactivate_clicked(GtkButton *w, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v) {
		v->setActive(!v->isActive());
		update_vmenu();
	}
}
void on_units_button_deactivate_clicked(GtkButton *w, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		u->setActive(!u->isActive());
		update_umenus();
	}
}

void on_function_edit_button_add_argument_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeIter iter;	
	gtk_list_store_append(tFunctionArguments_store, &iter);
	Argument *arg;
	if(edited_function && edited_function->isBuiltin()) {
		arg = new Argument();
	} else {
		int menu_index = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "function_edit_optionmenu_argument_type")));
		switch(menu_index) {
			case MENU_ARGUMENT_TYPE_TEXT: {arg = new TextArgument(); break;}
			case MENU_ARGUMENT_TYPE_DATE: {arg = new DateArgument(); break;}
			case MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {arg = new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
			case MENU_ARGUMENT_TYPE_POSITIVE_INTEGER: {arg = new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
			case MENU_ARGUMENT_TYPE_NONZERO_INTEGER: {arg = new IntegerArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
			case MENU_ARGUMENT_TYPE_INTEGER: {arg = new IntegerArgument(); break;}
			case MENU_ARGUMENT_TYPE_NONNEGATIVE: {arg = new FractionArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
			case MENU_ARGUMENT_TYPE_POSITIVE: {arg = new FractionArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
			case MENU_ARGUMENT_TYPE_NONZERO: {arg = new FractionArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
			case MENU_ARGUMENT_TYPE_FRACTION: {arg = new FractionArgument(); break;}
			case MENU_ARGUMENT_TYPE_VECTOR: {arg = new VectorArgument(); break;}
			case MENU_ARGUMENT_TYPE_MATRIX: {arg = new MatrixArgument(); break;}
			case MENU_ARGUMENT_TYPE_EXPRESSION_ITEM: {arg = new ExpressionItemArgument(); break;}
			case MENU_ARGUMENT_TYPE_FUNCTION: {arg = new FunctionArgument(); break;}
			case MENU_ARGUMENT_TYPE_UNIT: {arg = new UnitArgument(); break;}
			case MENU_ARGUMENT_TYPE_VARIABLE: {arg = new VariableArgument(); break;}
			case MENU_ARGUMENT_TYPE_BOOLEAN: {arg = new BooleanArgument(); break;}	
			default: {arg = new Argument();}
		}
	}
	arg->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_argument_name"))));		
	gtk_list_store_set(tFunctionArguments_store, &iter, 0, arg->name().c_str(), 1, arg->printlong().c_str(), 2, arg, -1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_argument_name")), "");
}
void on_function_edit_button_remove_argument_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionArguments));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		if(selected_argument) {
			delete selected_argument;
			selected_argument = NULL;
		}
		gtk_list_store_remove(tFunctionArguments_store, &iter);
	}
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_argument_name")), "");
}
void on_function_edit_button_modify_argument_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionArguments));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {	
		int argtype = ARGUMENT_TYPE_FREE;
		if(edited_function && edited_function->isBuiltin()) {
			if(!selected_argument) {
				selected_argument = new Argument();
			}
		} else {
			int menu_index = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (glade_xml, "function_edit_optionmenu_argument_type")));
			switch(menu_index) {
				case MENU_ARGUMENT_TYPE_TEXT: {argtype = ARGUMENT_TYPE_TEXT; break;}
				case MENU_ARGUMENT_TYPE_DATE: {argtype = ARGUMENT_TYPE_DATE; break;}
				case MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {}
				case MENU_ARGUMENT_TYPE_POSITIVE_INTEGER: {}
				case MENU_ARGUMENT_TYPE_NONZERO_INTEGER: {}
				case MENU_ARGUMENT_TYPE_INTEGER: {argtype = ARGUMENT_TYPE_INTEGER; break;}
				case MENU_ARGUMENT_TYPE_NONNEGATIVE: {}
				case MENU_ARGUMENT_TYPE_POSITIVE: {}
				case MENU_ARGUMENT_TYPE_NONZERO: {}
				case MENU_ARGUMENT_TYPE_FRACTION: {argtype = ARGUMENT_TYPE_FRACTION; break;}
				case MENU_ARGUMENT_TYPE_VECTOR: {argtype = ARGUMENT_TYPE_VECTOR; break;}
				case MENU_ARGUMENT_TYPE_MATRIX: {argtype = ARGUMENT_TYPE_MATRIX; break;}
				case MENU_ARGUMENT_TYPE_EXPRESSION_ITEM: {argtype = ARGUMENT_TYPE_EXPRESSION_ITEM; break;}
				case MENU_ARGUMENT_TYPE_FUNCTION: {argtype = ARGUMENT_TYPE_FUNCTION; break;}
				case MENU_ARGUMENT_TYPE_UNIT: {argtype = ARGUMENT_TYPE_UNIT; break;}
				case MENU_ARGUMENT_TYPE_VARIABLE: {argtype = ARGUMENT_TYPE_VARIABLE; break;}
				case MENU_ARGUMENT_TYPE_BOOLEAN: {argtype = ARGUMENT_TYPE_BOOLEAN; break;}	
			}			
			
			if(!selected_argument || argtype != selected_argument->type() || menu_index == MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER || menu_index == MENU_ARGUMENT_TYPE_POSITIVE_INTEGER || menu_index == MENU_ARGUMENT_TYPE_NONZERO_INTEGER || menu_index == MENU_ARGUMENT_TYPE_NONZERO || menu_index == MENU_ARGUMENT_TYPE_POSITIVE || menu_index == MENU_ARGUMENT_TYPE_NONNEGATIVE) {
				if(selected_argument) {
					delete selected_argument;
				}
				switch(menu_index) {
					case MENU_ARGUMENT_TYPE_TEXT: {selected_argument = new TextArgument(); break;}
					case MENU_ARGUMENT_TYPE_DATE: {selected_argument = new DateArgument(); break;}
					case MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {selected_argument = new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
					case MENU_ARGUMENT_TYPE_POSITIVE_INTEGER: {selected_argument = new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
					case MENU_ARGUMENT_TYPE_NONZERO_INTEGER: {selected_argument = new IntegerArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
					case MENU_ARGUMENT_TYPE_INTEGER: {selected_argument = new IntegerArgument(); break;}
					case MENU_ARGUMENT_TYPE_NONNEGATIVE: {selected_argument = new FractionArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
					case MENU_ARGUMENT_TYPE_POSITIVE: {selected_argument = new FractionArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
					case MENU_ARGUMENT_TYPE_NONZERO: {selected_argument = new FractionArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
					case MENU_ARGUMENT_TYPE_FRACTION: {selected_argument = new FractionArgument(); break;}
					case MENU_ARGUMENT_TYPE_VECTOR: {selected_argument = new VectorArgument(); break;}
					case MENU_ARGUMENT_TYPE_MATRIX: {selected_argument = new MatrixArgument(); break;}
					case MENU_ARGUMENT_TYPE_EXPRESSION_ITEM: {selected_argument = new ExpressionItemArgument(); break;}
					case MENU_ARGUMENT_TYPE_FUNCTION: {selected_argument = new FunctionArgument(); break;}
					case MENU_ARGUMENT_TYPE_UNIT: {selected_argument = new UnitArgument(); break;}
					case MENU_ARGUMENT_TYPE_VARIABLE: {selected_argument = new VariableArgument(); break;}
					case MENU_ARGUMENT_TYPE_BOOLEAN: {selected_argument = new BooleanArgument(); break;}	
					default: {selected_argument = new Argument();}
				}			
			}
			
		}
		selected_argument->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (glade_xml, "function_edit_entry_argument_name"))));
		gtk_list_store_set(tFunctionArguments_store, &iter, 0, selected_argument->name().c_str(), 1, selected_argument->printlong().c_str(), 2, (gpointer) selected_argument, -1);
	}
}
void on_function_edit_entry_argument_name_activate(GtkEntry *entry, gpointer user_data) {
	on_function_edit_button_add_argument_clicked(GTK_BUTTON(glade_xml_get_widget (glade_xml, "function_edit_button_add_argument_")), NULL);
}
void on_function_edit_button_rules_clicked(GtkButton *w, gpointer user_data) {
	edit_argument(get_selected_argument());
}
void on_argument_rules_checkbutton_enable_min_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_min"), gtk_toggle_button_get_active(w));
}
void on_argument_rules_checkbutton_enable_max_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_spinbutton_max"), gtk_toggle_button_get_active(w));
}
void on_argument_rules_checkbutton_enable_condition_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (glade_xml, "argument_rules_entry_condition"), gtk_toggle_button_get_active(w));
}

}
