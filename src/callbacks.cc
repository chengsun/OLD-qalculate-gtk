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
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <glade/glade.h>
#ifdef HAVE_LIBGNOME
#include <libgnome/libgnome.h>
#endif

#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "main.h"

extern GladeXML *main_glade, *about_glade, *argumentrules_glade, *csvimport_glade, *csvexport_glade, *nbexpression_glade, *decimals_glade;
extern GladeXML *functionedit_glade, *functions_glade, *matrixedit_glade, *nbases_glade, *plot_glade, *precision_glade;
extern GladeXML *preferences_glade, *unit_glade, *unitedit_glade, *units_glade, *unknownedit_glade, *variableedit_glade, *variables_glade;

bool changing_in_nbases_dialog;

#if GTK_MINOR_VERSION >= 3
extern GtkWidget *expander;
extern GtkEntryCompletion *completion;
extern GtkListStore *completion_store;
#endif

extern GtkWidget *expression;
extern GtkWidget *f_menu, *v_menu, *u_menu, *u_menu2, *recent_menu;
extern KnownVariable *vans, *vAns;
extern GtkWidget *tPlotFunctions;
extern GtkListStore *tPlotFunctions_store;
extern GtkWidget *tFunctionArguments;
extern GtkListStore *tFunctionArguments_store;
extern GtkWidget *tSubfunctions;
extern GtkListStore *tSubfunctions_store;
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
extern string selected_function_category;
extern Function *selected_function;
Function *edited_function;
unsigned int selected_subfunction;
unsigned int last_subfunction_index;
Argument *selected_argument;
Argument *edited_argument;
extern string selected_variable_category;
extern Variable *selected_variable;
extern string selected_unit_category;
extern Unit *selected_unit;
extern Unit *selected_to_unit;
int saved_precision, saved_angle_unit;
bool save_mode_on_exit;
bool save_defs_on_exit;
bool use_custom_result_font, use_custom_expression_font;
string custom_result_font, custom_expression_font;
bool hyp_is_on, saved_hyp_is_on;
bool show_buttons;
extern bool load_global_defs, fetch_exchange_rates_at_startup, first_time;
extern GtkWidget *omToUnit_menu;
bool block_unit_convert;
extern MathStructure *mstruct;
extern string result_text;
extern GtkWidget *resultview;
extern GdkPixmap *pixmap_result;
extern GdkPixbuf *pixbuf_result;
vector<vector<GtkWidget*> > element_entries;
bool b_busy;
GdkPixmap *tmp_pixmap;
bool expression_has_changed, current_object_has_changed;
int history_width, history_height;
AssumptionNumberType saved_assumption_type;
AssumptionSign saved_assumption_sign;

vector<string> initial_history;
vector<string> expression_history;
int expression_history_index = -1;
bool dont_change_index = false;

PlotLegendPlacement default_plot_legend_placement = PLOT_LEGEND_TOP_RIGHT;
bool default_plot_display_grid = true;
bool default_plot_full_border = false;
string default_plot_min = "0";
string default_plot_max = "10";
int default_plot_sampling_rate = 100;
bool default_plot_rows = false;
int default_plot_type = 0;
PlotStyle default_plot_style = PLOT_STYLE_LINES;
PlotSmoothing default_plot_smoothing = PLOT_SMOOTHING_NONE;
string default_plot_variable = "x";
bool default_plot_color = true;

string multi_sign;

gint current_object_start = -1, current_object_end = -1;

PrintOptions printops, saved_printops;
EvaluationOptions evalops, saved_evalops;

extern FILE *view_pipe_r, *view_pipe_w;
extern pthread_t view_thread;
extern pthread_attr_t view_thread_attr;


#define TEXT_TAGS			"<span size=\"xx-large\">"
#define TEXT_TAGS_END			"</span>"
#define TEXT_TAGS_SMALL			"<span size=\"large\">"
#define TEXT_TAGS_SMALL_END		"</span>"
#define TEXT_TAGS_XSMALL		"<span size=\"medium\">"
#define TEXT_TAGS_XSMALL_END		"</span>"

#define MARKUP_STRING(str, text)	if(ips.power_depth > 0) {str = TEXT_TAGS_SMALL;} else {str = TEXT_TAGS;} str += text; if(ips.power_depth > 0) {str += TEXT_TAGS_SMALL_END;} else {str += TEXT_TAGS_END;}			
#define CALCULATE_SPACE_W		gint space_w, space_h; PangoLayout *layout_space = gtk_widget_create_pango_layout(resultview, NULL); if(ips.power_depth > 0) {pango_layout_set_markup(layout_space, TEXT_TAGS_SMALL " " TEXT_TAGS_SMALL_END, -1);} else {pango_layout_set_markup(layout_space, TEXT_TAGS " " TEXT_TAGS_END, -1);} pango_layout_get_pixel_size(layout_space, &space_w, &space_h); g_object_unref(layout_space);


struct tree_struct {
	string item;
	list<tree_struct> items;
	list<tree_struct>::iterator it;
	list<tree_struct>::reverse_iterator rit;
	vector<void*> objects;	
	tree_struct *parent;
	void sort() {
		items.sort();
		for(list<tree_struct>::iterator it = items.begin(); it != items.end(); ++it) {
			it->sort();
		}
	}
	bool operator < (tree_struct &s1) const {
		return item < s1.item;	
	}	
};

tree_struct function_cats, unit_cats, variable_cats;
vector<void*> ia_units, ia_variables, ia_functions;	
vector<string> recent_functions_pre;
vector<string> recent_variables_pre;
vector<string> recent_units_pre;
vector<GtkWidget*> recent_function_items;
vector<GtkWidget*> recent_variable_items;
vector<GtkWidget*> recent_unit_items;
vector<Function*> recent_functions;
vector<Variable*> recent_variables;
vector<Unit*> recent_units;

bool completion_blocked = false;

void block_completion() {
#if GTK_MINOR_VERSION >= 3
	gtk_entry_completion_set_minimum_key_length(completion, 1000);
	completion_blocked = true;
#endif
}
void unblock_completion() {
#if GTK_MINOR_VERSION >= 3
#endif
}

void wrap_expression_selection() {
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {			
		gtk_editable_select_region(GTK_EDITABLE(expression), end, end);
		gtk_editable_insert_text(GTK_EDITABLE(expression), "(", 1, &start);
		end++;
		gtk_editable_insert_text(GTK_EDITABLE(expression), ")", 1, &end);				
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
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history")));
	GtkTextIter iter, iter_s;
	while(true) {
		if(CALCULATOR->error()->critical()) {
			critical = true;
		}
		if(b) {
			str += "\n";
		} else {
			b = true;
		}
		str += CALCULATOR->error()->message();
		gtk_text_buffer_get_start_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, CALCULATOR->error()->message().c_str(), -1);
		gtk_text_buffer_get_start_iter(tb, &iter_s);
		if(CALCULATOR->error()->critical()) {
			gtk_text_buffer_apply_tag_by_name(tb, "red_foreground", &iter_s, &iter);
		} else {
			gtk_text_buffer_apply_tag_by_name(tb, "blue_foreground", &iter_s, &iter);
		}
		gtk_text_buffer_insert(tb, &iter, "\n", -1);
		gtk_text_buffer_place_cursor(tb, &iter);
		if(!CALCULATOR->nextError()) break;
	}
	if(!str.empty()) {
		if(critical) {
			edialog = gtk_message_dialog_new(
					GTK_WINDOW(
						glade_xml_get_widget (main_glade, "main_window")
					),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					str.c_str());
		} else {
			edialog = gtk_message_dialog_new(
					GTK_WINDOW(
						glade_xml_get_widget (main_glade, "main_window")
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
unsigned int get_selected_subfunction() {
	return selected_subfunction;
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
	int cat_i, cat_i_prev; 
	bool b;	
	string str, cat, cat_sub;
	Unit *u = NULL;
	unit_cats.items.clear();
	unit_cats.objects.clear();
	unit_cats.parent = NULL;	
	ia_units.clear();
	list<tree_struct>::iterator it;	
	for(unsigned int i = 0; i < CALCULATOR->units.size(); i++) {
		if(!CALCULATOR->units[i]->isActive()) {
			b = false;
			for(unsigned int i3 = 0; i3 < ia_units.size(); i3++) {
				u = (Unit*) ia_units[i3];
				if(CALCULATOR->units[i]->title() < u->title()) {
					b = true;
					ia_units.insert(ia_units.begin() + i3, (void*) CALCULATOR->units[i]);
					break;
				}
			}
			if(!b) ia_units.push_back((void*) CALCULATOR->units[i]);						
		} else {
			tree_struct *item = &unit_cats;
			if(!CALCULATOR->units[i]->category().empty()) {
				cat = CALCULATOR->units[i]->category();
				cat_i = cat.find("/"); cat_i_prev = -1;
				b = false;
				while(true) {
					if(cat_i == (int) string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(it = item->items.begin(); it != item->items.end(); ++it) {
						if(cat_sub == it->item) {
							item = &*it;
							b = true;
							break;
						}
					}
					if(!b) {
						tree_struct cat;		
						item->items.push_back(cat);
						it = item->items.end();
						--it;
						it->parent = item;
						item = &*it;
						item->item = cat_sub;
					}
					if(cat_i == (int) string::npos) {
						break;
					}
					cat_i_prev = cat_i;
					cat_i = cat.find("/", cat_i_prev + 1);
				}
			}
			b = false;
			for(unsigned int i3 = 0; i3 < item->objects.size(); i3++) {
				u = (Unit*) item->objects[i3];
				if(CALCULATOR->units[i]->title() < u->title()) {
					b = true;
					item->objects.insert(item->objects.begin() + i3, (void*) CALCULATOR->units[i]);
					break;
				}
			}
			if(!b) item->objects.push_back((void*) CALCULATOR->units[i]);		
		}
	}
	
	unit_cats.sort();

}
void generate_variables_tree_struct() {

	int cat_i, cat_i_prev; 
	bool b;	
	string str, cat, cat_sub;
	Variable *v = NULL;
	variable_cats.items.clear();
	variable_cats.objects.clear();
	variable_cats.parent = NULL;
	ia_variables.clear();
	list<tree_struct>::iterator it;	
	for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->isActive()) {
			//deactivated variable
			b = false;
			for(unsigned int i3 = 0; i3 < ia_variables.size(); i3++) {
				v = (Variable*) ia_variables[i3];
				if(CALCULATOR->variables[i]->title() < v->title()) {
					b = true;
					ia_variables.insert(ia_variables.begin() + i3, (void*) CALCULATOR->variables[i]);
					break;
				}
			}
			if(!b) ia_variables.push_back((void*) CALCULATOR->variables[i]);											
		} else {
			tree_struct *item = &variable_cats;
			if(!CALCULATOR->variables[i]->category().empty()) {
				cat = CALCULATOR->variables[i]->category();
				cat_i = cat.find("/"); cat_i_prev = -1;
				b = false;
				while(true) {
					if(cat_i == (int) string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(it = item->items.begin(); it != item->items.end(); ++it) {
						if(cat_sub == it->item) {
							item = &*it;
							b = true;
							break;
						}
					}
					if(!b) {
						tree_struct cat;		
						item->items.push_back(cat);
						it = item->items.end();
						--it;
						it->parent = item;
						item = &*it;
						item->item = cat_sub;
					}
					if(cat_i == (int) string::npos) {
						break;
					}
					cat_i_prev = cat_i;
					cat_i = cat.find("/", cat_i_prev + 1);
				}
			}
			b = false;
			for(unsigned int i3 = 0; i3 < item->objects.size(); i3++) {
				v = (Variable*) item->objects[i3];
				if(CALCULATOR->variables[i]->title() < v->title()) {
					b = true;
					item->objects.insert(item->objects.begin() + i3, (void*) CALCULATOR->variables[i]);
					break;
				}
			}
			if(!b) item->objects.push_back((void*) CALCULATOR->variables[i]);		
		}
	}
	
	variable_cats.sort();

}
void generate_functions_tree_struct() {

	int cat_i, cat_i_prev; 
	bool b;	
	string str, cat, cat_sub;
	Function *f = NULL;
	function_cats.items.clear();
	function_cats.objects.clear();
	function_cats.parent = NULL;
	ia_functions.clear();
	list<tree_struct>::iterator it;

	for(unsigned int i = 0; i < CALCULATOR->functions.size(); i++) {
		if(!CALCULATOR->functions[i]->isActive()) {
			//deactivated function
			b = false;
			for(unsigned int i3 = 0; i3 < ia_functions.size(); i3++) {
				f = (Function*) ia_functions[i3];
				if(CALCULATOR->functions[i]->title() < f->title()) {
					b = true;
					ia_functions.insert(ia_functions.begin() + i3, (void*) CALCULATOR->functions[i]);
					break;
				}
			}
			if(!b) ia_functions.push_back((void*) CALCULATOR->functions[i]);								
		} else {
			tree_struct *item = &function_cats;
			if(!CALCULATOR->functions[i]->category().empty()) {
				cat = CALCULATOR->functions[i]->category();
				cat_i = cat.find("/"); cat_i_prev = -1;
				b = false;
				while(true) {
					if(cat_i == (int) string::npos) {
						cat_sub = cat.substr(cat_i_prev + 1, cat.length() - 1 - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev + 1, cat_i - 1 - cat_i_prev);
					}
					b = false;
					for(it = item->items.begin(); it != item->items.end(); ++it) {
						if(cat_sub == it->item) {
							item = &*it;
							b = true;
							break;
						}
					}
					if(!b) {
						tree_struct cat;		
						item->items.push_back(cat);
						it = item->items.end();
						--it;
						it->parent = item;
						item = &*it;
						item->item = cat_sub;
					}
					if(cat_i == (int) string::npos) {
						break;
					}
					cat_i_prev = cat_i;
					cat_i = cat.find("/", cat_i_prev + 1);
				}
			}
			b = false;
			for(unsigned int i3 = 0; i3 < item->objects.size(); i3++) {
				f = (Function*) item->objects[i3];
				if(CALCULATOR->functions[i]->title() < f->title()) {
					b = true;
					item->objects.insert(item->objects.begin() + i3, (void*) CALCULATOR->functions[i]);
					break;
				}
			}
			if(!b) item->objects.push_back((void*) CALCULATOR->functions[i]);
		}
	}
	
	function_cats.sort();
	
}

/*
	generate the function categories tree in manage functions dialog
*/
void update_functions_tree() {
	if(!functions_glade) return;
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tFunctionCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tFunctionCategories_selection_changed, NULL);
	gtk_tree_store_clear(tFunctionCategories_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tFunctionCategories_selection_changed, NULL);
	gtk_tree_store_append(tFunctionCategories_store, &iter3, NULL);
	gtk_tree_store_set(tFunctionCategories_store, &iter3, 0, _("All"), 1, _("All"), -1);
	string str;
	tree_struct *item, *item2;
	function_cats.it = function_cats.items.begin();
	if(function_cats.it != function_cats.items.end()) {
		item = &*function_cats.it;
		++function_cats.it;
		item->it = item->items.begin();
	} else {
		item = NULL;
	}
	str = "";
	iter2 = iter3;
	while(item) {
		gtk_tree_store_append(tFunctionCategories_store, &iter, &iter2);
		str += "/";
		str += item->item;
		gtk_tree_store_set(tFunctionCategories_store, &iter, 0, item->item.c_str(), 1, str.c_str(), -1);
		if(str == selected_function_category) {
			EXPAND_TO_ITER(model, tFunctionCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionCategories)), &iter);
		}
		while(item && item->it == item->items.end()) {
			int str_i = str.rfind("/");
			if(str_i == (int) string::npos) {
				str = "";
			} else {
				str = str.substr(0, str_i);
			}
			item = item->parent;
			gtk_tree_model_iter_parent(model, &iter2, &iter);
			iter = iter2;
		}
		if(item) {
			item2 = &*item->it;
			if(item->it == item->items.begin()) iter2 = iter;
			++item->it;
			item = item2;
			item->it = item->items.begin();	
		}
	}
	if(!function_cats.objects.empty()) {
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
	gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_insert"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_delete"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_deactivate"), FALSE);
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
			string str = selected_function_category.substr(1, selected_function_category.length() - 1);
			for(unsigned int i = 0; i < CALCULATOR->functions.size(); i++) {
				if(CALCULATOR->functions[i]->isActive() && CALCULATOR->functions[i]->category().substr(0, selected_function_category.length() - 1) == str) {
					setFunctionTreeItem(iter2, CALCULATOR->functions[i]);
				}
			}
		} else {			
			for(unsigned int i = 0; i < CALCULATOR->functions.size(); i++) {
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
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Function *f;
		gtk_tree_model_get(model, &iter, 1, &f, -1);
		//remember the new selection
		selected_function = f;
		for(unsigned int i = 0; i < CALCULATOR->functions.size(); i++) {
			if(CALCULATOR->functions[i] == selected_function) {
				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (functions_glade, "functions_textview_description")));
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
				str += "(";				
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
				str += ")";				
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
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_edit"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_deactivate"), TRUE);
				if(CALCULATOR->functions[i]->isActive()) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (functions_glade, "functions_buttonlabel_deactivate")), _("Deactivate"));
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (functions_glade, "functions_buttonlabel_deactivate")), _("Activate"));
				}
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_insert"), CALCULATOR->functions[i]->isActive());
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_delete"), CALCULATOR->functions[i]->isLocal());
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_insert"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_delete"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_deactivate"), FALSE);
		selected_function = NULL;
	}
}

/*
	generate the variable categories tree in manage variables dialog
*/
void update_variables_tree() {
	if(!variables_glade) return;
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tVariableCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tVariableCategories_selection_changed, NULL);
	gtk_tree_store_clear(tVariableCategories_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tVariableCategories_selection_changed, NULL);
	gtk_tree_store_append(tVariableCategories_store, &iter3, NULL);
	gtk_tree_store_set(tVariableCategories_store, &iter3, 0, _("All"), 1, _("All"), -1);
	string str;
	tree_struct *item, *item2;
	variable_cats.it = variable_cats.items.begin();
	if(variable_cats.it != variable_cats.items.end()) {
		item = &*variable_cats.it;
		++variable_cats.it;
		item->it = item->items.begin();
	} else {
		item = NULL;
	}
	str = "";
	iter2 = iter3;
	while(item) {
		gtk_tree_store_append(tVariableCategories_store, &iter, &iter2);
		str += "/";
		str += item->item;
		gtk_tree_store_set(tVariableCategories_store, &iter, 0, item->item.c_str(), 1, str.c_str(), -1);
		if(str == selected_variable_category) {
			EXPAND_TO_ITER(model, tVariableCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tVariableCategories)), &iter);
		}

		while(item && item->it == item->items.end()) {

			int str_i = str.rfind("/");
			if(str_i == (int) string::npos) {
				str = "";
			} else {
				str = str.substr(0, str_i);
			}
			item = item->parent;
			gtk_tree_model_iter_parent(model, &iter2, &iter);
			iter = iter2;
		}
		if(item) {
			item2 = &*item->it;
			if(item->it == item->items.begin()) iter2 = iter;
			++item->it;
			item = item2;
			item->it = item->items.begin();	
		}
	}

	if(!variable_cats.objects.empty()) {	
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
	if(v == vans || v == vAns) {
		value = _("the previous result");
	} else if(v->isKnown()) {
		if(((KnownVariable*) v)->isExpression()) {
			value = CALCULATOR->localizeExpression(((KnownVariable*) v)->expression());
			if(value.length() > 13) {
				value = value.substr(0, 10);
				value += "...";
			}
		} else {
			if(((KnownVariable*) v)->get().isMatrix()) {
				value = "matrix";
			} else if(((KnownVariable*) v)->get().isVector()) {
				value = "vector";
			} else {
				value = CALCULATOR->printMathStructureTimeOut(((KnownVariable*) v)->get(), 30000);
			}
		}
	} else {
		if(((UnknownVariable*) v)->assumptions()) {
			switch(((UnknownVariable*) v)->assumptions()->sign()) {
				case ASSUMPTION_SIGN_POSITIVE: {value = _("positive"); break;}
				case ASSUMPTION_SIGN_NONPOSITIVE: {value = _("non-positive"); break;}
				case ASSUMPTION_SIGN_NEGATIVE: {value = _("negative"); break;}
				case ASSUMPTION_SIGN_NONNEGATIVE: {value = _("non-negative"); break;}
				case ASSUMPTION_SIGN_NONZERO: {value = _("non-zero"); break;}
				default: {}
			}
			if(!value.empty() && !((UnknownVariable*) v)->assumptions()->numberType() == ASSUMPTION_NUMBER_NONE) value += " ";
			switch(((UnknownVariable*) v)->assumptions()->numberType()) {
				case ASSUMPTION_NUMBER_INTEGER: {value += _("integer"); break;}
				case ASSUMPTION_NUMBER_RATIONAL: {value += _("rational"); break;}
				case ASSUMPTION_NUMBER_REAL: {value += _("real"); break;}
				case ASSUMPTION_NUMBER_COMPLEX: {value += _("complex"); break;}
				case ASSUMPTION_NUMBER_NUMBER: {value += _("number"); break;}
				default: {}
			}
			if(value.empty()) value = _("unknown");
		} else {
			value = _("default assumptions");
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
	gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_insert"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_delete"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_deactivate"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_export"), FALSE);
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
			string str = selected_variable_category.substr(1, selected_variable_category.length() - 1);
			for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
				if(CALCULATOR->variables[i]->isActive() && CALCULATOR->variables[i]->category().substr(0, selected_variable_category.length() - 1) == str) {
					setVariableTreeItem(iter2, CALCULATOR->variables[i]);
				}
			}			
		} else {			
			for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
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
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Variable *v;
		gtk_tree_model_get(model, &iter, 2, &v, -1);
		//remember selection
		selected_variable = v;
		for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
			if(CALCULATOR->variables[i] == selected_variable) {
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_edit"), CALCULATOR->variables[i] != vans && CALCULATOR->variables[i] != vAns);
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_insert"), CALCULATOR->variables[i]->isActive() && CALCULATOR->variables[i] != vans && CALCULATOR->variables[i] != vAns);
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_deactivate"), CALCULATOR->variables[i] != vans && CALCULATOR->variables[i] != vAns);
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_export"), CALCULATOR->variables[i]->isKnown());
				if(CALCULATOR->variables[i]->isActive()) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (variables_glade, "variables_buttonlabel_deactivate")), _("Deactivate"));
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (variables_glade, "variables_buttonlabel_deactivate")), _("Activate"));
				}
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_delete"), CALCULATOR->variables[i]->isLocal() && CALCULATOR->variables[i] != vans && CALCULATOR->variables[i] != vAns && CALCULATOR->variables[i] != CALCULATOR->v_x && CALCULATOR->variables[i] != CALCULATOR->v_y && CALCULATOR->variables[i] != CALCULATOR->v_z);
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_insert"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_delete"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_deactivate"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_export"), FALSE);
		selected_variable = NULL;
	}

}


/*
	generate the unit categories tree in manage units dialog
*/
void update_units_tree() {
	if(!units_glade) return;
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tUnitCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitCategories_selection_changed, NULL);
	gtk_tree_store_clear(tUnitCategories_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitCategories_selection_changed, NULL);
	gtk_tree_store_append(tUnitCategories_store, &iter3, NULL);
	gtk_tree_store_set(tUnitCategories_store, &iter3, 0, _("All"), 1, _("All"), -1);
	string str;
	tree_struct *item, *item2;
	unit_cats.it = unit_cats.items.begin();
	if(unit_cats.it != unit_cats.items.end()) {
		item = &*unit_cats.it;
		++unit_cats.it;
		item->it = item->items.begin();
	} else {
		item = NULL;
	}
	str = "";
	iter2 = iter3;
	while(item) {
		gtk_tree_store_append(tUnitCategories_store, &iter, &iter2);
		str += "/";
		str += item->item;
		gtk_tree_store_set(tUnitCategories_store, &iter, 0, item->item.c_str(), 1, str.c_str(), -1);
		if(str == selected_unit_category) {
			EXPAND_TO_ITER(model, tUnitCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitCategories)), &iter);
		}
		while(item && item->it == item->items.end()) {
			int str_i = str.rfind("/");
			if(str_i == (int) string::npos) {
				str = "";
			} else {
				str = str.substr(0, str_i);
			}
			item = item->parent;
			gtk_tree_model_iter_parent(model, &iter2, &iter);
			iter = iter2;
		}
		if(item) {
			item2 = &*item->it;
			if(item->it == item->items.begin()) iter2 = iter;
			++item->it;
			item = item2;
			item->it = item->items.begin();	
		}
	}
	if(!unit_cats.objects.empty()) {	
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
			if(printops.abbreviate_units) {
				sbase = au->firstBaseUnit()->shortName();
			} else {
				sbase = au->firstBaseUnit()->singular();
			}
			if(au->firstBaseExp() != 1) {
				sbase += POWER;
				sbase += i2s(au->firstBaseExp());
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
	gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_edit"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_insert"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_delete"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_deactivate"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_convert_to"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_frame_convert"), FALSE);
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
			string str = selected_unit_category.substr(1, selected_unit_category.length() - 1);
			for(unsigned int i = 0; i < CALCULATOR->units.size(); i++) {	
				if(CALCULATOR->units[i]->isActive() && CALCULATOR->units[i]->category().substr(0, selected_unit_category.length() - 1) == str) {
					setUnitTreeItem(iter2, CALCULATOR->units[i]);
				}
			}
		} else {
			for(unsigned int i = 0; i < CALCULATOR->units.size(); i++) {
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
	gtk_option_menu_set_menu(GTK_OPTION_MENU(glade_xml_get_widget (units_glade, "units_optionmenu_to_unit")), omToUnit_menu);
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
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (units_glade, "units_optionmenu_to_unit")), h);
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
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Unit *u;
		gtk_tree_model_get(model, &iter, UNITS_POINTER_COLUMN, &u, -1);
		selected_unit = u;
		for(unsigned int i = 0; i < CALCULATOR->units.size(); i++) {
			if(CALCULATOR->units[i] == selected_unit) {
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_frame_convert"), TRUE);				
				if(printops.abbreviate_units) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (units_glade, "units_label_from_unit")), CALCULATOR->units[i]->shortName().c_str());
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (units_glade, "units_label_from_unit")), CALCULATOR->units[i]->plural(true).c_str());
				}
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_delete"), CALCULATOR->units[i]->isLocal());
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_convert_to"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_insert"), CALCULATOR->units[i]->isActive());
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_edit"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_deactivate"), TRUE);
				if(CALCULATOR->units[i]->isActive()) {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (units_glade, "units_buttonlabel_deactivate")), _("Deactivate"));
				} else {
					gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (units_glade, "units_buttonlabel_deactivate")), _("Activate"));
				}
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_insert"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_delete"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_deactivate"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_convert_to"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_frame_convert"), FALSE);
		selected_unit = NULL;
	}
	if(!block_unit_convert) convert_in_wUnits();
}

void on_tPlotFunctions_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	selected_argument = NULL;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr1, *gstr2;
		gint type, smoothing, style, axis, rows;
		gtk_tree_model_get(model, &iter, 0, &gstr1, 1, &gstr2, 2, &style, 3, &smoothing, 4, &type, 5, &axis, 6, &rows, -1);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")), gstr2);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_title")), gstr1);
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), style);
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing")), smoothing);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_vector")), type == 1);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_paired")), type == 2);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_yaxis1")), axis != 2);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_yaxis2")), axis == 2);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")), rows);
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_remove"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_modify"), TRUE);		
		g_free(gstr1);
		g_free(gstr2);
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_modify"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_remove"), FALSE);
	}
}

void on_tSubfunctions_selection_changed(GtkTreeSelection *treeselection, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	selected_subfunction = 0;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gboolean g_b = FALSE;
		guint index = 0;
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, 3, &index, 4, &g_b, -1);
		selected_subfunction = index;
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression")), gstr);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_precalculate")), g_b);
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_subfunction"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_remove_subfunction"), TRUE);
		g_free(gstr);
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_subfunction"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_remove_subfunction"), FALSE);
	}
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
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name")), selected_argument->name().c_str());
			switch(selected_argument->type()) {
				case ARGUMENT_TYPE_TEXT: {
					menu_index = MENU_ARGUMENT_TYPE_TEXT;
					break;
				}
				case ARGUMENT_TYPE_SYMBOLIC: {
					menu_index = MENU_ARGUMENT_TYPE_SYMBOLIC;
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
				case ARGUMENT_TYPE_NUMBER: {
					menu_index = MENU_ARGUMENT_TYPE_NUMBER;
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
				case ARGUMENT_TYPE_FILE: {
					menu_index = MENU_ARGUMENT_TYPE_FILE;
					break;
				}					
				case ARGUMENT_TYPE_BOOLEAN: {
					menu_index = MENU_ARGUMENT_TYPE_BOOLEAN;
					break;
				}
				case ARGUMENT_TYPE_ANGLE: {
					menu_index = MENU_ARGUMENT_TYPE_ANGLE;
					break;
				}
				case ARGUMENT_TYPE_GIAC: {
					menu_index = MENU_ARGUMENT_TYPE_GIAC;
					break;
				}						
			}			
		} else {
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name")), "");
		}
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (functionedit_glade, "function_edit_optionmenu_argument_type")), menu_index);
		if(!(get_edited_function() && get_edited_function()->isBuiltin())) {
			gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_rules"), TRUE);	
			gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_remove_argument"), TRUE);
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_argument"), TRUE);		
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_argument"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_remove_argument"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_rules"), FALSE);	
	}
}
void update_function_arguments_list(Function *f) {
	if(!functionedit_glade) return;
	selected_argument = NULL;
	gtk_list_store_clear(tFunctionArguments_store);
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_argument"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_remove_argument"), FALSE);	
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_rules"), FALSE);	
	if(f) {
		GtkTreeIter iter;
		Argument *arg;
		int args = f->maxargs();
		if(args < 0) {
			args = f->minargs() + 1;	
		}
		Argument defarg;
		string str, str2;
		for(int i = 1; i <= args; i++) {
			gtk_list_store_append(tFunctionArguments_store, &iter);
			arg = f->getArgumentDefinition(i);
			if(arg) {
				arg = arg->copy();
				str = arg->printlong();
				str2 = arg->name();
			} else {
				str = defarg.printlong();
				str2 = "";
			}			
			gtk_list_store_set(tFunctionArguments_store, &iter, 0, str2.c_str(), 1, str.c_str(), 2, (gpointer) arg, -1);
		}
	}
}


/*
	generate unit submenu in expression menu
*/
void create_umenu() {
	GtkWidget *item;
	GtkWidget *sub, *sub2, *sub3;
	item = glade_xml_get_widget (main_glade, "units_menu");
	sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);	
	
	u_menu = sub;
	sub2 = sub;
	Unit *u;
	tree_struct *titem, *titem2;
	unit_cats.rit = unit_cats.items.rbegin();
	if(unit_cats.rit != unit_cats.items.rend()) {
		titem = &*unit_cats.rit;
		++unit_cats.rit;
		titem->rit = titem->items.rbegin();
	} else {
		titem = NULL;
	}
	stack<GtkWidget*> menus;
	menus.push(sub);
	sub3 = sub;
	while(titem) {
		SUBMENU_ITEM_PREPEND(titem->item.c_str(), sub3)
		menus.push(sub);
		sub3 = sub;
		for(unsigned int i = 0; i < titem->objects.size(); i++) {
			u = (Unit*) titem->objects[i];
			if(u->isActive() && !u->isHidden()) {
				MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
			}
		}
		while(titem && titem->rit == titem->items.rend()) {
			titem = titem->parent;
			menus.pop();
			if(menus.size() > 0) sub3 = menus.top();
		}	
		if(titem) {
			titem2 = &*titem->rit;
			++titem->rit;
			titem = titem2;
			titem->rit = titem->items.rbegin();	
		}
	}
	sub = sub2;
	for(unsigned int i = 0; i < unit_cats.objects.size(); i++) {
		u = (Unit*) unit_cats.objects[i];
		if(u->isActive() && !u->isHidden()) {
			MENU_ITEM_WITH_POINTER(u->title(true).c_str(), insert_unit, u)
		}
	}		
	
	MENU_SEPARATOR	
	item = gtk_menu_item_new_with_label(_("Prefixes"));
	gtk_widget_show (item);
	gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
	create_pmenu(item);		
	
}

/*
	generate unit submenu in result menu
*/
void create_umenu2() {
	GtkWidget *item;
	GtkWidget *sub, *sub2, *sub3;
	item = glade_xml_get_widget (main_glade, "menu_item_result_units");
	sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);	
	u_menu2 = sub;
	sub2 = sub;
	Unit *u;
	tree_struct *titem, *titem2;
	unit_cats.rit = unit_cats.items.rbegin();
	if(unit_cats.rit != unit_cats.items.rend()) {
		titem = &*unit_cats.rit;
		++unit_cats.rit;
		titem->rit = titem->items.rbegin();
	} else {
		titem = NULL;
	}
	stack<GtkWidget*> menus;
	menus.push(sub);
	sub3 = sub;
	while(titem) {
		SUBMENU_ITEM_PREPEND(titem->item.c_str(), sub3)
		menus.push(sub);
		sub3 = sub;
		for(unsigned int i = 0; i < titem->objects.size(); i++) {
			u = (Unit*) titem->objects[i];
			if(u->isActive() && !u->isHidden()) {
				MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
			}
		}
		while(titem && titem->rit == titem->items.rend()) {
			titem = titem->parent;
			menus.pop();
			if(menus.size() > 0) sub3 = menus.top();
		}	
		if(titem) {
			titem2 = &*titem->rit;
			++titem->rit;
			titem = titem2;
			titem->rit = titem->items.rbegin();	
		}
	}
	sub = sub2;
	for(unsigned int i = 0; i < unit_cats.objects.size(); i++) {
		u = (Unit*) unit_cats.objects[i];
		if(u->isActive() && !u->isHidden()) {
			MENU_ITEM_WITH_POINTER(u->title(true).c_str(), convert_to_unit, u)
		}
	}		
}

/*
	recreate unit menus and update unit manager (when units have changed)
*/
void update_umenus() {
	gtk_widget_destroy(u_menu);
	gtk_widget_destroy(u_menu2);
	generate_units_tree_struct();
	create_umenu();
	recreate_recent_units();
	create_umenu2();
	update_units_tree();
	update_completion();
}

/*
	generate variables submenu in expression menu
*/
void create_vmenu() {

	GtkWidget *item;
	GtkWidget *sub, *sub2, *sub3;
	item = glade_xml_get_widget (main_glade, "variables_menu");
	sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);
	
	v_menu = sub;
	sub2 = sub;
	Variable *v;
	tree_struct *titem, *titem2;
	variable_cats.rit = variable_cats.items.rbegin();
	if(variable_cats.rit != variable_cats.items.rend()) {
		titem = &*variable_cats.rit;
		++variable_cats.rit;
		titem->rit = titem->items.rbegin();
	} else {
		titem = NULL;
	}

	stack<GtkWidget*> menus;
	menus.push(sub);
	sub3 = sub;
	while(titem) {
		SUBMENU_ITEM_PREPEND(titem->item.c_str(), sub3)
		menus.push(sub);
		sub3 = sub;
		for(unsigned int i = 0; i < titem->objects.size(); i++) {
			v = (Variable*) titem->objects[i];
			if(v->isActive() && !v->isHidden()) {
				MENU_ITEM_WITH_POINTER(v->title(true).c_str(), insert_variable, v);
			}
		}
		while(titem && titem->rit == titem->items.rend()) {
			titem = titem->parent;
			menus.pop();
			if(menus.size() > 0) sub3 = menus.top();
		}	
		if(titem) {
			titem2 = &*titem->rit;
			++titem->rit;
			titem = titem2;
			titem->rit = titem->items.rbegin();	
		}

	}
	sub = sub2;

	for(unsigned int i = 0; i < variable_cats.objects.size(); i++) {
		v = (Variable*) variable_cats.objects[i];
		if(v->isActive() && !v->isHidden()) {
			MENU_ITEM_WITH_POINTER(v->title(true).c_str(), insert_variable, v);
		}
	}		

}

/*
	generate prefixes submenu in expression menu
*/
void create_pmenu(GtkWidget *item) {
//	GtkWidget *item;
	GtkWidget *sub;
//	item = glade_xml_get_widget (main_glade, "menu_item_expression_prefixes");
	sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);	
	int index = 0;
	Prefix *p = CALCULATOR->getPrefix(index);
	while(p) {
		gchar *gstr = g_strdup_printf("%s (10<sup>%i</sup>)", p->name(false, true).c_str(), p->exponent());
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
	GtkWidget *item;
	GtkWidget *sub;
	item = glade_xml_get_widget (main_glade, "menu_item_result_prefixes");
	sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);	
	int index = 0;
	Prefix *p = CALCULATOR->getPrefix(index);
	while(p) {
		gchar *gstr = g_strdup_printf("%s (10<sup>%i</sup>)", p->name(false, true).c_str(), p->exponent());
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
	recreate_recent_variables();
	update_variables_tree();
	update_completion();
}

/*
	generate functions submenu in expression menu
*/
void create_fmenu() {
	GtkWidget *item;
	GtkWidget *sub, *sub2, *sub3;
	item = glade_xml_get_widget (main_glade, "functions_menu");
	sub = gtk_menu_new(); gtk_widget_show (sub); gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub);
	f_menu = sub;
	sub2 = sub;
	Function *f;
	tree_struct *titem, *titem2;
	function_cats.rit = function_cats.items.rbegin();
	if(function_cats.rit != function_cats.items.rend()) {
		titem = &*function_cats.rit;
		++function_cats.rit;
		titem->rit = titem->items.rbegin();
	} else {
		titem = NULL;
	}
	stack<GtkWidget*> menus;
	menus.push(sub);
	sub3 = sub;
	while(titem) {
		SUBMENU_ITEM_PREPEND(titem->item.c_str(), sub3)
		for(unsigned int i = 0; i < titem->objects.size(); i++) {
			f = (Function*) titem->objects[i];
			if(f->isActive() && !f->isHidden()) {
				MENU_ITEM_WITH_POINTER(f->title(true).c_str(), insert_function, f)
			}
		}
		menus.push(sub);
		sub3 = sub;
		while(titem && titem->rit == titem->items.rend()) {
			titem = titem->parent;
			menus.pop();
			if(menus.size() > 0) sub3 = menus.top();
		}	
		if(titem) {
			titem2 = &*titem->rit;
			++titem->rit;
			titem = titem2;
			titem->rit = titem->items.rbegin();	
		}
	}
	sub = sub2;
	for(unsigned int i = 0; i < function_cats.objects.size(); i++) {
		f = (Function*) function_cats.objects[i];
		if(f->isActive() && !f->isHidden()) {
			MENU_ITEM_WITH_POINTER(f->title(true).c_str(), insert_function, f)
		}
	}		
}

void update_completion() {
#if GTK_MINOR_VERSION >= 3
	GtkTreeIter iter;
	
	gtk_entry_set_completion(GTK_ENTRY(expression), NULL);
	
	completion = gtk_entry_completion_new();
	completion_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	//gtk_list_store_clear(completion_store);	
	string str;
	for(unsigned int i = 0; i < CALCULATOR->functions.size(); i++) {
		if(CALCULATOR->functions[i]->isActive()) {
			str = CALCULATOR->functions[i]->name();
			str += "()";
			gtk_list_store_append(completion_store, &iter);
			gtk_list_store_set(completion_store, &iter, 0, str.c_str(), 1, CALCULATOR->functions[i]->title().c_str(), -1);
		}
	}
	for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
		if(CALCULATOR->variables[i]->isActive()) {
			gtk_list_store_append(completion_store, &iter);
			gtk_list_store_set(completion_store, &iter, 0, CALCULATOR->variables[i]->name().c_str(), 1, CALCULATOR->variables[i]->title().c_str(), -1);
		}
	}
	for(unsigned int i = 0; i < CALCULATOR->units.size(); i++) {
		if(CALCULATOR->units[i]->isActive() && CALCULATOR->units[i]->unitType() != COMPOSITE_UNIT) {
			gtk_list_store_append(completion_store, &iter);
			gtk_list_store_set(completion_store, &iter, 0, CALCULATOR->units[i]->name().c_str(), 1, CALCULATOR->units[i]->title().c_str(), -1);
		}
	}
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
	gtk_entry_set_completion(GTK_ENTRY(expression), completion);
#endif
}

/*
	recreate functions menu and update function manager (when functions have changed)
*/
void update_fmenu() {
	gtk_widget_destroy(f_menu);
	generate_functions_tree_struct();
	create_fmenu();
	recreate_recent_functions();
	update_completion();
	update_functions_tree();
}


string get_value_string(const MathStructure &mstruct_, bool rlabel = false, Prefix *prefix = NULL) {
	printops.allow_non_usable = rlabel;
	printops.prefix = prefix;
	string str = CALCULATOR->printMathStructureTimeOut(mstruct_, 100000, printops);
	printops.allow_non_usable = false;
	printops.prefix = NULL;
	return str;
}

void draw_background(GdkPixmap *pixmap, gint w, gint h) {
//	if(resultview->style->bg_pixmap[GTK_WIDGET_STATE(resultview)]) {
//		gdk_draw_drawable(pixmap, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], resultview->style->bg_pixmap[GTK_WIDGET_STATE(resultview)], 0, 0, w, h, -1, -1);
//	} else {
		gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);
//	}
}
GdkPixmap *draw_structure(MathStructure &m, PrintOptions po = default_print_options, InternalPrintStruct ips = top_ips, gint *point_central = NULL) {

	if(ips.depth == 0 && po.is_approximate) *po.is_approximate = false;

	GdkPixmap *pixmap = NULL;
	gint w, h;
	gint central_point = 0;
	
	InternalPrintStruct ips_n = ips;
	
	switch(m.type()) {
		case STRUCT_NUMBER: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str, exp = "";
			bool exp_minus;
			ips_n.exp = &exp;
			ips_n.exp_minus = &exp_minus;
			if(ips.power_depth > 0) {
				str = TEXT_TAGS_SMALL;
			} else {
				str = TEXT_TAGS;
			}
			str += m.number().print(po, ips_n);
			if(!exp.empty()) {
				if(ips.power_depth > 0) {
					str += TEXT_TAGS_XSMALL "E" TEXT_TAGS_XSMALL_END;
				} else {
					str += TEXT_TAGS_SMALL "E" TEXT_TAGS_SMALL_END;
				}
				if(exp_minus) {
					str += "-";
				}
				str += exp;
			}
			if(ips.power_depth > 0) {
				str += TEXT_TAGS_SMALL_END;
			} else {
				str += TEXT_TAGS_END;
			}
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width;
			w += 1;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);	
			draw_background(pixmap, w, h);
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			break;
		}
		case STRUCT_SYMBOLIC: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(ips.power_depth > 0) {
				str = "<i>" TEXT_TAGS_SMALL;
			} else {
				str = "<i>" TEXT_TAGS;
			}
			str += m.symbol();
			if(ips.power_depth > 0) {
				str += TEXT_TAGS_SMALL_END "</i>";
			} else {
				str += TEXT_TAGS_END "</i>";
			}
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width;
			w += 1;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			draw_background(pixmap, w, h);
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			break;
		}
#ifdef HAVE_GIAC		
		case STRUCT_UNKNOWN: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(ips.power_depth > 0) {
				str = TEXT_TAGS_SMALL;
			} else {
				str = TEXT_TAGS;
			}
			str += m.unknown()->print();
			if(ips.power_depth > 0) {
				str += TEXT_TAGS_SMALL_END;
			} else {
				str += TEXT_TAGS_END;
			}
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width;
			w += 1;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			draw_background(pixmap, w, h);
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			break;
		}
#endif
		case STRUCT_ADDITION: {

			ips_n.depth++;
			
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> wpt;
			vector<gint> cpt;
			gint plus_w, plus_h, minus_w, minus_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
			
			CALCULATE_SPACE_W
			PangoLayout *layout_plus = gtk_widget_create_pango_layout(resultview, NULL);
			if(ips.power_depth > 0) {
				pango_layout_set_markup(layout_plus, TEXT_TAGS_SMALL "+" TEXT_TAGS_SMALL_END, -1);
			} else {
				pango_layout_set_markup(layout_plus, TEXT_TAGS "+" TEXT_TAGS_END, -1);
			}
			pango_layout_get_pixel_size(layout_plus, &plus_w, &plus_h);
			PangoLayout *layout_minus = gtk_widget_create_pango_layout(resultview, NULL);
			if(po.use_unicode_signs) {
				if(ips.power_depth > 0) {
					pango_layout_set_markup(layout_minus, TEXT_TAGS_SMALL SIGN_MINUS TEXT_TAGS_SMALL_END, -1);
				} else {
					pango_layout_set_markup(layout_minus, TEXT_TAGS SIGN_MINUS TEXT_TAGS_END, -1);
				}			
			} else {
				if(ips.power_depth > 0) {
					pango_layout_set_markup(layout_minus, TEXT_TAGS_SMALL "-" TEXT_TAGS_SMALL_END, -1);
				} else {
					pango_layout_set_markup(layout_minus, TEXT_TAGS "-" TEXT_TAGS_END, -1);
				}			
			}
			pango_layout_get_pixel_size(layout_minus, &minus_w, &minus_h);
			for(unsigned int i = 0; i < m.size(); i++) {
				hetmp = 0;		
				if(m[i].type() == STRUCT_NEGATE && i > 0) {
					ips_n.wrap = m[i][0].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
					pixmap_terms.push_back(draw_structure(m[i][0], po, ips_n, &hetmp));
				} else {
					ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
					pixmap_terms.push_back(draw_structure(m[i], po, ips_n, &hetmp));
				}
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[i]), &wtmp, &htmp);
				hpt.push_back(htmp);
				cpt.push_back(hetmp);
				wpt.push_back(wtmp);
				w += wtmp;
				if(m[i].type() == STRUCT_NEGATE && i > 0) {
					w += minus_w;
					if(minus_h / 2 > dh) {
						dh = minus_h / 2;
					}
					if(minus_h / 2 + minus_h % 2 > uh) {
						uh = minus_h / 2 + minus_h % 2;
					}						
				} else if(i > 0) {
					w += plus_w;
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
			draw_background(pixmap, w, h);			
			w = 0;
			for(unsigned int i = 0; i < pixmap_terms.size(); i++) {
				if(i > 0) {
					w += space_w;
					if(m[i].type() == STRUCT_NEGATE) {
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
		case STRUCT_NEGATE: {

			ips_n.depth++;

			gint minus_w, minus_h, uh, dh, h, w, ctmp, htmp, wtmp, hpa, cpa, wpa;
			
			PangoLayout *layout_minus = gtk_widget_create_pango_layout(resultview, NULL);
			
			if(po.use_unicode_signs) {
				if(ips.power_depth > 0) {
					pango_layout_set_markup(layout_minus, TEXT_TAGS_SMALL SIGN_MINUS TEXT_TAGS_SMALL_END, -1);
				} else {
					pango_layout_set_markup(layout_minus, TEXT_TAGS SIGN_MINUS TEXT_TAGS_END, -1);
				}			
			} else {
				if(ips.power_depth > 0) {
					pango_layout_set_markup(layout_minus, TEXT_TAGS_SMALL "-" TEXT_TAGS_SMALL_END, -1);
				} else {
					pango_layout_set_markup(layout_minus, TEXT_TAGS "-" TEXT_TAGS_END, -1);
				}			
			}
			pango_layout_get_pixel_size(layout_minus, &minus_w, &minus_h);

			w = minus_w + 1;
			uh = minus_h / 2 + minus_h % 2;
			dh = minus_h / 2;
			
			ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
			GdkPixmap *pixmap_arg = draw_structure(m[0], po, ips_n, &ctmp);
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_arg), &wtmp, &htmp);
			hpa = htmp;
			cpa = ctmp;
			wpa = wtmp;
			w += wtmp;
			if(ctmp > dh) {
				dh = ctmp;
			}
			if(htmp - ctmp > uh) {
				uh = htmp - ctmp;
			}				
			
			h = uh + dh;
			central_point = dh;

			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);
			
			w = 0;
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - minus_h / 2 - minus_h % 2, layout_minus);	
			w += minus_w + 1;
			gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_arg), 0, 0, w, uh - (hpa - cpa), -1, -1);
			g_object_unref(pixmap_arg);
			
			g_object_unref(layout_minus);		
			break;
		}
		case STRUCT_MULTIPLICATION: {

			ips_n.depth++;
			
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> wpt;
			vector<gint> cpt;
			gint mul_w, mul_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
			
			CALCULATE_SPACE_W
			PangoLayout *layout_mul = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(ips.power_depth > 0) {
				if(multi_sign == SIGN_MULTIPLICATION) {
					str = TEXT_TAGS_XSMALL;
					str += multi_sign;
					str += TEXT_TAGS_XSMALL_END;
				} else {
					str = TEXT_TAGS_SMALL;
					str += multi_sign;
					str += TEXT_TAGS_SMALL_END;
				}
			} else {
				if(multi_sign == SIGN_MULTIPLICATION) {
					str = TEXT_TAGS_SMALL;
					str += multi_sign;
					str += TEXT_TAGS_SMALL_END;
				} else {
					str = TEXT_TAGS;
					str += multi_sign;
					str += TEXT_TAGS_END;
				}
			}
			pango_layout_set_markup(layout_mul, str.c_str(), -1);
			pango_layout_get_pixel_size(layout_mul, &mul_w, &mul_h);
			bool par_prev = false;
			vector<int> nm;
			for(unsigned int i = 0; i < m.size(); i++) {
				hetmp = 0;		
				ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_terms.push_back(draw_structure(m[i], po, ips_n, &hetmp));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[i]), &wtmp, &htmp);
				hpt.push_back(htmp);
				cpt.push_back(hetmp);
				wpt.push_back(wtmp);
				w += wtmp;
				if(!po.short_multiplication && i > 0) {
					w += mul_w + space_w * 2;
					if(mul_h / 2 > dh) {
						dh = mul_h / 2;
					}
					if(mul_h / 2 + mul_h % 2 > uh) {
						uh = mul_h / 2 + mul_h % 2;
					}
					nm.push_back(-1);
				} else if(i > 0) {
					nm.push_back(m[i].neededMultiplicationSign(po, ips_n, m, i + 1, ips_n.wrap, par_prev, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0));
					switch(nm[i]) {
						case MULTIPLICATION_SIGN_SPACE: {
							w += space_w;
							break;
						}
						case MULTIPLICATION_SIGN_OPERATOR: {
							w += mul_w + space_w * 2;
							if(mul_h / 2 > dh) {
								dh = mul_h / 2;
							}
							if(mul_h / 2 + mul_h % 2 > uh) {
								uh = mul_h / 2 + mul_h % 2;
							}
							break;
						}
						case MULTIPLICATION_SIGN_OPERATOR_SHORT: {
							w += mul_w;
							if(mul_h / 2 > dh) {
								dh = mul_h / 2;
							}
							if(mul_h / 2 + mul_h % 2 > uh) {
								uh = mul_h / 2 + mul_h % 2;
							}
							break;
						}
						default: {
							if(m[i - 1].isNumber()) w++;
						}
					}
				} else {
					nm.push_back(-1);
				}
				if(htmp - hetmp > uh) {
					uh = htmp - hetmp;
				}
				if(hetmp > dh) {
					dh = hetmp;
				}				
			}
			central_point = dh;
			h = dh + uh;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);			
			w = 0;
			for(unsigned int i = 0; i < pixmap_terms.size(); i++) {
				if(!po.short_multiplication && i > 0) {
					w += space_w;
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - mul_h / 2 - mul_h % 2, layout_mul);
					w += mul_w;
					w += space_w;
				} else if(i > 0) {
					switch(nm[i]) {
						case MULTIPLICATION_SIGN_SPACE: {
							w += space_w;
							break;
						}
						case MULTIPLICATION_SIGN_OPERATOR: {
							w += space_w;
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - mul_h / 2 - mul_h % 2, layout_mul);
							w += mul_w;
							w += space_w;
							break;
						}
						case MULTIPLICATION_SIGN_OPERATOR_SHORT: {
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - mul_h / 2 - mul_h % 2, layout_mul);
							w += mul_w;
							break;
						}
						default: {
							if(m[i - 1].isNumber()) w++;
						}
					}
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_terms[i]), 0, 0, w, uh - (hpt[i] - cpt[i]), -1, -1);
				w += wpt[i];
				g_object_unref(pixmap_terms[i]);
			}
			g_object_unref(layout_mul);
			break;
		}
		case STRUCT_INVERSE: {}
		case STRUCT_DIVISION: {
			
			ips_n.depth++;
			ips_n.division_depth++;
			
			gint den_uh, den_w, den_dh, num_w, num_dh, num_uh, dh = 0, uh = 0, w = 0, h = 0, one_w, one_h;
			
			GdkPixmap *num_pixmap = NULL, *den_pixmap = NULL, *pixmap_one = NULL;
			if(m.type() == STRUCT_DIVISION) {
				ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				num_pixmap = draw_structure(m[0], po, ips_n, &num_dh);
				gdk_drawable_get_size(GDK_DRAWABLE(num_pixmap), &num_w, &h);
				num_uh = h - num_dh;
			} else {
				MathStructure onestruct(1, 1);
				pixmap_one = draw_structure(onestruct, po, ips_n, NULL);	
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_one), &one_w, &one_h);
				num_w = one_w; num_dh = one_h / 2; num_uh = one_h - num_dh;
			}
			if(m.type() == STRUCT_DIVISION) {
				ips_n.wrap = m[1].needsParenthesis(po, ips_n, m, 2, ips.division_depth > 1 || ips.power_depth > 0, ips.power_depth > 0);
				den_pixmap = draw_structure(m[1], po, ips_n, &den_dh);
			} else {
				ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 2, ips.division_depth > 1 || ips.power_depth > 0, ips.power_depth > 0);
				den_pixmap = draw_structure(m[0], po, ips_n, &den_dh);
			}
			gdk_drawable_get_size(GDK_DRAWABLE(den_pixmap), &den_w, &h);
			den_uh = h - den_dh;
			h = 0;
			bool flat = ips.division_depth > 0 || ips.power_depth > 0;
			if(!flat && po.place_units_separately) {
				flat = true;
				for(unsigned int i = 0; i < m.size(); i++) {
					if(m[i].isMultiplication()) {
						for(unsigned int i2 = 0; i2 < m[i].size(); i2++) {
							if(!m[i][i2].isUnit_exp()) {
								flat = false;
								break;
							}
						}
						if(!flat) break;
					} else if(!m[i].isUnit_exp()) {
						flat = false;
						break;
					}
				}
			}
			if(flat) {
				gint div_w, div_h;
				PangoLayout *layout_div = gtk_widget_create_pango_layout(resultview, NULL);
				CALCULATE_SPACE_W
				if(po.use_unicode_signs) {
					if(ips.power_depth > 0) {
						pango_layout_set_markup(layout_div, TEXT_TAGS_SMALL SIGN_DIVISION TEXT_TAGS_SMALL_END, -1);
					} else {
						pango_layout_set_markup(layout_div, TEXT_TAGS SIGN_DIVISION TEXT_TAGS_END, -1);
					}
				} else {
					if(ips.power_depth > 0) {
						pango_layout_set_markup(layout_div, TEXT_TAGS_SMALL "/" TEXT_TAGS_SMALL_END, -1);
					} else {
						pango_layout_set_markup(layout_div, TEXT_TAGS "/" TEXT_TAGS_END, -1);
					}
				}
				pango_layout_get_pixel_size(layout_div, &div_w, &div_h);
				w = num_w + den_w + space_w + space_w + div_w;
				dh = num_dh; uh = num_uh;
				if(den_dh > dh) h = den_dh;
				if(den_uh > uh) uh = den_uh;
				if(div_h / 2 > dh) {
					dh = div_h / 2;
				}
				if(div_h / 2 + div_h % 2 > uh) {
					uh = div_h / 2 + div_h % 2;
				}
				h = uh + dh;
				central_point = dh;
				pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
				draw_background(pixmap, w, h);
				w = 0;
				if(m.type() == STRUCT_DIVISION) {
					gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(num_pixmap), 0, 0, w, uh - num_uh, -1, -1);
				} else {
					gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_one), 0, 0, w, uh - one_h / 2 - one_h % 2, -1, -1);
				}
				w += num_w;
				w += space_w;
				gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - div_h / 2 - div_h % 2, layout_div);	
				w += div_w;
				w += space_w;
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(den_pixmap), 0, 0, w, uh - den_uh, -1, -1);
				g_object_unref(layout_div);
			} else {
				gint wfr;
				dh = den_dh + den_uh + 3;
				uh = num_dh + num_uh + 3;
				wfr = den_w;
				if(num_w > wfr) wfr = num_w;
				wfr += 2;
				w = wfr;
				h = uh + dh;
				central_point = dh;
				pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
				draw_background(pixmap, w, h);
				w = 0;
				if(m.type() == STRUCT_DIVISION) {
					gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(num_pixmap), 0, 0, w + (wfr - num_w) / 2, uh - 3 - num_uh - num_dh, -1, -1);
				} else {
					gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_one), 0, 0, w + (wfr - one_w) / 2, uh - 3 - one_h, -1, -1);
				}
				gdk_draw_line(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh, w + wfr, uh);
				gdk_draw_line(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh + 1, w + wfr, uh + 1);
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(den_pixmap), 0, 0, w + (wfr - den_w) / 2, uh + 3, -1, -1);
			}
			if(num_pixmap) g_object_unref(num_pixmap);
			if(den_pixmap) g_object_unref(den_pixmap);
			if(pixmap_one) g_object_unref(pixmap_one);
			break;
		}
		case STRUCT_POWER: {

			ips_n.depth++;
			
			gint base_w, base_h, exp_w, exp_h, w = 0, h = 0, ctmp = 0, power_w, power_h;
			CALCULATE_SPACE_W
			ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
			GdkPixmap *pixmap_base = draw_structure(m[0], po, ips_n, &central_point);			
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_base), &base_w, &base_h);
			
			PangoLayout *layout_power = NULL;
			if(ips.power_depth > 0) {
				layout_power = gtk_widget_create_pango_layout(resultview, NULL);
				pango_layout_set_markup(layout_power, TEXT_TAGS_SMALL "^" TEXT_TAGS_SMALL_END, -1);
				pango_layout_get_pixel_size(layout_power, &power_w, &power_h);			
			}
			ips_n.power_depth++;
			ips_n.wrap = m[1].needsParenthesis(po, ips_n, m, 2, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
			GdkPixmap *pixmap_exp = draw_structure(m[1], po, ips_n, &ctmp);
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_exp), &exp_w, &exp_h);
			
			h = base_h;
			w = base_w;
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
			
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);

			w = 0;
			gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_base), 0, 0, w, h - base_h, -1, -1);
			w += base_w;
			if(layout_power) {
				gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, 0, layout_power);
				w += power_w;
			}
			gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_exp), 0, 0, w, 0, -1, -1);
			
			g_object_unref(pixmap_base);
			g_object_unref(pixmap_exp);
			if(layout_power) g_object_unref(layout_power);
			break;
		}
		case STRUCT_COMPARISON: {}
		case STRUCT_AND: {}
		case STRUCT_XOR: {}
		case STRUCT_OR: {
			
			ips_n.depth++;
			
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> wpt;
			vector<gint> cpt;
			gint sign_w, sign_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
			CALCULATE_SPACE_W
			
			for(unsigned int i = 0; i < m.size(); i++) {
				hetmp = 0;		
				ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_terms.push_back(draw_structure(m[i], po, ips_n, &hetmp));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[i]), &wtmp, &htmp);
				hpt.push_back(htmp);
				cpt.push_back(hetmp);
				wpt.push_back(wtmp);
				w += wtmp;
				if(htmp - hetmp > uh) {
					uh = htmp - hetmp;
				}
				if(hetmp > dh) {
					dh = hetmp;
				}				
			}
			
			PangoLayout *layout_sign = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(ips.power_depth > 0) {
				str = TEXT_TAGS_SMALL;
			} else {
				str = TEXT_TAGS;
			}	
			if(m.type() == STRUCT_COMPARISON) {
				switch(m.comparisonType()) {
					case COMPARISON_EQUALS: {
						if(ips.depth == 0 && po.use_unicode_signs && (*po.is_approximate || m.isApproximate())) {
							str += SIGN_ALMOST_EQUAL;
						} else {
							str += "=";
						}
						break;
					}
					case COMPARISON_NOT_EQUALS: {
						if(po.use_unicode_signs) {
							str += SIGN_NOT_EQUAL;
						} else {
							str += "!=";
						}
						break;
					}
					case COMPARISON_GREATER: {
						str += "&gt;";
						break;
					}
					case COMPARISON_LESS: {
						str += "&lt;";
						break;
					}
					case COMPARISON_EQUALS_GREATER: {
						if(po.use_unicode_signs) {
							str += SIGN_GREATER_OR_EQUAL;
						} else {
							str += "&gt;=";
						}
						break;
					}
					case COMPARISON_EQUALS_LESS: {
						if(po.use_unicode_signs) {
							str += SIGN_LESS_OR_EQUAL;
						} else {
							str += "&lt;=";
						}
						break;
					}
				}
			} else if(m.type() == STRUCT_AND) {
				str += "&amp;";
			} else if(m.type() == STRUCT_OR) {
				str += "|";
			} else if(m.type() == STRUCT_XOR) {
				str += "XOR";
			}
			
			if(ips.power_depth > 0) {
				str += TEXT_TAGS_SMALL_END;
			} else {
				str += TEXT_TAGS_END;
			}
			pango_layout_set_markup(layout_sign, str.c_str(), -1);
			pango_layout_get_pixel_size(layout_sign, &sign_w, &sign_h);
			if(sign_h / 2 > dh) {
				dh = sign_h / 2;
			}
			if(sign_h / 2 + sign_h % 2 > uh) {
				uh = sign_h / 2 + sign_h % 2;
			}
			w += sign_w * (m.size() - 1);
			
			w += space_w * (pixmap_terms.size() - 1) * 2;
			
			central_point = dh;
			h = dh + uh;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);
			w = 0;
			for(unsigned int i = 0; i < pixmap_terms.size(); i++) {
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
		case STRUCT_NOT: {

			ips_n.depth++;

			gint not_w, not_h, uh, dh, h, w, ctmp, htmp, wtmp, hpa, cpa, wpa;
			
			PangoLayout *layout_not = gtk_widget_create_pango_layout(resultview, NULL);
			
			if(ips.power_depth > 0) {
				pango_layout_set_markup(layout_not, TEXT_TAGS_SMALL "!" TEXT_TAGS_SMALL_END, -1);
			} else {
				pango_layout_set_markup(layout_not, TEXT_TAGS "!" TEXT_TAGS_END, -1);
			}
			pango_layout_get_pixel_size(layout_not, &not_w, &not_h);

			w = not_w + 1;
			uh = not_h / 2 + not_h % 2;
			dh = not_h / 2;
			
			ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
			GdkPixmap *pixmap_arg = draw_structure(m[0], po, ips_n, &ctmp);
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_arg), &wtmp, &htmp);
			hpa = htmp;
			cpa = ctmp;
			wpa = wtmp;
			w += wtmp;
			if(ctmp > dh) {
				dh = ctmp;
			}
			if(htmp - ctmp > uh) {
				uh = htmp - ctmp;
			}				
			
			h = uh + dh;
			central_point = dh;

			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);
			
			w = 0;
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - not_h / 2 - not_h % 2, layout_not);	
			w += not_w + 1;
			gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_arg), 0, 0, w, uh - (hpa - cpa), -1, -1);
			g_object_unref(pixmap_arg);
			
			g_object_unref(layout_not);		
			break;
		}
		case STRUCT_VECTOR: {
			
			ips_n.depth++;
			
			if(m.isMatrix()) {
				gint wtmp, htmp, ctmp = 0, w = 0, h = 0;
				CALCULATE_SPACE_W
				vector<gint> col_w;
				vector<gint> row_h;
				vector<gint> row_uh;
				vector<gint> row_dh;									
				vector<vector<gint> > element_w;
				vector<vector<gint> > element_h;
				vector<vector<gint> > element_c;			
				vector<vector<GdkPixmap*> > pixmap_elements;
				element_w.resize(m.size());
				element_h.resize(m.size());
				element_c.resize(m.size());						
				pixmap_elements.resize(m.size());
				PangoLayout *layout_comma = gtk_widget_create_pango_layout(resultview, NULL);
				string str;
				gint comma_w = 0, comma_h = 0;
				MARKUP_STRING(str, po.comma())
				pango_layout_set_markup(layout_comma, str.c_str(), -1);		
				pango_layout_get_pixel_size(layout_comma, &comma_w, &comma_h);
				for(unsigned int index_r = 0; index_r < m.size(); index_r++) {
					for(unsigned int index_c = 0; index_c < m[index_r].size(); index_c++) {
						ctmp = 0;
						ips_n.wrap = m[index_r][index_c].needsParenthesis(po, ips_n, m, index_r + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
						pixmap_elements[index_r].push_back(draw_structure(m[index_r][index_c], po, ips_n, &ctmp));
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
				for(unsigned int i = 0; i < col_w.size(); i++) {
					w += col_w[i];
					if(i != 0) {
						w += space_w * 2;
					}
				}
	
				gint wlr, wll;
				wll = 10;
				wlr = 10;
			
				w += wlr + 1;
				w += wll + 3;
				central_point = h / 2;
				pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
				draw_background(pixmap, w, h);
				GdkGC *line_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
				gdk_gc_copy(line_gc, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)]);
				gdk_gc_set_line_attributes(line_gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
				w = 1;
				gdk_draw_line(GDK_DRAWABLE(pixmap), line_gc, w, 1, w, h - 1);
				gdk_draw_line(GDK_DRAWABLE(pixmap), line_gc, w, 1, w + 7, 1);
				gdk_draw_line(GDK_DRAWABLE(pixmap), line_gc, w, h - 1, w + 7, h - 1);
				h = 2;
				for(unsigned int index_r = 0; index_r < m.size(); index_r++) {
					w = wll + 1;
					for(unsigned int index_c = 0; index_c < m[index_r].size(); index_c++) {
						gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_elements[index_r][index_c]), 0, 0, w + (col_w[index_c] - element_w[index_r][index_c]), h + row_uh[index_r] - (element_h[index_r][index_c] - element_c[index_r][index_c]), -1, -1);				
						w += col_w[index_c];
						if(index_c != m[index_r].size() - 1) {
							w += space_w * 2;
						}
						g_object_unref(pixmap_elements[index_r][index_c]);
					}
					h += row_h[index_r];
					h += 4;
				}
				h -= 4;
				h += 2;
				//w += 1;
				w += wll - 7;
				gdk_draw_line(GDK_DRAWABLE(pixmap), line_gc, w + 7, 1, w + 7, h - 1);
				gdk_draw_line(GDK_DRAWABLE(pixmap), line_gc, w, 1, w + 7, 1);
				gdk_draw_line(GDK_DRAWABLE(pixmap), line_gc, w, h - 1, w + 7, h - 1);
				g_object_unref(layout_comma);
				gdk_gc_unref(line_gc);
				break;
			}
			
			gint comma_w, comma_h, uh = 0, dh = 0, h = 0, w = 0, ctmp, htmp, wtmp, arc_w, arc_h;
			vector<GdkPixmap*> pixmap_args;
			vector<gint> hpa;
			vector<gint> cpa;			
			vector<gint> wpa;
			
			CALCULATE_SPACE_W
			PangoLayout *layout_comma = gtk_widget_create_pango_layout(resultview, NULL);
			string str, func_str;
			MARKUP_STRING(str, CALCULATOR->getComma())
			pango_layout_set_markup(layout_comma, str.c_str(), -1);		
			pango_layout_get_pixel_size(layout_comma, &comma_w, &comma_h);

			if(m.size() == 0) {
				PangoLayout *layout_one = gtk_widget_create_pango_layout(resultview, NULL);
				MARKUP_STRING(str, "1")
				pango_layout_set_markup(layout_one, str.c_str(), -1);
				pango_layout_get_pixel_size(layout_one, &w, &h);
				uh = h / 2 + h % 2;
				dh = h / 2;
				w = 2;
				g_object_unref(layout_one);
			}
			for(unsigned int index = 0; index < m.size(); index++) {
				ips_n.wrap = m[index].needsParenthesis(po, ips_n, m, index + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_args.push_back(draw_structure(m[index], po, ips_n, &ctmp));
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
			if(uh > dh) dh = uh;
			else uh = dh;
			h = uh + dh;
			central_point = dh;
			arc_h = dh * 2;
			arc_w = arc_h / 6;
			w += arc_w * 2 + 2;
			w += 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);
			GdkGC *line_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
			gdk_gc_copy(line_gc, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)]);
			if(ips.power_depth > 0) {
				gdk_gc_set_line_attributes(line_gc, 1, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
			} else {
				gdk_gc_set_line_attributes(line_gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
			}
			w = 0;
			gdk_draw_arc(GDK_DRAWABLE(pixmap), line_gc, FALSE, w + 1, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 90 * 64, 180 * 64);	
			w += arc_w + 2;
			if(m.size() == 0) w += 2;
			for(unsigned int index = 0; index < m.size(); index++) {
				if(index > 0) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - comma_h / 2 - comma_h % 2, layout_comma);	
					w += comma_w;
					w += space_w;
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_args[index]), 0, 0, w, uh - (hpa[index] - cpa[index]), -1, -1);
				w += wpa[index];
				g_object_unref(pixmap_args[index]);
			}	
			gdk_draw_arc(GDK_DRAWABLE(pixmap), line_gc, FALSE, w - arc_w, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 270 * 64, 180 * 64);			
			w += arc_w;				
			
			g_object_unref(layout_comma);
			gdk_gc_unref(line_gc);

			break;
		}
		case STRUCT_UNIT: {
			string str, str2;
			if(ips.power_depth > 0) {
				str = TEXT_TAGS_SMALL;
			} else {
				str = TEXT_TAGS;
			}
			if(po.abbreviate_units && m.unit()->name(po.use_unicode_signs).find("_") == string::npos) {
				if(m.prefix()) str += m.prefix()->shortName(true, po.use_unicode_signs);
				str += m.unit()->shortName(po.use_unicode_signs);
			} else if(m.isPlural()) {
				if(m.prefix()) str += m.prefix()->longName(true, po.use_unicode_signs);
				str += m.unit()->plural(true, po.use_unicode_signs);
			} else {
				if(m.prefix()) str += m.prefix()->longName(true, po.use_unicode_signs);
				str += m.unit()->singular(true, po.use_unicode_signs);
			}
			gsub("_", " ", str);
			if(ips.power_depth > 0) {
				str += TEXT_TAGS_SMALL_END;
			} else {
				str += TEXT_TAGS_END;
			}		
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			pango_layout_set_markup(layout, str.c_str(), -1);
			pango_layout_get_pixel_size(layout, &w, &h);
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, 0, layout);	
			g_object_unref(layout);
			break;
		}
		case STRUCT_VARIABLE: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(m.variable() == CALCULATOR->v_i) {
				if(ips.power_depth > 0) {
					str = TEXT_TAGS_SMALL;
				} else {
					str = TEXT_TAGS;
				}
			} else {
				if(ips.power_depth > 0) {
					str = "<i>" TEXT_TAGS_SMALL;
				} else {
					str = "<i>" TEXT_TAGS;
				}
			}
			str += m.variable()->name(po.use_unicode_signs);
			if(m.variable() == CALCULATOR->v_i) {
				if(ips.power_depth > 0) {
					str += TEXT_TAGS_SMALL_END;
				} else {
					str += TEXT_TAGS_END;
				}
			} else {
				if(ips.power_depth > 0) {
					str += TEXT_TAGS_SMALL_END "</i>";
				} else {
					str += TEXT_TAGS_END "</i>";
				}
			} 
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width;
			w += 1;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);	
			draw_background(pixmap, w, h);
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			break;
		}
		case STRUCT_FUNCTION: {

			ips_n.depth++;
			
			gint comma_w, comma_h, function_w, function_h, uh, dh, h, w, ctmp, htmp, wtmp, arc_w, arc_h;
			vector<GdkPixmap*> pixmap_args;
			vector<gint> hpa;
			vector<gint> cpa;			
			vector<gint> wpa;
			
			CALCULATE_SPACE_W
			PangoLayout *layout_comma = gtk_widget_create_pango_layout(resultview, NULL);
			string str, func_str;
			MARKUP_STRING(str, po.comma())
			pango_layout_set_markup(layout_comma, str.c_str(), -1);		
			pango_layout_get_pixel_size(layout_comma, &comma_w, &comma_h);
			PangoLayout *layout_function = gtk_widget_create_pango_layout(resultview, NULL);
			func_str = m.function()->name(po.use_unicode_signs);
			gsub("_", " ", func_str);
			MARKUP_STRING(str, func_str)
			pango_layout_set_markup(layout_function, str.c_str(), -1);			
			pango_layout_get_pixel_size(layout_function, &function_w, &function_h);
			w = function_w + 1;
			uh = function_h / 2 + function_h % 2;
			dh = function_h / 2;

			for(unsigned int index = 0; index < m.size(); index++) {
				ips_n.wrap = m[index].needsParenthesis(po, ips_n, m, index + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_args.push_back(draw_structure(m[index], po, ips_n, &ctmp));
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
			if(uh > dh) dh = uh;
			else uh = dh;
			h = uh + dh;
			central_point = dh;
			arc_h = dh * 2;
			arc_w = arc_h / 6;
			w += arc_w * 2 + 2;
			w += 2;

			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);
			
			GdkGC *line_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
			gdk_gc_copy(line_gc, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)]);
			if(ips.power_depth > 0) {
				gdk_gc_set_line_attributes(line_gc, 1, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
			} else {
				gdk_gc_set_line_attributes(line_gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
			}
			w = 0;
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - function_h / 2 - function_h % 2, layout_function);	
			w += function_w + 1;
			gdk_draw_arc(GDK_DRAWABLE(pixmap), line_gc, FALSE, w + 1, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 90 * 64, 180 * 64);	
			w += arc_w + 2;
			for(unsigned int index = 0; index < m.size(); index++) {
				if(index > 0) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - comma_h / 2 - comma_h % 2, layout_comma);	
					w += comma_w;
					w += space_w;
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_args[index]), 0, 0, w, uh - (hpa[index] - cpa[index]), -1, -1);
				w += wpa[index];
				g_object_unref(pixmap_args[index]);
			}	
			gdk_draw_arc(GDK_DRAWABLE(pixmap), line_gc, FALSE, w - arc_w, uh - arc_h / 2 - arc_h % 2, arc_w * 2, arc_h, 270 * 64, 180 * 64);			
			w += arc_w;				
			
			g_object_unref(layout_comma);
			g_object_unref(layout_function);
			gdk_gc_unref(line_gc);

			break;
		}
		case STRUCT_UNDEFINED: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			if(ips.power_depth > 0) {
				str = TEXT_TAGS_SMALL;
			} else {
				str = TEXT_TAGS;
			}
			str += _("undefined");
			if(ips.power_depth > 0) {
				str += TEXT_TAGS_SMALL_END;
			} else {
				str += TEXT_TAGS_END;
			}
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width;
			w += 1;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);	
			draw_background(pixmap, w, h);
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			break;
		}
	}
	if(ips.wrap && pixmap) {
		gint w, h, base_h, base_w;
		gdk_drawable_get_size(GDK_DRAWABLE(pixmap), &base_w, &base_h);
		h = base_h;
		w = base_w;
		gint arc_base_w = base_h / 6;
		base_h += 4;
		central_point += 2;
		w += arc_base_w * 2 + 4;
		GdkPixmap *pixmap_old = pixmap;
		pixmap = gdk_pixmap_new(resultview->window, w, h, -1);	
		draw_background(pixmap, w, h);
		GdkGC *line_gc = gdk_gc_new(GDK_DRAWABLE(pixmap));
		gdk_gc_copy(line_gc, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)]);
		w = 1;
		if(ips.power_depth > 0) {
			gdk_gc_set_line_attributes(line_gc, 1, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
		} else {
			gdk_gc_set_line_attributes(line_gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
		}
		gdk_draw_arc(GDK_DRAWABLE(pixmap), line_gc, FALSE, w, 0, arc_base_w * 2, h, 90 * 64, 180 * 64);
		w += arc_base_w + 1;
		gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_old), 0, 0, w, (h - base_h) / 2, -1, -1);
		w += base_w;
		gdk_draw_arc(GDK_DRAWABLE(pixmap), line_gc, FALSE, w - arc_base_w, 0, arc_base_w * 2, h, 270 * 64, 180 * 64);			
		g_object_unref(pixmap_old);
		gdk_gc_unref(line_gc);
	}
	if(ips.depth == 0 && !(m.isComparison() && ((m.comparisonType() == COMPARISON_EQUALS && po.use_unicode_signs) || (!*po.is_approximate && !m.isApproximate()))) && pixmap) {
		gint w, h, wle, hle, w_new, h_new;
		gdk_drawable_get_size(GDK_DRAWABLE(pixmap), &w, &h);			
		GdkPixmap *pixmap_old = pixmap;
		PangoLayout *layout_equals = gtk_widget_create_pango_layout(resultview, NULL);
		if(*po.is_approximate || m.isApproximate()) {
			if(po.use_unicode_signs) {
				pango_layout_set_markup(layout_equals, TEXT_TAGS SIGN_ALMOST_EQUAL " " TEXT_TAGS_END, -1);
			} else {
				string str = TEXT_TAGS;
				str += _("approx.");
				str += " " TEXT_TAGS_END;
				pango_layout_set_markup(layout_equals, str.c_str(), -1);
			}
		} else {
			pango_layout_set_markup(layout_equals, TEXT_TAGS "= " TEXT_TAGS_END, -1);
		}
		pango_layout_get_pixel_size(layout_equals, &wle, &hle);
		w_new = w + wle;
		h_new = h;
		pixmap = gdk_pixmap_new(resultview->window, w_new, h_new, -1);			
		draw_background(pixmap, w_new, h_new);
		gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, h - central_point - hle / 2 - hle % 2, layout_equals);	
		gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_old), 0, 0, wle, 0, -1, -1);
		g_object_unref(pixmap_old);
		g_object_unref(layout_equals);
	}
	if(!pixmap) {
		pixmap = gdk_pixmap_new(resultview->window, 1, 1, -1);
		draw_background(pixmap, 1, 1);
	}
	if(point_central) *point_central = central_point;
	return pixmap;
}

void clearresult() {
	//draw_background(resultview->window, resultview->allocation.width, resultview->allocation.height);
	gdk_window_clear(resultview->window);
	if(pixbuf_result) {
		gdk_pixbuf_unref(pixbuf_result);
		pixbuf_result = NULL;
	}
}

void on_abort_display(GtkDialog *w, gint arg1, gpointer user_data) {
	pthread_cancel(view_thread);
	CALCULATOR->restoreState();
	CALCULATOR->clearBuffers();
	b_busy = false;
	pthread_create(&view_thread, &view_thread_attr, view_proc, view_pipe_r);
}

void *view_proc(void *pipe) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	
	FILE *view_pipe = (FILE*) pipe;
	
	while(true) {
	
		void *x = NULL;
		fread(&x, sizeof(void*), 1, view_pipe);
		MathStructure m(*((MathStructure*) x));
		printops.allow_non_usable = false;
		m.format(printops);
		result_text = m.print(printops);	
	
		if(result_text.length() > 1500) {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			pango_layout_set_markup(layout, _("result is too long\nsee history"), -1);
			gint w = 0, h = 0;
			pango_layout_get_pixel_size(layout, &w, &h);
			tmp_pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			gdk_draw_rectangle(tmp_pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
			gdk_draw_layout(GDK_DRAWABLE(tmp_pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			*printops.is_approximate = false;
		} else if(result_text == _("aborted")) {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			pango_layout_set_markup(layout, _("calculation was aborted"), -1);
			gint w = 0, h = 0;
			pango_layout_get_pixel_size(layout, &w, &h);
			tmp_pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			gdk_draw_rectangle(tmp_pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
			gdk_draw_layout(GDK_DRAWABLE(tmp_pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			*printops.is_approximate = false;
		} else {
			printops.allow_non_usable = true;
			tmp_pixmap = draw_structure(m, printops);
			printops.allow_non_usable = false;
		}
		b_busy = false;
	}
	return NULL;
}
gboolean on_event(GtkWidget *w, GdkEvent *e, gpointer user_data) {
	if(e->type == GDK_EXPOSE || e->type == GDK_PROPERTY_NOTIFY || e->type == GDK_CONFIGURE || e->type == GDK_FOCUS_CHANGE || e->type == GDK_VISIBILITY_NOTIFY) {
		return FALSE;
	}
	return TRUE;
}
/*
	set result in result widget and add to history widget
*/
void setResult(Prefix *prefix = NULL, bool update_history = true) {

	if(expression_has_changed) {
		execute_expression();
		if(!prefix) return;
	}
	
	b_busy = true;	

	GtkTextIter iter;
	GtkTextBuffer *tb = NULL;

	//update "ans" variables
	vans->set(*mstruct);
	vAns->set(*mstruct);

	if(update_history) {
		//result_text = expr;
		gtk_editable_select_region(GTK_EDITABLE(expression), 0, -1);
		display_errors();
		tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history")));
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
	if(pixbuf_result) {
		gdk_pixbuf_unref(pixbuf_result);
	}
	pixbuf_result = NULL;

	printops.prefix = prefix;
	tmp_pixmap = NULL;
	
	CALCULATOR->saveState();

	gulong handler_id = g_signal_connect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), "event", G_CALLBACK(on_event), NULL);

	fwrite(&mstruct, sizeof(void*), 1, view_pipe_w);
	fflush(view_pipe_w);

	struct timespec rtime;
	rtime.tv_sec = 0;
	rtime.tv_nsec = 10000000;
	int i = 0;
	while(b_busy && i < 50) {
		nanosleep(&rtime, NULL);
		i++;
	}
	i = 0;
	GtkWidget *dialog = NULL;

	if(b_busy) {
		dialog = glade_xml_get_widget (main_glade, "progress_dialog");
		gtk_window_set_title(GTK_WINDOW(dialog), "Processing...");
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (main_glade, "progress_label_message")), "Processing...");
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
		g_signal_connect(GTK_OBJECT(dialog), "response", G_CALLBACK(on_abort_display), NULL);
		gtk_widget_show(dialog);
	}
	rtime.tv_nsec = 100000000;
	while(b_busy) {
		while(gtk_events_pending()) gtk_main_iteration();
		nanosleep(&rtime, NULL);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(glade_xml_get_widget (main_glade, "progress_progressbar")));
	}

	b_busy = true;
	if(dialog) {
		gtk_widget_hide(dialog);
		gtk_widget_grab_focus(expression);
	}

	g_signal_handler_disconnect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), handler_id);

	gint w = 0, wr = 0, h = 0, hr = 0, h_new, w_new;
	if(!tmp_pixmap) {
		PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
		pango_layout_set_markup(layout, _("result processing was aborted"), -1);
		pango_layout_get_pixel_size(layout, &w, &h);
		tmp_pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
		gdk_draw_rectangle(tmp_pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
		gdk_draw_layout(GDK_DRAWABLE(tmp_pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
		g_object_unref(layout);
		*printops.is_approximate = false;
	}
	gdk_drawable_get_size(GDK_DRAWABLE(tmp_pixmap), &w, &h);	
	gtk_widget_get_size_request(resultview, &wr, &hr);	
	if(h < 50) {
		h_new = 50;
	} else {
		h_new = h;
	}
	if(w < glade_xml_get_widget(main_glade, "scrolled_result")->allocation.width - 20) {
		w_new = glade_xml_get_widget(main_glade, "scrolled_result")->allocation.width - 20;
	} else {
		w_new = w;
	}	
	
	if(wr != w_new || hr != h_new) {
		if(h_new > 200) {
			if(w_new > glade_xml_get_widget(main_glade, "scrolled_result")->allocation.width - 20) {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(main_glade, "scrolled_result")), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);		
			} else {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(main_glade, "scrolled_result")), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
			}
			gtk_widget_set_size_request(glade_xml_get_widget(main_glade, "scrolled_result"), -1, 200);						
		} else {
			if(w_new > glade_xml_get_widget(main_glade, "scrolled_result")->allocation.width - 20) {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(main_glade, "scrolled_result")), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);		
			} else {
				gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(glade_xml_get_widget(main_glade, "scrolled_result")), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			}	
			gtk_widget_set_size_request(glade_xml_get_widget(main_glade, "scrolled_result"), -1, -1);					
		}							
		gtk_widget_set_size_request(resultview, w_new, h_new);
		while(gtk_events_pending()) gtk_main_iteration();
	}

	GdkPixbuf *pixbuf_result_tmp = gdk_pixbuf_get_from_drawable(NULL, tmp_pixmap, NULL, 0, 0, 0, 0, -1, -1);
	if(pixbuf_result_tmp) {
		guchar *pixels = gdk_pixbuf_get_pixels(pixbuf_result_tmp);
		pixbuf_result = gdk_pixbuf_add_alpha(pixbuf_result_tmp, TRUE, pixels[0], pixels[1], pixels[2]);
		gdk_pixbuf_unref(pixbuf_result_tmp);
		if(pixbuf_result) {
			if(resultview->allocation.width > w) {
				//gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(tmp_pixmap), 0, 0, resultview->allocation.width - w, (resultview->allocation.height - h) / 2, -1, -1);
				gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, resultview->allocation.width - w, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			} else {
				//gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(tmp_pixmap), 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1);
				gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			}
		}
	}
	pixmap_result = tmp_pixmap;
	
	if(update_history) {
		if(result_text.length() > 500000) {
			result_text = "(...)";
		}
		if(!(*printops.is_approximate) && !mstruct->isApproximate()) {
			gtk_text_buffer_insert(tb, &iter, "=", -1);	
		} else {
			if(printops.use_unicode_signs) {
				gtk_text_buffer_insert(tb, &iter, SIGN_ALMOST_EQUAL, -1);		
			} else {
				string str_approx = "= ";
				str_approx += _("approx.");
				gtk_text_buffer_insert(tb, &iter, str_approx.c_str(), -1);
			}
		}
		gtk_text_buffer_insert(tb, &iter, " ", -1);	
		gtk_text_buffer_insert(tb, &iter, result_text.c_str(), -1);
		gtk_text_buffer_place_cursor(tb, &iter);
		while(CALCULATOR->error()) {
			CALCULATOR->nextError();
		}
	}
	printops.prefix = NULL;
	b_busy = false;
	display_errors();
}

void viewresult(Prefix *prefix = NULL) {
	setResult(prefix, false);
}

void set_prefix(GtkMenuItem *w, gpointer user_data) {
	//mstruct->clearPrefixes();
	setResult((Prefix*) user_data);
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
	for(unsigned int i = 0; i < expression_history.size(); i++) {
		if(expression_history[i] == str) {
			expression_history.erase(expression_history.begin() + i);
			break;
		}
	}
	if(expression_history.size() >= 15) {
		expression_history.pop_back();
	}
	expression_history.insert(expression_history.begin(), str);
	expression_history_index = 0;

	b_busy = true;
	gulong handler_id = g_signal_connect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), "event", G_CALLBACK(on_event), NULL);
	CALCULATOR->calculate(*mstruct, CALCULATOR->unlocalizeExpression(str), 0, evalops);
	struct timespec rtime;
	rtime.tv_sec = 0;
	rtime.tv_nsec = 10000000;
	int i = 0;
	while(CALCULATOR->busy() && i < 50) {
		nanosleep(&rtime, NULL);
		i++;
	}
	i = 0;
	GtkWidget *dialog = NULL;	
	if(CALCULATOR->busy()) {
		dialog = glade_xml_get_widget (main_glade, "progress_dialog");
		gtk_window_set_title(GTK_WINDOW(dialog), "Calculating...");
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (main_glade, "progress_label_message")), "Calculating...");
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
		g_signal_connect(GTK_OBJECT(dialog), "response", G_CALLBACK(on_abort_calculation), NULL);
		gtk_widget_show(dialog);
	}
	rtime.tv_nsec = 100000000;
	while(CALCULATOR->busy()) {
		while(gtk_events_pending()) gtk_main_iteration();
		nanosleep(&rtime, NULL);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(glade_xml_get_widget (main_glade, "progress_progressbar")));
	}
	if(dialog) {
		gtk_widget_hide(dialog);
		gtk_widget_grab_focus(expression);
	}
	b_busy = false;
	display_errors();
	result_text = str;
	setResult();
	g_signal_handler_disconnect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), handler_id);
	gtk_widget_grab_focus(expression);
	//gtk_editable_set_position(GTK_EDITABLE(expression), -1);
}



/*
	calculate position of expression menu
*/
void menu_e_posfunc(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
	GtkWidget *window = glade_xml_get_widget (main_glade, "main_window");
	gint root_x = 0, root_y = 0, size_x = 0, size_y = 0;
	GdkRectangle rect;
	gdk_window_get_frame_extents(window->window, &rect);
	gtk_window_get_position(GTK_WINDOW(window), &root_x, &root_y);
	gtk_window_get_size(GTK_WINDOW(window), &size_x, &size_y);
	*x = root_x + (rect.width - size_x) / 2 + glade_xml_get_widget (main_glade, "togglebutton_expression")->allocation.x + glade_xml_get_widget (main_glade, "togglebutton_expression")->allocation.width;
	*y = root_y + (rect.height - size_y) / 2 + glade_xml_get_widget (main_glade, "togglebutton_expression")->allocation.y;
	*push_in = false;
}

/*
	calculate position of result menu
*/
void menu_r_posfunc(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
	GtkWidget *window = glade_xml_get_widget (main_glade, "main_window");
	gint root_x = 0, root_y = 0, size_x = 0, size_y = 0;
	GdkRectangle rect;
	gdk_window_get_frame_extents(window->window, &rect);
	gtk_window_get_position(GTK_WINDOW(window), &root_x, &root_y);
	gtk_window_get_size(GTK_WINDOW(window), &size_x, &size_y);
	*x = root_x + (rect.width - size_x) / 2 + glade_xml_get_widget (main_glade, "togglebutton_result")->allocation.x + glade_xml_get_widget (main_glade, "togglebutton_result")->allocation.width;
	*y = root_y + (rect.height - size_y) / 2 + glade_xml_get_widget (main_glade, "togglebutton_result")->allocation.y;
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
	block_completion();
	gtk_editable_insert_text(GTK_EDITABLE(expression), name, strlen(name), &position);
	unblock_completion();
	gtk_editable_set_position(GTK_EDITABLE(expression), position);
	gtk_widget_grab_focus(expression);
	//unselect
	gtk_editable_select_region(GTK_EDITABLE(expression), position, position);
}

void recreate_recent_functions() {
	GtkWidget *item, *sub;
	sub = f_menu;
	recent_function_items.clear();
	bool b = false;
	for(unsigned int i = 0; i < recent_functions.size(); i++) {
		if(!CALCULATOR->hasFunction(recent_functions[i])) {
			recent_functions.erase(recent_functions.begin() + i);
			i--;
		} else {
			if(!b) {
				MENU_SEPARATOR_PREPEND
				b = true;
			}
			item = gtk_menu_item_new_with_label(recent_functions[i]->title(true).c_str()); 
			recent_function_items.push_back(item);
			gtk_widget_show(item); 
			gtk_menu_shell_prepend(GTK_MENU_SHELL(sub), item);
			gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(insert_function), (gpointer) recent_functions[i]);
		}
	}
}
void recreate_recent_variables() {
	GtkWidget *item, *sub;
	sub = v_menu;
	recent_variable_items.clear();
	bool b = false;
	for(unsigned int i = 0; i < recent_variables.size(); i++) {
		if(!CALCULATOR->hasVariable(recent_variables[i])) {
			recent_variables.erase(recent_variables.begin() + i);
			i--;
		} else {
			if(!b) {
				MENU_SEPARATOR_PREPEND
				b = true;
			}
			item = gtk_menu_item_new_with_label(recent_variables[i]->title(true).c_str()); 
			recent_variable_items.push_back(item);
			gtk_widget_show(item); 
			gtk_menu_shell_prepend(GTK_MENU_SHELL(sub), item);
			gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(insert_variable), (gpointer) recent_variables[i]);
		}
	}
}
void recreate_recent_units() {
	GtkWidget *item, *sub;
	sub = u_menu;
	recent_unit_items.clear();
	bool b = false;
	for(unsigned int i = 0; i < recent_units.size(); i++) {
		if(!CALCULATOR->hasUnit(recent_units[i])) {
			recent_units.erase(recent_units.begin() + i);
			i--;
		} else {
			if(!b) {
				MENU_SEPARATOR_PREPEND
				b = true;
			}
			item = gtk_menu_item_new_with_label(recent_units[i]->title(true).c_str()); 
			recent_unit_items.push_back(item);
			gtk_widget_show(item); 
			gtk_menu_shell_prepend(GTK_MENU_SHELL(sub), item);
			gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(insert_unit), (gpointer) recent_units[i]);
		}
	}
}

void function_inserted(Function *object) {
	if(!object) {
		return;
	}
	GtkWidget *item, *sub;
	sub = f_menu;
	if(recent_function_items.size() <= 0) {
		MENU_SEPARATOR_PREPEND
	}
	for(unsigned int i = 0; i < recent_functions.size(); i++) {
		if(recent_functions[i] == object) {
			recent_functions.erase(recent_functions.begin() + i);
			gtk_widget_destroy(recent_function_items[i]);
			recent_function_items.erase(recent_function_items.begin() + i);
			break;
		}
	}
	if(recent_function_items.size() >= 5) {
		recent_functions.erase(recent_functions.begin());
		gtk_widget_destroy(recent_function_items[0]);
		recent_function_items.erase(recent_function_items.begin());
	}
	item = gtk_menu_item_new_with_label(object->title(true).c_str()); 
	recent_function_items.push_back(item);
	recent_functions.push_back(object);
	gtk_widget_show(item); 
	gtk_menu_shell_prepend(GTK_MENU_SHELL(sub), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(insert_function), (gpointer) object);
}
void variable_inserted(Variable *object) {
	if(!object) {
		return;
	}
	GtkWidget *item, *sub;
	sub = v_menu;
	if(recent_variable_items.size() <= 0) {
		MENU_SEPARATOR_PREPEND
	}
	for(unsigned int i = 0; i < recent_variables.size(); i++) {
		if(recent_variables[i] == object) {
			recent_variables.erase(recent_variables.begin() + i);
			gtk_widget_destroy(recent_variable_items[i]);
			recent_variable_items.erase(recent_variable_items.begin() + i);
			break;
		}
	}
	if(recent_variable_items.size() >= 5) {
		recent_variables.erase(recent_variables.begin());
		gtk_widget_destroy(recent_variable_items[0]);
		recent_variable_items.erase(recent_variable_items.begin());
	}
	item = gtk_menu_item_new_with_label(object->title(true).c_str()); 
	recent_variable_items.push_back(item);
	recent_variables.push_back(object);
	gtk_widget_show(item); 
	gtk_menu_shell_prepend(GTK_MENU_SHELL(sub), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(insert_variable), (gpointer) object);
}
void unit_inserted(Unit *object) {
	if(!object) {
		return;
	}
	GtkWidget *item, *sub;
	sub = u_menu;
	if(recent_unit_items.size() <= 0) {
		MENU_SEPARATOR_PREPEND
	}
	for(unsigned int i = 0; i < recent_units.size(); i++) {
		if(recent_units[i] == object) {
			recent_units.erase(recent_units.begin() + i);
			gtk_widget_destroy(recent_unit_items[i]);
			recent_unit_items.erase(recent_unit_items.begin() + i);
			break;
		}
	}
	if(recent_unit_items.size() >= 5) {
		recent_units.erase(recent_units.begin());
		gtk_widget_destroy(recent_unit_items[0]);
		recent_unit_items.erase(recent_unit_items.begin());
	}
	item = gtk_menu_item_new_with_label(object->title(true).c_str()); 
	recent_unit_items.push_back(item);
	recent_units.push_back(object);
	gtk_widget_show(item); 
	gtk_menu_shell_prepend(GTK_MENU_SHELL(sub), item);
	gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(insert_unit), (gpointer) object);
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
		string str = f->name(printops.use_unicode_signs) + "()";
		gchar *gstr = g_strdup(str.c_str());
		function_inserted(f);
		insert_text(gstr);
		g_free(gstr);
		return;
	}
	string f_title = f->title(true);
	dialog = gtk_dialog_new_with_buttons(f_title.c_str(), GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);

	GtkWidget *b_exec = gtk_button_new();
	GtkWidget *b_hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(b_hbox), gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_BUTTON), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(b_hbox), gtk_label_new(_("Execute")), FALSE, FALSE, 0);
	GtkWidget *b_align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_container_add(GTK_CONTAINER(b_align), b_hbox);
	gtk_container_add(GTK_CONTAINER(b_exec), b_align);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), b_exec, GTK_RESPONSE_APPLY);
	
	GtkWidget *b_insert = gtk_button_new();
	b_hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(b_hbox), gtk_image_new_from_stock(GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(b_hbox), gtk_label_new(_("Insert")), FALSE, FALSE, 0);
	b_align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_container_add(GTK_CONTAINER(b_align), b_hbox);
	gtk_container_add(GTK_CONTAINER(b_insert), b_align);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), b_insert, GTK_RESPONSE_ACCEPT);
	
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
	int args = 0;
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
	vector<GtkWidget*> boolean_buttons;
	int boolean_index[args];
	int bindex = 0;
	GtkWidget *descr, *descr_box, *descr_frame;
	string argstr, typestr; 
	string argtype;
	//create argument entries
	Argument *arg;
	for(int i = 0; i < args; i++) {
		arg = f->getArgumentDefinition(i + 1);
		if(!arg || arg->name().empty()) {
			if(args == 1) {
				argstr = _("Value");
			} else {
				argstr = _("Argument");
				argstr += " ";
				argstr += i2s(i + 1);
			}
		} else {
			argstr = arg->name();
		}
		typestr = "";
		argtype = "";
		label[i] = gtk_label_new(argstr.c_str());
		gtk_misc_set_alignment(GTK_MISC(label[i]), 0, 0.5);
		if(arg) {
			switch(arg->type()) {
				case ARGUMENT_TYPE_INTEGER: {
					IntegerArgument *iarg = (IntegerArgument*) arg;
					gdouble min = -1000000, max = 1000000;
					if(iarg->min()) {
						min = iarg->min()->intValue();
					}
					if(iarg->max()) {
						max = iarg->max()->intValue();
					}				
					entry[i] = gtk_spin_button_new_with_range(min, max, 1);
					gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(entry[i]), TRUE);
					if(!f->getDefaultValue(i + 1).empty()) {
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry[i]), s2i(f->getDefaultValue(i + 1)));
					} else if(!arg->zeroForbidden() && min <= 0 && max >= 0) {
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
					/*entry[i] = gtk_spin_button_new_with_range(0, 1, 1);
					gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(entry[i]), FALSE);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(entry[i]), 0);*/
					boolean_index[i] = bindex;
					bindex += 2;
					entry[i] = gtk_hbox_new(TRUE, 6);
					boolean_buttons.push_back(gtk_radio_button_new_with_label(NULL, "True"));
					gtk_box_pack_start(GTK_BOX(entry[i]), boolean_buttons[boolean_buttons.size() - 1], TRUE, TRUE, 0);
					boolean_buttons.push_back(gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(boolean_buttons[boolean_buttons.size() - 1]), "False"));
					gtk_box_pack_end(GTK_BOX(entry[i]), boolean_buttons[boolean_buttons.size() - 1], TRUE, TRUE, 0);
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(boolean_buttons[boolean_buttons.size() - 1]), TRUE);
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
		} else {
			entry[i] = gtk_entry_new();
		}
		if(typestr.empty() && i >= f->minargs() && !has_vector) {
			typestr = "(";
			typestr += _("optional");
			typestr += ")";			
		}
		if(arg) {
			switch(arg->type()) {		
				case ARGUMENT_TYPE_DATE: {
					typestr = typestr.substr(1, typestr.length() - 2);
					type_label[i] = gtk_button_new_with_label(typestr.c_str());
					g_signal_connect((gpointer) type_label[i], "clicked", G_CALLBACK(on_type_label_date_clicked), (gpointer) entry[i]);
					break;
				}
				case ARGUMENT_TYPE_FILE: {
					typestr = typestr.substr(1, typestr.length() - 2);
					type_label[i] = gtk_button_new_with_label(typestr.c_str());
					g_signal_connect((gpointer) type_label[i], "clicked", G_CALLBACK(on_type_label_file_clicked), (gpointer) entry[i]);
					break;
				}
				default: {
					type_label[i] = gtk_label_new(typestr.c_str());		
					gtk_misc_set_alignment(GTK_MISC(type_label[i]), 1, 0.5);
				}
			}
		} else {
			type_label[i] = gtk_label_new(typestr.c_str());		
			gtk_misc_set_alignment(GTK_MISC(type_label[i]), 1, 0.5);
		}
		argstr = f->getDefaultValue(i + 1);
		if(arg && (arg->suggestsQuotes() || arg->type() == ARGUMENT_TYPE_TEXT) && argstr.length() >= 2 && argstr[0] == '\"' && argstr[argstr.length() - 1] == '\"') {
			argstr = argstr.substr(1, argstr.length() - 2);
		}
		if(arg && arg->type() == ARGUMENT_TYPE_BOOLEAN) {
			if(argstr == "1") {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(boolean_buttons[boolean_buttons.size() - 2]), TRUE);
			}
		} else {
			gtk_entry_set_text(GTK_ENTRY(entry[i]), argstr.c_str());
			//insert selection in expression entry into the first argument entry
			if(i == 0) {
				gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
				gtk_entry_set_text(GTK_ENTRY(entry[i]), gstr);
				g_free(gstr);
			}
		}
		gtk_table_attach(GTK_TABLE(table), label[i], 0, 1, i, i + 1, (GtkAttachOptions) (GTK_FILL), GTK_FILL, 0, 0);
		gtk_table_attach(GTK_TABLE(table), entry[i], 1, 2, i, i + 1, (GtkAttachOptions) (GTK_FILL | GTK_EXPAND), GTK_FILL, 0, 0);
		gtk_table_attach(GTK_TABLE(table), type_label[i], 2, 3, i, i + 1, (GtkAttachOptions) (GTK_FILL), GTK_FILL, 0, 0);
	}
	//display function description
	if(!f->description().empty()) {
		descr_frame = gtk_frame_new(NULL); 
		gtk_container_add(GTK_CONTAINER(vbox_pre), descr_frame);
		descr_box = gtk_vbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(descr_box), 6);
		gtk_container_add(GTK_CONTAINER(descr_frame), descr_box);
		descr = gtk_label_new(f->description().c_str());
		gtk_label_set_line_wrap(GTK_LABEL(descr), TRUE);
		gtk_box_pack_start(GTK_BOX(descr_box), descr, TRUE, TRUE, 0);
	}
	gtk_widget_show_all(dialog);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_ACCEPT || response == GTK_RESPONSE_APPLY) {
		
		string str = f->name(printops.use_unicode_signs) + "(", str2;
		for(int i = 0; i < args; i++) {
			if(f->getArgumentDefinition(i + 1) && f->getArgumentDefinition(i + 1)->type() == ARGUMENT_TYPE_BOOLEAN) {
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(boolean_buttons[boolean_index[i]]))) {
					str2 = "1";
				} else {
					str2 = "0";
				}
			} else {
				str2 = gtk_entry_get_text(GTK_ENTRY(entry[i]));
			}

			//if the minimum number of function arguments have been filled, do not add anymore if entry is empty
			if(i >= f->minargs()) {
				remove_blank_ends(str2);
			}
			if((i < f->minargs() || !str2.empty()) && f->getArgumentDefinition(i + 1) && (f->getArgumentDefinition(i + 1)->suggestsQuotes() || (f->getArgumentDefinition(i + 1)->type() == ARGUMENT_TYPE_TEXT && str2.find(CALCULATOR->getComma()) != string::npos))) {
				if(str2.length() < 1 || (str2[0] != '\"' && str[0] != '\'')) { 
					str2.insert(0, "\"");
					str2 += "\"";
				}
			}
			if(i > 0) {
				str += CALCULATOR->getComma();
				str += " ";
			}
			str += str2;
		}
		str += ")";
		
		//redo selection if "OK" was clicked, clear expression entry "Execute"
		if(response == GTK_RESPONSE_ACCEPT) {
			gtk_editable_select_region(GTK_EDITABLE(expression), start, end);
		} else {
			gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
		}
		
		insert_text(str.c_str());
		//Calculate directly when "Execute" was clicked
		if(response == GTK_RESPONSE_APPLY) {
			execute_expression();
		}
		function_inserted(f);
	}
	gtk_widget_destroy(dialog);
}

/*
	called from function menu
*/
void insert_function(GtkMenuItem *w, gpointer user_data) {
	insert_function((Function*) user_data, glade_xml_get_widget (main_glade, "main_window"));
}

/*
	called from variable menu
	just insert text data stored in menu item
*/
void insert_variable(GtkMenuItem *w, gpointer user_data) {
	Variable *v = (Variable*) user_data;
	if(!CALCULATOR->hasVariable(v)) {
		show_message(_("Variable does not exist anymore."), glade_xml_get_widget (main_glade, "main_window"));
		return;
	}
	insert_text(v->name(printops.use_unicode_signs).c_str());
	variable_inserted((Variable*) user_data);
}
//from prefix menu
void insert_prefix(GtkMenuItem *w, gpointer user_data) {
	insert_text(((Prefix*) user_data)->name(printops.abbreviate_units, printops.use_unicode_signs).c_str());
}
//from unit menu
void insert_unit(GtkMenuItem *w, gpointer user_data) {
	if(printops.abbreviate_units) {
		insert_text(((Unit*) user_data)->shortName(printops.use_unicode_signs).c_str());
	} else {
		insert_text(((Unit*) user_data)->plural(true, printops.use_unicode_signs).c_str());
	}
	unit_inserted((Unit*) user_data);
}

/*
	display edit/new unit dialog
	creates new unit if u == NULL, win is parent window
*/
void
edit_unit(const char *category = "", Unit *u = NULL, GtkWidget *win = NULL)
{
	GtkWidget *dialog = get_unit_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	
	if(u) {
		if(u->isLocal())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unit"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unit (global)"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Unit"));
	}

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category")), category);

	//clear entries
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_plural")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal")), "");	
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), "");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp")), 1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed")), "");

	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_button_ok"), TRUE);

	if(u) {
		//fill in original parameters
		if(u->unitType() == BASE_UNIT) {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), UNIT_CLASS_BASE_UNIT);
		} else if(u->unitType() == ALIAS_UNIT) {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), UNIT_CLASS_ALIAS_UNIT);
		} else if(u->unitType() == COMPOSITE_UNIT) {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), UNIT_CLASS_COMPOSITE_UNIT);
		}
		on_unit_edit_optionmenu_class_changed(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), NULL);	
		gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class"), !u->isBuiltin());

		//gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class"), u->isLocal() && !u->isBuiltin());

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name")), u->name().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal")), u->name().c_str());		
		
		if(u->unitType() != COMPOSITE_UNIT) {
			gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"), !u->isBuiltin());
		}

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_hidden")), u->isHidden());
		
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_plural")), u->plural(false).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular")), u->singular(false).c_str());

		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category")), u->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc")), u->title(false).c_str());

		switch(u->unitType()) {
			case ALIAS_UNIT: {
				AliasUnit *au = (AliasUnit*) u;
				if(au->firstBaseUnit()->unitType() == COMPOSITE_UNIT) {
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), ((CompositeUnit*) (au->firstBaseUnit()))->referenceName().c_str());
				} else {
					if(printops.abbreviate_units)
						gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), au->firstBaseUnit()->shortName().c_str());
					else
						gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), au->firstBaseUnit()->singular().c_str());	
				}		
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp")), au->firstBaseExp());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation")), CALCULATOR->localizeExpression(au->expression()).c_str());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed")), CALCULATOR->localizeExpression(au->reverseExpression()).c_str());
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact")), !au->isApproximate());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"), !u->isBuiltin());
				break;
			}
			case COMPOSITE_UNIT: {
				if(printops.abbreviate_units) {
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), u->shortName().c_str());
				} else {
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), u->singular().c_str());
				}
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal")), ((CompositeUnit*) u)->referenceName().c_str());			
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"), !u->isBuiltin());
			}
		}
	} else {
		//default values
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_hidden")), false);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact")), TRUE);
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), UNIT_CLASS_ALIAS_UNIT);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation")), "1");
		on_unit_edit_optionmenu_class_changed(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), NULL);
	}

run_unit_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		int type = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")));		
		string str;
		if(type == UNIT_CLASS_COMPOSITE_UNIT) str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal")));
		else str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name given
			if(type == UNIT_CLASS_COMPOSITE_UNIT) {
				show_message(_("Empty internal name field."), dialog);
				goto run_unit_edit_dialog;			
			} else {
				str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular")));
				remove_blank_ends(str);
				if(str.empty()) {
					//no singular unit name either -- open dialog again
					show_message(_("Empty name field."), dialog);
					goto run_unit_edit_dialog;
				} else {
					//switch singular name and name
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name")), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular"))));
					gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular")), "");
				}
			}
		}

		//unit with the same name exists -- overwrite or open the dialog again
		if(type == UNIT_CLASS_COMPOSITE_UNIT) {
			if((!u || u->referenceName() != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal")))) && CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal"))), u) && !ask_question(_("A variable or unit with the same internal name already exists.\nDo you want to overwrite it?"), dialog)) {
				goto run_unit_edit_dialog;
			}		
		} else {
			if((!u || u->name() != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name")))) && CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"))), u) && !ask_question(_("A variable or unit with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
				goto run_unit_edit_dialog;
			}
			if((!u || u->plural(false) != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_plural")))) && CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_plural"))), u) && !ask_question(_("A variable or unit with the same plural name already exists.\nDo you want to overwrite it?"), dialog)) {
				goto run_unit_edit_dialog;
			}
			if((!u || u->singular(false) != gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular")))) && CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular"))), u) && !ask_question(_("A variable or unit with the same singular name already exists.\nDo you want to overwrite it?"), dialog)) {
				goto run_unit_edit_dialog;
			}
		}
		if(u) {
			//edited an existing unit -- update unit
			u->setLocal(true);
			gint i1 = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")));
			switch(u->unitType()) {
				case ALIAS_UNIT: {
					if(i1 != UNIT_CLASS_ALIAS_UNIT) {
						u->destroy();
						u = NULL;
						break;
					}
					if(!u->isBuiltin()) {
						AliasUnit *au = (AliasUnit*) u;
						Unit *bu = CALCULATOR->getUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
						if(!bu) bu = CALCULATOR->getCompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
						if(!bu) {
							show_message(_("Base unit does not exist."), dialog);
							goto run_unit_edit_dialog;
						}
						au->setBaseUnit(bu);
						au->setExpression(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation")))));
						au->setReverseExpression(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed")))));
						au->setExponent(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp"))));
						au->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact"))));
					}
					break;
				}
				case COMPOSITE_UNIT: {
					if(i1 != UNIT_CLASS_COMPOSITE_UNIT) {
						u->destroy();
						u = NULL;
						break;
					}
					if(!u->isBuiltin()) {
						u->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal"))));
						((CompositeUnit*) u)->setBaseExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
					}
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
				if(!u->isBuiltin()) {
					u->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"))));
				}
				u->setPlural(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_plural"))));
				u->setSingular(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular"))));
			}
			u->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))));
			u->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))));			
		}
		if(!u) {
			//new unit
			switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")))) {
				case UNIT_CLASS_ALIAS_UNIT: {
					Unit *bu = CALCULATOR->getUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
					if(!bu) bu = CALCULATOR->getCompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
					if(!bu) {
						show_message(_("Base unit does not exist."), dialog);
						goto run_unit_edit_dialog;
					}
					u = new AliasUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_plural"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))), bu, CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation")))), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp"))), CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed")))), true);
					((AliasUnit*) u)->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact"))));
					break;
				}
				case UNIT_CLASS_COMPOSITE_UNIT: {
					CompositeUnit *cu = new CompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_internal"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))), true);
					u = cu;
					break;
				}
				default: {
					u = new Unit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_plural"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_singular"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))), true);
					break;
				}
			}
			if(u) {
				CALCULATOR->addUnit(u);
			}
		}
		if(u) {
			u->setHidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_hidden"))));
			//select the new unit
			selected_unit = u;
			if(!u->isActive()) {
				selected_unit_category = _("Inactive");
			} else if(u->category().empty()) {
				selected_unit_category = _("Uncategorized");
			} else {
				selected_unit_category = "/";
				selected_unit_category += u->category();
			}
		}
		update_umenus();
		unit_inserted(u);
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

void edit_argument(Argument *arg) {
	if(!arg) {
		arg = new Argument();
	}
	edited_argument = arg;
	GtkWidget *dialog = get_argument_rules_dialog();	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (functionedit_glade, "function_edit_dialog")));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_test")), arg->tests());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_allow_matrix")), arg->matrixAllowed());
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_allow_matrix"), TRUE);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_entry_condition")), CALCULATOR->localizeExpression(arg->getCustomCondition()).c_str());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_condition")), !arg->getCustomCondition().empty());
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_entry_condition"), !arg->getCustomCondition().empty());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_forbid_zero")), arg->zeroForbidden());
	switch(arg->type()) {
		case ARGUMENT_TYPE_NUMBER: {
			NumberArgument *farg = (NumberArgument*) arg;
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_box_min"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_min")), farg->min() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_min_include_equals")), farg->includeEqualsMin());
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min"), farg->min() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_min_include_equals"), TRUE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")), FALSE);
			if(farg->min()) {
				PrintOptions po;
				po.number_fraction_format = FRACTION_DECIMAL_EXACT;
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")), farg->min()->print(po).c_str());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")), 0);
			}
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_box_max"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_max")), farg->max() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_max_include_equals")), farg->includeEqualsMax());
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max"), farg->max() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_max_include_equals"), TRUE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")), FALSE);
			if(farg->max()) {
				PrintOptions po;
				po.number_fraction_format = FRACTION_DECIMAL_EXACT;
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")), farg->max()->print(po).c_str());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")), 0);
			}			
			break;
		}
		case ARGUMENT_TYPE_INTEGER: {
			IntegerArgument *iarg = (IntegerArgument*) arg;
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_box_min"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_min")), iarg->min() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_min_include_equals")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min"), iarg->min() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_min_include_equals"), FALSE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")), TRUE);
			if(iarg->min()) {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")), iarg->min()->intValue());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")), 0);
			}
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_box_max"), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_max")), iarg->max() != NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_max_include_equals")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max"), iarg->max() != NULL);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_max_include_equals"), FALSE);
			gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")), TRUE);
			if(iarg->max()) {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")), iarg->max()->intValue());		
			} else {
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")), 0);
			}	
			break;
		}
		case ARGUMENT_TYPE_FREE: {}
		case ARGUMENT_TYPE_MATRIX: {		
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_allow_matrix")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_allow_matrix"), FALSE);	
		}
		default: {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_min")), FALSE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_max")), FALSE);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")), 0);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")), 0);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_box_min"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_box_max"), FALSE);
		}
	}
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		arg->setTests(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_test"))));
		arg->setMatrixAllowed(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_allow_matrix"))));
		arg->setZeroForbidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_forbid_zero"))));
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_condition")))) {
			arg->setCustomCondition(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_entry_condition"))));
		} else {
			arg->setCustomCondition("");
		}
		if(arg->type() == ARGUMENT_TYPE_NUMBER) {
			NumberArgument *farg = (NumberArgument*) arg;
			farg->setIncludeEqualsMin(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_min_include_equals"))));
			farg->setIncludeEqualsMax(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_max_include_equals"))));
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_min")))) {
				MathStructure mstruct2 = CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min")))));
				farg->setMin(&mstruct2.number());
			} else {
				farg->setMin(NULL);
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_max")))) {
				MathStructure mstruct2 = CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max")))));
				farg->setMax(&mstruct2.number());
			} else {
				farg->setMax(NULL);
			}			
		} else if(arg->type() == ARGUMENT_TYPE_INTEGER) {
			IntegerArgument *iarg = (IntegerArgument*) arg;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_min")))) {
				Number integ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min"))), 1);
				iarg->setMin(&integ);
			} else {
				iarg->setMin(NULL);
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_max")))) {
				Number integ(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max"))), 1);
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
	gtk_widget_grab_focus(expression);
}

/*
	display edit/new function dialog
	creates new function if f == NULL, win is parent window
*/
void edit_function(const char *category = "", Function *f = NULL, GtkWidget *win = NULL) {

	GtkWidget *dialog = get_function_edit_dialog();	
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

	GtkTextBuffer *description_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (functionedit_glade, "function_edit_textview_description")));
	GtkTextBuffer *expression_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (functionedit_glade, "function_edit_textview_expression")));	

	//clear entries
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_condition")), "");
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name"), !f || !f->isBuiltin());
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_textview_expression"), !f || !f->isBuiltin());
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_category")), category);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_desc")), "");
	gtk_text_buffer_set_text(description_buffer, "", -1);
	gtk_text_buffer_set_text(expression_buffer, "", -1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_hidden")), false);

	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_ok"), TRUE);	
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_optionmenu_argument_type"), !f || !f->isBuiltin());
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_argument"), !f || !f->isBuiltin());

	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_subfunctions"), !f || !f->isBuiltin());
	gtk_list_store_clear(tSubfunctions_store);
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_subfunction"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_remove_subfunction"), FALSE);		
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_subfunction"), !f || !f->isBuiltin());
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression")), "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_precalculate")), TRUE);
	selected_subfunction = 0;
	last_subfunction_index = 0;
	if(f) {
		//fill in original paramaters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name")), f->name().c_str());
		if(!f->isBuiltin()) {
			gtk_text_buffer_set_text(expression_buffer, CALCULATOR->localizeExpression(((UserFunction*) f)->equation()).c_str(), -1);
		}
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_category")), f->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_desc")), f->title(false).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_condition")), CALCULATOR->localizeExpression(f->condition()).c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_hidden")), f->isHidden());
		gtk_text_buffer_set_text(description_buffer, f->description().c_str(), -1);
		
		if(!f->isBuiltin()) {
			GtkTreeIter iter;
			string str, str2;
			for(unsigned int i = 1; i <= ((UserFunction*) f)->countSubfunctions(); i++) {
				gtk_list_store_append(tSubfunctions_store, &iter);
				if(((UserFunction*) f)->subfunctionPrecalculated(i)) {
					str = _("yes");
				} else {
					str = _("no");
				}
				str2 = "\\";
				str2 += i2s(i);
				gtk_list_store_set(tSubfunctions_store, &iter, 0, str2.c_str(), 1, ((UserFunction*) f)->getSubfunction(i).c_str(), 2, str.c_str(), 3, i, 4, ((UserFunction*) f)->subfunctionPrecalculated(i), -1);
				last_subfunction_index = i;
			}
		}
	}
	update_function_arguments_list(f);
	
run_function_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name -- open dialog again
			show_message(_("Empty name field."), dialog);
			goto run_function_edit_dialog;
		}
		GtkTextIter e_iter_s, e_iter_e;
		gtk_text_buffer_get_start_iter(expression_buffer, &e_iter_s);
		gtk_text_buffer_get_end_iter(expression_buffer, &e_iter_e);		
		string str2 = CALCULATOR->unlocalizeExpression(gtk_text_buffer_get_text(expression_buffer, &e_iter_s, &e_iter_e, FALSE));
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
		if((!f || str != f->name()) && CALCULATOR->functionNameTaken(str, f) && !ask_question(_("A function with the same name already exists.\nDo you want to overwrite the function?"), dialog)) {
			goto run_function_edit_dialog;
		}	
		if(f) {
			f->setLocal(true);
			//edited an existing function
			if(!f->isBuiltin()) {
				f->setName(str);
			}
			f->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_category"))));
			f->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_desc"))));
			f->setDescription(gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE));
			if(!f->isBuiltin()) {
				((UserFunction*) f)->setEquation(str2);
				f->clearArgumentDefinitions();
			}	
		} else {
			//new function
			f = new UserFunction(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_category"))), str, str2, true, -1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_desc"))), gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE));
			CALCULATOR->addFunction(f);
		}
		if(f) {
			f->setCondition(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_condition")))));
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
			b = !f->isBuiltin() && gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tSubfunctions_store), &iter);
			if(!f->isBuiltin()) ((UserFunction*) f)->clearSubfunctions();
			while(b) {
				gchar *gstr;
				gboolean g_b = FALSE;
				gtk_tree_model_get(GTK_TREE_MODEL(tSubfunctions_store), &iter, 1, &gstr, 4, &g_b, -1);
				((UserFunction*) f)->addSubfunction(gstr, g_b);
				b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tSubfunctions_store), &iter);
				g_free(gstr);
			}		
			f->setHidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_hidden"))));
			if(!f->isActive()) {
				selected_function_category = _("Inactive");
			} else if(f->category().empty()) {
				selected_function_category = _("Uncategorized");
			} else {
				selected_function_category = "/";
				selected_function_category += f->category();
			}
			//select the new function
			selected_function = f;
		}
		update_fmenu();	
		function_inserted(f);
	}
	edited_function = NULL;
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	"New function" menu item selected
*/
void new_function(GtkMenuItem *w, gpointer user_data)
{
	edit_function(
			"",
			NULL,
			glade_xml_get_widget (main_glade, "main_window"));
}
/*
	"New unit" menu item selected
*/
void new_unit(GtkMenuItem *w, gpointer user_data)
{
	edit_unit(
			"",
			NULL,
			glade_xml_get_widget (main_glade, "main_window"));
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
					glade_xml_get_widget (main_glade, "main_window")
				),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Unit does not exist"));
		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
	//result is stored in MathStructure *mstruct
	*mstruct = CALCULATOR->convert(*mstruct, u, evalops);
	setResult();
	gtk_widget_grab_focus(expression);
}

void edit_unknown(const char *category, Variable *var, GtkWidget *win) {

	if(var != NULL && var->isKnown()) {
		edit_variable(category, var, NULL, win);
		return;
	}
	
	UnknownVariable *v = (UnknownVariable*) var;
	
	GtkWidget *dialog = get_unknown_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	
	if(v) {
		if(v->isLocal())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unknown Variable"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Unknown Variable (global)"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Unknown Variable"));
	}
	
	if(v) {
		//fill in original parameters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name")), v->name().c_str());
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"), !v->isBuiltin());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_desc")), v->title(false).c_str());
		if(v->assumptions()) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_type"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_sign"), TRUE);
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type")), v->assumptions()->numberType());
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign")), v->assumptions()->sign());
		} else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_type"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_sign"), FALSE);
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type")), CALCULATOR->defaultAssumptions()->numberType());
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign")), CALCULATOR->defaultAssumptions()->sign());
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"), TRUE);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_type"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_sign"), TRUE);

		//fill in default values
		string v_name = CALCULATOR->getName();
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name")), v_name.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_desc")), "");
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type")), CALCULATOR->defaultAssumptions()->numberType());
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign")), CALCULATOR->defaultAssumptions()->sign());

	}

run_unknown_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name -- open dialog again
			show_message(_("Empty name field."), dialog);
			goto run_unknown_edit_dialog;
		}

		//unknown with the same name exists -- overwrite or open dialog again
		if((!v || str != v->name()) && CALCULATOR->variableNameTaken(str, v) && !ask_question(_("An unit or variable with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
			goto run_unknown_edit_dialog;
		}
		if(!v) {
			//no need to create a new unknown when a unknown with the same name exists
			var = CALCULATOR->getActiveVariable(str);
			if(var && var->isLocal() && !var->isKnown()) v = (UnknownVariable*) var;
		}
		if(v) {
			//update existing unknown
			v->setLocal(true);
			if(!v->isBuiltin()) {
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")))) {
					if(!v->assumptions()) v->setAssumptions(new Assumptions());
					v->assumptions()->setNumberType((AssumptionNumberType) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"))));
					v->assumptions()->setSign((AssumptionSign) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"))));
					v->setName(str);
				} else {
					v->setAssumptions(NULL);
				}
			}
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_desc"))));
		} else {
			//new unknown
			v = (UnknownVariable*) CALCULATOR->addVariable(new UnknownVariable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_category"))), str, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_desc"))), true));
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")))) {
				if(!v->assumptions()) v->setAssumptions(new Assumptions());
				v->assumptions()->setNumberType((AssumptionNumberType) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"))));
				v->assumptions()->setSign((AssumptionSign) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"))));
			} else {
				v->setAssumptions(NULL);
			}
		}
		if(v) {
			//select the new unknown
			selected_variable = v;
			if(!v->isActive()) {
				selected_variable_category = _("Inactive");
			} else if(v->category().empty()) {
				selected_variable_category = _("Uncategorized");
			} else {
				selected_variable_category = "/";
				selected_variable_category += v->category();
			}
		}
		update_vmenu();
		variable_inserted(v);
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	display edit/new variable dialog
	creates new variable if v == NULL, mstruct_ is forced value, win is parent window
*/
void edit_variable(const char *category, Variable *var, MathStructure *mstruct_, GtkWidget *win) {

	if(var != NULL && !var->isKnown()) {
		edit_unknown(category, var, win);
		return;
	}
	KnownVariable *v = (KnownVariable*) var;

	if((v != NULL && v->get().isVector() && (!mstruct_ || mstruct_->isVector())) || (mstruct_ && !v && mstruct_->isVector())) {
		edit_matrix(category, v, mstruct_, win);
		return;
	}

	GtkWidget *dialog = get_variable_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	if(v) {
		if(v->isLocal())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Variable"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Variable (global)"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Variable"));
	}

	gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_button_ok"), TRUE);		

	if(v) {
		//fill in original parameters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name")), v->name().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value")), get_value_string(v->get(), false, NULL).c_str());
		bool b_approx = *printops.is_approximate || v->isApproximate();
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact")), !b_approx);
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"), !v->isBuiltin());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_desc")), v->title(false).c_str());
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"), TRUE);

		//fill in default values
		string v_name = CALCULATOR->getName();
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name")), v_name.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value")), get_value_string(*mstruct).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_desc")), "");		
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact")), TRUE);				

	}
	if(mstruct_) {
		//forced value
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"), FALSE);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value")), get_value_string(*mstruct_).c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact")), !mstruct_->isApproximate());
	}

run_variable_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name")));
		string str2 = CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value"))));
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
		if((!v || str != v->name()) && CALCULATOR->variableNameTaken(str, v) && !ask_question(_("An unit or variable with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
			goto run_variable_edit_dialog;
		}
		if(!v) {
			//no need to create a new variable when a variable with the same name exists
			var = CALCULATOR->getActiveVariable(str);
			if(var && var->isLocal() && var->isKnown()) v = (KnownVariable*) var;
		}
		if(v) {
			//update existing variable
			v->setLocal(true);
			if(!v->isBuiltin()) {
				if(mstruct_) {
					v->set(*mstruct_);
				} else {
					v->set(str2);
				}
				v->setName(str);
				v->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"))));
			}
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_desc"))));
		} else {
			//new variable
			if(mstruct_) {
				//forced value
				v = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_category"))), str, *mstruct_, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_desc"))), true));
			} else {
				v = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_category"))), str, str2, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_desc"))), true));
				v->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"))));
			}
		}
		if(v) {
			//select the new variable
			selected_variable = v;
			if(!v->isActive()) {
				selected_variable_category = _("Inactive");
			} else if(v->category().empty()) {
				selected_variable_category = _("Uncategorized");
			} else {
				selected_variable_category = "/";
				selected_variable_category += v->category();
			}
		}
		update_vmenu();
		variable_inserted(v);
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

/*
	display edit/new matrix dialog
	creates new matrix if v == NULL, mstruct_ is forced value, win is parent window
*/
void edit_matrix(const char *category, Variable *var, MathStructure *mstruct_, GtkWidget *win, gboolean create_vector) {

	if(var != NULL && !var->isKnown()) {
		edit_unknown(category, var, win);
		return;
	}
	KnownVariable *v = (KnownVariable*) var;

	if((v && !v->get().isVector()) || (mstruct_ && !mstruct_->isVector())) {
		edit_variable(category, v, mstruct_, win);
		return;
	}

	GtkWidget *dialog = get_matrix_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	if(mstruct_) {
		create_vector = !mstruct_->isMatrix();
	} else if(v) {
		create_vector = !v->get().isMatrix();	
	}
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector")), create_vector);

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
	gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_button_ok"), TRUE);		

	int r = 4, c = 4;
	const MathStructure *old_vctr = NULL;
	if(v) {
		if(create_vector) {
			old_vctr = &v->get();
		} else {
			c = v->get().columns();
			r = v->get().rows();
		}	
		//fill in original parameters
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name")), v->name().c_str());
		//can only change name and value of user variable
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows"), !v->isBuiltin());		
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns"), !v->isBuiltin());				
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_table_elements"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix"), !v->isBuiltin());						
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector"), !v->isBuiltin());								
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc")), v->title(false).c_str());	
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows"), TRUE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns"), TRUE);				
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_table_elements"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix"), TRUE);						
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector"), TRUE);								
	
		//fill in default values
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc")), "");		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows")), 3);		
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns")), 3);						
	}
	if(mstruct_) {
		//forced value
		if(create_vector) {
			old_vctr = mstruct_;
		} else {
			c = mstruct_->columns();
			r = mstruct_->rows();
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns"), FALSE);				
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_table_elements"), FALSE);						
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector"), FALSE);		
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

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows")), r);		
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns")), c);					
	on_matrix_edit_spinbutton_columns_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns")), NULL);
	on_matrix_edit_spinbutton_rows_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows")), NULL);		

	PrintOptions po;
	po.number_fraction_format = FRACTION_DECIMAL_EXACT;
	while(gtk_events_pending()) gtk_main_iteration();
	for(unsigned int index_r = 0; index_r < element_entries.size(); index_r++) {
		for(unsigned int index_c = 0; index_c < element_entries[index_r].size(); index_c++) {
			if(create_vector) {
				if(old_vctr && index_r * element_entries[index_r].size() + index_c < old_vctr->components()) {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), old_vctr->getComponent(index_r * element_entries[index_r].size() + index_c + 1)->print(po).c_str());
				} else {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), "");
				}
			} else {
				if(v) {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), v->get().getElement(index_r + 1, index_c + 1)->print(po).c_str());
				} else if(mstruct_) {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), mstruct_->getElement(index_r + 1, index_c + 1)->print(po).c_str());			
				} else {
					gtk_entry_set_text(GTK_ENTRY(element_entries[index_r][index_c]), "0");
				}
			}
		}
	}		
run_matrix_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no name -- open dialog again
			show_message(_("Empty name field."), dialog);
			goto run_matrix_edit_dialog;
		}

		//variable with the same name exists -- overwrite or open dialog again
		if((!v || str != v->name()) && CALCULATOR->variableNameTaken(str) && !ask_question(_("An unit or variable with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
			goto run_matrix_edit_dialog;
		}
		if(!v) {
			//no need to create a new variable when a variable with the same name exists
			var = CALCULATOR->getActiveVariable(str);
			if(var && var->isLocal() && var->isKnown()) v = (KnownVariable*) var;
		}
		MathStructure mstruct_new;
		if(!mstruct_) {
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector")))) {
				mstruct_new.clearVector();
				string str;
				for(unsigned int index_r = 0; index_r < element_entries.size(); index_r++) {
					for(unsigned int index_c = 0; index_c < element_entries[index_r].size(); index_c++) {
						str = gtk_entry_get_text(GTK_ENTRY(element_entries[index_r][index_c]));
						remove_blank_ends(str);
						if(!str.empty()) {
							mstruct_new.addComponent(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str)));
						}
					}
				}
			} else {
				mstruct_new.clearMatrix();
				mstruct_new.resizeMatrix(element_entries.size(), element_entries[0].size(), m_undefined);
				for(unsigned int index_r = 0; index_r < element_entries.size(); index_r++) {
					for(unsigned int index_c = 0; index_c < element_entries[index_r].size(); index_c++) {
						mstruct_new.setElement(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(element_entries[index_r][index_c])))), index_r + 1, index_c + 1);
					}
				}
			}					
		}		
		if(v) {
			v->setLocal(true);
			//update existing variable
			if(!v->isBuiltin()) {
				if(mstruct_) {
					v->set(*mstruct_);
				} else {
					v->set(mstruct_new);
				}
				v->setName(str);
			}
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc"))));
		} else {
			//new variable
			if(mstruct_) {
				v = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category"))), str, *mstruct_, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc"))), true));
			} else {
				v = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category"))), str, mstruct_new, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc"))), true));
			}
		}
		if(v) {
			//select the new variable
			selected_variable = v;
			if(!v->isActive()) {
				selected_variable_category = _("Inactive");
			} else if(v->category().empty()) {
				selected_variable_category = _("Uncategorized");
			} else {
				selected_variable_category = "/";
				selected_variable_category += v->category();
			}
		}
		update_vmenu();
		variable_inserted(v);
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}

void import_csv_file(GtkWidget *win) {

	GtkWidget *dialog = get_csv_import_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file")), "");	
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_desc")), "");

run_csv_import_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no filename -- open dialog again
			show_message(_("No file name entered."), dialog);
			goto run_csv_import_dialog;
		}
		string delimiter = "";
		switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (csvimport_glade, "csv_import_optionmenu_delimiter")))) {
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
				delimiter = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_delimiter_other")));
				break;
			}			
		}
		if(delimiter.empty()) {
			//no filename -- open dialog again
			show_message(_("No delimiter selected."), dialog);
			goto run_csv_import_dialog;
		}		
		if(!CALCULATOR->importCSV(str.c_str(), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (csvimport_glade, "csv_import_spinbutton_first_row"))), gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (csvimport_glade, "csv_import_checkbutton_headers"))), delimiter, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (csvimport_glade, "csv_import_radiobutton_matrix"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_name"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_desc"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_category"))))) {
			GtkWidget *edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (main_glade, "main_window")
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

void export_csv_file(KnownVariable *v, GtkWidget *win) {

	GtkWidget *dialog = get_csv_export_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	if(v) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")), v->name().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix")), v->name().c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_matrix")), TRUE);
	} else {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix")), "");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_current")), TRUE);
	}
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_matrix"), !v);
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_current"), !v);
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix"), FALSE);

run_csv_export_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no filename -- open dialog again
			show_message(_("No file name entered."), dialog);
			goto run_csv_export_dialog;
		}
		string delimiter = "";
		switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (csvexport_glade, "csv_export_optionmenu_delimiter")))) {
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
				delimiter = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_delimiter_other")));
				break;
			}			
		}
		if(delimiter.empty()) {
			//no delimiter -- open dialog again
			show_message(_("No delimiter selected."), dialog);
			goto run_csv_export_dialog;
		}
		MathStructure *matrix_struct;
		if(v) {
			matrix_struct = (MathStructure*) &v->get();
		} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_current")))) {
			matrix_struct = mstruct;
		} else {
			string str2 = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix")));
			remove_blank_ends(str2);
			if(str2.empty()) {
				show_message(_("No variable name entered."), dialog);
				goto run_csv_export_dialog;
			}
			Variable *var = CALCULATOR->getActiveVariable(str2);
			if(!var || !var->isKnown()) {
				var = CALCULATOR->getVariable(str2);
				while(var && !var->isKnown()) {
					var = CALCULATOR->getVariable(str2);
				}
			}
			if(!var || !var->isKnown()) {
				show_message(_("No known variable with entered name found."), dialog);
				goto run_csv_export_dialog;
			}
			matrix_struct = (MathStructure*) &((KnownVariable*) var)->get();
		}		
		if(!CALCULATOR->exportCSV(*matrix_struct, str.c_str(), delimiter)) {
			GtkWidget *edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (main_glade, "main_window")
				),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Could not export to file \n%s"),
				str.c_str()
			);
			gtk_dialog_run(GTK_DIALOG(edialog));
			gtk_widget_destroy(edialog);
		}
	}
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
	
}


/*
	add a new variable (from menu) with the value of result
*/
void add_as_variable()
{
	edit_variable(_("Temporary"), NULL, mstruct, glade_xml_get_widget (main_glade, "main_window"));
}

void new_unknown(GtkMenuItem *w, gpointer user_data)
{
	edit_unknown(_("My Variables"), NULL, glade_xml_get_widget (main_glade, "main_window"));
}

/*
	add a new variable (from menu)
*/
void new_variable(GtkMenuItem *w, gpointer user_data)
{
	edit_variable(_("My Variables"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"));
}

/*
	add a new matrix (from menu)
*/
void new_matrix(GtkMenuItem *w, gpointer user_data)
{
	edit_matrix(_("Matrices"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), FALSE);
}
/*
	add a new vector (from menu)
*/
void new_vector(GtkMenuItem *w, gpointer user_data)
{
	edit_matrix(_("Vectors"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), TRUE);
}

/*
	insert one-argument function when button clicked
*/
void insertButtonFunction(gchar *text, bool append_space = true) {
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {
		//set selection as argument
		gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
		gchar *gstr2 = g_strdup_printf("%s(%s)", text, gstr);
		insert_text(gstr2);
		g_free(gstr);
		g_free(gstr2);
	} else {
		gchar *gstr2;
		//one-argument functions do not need parenthesis
		if(append_space) {
			gstr2 = g_strdup_printf("%s ", text);
		} else {
			gstr2 = g_strdup_printf("%s", text);
		}
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
			mi = glade_xml_get_widget (main_glade, "menu_item_radians");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_radians_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_radians_activate, NULL);						
			break;
		}
		case GRADIANS: {
			mi = glade_xml_get_widget (main_glade, "menu_item_gradians");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_gradians_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_gradians_activate, NULL);						
			break;
		}
		case DEGREES: {
			mi = glade_xml_get_widget (main_glade, "menu_item_degrees");
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
			tb = glade_xml_get_widget (main_glade, "radiobutton_radians");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_radians_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_radians_toggled, NULL);							
			break;
		}
	case GRADIANS: {
			tb = glade_xml_get_widget (main_glade, "radiobutton_gradians");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_gradians_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_gradians_toggled, NULL);							
			break;
		}
	case DEGREES: {
			tb = glade_xml_get_widget (main_glade, "radiobutton_degrees");
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
	evalops.parse_options.functions_enabled = !b;
	evalops.parse_options.variables_enabled = !b;
	evalops.parse_options.units_enabled = !b;
}

/*
	Open variable manager
*/
void manage_variables() {
	GtkWidget *dialog = get_variables_dialog();
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
}

/*
	Open function manager
*/
void manage_functions() {
	GtkWidget *dialog = get_functions_dialog();
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));	
}

/*
	Open unit manager
*/
void manage_units() {
	GtkWidget *dialog = get_units_dialog();
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
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
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "togglebutton_expression")), FALSE);
}

/*
	update menu button when menu is deactivated
*/
void
on_menu_r_deactivate                   (GtkMenuShell       *menushell,
                                        gpointer         user_data) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "togglebutton_result")), FALSE);
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
		const gchar *fromValue = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_from_val")));
		const gchar *toValue = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_to_val")));
		//determine conversion direction
		if(toFrom > 0) {
			if(uFrom == uTo) {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_from_val")), toValue);
			} else {
				EvaluationOptions eo;
				eo.approximation = APPROXIMATION_APPROXIMATE;
				PrintOptions po;
				po.number_fraction_format = FRACTION_DECIMAL;
				MathStructure v_mstruct = CALCULATOR->convert(toValue, uTo, uFrom, eo);
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_from_val")), v_mstruct.print(po).c_str());
			}
		} else {
			if(uFrom == uTo) {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_to_val")), fromValue);
			} else {
				EvaluationOptions eo;
				eo.approximation = APPROXIMATION_APPROXIMATE;
				PrintOptions po;
				po.number_fraction_format = FRACTION_DECIMAL;
				MathStructure v_mstruct = CALCULATOR->convert(fromValue, uFrom, uTo, eo);
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_to_val")), v_mstruct.print(po).c_str());
			}
		}
	}
}

/*
	save definitions to ~/.qalculate/qalculate.cfg
	the hard work is done in the Calculator class
*/
void save_defs() {
	if(!CALCULATOR->saveDefinitions()) {
		GtkWidget *edialog = gtk_message_dialog_new(
				GTK_WINDOW(
					glade_xml_get_widget (main_glade, "main_window")
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
	saved_precision = CALCULATOR->getPrecision();
	saved_angle_unit = CALCULATOR->angleMode();
	saved_hyp_is_on = hyp_is_on;
	saved_printops = printops;
	saved_evalops = evalops;
	saved_assumption_type = CALCULATOR->defaultAssumptions()->numberType();
	saved_assumption_sign = CALCULATOR->defaultAssumptions()->sign();
}

/*
	load preferences from ~/.qalculate/qalculate-gtk.cfg
*/
void load_preferences() {

	multi_sign = SIGN_MULTIDOT;

	default_plot_legend_placement = PLOT_LEGEND_TOP_RIGHT;
	default_plot_display_grid = true;
	default_plot_full_border = false;
	default_plot_min = "0";
	default_plot_max = "10";
	default_plot_sampling_rate = 100;
	default_plot_rows = false;
	default_plot_type = 0;
	default_plot_style = PLOT_STYLE_LINES;
	default_plot_smoothing = PLOT_SMOOTHING_NONE;
	default_plot_variable = "x";
	default_plot_color = true;

	printops.is_approximate = new bool(false);
	printops.prefix = NULL;
	printops.use_min_decimals = false;
	printops.use_denominator_prefix = true;
	printops.min_decimals = 0;
	printops.use_max_decimals = false;
	printops.max_decimals = 2;
	printops.base = 10;
	printops.min_exp = EXP_PRECISION;
	printops.negative_exponents = false;
	printops.sort_options.minus_last = true;
	printops.indicate_infinite_series = false;
	printops.round_halfway_to_even = false;
	printops.number_fraction_format = FRACTION_DECIMAL;
	printops.abbreviate_units = true;
	printops.use_unicode_signs = true;
	printops.use_unit_prefixes = true;
	printops.spacious = true;
	printops.short_multiplication = true;
	printops.place_units_separately = true;
	printops.use_all_prefixes = false;
	printops.excessive_parenthesis = false;
	printops.allow_non_usable = false;
	printops.lower_case_numbers = false;
	
	evalops.approximation = APPROXIMATION_TRY_EXACT;
	evalops.sync_units = true;
	evalops.structuring = STRUCTURING_SIMPLIFY;
	evalops.parse_options.unknowns_enabled = false;
	evalops.parse_options.base = BASE_DECIMAL;
	
	save_mode_on_exit = true;
	save_defs_on_exit = true;
	hyp_is_on = false;
	use_custom_result_font = false;
	use_custom_expression_font = false;
	custom_result_font = "";
	custom_expression_font = "";
	show_buttons = true;
	load_global_defs = true;
	fetch_exchange_rates_at_startup = false;
	first_time = false;
	FILE *file = NULL;
	gchar *gstr2 = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate-gtk.cfg", NULL);
	file = fopen(gstr2, "r");
	expression_history.clear();
	expression_history_index = -1;
	history_width = 325;
	history_height = 250;
	g_free(gstr2);
	if(file) {
		char line[10000];
		string stmp, svalue, svar;
		unsigned int i;
		int v;
		while(true) {
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
				/*else if(svar == "load_global_definitions")
					load_global_defs = v;*/
				else if(svar == "fetch_exchange_rates_at_startup")
					fetch_exchange_rates_at_startup = v;
				else if(svar == "show_buttons")
					show_buttons = v;
				else if(svar == "min_deci")
					printops.min_decimals = v;
				else if(svar == "use_min_deci")
					printops.use_min_decimals = v;
				else if(svar == "max_deci")
					printops.max_decimals = v;
				else if(svar == "use_max_deci")
					printops.use_max_decimals = v;					
				else if(svar == "precision")
					CALCULATOR->setPrecision(v);
				else if(svar == "min_exp")
					printops.min_exp = v;
				else if(svar == "negative_exponents")
					printops.negative_exponents = v;
				else if(svar == "sort_minus_last")
					printops.sort_options.minus_last = v;		
				else if(svar == "spacious")
					printops.spacious = v;	
				else if(svar == "excessive_parenthesis")
					printops.excessive_parenthesis = v;
				else if(svar == "short_multiplication")
					printops.short_multiplication = v;
				else if(svar == "place_units_separately")
					printops.place_units_separately = v;
				else if(svar == "display_mode") {	//obsolete
					switch(v) {
						case 1: {
							printops.min_exp = EXP_PRECISION;
							printops.negative_exponents = false;
							printops.sort_options.minus_last = true;
							break;
						}
						case 2: {
							printops.min_exp = EXP_SCIENTIFIC;
							printops.negative_exponents = true;
							printops.sort_options.minus_last = false;
							break;
						}
						case 3: {
							printops.min_exp = EXP_PURE;
							printops.negative_exponents = true;
							printops.sort_options.minus_last = false;
							break;
						}
						case 4: {
							printops.min_exp = EXP_NONE;
							printops.negative_exponents = false;
							printops.sort_options.minus_last = true;
							break;
						}
					}
				} else if(svar == "use_prefixes")
					printops.use_unit_prefixes = v;
				else if(svar == "fractional_mode") {	//obsolete
					switch(v) {
						case 1: {
							printops.number_fraction_format = FRACTION_DECIMAL;
							break;
						}
						case 2: {
							printops.number_fraction_format = FRACTION_COMBINED;
							break;
						}
						case 3: {
							printops.number_fraction_format = FRACTION_FRACTIONAL;
							break;
						}
					}
				} else if(svar == "number_fraction_format")
					printops.number_fraction_format = (NumberFractionFormat) v;					
				else if(svar == "number_base")
					printops.base = v;
				else if(svar == "number_base_expression")
					evalops.parse_options.base = v;	
				else if(svar == "angle_unit")
					CALCULATOR->angleMode(v);
				else if(svar == "hyp_is_on")
					hyp_is_on = v;
				else if(svar == "functions_enabled")
					evalops.parse_options.functions_enabled = v;
				else if(svar == "variables_enabled")
					evalops.parse_options.variables_enabled = v;
				else if(svar == "donot_calculate_variables")
					evalops.calculate_variables = !v;
				else if(svar == "unknownvariables_enabled")
					evalops.parse_options.unknowns_enabled = v;
				else if(svar == "units_enabled")
					evalops.parse_options.units_enabled = v;
				else if(svar == "use_short_units")
					printops.abbreviate_units = v;
				else if(svar == "all_prefixes_enabled")
					printops.use_all_prefixes = v;
				else if(svar == "denominator_prefix_enabled")
					printops.use_denominator_prefix = v;
				else if(svar == "use_unicode_signs")
					printops.use_unicode_signs = v;	
				else if(svar == "lower_case_numbers")
					printops.lower_case_numbers = v;	
				else if(svar == "use_custom_result_font")
					use_custom_result_font = v;
				else if(svar == "use_custom_expression_font")
					use_custom_expression_font = v;											
				else if(svar == "custom_result_font")
					custom_result_font = svalue;
				else if(svar == "custom_expression_font")
					custom_expression_font = svalue;	
				else if(svar == "multiplication_sign")
					multi_sign = svalue;	
				else if(svar == "indicate_infinite_series")
					printops.indicate_infinite_series = v;
				else if(svar == "round_halfway_to_even")
					printops.round_halfway_to_even = v;	
				else if(svar == "always_exact")		//obsolete
					evalops.approximation = APPROXIMATION_EXACT;
				else if(svar == "approximation")
					evalops.approximation = (ApproximationMode) v;
				else if(svar == "in_rpn_mode")
					evalops.parse_options.rpn = v;
				else if(svar == "default_assumption_type")
					CALCULATOR->defaultAssumptions()->setNumberType((AssumptionNumberType) v);
				else if(svar == "default_assumption_sign")
					CALCULATOR->defaultAssumptions()->setSign((AssumptionSign) v);
				else if(svar == "recent_functions") {
					unsigned int v_i = 0;
					while(true) {
						v_i = svalue.find(',');
						if(v_i == string::npos) {
							svar = svalue.substr(0, svalue.length());
							remove_blank_ends(svar);
							if(!svar.empty()) {
								recent_functions_pre.push_back(svar);	
							}
							break;
						} else {
							svar = svalue.substr(0, v_i);
							svalue = svalue.substr(v_i + 1, svalue.length() - (v_i + 1));
							remove_blank_ends(svar);
							if(!svar.empty()) {
								recent_functions_pre.push_back(svar);	
							}
						}
					}
				} else if(svar == "recent_variables") {
					unsigned int v_i = 0;
					while(true) {
						v_i = svalue.find(',');
						if(v_i == string::npos) {
							svar = svalue.substr(0, svalue.length());
							remove_blank_ends(svar);
							if(!svar.empty()) {
								recent_variables_pre.push_back(svar);	
							}
							break;
						} else {
							svar = svalue.substr(0, v_i);
							svalue = svalue.substr(v_i + 1, svalue.length() - (v_i + 1));
							remove_blank_ends(svar);
							if(!svar.empty()) {
								recent_variables_pre.push_back(svar);	
							}
						}
					}
				} else if(svar == "recent_units") {
					unsigned int v_i = 0;
					while(true) {
						v_i = svalue.find(',');
						if(v_i == string::npos) {
							svar = svalue.substr(0, svalue.length());
							remove_blank_ends(svar);
							if(!svar.empty()) {
								recent_units_pre.push_back(svar);	
							}
							break;
						} else {
							svar = svalue.substr(0, v_i);
							svalue = svalue.substr(v_i + 1, svalue.length() - (v_i + 1));
							remove_blank_ends(svar);
							if(!svar.empty()) {
								recent_units_pre.push_back(svar);	
							}
						}
					}
				} else if(svar == "plot_legend_placement")
					default_plot_legend_placement = (PlotLegendPlacement) v;
				else if(svar == "plot_style")
					default_plot_style = (PlotStyle) v;
				else if(svar == "plot_smoothing")
					default_plot_smoothing = (PlotSmoothing) v;		
				else if(svar == "plot_display_grid")
					default_plot_display_grid = v;
				else if(svar == "plot_full_border")
					default_plot_full_border = (PlotSmoothing) v;
				else if(svar == "plot_min")
					default_plot_min = svalue;	
				else if(svar == "plot_max")
					default_plot_max = svalue;		
				else if(svar == "plot_sampling_rate")
					default_plot_sampling_rate = v;	
				else if(svar == "plot_variable")
					default_plot_variable = svalue;
				else if(svar == "plot_rows")
					default_plot_rows = v;	
				else if(svar == "plot_type")
					default_plot_type = v;	
				else if(svar == "plot_color")
					default_plot_color = v;
				else if(svar == "expression_history")
					expression_history.push_back(svalue);		
				else if(svar == "history") 
					initial_history.push_back(svalue);
				else if(svar == "history_width") 
					history_width = v;
				else if(svar == "history_height") 
					history_height = v;	
			}
		}
	} else {
		first_time = true;
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
					glade_xml_get_widget (main_glade, "main_window")
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
	fprintf(file, "fetch_exchange_rates_at_startup=%i\n", fetch_exchange_rates_at_startup);
#if GTK_MINOR_VERSION >= 3	
	fprintf(file, "show_buttons=%i\n", gtk_expander_get_expanded(GTK_EXPANDER(expander)));
#else
	fprintf(file, "show_buttons=%i\n", GTK_WIDGET_VISIBLE(glade_xml_get_widget (main_glade, "buttons")));
#endif
	fprintf(file, "spacious=%i\n", printops.spacious);
	fprintf(file, "excessive_parenthesis=%i\n", printops.excessive_parenthesis);
	fprintf(file, "short_multiplication=%i\n", printops.short_multiplication);
	fprintf(file, "use_unicode_signs=%i\n", printops.use_unicode_signs);
	fprintf(file, "lower_case_numbers=%i\n", printops.lower_case_numbers);
	fprintf(file, "place_units_separately=%i\n", printops.place_units_separately);	
	fprintf(file, "use_custom_result_font=%i\n", use_custom_result_font);	
	fprintf(file, "use_custom_expression_font=%i\n", use_custom_expression_font);	
	fprintf(file, "custom_result_font=%s\n", custom_result_font.c_str());	
	fprintf(file, "custom_expression_font=%s\n", custom_expression_font.c_str());
	fprintf(file, "multiplication_sign=%s\n", multi_sign.c_str());
	if(history_width != 325 || history_height != 250) {
		fprintf(file, "history_width=%i\n", history_width);	
		fprintf(file, "history_height=%i\n", history_height);	
	}
	for(unsigned int i = 0; i < expression_history.size(); i++) {
		fprintf(file, "expression_history=%s\n", expression_history[i].c_str()); 
	}	
	GtkTextIter iter1, iter2;
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history")));
	int lc = gtk_text_buffer_get_line_count(tb);
	if(lc > 20) lc = 20;
	lc--;
	for(int i = 0; i < lc; i++) {
		gtk_text_buffer_get_iter_at_line(tb, &iter1, i);
		gtk_text_buffer_get_iter_at_line(tb, &iter2, i + 1);
		fprintf(file, "history=%s", gtk_text_buffer_get_text(tb, &iter1, &iter2, TRUE)); 
	}
	fprintf(file, "recent_functions="); 
	for(int i = (int) (recent_functions.size()) - 1; i >= 0; i--) {
		fprintf(file, "%s", recent_functions[i]->referenceName().c_str()); 
		if(i != 0) fprintf(file, ","); 
	}
	fprintf(file, "\n"); 
	fprintf(file, "recent_variables="); 
	for(int i = (int) (recent_variables.size()) - 1; i >= 0; i--) {
		fprintf(file, "%s", recent_variables[i]->referenceName().c_str()); 
		if(i != 0) fprintf(file, ","); 
	}
	fprintf(file, "\n"); 	
	fprintf(file, "recent_units="); 
	for(int i = (int) (recent_units.size()) - 1; i >= 0; i--) {
		fprintf(file, "%s", recent_units[i]->referenceName().c_str()); 
		if(i != 0) fprintf(file, ","); 
	}
	fprintf(file, "\n"); 	
	if(mode)
		set_saved_mode();
	fprintf(file, "\n[Mode]\n");
	fprintf(file, "min_deci=%i\n", saved_printops.min_decimals);
	fprintf(file, "use_min_deci=%i\n", saved_printops.use_min_decimals);
	fprintf(file, "max_deci=%i\n", saved_printops.max_decimals);
	fprintf(file, "use_max_deci=%i\n", saved_printops.use_max_decimals);	
	fprintf(file, "precision=%i\n", saved_precision);
	fprintf(file, "min_exp=%i\n", saved_printops.min_exp);
	fprintf(file, "negative_exponents=%i\n", saved_printops.negative_exponents);
	fprintf(file, "sort_minus_last=%i\n", saved_printops.sort_options.minus_last);
	fprintf(file, "number_fraction_format=%i\n", saved_printops.number_fraction_format);	
	fprintf(file, "use_prefixes=%i\n", saved_printops.use_unit_prefixes);
	fprintf(file, "use_short_units=%i\n", saved_printops.abbreviate_units);
	fprintf(file, "all_prefixes_enabled=%i\n", saved_printops.use_all_prefixes);
	fprintf(file, "denominator_prefix_enabled=%i\n", saved_printops.use_denominator_prefix);
	fprintf(file, "number_base=%i\n", saved_printops.base);
	fprintf(file, "number_base_expression=%i\n", saved_evalops.parse_options.base);
	fprintf(file, "angle_unit=%i\n", saved_angle_unit);
	fprintf(file, "hyp_is_on=%i\n", saved_hyp_is_on);
	fprintf(file, "functions_enabled=%i\n", saved_evalops.parse_options.functions_enabled);
	fprintf(file, "variables_enabled=%i\n", saved_evalops.parse_options.variables_enabled);
	fprintf(file, "donot_calculate_variables=%i\n", !saved_evalops.calculate_variables);	
	fprintf(file, "unknownvariables_enabled=%i\n", saved_evalops.parse_options.unknowns_enabled);
	fprintf(file, "units_enabled=%i\n", saved_evalops.parse_options.units_enabled);
	fprintf(file, "indicate_infinite_series=%i\n", saved_printops.indicate_infinite_series);
	fprintf(file, "round_halfway_to_even=%i\n", saved_printops.round_halfway_to_even);
	fprintf(file, "approximation=%i\n", saved_evalops.approximation);	
	fprintf(file, "in_rpn_mode=%i\n", saved_evalops.parse_options.rpn);
	fprintf(file, "default_assumption_type=%i\n", CALCULATOR->defaultAssumptions()->numberType());
	fprintf(file, "default_assumption_sign=%i\n", CALCULATOR->defaultAssumptions()->sign());
	
	fprintf(file, "\n[Plotting]\n");
	fprintf(file, "plot_legend_placement=%i\n", default_plot_legend_placement);
	fprintf(file, "plot_style=%i\n", default_plot_style);
	fprintf(file, "plot_smoothing=%i\n", default_plot_smoothing);
	fprintf(file, "plot_display_grid=%i\n", default_plot_display_grid);
	fprintf(file, "plot_full_border=%i\n", default_plot_full_border);
	fprintf(file, "plot_min=%s\n", default_plot_min.c_str());
	fprintf(file, "plot_max=%s\n", default_plot_max.c_str());
	fprintf(file, "plot_sampling_rate=%i\n", default_plot_sampling_rate);
	fprintf(file, "plot_variable=%s\n", default_plot_variable.c_str());
	fprintf(file, "plot_rows=%i\n", default_plot_rows);
	fprintf(file, "plot_type=%i\n", default_plot_type);
	fprintf(file, "plot_color=%i\n", default_plot_color);

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
	GtkWidget *dialog = get_preferences_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	save_preferences();
	gtk_widget_grab_focus(expression);
}

#ifdef __cplusplus
extern "C" {
#endif

void on_history_dialog_destroy_event(GtkWidget *widget, gpointer user_data) {
	gint w = 0, h = 0;
	gtk_window_get_size(GTK_WINDOW(glade_xml_get_widget(main_glade, "history_dialog")), &w, &h);
	history_width = w;
	history_height = h;
	gtk_widget_hide(glade_xml_get_widget(main_glade, "history_dialog"));
	gtk_widget_grab_focus(expression);
}

#if GTK_MINOR_VERSION >= 3
void set_current_object() {
	if(!current_object_has_changed) return;
	while(gtk_events_pending()) gtk_main_iteration();
	int pos = gtk_editable_get_position(GTK_EDITABLE(expression));
	if(pos == 0) {
		current_object_start = -1;
		current_object_end = -1;
		return;
	}
	int start = pos - 1;
	gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), 0, pos);
	gchar *p = gstr + strlen(gstr);
	for(; start >= 0; start--) {
		p = g_utf8_prev_char(p);
		if(!CALCULATOR->utf8_pos_is_valid_in_name(p)) {
			break;
		}
	}
	start++;
	g_free(gstr);
	if(start >= pos) {
		current_object_start = -1;
		current_object_end = -1;
	} else {
		current_object_start = start;
		current_object_end = pos;
		gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), pos, -1);
		p = gstr;
		while(p[0] != '\0') {
			if(!CALCULATOR->utf8_pos_is_valid_in_name(p)) {
				break;
			}
			current_object_end++;
			p = g_utf8_next_char(p);
		}
		g_free(gstr);
	}
	current_object_has_changed = false;
}
gboolean on_completion_match_selected(GtkEntryCompletion *entrycompletion, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data) {
	set_current_object();
	gchar *gstr;
	gtk_tree_model_get(model, iter, 0, &gstr, -1);
	if(!gstr) return TRUE;
	gtk_editable_delete_text(GTK_EDITABLE(expression), current_object_start, current_object_end);
	gint pos = current_object_start;
	block_completion();
	if(gstr[strlen(gstr) - 1] == ')') {
		gchar *gstr2 = gtk_editable_get_chars(GTK_EDITABLE(expression), pos, pos + 1);
		if(strlen(gstr2) > 0 && gstr2[0] == '(') {
			gtk_editable_insert_text(GTK_EDITABLE(expression), gstr, strlen(gstr) - 2, &pos);
			gtk_editable_set_position(GTK_EDITABLE(expression), pos);
		} else {
			gtk_editable_insert_text(GTK_EDITABLE(expression), gstr, -1, &pos);
			gtk_editable_set_position(GTK_EDITABLE(expression), pos -1);
		}
		g_free(gstr2);
	} else {
		gtk_editable_insert_text(GTK_EDITABLE(expression), gstr, -1, &pos);
		gtk_editable_set_position(GTK_EDITABLE(expression), pos);
	}
	g_free(gstr);
	unblock_completion();
	return TRUE;
}
gboolean completion_match_func(GtkEntryCompletion *entrycompletion, const gchar *key, GtkTreeIter *iter, gpointer user_data) {
	set_current_object();
	if(current_object_start < 0) return FALSE;
	gchar *gstr1, *gstr2;
	gboolean b_match = false;
	gtk_tree_model_get(GTK_TREE_MODEL(completion_store), iter, 0, &gstr1, -1);
	if(!gstr1) return FALSE;	
	gstr2 = gtk_editable_get_chars(GTK_EDITABLE(expression), current_object_start, current_object_end);
	if(strlen(gstr2) <= strlen(gstr1)) {
		b_match = TRUE;
		for(unsigned int i = 0; i < strlen(gstr2); i++) {
			if(gstr1[i] != gstr2[i]) {
				b_match = FALSE;
				break;
			}
		}
	}
	g_free(gstr1);
	g_free(gstr2);
	return b_match;
}
#endif


void on_menu_item_quit_activate(GtkMenuItem *w, gpointer user_data) {
	on_gcalc_exit(NULL, NULL, user_data);
}

/*
	change preferences
*/
void on_preferences_checkbutton_lower_case_numbers_toggled(GtkToggleButton *w, gpointer user_data) {
	printops.lower_case_numbers = gtk_toggle_button_get_active(w);
	setResult();
}
void on_preferences_checkbutton_unicode_signs_toggled(GtkToggleButton *w, gpointer user_data) {
	printops.use_unicode_signs = gtk_toggle_button_get_active(w);
	if(printops.use_unicode_signs) {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sub")), SIGN_MINUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_add")), SIGN_PLUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_times")), SIGN_MULTIPLICATION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_divide")), SIGN_DIVISION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sqrt")), SIGN_SQRT);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_dot")), SIGN_MULTIDOT);	
	} else {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sub")), MINUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_add")), PLUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_times")), MULTIPLICATION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_divide")), DIVISION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sqrt")), "SQRT");	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_dot")), CALCULATOR->getDecimalPoint().c_str());
		if(multi_sign != "*") {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_asterisk")), TRUE);
		}
	}
	setResult();
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
void on_preferences_checkbutton_fetch_exchange_rates_toggled(GtkToggleButton *w, gpointer user_data) {
	fetch_exchange_rates_at_startup = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_custom_result_font_toggled(GtkToggleButton *w, gpointer user_data) {
	use_custom_result_font = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_result_font"), use_custom_result_font);
	if(use_custom_result_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_result_font.c_str());
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
void on_preferences_checkbutton_custom_expression_font_toggled(GtkToggleButton *w, gpointer user_data) {
	use_custom_expression_font = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_expression_font"), use_custom_expression_font);
	if(use_custom_expression_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_expression_font.c_str());
		gtk_widget_modify_font(expression, font);
		pango_font_description_free(font);
		viewresult(NULL);		
	} else {
		PangoFontDescription *font = pango_font_description_from_string("");
//		pango_font_description_set_weight(font, PANGO_WEIGHT_BOLD);		
		gtk_widget_modify_font(expression, font);
		pango_font_description_free(font);
		viewresult(NULL);			
	}
}
void on_preferences_radiobutton_dot_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		multi_sign = SIGN_MULTIDOT;
		viewresult(NULL);
	}
}
void on_preferences_radiobutton_ex_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		multi_sign = SIGN_MULTIPLICATION;
		viewresult(NULL);
	}
}
void on_preferences_radiobutton_asterisk_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		multi_sign = "*";
		viewresult(NULL);
	}
}
void on_preferences_button_result_font_clicked(GtkButton *w, gpointer user_data) {
	GtkWidget *d = gtk_font_selection_dialog_new(_("Select result font"));
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(d), custom_result_font.c_str());
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		custom_result_font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(d));
		gtk_button_set_label(w, custom_result_font.c_str());
		PangoFontDescription *font = pango_font_description_from_string(custom_result_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
		viewresult(NULL);
	}
	gtk_widget_destroy(d);
}
void on_preferences_button_expression_font_clicked(GtkButton *w, gpointer user_data) {
	GtkWidget *d = gtk_font_selection_dialog_new(_("Select expression font"));
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(d), custom_expression_font.c_str());
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		custom_expression_font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(d));
		gtk_button_set_label(w, custom_expression_font.c_str());
		PangoFontDescription *font = pango_font_description_from_string(custom_expression_font.c_str());
		gtk_widget_modify_font(expression, font);
		pango_font_description_free(font);
		viewresult(NULL);
	}
	gtk_widget_destroy(d);
}

/*
	hide unit manager when "Close" clicked
*/
void on_units_button_close_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(glade_xml_get_widget (units_glade, "units_dialog"));
	gtk_widget_grab_focus(expression);
}

/*
	change conversion direction in unit manager on user request
*/
void on_units_toggle_button_from_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (units_glade, "units_toggle_button_to")), FALSE);
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
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (units_glade, "units_toggle_button_from")), FALSE);
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

void update_resultview_popup() {
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_octal_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_decimal_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_hexadecimal_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_binary_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_roman"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_roman_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_sexagesimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_sexagesimal_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_time_format"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_time_format_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_custom_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_custom_base_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_normal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_normal_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_scientific_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_purely_scientific_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_non_scientific_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_prefixes_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_short_units"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_short_units_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_all_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_denominator_prefixes_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_denominator_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_all_prefixes_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal_exact"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_exact_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_combined"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_combined_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_fraction"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_fraction_activate, NULL);
	if(mstruct && mstruct->containsType(STRUCT_UNIT)) {
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_short_units"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_display_prefixes"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_all_prefixes"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_denominator_prefixes"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_unit_settings"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_convert_to_unit"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_convert_to_base_units"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_convert_to_best_unit"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_units"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_octal"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_decimal"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_hexadecimal"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_binary"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_roman"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_sexagesimal"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_time_format"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_custom_base"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_base"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_normal"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_display"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_factorize"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_factorize"));
	} else {
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_short_units"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_prefixes"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_all_prefixes"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_denominator_prefixes"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_unit_settings"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_convert_to_unit"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_convert_to_base_units"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_convert_to_best_unit"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_units"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_octal"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_decimal"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_hexadecimal"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_binary"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_roman"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_sexagesimal"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_time_format"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_custom_base"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_base"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_display_normal"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_display"));
		if(mstruct && mstruct->countChilds() > 0) {
			gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_factorize"));
			gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_factorize"));
		} else {
			gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_factorize"));
			gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_factorize"));
		}
	}
	switch (printops.base) {
		case BASE_OCTAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_octal")), TRUE);
			break;
		}
		case BASE_DECIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_decimal")), TRUE);
			break;
		}
		case BASE_HEXADECIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_hexadecimal")), TRUE);
			break;
		}
		case BASE_BINARY: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_binary")), TRUE);
			break;
		}
		case BASE_ROMAN_NUMERALS: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_roman")), TRUE);
			break;
		}
		case BASE_SEXAGESIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_sexagesimal")), TRUE);
			break;
		}
		case BASE_TIME: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_time_format")), TRUE);
			break;
		}
		default: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_custom_base")), TRUE);
			break;
		}
	}
	switch(printops.min_exp) {
		case EXP_PRECISION: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_display_normal")), TRUE);
			break;
		}
		case EXP_SCIENTIFIC: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_display_scientific")), TRUE);
			break;
		}
		case EXP_PURE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_display_purely_scientific")), TRUE);
			break;
		}
		case EXP_NONE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_display_non_scientific")), TRUE);
			break;
		}
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_display_prefixes")), printops.use_unit_prefixes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_short_units")), printops.abbreviate_units);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_all_prefixes")), printops.use_all_prefixes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_denominator_prefixes")), printops.use_denominator_prefix);
	switch(printops.number_fraction_format) {
		case FRACTION_DECIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_fraction_decimal")), TRUE);
			break;
		}
		case FRACTION_DECIMAL_EXACT: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_fraction_decimal_exact")), TRUE);
			break;
		}
		case FRACTION_COMBINED: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_fraction_combined")), TRUE);
			break;		
		}
		case FRACTION_FRACTIONAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_fraction_fraction")), TRUE);
			break;		
		}
	}
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_octal_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_decimal_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_hexadecimal_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_binary_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_roman"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_roman_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_sexagesimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_sexagesimal_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_time_format"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_time_format_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_custom_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_custom_base_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_normal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_normal_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_scientific_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_purely_scientific_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_non_scientific_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_prefixes_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_short_units"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_short_units_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_short_units"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_all_prefixes_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_all_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_denominator_prefixes_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal_exact"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_exact_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_combined"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_combined_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_fraction"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_fraction_activate, NULL);
}
gboolean on_resultview_button_press_event(GtkWidget *w, GdkEventButton *event, gpointer user_data) {
	if(event->button == 3) {
		update_resultview_popup();
		gtk_menu_popup(GTK_MENU(glade_xml_get_widget (main_glade, "popup_menu_resultview")), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	}
	return FALSE;
}
gboolean on_resultview_popup_menu(GtkWidget *w, gpointer user_data) {
	update_resultview_popup();
	gtk_menu_popup(GTK_MENU(glade_xml_get_widget (main_glade, "popup_menu_resultview")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	return TRUE;
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
	"Close" clicked -- quit
*/
void on_button_close_clicked(GtkButton *w, gpointer user_data) {
	on_gcalc_exit(NULL, NULL, user_data);
}

void on_button_history_clicked(GtkToggleButton *togglebutton, gpointer user_data) {
	if(!GTK_WIDGET_VISIBLE(glade_xml_get_widget(main_glade, "history_dialog"))) {
		gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(main_glade, "history_dialog")), history_width, history_height);
	}
	gtk_widget_show(glade_xml_get_widget(main_glade, "history_dialog"));
	gtk_window_present(GTK_WINDOW(glade_xml_get_widget(main_glade, "history_dialog")));
}
/*
	angle mode radio buttons toggled
*/
void on_radiobutton_radians_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		CALCULATOR->angleMode(RADIANS);
		set_angle_item();
		execute_expression();
	}
}
void on_radiobutton_degrees_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		CALCULATOR->angleMode(DEGREES);
		set_angle_item();
		execute_expression();
	}
}
void on_radiobutton_gradians_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		CALCULATOR->angleMode(GRADIANS);
		set_angle_item();
		execute_expression();
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
	GtkWidget	* window = glade_xml_get_widget (main_glade, "main_window");
	gint w = 0, h = 0, hh = 150;

	if(GTK_WIDGET_VISIBLE(glade_xml_get_widget (main_glade, "buttons"))) {
		hh = glade_xml_get_widget (main_glade, "buttons")->allocation.height;
		gtk_widget_hide(glade_xml_get_widget (main_glade, "buttons"));
		gtk_button_set_label(button, _("Show keypad"));
		//the extra widgets increased the window height with 150 pixels, decrease again
		gtk_window_get_size(GTK_WINDOW(window), &w, &h);
		gtk_window_resize(GTK_WINDOW(window), w, h - hh);
	} else {
		gtk_widget_show(glade_xml_get_widget (main_glade, "buttons"));
		gtk_button_set_label(button, _("Hide keypad"));
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
	if(plot_glade && GTK_WIDGET_VISIBLE(glade_xml_get_widget (plot_glade, "plot_dialog"))) {
		gtk_widget_hide(glade_xml_get_widget (plot_glade, "plot_dialog"));
	}
	if(GTK_WIDGET_VISIBLE(glade_xml_get_widget (main_glade, "history_dialog"))) {
		on_history_dialog_destroy_event(glade_xml_get_widget(main_glade, "history_dialog"), NULL);
	}
	if(save_mode_on_exit) {
		save_mode();
	} else {
		save_preferences();
	}
	if(save_defs_on_exit) {
		save_defs();
	}
	pthread_cancel(view_thread);
	CALCULATOR->terminateThreads();
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
		printops.number_fraction_format = FRACTION_FRACTIONAL;
		GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "menu_item_fraction_fraction");
		g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_fraction_activate, NULL);		
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w_fraction), TRUE);		
		g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_fraction_activate, NULL);		
	} else {
		printops.number_fraction_format = FRACTION_DECIMAL;
		GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "menu_item_fraction_decimal");
		g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_decimal_activate, NULL);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w_fraction), TRUE);
		g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_fraction_decimal_activate, NULL);
	}
	setResult();	
	gtk_widget_grab_focus(expression);	
}

/*
	Tan/Sin/Cos button clicked -- insert corresponding function
*/
void on_button_tan_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("tanh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_hyp")), FALSE);
	} else
		insertButtonFunction("tan");
}
void on_button_sine_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("sinh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_hyp")), FALSE);
	} else
		insertButtonFunction("sin");
}
void on_button_cosine_clicked(GtkButton *w, gpointer user_data) {
	if(hyp_is_on) {
		insertButtonFunction("cosh");
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_hyp")), FALSE);
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
		gtk_menu_popup(GTK_MENU(gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_expression")))), NULL, NULL, menu_e_posfunc, NULL, 0, 0);
	} else {
		gtk_menu_popdown(GTK_MENU(gtk_menu_item_get_submenu (GTK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_expression")))));
	}
}

/*
	result menu button clicked -- popup or hide the menu
*/
void
on_togglebutton_result_toggled                      (GtkToggleButton       *button,
                                        gpointer         user_data) {
	if(gtk_toggle_button_get_active(button)) {
		gtk_menu_popup(GTK_MENU(gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_result")))), NULL, NULL, menu_r_posfunc, NULL, 0, 0);
	} else {
		gtk_menu_popdown(GTK_MENU(gtk_menu_item_get_submenu(GTK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_result")))));
	}
}

/*
	clear the displayed result when expression changes
*/
void on_expression_changed(GtkEditable *w, gpointer user_data) {
#if GTK_MINOR_VERSION >= 3
	if(completion_blocked) {
		completion_blocked = false;
	} else {
		gtk_entry_completion_set_minimum_key_length(completion, 1);
	}
#endif
	expression_has_changed = true;
	current_object_has_changed = true;
	if(result_text.empty()) return;
	if(!dont_change_index) expression_history_index = -1;
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
	insert_text(CALCULATOR->getDecimalPoint().c_str());
}
void on_button_brace_open_clicked(GtkButton *w, gpointer user_data) {
	insert_text("(");
}
void on_button_brace_close_clicked(GtkButton *w, gpointer user_data) {
	insert_text(")");
}
void on_button_times_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text(multi_sign.c_str());
}
void on_button_add_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
//	if(printops.use_unicode_signs) insert_text(SIGN_PLUS);
//	else 
	insert_text("+");
}
void on_button_sub_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	if(printops.use_unicode_signs) insert_text(SIGN_MINUS);
	else insert_text("-");
}
void on_button_divide_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
//	if(printops.use_unicode_signs) insert_text(SIGN_DIVISION);
//	else 
	insert_text("/");
}
void on_button_ans_clicked(GtkButton *w, gpointer user_data) {
	insert_text("Ans");
}
void on_button_exp_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text("E");
}
void on_button_xy_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text("^");
}
void on_button_square_clicked(GtkButton *w, gpointer user_data) {
	wrap_expression_selection();
	insert_text("^");
	insert_text("2");
}

/*
	Button clicked -- insert corresponding function
*/
void on_button_sqrt_clicked(GtkButton *w, gpointer user_data) {
	if(printops.use_unicode_signs) insertButtonFunction(SIGN_SQRT, false);
	else insertButtonFunction("sqrt");
}
void on_button_log_clicked(GtkButton *w, gpointer user_data) {
	insertButtonFunction("log10");
}
void on_button_ln_clicked(GtkButton *w, gpointer user_data) {
	insertButtonFunction("ln");
}

void on_menu_item_manage_variables_activate(GtkMenuItem *w, gpointer user_data) {
	manage_variables();
}
void on_menu_item_manage_functions_activate(GtkMenuItem *w, gpointer user_data) {
	manage_functions();
}
void on_menu_item_manage_units_activate(GtkMenuItem *w, gpointer user_data) {
	manage_units();
}

void on_menu_item_import_csv_file_activate(GtkMenuItem *w, gpointer user_data) {
	import_csv_file(glade_xml_get_widget (main_glade, "main_window"));
}

void on_menu_item_export_csv_file_activate(GtkMenuItem *w, gpointer user_data) {
	export_csv_file(NULL, glade_xml_get_widget (main_glade, "main_window"));
}


void on_menu_item_convert_to_unit_expression_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = get_unit_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	if(GTK_WIDGET_VISIBLE(dialog)) {
		gtk_window_present(GTK_WINDOW(dialog));
	} else {
		gtk_widget_show(dialog);
	}
}
void on_menu_item_convert_to_best_unit_activate(GtkMenuItem *w, gpointer user_data) {
	mstruct->set(CALCULATOR->convertToBestUnit(*mstruct, evalops));
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_convert_to_base_units_activate(GtkMenuItem *w, gpointer user_data) {
	mstruct->set(CALCULATOR->convertToBaseUnits(*mstruct, evalops));
	setResult();
	gtk_widget_grab_focus(expression);
}

void on_menu_item_assumptions_integer_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_INTEGER);
	execute_expression();
}
void on_menu_item_assumptions_rational_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_RATIONAL);
	execute_expression();
}
void on_menu_item_assumptions_real_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_REAL);
	execute_expression();
}
void on_menu_item_assumptions_complex_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_COMPLEX);
	execute_expression();
}
void on_menu_item_assumptions_number_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_NUMBER);
	execute_expression();
}
void on_menu_item_assumptions_none_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_NONE);
	execute_expression();
}
void on_menu_item_assumptions_nonzero_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONZERO);
	execute_expression();
}
void on_menu_item_assumptions_positive_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_POSITIVE);
	execute_expression();
}
void on_menu_item_assumptions_nonnegative_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONNEGATIVE);
	execute_expression();
}
void on_menu_item_assumptions_negative_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NEGATIVE);
	execute_expression();
}
void on_menu_item_assumptions_nonpositive_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONPOSITIVE);
	execute_expression();
}
void on_menu_item_assumptions_unknown_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_UNKNOWN);
	execute_expression();
}

void on_menu_item_enable_variables_activate(GtkMenuItem *w, gpointer user_data) {
	evalops.parse_options.variables_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
}
void on_menu_item_enable_functions_activate(GtkMenuItem *w, gpointer user_data) {
	evalops.parse_options.functions_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
}
void on_menu_item_enable_units_activate(GtkMenuItem *w, gpointer user_data) {
	evalops.parse_options.units_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
}
void on_menu_item_enable_unknown_variables_activate(GtkMenuItem *w, gpointer user_data) {
	 evalops.parse_options.unknowns_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
}
void on_menu_item_calculate_variables_activate(GtkMenuItem *w, gpointer user_data) {
	evalops.calculate_variables = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	execute_expression();
}

void on_menu_item_new_unknown_activate(GtkMenuItem *w, gpointer user_data) {
	edit_unknown(_("My Variables"), NULL, glade_xml_get_widget (main_glade, "main_window"));
}
void on_menu_item_new_variable_activate(GtkMenuItem *w, gpointer user_data) {
	edit_variable(_("My Variables"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"));
}
void on_menu_item_new_matrix_activate(GtkMenuItem *w, gpointer user_data) {
	edit_matrix(_("Matrices"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), FALSE);
}
void on_menu_item_new_vector_activate(GtkMenuItem *w, gpointer user_data) {
	edit_matrix(_("Vectors"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), TRUE);
}
void on_menu_item_new_function_activate(GtkMenuItem *w, gpointer user_data) {
	edit_function("", NULL,	glade_xml_get_widget (main_glade, "main_window"));
}
void on_menu_item_new_unit_activate(GtkMenuItem *w, gpointer user_data) {
	edit_unit("", NULL, glade_xml_get_widget (main_glade, "main_window"));
}

void on_menu_item_rpn_mode_activate(GtkMenuItem *w, gpointer user_data) {
	evalops.parse_options.rpn = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	gtk_widget_grab_focus(expression);
}
void fetch_exchange_rates(int timeout) {
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, "Fetching exchange rates.");
	gtk_widget_show_now(dialog);
	while(gtk_events_pending()) gtk_main_iteration();
	CALCULATOR->fetchExchangeRates(timeout);
	gtk_widget_destroy(dialog);
	gtk_widget_grab_focus(expression);
}
void on_menu_item_fetch_exchange_rates_activate(GtkMenuItem *w, gpointer user_data) {
	fetch_exchange_rates(15);
	CALCULATOR->loadExchangeRates();
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
		execute_expression();
	}
}
void on_menu_item_radians_activate(GtkMenuItem *w, gpointer user_data) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		gint i = RADIANS;
		CALCULATOR->angleMode(i);
		set_angle_button();
		execute_expression();
	}
}
void on_menu_item_gradians_activate(GtkMenuItem *w, gpointer user_data) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		gint i = GRADIANS;
		CALCULATOR->angleMode(i);
		set_angle_button();
		execute_expression();
	}
}
void on_menu_item_binary_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_BINARY;
	setResult();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (main_glade, "number_base_spinbutton_base")), 2);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_widget_grab_focus(expression);
}
void on_menu_item_octal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_OCTAL;
	setResult();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (main_glade, "number_base_spinbutton_base")), 8);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_widget_grab_focus(expression);
}
void on_menu_item_decimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_DECIMAL;
	setResult();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (main_glade, "number_base_spinbutton_base")), 10);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_widget_grab_focus(expression);
}
void on_menu_item_hexadecimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_HEXADECIMAL;
	setResult();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (main_glade, "number_base_spinbutton_base")), 16);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "number_base_spinbutton_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_number_base_spinbutton_base_value_changed, NULL);
	gtk_widget_grab_focus(expression);
}
void on_menu_item_custom_base_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	gtk_widget_show(glade_xml_get_widget (main_glade, "number_base_dialog"));
	gtk_window_present(GTK_WINDOW(glade_xml_get_widget (main_glade, "number_base_dialog")));
	on_number_base_spinbutton_base_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (main_glade, "number_base_spinbutton_base")), NULL);
}
void on_number_base_spinbutton_base_value_changed(GtkSpinButton *w, gpointer user_data) {
	printops.base = gtk_spin_button_get_value_as_int(w);
	switch(printops.base) {
		case 2: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_binary_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_binary")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_binary_activate, NULL);
			break;
		}
		case 8: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_octal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_octal")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_octal_activate, NULL);
			break;
		}
		case 10: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_decimal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_decimal")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_decimal_activate, NULL);
			break;
		}
		case 16: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_hexadecimal")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			break;
		}
		default: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_custom_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_custom_base_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_custom_base")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_custom_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_custom_base_activate, NULL);
			break;
		}
	}
	setResult();
}
void on_menu_item_roman_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_ROMAN_NUMERALS;
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_sexagesimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_SEXAGESIMAL;
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_time_format_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_TIME;
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_expression_base_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = get_number_base_expression_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
}
void on_number_base_expression_radiobutton_binary_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_BINARY;
	gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
}
void on_number_base_expression_radiobutton_octal_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_OCTAL;
	gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
}
void on_number_base_expression_radiobutton_decimal_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_DECIMAL;
	gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
}
void on_number_base_expression_radiobutton_hexadecimal_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_HEXADECIMAL;
	gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
}
void on_number_base_expression_radiobutton_custom_base_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base")));
	gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), TRUE);
}
void on_number_base_expression_spinbutton_base_value_changed(GtkSpinButton *w, gpointer user_data) {
	evalops.parse_options.base = gtk_spin_button_get_value_as_int(w);
}
void on_number_base_expression_radiobutton_roman_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_ROMAN_NUMERALS;
	gtk_widget_set_sensitive(glade_xml_get_widget (nbexpression_glade, "number_base_expression_spinbutton_custom_base"), FALSE);
}
void on_menu_item_short_units_activate(GtkMenuItem *w, gpointer user_data) {
	printops.abbreviate_units = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult();
}
void on_menu_item_all_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	printops.use_all_prefixes = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult();
}
void on_menu_item_denominator_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	printops.use_denominator_prefix = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult();
}
void on_menu_item_place_units_separately_activate(GtkMenuItem *w, gpointer user_data) {
	printops.place_units_separately = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult();
}
void on_menu_item_multiple_roots_activate(GtkMenuItem *w, gpointer user_data) {
	//CALCULATOR->setMultipleRootsEnabled(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
	execute_expression();
}
void on_menu_item_factorize_activate(GtkMenuItem *w, gpointer user_data) {
	mstruct->factorize(evalops);
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_convert_number_bases_activate(GtkMenuItem *w, gpointer user_data) {
	changing_in_nbases_dialog = false;
	GtkWidget *dialog = get_nbases_dialog();
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
}
void on_menu_item_plot_functions_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = get_plot_dialog();
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")), gtk_entry_get_text(GTK_ENTRY(expression)));
	if(!GTK_WIDGET_VISIBLE(dialog)) {
		gtk_list_store_clear(tPlotFunctions_store);
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_modify"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_remove"), FALSE);	

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_grid")), default_plot_display_grid);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_full_border")), default_plot_full_border);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")), default_plot_rows);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_color")), default_plot_color);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_mono")), !default_plot_color);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_min")), default_plot_min.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_max")), default_plot_max.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")), default_plot_variable.c_str());
		switch(default_plot_type) {
			case 1: {gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_vector")), TRUE); break;}
			case 2: {gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_paired")), TRUE); break;}
			default: {gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_function")), TRUE); break;}
		}
		switch(default_plot_legend_placement) {
			case PLOT_LEGEND_NONE: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")), PLOTLEGEND_MENU_NONE); break;}
			case PLOT_LEGEND_TOP_LEFT: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")), PLOTLEGEND_MENU_TOP_LEFT); break;}
			case PLOT_LEGEND_TOP_RIGHT: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")), PLOTLEGEND_MENU_TOP_RIGHT); break;}
			case PLOT_LEGEND_BOTTOM_LEFT: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")), PLOTLEGEND_MENU_BOTTOM_LEFT); break;}
			case PLOT_LEGEND_BOTTOM_RIGHT: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")), PLOTLEGEND_MENU_BOTTOM_RIGHT); break;}
			case PLOT_LEGEND_BELOW: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")), PLOTLEGEND_MENU_BELOW); break;}
			case PLOT_LEGEND_OUTSIDE: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")), PLOTLEGEND_MENU_OUTSIDE); break;}
		}
		switch(default_plot_smoothing) {
			case PLOT_SMOOTHING_NONE: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing")), SMOOTHING_MENU_NONE); break;}
			case PLOT_SMOOTHING_UNIQUE: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing")), SMOOTHING_MENU_UNIQUE); break;}
			case PLOT_SMOOTHING_CSPLINES: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing")), SMOOTHING_MENU_CSPLINES); break;}
			case PLOT_SMOOTHING_BEZIER: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing")), SMOOTHING_MENU_BEZIER); break;}
			case PLOT_SMOOTHING_SBEZIER: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing")), SMOOTHING_MENU_SBEZIER); break;}
		}
		switch(default_plot_style) {
			case PLOT_STYLE_LINES: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_LINES); break;}
			case PLOT_STYLE_POINTS: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_LINES); break;}
			case PLOT_STYLE_POINTS_LINES: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_LINESPOINTS); break;}
			case PLOT_STYLE_BOXES: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_BOXES); break;}
			case PLOT_STYLE_HISTOGRAM: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_HISTEPS); break;}
			case PLOT_STYLE_STEPS: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_STEPS); break;}
			case PLOT_STYLE_CANDLESTICKS: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_CANDLESTICKS); break;}
			case PLOT_STYLE_DOTS: {gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")), PLOTSTYLE_MENU_DOTS); break;}
		}
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (plot_glade, "plot_spinbutton_steps")), default_plot_sampling_rate);
		
		gtk_widget_show(dialog);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (plot_glade, "plot_notebook")), 1);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (plot_glade, "plot_notebook")), 0);
	} else {
		gtk_window_present(GTK_WINDOW(dialog));
	}
}
void on_plot_dialog_hide(GtkWidget *w, gpointer user_data) {
	default_plot_display_grid = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_grid")));
	default_plot_full_border = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_full_border")));
	default_plot_rows = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")));
	default_plot_color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_color")));
	default_plot_min = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_min")));
	default_plot_max = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_max")));
	default_plot_variable = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_vector")))) {
		default_plot_type = 1;
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_paired")))) {
		default_plot_type = 2;
	} else {
		default_plot_type = 0;
	}
	switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")))) {
		case PLOTLEGEND_MENU_NONE: {default_plot_legend_placement = PLOT_LEGEND_NONE; break;}
		case PLOTLEGEND_MENU_TOP_LEFT: {default_plot_legend_placement = PLOT_LEGEND_TOP_LEFT; break;}
		case PLOTLEGEND_MENU_TOP_RIGHT: {default_plot_legend_placement = PLOT_LEGEND_TOP_RIGHT; break;}
		case PLOTLEGEND_MENU_BOTTOM_LEFT: {default_plot_legend_placement = PLOT_LEGEND_BOTTOM_LEFT; break;}
		case PLOTLEGEND_MENU_BOTTOM_RIGHT: {default_plot_legend_placement = PLOT_LEGEND_BOTTOM_RIGHT; break;}
		case PLOTLEGEND_MENU_BELOW: {default_plot_legend_placement = PLOT_LEGEND_BELOW; break;}
		case PLOTLEGEND_MENU_OUTSIDE: {default_plot_legend_placement = PLOT_LEGEND_OUTSIDE; break;}
	}
	switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing")))) {
		case SMOOTHING_MENU_NONE: {default_plot_smoothing = PLOT_SMOOTHING_NONE; break;}
		case SMOOTHING_MENU_UNIQUE: {default_plot_smoothing = PLOT_SMOOTHING_UNIQUE; break;}
		case SMOOTHING_MENU_CSPLINES: {default_plot_smoothing = PLOT_SMOOTHING_CSPLINES; break;}
		case SMOOTHING_MENU_BEZIER: {default_plot_smoothing = PLOT_SMOOTHING_BEZIER; break;}
		case SMOOTHING_MENU_SBEZIER: {default_plot_smoothing = PLOT_SMOOTHING_SBEZIER; break;}
	}
	switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style")))) {
		case PLOTSTYLE_MENU_LINES: {default_plot_style = PLOT_STYLE_LINES; break;}
		case PLOTSTYLE_MENU_POINTS: {default_plot_style = PLOT_STYLE_POINTS; break;}
		case PLOTSTYLE_MENU_LINESPOINTS: {default_plot_style = PLOT_STYLE_POINTS_LINES; break;}
		case PLOTSTYLE_MENU_BOXES: {default_plot_style = PLOT_STYLE_BOXES; break;}
		case PLOTSTYLE_MENU_HISTEPS: {default_plot_style = PLOT_STYLE_HISTOGRAM; break;}
		case PLOTSTYLE_MENU_STEPS: {default_plot_style = PLOT_STYLE_STEPS; break;}
		case PLOTSTYLE_MENU_CANDLESTICKS: {default_plot_style = PLOT_STYLE_CANDLESTICKS; break;}
		case PLOTSTYLE_MENU_DOTS: {default_plot_style = PLOT_STYLE_DOTS; break;}
	}
	default_plot_sampling_rate = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (plot_glade, "plot_spinbutton_steps")));
	CALCULATOR->closeGnuplot();
}
void on_popup_menu_item_display_normal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_normal")), TRUE);
}
void on_popup_menu_item_display_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_scientific")), TRUE);
}
void on_popup_menu_item_display_purely_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_purely_scientific")), TRUE);
}
void on_popup_menu_item_display_non_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_non_scientific")), TRUE);
}
void on_popup_menu_item_display_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_prefixes")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_fraction_decimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_decimal")), TRUE);
}
void on_popup_menu_item_fraction_decimal_exact_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_decimal_exact")), TRUE);
}
void on_popup_menu_item_fraction_combined_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_combined")), TRUE);
}
void on_popup_menu_item_fraction_fraction_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_fraction")), TRUE);
}
void on_popup_menu_item_binary_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_binary")), TRUE);
}
void on_popup_menu_item_roman_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_roman")), TRUE);
}
void on_popup_menu_item_sexagesimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sexagesimal")), TRUE);
}
void on_popup_menu_item_time_format_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_time_format")), TRUE);
}
void on_popup_menu_item_octal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_octal")), TRUE);
}
void on_popup_menu_item_decimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_decimal")), TRUE);
}
void on_popup_menu_item_hexadecimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_hexadecimal")), TRUE);
}
void on_popup_menu_item_custom_base_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_custom_base")), TRUE);
}
void on_popup_menu_item_short_units_activate(GtkMenuItem *w, gpointer user_data) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_short_units")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_all_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_all_prefixes")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_denominator_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_denominator_prefixes")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}

void on_menu_item_display_normal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_PRECISION;
	printops.negative_exponents = false;
	printops.sort_options.minus_last = true;
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_SCIENTIFIC;
	printops.negative_exponents = true;
	printops.sort_options.minus_last = false;
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_purely_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_PURE;
	printops.negative_exponents = true;
	printops.sort_options.minus_last = false;
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_non_scientific_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_NONE;
	printops.negative_exponents = false;
	printops.sort_options.minus_last = true;
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_display_prefixes_activate(GtkMenuItem *w, gpointer user_data) {
	printops.use_unit_prefixes = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_indicate_infinite_series_activate(GtkMenuItem *w, gpointer user_data) {
	printops.indicate_infinite_series = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_round_halfway_to_even_activate(GtkMenuItem *w, gpointer user_data) {
	printops.round_halfway_to_even = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_always_exact_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.approximation = APPROXIMATION_EXACT;
	execute_expression();
}
void on_menu_item_try_exact_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.approximation = APPROXIMATION_TRY_EXACT;
	execute_expression();
}
void on_menu_item_approximate_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.approximation = APPROXIMATION_APPROXIMATE;
	execute_expression();
}
void on_menu_item_fraction_decimal_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_DECIMAL;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_fraction_decimal_exact_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_DECIMAL_EXACT;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_fraction_combined_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_COMBINED;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	setResult();
	gtk_widget_grab_focus(expression);
}
void on_menu_item_fraction_fraction_activate(GtkMenuItem *w, gpointer user_data) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_FRACTIONAL;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), TRUE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	setResult();
	gtk_widget_grab_focus(expression);
}

void on_menu_item_save_activate(GtkMenuItem *w, gpointer user_data) {
	add_as_variable();
}
void on_menu_item_copy_activate(GtkMenuItem *w, gpointer user_data) {
	gtk_clipboard_set_text(gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE)), result_text.c_str(), -1);
}
void on_menu_item_precision_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = get_precision_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_precision_dialog_spinbutton_precision_value_changed, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision")), PRECISION);	
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_precision_dialog_spinbutton_precision_value_changed, NULL);
	gtk_widget_show(dialog);
}
void on_menu_item_decimals_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = get_decimals_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_max_toggled, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_min_toggled, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_max_value_changed, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_min_value_changed, NULL);	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_min")), printops.use_min_decimals);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_max")), printops.use_max_decimals);	
	gtk_widget_set_sensitive (glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_min"), printops.use_min_decimals);
	gtk_widget_set_sensitive (glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_max"), printops.use_max_decimals);	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_min")), printops.min_decimals);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_max")), printops.max_decimals);	
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_max_toggled, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_checkbutton_min_toggled, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_max"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_max_value_changed, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_min"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_decimals_dialog_spinbutton_min_value_changed, NULL);	
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
				glade_xml_get_widget (unitedit_glade, *pointer),
				(gtk_option_menu_get_history(om) == UNIT_CLASS_COMPOSITE_UNIT) ? FALSE : TRUE
				);
	}

	/* making the composite widgets (un)sensitive */
	for (pointer = composite2; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (unitedit_glade, *pointer),
				(gtk_option_menu_get_history(om) == UNIT_CLASS_COMPOSITE_UNIT) ? TRUE : FALSE
				);
	}

	/* making the alias widgets (un)sensitive */
	for (pointer = alias; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (unitedit_glade, *pointer),
				(gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT) ? TRUE : FALSE
				);
	}

	/* making the non-base widgets (un)sensitive */
	for (pointer = base; *pointer != NULL; pointer++)
	{
		gtk_widget_set_sensitive (
				glade_xml_get_widget (unitedit_glade, *pointer),
				(gtk_option_menu_get_history(om) == UNIT_CLASS_BASE_UNIT) ? FALSE : TRUE
				);
	}

	
	
}
/*
	"New" button clicked in unit manager -- open new unit dialog
*/
void on_units_button_new_clicked(GtkButton *button, gpointer user_data) {
	if(selected_unit_category.empty() || selected_unit_category[0] != '/') {
		edit_unit("", NULL, glade_xml_get_widget (units_glade, "units_dialog"));
	} else {
		//fill in category field with selected category
		edit_unit(selected_unit_category.substr(1, selected_unit_category.length() - 1).c_str(), NULL, glade_xml_get_widget (units_glade, "units_dialog"));
	}
}

/*
	"Edit" button clicked in unit manager -- open edit unit dialog for selected unit
*/
void on_units_button_edit_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		edit_unit("", u, glade_xml_get_widget (units_glade, "units_dialog"));
	}
}

/*
	"Insert" button clicked in unit manager -- insert selected unit in expression entry
*/
void on_units_button_insert_clicked(GtkButton *button, gpointer user_data) {
	Unit *u = get_selected_unit();
	if(u) {
		gchar *gstr;
		if(printops.abbreviate_units) {
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
		CALCULATOR->convert(*mstruct, u, evalops);
		setResult();
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
			show_message(_("Cannot delete unit as it is needed by other units."), glade_xml_get_widget (units_glade, "units_dialog"));
			return;
		}
		for(unsigned int i = 0; i < recent_units.size(); i++) {
			if(recent_units[i] == u) {
				recent_units.erase(recent_units.begin() + i);
				gtk_widget_destroy(recent_unit_items[i]);
				recent_unit_items.erase(recent_unit_items.begin() + i);
				break;
			}
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
	if(selected_variable_category.empty() || selected_variable_category[0] != '/') {
		edit_variable("", NULL, NULL, glade_xml_get_widget (variables_glade, "variables_dialog"));
	} else {
		//fill in category field with selected category
		edit_variable(selected_variable_category.substr(1, selected_variable_category.length() - 1).c_str(), NULL, NULL, glade_xml_get_widget (variables_glade, "variables_dialog"));
	}
}

/*
	"Edit" button clicked in variable manager -- open edit dialog for selected variable
*/
void on_variables_button_edit_clicked(GtkButton *button, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v) {
		if(!CALCULATOR->hasVariable(v)) {
			show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
			return;
		}
		edit_variable("", v, NULL, glade_xml_get_widget (variables_glade, "variables_dialog"));
	}
}

/*
	"Insert" button clicked in variable manager -- insert variable name in expression entry
*/
void on_variables_button_insert_clicked(GtkButton *button, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v) {
		if(!CALCULATOR->hasVariable(v)) {
			show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
			return;
		}
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
	if(v && !CALCULATOR->hasVariable(v)) {
		show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
		return;
	}
	if(v && v->isLocal()) {
		for(unsigned int i = 0; i < recent_variables.size(); i++) {
			if(recent_variables[i] == v) {
				recent_variables.erase(recent_variables.begin() + i);
				gtk_widget_destroy(recent_variable_items[i]);
				recent_variable_items.erase(recent_variable_items.begin() + i);
				break;
			}
		}
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

void on_variables_button_export_clicked(GtkButton *button, gpointer user_data) {
	Variable *v = get_selected_variable();
	if(v && !CALCULATOR->hasVariable(v)) {
		show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
		return;
	}
	if(v && v->isKnown()) {
		export_csv_file((KnownVariable*) v, glade_xml_get_widget (variables_glade, "variables_dialog"));
	}
}

/*
	"Close" button clicked in variable manager -- hide
*/
void on_variables_button_close_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(glade_xml_get_widget (variables_glade, "variables_dialog"));
	gtk_widget_grab_focus(expression);
}

/*
	"New" button clicked in function manager -- open new function dialog
*/
void on_functions_button_new_clicked(GtkButton *button, gpointer user_data) {
	if(selected_function_category.empty() || selected_function_category[0] != '/') {
		edit_function("", NULL, glade_xml_get_widget (functions_glade, "functions_dialog"));
	} else {
		//fill in category field with selected category
		edit_function(selected_function_category.substr(1, selected_function_category.length() - 1).c_str(), NULL, glade_xml_get_widget (functions_glade, "functions_dialog"));
	}
}

/*
	"Edit" button clicked in function manager -- open edit function dialog for selected function
*/
void on_functions_button_edit_clicked(GtkButton *button, gpointer user_data) {
	Function *f = get_selected_function();
	if(f) {
		edit_function("", f, glade_xml_get_widget (functions_glade, "functions_dialog"));
	}
}

/*
	"Insert" button clicked in function manager -- open dialog for insertion of function in expression entry
*/
void on_functions_button_insert_clicked(GtkButton *button, gpointer user_data) {
	insert_function(get_selected_function(), glade_xml_get_widget (functions_glade, "functions_dialog"));
}

/*
	"Delete" button clicked in function manager -- deletion of selected function requested
*/
void on_functions_button_delete_clicked(GtkButton *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Function *f = get_selected_function();
	if(f && f->isLocal()) {
		for(unsigned int i = 0; i < recent_functions.size(); i++) {
			if(recent_functions[i] == f) {
				recent_functions.erase(recent_functions.begin() + i);
				gtk_widget_destroy(recent_function_items[i]);
				recent_function_items.erase(recent_function_items.begin() + i);
				break;
			}
		}
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
	gtk_widget_hide(glade_xml_get_widget (functions_glade, "functions_dialog"));
	gtk_widget_grab_focus(expression);
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
	GtkTable *table = GTK_TABLE(glade_xml_get_widget(matrixedit_glade, "matrix_edit_table_elements")); 
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
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix")))) {
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
	GtkTable *table = GTK_TABLE(glade_xml_get_widget(matrixedit_glade, "matrix_edit_table_elements")); 
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
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix")))) {
				gtk_entry_set_text(GTK_ENTRY(entry), "0");
			}		
			gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
			gtk_table_attach(table, entry, index_c, index_c + 1, index_r, index_r + 1, (GtkAttachOptions) 0, GTK_FILL, 0, 0);
			gtk_widget_show(entry);
			element_entries[index_r].push_back(entry);
		}
	}
}

void update_nbases_entries(const MathStructure &value, int base) {
	GtkWidget *w_dec, *w_bin, *w_oct, *w_hex;
	w_dec = glade_xml_get_widget (nbases_glade, "nbases_entry_decimal");
	w_bin = glade_xml_get_widget (nbases_glade, "nbases_entry_binary");	
	w_oct = glade_xml_get_widget (nbases_glade, "nbases_entry_octal");	
	w_hex = glade_xml_get_widget (nbases_glade, "nbases_entry_hexadecimal");	
	g_signal_handlers_block_matched((gpointer) w_dec, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_decimal_changed, NULL);			
	g_signal_handlers_block_matched((gpointer) w_bin, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_binary_changed, NULL);
	g_signal_handlers_block_matched((gpointer) w_oct, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_octal_changed, NULL);
	g_signal_handlers_block_matched((gpointer) w_hex, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_hexadecimal_changed, NULL);
	PrintOptions po;
	po.number_fraction_format = FRACTION_DECIMAL;
	if(base != 10) {po.base = 10; gtk_entry_set_text(GTK_ENTRY(w_dec), value.print(po).c_str());}
	if(base != 2) {po.base = 2; gtk_entry_set_text(GTK_ENTRY(w_bin), value.print(po).c_str());}	
	if(base != 8) {po.base = 8; gtk_entry_set_text(GTK_ENTRY(w_oct), value.print(po).c_str());}	
	if(base != 16) {po.base = 16; gtk_entry_set_text(GTK_ENTRY(w_hex), value.print(po).c_str());}	
	g_signal_handlers_unblock_matched((gpointer) w_dec, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_decimal_changed, NULL);			
	g_signal_handlers_unblock_matched((gpointer) w_bin, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_binary_changed, NULL);
	g_signal_handlers_unblock_matched((gpointer) w_oct, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_octal_changed, NULL);
	g_signal_handlers_unblock_matched((gpointer) w_hex, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_nbases_entry_hexadecimal_changed, NULL);
}
void on_nbases_button_close_clicked(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(glade_xml_get_widget (nbases_glade, "nbases_dialog"));
	gtk_widget_grab_focus(expression);
}
void on_nbases_entry_decimal_changed(GtkEditable *editable, gpointer user_data) {	
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;
	if(is_in(OPERATORS EXP, str[str.length() - 1])) return;
	changing_in_nbases_dialog = true;	
	MathStructure value = CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(editable))));
	update_nbases_entries(value, 10);
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_binary_changed(GtkEditable *editable, gpointer user_data) {
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;
	if(is_in(OPERATORS, str[str.length() - 1])) return;
	EvaluationOptions eo;
	eo.parse_options.base = BASE_BINARY;
	changing_in_nbases_dialog = true;	
	update_nbases_entries(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str), eo), 2);
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_octal_changed(GtkEditable *editable, gpointer user_data) {
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;	
	if(is_in(OPERATORS, str[str.length() - 1])) return;
	EvaluationOptions eo;
	eo.parse_options.base = BASE_OCTAL;
	changing_in_nbases_dialog = true;	
	update_nbases_entries(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str), eo), 8);
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_hexadecimal_changed(GtkEditable *editable, gpointer user_data) {
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;	
	if(is_in(OPERATORS, str[str.length() - 1])) return;
	EvaluationOptions eo;
	eo.parse_options.base = BASE_HEXADECIMAL;
	changing_in_nbases_dialog = true;	
	update_nbases_entries(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str), eo), 16);
	changing_in_nbases_dialog = false;	
}

void on_button_functions_clicked(GtkButton *button, gpointer user_data) {
	manage_functions();
}
void on_button_variables_clicked(GtkButton *button, gpointer user_data) {
	manage_variables();
}
void on_button_units_clicked(GtkButton *button, gpointer user_data) {
	manage_units();
}
void on_button_convert_clicked(GtkButton *button, gpointer user_data) {
	on_menu_item_convert_to_unit_expression_activate(NULL, user_data);
}

void on_menu_item_about_activate(GtkMenuItem *w, gpointer user_data) {
	GtkWidget *dialog = get_about_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
	gtk_widget_grab_focus(expression);
}
void on_menu_item_help_activate(GtkMenuItem *w, gpointer user_data) {
#ifdef HAVE_LIBGNOME
	GError *error = NULL;
	gnome_help_display_desktop(NULL, "qalculate-gtk", "qalculate-gtk", NULL, &error);
	if (error) {
		gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help for Qalculate!.\n%s"), error_str);
		g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		gtk_widget_show (dialog);
		g_free(error_str);
		g_error_free(error);
	}
#endif	
	gtk_widget_grab_focus(expression);
}

/*
	precision has changed in precision dialog
*/
void on_precision_dialog_spinbutton_precision_value_changed(GtkSpinButton *w, gpointer user_data) {
	CALCULATOR->setPrecision(gtk_spin_button_get_value_as_int(w));
//	execute_expression();
}
void on_precision_dialog_button_recalculate_clicked(GtkButton *w, gpointer user_data) {
	CALCULATOR->setPrecision(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision"))));
	execute_expression();
}


void on_decimals_dialog_spinbutton_max_value_changed(GtkSpinButton *w, gpointer user_data) {
	printops.max_decimals = gtk_spin_button_get_value_as_int(w);
	setResult();
}
void on_decimals_dialog_spinbutton_min_value_changed(GtkSpinButton *w, gpointer user_data) {
	printops.min_decimals = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
	setResult();
}
void on_decimals_dialog_checkbutton_max_toggled(GtkToggleButton *w, gpointer user_data) {
	printops.use_max_decimals = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_max"), printops.use_max_decimals);
	setResult();
}
void on_decimals_dialog_checkbutton_min_toggled(GtkToggleButton *w, gpointer user_data) {
	printops.use_min_decimals = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_min"), printops.use_min_decimals);
	setResult();
}

void on_unknown_edit_checkbutton_custom_assumptions_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_type"), gtk_toggle_button_get_active(w));
	gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_sign"), gtk_toggle_button_get_active(w));
}

gboolean on_expression_key_press_event(GtkWidget *w, GdkEventKey *event, gpointer user_data) {
	switch(event->keyval) {
		case GDK_dead_circumflex: {
			gint end = 0;
			wrap_expression_selection();
			end = gtk_editable_get_position(GTK_EDITABLE(expression));
			gtk_editable_insert_text(GTK_EDITABLE(expression), "^", -1, &end);				
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
		case GDK_Page_Up: {
			if(expression_history_index + 1 < (int) expression_history.size()) {
				expression_history_index++;
				dont_change_index = true;
				gtk_entry_set_text(GTK_ENTRY(expression), expression_history[expression_history_index].c_str());
				dont_change_index = false;
			}
			return TRUE;
		}
		case GDK_Page_Down: {
			if(expression_history_index > -1) {
				expression_history_index--;
				dont_change_index = true;
				if(expression_history_index < 0) {
					gtk_entry_set_text(GTK_ENTRY(expression), "");
				} else {
					gtk_entry_set_text(GTK_ENTRY(expression), expression_history[expression_history_index].c_str());
				}
				dont_change_index = false;
			}
			return TRUE;
		}
	}
	gint pos = gtk_editable_get_position(GTK_EDITABLE(expression));
	switch(event->keyval) {
		case GDK_KP_Multiply: {}			
		case GDK_asterisk: {
			gtk_editable_insert_text(GTK_EDITABLE(expression), multi_sign.c_str(), -1, &pos);
			gtk_editable_set_position(GTK_EDITABLE(expression), pos);
			return TRUE;			
		}								
		case GDK_KP_Subtract: {}								
		case GDK_minus: {
			if(printops.use_unicode_signs) {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_MINUS, -1, &pos);
				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
				return TRUE;
			}
		}						
	}	
	return FALSE;
}

gboolean on_resultview_expose_event(GtkWidget *w, GdkEventExpose *event, gpointer user_data) {
	if(pixbuf_result) {
		gint w = 0, h = 0;
		//gdk_drawable_get_size(GDK_DRAWABLE(pixmap_result), &w, &h);	
		w = gdk_pixbuf_get_width(pixbuf_result);
		h = gdk_pixbuf_get_height(pixbuf_result);
		if(resultview->allocation.width > w) {
//			gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_result), 0, 0, resultview->allocation.width - w, (resultview->allocation.height - h) / 2, -1, -1);
			gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, resultview->allocation.width - w, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
		} else {
//			gdk_draw_drawable(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_result), 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1);
			gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
		}
	}	
	return TRUE;
}
void on_matrix_edit_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer user_data) {
	if(gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Components (in horizontal order)"));
	}
}
void on_matrix_edit_radiobutton_vector_toggled(GtkToggleButton *w, gpointer user_data) {
	if(!gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Components (in horizontal order)"));
	}
}
void on_csv_import_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer user_data) {
}
void on_csv_import_radiobutton_vectors_toggled(GtkToggleButton *w, gpointer user_data) {
}
void on_csv_import_optionmenu_delimiter_changed(GtkOptionMenu *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (csvimport_glade, "csv_import_entry_delimiter_other"), gtk_option_menu_get_history(w) == DELIMITER_OTHER);
}
void on_csv_import_button_file_clicked(GtkButton *button, gpointer user_data) {
#if GTK_MINOR_VERSION >= 3
	GtkWidget *d = gtk_file_chooser_dialog_new(_("Select file to import"), GTK_WINDOW(glade_xml_get_widget(csvimport_glade, "csv_import_dialog")), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_MICRO_VERSION >= 6 || GTK_MINOR_VERSION >= 4
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(d), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file"))));
#endif	
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file")), gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)));
	}
	gtk_widget_destroy(d);
#else	
	GtkWidget *d = gtk_file_selection_new(_("Select file to import"));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file")), gtk_file_selection_get_filename(GTK_FILE_SELECTION(d)));
	}
	gtk_widget_destroy(d);
#endif
}

void on_csv_export_optionmenu_delimiter_changed(GtkOptionMenu *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_entry_delimiter_other"), gtk_option_menu_get_history(w) == DELIMITER_OTHER);
}
void on_csv_export_button_file_clicked(GtkButton *button, gpointer user_data) {
#if GTK_MINOR_VERSION >= 3
	GtkWidget *d = gtk_file_chooser_dialog_new(_("Select file to export to"), GTK_WINDOW(glade_xml_get_widget(csvimport_glade, "csv_import_dialog")), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_MICRO_VERSION >= 6 || GTK_MINOR_VERSION >= 4
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(d), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file"))));
#endif	
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")), gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)));
	}
	gtk_widget_destroy(d);
#else	
	GtkWidget *d = gtk_file_selection_new(_("Select file to export to"));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")), gtk_file_selection_get_filename(GTK_FILE_SELECTION(d)));
	}
	gtk_widget_destroy(d);
#endif
}
void on_csv_export_radiobutton_current_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix"), !gtk_toggle_button_get_active(w));
}
void on_csv_export_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix"), gtk_toggle_button_get_active(w));
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
void on_type_label_file_clicked(GtkButton *w, gpointer user_data) {
#if GTK_MINOR_VERSION >= 3
	GtkWidget *d = gtk_file_chooser_dialog_new(_("Select file to import"), GTK_WINDOW(glade_xml_get_widget(csvimport_glade, "csv_import_dialog")), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_MICRO_VERSION >= 6 || GTK_MINOR_VERSION >= 4
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(d), gtk_entry_get_text(GTK_ENTRY(user_data)));
#endif
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
		gtk_entry_set_text(GTK_ENTRY(user_data), gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)));
	}
	gtk_widget_destroy(d);
#else	
	GtkWidget *d = gtk_file_selection_new(_("Select file to import"));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		gtk_entry_set_text(GTK_ENTRY(user_data), gtk_file_selection_get_filename(GTK_FILE_SELECTION(d)));
	}
	gtk_widget_destroy(d);
#endif
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

void on_function_edit_button_subfunctions_clicked(GtkButton *w, gpointer user_data) {
	gtk_window_set_transient_for(GTK_WINDOW(glade_xml_get_widget (functionedit_glade, "function_edit_dialog_subfunctions")), GTK_WINDOW(glade_xml_get_widget (functionedit_glade, "function_edit_dialog")));
	gtk_dialog_run(GTK_DIALOG(glade_xml_get_widget (functionedit_glade, "function_edit_dialog_subfunctions")));
	gtk_widget_hide(glade_xml_get_widget (functionedit_glade, "function_edit_dialog_subfunctions"));
}
void on_function_edit_button_add_subfunction_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeIter iter;	
	gtk_list_store_append(tSubfunctions_store, &iter);
	string str = "\\";
	last_subfunction_index++;
	str += i2s(last_subfunction_index);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_precalculate")))) {
		gtk_list_store_set(tSubfunctions_store, &iter, 0, str.c_str(), 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("yes"), 3, last_subfunction_index, 4, TRUE, -1);
	} else {
		gtk_list_store_set(tSubfunctions_store, &iter, 0, str.c_str(), 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("no"), 3, last_subfunction_index, 4, FALSE, -1);
	}
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression")), "");
}
void on_function_edit_button_modify_subfunction_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tSubfunctions)), &model, &iter)) {
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_precalculate")))) {
			gtk_list_store_set(tSubfunctions_store, &iter, 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("yes"), 4, TRUE, -1);
		} else {
			gtk_list_store_set(tSubfunctions_store, &iter, 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("no"), 4, FALSE, -1);
		}
	}
}
void on_function_edit_button_remove_subfunction_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tSubfunctions)), &model, &iter)) {
		GtkTreeIter iter2 = iter;
		while(gtk_tree_model_iter_next(GTK_TREE_MODEL(tSubfunctions_store), &iter2)) {
			guint index;
			gtk_tree_model_get(GTK_TREE_MODEL(tSubfunctions_store), &iter2, 3, &index, -1);
			index--;
			string str = "\\";
			str += i2s(index);
			gtk_list_store_set(tSubfunctions_store, &iter2, 0, str.c_str(), 3, index, -1);
		}
		gtk_list_store_remove(tSubfunctions_store, &iter);
		last_subfunction_index--;
	}
}
void on_function_edit_entry_subexpression_activate(GtkEntry *entry, gpointer user_data) {
	if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_subfunction"))) {
		on_function_edit_button_add_subfunction_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_subfunction")), NULL);
	} else if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_subfunction"))) {
		on_function_edit_button_modify_subfunction_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_subfunction")), NULL);
	}
}
void on_function_edit_button_add_argument_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeIter iter;	
	gtk_list_store_append(tFunctionArguments_store, &iter);
	Argument *arg;
	if(edited_function && edited_function->isBuiltin()) {
		arg = new Argument();
	} else {
		int menu_index = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (functionedit_glade, "function_edit_optionmenu_argument_type")));
		switch(menu_index) {
			case MENU_ARGUMENT_TYPE_TEXT: {arg = new TextArgument(); break;}
			case MENU_ARGUMENT_TYPE_SYMBOLIC: {arg = new SymbolicArgument(); break;}
			case MENU_ARGUMENT_TYPE_DATE: {arg = new DateArgument(); break;}
			case MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {arg = new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
			case MENU_ARGUMENT_TYPE_POSITIVE_INTEGER: {arg = new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
			case MENU_ARGUMENT_TYPE_NONZERO_INTEGER: {arg = new IntegerArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
			case MENU_ARGUMENT_TYPE_INTEGER: {arg = new IntegerArgument(); break;}
			case MENU_ARGUMENT_TYPE_NONNEGATIVE: {arg = new NumberArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
			case MENU_ARGUMENT_TYPE_POSITIVE: {arg = new NumberArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
			case MENU_ARGUMENT_TYPE_NONZERO: {arg = new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
			case MENU_ARGUMENT_TYPE_NUMBER: {arg = new NumberArgument(); break;}
			case MENU_ARGUMENT_TYPE_VECTOR: {arg = new VectorArgument(); break;}
			case MENU_ARGUMENT_TYPE_MATRIX: {arg = new MatrixArgument(); break;}
			case MENU_ARGUMENT_TYPE_EXPRESSION_ITEM: {arg = new ExpressionItemArgument(); break;}
			case MENU_ARGUMENT_TYPE_FUNCTION: {arg = new FunctionArgument(); break;}
			case MENU_ARGUMENT_TYPE_UNIT: {arg = new UnitArgument(); break;}
			case MENU_ARGUMENT_TYPE_VARIABLE: {arg = new VariableArgument(); break;}
			case MENU_ARGUMENT_TYPE_FILE: {arg = new FileArgument(); break;}
			case MENU_ARGUMENT_TYPE_BOOLEAN: {arg = new BooleanArgument(); break;}	
			case MENU_ARGUMENT_TYPE_ANGLE: {arg = new AngleArgument(); break;}	
			case MENU_ARGUMENT_TYPE_GIAC: {arg = new GiacArgument(); break;}	
			default: {arg = new Argument();}
		}
	}
	arg->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name"))));		
	gtk_list_store_set(tFunctionArguments_store, &iter, 0, arg->name().c_str(), 1, arg->printlong().c_str(), 2, arg, -1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name")), "");
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
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name")), "");
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
			int menu_index = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (functionedit_glade, "function_edit_optionmenu_argument_type")));
			switch(menu_index) {
				case MENU_ARGUMENT_TYPE_TEXT: {argtype = ARGUMENT_TYPE_TEXT; break;}
				case MENU_ARGUMENT_TYPE_SYMBOLIC: {argtype = ARGUMENT_TYPE_SYMBOLIC; break;}
				case MENU_ARGUMENT_TYPE_DATE: {argtype = ARGUMENT_TYPE_DATE; break;}
				case MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {}
				case MENU_ARGUMENT_TYPE_POSITIVE_INTEGER: {}
				case MENU_ARGUMENT_TYPE_NONZERO_INTEGER: {}
				case MENU_ARGUMENT_TYPE_INTEGER: {argtype = ARGUMENT_TYPE_INTEGER; break;}
				case MENU_ARGUMENT_TYPE_NONNEGATIVE: {}
				case MENU_ARGUMENT_TYPE_POSITIVE: {}
				case MENU_ARGUMENT_TYPE_NONZERO: {}
				case MENU_ARGUMENT_TYPE_NUMBER: {argtype = ARGUMENT_TYPE_NUMBER; break;}
				case MENU_ARGUMENT_TYPE_VECTOR: {argtype = ARGUMENT_TYPE_VECTOR; break;}
				case MENU_ARGUMENT_TYPE_MATRIX: {argtype = ARGUMENT_TYPE_MATRIX; break;}
				case MENU_ARGUMENT_TYPE_EXPRESSION_ITEM: {argtype = ARGUMENT_TYPE_EXPRESSION_ITEM; break;}
				case MENU_ARGUMENT_TYPE_FUNCTION: {argtype = ARGUMENT_TYPE_FUNCTION; break;}
				case MENU_ARGUMENT_TYPE_UNIT: {argtype = ARGUMENT_TYPE_UNIT; break;}
				case MENU_ARGUMENT_TYPE_VARIABLE: {argtype = ARGUMENT_TYPE_VARIABLE; break;}
				case MENU_ARGUMENT_TYPE_FILE: {argtype = ARGUMENT_TYPE_FILE; break;}
				case MENU_ARGUMENT_TYPE_BOOLEAN: {argtype = ARGUMENT_TYPE_BOOLEAN; break;}	
				case MENU_ARGUMENT_TYPE_ANGLE: {argtype = ARGUMENT_TYPE_ANGLE; break;}	
				case MENU_ARGUMENT_TYPE_GIAC: {argtype = ARGUMENT_TYPE_GIAC; break;}	
			}			
			
			if(!selected_argument || argtype != selected_argument->type() || menu_index == MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER || menu_index == MENU_ARGUMENT_TYPE_POSITIVE_INTEGER || menu_index == MENU_ARGUMENT_TYPE_NONZERO_INTEGER || menu_index == MENU_ARGUMENT_TYPE_NONZERO || menu_index == MENU_ARGUMENT_TYPE_POSITIVE || menu_index == MENU_ARGUMENT_TYPE_NONNEGATIVE) {
				if(selected_argument) {
					delete selected_argument;
				}
				switch(menu_index) {
					case MENU_ARGUMENT_TYPE_TEXT: {selected_argument = new TextArgument(); break;}
					case MENU_ARGUMENT_TYPE_SYMBOLIC: {selected_argument = new SymbolicArgument(); break;}
					case MENU_ARGUMENT_TYPE_DATE: {selected_argument = new DateArgument(); break;}
					case MENU_ARGUMENT_TYPE_NONNEGATIVE_INTEGER: {selected_argument = new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
					case MENU_ARGUMENT_TYPE_POSITIVE_INTEGER: {selected_argument = new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
					case MENU_ARGUMENT_TYPE_NONZERO_INTEGER: {selected_argument = new IntegerArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
					case MENU_ARGUMENT_TYPE_INTEGER: {selected_argument = new IntegerArgument(); break;}
					case MENU_ARGUMENT_TYPE_NONNEGATIVE: {selected_argument = new NumberArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE); break;}
					case MENU_ARGUMENT_TYPE_POSITIVE: {selected_argument = new NumberArgument("", ARGUMENT_MIN_MAX_POSITIVE); break;}
					case MENU_ARGUMENT_TYPE_NONZERO: {selected_argument = new NumberArgument("", ARGUMENT_MIN_MAX_NONZERO); break;}
					case MENU_ARGUMENT_TYPE_NUMBER: {selected_argument = new NumberArgument(); break;}
					case MENU_ARGUMENT_TYPE_VECTOR: {selected_argument = new VectorArgument(); break;}
					case MENU_ARGUMENT_TYPE_MATRIX: {selected_argument = new MatrixArgument(); break;}
					case MENU_ARGUMENT_TYPE_EXPRESSION_ITEM: {selected_argument = new ExpressionItemArgument(); break;}
					case MENU_ARGUMENT_TYPE_FUNCTION: {selected_argument = new FunctionArgument(); break;}
					case MENU_ARGUMENT_TYPE_UNIT: {selected_argument = new UnitArgument(); break;}
					case MENU_ARGUMENT_TYPE_VARIABLE: {selected_argument = new VariableArgument(); break;}
					case MENU_ARGUMENT_TYPE_FILE: {selected_argument = new FileArgument(); break;}
					case MENU_ARGUMENT_TYPE_BOOLEAN: {selected_argument = new BooleanArgument(); break;}	
					case MENU_ARGUMENT_TYPE_ANGLE: {selected_argument = new AngleArgument(); break;}
					case MENU_ARGUMENT_TYPE_GIAC: {selected_argument = new GiacArgument(); break;}	
					default: {selected_argument = new Argument();}
				}			
			}
			
		}
		selected_argument->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name"))));
		gtk_list_store_set(tFunctionArguments_store, &iter, 0, selected_argument->name().c_str(), 1, selected_argument->printlong().c_str(), 2, (gpointer) selected_argument, -1);
	}
}
void on_function_edit_entry_argument_name_activate(GtkEntry *entry, gpointer user_data) {
	if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_argument"))) {
		on_function_edit_button_add_argument_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_argument")), NULL);
	} else if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_argument"))) {
		on_function_edit_button_modify_argument_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_argument")), NULL);
	}
}
void on_function_edit_button_rules_clicked(GtkButton *w, gpointer user_data) {
	edit_argument(get_selected_argument());
}
void on_argument_rules_checkbutton_enable_min_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min"), gtk_toggle_button_get_active(w));
}
void on_argument_rules_checkbutton_enable_max_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max"), gtk_toggle_button_get_active(w));
}
void on_argument_rules_checkbutton_enable_condition_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_entry_condition"), gtk_toggle_button_get_active(w));
}

bool generate_plot(plot_parameters &pp, vector<MathStructure> &y_vectors, vector<MathStructure> &x_vectors, vector<plot_data_parameters*> &pdps) {
	GtkTreeIter iter;
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
	if(!b) {
		string expression = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")));
		if(!expression.empty()) {
			on_plot_button_add_clicked(GTK_BUTTON(glade_xml_get_widget (plot_glade, "plot_button_add")), NULL);
			b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
		} else {
			show_message(_("No functions defined."), glade_xml_get_widget(plot_glade, "plot_dialog"));
			return false;
		}
	}
	EvaluationOptions eo;
	eo.approximation = APPROXIMATION_APPROXIMATE;
	MathStructure min(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_min")))), eo));
	MathStructure max(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_max")))), eo));
	int steps = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (plot_glade, "plot_spinbutton_steps")));
	MathStructure x_vector;
	while(b) {
		x_vector.clearVector();
		int count = 1;
		gchar *gstr1, *gstr2;
		gint type = 0, style = 0, smoothing = 0, axis = 1, rows = 0;
		gtk_tree_model_get(GTK_TREE_MODEL(tPlotFunctions_store), &iter, 0, &gstr1, 1, &gstr2, 2, &style, 3, &smoothing, 4, &type, 5, &axis, 6, &rows, -1);
		if(type == 1) {
			MathStructure mstruct(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gstr2), eo));
			if(mstruct.isMatrix()) {
				count = 0;
				if(rows) {
					for(unsigned int i = 1; i <= mstruct.rows(); i++) {
						y_vectors.push_back(m_undefined);
						mstruct.rowToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						count++;
					}
				} else {
					for(unsigned int i = 1; i <= mstruct.columns(); i++) {
						y_vectors.push_back(m_undefined);
						mstruct.columnToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						count++;
					}
				}
			} else if(mstruct.isVector()) {
				y_vectors.push_back(mstruct);
				x_vectors.push_back(m_undefined);
			} else {
				mstruct.transform(STRUCT_VECTOR);
				y_vectors.push_back(mstruct);
				x_vectors.push_back(m_undefined);
			}
		} else if(type == 2) {
			MathStructure mstruct(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gstr2), eo));
			if(mstruct.isMatrix()) {
				count = 0;
				if(rows) {
					for(unsigned int i = 1; i <= mstruct.rows(); i += 2) {
						y_vectors.push_back(m_undefined);
						mstruct.rowToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						mstruct.rowToVector(i + 1, x_vectors[x_vectors.size() - 1]);
						count++;
					}
				} else {
					for(unsigned int i = 1; i <= mstruct.columns(); i += 2) {
						y_vectors.push_back(m_undefined);
						mstruct.columnToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						mstruct.columnToVector(i + 1, x_vectors[x_vectors.size() - 1]);
						count++;
					}
				}
			} else if(mstruct.isVector()) {
				y_vectors.push_back(mstruct);
				x_vectors.push_back(m_undefined);
			} else {
				mstruct.transform(STRUCT_VECTOR);
				y_vectors.push_back(mstruct);
				x_vectors.push_back(m_undefined);
			}
		} else {
			y_vectors.push_back(CALCULATOR->expressionToPlotVector(gstr2, min, max, steps, &x_vector, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")))));
			x_vectors.push_back(x_vector);
		}
		for(int i = 0; i < count; i++) {
			plot_data_parameters *pdp = new plot_data_parameters();
			pdp->title = gstr1;
			if(count > 1) {
				pdp->title += " :";
				pdp->title += i2s(i + 1);
			}
			remove_blank_ends(pdp->title);
			if(pdp->title.empty()) {
				pdp->title = gstr2;
			}
			switch(smoothing) {
				case SMOOTHING_MENU_NONE: {pdp->smoothing = PLOT_SMOOTHING_NONE; break;}
				case SMOOTHING_MENU_UNIQUE: {pdp->smoothing = PLOT_SMOOTHING_UNIQUE; break;}
				case SMOOTHING_MENU_CSPLINES: {pdp->smoothing = PLOT_SMOOTHING_CSPLINES; break;}
				case SMOOTHING_MENU_BEZIER: {pdp->smoothing = PLOT_SMOOTHING_BEZIER; break;}
				case SMOOTHING_MENU_SBEZIER: {pdp->smoothing = PLOT_SMOOTHING_SBEZIER; break;}
			}
			switch(style) {
				case PLOTSTYLE_MENU_LINES: {pdp->style = PLOT_STYLE_LINES; break;}
				case PLOTSTYLE_MENU_POINTS: {pdp->style = PLOT_STYLE_POINTS; break;}
				case PLOTSTYLE_MENU_LINESPOINTS: {pdp->style = PLOT_STYLE_POINTS_LINES; break;}
				case PLOTSTYLE_MENU_DOTS: {pdp->style = PLOT_STYLE_DOTS; break;}
				case PLOTSTYLE_MENU_BOXES: {pdp->style = PLOT_STYLE_BOXES; break;}
				case PLOTSTYLE_MENU_HISTEPS: {pdp->style = PLOT_STYLE_HISTOGRAM; break;}
				case PLOTSTYLE_MENU_STEPS: {pdp->style = PLOT_STYLE_STEPS; break;}
				case PLOTSTYLE_MENU_CANDLESTICKS: {pdp->style = PLOT_STYLE_CANDLESTICKS; break;}
			}
			pdp->yaxis2 = (axis == 2);
			pdps.push_back(pdp);
		}
		g_free(gstr1);
		g_free(gstr2);
		b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
	}
	switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_legend_place")))) {
		case PLOTLEGEND_MENU_NONE: {pp.legend_placement = PLOT_LEGEND_NONE; break;}
		case PLOTLEGEND_MENU_TOP_LEFT: {pp.legend_placement = PLOT_LEGEND_TOP_LEFT; break;}
		case PLOTLEGEND_MENU_TOP_RIGHT: {pp.legend_placement = PLOT_LEGEND_TOP_RIGHT; break;}
		case PLOTLEGEND_MENU_BOTTOM_LEFT: {pp.legend_placement = PLOT_LEGEND_BOTTOM_LEFT; break;}
		case PLOTLEGEND_MENU_BOTTOM_RIGHT: {pp.legend_placement = PLOT_LEGEND_BOTTOM_RIGHT; break;}
		case PLOTLEGEND_MENU_BELOW: {pp.legend_placement = PLOT_LEGEND_BELOW; break;}
		case PLOTLEGEND_MENU_OUTSIDE: {pp.legend_placement = PLOT_LEGEND_OUTSIDE; break;}
	}
	pp.title = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_plottitle")));
	pp.x_label = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_xlabel")));
	pp.y_label = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_ylabel")));
	pp.grid = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_grid")));
	pp.x_log = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_xlog")));
	pp.y_log = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_ylog")));
	pp.x_log_base = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (plot_glade, "plot_spinbutton_xlog_base")));
	pp.y_log_base = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (plot_glade, "plot_spinbutton_ylog_base")));
	pp.color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_color")));
	pp.show_all_borders = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_full_border")));
	return true;
}
void on_plot_button_save_clicked(GtkButton *w, gpointer user_data) {
	GtkWidget *d;
#if GTK_MINOR_VERSION >= 3
	d = gtk_file_chooser_dialog_new(_("Select file to export"), GTK_WINDOW(glade_xml_get_widget(plot_glade, "plot_dialog")), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Allowed File Types"));
	gtk_file_filter_add_mime_type(filter, "image/x-xfig");
	gtk_file_filter_add_mime_type(filter, "image/svg");
	gtk_file_filter_add_mime_type(filter, "text/x-tex");
	gtk_file_filter_add_mime_type(filter, "application/postscript");
	gtk_file_filter_add_mime_type(filter, "image/x-eps");
	gtk_file_filter_add_mime_type(filter, "image/png");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(d), filter);
	GtkFileFilter *filter_all = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter_all, "*");
	gtk_file_filter_set_name(filter_all, _("All Files"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(d), filter_all);
	string title = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_plottitle")));
	if(title.empty()) {
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(d), "plot.png");
	} else {
		title += ".png";
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(d), title.c_str());
	}
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
#else	
	d = gtk_file_selection_new(_("Select file to export"));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
#endif	
		vector<MathStructure> y_vectors;
		vector<MathStructure> x_vectors;
		vector<plot_data_parameters*> pdps;
		plot_parameters pp;
		if(generate_plot(pp, y_vectors, x_vectors, pdps)) {
#if GTK_MINOR_VERSION >= 3			
			pp.filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
#else		
			pp.filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(d));
#endif			
			pp.filetype = PLOT_FILETYPE_AUTO;
			CALCULATOR->plotVectors(&pp, y_vectors, x_vectors, pdps);
			for(unsigned int i = 0; i < pdps.size(); i++) {
				if(pdps[i]) delete pdps[i];
			}
		}
	}
	gtk_widget_destroy(d);
}
void update_plot() {
	vector<MathStructure> y_vectors;
	vector<MathStructure> x_vectors;
	vector<plot_data_parameters*> pdps;
	plot_parameters pp;
	if(!generate_plot(pp, y_vectors, x_vectors, pdps)) {
		return;
	}
	CALCULATOR->plotVectors(&pp, y_vectors, x_vectors, pdps);
	for(unsigned int i = 0; i < pdps.size(); i++) {
		if(pdps[i]) delete pdps[i];
	}
}
void on_plot_button_apply_clicked(GtkButton *w, gpointer user_data) {
	update_plot();
}
void on_plot_button_add_clicked(GtkButton *w, gpointer user_data) {
	string expression = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")));
	if(expression.empty()) {
		show_message(_("Empty expression."), glade_xml_get_widget(plot_glade, "plot_dialog"));
		return;
	}
	GtkTreeIter iter;	
	gtk_list_store_append(tPlotFunctions_store, &iter);
	gint type = 0, axis = 1, rows = 0;
	string title = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_title")));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_vector")))) {
		type = 1;
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_paired")))) {
		type = 2;
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_yaxis2")))) {
		axis = 2;
	}
	if((type == 1 || type == 2) && title.empty()) {
		Variable *v = CALCULATOR->getActiveVariable(expression);
		if(v) {
			title = v->title(false);
		}
	}
	rows = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")));
	gtk_list_store_set(tPlotFunctions_store, &iter, 0, title.c_str(), 1, expression.c_str(), 2, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style"))), 3, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing"))), 4, type, 5, axis, 6, rows, -1);
	update_plot();
}
void on_plot_button_modify_clicked(GtkButton *w, gpointer user_data) {

	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {	
		string expression = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")));
		if(expression.empty()) {
			show_message(_("Empty expression."), glade_xml_get_widget(plot_glade, "plot_dialog"));
			return;
		}
		gint type = 0, axis = 1, rows = 0;
		string title = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_title")));
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_vector")))) {
			type = 1;
		} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_paired")))) {
			type = 2;
		}
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_yaxis2")))) {
			axis = 2;
		}
		if((type == 1 || type == 2) && title.empty()) {
			Variable *v = CALCULATOR->getActiveVariable(expression);
			if(v) {
				title = v->title(false);
			}
		}
		rows = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")));
		gtk_list_store_set(tPlotFunctions_store, &iter, 0, title.c_str(), 1, expression.c_str(), 2, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style"))), 3, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing"))), 4, type, 5, axis, 6, rows, -1);
	}
	update_plot();
}
void on_plot_button_remove_clicked(GtkButton *w, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		gtk_list_store_remove(tPlotFunctions_store, &iter);
	}
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_title")), "");	
	update_plot();
}

void on_plot_checkbutton_xlog_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_spinbutton_xlog_base"), gtk_toggle_button_get_active(w));
}
void on_plot_checkbutton_ylog_toggled(GtkToggleButton *w, gpointer user_data) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_spinbutton_ylog_base"), gtk_toggle_button_get_active(w));
}
void on_plot_entry_expression_activate(GtkEntry *entry, gpointer user_data) {
	on_plot_button_add_clicked(GTK_BUTTON(glade_xml_get_widget (plot_glade, "plot_button_add")), NULL);
}

void on_unit_dialog_button_ok_clicked(GtkButton *w, gpointer user_data) {
	*mstruct = CALCULATOR->convert(*mstruct, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit"))), evalops);
	setResult();
	gtk_widget_hide(glade_xml_get_widget (unit_glade, "unit_dialog"));
	gtk_widget_grab_focus(expression);
}
void on_unit_dialog_button_apply_clicked(GtkButton *w, gpointer user_data) {
	*mstruct = CALCULATOR->convert(*mstruct, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit"))), evalops);
	setResult();
}
void on_unit_dialog_entry_unit_activate(GtkEntry *entry, gpointer user_data) {
	on_unit_dialog_button_ok_clicked(GTK_BUTTON(glade_xml_get_widget (unit_glade, "unit_dialog_button_ok")), NULL);
}

#ifdef __cplusplus
}
#endif
