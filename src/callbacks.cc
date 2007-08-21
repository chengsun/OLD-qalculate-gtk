/*
    Qalculate

    Copyright (C) 2003-2007  Niklas Knutsson (nq@altern.org)

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
#include <dirent.h>
#include <pthread.h>
#include <glade/glade.h>
#ifdef HAVE_LIBGNOME
#include <libgnome/libgnome.h>
#endif

#include "support.h"
#include "callbacks.h"
#include "interface.h"
#include "main.h"
#include <stack>
#include <deque>

extern bool do_timeout, check_expression_position;
extern gint expression_position;

extern GladeXML *main_glade, *about_glade, *argumentrules_glade, *csvimport_glade, *csvexport_glade, *setbase_glade, *datasetedit_glade, *datasets_glade, *decimals_glade;
extern GladeXML *functionedit_glade, *functions_glade, *matrixedit_glade, *matrix_glade, *namesedit_glade, *nbases_glade, *plot_glade, *precision_glade;
extern GladeXML *preferences_glade, *unit_glade, *unitedit_glade, *units_glade, *unknownedit_glade, *variableedit_glade, *variables_glade;
extern GladeXML *periodictable_glade;

bool changing_in_nbases_dialog;

extern GtkWidget *tabs, *expander_keypad, *expander_history, *expander_stack;
extern GtkEntryCompletion *completion;
extern GtkListStore *completion_store;

extern GtkTooltips *main_tooltips;
extern GtkWidget *expression, *statuslabel_l, *statuslabel_r;
extern GtkWidget *f_menu, *v_menu, *u_menu, *u_menu2, *recent_menu;
extern KnownVariable *vans[5];
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
extern GtkWidget *tUnitSelectorCategories;
extern GtkWidget *tUnitSelector;
extern GtkListStore *tUnitSelector_store;
extern GtkTreeStore *tUnitSelectorCategories_store;
extern GtkWidget *tDataObjects, *tDatasets;
extern GtkListStore *tDataObjects_store, *tDatasets_store;
extern GtkWidget *tDataProperties;
extern GtkListStore *tDataProperties_store;
extern GtkWidget *tNames;
extern GtkListStore *tNames_store;
extern GtkAccelGroup *accel_group;
extern string selected_function_category;
extern MathFunction *selected_function;
DataObject *selected_dataobject = NULL;
DataSet *selected_dataset = NULL;
DataProperty *selected_dataproperty = NULL;
MathFunction *edited_function = NULL;
KnownVariable *edited_variable = NULL;
UnknownVariable *edited_unknown = NULL;
KnownVariable *edited_matrix = NULL;
Unit *edited_unit = NULL;
DataSet *edited_dataset = NULL;
DataProperty *edited_dataproperty = NULL;
bool editing_variable = false, editing_unknown = false, editing_matrix = false, editing_unit = false, editing_function = false, editing_dataset = false, editing_dataproperty = false;
size_t selected_subfunction;
size_t last_subfunction_index;
Argument *selected_argument;
Argument *edited_argument;
extern string selected_variable_category;
extern Variable *selected_variable;
extern string selected_unit_category;
extern string selected_unit_selector_category;
extern Unit *selected_unit;
extern Unit *selected_to_unit;
bool save_mode_on_exit;
bool save_defs_on_exit;
bool use_custom_result_font, use_custom_expression_font, use_custom_status_font;
string custom_result_font, custom_expression_font, custom_status_font, wget_args;
bool hyp_is_on, inv_is_on;
bool show_keypad, show_history, show_stack;
extern bool load_global_defs, fetch_exchange_rates_at_startup, first_time, first_qalculate_run;
bool display_expression_status, enable_completion;
extern GtkWidget *omToUnit_menu;
bool block_unit_convert, block_unit_selector_convert;
extern MathStructure *mstruct, *matrix_mstruct, *parsed_mstruct, *parsed_tostruct;
extern bool prev_result_approx;
extern string result_text, parsed_text;
extern GtkWidget *resultview;
extern GtkWidget *historyview;
extern GtkWidget *stackview;
extern GtkListStore *stackstore;
extern GtkCellRenderer *register_renderer;
extern GtkTreeViewColumn *register_column;
extern GdkPixmap *pixmap_result;
extern GdkPixbuf *pixbuf_result;
vector<vector<GtkWidget*> > insert_element_entries;
bool b_busy;
GdkPixmap *tmp_pixmap;
bool expression_has_changed = false, current_object_has_changed = false, expression_has_changed2 = false;
bool block_result_update = false, block_expression_execution = false, block_display_parse = false;
string parsed_expression;
bool parsed_had_errors = false, parsed_had_warnings = false;
vector<DataProperty*> tmp_props;
vector<DataProperty*> tmp_props_orig;

extern GtkWidget *tMatrixEdit, *tMatrix;
extern GtkListStore *tMatrixEdit_store, *tMatrix_store;
vector<GtkTreeViewColumn*> matrix_edit_columns, matrix_columns;

extern GtkAccelGroup *accel_group;

extern gint win_height, win_width;

vector<string> expression_history;
int expression_history_index = -1;
bool dont_change_index = false;

PlotLegendPlacement default_plot_legend_placement = PLOT_LEGEND_TOP_RIGHT;
bool default_plot_display_grid = true;
bool default_plot_full_border = false;
string default_plot_min = "0";
string default_plot_max = "10";
string default_plot_step = "1";
int default_plot_sampling_rate = 100;
bool default_plot_use_sampling_rate = true;
bool default_plot_rows = false;
int default_plot_type = 0;
PlotStyle default_plot_style = PLOT_STYLE_LINES;
PlotSmoothing default_plot_smoothing = PLOT_SMOOTHING_NONE;
string default_plot_variable = "x";
bool default_plot_color = true;

string status_error_color, status_warning_color;

bool names_edited = false;

gint current_object_start = -1, current_object_end = -1;
bool stop_timeouts = false;

PrintOptions printops, parse_printops;
EvaluationOptions evalops;

bool rpn_mode, rpn_keypad_only;

extern FILE *view_pipe_r, *view_pipe_w, *command_pipe_r, *command_pipe_w;
extern pthread_t view_thread, command_thread;
extern pthread_attr_t view_thread_attr, command_thread_attr;
bool exit_in_progress = false, command_aborted = false;
extern bool command_thread_started;

vector<mode_struct> modes;
vector<GtkWidget*> mode_items;
vector<GtkWidget*> popup_result_mode_items;

Sgi::hash_map<int, GdkPixbuf*> left_par_pix;
Sgi::hash_map<int, GdkPixbuf*> right_par_pix;
GdkPixbuf *pixbuf_left_par = NULL;
GdkPixbuf *pixbuf_right_par = NULL;

deque<string> inhistory;
deque<int> inhistory_type;

GtkTextMark *initial_result_pos = NULL;
int initial_result_index = 0;

#define TEXT_TAGS			"<span size=\"xx-large\">"
#define TEXT_TAGS_END			"</span>"
#define TEXT_TAGS_SMALL			"<span size=\"large\">"
#define TEXT_TAGS_SMALL_END		"</span>"
#define TEXT_TAGS_XSMALL		"<span size=\"medium\">"
#define TEXT_TAGS_XSMALL_END		"</span>"

#define TTB(str)			if(scaledown <= 0) {str += "<span size=\"xx-large\">";} else if(scaledown == 1) {str += "<span size=\"x-large\">";} else if(scaledown == 2) {str += "<span size=\"large\">";} else {str += "<span size=\"medium\">";}
#define TTB_SMALL(str)			if(scaledown <= 0) {str += "<span size=\"large\">";} else if(scaledown == 1) {str += "<span size=\"medium\">";} else if(scaledown == 2) {str += "<span size=\"small\">";} else {str += "<span size=\"x-small\">";}
#define TTB_XSMALL(str)			if(scaledown <= 0) {str += "<span size=\"medium\">";} else if(scaledown == 1) {str += "<span size=\"small\">";} else {str += "<span size=\"x-small\">";}
#define TTBP(str)			if(ips.power_depth > 0) {TTB_SMALL(str);} else {TTB(str);}
#define TTBP_SMALL(str)			if(ips.power_depth > 0) {TTB_XSMALL(str);} else {TTB_SMALL(str);}
#define TTE(str)			str += "</span>";
#define TT(str, x)			{if(scaledown <= 0) {str += "<span size=\"xx-large\">";} else if(scaledown == 1) {str += "<span size=\"x-large\">";} else if(scaledown == 2) {str += "<span size=\"large\">";} else {str += "<span size=\"medium\">";} str += x; str += "</span>";}
#define TT_SMALL(str, x)		{if(scaledown <= 0) {str += "<span size=\"large\">";} else if(scaledown == 1) {str += "<span size=\"medium\">";} else if(scaledown == 2) {str += "<span size=\"small\">";} else {str += "<span size=\"x-small\">";} str += x; str += "</span>";}
#define TT_XSMALL(str, x)		{if(scaledown <= 0) {str += "<span size=\"medium\">";} else if(scaledown == 1) {str += "<span size=\"small\">";} else {str += "<span size=\"x-small\">";} str += x; str += "</span>";}
#define TTP(str, x)			if(ips.power_depth > 0) {TT_SMALL(str, x);} else {TT(str, x);}
#define TTP_SMALL(str, x)		if(ips.power_depth > 0) {TT_XSMALL(str, x);} else {TT_SMALL(str, x);}

#define PANGO_TT(layout, x)		if(scaledown <= 0) {pango_layout_set_markup(layout, "<span size=\"xx-large\">" x "</span>", -1);} else if(scaledown == 1) {pango_layout_set_markup(layout, "<span size=\"x-large\">" x "</span>", -1);} else if(scaledown == 2) {pango_layout_set_markup(layout, "<span size=\"large\">" x "</span>", -1);} else {pango_layout_set_markup(layout, "<span size=\"medium\">" x "</span>", -1);}
#define PANGO_TT_SMALL(layout, x)	if(scaledown <= 0) {pango_layout_set_markup(layout, "<span size=\"large\">" x "</span>", -1);} else if(scaledown == 1) {pango_layout_set_markup(layout, "<span size=\"medium\">" x "</span>", -1);} else if(scaledown == 1) {pango_layout_set_markup(layout, "<span size=\"medium\">" x "</span>", -1);} else {pango_layout_set_markup(layout, "<span size=\"x-small\">" x "</span>", -1);}
#define PANGO_TT_XSMALL(layout, x)	if(scaledown <= 0) {pango_layout_set_markup(layout, "<span size=\"medium\">" x "</span>", -1);} else if(scaledown == 1) {pango_layout_set_markup(layout, "<span size=\"small\">" x "</span>", -1);} else {pango_layout_set_markup(layout, "<span size=\"x-small\">" x "</span>", -1);}
#define PANGO_TTP(layout, x)		if(ips.power_depth > 0) {PANGO_TT_SMALL(layout, x);} else {PANGO_TT(layout, x);}
#define PANGO_TTP_SMALL(layout, x)	if(ips.power_depth > 0) {PANGO_TT_XSMALL(layout, x);} else {PANGO_TT_SMALL(layout, x);}

#define CALCULATE_SPACE_W		gint space_w, space_h; PangoLayout *layout_space = gtk_widget_create_pango_layout(resultview, NULL); PANGO_TTP(layout_space, " "); pango_layout_get_pixel_size(layout_space, &space_w, &space_h); g_object_unref(layout_space);

enum {
	COMMAND_FACTORIZE,
	COMMAND_SIMPLIFY
};

void set_assumptions_items(AssumptionType, AssumptionSign);
void set_mode_items(const PrintOptions&, const EvaluationOptions&, AssumptionType, AssumptionSign, bool, int, bool);

void clear_parenthesis_pix() {
	if(pixbuf_left_par) {
		gdk_pixbuf_unref(pixbuf_left_par);
		pixbuf_left_par = NULL;
	}
	if(pixbuf_right_par) {
		gdk_pixbuf_unref(pixbuf_right_par);
		pixbuf_right_par = NULL;
	}
	for(Sgi::hash_map<int, GdkPixbuf*>::iterator it = left_par_pix.begin(); it != left_par_pix.end(); ++it) {
		gdk_pixbuf_unref(it->second);
	}
	left_par_pix.clear();
	for(Sgi::hash_map<int, GdkPixbuf*>::iterator it = right_par_pix.begin(); it != right_par_pix.end(); ++it) {
		gdk_pixbuf_unref(it->second);
	}
	right_par_pix.clear();
}
void result_font_modified() {
	set_result_size_request();
	clear_parenthesis_pix();
	result_display_updated();
}


PangoCoverageLevel get_least_coverage(const gchar *gstr, GtkWidget *widget) {

	PangoCoverageLevel level = PANGO_COVERAGE_EXACT;
	PangoContext *context = gtk_widget_get_pango_context(widget);
	PangoLanguage *language = pango_context_get_language(context);
	PangoFontset *fontset = pango_context_load_fontset(context, gtk_widget_get_style(widget)->font_desc, language);
	while(gstr[0] != '\0') {
		if(gstr[0] < 0) {
			gunichar gu = g_utf8_get_char_validated(gstr, -1);
			if(gu != (gunichar) -1 && gu != (gunichar) -2) {
				PangoFont *font = pango_fontset_get_font(fontset, (guint) gu);
				if(font) {
					PangoCoverage *coverage = pango_font_get_coverage(font, language);
					if(pango_coverage_get(coverage, (int) gu) < level) {
						level = pango_coverage_get(coverage, gu);
					}
					g_object_unref(font);
					pango_coverage_unref(coverage);
				} else {
					level = PANGO_COVERAGE_NONE;
				}
			}
		}
		gstr = g_utf8_find_next_char(gstr, NULL);
		if(!gstr) break;
	}
	g_object_unref(fontset);
	return level;

}

bool can_display_unicode_string_function(const char *str, void *w) {
	if(!w) w = (void*) historyview;
	return get_least_coverage(str, (GtkWidget*) w) >= PANGO_COVERAGE_APPROXIMATE;
}

void set_result_size_request() {
	MathStructure mtest, mtest2;
	mtest.setType(STRUCT_DIVISION);
	mtest.addChild(1);
	mtest2.setType(STRUCT_POWER);
	mtest2.addChild(1);
	mtest2.addChild(1);
	mtest.addChild(mtest2);
	mtest.format();
	PrintOptions po;
	po.can_display_unicode_string_function = &can_display_unicode_string_function;
	po.can_display_unicode_string_arg = (void*) resultview;
	GdkPixmap *tmp_pixmap = draw_structure(mtest, po);
	gint w, h;
	gdk_drawable_get_size(GDK_DRAWABLE(tmp_pixmap), &w, &h);
	h += 8;
	g_object_unref(tmp_pixmap);
	gtk_widget_set_size_request(glade_xml_get_widget (main_glade, "scrolled_result"), -1, h);
}

void set_unicode_buttons() {
	if(printops.use_unicode_signs) {
		if(can_display_unicode_string_function(SIGN_MINUS, (void*) glade_xml_get_widget (main_glade, "button_sub"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sub")), SIGN_MINUS);
		else gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sub")), MINUS);
		if(can_display_unicode_string_function(SIGN_PLUS, (void*) glade_xml_get_widget (main_glade, "button_add"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_add")), SIGN_PLUS);
		else gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_add")), PLUS);
		if(can_display_unicode_string_function(SIGN_MULTIPLICATION, (void*) glade_xml_get_widget (main_glade, "button_times"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_times")), SIGN_MULTIPLICATION);
		else gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_times")), MULTIPLICATION);	
		if(can_display_unicode_string_function(SIGN_DIVISION_SLASH, (void*) glade_xml_get_widget (main_glade, "button_divide"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_divide")), SIGN_DIVISION_SLASH);	
		else if(can_display_unicode_string_function(SIGN_DIVISION, (void*) glade_xml_get_widget (main_glade, "button_divide"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_divide")), SIGN_DIVISION);
		else gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_divide")), DIVISION);	
		if(can_display_unicode_string_function(SIGN_SQRT, (void*) glade_xml_get_widget (main_glade, "button_sqrt"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sqrt")), SIGN_SQRT);	
		else gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sqrt")), "sqrt");	
		if(can_display_unicode_string_function(SIGN_MULTIDOT, (void*) glade_xml_get_widget (main_glade, "button_dot"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_dot")), SIGN_MULTIDOT);
		else gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_dot")), CALCULATOR->getDecimalPoint().c_str());
	} else {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sub")), MINUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_add")), PLUS);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_times")), MULTIPLICATION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_divide")), DIVISION);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_sqrt")), "sqrt");	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (main_glade, "button_dot")), CALCULATOR->getDecimalPoint().c_str());
	}
}


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
vector<MathFunction*> recent_functions;
vector<Variable*> recent_variables;
vector<Unit*> recent_units;

bool completion_blocked = false;

void block_completion() {
	gtk_entry_completion_set_minimum_key_length(completion, 1000);
	completion_blocked = true;
}
void unblock_completion() {
}

bool is_answer_variable(Variable *v) {
	return v == vans[0] || v == vans[1] || v == vans[2] || v == vans[3] || v == vans[4];
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

#define STATUS_SPACE	if(b) str += "  "; else b = true;

void set_status_text(string text, bool break_begin = false, bool had_errors = false, bool had_warnings = false) {

	string str;
	if(had_errors) {
		str = "<span size=\"small\" foreground=\"";
		str += status_error_color;
		str += "\">";
	} else if(had_warnings) {
		str = "<span size=\"small\" foreground=\"";
		str += status_warning_color;
		str += "\">";
	} else {
		str = "<span size=\"small\">";
	}
	if(text.empty()) str += " ";
	else str += text;
	str += "</span>";

#if GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION < 8
	gint w, h;
	int l = 0;
	while(true) {
		PangoLayout *layout_test = gtk_widget_create_pango_layout(statuslabel_l, NULL);
		pango_layout_set_markup(layout_test, str.c_str(), -1);
		pango_layout_get_pixel_size(layout_test, &w, &h);
		g_object_unref(layout_test);
		if(w < statuslabel_l->allocation.width - 20) {
			break;
		}
		if(l == 0) {
			l = ((text.length() * statuslabel_l->allocation.width - 20) / w) - 3;
			if(l > (int) text.length() - 3) l = text.length() - 3;
		} else {
			l -= 6;
		}
		if(l <= 0) {
			set_status_text("", break_begin, had_errors, had_warnings);
			return;
		}
		if(had_errors) {
			str = "<span size=\"small\" foreground=\"";
			str += status_error_color;
			str += "\">";
		} else if(had_warnings) {
			str = "<span size=\"small\" foreground=\"";
			str += status_warning_color;
			str += "\">";
		} else {
			str = "<span size=\"small\">";
		}
		if(break_begin) {
			int i = 0;
			while(i < 5 && text.length() - l > 0 && (unsigned char) text[text.length() - l] >= 0x80 && (unsigned char) text[text.length() - l] <= 0xBF) {
				l++;
				i++;
			}
			text = text.substr(text.length() - l, l);
			str += "...";
			str += text;
		} else {
			size_t b_begin, b_end;
			bool add_bold_end = false;
			b_begin = text.find("<b>");
			if(b_begin != string::npos) {
				b_end = text.find("</b>", b_begin);
			}
			if(b_begin != string::npos && l > (int) b_begin && l < (int) b_end + 4) {
				if(l > (int) b_end) l = (int) b_end + 4;
				else if(l < (int) b_begin + 4) l = (int) b_begin;
				else add_bold_end = true;
			}
			if(l == 0) {
				set_status_text("", break_begin, had_errors, had_warnings);
				return;
			}
			int i = 0;
			while(i < 5 && l + 1 < (int) text.length() && (unsigned char) text[l + 1] >= 0x80 && (unsigned char) text[l + 1] <= 0xBF) {
				l++;
				i++;
			}
			text = text.substr(0, l);
			if(add_bold_end) text += "</b>";
			str += text;
			str += "...";
		}
		str += "</span>";
	}
#else
	if(break_begin) gtk_label_set_ellipsize(GTK_LABEL(statuslabel_l), PANGO_ELLIPSIZE_START);
	else gtk_label_set_ellipsize(GTK_LABEL(statuslabel_l), PANGO_ELLIPSIZE_END);
#endif
	
	gtk_label_set_markup(GTK_LABEL(statuslabel_l), str.c_str());
	
}

void display_parse_status();

void update_status_text() {

	string str = "<span size=\"x-small\">";
	
	bool b = false;
	if(evalops.approximation == APPROXIMATION_EXACT) {
		STATUS_SPACE
		str += _("EXACT");
	} else if(evalops.approximation == APPROXIMATION_APPROXIMATE) {
		STATUS_SPACE
		str += _("APPROX");
	}
	if(evalops.parse_options.rpn) {
		STATUS_SPACE
		str += _("RPN");
	}
	switch(evalops.parse_options.base) {
		case BASE_DECIMAL: {
			break;
		}
		case BASE_BINARY: {
			STATUS_SPACE
			str += _("BIN");
			break;
		}
		case BASE_OCTAL: {
			STATUS_SPACE
			str += _("OCT");
			break;
		}
		case BASE_HEXADECIMAL: {
			STATUS_SPACE
			str += _("HEX");
			break;
		}
		case BASE_ROMAN_NUMERALS: {
			STATUS_SPACE
			str += _("ROMAN");
			break;
		}
		default: {
			STATUS_SPACE
			str += i2s(evalops.parse_options.base);
			break;
		}
	}
	switch (evalops.parse_options.angle_unit) {
		case ANGLE_UNIT_DEGREES: {
			STATUS_SPACE
			str += _("DEG");
			break;
		}
		case ANGLE_UNIT_RADIANS: {
			STATUS_SPACE
			str += _("RAD");
			break;
		}
		case ANGLE_UNIT_GRADIANS: {
			STATUS_SPACE
			str += _("GRA");
			break;
		}
		default: {}
	}
	if(evalops.parse_options.read_precision != DONT_READ_PRECISION) {
		STATUS_SPACE
		str += _("PREC");
	}
	if(!evalops.parse_options.functions_enabled) {
		STATUS_SPACE
		str += "<s>";
		str += _("FUNC");
		str += "</s>";
	}
	if(!evalops.parse_options.units_enabled) {
		STATUS_SPACE
		str += "<s>";
		str += _("UNIT");
		str += "</s>";
	}
	if(!evalops.parse_options.variables_enabled) {
		STATUS_SPACE
		str += "<s>";
		str += _("VAR");
		str += "</s>";
	}
	if(!evalops.allow_infinite) {
		STATUS_SPACE
		str += "<s>";
		str += _("INF");
		str += "</s>";
	}
	if(!evalops.allow_complex) {
		STATUS_SPACE
		str += "<s>";
		str += _("CPLX");
		str += "</s>";
	}

	remove_blank_ends(str);
	if(!b) str += " ";
	
	str += "</span>";

	if(str != gtk_label_get_label(GTK_LABEL(statuslabel_r))) {
		gtk_label_set_text(GTK_LABEL(statuslabel_l), "");	
		gtk_label_set_markup(GTK_LABEL(statuslabel_r), str.c_str());	
		display_parse_status();
	}
	
}


/*
	display errors generated under calculation
*/
void display_errors(GtkTextIter *iter = NULL, GtkWidget *win = NULL, int *inhistory_index = NULL) {
	if(!CALCULATOR->message()) return;
	bool b = false, critical = false;
	MessageType mtype;
	GtkWidget *edialog;
	string str = "";
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history")));
	while(true) {
		mtype = CALCULATOR->message()->type();
		if(mtype == MESSAGE_ERROR || mtype == MESSAGE_WARNING) {
			if(b) {
				str += "\n";
			} else {
				b = true;
			}
			str += CALCULATOR->message()->message();
			if(iter && inhistory_index) {
				if(mtype == MESSAGE_ERROR) {
					critical = true;
					inhistory.insert(inhistory.begin() + *inhistory_index, CALCULATOR->message()->message());
					inhistory_type.insert(inhistory_type.begin() + *inhistory_index, QALCULATE_HISTORY_ERROR);
					gtk_text_buffer_insert_with_tags_by_name(tb, iter, "- ", -1, "history_error", NULL);
					gtk_text_buffer_insert_with_tags_by_name(tb, iter, CALCULATOR->message()->message().c_str(), -1, "history_error", NULL);
				} else if(mtype == MESSAGE_WARNING) {
					inhistory.insert(inhistory.begin() + *inhistory_index, CALCULATOR->message()->message());
					inhistory_type.insert(inhistory_type.begin() + *inhistory_index, QALCULATE_HISTORY_WARNING);
					gtk_text_buffer_insert_with_tags_by_name(tb, iter, "- ", -1, "history_warning", NULL);
					gtk_text_buffer_insert_with_tags_by_name(tb, iter, CALCULATOR->message()->message().c_str(), -1, "history_warning", NULL);
				}
				*inhistory_index += 1;
				gtk_text_buffer_insert(tb, iter, "\n", -1);
				gtk_text_buffer_place_cursor(tb, iter);
			}
		} else {
			edialog = gtk_message_dialog_new(
					GTK_WINDOW(win),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_CLOSE,
					CALCULATOR->message()->message().c_str());
			gtk_dialog_run(GTK_DIALOG(edialog));
			gtk_widget_destroy(edialog);
		}
		if(!CALCULATOR->nextMessage()) break;
	}
	if(!str.empty()) {
		if(critical) {
			edialog = gtk_message_dialog_new(
					GTK_WINDOW(win),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					str.c_str());
		} else {
			edialog = gtk_message_dialog_new(
					GTK_WINDOW(win),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE,
					str.c_str());
		}

		gtk_dialog_run(GTK_DIALOG(edialog));
		gtk_widget_destroy(edialog);
	}
}

gboolean on_display_errors_timeout(gpointer) {
	if(stop_timeouts) return false;
	if(!do_timeout) return true;
	if(CALCULATOR->checkSaveFunctionCalled()) {
		update_vmenu();
	}
	display_errors();
	return true;
}

void display_function_hint(MathFunction *f, int arg_index = 1) {
	if(!f) return;
	int iargs = f->maxargs();
	Argument *arg;
	Argument default_arg;
	string str, str2, str3;
	const ExpressionName *ename = &f->preferredName(false, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) statuslabel_l);
	bool last_is_vctr = f->getArgumentDefinition(iargs) && f->getArgumentDefinition(iargs)->type() == ARGUMENT_TYPE_VECTOR;
	if(arg_index > iargs && iargs >= 0 && !last_is_vctr) {
		gchar *gstr = g_strdup_printf(_("Too many arguments for %s()."), ename->name.c_str());
		set_status_text(gstr, false, false, true);
		g_free(gstr);
		return;
	}
	str += ename->name;		
	if(iargs < 0) {
		iargs = f->minargs() + 1;
		if(arg_index > iargs) arg_index = iargs;
	}
	if(arg_index > iargs && last_is_vctr) arg_index = iargs;
	str += "(";
	int i_reduced = 0;
	if(iargs != 0) {
		for(int i2 = 1; i2 <= iargs; i2++) {			
			if(i2 > f->minargs() && arg_index < i2) {
				str += "[";
			}
			if(i2 > 1) {
				str += CALCULATOR->getComma();
				str += " ";
			}
			if(i2 == arg_index) str += "<b>";
			arg = f->getArgumentDefinition(i2);
			if(arg && !arg->name().empty()) {
				str2 = arg->name();
			} else {
				str2 = _("argument");
				str2 += " ";
				str2 += i2s(i2);
			}
			if(i2 == arg_index) {
				if(arg) {
					if(i_reduced == 2) str3 = arg->print();
					else str3 = arg->printlong();
				} else {
					Argument arg_default;
					if(i_reduced == 2) str3 = arg_default.print();
					else str3 = arg_default.printlong();
				}
				if(!str3.empty()) {
					str2 += ": ";
					str2 += str3;
				}
				gsub("&", "&amp;", str2);
				gsub(">", "&gt;", str2);
				gsub("<", "&lt;", str2);
				str += str2;					
				str += "</b>";
				if(i_reduced < 2) {
					PangoLayout *layout_test = gtk_widget_create_pango_layout(statuslabel_l, NULL);
					string str3 = "<span size=\"small\">";
					str3 += str;
					str3 += "</span>";
					pango_layout_set_markup(layout_test, str3.c_str(), -1);
					gint w, h;
					pango_layout_get_pixel_size(layout_test, &w, &h);
					if(w > statuslabel_l->allocation.width - 20) {
						str = ename->name;	
						str += "(";
						if(i2 != 1) {
							str += "...";
							i_reduced++;
						} else {
							i_reduced = 2;
						}
						i2--;
					}
					g_object_unref(layout_test);	
				} else {
					i_reduced = 0;
				}
			} else {
				gsub("&", "&amp;", str2);
				gsub(">", "&gt;", str2);
				gsub("<", "&lt;", str2);
				str += str2;
				if(i2 > f->minargs() && arg_index < i2) {
					str += "]";
				}
			}
		}
		if(f->maxargs() < 0) {
			str += CALCULATOR->getComma();
			str += " ...";
		}
	}
	str += ")";
	set_status_text(str);
}

void display_parse_status() {
	if(!display_expression_status) return;
	if(block_display_parse) return;
	if(strlen(gtk_entry_get_text(GTK_ENTRY(expression))) == 0) {
		set_status_text("", true, false, false);
		parsed_expression = "";
		expression_has_changed2 = false;
		return;
	}
	MathStructure mparse, mfunc;
	int pos = gtk_editable_get_position(GTK_EDITABLE(expression));
	bool full_parsed = false;
	string str_e, str_u;
	bool had_errors = false, had_warnings = false;
	evalops.parse_options.preserve_format = true;
	if(pos > 0) {
		evalops.parse_options.unended_function = &mfunc;
		CALCULATOR->beginTemporaryStopMessages();
		if(pos < g_utf8_strlen(gtk_entry_get_text(GTK_ENTRY(expression)), -1)) {
			gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), 0, pos);
			str_e = CALCULATOR->unlocalizeExpression(gstr, evalops.parse_options);
			if(!CALCULATOR->separateToExpression(str_e, str_u, evalops)) {
				CALCULATOR->parse(&mparse, str_e, evalops.parse_options);
			}
			g_free(gstr);
		} else {
			str_e = CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(expression)), evalops.parse_options);
			CALCULATOR->separateToExpression(str_e, str_u, evalops);
			CALCULATOR->parse(&mparse, str_e, evalops.parse_options);
			full_parsed = true;
		}
		int warnings_count;
		had_errors = CALCULATOR->endTemporaryStopMessages(NULL, &warnings_count) > 0;
		had_warnings = warnings_count > 0;
		evalops.parse_options.unended_function = NULL;
	}
	if(mfunc.isFunction()) {
		if(mfunc.countChildren() == 0) {
			display_function_hint(mfunc.function(), 1);
		} else {
			display_function_hint(mfunc.function(), mfunc.countChildren());
		}
	}
	if(expression_has_changed2) {
		if(!full_parsed) {
			CALCULATOR->beginTemporaryStopMessages();
			str_e = CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(expression)), evalops.parse_options);
			CALCULATOR->separateToExpression(str_e, str_u, evalops);
			CALCULATOR->parse(&mparse, str_e, evalops.parse_options);
			int warnings_count;
			had_errors = CALCULATOR->endTemporaryStopMessages(NULL, &warnings_count) > 0;
			had_warnings = warnings_count > 0;
		}
		PrintOptions po;
		po.preserve_format = true;
		po.show_ending_zeroes = true;
		po.lower_case_e = printops.lower_case_e;
		po.lower_case_numbers = printops.lower_case_numbers;
		po.base_display = printops.base_display;
		po.abbreviate_names = false;
		po.hide_underscore_spaces = true;
		po.use_unicode_signs = printops.use_unicode_signs;
		po.multiplication_sign = printops.multiplication_sign;
		po.division_sign = printops.division_sign;
		po.short_multiplication = false;
		po.excessive_parenthesis = true;
		po.improve_division_multipliers = false;
		po.can_display_unicode_string_function = &can_display_unicode_string_function;
		po.can_display_unicode_string_arg = (void*) statuslabel_l;
		po.spell_out_logical_operators = printops.spell_out_logical_operators;
		po.restrict_to_parent_precision = false;
		mparse.format(po);
		parsed_expression = mparse.print(po);
		if(!str_u.empty()) {
			parsed_expression += CALCULATOR->localToString();
			CALCULATOR->beginTemporaryStopMessages();
			CompositeUnit cu("", "temporary_composite_parse", "", str_u);
			int warnings_count;
			had_errors = CALCULATOR->endTemporaryStopMessages(NULL, &warnings_count) > 0 || had_errors;
			had_warnings = had_warnings || warnings_count > 0;
			mparse = cu.generateMathStructure(!printops.negative_exponents);
			mparse.format(po);
			parsed_expression += mparse.print(po);
		}
		parsed_had_errors = had_errors; parsed_had_warnings = had_warnings;
		gsub("&", "&amp;", parsed_expression);
		gsub(">", "&gt;", parsed_expression);
		gsub("<", "&lt;", parsed_expression);
		if(!mfunc.isFunction()) set_status_text(parsed_expression.c_str(), true, had_errors, had_warnings);
		expression_has_changed2 = false;
	} else if(!mfunc.isFunction()) {
		set_status_text(parsed_expression.c_str(), true, parsed_had_errors, parsed_had_warnings);
	}
	evalops.parse_options.preserve_format = false;
}


gboolean on_check_expression_position_timeout(gpointer) {
	if(stop_timeouts) return false;
	if(!check_expression_position) return true;
	check_expression_position = false;
	if(gtk_editable_get_position(GTK_EDITABLE(expression)) != expression_position || expression_has_changed2) {
		expression_position = gtk_editable_get_position(GTK_EDITABLE(expression));
		display_parse_status();
	}
	check_expression_position = true;
	return true;
}

/*
	set focus on expression entry without losing selection
*/
void focus_keeping_selection() {
	if(GTK_WIDGET_HAS_FOCUS(expression)) return;
	gint start = 0, end = 0;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end);
	gtk_widget_grab_focus(expression);
	gtk_editable_select_region(GTK_EDITABLE(expression), start, end);
}

MathFunction *get_selected_function() {
	return selected_function;
}

MathFunction *get_edited_function() {
	return edited_function;
}
Unit *get_edited_unit() {
	return edited_unit;
}
DataSet *get_edited_dataset() {
	return edited_dataset;
}
DataProperty *get_edited_dataproperty() {
	return edited_dataproperty;
}
KnownVariable *get_edited_variable() {
	return edited_variable;
}
UnknownVariable *get_edited_unknown() {
	return edited_unknown;
}
KnownVariable *get_edited_matrix() {
	return edited_matrix;
}

Argument *get_edited_argument() {
	return edited_argument;
}
Argument *get_selected_argument() {
	return selected_argument;
}
size_t get_selected_subfunction() {
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
	size_t cat_i, cat_i_prev; 
	bool b;	
	string str, cat, cat_sub;
	Unit *u = NULL;
	unit_cats.items.clear();
	unit_cats.objects.clear();
	unit_cats.parent = NULL;	
	ia_units.clear();
	list<tree_struct>::iterator it;	
	for(size_t i = 0; i < CALCULATOR->units.size(); i++) {
		if(!CALCULATOR->units[i]->isActive()) {
			b = false;
			for(size_t i3 = 0; i3 < ia_units.size(); i3++) {
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
				cat_i = cat.find("/"); cat_i_prev = 0;
				b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev, cat.length() - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev, cat_i - cat_i_prev);
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
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i + 1;
					cat_i = cat.find("/", cat_i_prev);
				}
			}
			b = false;
			for(size_t i3 = 0; i3 < item->objects.size(); i3++) {
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

	size_t cat_i, cat_i_prev; 
	bool b;	
	string str, cat, cat_sub;
	Variable *v = NULL;
	variable_cats.items.clear();
	variable_cats.objects.clear();
	variable_cats.parent = NULL;
	ia_variables.clear();
	list<tree_struct>::iterator it;	
	for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->isActive()) {
			//deactivated variable
			b = false;
			for(size_t i3 = 0; i3 < ia_variables.size(); i3++) {
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
				cat_i = cat.find("/"); cat_i_prev = 0;
				b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev, cat.length() - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev, cat_i - cat_i_prev);
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
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i + 1;
					cat_i = cat.find("/", cat_i_prev);
				}
			}
			b = false;
			for(size_t i3 = 0; i3 < item->objects.size(); i3++) {
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

	size_t cat_i, cat_i_prev; 
	bool b;	
	string str, cat, cat_sub;
	MathFunction *f = NULL;
	function_cats.items.clear();
	function_cats.objects.clear();
	function_cats.parent = NULL;
	ia_functions.clear();
	list<tree_struct>::iterator it;

	for(size_t i = 0; i < CALCULATOR->functions.size(); i++) {
		if(!CALCULATOR->functions[i]->isActive()) {
			//deactivated function
			b = false;
			for(size_t i3 = 0; i3 < ia_functions.size(); i3++) {
				f = (MathFunction*) ia_functions[i3];
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
				cat_i = cat.find("/"); cat_i_prev = 0;
				b = false;
				while(true) {
					if(cat_i == string::npos) {
						cat_sub = cat.substr(cat_i_prev, cat.length() - cat_i_prev);
					} else {
						cat_sub = cat.substr(cat_i_prev, cat_i - cat_i_prev);
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
					if(cat_i == string::npos) {
						break;
					}
					cat_i_prev = cat_i + 1;
					cat_i = cat.find("/", cat_i_prev);
				}
			}
			b = false;
			for(size_t i3 = 0; i3 < item->objects.size(); i3++) {
				f = (MathFunction*) item->objects[i3];
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
			size_t str_i = str.rfind("/");
			if(str_i == string::npos) {
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

void setFunctionTreeItem(GtkTreeIter &iter2, MathFunction *f) {
	gtk_list_store_append(tFunctions_store, &iter2);
	gtk_list_store_set(tFunctions_store, &iter2, 0, f->title(true).c_str(), 1, (gpointer) f, -1);
	if(f == selected_function) {
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctions)), &iter2);
	}
}

/*
	generate the function tree in manage functions dialog when category selection has changed
*/
void on_tFunctionCategories_selection_changed(GtkTreeSelection *treeselection, gpointer) {
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
	gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_apply"), FALSE);
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
			for(size_t i = 0; i < CALCULATOR->functions.size(); i++) {
				if(CALCULATOR->functions[i]->isActive() && CALCULATOR->functions[i]->category().substr(0, selected_function_category.length() - 1) == str) {
					setFunctionTreeItem(iter2, CALCULATOR->functions[i]);
				}
			}
		} else {			
			for(size_t i = 0; i < CALCULATOR->functions.size(); i++) {
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
void on_tFunctions_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		MathFunction *f;
		gtk_tree_model_get(model, &iter, 1, &f, -1);
		//remember the new selection
		selected_function = f;
		for(size_t i = 0; i < CALCULATOR->functions.size(); i++) {
			if(CALCULATOR->functions[i] == selected_function) {
				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (functions_glade, "functions_textview_description")));
				gtk_text_buffer_set_text(buffer, "", -1);
				GtkTextIter iter;
				f = CALCULATOR->functions[i];
				Argument *arg;
				Argument default_arg;
				string str, str2;
				const ExpressionName *ename = &f->preferredName(false, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) glade_xml_get_widget (functions_glade, "functions_textview_description"));
				str += ename->name;
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
				for(size_t i2 = 1; i2 <= f->countNames(); i2++) {
					if(&f->getName(i2) != ename) {
						str += "\n";
						str += f->getName(i2).name;
					}
				}
				gtk_text_buffer_get_end_iter(buffer, &iter);
				gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "italic", NULL);
				str = "";
				str += "\n";
				if(f->subtype() == SUBTYPE_DATA_SET) {
					str += "\n";
					gchar *gstr = g_strdup_printf(_("Retrieves data from the %s data set for a given object and property. If \"info\" is typed as property, a dialog window will pop up with all properties of the object."), f->title().c_str());
					str += gstr;
					g_free(gstr);
					str += "\n";
				}
				if(!f->description().empty()) {
					str += "\n";
					str += f->description();
					str += "\n";
				}
				if(f->subtype() == SUBTYPE_DATA_SET && !((DataSet*) f)->copyright().empty()) {
					str += "\n";
					str += ((DataSet*) f)->copyright();
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
							//optional argument
							str2 += _("optional");
							if(!f->getDefaultValue(i2).empty()) {
								str2 += ", ";
								//argument default, in description
								str2 += _("default: ");
								str2 += f->getDefaultValue(i2);
							}
							str2 += ")";
						}
						str2 += "\n";
						gtk_text_buffer_get_end_iter(buffer, &iter);
						gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
						gtk_text_buffer_get_end_iter(buffer, &iter);
						gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str2.c_str(), -1, "italic", NULL);
					}
				}
				if(!f->condition().empty()) {
					str = "\n";
					str += _("Requirement");
					str += ": ";
					str += f->printCondition();
					str += "\n";
					gtk_text_buffer_get_end_iter(buffer, &iter);
					gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
				}
				if(f->subtype() == SUBTYPE_DATA_SET) {
					DataSet *ds = (DataSet*) f;
					str = "\n";
					str += _("Properties");
					str += "\n";
					gtk_text_buffer_get_end_iter(buffer, &iter);
					gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "bold", NULL);
					DataPropertyIter it;
					DataProperty *dp = ds->getFirstProperty(&it);
					while(dp) {	
						if(!dp->isHidden()) {
							if(!dp->title(false).empty()) {
								str = dp->title();	
								str += ": ";
							}
							for(size_t i = 1; i <= dp->countNames(); i++) {
								if(i > 1) str += ", ";
								str += dp->getName(i);
							}
							if(dp->isKey()) {
								str += " (";
								//indicating that the property is a data set key
								str += _("key");
								str += ")";
							}
							str += "\n";
							gtk_text_buffer_get_end_iter(buffer, &iter);
							gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
							if(!dp->description().empty()) {
								str = dp->description();
								str += "\n";
								gtk_text_buffer_get_end_iter(buffer, &iter);
								gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "italic", NULL);
							}
						}
						dp = ds->getNextProperty(&it);
					}
				}
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_edit"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_deactivate"), TRUE);
				if(CALCULATOR->functions[i]->isActive()) {
					gtk_label_set_text_with_mnemonic(GTK_LABEL(glade_xml_get_widget (functions_glade, "functions_buttonlabel_deactivate")), _("Deacti_vate"));
				} else {
					gtk_label_set_text_with_mnemonic(GTK_LABEL(glade_xml_get_widget (functions_glade, "functions_buttonlabel_deactivate")), _("Acti_vate"));
				}
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_insert"), CALCULATOR->functions[i]->isActive());
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_apply"), CALCULATOR->functions[i]->isActive() && CALCULATOR->functions[i]->minargs() <= 1);
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_delete"), CALCULATOR->functions[i]->isLocal());
			}
		}
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_edit"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_insert"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_delete"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (functions_glade, "functions_button_deactivate"), FALSE);
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (functions_glade, "functions_textview_description"))), "", -1);
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
			size_t str_i = str.rfind("/");
			if(str_i == string::npos) {
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
	if(is_answer_variable(v)) {
		value = _("a previous result");
	} else if(v->isKnown()) {
		if(((KnownVariable*) v)->isExpression()) {
			value = CALCULATOR->localizeExpression(((KnownVariable*) v)->expression());
			if(value.length() > 20) {
				value = value.substr(0, 17);
				value += "...";
			}
		} else {
			if(((KnownVariable*) v)->get().isMatrix()) {
				value = _("matrix");
			} else if(((KnownVariable*) v)->get().isVector()) {
				value = _("vector");
			} else {
				value = CALCULATOR->printMathStructureTimeOut(((KnownVariable*) v)->get(), 30);
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
			if(!value.empty() && !((UnknownVariable*) v)->assumptions()->type() == ASSUMPTION_TYPE_NONE) value += " ";
			switch(((UnknownVariable*) v)->assumptions()->type()) {
				case ASSUMPTION_TYPE_INTEGER: {value += _("integer"); break;}
				case ASSUMPTION_TYPE_RATIONAL: {value += _("rational"); break;}
				case ASSUMPTION_TYPE_REAL: {value += _("real"); break;}
				case ASSUMPTION_TYPE_COMPLEX: {value += _("complex"); break;}
				case ASSUMPTION_TYPE_NUMBER: {value += _("number"); break;}
				case ASSUMPTION_TYPE_NONMATRIX: {value += _("(not matrix)"); break;}
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
void on_tVariableCategories_selection_changed(GtkTreeSelection *treeselection, gpointer) {
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
			for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
				if(CALCULATOR->variables[i]->isActive() && CALCULATOR->variables[i]->category().substr(0, selected_variable_category.length() - 1) == str) {
					setVariableTreeItem(iter2, CALCULATOR->variables[i]);
				}
			}			
		} else {			
			for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
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
void on_tVariables_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Variable *v;
		gtk_tree_model_get(model, &iter, 2, &v, -1);
		if(!CALCULATOR->stillHasVariable(v)) {
			show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
			selected_variable = NULL;
			update_vmenu();
			return;
		}
		//remember selection
		selected_variable = v;
		for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
			if(CALCULATOR->variables[i] == selected_variable) {
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_edit"), !is_answer_variable(CALCULATOR->variables[i]));
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_insert"), CALCULATOR->variables[i]->isActive());
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_deactivate"), !is_answer_variable(CALCULATOR->variables[i]));
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_export"), CALCULATOR->variables[i]->isKnown());
				if(CALCULATOR->variables[i]->isActive()) {
					gtk_label_set_text_with_mnemonic(GTK_LABEL(glade_xml_get_widget (variables_glade, "variables_buttonlabel_deactivate")), _("Deacti_vate"));
				} else {
					gtk_label_set_text_with_mnemonic(GTK_LABEL(glade_xml_get_widget (variables_glade, "variables_buttonlabel_deactivate")), _("Acti_vate"));
				}
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (variables_glade, "variables_button_delete"), CALCULATOR->variables[i]->isLocal() && !is_answer_variable(CALCULATOR->variables[i]) && CALCULATOR->variables[i] != CALCULATOR->v_x && CALCULATOR->variables[i] != CALCULATOR->v_y && CALCULATOR->variables[i] != CALCULATOR->v_z);
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
			size_t str_i = str.rfind("/");
			if(str_i == string::npos) {
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
	for(size_t i = 1; i <= u->countNames(); i++) {
		if(i > 1) snames += " / ";
		snames += u->getName(i).name;
	}
	//depending on unit type display relation to base unit(s)
	switch(u->subtype()) {
		case SUBTYPE_COMPOSITE_UNIT: {
			snames = "";
			sbase = ((CompositeUnit*) u)->print(false, true, printops.use_unicode_signs, &can_display_unicode_string_function, (void*) tUnits);
			break;
		}
		case SUBTYPE_ALIAS_UNIT: {
			au = (AliasUnit*) u;
			sbase = au->firstBaseUnit()->preferredDisplayName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) tUnits).name;
			if(au->firstBaseExponent() != 1) {
				sbase += POWER;
				sbase += i2s(au->firstBaseExponent());
			}
			break;
		}
		case SUBTYPE_BASE_UNIT: {
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
void on_tUnitCategories_selection_changed(GtkTreeSelection *treeselection, gpointer) {
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
			for(size_t i = 0; i < CALCULATOR->units.size(); i++) {	
				if(CALCULATOR->units[i]->isActive() && CALCULATOR->units[i]->category().substr(0, selected_unit_category.length() - 1) == str) {
					setUnitTreeItem(iter2, CALCULATOR->units[i]);
				}
			}
		} else {
			for(size_t i = 0; i < CALCULATOR->units.size(); i++) {
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
void on_tUnits_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Unit *u;
		gtk_tree_model_get(model, &iter, UNITS_POINTER_COLUMN, &u, -1);
		selected_unit = u;
		for(size_t i = 0; i < CALCULATOR->units.size(); i++) {
			if(CALCULATOR->units[i] == selected_unit) {
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_frame_convert"), TRUE);				
				gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (units_glade, "units_label_from_unit")), CALCULATOR->units[i]->print(true, printops.abbreviate_names, printops.use_unicode_signs, &can_display_unicode_string_function, (void*) glade_xml_get_widget (units_glade, "units_label_from_unit")).c_str());
				//user cannot delete global definitions
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_delete"), CALCULATOR->units[i]->isLocal());
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_convert_to"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_insert"), CALCULATOR->units[i]->isActive());
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_edit"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (units_glade, "units_button_deactivate"), TRUE);
				if(CALCULATOR->units[i]->isActive()) {
					gtk_label_set_text_with_mnemonic(GTK_LABEL(glade_xml_get_widget (units_glade, "units_buttonlabel_deactivate")), _("Deacti_vate"));
				} else {
					gtk_label_set_text_with_mnemonic(GTK_LABEL(glade_xml_get_widget (units_glade, "units_buttonlabel_deactivate")), _("Acti_vate"));
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

void update_unit_selector_tree() {
	if(!unit_glade) return;
	GtkTreeIter iter, iter2, iter3;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tUnitSelectorCategories));
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelectorCategories));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitSelectorCategories_selection_changed, NULL);
	gtk_tree_store_clear(tUnitSelectorCategories_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitSelectorCategories_selection_changed, NULL);
	gtk_tree_store_append(tUnitSelectorCategories_store, &iter3, NULL);
	gtk_tree_store_set(tUnitSelectorCategories_store, &iter3, 0, _("All"), 1, _("All"), -1);
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
		gtk_tree_store_append(tUnitSelectorCategories_store, &iter, &iter2);
		str += "/";
		str += item->item;
		gtk_tree_store_set(tUnitSelectorCategories_store, &iter, 0, item->item.c_str(), 1, str.c_str(), -1);
		if(str == selected_unit_category) {
			EXPAND_TO_ITER(model, tUnitSelectorCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelectorCategories)), &iter);
		}
		while(item && item->it == item->items.end()) {
			size_t str_i = str.rfind("/");
			if(str_i == string::npos) {
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
		gtk_tree_store_append(tUnitSelectorCategories_store, &iter, &iter3);
		gtk_tree_store_set(tUnitSelectorCategories_store, &iter, 0, _("Uncategorized"), 1, _("Uncategorized"), -1);
		if(selected_unit_category == _("Uncategorized")) {
			EXPAND_TO_ITER(model, tUnitSelectorCategories, iter)
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelectorCategories)), &iter);
		}
	}	
	if(!gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelectorCategories)), &model, &iter)) {
		//if no category has been selected (previously selected has been renamed/deleted), select "All"
		selected_unit_category = _("All");
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tUnitSelectorCategories_store), &iter);
		EXPAND_ITER(model, tUnitSelectorCategories, iter)
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelectorCategories)), &iter);
	}
}

void setUnitSelectorTreeItem(GtkTreeIter &iter2, Unit *u) {
	gtk_list_store_append(tUnitSelector_store, &iter2);
	string snames, sbase;
	gtk_list_store_set(tUnitSelector_store, &iter2, 0, u->title(true).c_str(), 1, (gpointer) u, -1);
}

/*
	generate the unit tree and units conversion menu in manage units dialog when category selection has changed
*/
void on_tUnitSelectorCategories_selection_changed(GtkTreeSelection *treeselection, gpointer) {

	block_unit_selector_convert = true;

	GtkTreeModel *model;
	GtkTreeIter iter, iter2;

	bool no_cat = false, b_all = false;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelector));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitSelector_selection_changed, NULL);
	gtk_list_store_clear(tUnitSelector_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tUnitSelector_selection_changed, NULL);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr;
		gtk_tree_model_get(model, &iter, 1, &gstr, -1);
		selected_unit_selector_category = gstr;
		if(selected_unit_selector_category == _("All")) {
			b_all = true;
		} else if(selected_unit_selector_category == _("Uncategorized")) {
			no_cat = true;
		}
		if(!b_all && !no_cat && selected_unit_selector_category[0] == '/') {
			string str = selected_unit_selector_category.substr(1, selected_unit_selector_category.length() - 1);
			for(size_t i = 0; i < CALCULATOR->units.size(); i++) {	
				if(CALCULATOR->units[i]->isActive() && !CALCULATOR->units[i]->isHidden() && CALCULATOR->units[i]->category().substr(0, selected_unit_selector_category.length() - 1) == str) {
					setUnitSelectorTreeItem(iter2, CALCULATOR->units[i]);
				}
			}
		} else {
			for(size_t i = 0; i < CALCULATOR->units.size(); i++) {
				if(CALCULATOR->units[i]->isActive() && !CALCULATOR->units[i]->isHidden() && (b_all || (no_cat && CALCULATOR->units[i]->category().empty()) || CALCULATOR->units[i]->category() == selected_unit_selector_category)) {
					setUnitSelectorTreeItem(iter2, CALCULATOR->units[i]);			
				}
			}
		}
		g_free(gstr);
	} else {
		selected_unit_selector_category = "";
	}
	
	block_unit_selector_convert = false;
	
}

void on_tUnitSelector_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		Unit *u;
		gtk_tree_model_get(model, &iter, 1, &u, -1);
		for(size_t i = 0; i < CALCULATOR->units.size(); i++) {
			if(CALCULATOR->units[i] == u) {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit")), u->print(false, printops.abbreviate_names, printops.use_unicode_signs, &can_display_unicode_string_function, (void*) glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit")).c_str());
				gtk_tree_selection_unselect_all(treeselection);
				gtk_widget_grab_focus(glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit"));
				if(!block_unit_selector_convert) gtk_button_clicked(GTK_BUTTON(glade_xml_get_widget (unit_glade, "unit_dialog_button_apply")));
			}
		}
	}
}


void update_datasets_tree() {
	if(!datasets_glade) return;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tDatasets));
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tDatasets_selection_changed, NULL);
	gtk_list_store_clear(tDatasets_store);
	DataSet *ds;
	bool b = false;
	for(size_t i = 1; ; i++) {
		ds = CALCULATOR->getDataSet(i);
		if(!ds) break;
		gtk_list_store_append(tDatasets_store, &iter);
		gtk_list_store_set(tDatasets_store, &iter, 0, ds->title().c_str(), 1, (gpointer) ds, -1);
		if(ds == selected_dataset) {
			g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tDatasets_selection_changed, NULL);
			gtk_tree_selection_select_iter(select, &iter);
			g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tDatasets_selection_changed, NULL);
			b = true;
		}
	}
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tDatasets_selection_changed, NULL);
	if(!b) {
		gtk_tree_selection_unselect_all(select);
		selected_dataset = NULL;
	}
}

void on_tDatasets_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model, *model2;
	GtkTreeIter iter, iter2;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tDataObjects));
	gtk_tree_selection_unselect_all(select);
	g_signal_handlers_block_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tDataObjects_selection_changed, NULL);
	gtk_list_store_clear(tDataObjects_store);
	g_signal_handlers_unblock_matched((gpointer) select, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_tDataObjects_selection_changed, NULL);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		DataSet *ds = NULL;
		gtk_tree_model_get(model, &iter, 1, &ds, -1);
		selected_dataset = ds;
		if(!ds) return;
		DataObjectIter it;
		DataPropertyIter pit;
		DataProperty *dp;
		DataObject *o = ds->getFirstObject(&it);
		bool b = false;
		while(o) {
			b = true;
			gtk_list_store_append(tDataObjects_store, &iter2);
			dp = ds->getFirstProperty(&pit);
			size_t index = 0;
			while(dp) {
				if(!dp->isHidden() && dp->isKey()) {
					gtk_list_store_set(tDataObjects_store, &iter2, index, o->getPropertyDisplayString(dp).c_str(), -1);
					index++;
					if(index > 2) break;
				}
				dp = ds->getNextProperty(&pit);
			}
			while(index < 3) {
				gtk_list_store_set(tDataObjects_store, &iter2, index, "", -1);
				index++;
			}
			gtk_list_store_set(tDataObjects_store, &iter2, 3, (gpointer) o, -1);
			if(o == selected_dataobject) {
				gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tDataObjects)), &iter2);
			}
			o = ds->getNextObject(&it);
		}
		if(b && (!selected_dataobject || !gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tDataObjects)), &model2, &iter2))) {
			gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tDataObjects_store), &iter2);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tDataObjects)), &iter2);
		}
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (datasets_glade, "datasets_textview_description")));
		gtk_text_buffer_set_text(buffer, "", -1);
		GtkTextIter iter;
		string str, str2;
		if(!ds->description().empty()) {
			str = ds->description();
			str += "\n";
			str += "\n";
			gtk_text_buffer_get_end_iter(buffer, &iter);
			gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
		}	
		str = _("Properties");
		str += "\n";
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "bold", NULL);
		dp = ds->getFirstProperty(&pit);
		while(dp) {	
			if(!dp->isHidden()) {
				str = "";
				if(!dp->title(false).empty()) {
					str += dp->title();	
					str += ": ";
				}
				for(size_t i = 1; i <= dp->countNames(); i++) {
					if(i > 1) str += ", ";
					str += dp->getName(i);
				}
				if(dp->isKey()) {
					str += " (";
					str += _("key");
					str += ")";
				}
				str += "\n";
				gtk_text_buffer_get_end_iter(buffer, &iter);
				gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
				if(!dp->description().empty()) {
					str = dp->description();
					str += "\n";
					gtk_text_buffer_get_end_iter(buffer, &iter);
					gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "italic", NULL);
				}
			}
			dp = ds->getNextProperty(&pit);
		}
		str = "\n";
		str += _("Data Retrieval Function");
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "bold", NULL);
		Argument *arg;
		Argument default_arg;
		const ExpressionName *ename = &ds->preferredName(false, true, false, false, &can_display_unicode_string_function, (void*) glade_xml_get_widget (datasets_glade, "datasets_textview_description"));
		str = "\n";
		str += ename->name;
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "bold", "italic", NULL);
		str = "";
		int iargs = ds->maxargs();
		if(iargs < 0) {
			iargs = ds->minargs() + 1;
		}
		str += "(";				
		if(iargs != 0) {
			for(int i2 = 1; i2 <= iargs; i2++) {	
				if(i2 > ds->minargs()) {
					str += "[";
				}
				if(i2 > 1) {
					str += CALCULATOR->getComma();
					str += " ";
				}
				arg = ds->getArgumentDefinition(i2);
				if(arg && !arg->name().empty()) {
					str2 = arg->name();
				} else {
					str2 = _("argument");
					str2 += " ";
					str2 += i2s(i2);
				}
				str += str2;
				if(i2 > ds->minargs()) {
					str += "]";
				}
			}
			if(ds->maxargs() < 0) {
				str += CALCULATOR->getComma();
				str += " ...";
			}
		}
		str += ")";
		for(size_t i2 = 1; i2 <= ds->countNames(); i2++) {
			if(&ds->getName(i2) != ename) {
				str += "\n";
				str += ds->getName(i2).name;
			}
		}
		str += "\n\n";
		gtk_text_buffer_get_end_iter(buffer, &iter);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str.c_str(), -1, "italic", NULL);
		if(!ds->copyright().empty()) {
			str = "\n";
			str = ds->copyright();
			str += "\n";
			gtk_text_buffer_get_end_iter(buffer, &iter);
			gtk_text_buffer_insert(buffer, &iter, str.c_str(), -1);
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_editset"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_delset"), ds->isLocal());
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_newobject"), TRUE);
	} else {
		gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (datasets_glade, "datasets_textview_description"))), "", -1);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_editset"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_delset"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_newobject"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_editobject"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_delobject"), FALSE);
		selected_dataset = NULL;
	}
}

void update_dataobjects() {
	on_tDatasets_selection_changed(gtk_tree_view_get_selection(GTK_TREE_VIEW(tDatasets)), NULL);
}

void on_dataset_button_function_clicked(GtkButton *w, gpointer user_data) {
	DataProperty *dp = (DataProperty*) user_data;
	DataObject *o = selected_dataobject;
	DataSet *ds = NULL;
	if(o) ds = dp->parentSet();
	if(ds && o) {
		string str = ds->preferredDisplayName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) w).name;
		str += "(";
		str += o->getProperty(ds->getPrimaryKeyProperty());
		str += CALCULATOR->getComma();
		str += " ";
		str += dp->getName();
		str += ")";
		insert_text(str.c_str());
	}
}
void on_tDataObjects_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkWidget *ptable = glade_xml_get_widget(datasets_glade, "datasets_table_properties");
	GList *childlist = gtk_container_get_children(GTK_CONTAINER(ptable));
	for(guint i = 0; ; i++) {
		GtkWidget *w = (GtkWidget*) g_list_nth_data(childlist, i);
		if(!w) break;
		gtk_widget_destroy(w);
	}
	g_list_free(childlist);
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		DataObject *o = NULL;
		gtk_tree_model_get(model, &iter, 3, &o, -1);
		selected_dataobject = o;
		if(!o) return;
		DataSet *ds = o->parentSet();
		if(!ds) return;
		DataPropertyIter it;
		DataProperty *dp = ds->getFirstProperty(&it);
		string sval;
		int rows = 1;
		gtk_table_resize(GTK_TABLE(ptable), rows, 3);
		gtk_table_set_col_spacing(GTK_TABLE(ptable), 0, 20);
		GtkWidget *button, *label, *align;
		string str;
		while(dp) {
			if(!dp->isHidden()) {
				sval = o->getPropertyDisplayString(dp);
				if(!sval.empty()) {
					gtk_table_resize(GTK_TABLE(ptable), rows, 3);
					label = gtk_label_new(NULL);
					str = "<span weight=\"bold\">"; str += dp->title(); str += ":"; str += "</span>";
					gtk_label_set_markup(GTK_LABEL(label), str.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), FALSE);
					gtk_table_attach(GTK_TABLE(ptable), label, 0, 1, rows - 1, rows, GTK_FILL, GTK_FILL, 0, 0);
					label = gtk_label_new(NULL);
					gtk_label_set_markup(GTK_LABEL(label), sval.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), TRUE);
					gtk_table_attach(GTK_TABLE(ptable), label, 1, 2, rows - 1, rows, GTK_FILL, GTK_FILL, 0, 0);
					button = gtk_button_new();
					gtk_container_add(GTK_CONTAINER(button), gtk_image_new_from_stock("gtk-paste", GTK_ICON_SIZE_BUTTON));
					align = gtk_alignment_new(1.0, 0.5, 0.0, 1.0);
					gtk_container_add(GTK_CONTAINER(align), button);
					gtk_table_attach(GTK_TABLE(ptable), align, 2, 3, rows - 1, rows, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), GTK_FILL, 0, 0);
					g_signal_connect((gpointer) button, "clicked", G_CALLBACK(on_dataset_button_function_clicked), (gpointer) dp);
					rows++;
				}
			}
			dp = ds->getNextProperty(&it);
		}
		gtk_widget_show_all(ptable);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_editobject"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_delobject"), o->isUserModified());
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_editobject"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasets_glade, "datasets_button_delobject"), FALSE);
		selected_dataobject = NULL;
	}
}

void on_tDataProperties_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	selected_dataproperty = NULL;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, 3, &selected_dataproperty, -1);
	}
	if(selected_dataproperty) {
		gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_edit_property"), selected_dataproperty->isUserModified());
		gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_del_property"), selected_dataproperty->isUserModified());
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_edit_property"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_del_property"), FALSE);
	}
}


void on_tPlotFunctions_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	selected_argument = NULL;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gchar *gstr1, *gstr2, *gstr3;
		gint type, smoothing, style, axis, rows;
		gtk_tree_model_get(model, &iter, 0, &gstr1, 1, &gstr2, 2, &style, 3, &smoothing, 4, &type, 5, &axis, 6, &rows, 9, &gstr3, -1);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")), gstr2);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")), gstr3);
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
		g_free(gstr3);
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_modify"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_button_remove"), FALSE);
	}
}

void on_tSubfunctions_selection_changed(GtkTreeSelection *treeselection, gpointer) {
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

void on_tFunctionArguments_selection_changed(GtkTreeSelection *treeselection, gpointer) {
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
				case ARGUMENT_TYPE_DATA_OBJECT: {
					menu_index = MENU_ARGUMENT_TYPE_DATA_OBJECT;
					break;
				}
				case ARGUMENT_TYPE_DATA_PROPERTY: {
					menu_index = MENU_ARGUMENT_TYPE_DATA_PROPERTY;
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
void update_function_arguments_list(MathFunction *f) {
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

void on_tNames_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	selected_subfunction = 0;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		gboolean abbreviation = FALSE, suffix = FALSE, unicode = FALSE, plural = FALSE, reference = FALSE, avoid_input = FALSE, case_sensitive = FALSE;
		gchar *name;
		gtk_tree_model_get(model, &iter, NAMES_NAME_COLUMN, &name, NAMES_ABBREVIATION_COLUMN, &abbreviation, NAMES_SUFFIX_COLUMN, &suffix, NAMES_UNICODE_COLUMN, &unicode, NAMES_PLURAL_COLUMN, &plural, NAMES_REFERENCE_COLUMN, &reference, NAMES_AVOID_INPUT_COLUMN, &avoid_input, NAMES_CASE_SENSITIVE_COLUMN, &case_sensitive, -1);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name")), name);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation")), abbreviation);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_suffix")), suffix);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_unicode")), unicode);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural")), plural);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference")), reference);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_avoid_input")), avoid_input);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive")), case_sensitive);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_button_modify"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_button_remove"), TRUE);
		g_free(name);
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_button_modify"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_button_remove"), FALSE);
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
		for(size_t i = 0; i < titem->objects.size(); i++) {
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
	for(size_t i = 0; i < unit_cats.objects.size(); i++) {
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
		for(size_t i = 0; i < titem->objects.size(); i++) {
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
	for(size_t i = 0; i < unit_cats.objects.size(); i++) {
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
	update_unit_selector_tree();
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
		for(size_t i = 0; i < titem->objects.size(); i++) {
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

	for(size_t i = 0; i < variable_cats.objects.size(); i++) {
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
		gchar *gstr = NULL;
		switch(p->type()) {
			case PREFIX_DECIMAL: {
				gstr = g_strdup_printf("%s (10<sup>%i</sup>)", p->name(false, true, &can_display_unicode_string_function, (void*) item).c_str(), ((DecimalPrefix*) p)->exponent());
				break;
			}
			case PREFIX_BINARY: {
				gstr = g_strdup_printf("%s (2<sup>%i</sup>)", p->name(false, true, &can_display_unicode_string_function, (void*) item).c_str(), ((BinaryPrefix*) p)->exponent());
				break;
			}
			case PREFIX_NUMBER: {				
				gstr = g_strdup_printf("%s", p->name(false, true, &can_display_unicode_string_function, (void*) item).c_str());
				break;
			}			
		}
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
	MENU_ITEM_WITH_POINTER(_("No Prefix"), set_prefix, CALCULATOR->decimal_null_prefix)
	Prefix *p = CALCULATOR->getPrefix(index);
	while(p) {
		gchar *gstr = NULL;
		switch(p->type()) {
			case PREFIX_DECIMAL: {
				gstr = g_strdup_printf("%s (10<sup>%i</sup>)", p->name(false, true, &can_display_unicode_string_function, (void*) item).c_str(), ((DecimalPrefix*) p)->exponent());
				break;
			}
			case PREFIX_BINARY: {
				gstr = g_strdup_printf("%s (2<sup>%i</sup>)", p->name(false, true, &can_display_unicode_string_function, (void*) item).c_str(), ((BinaryPrefix*) p)->exponent());
				break;
			}
			case PREFIX_NUMBER: {				
				gstr = g_strdup_printf("%s", p->name(false, true, &can_display_unicode_string_function, (void*) item).c_str());
				break;
			}			
		}
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
	MathFunction *f;
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
		for(size_t i = 0; i < titem->objects.size(); i++) {
			f = (MathFunction*) titem->objects[i];
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
	for(size_t i = 0; i < function_cats.objects.size(); i++) {
		f = (MathFunction*) function_cats.objects[i];
		if(f->isActive() && !f->isHidden()) {
			MENU_ITEM_WITH_POINTER(f->title(true).c_str(), insert_function, f)
		}
	}		
}

string sub_suffix(const ExpressionName *ename) {
	size_t i = ename->name.rfind('_');
	bool b = i == string::npos || i == ename->name.length() - 1 || i == 0;
	size_t i2 = 1;
	string str;
	if(b) {
		if(is_in(NUMBERS, ename->name[ename->name.length() - 1])) {
			while(ename->name.length() > i2 + 1 && is_in(NUMBERS, ename->name[ename->name.length() - 1 - i2])) {
					i2++;
			}
		}
		str += ename->name.substr(0, ename->name.length() - i2);
	} else {
		str += ename->name.substr(0, i);
	}
	str += "<span size=\"small\"><sub>";
	if(b) str += ename->name.substr(ename->name.length() - i2, i2);
	else str += ename->name.substr(i + 1, ename->name.length() - (i + 1));
	str += "</sub></span>";
	return str;
}

void update_completion() {

	GtkTreeIter iter;
	
	gtk_entry_set_completion(GTK_ENTRY(expression), NULL);
	
	if(!enable_completion) return;
	
	completion = gtk_entry_completion_new();
	completion_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	//gtk_list_store_clear(completion_store);	
	string str;
	for(size_t i = 0; i < CALCULATOR->functions.size(); i++) {
		if(CALCULATOR->functions[i]->isActive()) {
			gtk_list_store_append(completion_store, &iter);
			const ExpressionName *ename, *ename_r;
			ename_r = &CALCULATOR->functions[i]->preferredInputName(false, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression);
			if(ename_r->suffix && ename_r->name.length() > 1) {
				str = sub_suffix(ename_r);
			} else {
				str = ename_r->name;
			}
			str += "()";
			for(size_t name_i = 1; name_i <= CALCULATOR->functions[i]->countNames(); name_i++) {
				ename = &CALCULATOR->functions[i]->getName(name_i);
				if(ename && ename != ename_r && !ename->plural && (!ename->unicode || can_display_unicode_string_function(ename->name.c_str(), (void*) expression))) {
					str += " <i>";
					if(ename->suffix && ename->name.length() > 1) {
						str += sub_suffix(ename);
					} else {
						str += ename->name;
					}
					str += "()</i>";
				}
			}			
			gtk_list_store_set(completion_store, &iter, 0, str.c_str(), 1, CALCULATOR->functions[i]->title().c_str(), 2, CALCULATOR->functions[i], -1);
		}
	}
	for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
		if(CALCULATOR->variables[i]->isActive()) {
			gtk_list_store_append(completion_store, &iter);
			const ExpressionName *ename, *ename_r;
			bool b = false;
			ename_r = &CALCULATOR->variables[i]->preferredInputName(false, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression);
			for(size_t name_i = 1; name_i <= CALCULATOR->variables[i]->countNames(); name_i++) {
				ename = &CALCULATOR->variables[i]->getName(name_i);
				if(ename && ename != ename_r && !ename->plural && (!ename->unicode || can_display_unicode_string_function(ename->name.c_str(), (void*) expression))) {
					if(!b) {
						if(ename_r->suffix && ename_r->name.length() > 1) {
							str = sub_suffix(ename_r);
						} else {
							str = ename_r->name;
						}
						b = true;
					}
					str += " <i>";
					if(ename->suffix && ename->name.length() > 1) {
						str += sub_suffix(ename);
					} else {
						str += ename->name;
					}
					str += "</i>";
				}
			}
			if(!CALCULATOR->variables[i]->title(false).empty()) {
				if(b) gtk_list_store_set(completion_store, &iter, 0, str.c_str(), 1, CALCULATOR->variables[i]->title().c_str(), 2, CALCULATOR->variables[i], -1);
				else gtk_list_store_set(completion_store, &iter, 0, ename_r->name.c_str(), 1, CALCULATOR->variables[i]->title().c_str(), 2, CALCULATOR->variables[i], -1);
			} else {
				Variable *v = CALCULATOR->variables[i];
				string title;
				if(is_answer_variable(v)) {
					title = _("a previous result");
				} else if(v->isKnown()) {
					if(((KnownVariable*) v)->isExpression()) {
						title = CALCULATOR->localizeExpression(((KnownVariable*) v)->expression());
					} else {
						if(((KnownVariable*) v)->get().isMatrix()) {
							title = _("matrix");
						} else if(((KnownVariable*) v)->get().isVector()) {
							title = _("vector");
						} else {
							title = CALCULATOR->printMathStructureTimeOut(((KnownVariable*) v)->get(), 30);
						}
					}
				} else {
					if(((UnknownVariable*) v)->assumptions()) {
						switch(((UnknownVariable*) v)->assumptions()->sign()) {
							case ASSUMPTION_SIGN_POSITIVE: {title = _("positive"); break;}
							case ASSUMPTION_SIGN_NONPOSITIVE: {title = _("non-positive"); break;}
							case ASSUMPTION_SIGN_NEGATIVE: {title = _("negative"); break;}
							case ASSUMPTION_SIGN_NONNEGATIVE: {title = _("non-negative"); break;}
							case ASSUMPTION_SIGN_NONZERO: {title = _("non-zero"); break;}
							default: {}
						}
						if(!title.empty() && !((UnknownVariable*) v)->assumptions()->type() == ASSUMPTION_TYPE_NONE) title += " ";
						switch(((UnknownVariable*) v)->assumptions()->type()) {
							case ASSUMPTION_TYPE_INTEGER: {title += _("integer"); break;}
							case ASSUMPTION_TYPE_RATIONAL: {title += _("rational"); break;}
							case ASSUMPTION_TYPE_REAL: {title += _("real"); break;}
							case ASSUMPTION_TYPE_COMPLEX: {title += _("complex"); break;}
							case ASSUMPTION_TYPE_NUMBER: {title += _("number"); break;}
							case ASSUMPTION_TYPE_NONMATRIX: {title += _("(not matrix)"); break;}
							default: {}
						}
						if(title.empty()) title = _("unknown");
					} else {
						title = _("default assumptions");
					}		
				}
				if(b) gtk_list_store_set(completion_store, &iter, 0, str.c_str(), 1, title.c_str(), 2, CALCULATOR->variables[i], -1);
				else gtk_list_store_set(completion_store, &iter, 0, ename_r->name.c_str(), 1, title.c_str(), 2, CALCULATOR->variables[i], -1);
			}
		}
	}
	for(size_t i = 0; i < CALCULATOR->units.size(); i++) {
		if(CALCULATOR->units[i]->isActive() && CALCULATOR->units[i]->subtype() != SUBTYPE_COMPOSITE_UNIT) {
			gtk_list_store_append(completion_store, &iter);
			const ExpressionName *ename, *ename_r;
			bool b = false;
			ename_r = &CALCULATOR->units[i]->preferredInputName(false, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression);
			for(size_t name_i = 1; name_i <= CALCULATOR->units[i]->countNames(); name_i++) {
				ename = &CALCULATOR->units[i]->getName(name_i);
				if(ename && ename != ename_r && !ename->plural && (!ename->unicode || can_display_unicode_string_function(ename->name.c_str(), (void*) expression))) {
					if(!b) {
						if(ename_r->suffix && ename_r->name.length() > 1) {
							str = sub_suffix(ename_r);
						} else {
							str = ename_r->name;
						}
						b = true;
					}
					str += " <i>";
					if(ename->suffix && ename->name.length() > 1) {
						str += sub_suffix(ename);
					} else {
						str += ename->name;
					}
					str += "</i>";
				}
			}
			if(b) gtk_list_store_set(completion_store, &iter, 0, str.c_str(), 1, CALCULATOR->units[i]->title().c_str(), 2, CALCULATOR->units[i], -1);
			else gtk_list_store_set(completion_store, &iter, 0, ename_r->name.c_str(), 1, CALCULATOR->units[i]->title().c_str(), 2, CALCULATOR->units[i], -1);
		}
	}	

	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(completion_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(completion_store), 0, GTK_SORT_ASCENDING);
	gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(completion_store));
	g_object_unref(completion_store);
	GtkCellRenderer *cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(completion), cell, TRUE);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(completion), cell, "markup", 0);	
	cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(completion), cell, FALSE);
	g_object_set(G_OBJECT(cell), "style", PANGO_STYLE_ITALIC, NULL);
	gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(completion), cell, "text", 1);
	gtk_entry_completion_set_match_func(completion, &completion_match_func, NULL, NULL);
	g_signal_connect((gpointer) completion, "match-selected", G_CALLBACK(on_completion_match_selected), NULL);
	gtk_entry_set_completion(GTK_ENTRY(expression), completion);
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
	string str = CALCULATOR->printMathStructureTimeOut(mstruct_, 100, printops);
	printops.allow_non_usable = false;
	printops.prefix = NULL;
	return str;
}


void draw_background(GdkPixmap *pixmap, gint w, gint h) {
	gdk_draw_rectangle(pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);
}

GdkPixbuf *get_left_parenthesis(gint arc_w, gint arc_h, int) {
	if(left_par_pix.count(arc_w * 1000 + arc_h) > 0) {
		return left_par_pix[arc_w * 1000 + arc_h];
	}
	if(!pixbuf_left_par) {
		gint par_h = 0, par_w = 0;
		PangoLayout *layout_parl = gtk_widget_create_pango_layout(resultview, NULL);
		pango_layout_set_markup(layout_parl, "<span font_desc=\"100\">(</span>", -1);
		pango_layout_get_pixel_size(layout_parl, &par_w, &par_h);
		GdkPixmap *pixmap_tmp = gdk_pixmap_new(resultview->window, par_w, par_h, -1);
		draw_background(pixmap_tmp, par_w, par_h);
		gdk_draw_layout(GDK_DRAWABLE(pixmap_tmp), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, 0, layout_parl);	
		pixbuf_left_par = gdk_pixbuf_get_from_drawable(NULL, pixmap_tmp, NULL, 0, 0, 0, 0, -1, -1);
		gdk_pixmap_unref(pixmap_tmp);
		g_object_unref(layout_parl);
	}
	GdkPixbuf *pixbuf_parl = gdk_pixbuf_scale_simple(pixbuf_left_par, arc_w, arc_h, GDK_INTERP_HYPER);
	left_par_pix[arc_w * 1000 + arc_h] = pixbuf_parl;
	return pixbuf_parl;
}
GdkPixbuf *get_right_parenthesis(gint arc_w, gint arc_h, int) {
	if(right_par_pix.count(arc_w * 1000 + arc_h) > 0) {
		return right_par_pix[arc_w * 1000 + arc_h];
	}
	if(!pixbuf_right_par) {
		gint par_h = 0, par_w = 0;
		PangoLayout *layout_parl = gtk_widget_create_pango_layout(resultview, NULL);
		pango_layout_set_markup(layout_parl, "<span font_desc=\"100\">)</span>", -1);
		pango_layout_get_pixel_size(layout_parl, &par_w, &par_h);
		GdkPixmap *pixmap_tmp = gdk_pixmap_new(resultview->window, par_w, par_h, -1);
		draw_background(pixmap_tmp, par_w, par_h);
		gdk_draw_layout(GDK_DRAWABLE(pixmap_tmp), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 0, 0, layout_parl);	
		pixbuf_right_par = gdk_pixbuf_get_from_drawable(NULL, pixmap_tmp, NULL, 0, 0, 0, 0, -1, -1);
		gdk_pixmap_unref(pixmap_tmp);
		g_object_unref(layout_parl);
	}
	GdkPixbuf *pixbuf_parl = gdk_pixbuf_scale_simple(pixbuf_right_par, arc_w, arc_h, GDK_INTERP_HYPER);
	right_par_pix[arc_w * 1000 + arc_h] = pixbuf_parl;
	return pixbuf_parl;
}

GdkPixmap *draw_structure(MathStructure &m, PrintOptions po, InternalPrintStruct ips, gint *point_central, int scaledown) {

	if(ips.depth == 0 && po.is_approximate) *po.is_approximate = false;

	GdkPixmap *pixmap = NULL;
	gint w, h;
	gint central_point = 0;
	
	InternalPrintStruct ips_n = ips;
	if(m.isApproximate()) ips_n.parent_approximate = true;
	if(m.precision() > 0 && (ips_n.parent_precision < 1 || m.precision() < ips_n.parent_precision)) ips_n.parent_precision = m.precision();
	
	switch(m.type()) {
		case STRUCT_NUMBER: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str, exp = "";
			bool exp_minus;
			ips_n.exp = &exp;
			ips_n.exp_minus = &exp_minus;
			TTBP(str)
			str += m.number().print(po, ips_n);
			if(!exp.empty()) {
				if(po.lower_case_e) {TTP(str, "e");}
				else {TTP_SMALL(str, "E");}
				if(exp_minus) {
					str += "-";
				}
				str += exp;
			}
			if(po.base != BASE_DECIMAL && po.base != BASE_HEXADECIMAL && po.base > 0 && po.base <= 36) {
				TTBP_SMALL(str)
				str += "<sub>";
				str += i2s(po.base);
				str += "</sub>";
				TTE(str)
			}
			TTE(str)
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width + rect.x;
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
			str = "<i>";
			TTBP(str)
			str += m.symbol();
			TTE(str)
			str += "</i>";
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width + rect.x;
			w += 1;
			central_point = h / 2;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			draw_background(pixmap, w, h);
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			break;
		}
		case STRUCT_ADDITION: {

			ips_n.depth++;
			
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> wpt;
			vector<gint> cpt;
			gint plus_w, plus_h, minus_w, minus_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
			
			CALCULATE_SPACE_W
			PangoLayout *layout_plus = gtk_widget_create_pango_layout(resultview, NULL);
			PANGO_TTP(layout_plus, "+");
			pango_layout_get_pixel_size(layout_plus, &plus_w, &plus_h);
			PangoLayout *layout_minus = gtk_widget_create_pango_layout(resultview, NULL);
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) {
				PANGO_TTP(layout_minus, SIGN_MINUS);
			} else {
				PANGO_TTP(layout_minus, "-");
			}
			pango_layout_get_pixel_size(layout_minus, &minus_w, &minus_h);
			for(size_t i = 0; i < m.size(); i++) {
				hetmp = 0;		
				if(m[i].type() == STRUCT_NEGATE && i > 0) {
					ips_n.wrap = m[i][0].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
					pixmap_terms.push_back(draw_structure(m[i][0], po, ips_n, &hetmp, scaledown));
				} else {
					ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
					pixmap_terms.push_back(draw_structure(m[i], po, ips_n, &hetmp, scaledown));
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
			for(size_t i = 0; i < pixmap_terms.size(); i++) {
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
			
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MINUS, po.can_display_unicode_string_arg))) {
				PANGO_TTP(layout_minus, SIGN_MINUS);
			} else {
				PANGO_TTP(layout_minus, "-");
			}
			pango_layout_get_pixel_size(layout_minus, &minus_w, &minus_h);

			w = minus_w + 1;
			uh = minus_h / 2 + minus_h % 2;
			dh = minus_h / 2;
			
			ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
			GdkPixmap *pixmap_arg = draw_structure(m[0], po, ips_n, &ctmp, scaledown);
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
			if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIDOT, po.can_display_unicode_string_arg))) {
				TTP(str, SIGN_MULTIDOT);
			} else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIBULLET, po.can_display_unicode_string_arg))) {
				TTP(str, SIGN_MULTIBULLET);
			} else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_DOT && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_SMALLCIRCLE, po.can_display_unicode_string_arg))) {
				TTP(str, SIGN_SMALLCIRCLE);
			} else if(po.use_unicode_signs && po.multiplication_sign == MULTIPLICATION_SIGN_X && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_MULTIPLICATION, po.can_display_unicode_string_arg))) {
				TTP_SMALL(str, SIGN_MULTIPLICATION);
			} else {
				TTP(str, "*");
			}
			pango_layout_set_markup(layout_mul, str.c_str(), -1);
			pango_layout_get_pixel_size(layout_mul, &mul_w, &mul_h);
			bool par_prev = false;
			vector<int> nm;
			for(size_t i = 0; i < m.size(); i++) {
				hetmp = 0;		
				ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_terms.push_back(draw_structure(m[i], po, ips_n, &hetmp, scaledown));
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
					nm.push_back(m[i].neededMultiplicationSign(po, ips_n, m, i + 1, ips_n.wrap || (m[i].isPower() && m[i][0].needsParenthesis(po, ips_n, m[i], 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0)), par_prev, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0));
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
				par_prev = ips_n.wrap;		
			}
			central_point = dh;
			h = dh + uh;
			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);			
			w = 0;
			for(size_t i = 0; i < pixmap_terms.size(); i++) {
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
			
			bool flat = ips.division_depth > 0 || ips.power_depth > 0;
			if(!flat && po.place_units_separately) {
				flat = true;
				size_t i = 0;
				if(m.isDivision()) {
					i = 1;
				}
				if(m[i].isMultiplication()) {
					for(size_t i2 = 0; i2 < m[i].size(); i2++) {
						if(!m[i][i2].isUnit_exp()) {
							flat = false;
							break;
						}
					}
				} else if(!m[i].isUnit_exp()) {
					flat = false;
				}
				if(flat) {
					ips_n.division_depth--;
				}
			}
			
			GdkPixmap *num_pixmap = NULL, *den_pixmap = NULL, *pixmap_one = NULL;
			if(m.type() == STRUCT_DIVISION) {
				ips_n.wrap = (!m[0].isDivision() || !flat || ips.division_depth > 0 || ips.power_depth > 0) && m[0].needsParenthesis(po, ips_n, m, 1, flat, ips.power_depth > 0);
				num_pixmap = draw_structure(m[0], po, ips_n, &num_dh, scaledown);
				gdk_drawable_get_size(GDK_DRAWABLE(num_pixmap), &num_w, &h);
				num_uh = h - num_dh;
			} else {
				MathStructure onestruct(1, 1);
				ips_n.wrap = false;
				pixmap_one = draw_structure(onestruct, po, ips_n, NULL, scaledown);	
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_one), &one_w, &one_h);
				num_w = one_w; num_dh = one_h / 2; num_uh = one_h - num_dh;
			}
			if(m.type() == STRUCT_DIVISION) {
				ips_n.wrap = m[1].needsParenthesis(po, ips_n, m, 2, flat, ips.power_depth > 0);
				den_pixmap = draw_structure(m[1], po, ips_n, &den_dh, scaledown);
			} else {
				ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 2, flat, ips.power_depth > 0);
				den_pixmap = draw_structure(m[0], po, ips_n, &den_dh, scaledown);
			}
			gdk_drawable_get_size(GDK_DRAWABLE(den_pixmap), &den_w, &h);
			den_uh = h - den_dh;
			h = 0;
			if(flat) {
				gint div_w, div_h;
				PangoLayout *layout_div = gtk_widget_create_pango_layout(resultview, NULL);
				CALCULATE_SPACE_W
				if(po.use_unicode_signs && po.division_sign == DIVISION_SIGN_DIVISION && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_DIVISION, po.can_display_unicode_string_arg))) {
					PANGO_TTP(layout_div, SIGN_DIVISION);
				} else if(po.use_unicode_signs && po.division_sign == DIVISION_SIGN_DIVISION_SLASH && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_DIVISION_SLASH, po.can_display_unicode_string_arg))) {
					PANGO_TTP(layout_div, SIGN_DIVISION_SLASH);
				} else {
					PANGO_TTP(layout_div, "/");
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
				gdk_draw_line(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - 1, w + wfr, uh - 1);
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
			GdkPixmap *pixmap_base = draw_structure(m[0], po, ips_n, &central_point, scaledown);
			gdk_drawable_get_size(GDK_DRAWABLE(pixmap_base), &base_w, &base_h);
			
			PangoLayout *layout_power = NULL;
			if(ips.power_depth > 0) {
				layout_power = gtk_widget_create_pango_layout(resultview, NULL);
				PANGO_TT_SMALL(layout_power, "^");
				pango_layout_get_pixel_size(layout_power, &power_w, &power_h);			
			}
			ips_n.power_depth++;
			ips_n.wrap = m[1].needsParenthesis(po, ips_n, m, 2, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
			PrintOptions po2 = po;
			po2.show_ending_zeroes = false;
			GdkPixmap *pixmap_exp = draw_structure(m[1], po2, ips_n, &ctmp, scaledown);
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
					h += exp_h / 3;
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
			} else {
			}
			gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_exp), 0, 0, w, 0, -1, -1);
			
			g_object_unref(pixmap_base);
			g_object_unref(pixmap_exp);
			if(layout_power) g_object_unref(layout_power);
			break;
		}
		case STRUCT_LOGICAL_AND: {
			if(!po.preserve_format && m.size() == 2 && m[0].isComparison() && m[1].isComparison() && m[0].comparisonType() != COMPARISON_EQUALS && m[0].comparisonType() != COMPARISON_NOT_EQUALS && m[1].comparisonType() != COMPARISON_EQUALS && m[1].comparisonType() != COMPARISON_NOT_EQUALS && m[0][0] == m[1][0]) {
				ips_n.depth++;
			
				vector<GdkPixmap*> pixmap_terms;
				vector<gint> hpt;
				vector<gint> wpt;
				vector<gint> cpt;
				gint sign_w, sign_h, sign2_w, sign2_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
				CALCULATE_SPACE_W
			
				hetmp = 0;		
				ips_n.wrap = m[0][1].needsParenthesis(po, ips_n, m[0], 2, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_terms.push_back(draw_structure(m[0][1], po, ips_n, &hetmp, scaledown));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[0]), &wtmp, &htmp);
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
				hetmp = 0;		
				ips_n.wrap = m[0][0].needsParenthesis(po, ips_n, m[0], 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_terms.push_back(draw_structure(m[0][0], po, ips_n, &hetmp, scaledown));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[1]), &wtmp, &htmp);
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
				hetmp = 0;		
				ips_n.wrap = m[1][1].needsParenthesis(po, ips_n, m[1], 2, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_terms.push_back(draw_structure(m[1][1], po, ips_n, &hetmp, scaledown));
				gdk_drawable_get_size(GDK_DRAWABLE(pixmap_terms[2]), &wtmp, &htmp);
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
			
				PangoLayout *layout_sign = gtk_widget_create_pango_layout(resultview, NULL);
				string str;
				TTBP(str);
				switch(m[0].comparisonType()) {
					case COMPARISON_LESS: {
						str += "&gt;";
						break;
					}
					case COMPARISON_GREATER: {
						str += "&lt;";
						break;
					}
					case COMPARISON_EQUALS_LESS: {
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_GREATER_OR_EQUAL, po.can_display_unicode_string_arg))) {
							str += SIGN_GREATER_OR_EQUAL;
						} else {
							str += "&gt;=";
						}
						break;
					}
					case COMPARISON_EQUALS_GREATER: {
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_LESS_OR_EQUAL, po.can_display_unicode_string_arg))) {
							str += SIGN_LESS_OR_EQUAL;
						} else {
							str += "&lt;=";
						}
						break;
					}
					default: {}
				}
				TTE(str);
				pango_layout_set_markup(layout_sign, str.c_str(), -1);
				pango_layout_get_pixel_size(layout_sign, &sign_w, &sign_h);
				if(sign_h / 2 > dh) {
					dh = sign_h / 2;
				}
				if(sign_h / 2 + sign_h % 2 > uh) {
					uh = sign_h / 2 + sign_h % 2;
				}
				w += sign_w;
				
				PangoLayout *layout_sign2 = gtk_widget_create_pango_layout(resultview, NULL);
				str = "";
				TTBP(str);
				switch(m[1].comparisonType()) {
					case COMPARISON_GREATER: {
						str += "&gt;";
						break;
					}
					case COMPARISON_LESS: {
						str += "&lt;";
						break;
					}
					case COMPARISON_EQUALS_GREATER: {
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_GREATER_OR_EQUAL, po.can_display_unicode_string_arg))) {
							str += SIGN_GREATER_OR_EQUAL;
						} else {
							str += "&gt;=";
						}
						break;
					}
					case COMPARISON_EQUALS_LESS: {
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_LESS_OR_EQUAL, po.can_display_unicode_string_arg))) {
							str += SIGN_LESS_OR_EQUAL;
						} else {
							str += "&lt;=";
						}
						break;
					}
					default: {}
				}
				TTE(str);
				pango_layout_set_markup(layout_sign2, str.c_str(), -1);
				pango_layout_get_pixel_size(layout_sign2, &sign2_w, &sign2_h);
				if(sign2_h / 2 > dh) {
					dh = sign2_h / 2;
				}
				if(sign2_h / 2 + sign2_h % 2 > uh) {
					uh = sign2_h / 2 + sign2_h % 2;
				}
				w += sign2_w;
				
			
				w += space_w * (pixmap_terms.size() - 1) * 2;
			
				central_point = dh;
				h = dh + uh;
				pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
				draw_background(pixmap, w, h);
				w = 0;
				for(size_t i = 0; i < pixmap_terms.size(); i++) {
					if(i > 0) {
						w += space_w;
						if(i == 1) {
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - sign_h / 2 - sign_h % 2, layout_sign);
							w += sign_w;
						} else {
							gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - sign2_h / 2 - sign2_h % 2, layout_sign2);
							w += sign2_w;
						}
						w += space_w;			
					}
					gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_terms[i]), 0, 0, w, uh - (hpt[i] - cpt[i]), -1, -1);
					w += wpt[i];
					g_object_unref(pixmap_terms[i]);
				}
				g_object_unref(layout_sign);
				g_object_unref(layout_sign2);
				break;
			}
		}
		case STRUCT_COMPARISON: {}		
		case STRUCT_LOGICAL_XOR: {}
		case STRUCT_LOGICAL_OR: {}
		case STRUCT_BITWISE_AND: {}
		case STRUCT_BITWISE_XOR: {}
		case STRUCT_BITWISE_OR: {
			
			ips_n.depth++;
			
			vector<GdkPixmap*> pixmap_terms;
			vector<gint> hpt;
			vector<gint> wpt;
			vector<gint> cpt;
			gint sign_w, sign_h, wtmp, htmp, hetmp = 0, w = 0, h = 0, dh = 0, uh = 0;
			CALCULATE_SPACE_W
			
			for(size_t i = 0; i < m.size(); i++) {
				hetmp = 0;		
				ips_n.wrap = m[i].needsParenthesis(po, ips_n, m, i + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_terms.push_back(draw_structure(m[i], po, ips_n, &hetmp, scaledown));
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
			TTBP(str);
			if(m.type() == STRUCT_COMPARISON) {
				switch(m.comparisonType()) {
					case COMPARISON_EQUALS: {
						if(ips.depth == 0 && po.use_unicode_signs && ((po.is_approximate && *po.is_approximate) || m.isApproximate()) && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_ALMOST_EQUAL, po.can_display_unicode_string_arg))) {
							str += SIGN_ALMOST_EQUAL;
						} else {
							str += "=";
						}
						break;
					}
					case COMPARISON_NOT_EQUALS: {
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_NOT_EQUAL, po.can_display_unicode_string_arg))) {
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
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_GREATER_OR_EQUAL, po.can_display_unicode_string_arg))) {
							str += SIGN_GREATER_OR_EQUAL;
						} else {
							str += "&gt;=";
						}
						break;
					}
					case COMPARISON_EQUALS_LESS: {
						if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_LESS_OR_EQUAL, po.can_display_unicode_string_arg))) {
							str += SIGN_LESS_OR_EQUAL;
						} else {
							str += "&lt;=";
						}
						break;
					}
				}
			} else if(m.type() == STRUCT_LOGICAL_AND) {
				if(po.spell_out_logical_operators) str += _("and");
				else str += "&amp;&amp;";
			} else if(m.type() == STRUCT_LOGICAL_OR) {
				if(po.spell_out_logical_operators) str += _("or");
				else str += "||";
			} else if(m.type() == STRUCT_LOGICAL_XOR) {
				str += "XOR";
			} else if(m.type() == STRUCT_BITWISE_AND) {
				str += "&amp;";
			} else if(m.type() == STRUCT_BITWISE_OR) {
				str += "|";
			} else if(m.type() == STRUCT_BITWISE_XOR) {
				str += "XOR";
			}
			
			TTE(str);
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
			for(size_t i = 0; i < pixmap_terms.size(); i++) {
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
		case STRUCT_LOGICAL_NOT: {}
		case STRUCT_BITWISE_NOT: {

			ips_n.depth++;

			gint not_w, not_h, uh, dh, h, w, ctmp, htmp, wtmp, hpa, cpa, wpa;
			
			PangoLayout *layout_not = gtk_widget_create_pango_layout(resultview, NULL);
			
			if(m.type() == STRUCT_LOGICAL_NOT) {
				PANGO_TTP(layout_not, "!");
			} else {
				PANGO_TTP(layout_not, "~");
			}
			pango_layout_get_pixel_size(layout_not, &not_w, &not_h);

			w = not_w + 1;
			uh = not_h / 2 + not_h % 2;
			dh = not_h / 2;
			
			ips_n.wrap = m[0].needsParenthesis(po, ips_n, m, 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
			GdkPixmap *pixmap_arg = draw_structure(m[0], po, ips_n, &ctmp, scaledown);
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
				if(m[0].size() == 0) {
					PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
					string str;
					TTBP(str)
					str += "[ ]";
					TTE(str)
					pango_layout_set_markup(layout, str.c_str(), -1);
					PangoRectangle rect;
					pango_layout_get_pixel_size(layout, &w, &h);
					pango_layout_get_pixel_extents(layout, &rect, NULL);
					w = rect.width + rect.x;
					w += 1;
					central_point = h / 2;
					pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
					draw_background(pixmap, w, h);
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
					g_object_unref(layout);
					break;
				}
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
				TTP(str, po.comma())
				pango_layout_set_markup(layout_comma, str.c_str(), -1);		
				pango_layout_get_pixel_size(layout_comma, &comma_w, &comma_h);
				for(size_t index_r = 0; index_r < m.size(); index_r++) {
					for(size_t index_c = 0; index_c < m[index_r].size(); index_c++) {
						ctmp = 0;
						ips_n.wrap = m[index_r][index_c].needsParenthesis(po, ips_n, m, index_r + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
						pixmap_elements[index_r].push_back(draw_structure(m[index_r][index_c], po, ips_n, &ctmp, scaledown));
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
				for(size_t i = 0; i < col_w.size(); i++) {
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
				for(size_t index_r = 0; index_r < m.size(); index_r++) {
					w = wll + 1;
					for(size_t index_c = 0; index_c < m[index_r].size(); index_c++) {
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
			TTP(str, CALCULATOR->getComma())
			pango_layout_set_markup(layout_comma, str.c_str(), -1);		
			pango_layout_get_pixel_size(layout_comma, &comma_w, &comma_h);

			if(m.size() == 0) {
				PangoLayout *layout_one = gtk_widget_create_pango_layout(resultview, NULL);
				TTP(str, "1")
				pango_layout_set_markup(layout_one, str.c_str(), -1);
				pango_layout_get_pixel_size(layout_one, &w, &h);
				uh = h / 2 + h % 2;
				dh = h / 2;
				w = 2;
				g_object_unref(layout_one);
			}
			for(size_t index = 0; index < m.size(); index++) {
				ips_n.wrap = m[index].needsParenthesis(po, ips_n, m, index + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_args.push_back(draw_structure(m[index], po, ips_n, &ctmp, scaledown));
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
		
			if(dh > uh) uh = dh;
			h = uh + dh;
			central_point = dh;
			arc_h = dh * 2;
			arc_w = (int) ::sqrt((double) arc_h * 3);
			w += arc_w * 2;
			w += 1;

			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);			
			draw_background(pixmap, w, h);

			w = 0;
			gdk_draw_pixbuf(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], get_left_parenthesis(arc_w, arc_h, scaledown), 0, 0, w, uh - arc_h / 2 - arc_h % 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			w += arc_w;
			if(m.size() == 0) w += 2;
			for(size_t index = 0; index < m.size(); index++) {
				if(index > 0) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - comma_h / 2 - comma_h % 2, layout_comma);	
					w += comma_w;
					w += space_w;
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_args[index]), 0, 0, w, uh - (hpa[index] - cpa[index]), -1, -1);
				w += wpa[index];
				g_object_unref(pixmap_args[index]);
			}
			gdk_draw_pixbuf(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], get_right_parenthesis(arc_w, arc_h, scaledown), 0, 0, w, uh - arc_h / 2 - arc_h % 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			
			g_object_unref(layout_comma);

			break;
		}
		case STRUCT_UNIT: {
			string str, str2;
			TTBP(str);
			
			const ExpressionName *ename = &m.unit()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, m.isPlural(), po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg);
			if(m.prefix()) {
				str += m.prefix()->name(po.abbreviate_names && ename->abbreviation && (ename->suffix || ename->name.find("_") == string::npos), po.use_unicode_signs, po.can_display_unicode_string_function, po.can_display_unicode_string_arg);
			}
			if(ename->suffix && ename->name.length() > 1) {
				size_t i = ename->name.rfind('_');
				bool b = i == string::npos || i == ename->name.length() - 1 || i == 0;
				size_t i2 = 1;
				if(b) {
					if(is_in(NUMBERS, ename->name[ename->name.length() - 1])) {
						while(ename->name.length() > i2 + 1 && is_in(NUMBERS, ename->name[ename->name.length() - 1 - i2])) {
							i2++;
						}
					}
					str += ename->name.substr(0, ename->name.length() - i2);
				} else {
					str += ename->name.substr(0, i);
				}
				TTBP_SMALL(str);
				str += "<sub>";
				if(b) str += ename->name.substr(ename->name.length() - i2, i2);
				else str += ename->name.substr(i + 1, ename->name.length() - (i + 1));
				str += "</sub>";
				TTE(str);
			} else {
				str += ename->name;
			}
			gsub("_", " ", str);

			TTE(str);
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
			
			if(m.variable() != CALCULATOR->v_i) {
				str = "<i>";
			}
			TTBP(str);
			
			const ExpressionName *ename = &m.variable()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg);
			if(ename->suffix && ename->name.length() > 1) {
				size_t i = ename->name.rfind('_');
				bool b = i == string::npos || i == ename->name.length() - 1 || i == 0;
				size_t i2 = 1;
				if(b) {
					if(is_in(NUMBERS, ename->name[ename->name.length() - 1])) {
						while(ename->name.length() > i2 + 1 && is_in(NUMBERS, ename->name[ename->name.length() - 1 - i2])) {
							i2++;
						}
					}
					str += ename->name.substr(0, ename->name.length() - i2);
				} else {
					str += ename->name.substr(0, i);
				}
				TTBP_SMALL(str);
				str += "<sub>";
				if(b) str += ename->name.substr(ename->name.length() - i2, i2);
				else str += ename->name.substr(i + 1, ename->name.length() - (i + 1));
				str += "</sub>";
				TTE(str);
			} else {
				str += ename->name;
			}
			gsub("_", " ", str);
			
			TTE(str);
			if(m.variable() != CALCULATOR->v_i) {
				str += "</i>";
			} 
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width + rect.x;
			w += 1;
			if(m.variable() == CALCULATOR->v_i) w += 1;
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
			string str;
			TTP(str, po.comma())
			pango_layout_set_markup(layout_comma, str.c_str(), -1);		
			pango_layout_get_pixel_size(layout_comma, &comma_w, &comma_h);
			PangoLayout *layout_function = gtk_widget_create_pango_layout(resultview, NULL);
		
			str = "";	
			TTBP(str);
			
			const ExpressionName *ename = &m.function()->preferredDisplayName(po.abbreviate_names, po.use_unicode_signs, false, po.use_reference_names, po.can_display_unicode_string_function, po.can_display_unicode_string_arg);
			if(ename->suffix && ename->name.length() > 1) {
				size_t i = ename->name.rfind('_');
				bool b = i == string::npos || i == ename->name.length() - 1 || i == 0;
				size_t i2 = 1;
				if(b) {
					if(is_in(NUMBERS, ename->name[ename->name.length() - 1])) {
						while(ename->name.length() > i2 + 1 && is_in(NUMBERS, ename->name[ename->name.length() - 1 - i2])) {
							i2++;
						}
					}
					str += ename->name.substr(0, ename->name.length() - i2);
				} else {
					str += ename->name.substr(0, i);
				}
				TTBP_SMALL(str);
				str += "<sub>";
				if(b) str += ename->name.substr(ename->name.length() - i2, i2);
				else str += ename->name.substr(i + 1, ename->name.length() - (i + 1));
				str += "</sub>";
				TTE(str);
			} else {
				str += ename->name;
			}
			gsub("_", " ", str);
			
			TTE(str);
			
			pango_layout_set_markup(layout_function, str.c_str(), -1);			
			pango_layout_get_pixel_size(layout_function, &function_w, &function_h);
			w = function_w + 1;
			uh = function_h / 2 + function_h % 2;
			dh = function_h / 2;

			for(size_t index = 0; index < m.size(); index++) {
				ips_n.wrap = m[index].needsParenthesis(po, ips_n, m, index + 1, ips.division_depth > 0 || ips.power_depth > 0, ips.power_depth > 0);
				pixmap_args.push_back(draw_structure(m[index], po, ips_n, &ctmp, scaledown));
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
			
			if(dh > uh) uh = dh;
			h = uh + dh;
			central_point = dh;
			arc_h = dh * 2;
			arc_w = (int) ::sqrt((double) arc_h * 3);
			w += arc_w * 2;
			w += 1;

			pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			draw_background(pixmap, w, h);
			
			w = 0;
			gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - function_h / 2 - function_h % 2, layout_function);	
			w += function_w;
			gdk_draw_pixbuf(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], get_left_parenthesis(arc_w, arc_h, scaledown), 0, 0, w, uh - arc_h / 2 - arc_h % 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			w += arc_w;
			for(size_t index = 0; index < m.size(); index++) {
				if(index > 0) {
					gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], w, uh - comma_h / 2 - comma_h % 2, layout_comma);	
					w += comma_w;
					w += space_w;
				}
				gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_args[index]), 0, 0, w, uh - (hpa[index] - cpa[index]), -1, -1);
				w += wpa[index];
				g_object_unref(pixmap_args[index]);
			}
			gdk_draw_pixbuf(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], get_right_parenthesis(arc_w, arc_h, scaledown), 0, 0, w, uh - arc_h / 2 - arc_h % 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
						
			g_object_unref(layout_comma);
			g_object_unref(layout_function);

			break;
		}
		case STRUCT_UNDEFINED: {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			string str;
			TTP(str, _("undefined"));
			pango_layout_set_markup(layout, str.c_str(), -1);
			PangoRectangle rect;
			pango_layout_get_pixel_size(layout, &w, &h);
			pango_layout_get_pixel_extents(layout, &rect, NULL);
			w = rect.width + rect.x;
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
		gint arc_base_h = central_point * 2;
		if(h < arc_base_h) h = arc_base_h;
		gint arc_base_w = (int) ::sqrt((double) arc_base_h * 3);
		//base_h += 4;
		//central_point += 2;
		w += arc_base_w * 2;
		GdkPixmap *pixmap_old = pixmap;
		pixmap = gdk_pixmap_new(resultview->window, w, h, -1);	
		draw_background(pixmap, w, h);
		w = 0;
		gdk_draw_pixbuf(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], get_left_parenthesis(arc_base_w, arc_base_h, scaledown), 0, 0, w, h - arc_base_h, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
		w += arc_base_w;
		gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_old), 0, 0, w, (h - base_h) / 2, -1, -1);
		w += base_w;
		gdk_draw_pixbuf(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], get_right_parenthesis(arc_base_w, arc_base_h, scaledown), 0, 0, w, h - arc_base_h, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
		g_object_unref(pixmap_old);
	}
	if(ips.depth == 0 && !(m.isComparison() && (!((po.is_approximate && *po.is_approximate) || m.isApproximate()) || (m.comparisonType() == COMPARISON_EQUALS && po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_ALMOST_EQUAL, po.can_display_unicode_string_arg))))) && pixmap) {
		gint w, h, wle, hle, w_new, h_new;
		gdk_drawable_get_size(GDK_DRAWABLE(pixmap), &w, &h);			
		GdkPixmap *pixmap_old = pixmap;
		PangoLayout *layout_equals = gtk_widget_create_pango_layout(resultview, NULL);
		if((po.is_approximate && *po.is_approximate) || m.isApproximate()) {
			if(po.use_unicode_signs && (!po.can_display_unicode_string_function || (*po.can_display_unicode_string_function) (SIGN_ALMOST_EQUAL, po.can_display_unicode_string_arg))) {
				PANGO_TT(layout_equals, SIGN_ALMOST_EQUAL);
			} else {
				
				string str;
				TT(str, _("approx."));
				pango_layout_set_markup(layout_equals, str.c_str(), -1);
			}
		} else {
			PANGO_TT(layout_equals, "=");
		}
		CALCULATE_SPACE_W
		pango_layout_get_pixel_size(layout_equals, &wle, &hle);
		w_new = w + wle + 1 + space_w;
		h_new = h;
		pixmap = gdk_pixmap_new(resultview->window, w_new, h_new, -1);			
		draw_background(pixmap, w_new, h_new);
		gdk_draw_layout(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, h - central_point - hle / 2 - hle % 2, layout_equals);	
		gdk_draw_drawable(GDK_DRAWABLE(pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], GDK_DRAWABLE(pixmap_old), 0, 0, wle + 1 + space_w, 0, -1, -1);
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
	if(pixbuf_result) {
		gdk_window_clear(resultview->window);
		gdk_pixbuf_unref(pixbuf_result);
		pixbuf_result = NULL;
		gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_save_image"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "popup_menu_item_save_image"), FALSE);
	}
}

void on_abort_display(GtkDialog*, gint, gpointer) {
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
		int scale_n = 0;
		fread(&scale_n, sizeof(int), 1, view_pipe);
		fread(&x, sizeof(void*), 1, view_pipe);
		MathStructure m(*((MathStructure*) x));
		bool b_stack = false;
		fread(&b_stack, sizeof(bool), 1, view_pipe);
		fread(&x, sizeof(void*), 1, view_pipe);
		MathStructure *mm = (MathStructure*) x;
		if(scale_n == 0) {
			fread(&x, sizeof(void*), 1, view_pipe);
			printops.can_display_unicode_string_arg = (void*) historyview;
			if(x) {
				PrintOptions po;
				po.preserve_format = true;
				po.show_ending_zeroes = true;
				po.lower_case_e = printops.lower_case_e;
				po.lower_case_numbers = printops.lower_case_numbers;
				po.base_display = printops.base_display;
				po.abbreviate_names = false;
				po.use_unicode_signs = printops.use_unicode_signs;
				po.multiplication_sign = printops.multiplication_sign;
				po.division_sign = printops.division_sign;
				po.short_multiplication = false;
				po.excessive_parenthesis = true;
				po.improve_division_multipliers = false;
				po.can_display_unicode_string_function = &can_display_unicode_string_function;
				po.can_display_unicode_string_arg = (void*) statuslabel_l;
				po.spell_out_logical_operators = printops.spell_out_logical_operators;
				po.restrict_to_parent_precision = false;
				MathStructure mp(*((MathStructure*) x));								
				fread(&po.is_approximate, sizeof(bool*), 1, view_pipe);
				mp.format(po);
				parsed_text = mp.print(po);
				fread(&x, sizeof(void*), 1, view_pipe);
				mp.set(*((MathStructure*) x));
				if(!mp.isUndefined()) {
					parsed_text += CALCULATOR->localToString();
					mp.format(po);
					parsed_text += mp.print(po); 
				}
			}			
		}
		printops.allow_non_usable = false;

		if(mm && m.isMatrix()) {
			mm->set(m);
			MathStructure mm2(m);
			string mstr;
			int c = mm->columns(), r = mm->rows();
			for(int index_r = 0; index_r < r; index_r++) {
				for(int index_c = 0; index_c < c; index_c++) {
					mm->getElement(index_r + 1, index_c + 1)->set(_("aborted"));
				}
			}
			for(int index_r = 0; index_r < r; index_r++) {
				for(int index_c = 0; index_c < c; index_c++) {
					mm2.getElement(index_r + 1, index_c + 1)->format(printops);
					mstr = mm2.getElement(index_r + 1, index_c + 1)->print(printops);
					mm->getElement(index_r + 1, index_c + 1)->set(mstr);
				}
			}
		}

		m.format(printops);
		if(scale_n == 0) result_text = m.print(printops);
		printops.can_display_unicode_string_arg = NULL;
	
		if(!b_stack && result_text.length() > 1500) {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			int index = 0;
			//GtkTextView can not display very very long lines
			while(result_text.length() - index > 2500) {								
				size_t index_utf8 = 2500 + index;
				size_t space_index = 0;
				while(space_index < 100 && result_text[index_utf8 - space_index] != ' ') {
					space_index++;
				}
				if(space_index < 100) {
					index_utf8 -= space_index;
				} else {
					while(result_text[index_utf8] < 0 && (unsigned char) result_text[index_utf8] < 0xC2) {
						index_utf8--;
					}
				}
				result_text.insert(index_utf8, "\n");
				index = index_utf8;
			}
			pango_layout_set_markup(layout, _("result is too long\nsee history"), -1);
			gint w = 0, h = 0;
			pango_layout_get_pixel_size(layout, &w, &h);
			tmp_pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			gdk_draw_rectangle(tmp_pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
			gdk_draw_layout(GDK_DRAWABLE(tmp_pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			*printops.is_approximate = false;
		} else if(!b_stack && result_text == _("aborted")) {
			PangoLayout *layout = gtk_widget_create_pango_layout(resultview, NULL);
			pango_layout_set_markup(layout, _("calculation was aborted"), -1);
			gint w = 0, h = 0;
			pango_layout_get_pixel_size(layout, &w, &h);
			tmp_pixmap = gdk_pixmap_new(resultview->window, w, h, -1);
			gdk_draw_rectangle(tmp_pixmap, resultview->style->bg_gc[GTK_WIDGET_STATE(resultview)], TRUE, 0, 0, w, h);	
			gdk_draw_layout(GDK_DRAWABLE(tmp_pixmap), resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], 1, 0, layout);	
			g_object_unref(layout);
			*printops.is_approximate = false;
		} else if(!b_stack) {
			printops.allow_non_usable = true;
			printops.can_display_unicode_string_arg = (void*) resultview;
			
			tmp_pixmap = draw_structure(m, printops, top_ips, NULL, scale_n);
			
			printops.can_display_unicode_string_arg = NULL;
			printops.allow_non_usable = false;
		}
		b_busy = false;
	}
	return NULL;
}
gboolean on_event(GtkWidget*, GdkEvent *e, gpointer) {
	if(e->type == GDK_EXPOSE || e->type == GDK_PROPERTY_NOTIFY || e->type == GDK_CONFIGURE || e->type == GDK_FOCUS_CHANGE || e->type == GDK_VISIBILITY_NOTIFY) {
		return FALSE;
	}
	return TRUE;
}
/*
	set result in result widget and add to history widget
*/
void setResult(Prefix *prefix, bool update_history, bool update_parse, bool force, string transformation, size_t stack_index, bool register_moved) {

	if(block_result_update) return;

	if(expression_has_changed && !rpn_mode) {
		if(!force) return;
		execute_expression();
		if(!prefix) return;
	}

	if(!rpn_mode) stack_index = 0;
	if(stack_index != 0) {
		update_history = true;
		update_parse = false;
	}
	if(register_moved) {
		update_history = true;
		update_parse = false;
	}

	do_timeout = false;
	b_busy = true;

	GtkTextIter iter;
	GtkTextBuffer *tb = NULL;
	
	int inhistory_index = 0;
	if(update_history) {
		tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history")));
		if(update_parse || register_moved) {
			if(register_moved) {
				result_text = _("RPN Register Moved");
				inhistory_type.push_front(QALCULATE_HISTORY_REGISTER_MOVED);
				inhistory.push_front("");
			} else {
				remove_blank_ends(result_text);
				gsub("\n", " ", result_text);
				inhistory_type.push_front(QALCULATE_HISTORY_EXPRESSION);
				inhistory.push_front(result_text);
			}
			gtk_text_buffer_get_start_iter(tb, &iter);
			gtk_text_buffer_insert(tb, &iter, result_text.c_str(), -1);
			inhistory_index++;
			gtk_text_buffer_insert(tb, &iter, " ", -1);
			gtk_text_buffer_insert_with_tags_by_name(tb, &iter, "\n", -1, "history_separator", NULL);
			gtk_text_buffer_get_iter_at_line_index(tb, &iter, 0, result_text.length() + 1);
		} else if(initial_result_pos) {
			gtk_text_buffer_get_iter_at_mark(tb, &iter, initial_result_pos);
			inhistory_index = initial_result_index;
			if(!transformation.empty()) {
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, transformation.c_str(), -1, "history_transformation", NULL);
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, ":  ", -1, "history_transformation", NULL);
				inhistory.insert(inhistory.begin() + inhistory_index, transformation);
				inhistory_type.insert(inhistory_type.begin() + inhistory_index, QALCULATE_HISTORY_TRANSFORMATION);
				inhistory_index++;
			}
		} else {
			b_busy = false;
			do_timeout = true;
			return;
		}
		result_text = "?";
	}
	if(update_parse) {
		parsed_text = "aborted";
	}

	if(stack_index == 0) {
		clearresult();
	}

	int scale_n = 0;
	GtkWidget *dialog = NULL;
	CALCULATOR->saveState();
	gint w = 0, h = 0;
	bool parsed_approx = false;
	while(true) {

		if(stack_index == 0) {
			if(pixmap_result) {
				g_object_unref(pixmap_result);
			}
			pixmap_result = NULL;
			if(pixbuf_result) {
				gdk_pixbuf_unref(pixbuf_result);
			}
			pixbuf_result = NULL;
			gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_save_image"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "popup_menu_item_save_image"), FALSE);
		}

		printops.prefix = prefix;
		tmp_pixmap = NULL;		

		gulong handler_id = g_signal_connect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), "event", G_CALLBACK(on_event), NULL);

		fwrite(&scale_n, sizeof(int), 1, view_pipe_w);
		if(stack_index == 0) {
			fwrite(&mstruct, sizeof(void*), 1, view_pipe_w);
		} else {
			MathStructure *mreg = CALCULATOR->getRPNRegister(stack_index + 1);
			fwrite(&mreg, sizeof(void*), 1, view_pipe_w);
		}
		bool b_stack = stack_index != 0;
		fwrite(&b_stack, sizeof(bool), 1, view_pipe_w);
		if(scale_n == 0) {
			if(b_stack) {
				void *x = NULL;
				fwrite(&x, sizeof(void*), 1, view_pipe_w);
			} else {
				matrix_mstruct->clear();
				fwrite(&matrix_mstruct, sizeof(void*), 1, view_pipe_w);
			}
			if(update_parse) {
				fwrite(&parsed_mstruct, sizeof(void*), 1, view_pipe_w);
				bool *parsed_approx_p = &parsed_approx;
				fwrite(&parsed_approx_p, sizeof(void*), 1, view_pipe_w);
				fwrite(&parsed_tostruct, sizeof(void*), 1, view_pipe_w);
			} else {
				void *x = NULL;
				fwrite(&x, sizeof(void*), 1, view_pipe_w);
			}
		} else {
			void *x = NULL;
			fwrite(&x, sizeof(void*), 1, view_pipe_w);
		}
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

		if(b_busy) {
			if(!dialog) {
				dialog = glade_xml_get_widget (main_glade, "progress_dialog");
				gtk_window_set_title(GTK_WINDOW(dialog), _("Processing..."));
				gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (main_glade, "progress_label_message")), _("Processing..."));
				gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
				g_signal_connect(GTK_OBJECT(dialog), "response", G_CALLBACK(on_abort_display), NULL);
			}
			gtk_widget_show(dialog);
		}
		rtime.tv_nsec = 100000000;
		while(b_busy) {
			while(gtk_events_pending()) gtk_main_iteration();
			nanosleep(&rtime, NULL);
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(glade_xml_get_widget (main_glade, "progress_progressbar")));
		}

		b_busy = true;

		g_signal_handler_disconnect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), handler_id);

		if(stack_index == 0) {

			bool was_aborted = !tmp_pixmap;
			if(was_aborted) {
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
			gtk_widget_set_size_request(resultview, w, h);
			while(gtk_events_pending()) gtk_main_iteration();
			if(!was_aborted && scale_n < 3 && h > glade_xml_get_widget(main_glade, "resultport")->allocation.height) {
				int scroll_diff = glade_xml_get_widget(main_glade, "scrolled_result")->allocation.height - glade_xml_get_widget(main_glade, "resultport")->allocation.height - 8;
				double scale_div = (double) h / (glade_xml_get_widget(main_glade, "resultport")->allocation.height + scroll_diff);
				if(scale_div > 1.44) {
					scale_n = 3;
				} else if(scale_n < 2 && scale_div > 1.2) {
					scale_n = 2;
				} else {
					scale_n++;
				}
			} else {
				break;
			}
		} else {
			break;
		}
	}
	if(dialog) {
		gtk_widget_hide(dialog);
	}

	if(stack_index == 0) {
		GdkPixbuf *pixbuf_result_tmp = gdk_pixbuf_get_from_drawable(NULL, tmp_pixmap, NULL, 0, 0, 0, 0, -1, -1);
		if(pixbuf_result_tmp) {
			guchar *pixels = gdk_pixbuf_get_pixels(pixbuf_result_tmp);
			pixbuf_result = gdk_pixbuf_add_alpha(pixbuf_result_tmp, TRUE, pixels[0], pixels[1], pixels[2]);
			gdk_pixbuf_unref(pixbuf_result_tmp);
			if(pixbuf_result) {
				if(resultview->allocation.width - 15 > w) {
					gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, resultview->allocation.width - w - 15, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
				} else {
					gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
				}
				gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_save_image"), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "popup_menu_item_save_image"), TRUE);
			}
		}
		pixmap_result = tmp_pixmap;
	}
	if(register_moved) {
		update_parse = true;
		parsed_text = result_text;
	}
	if(stack_index != 0) {
		if(result_text.length() > 500000) {
			result_text = "(...)";
		}
		display_errors(NULL, glade_xml_get_widget (main_glade, "main_window"));
		RPNRegisterChanged(result_text, stack_index);
	} else if(update_history) {
		if(result_text.length() > 500000) {
			result_text = "(...)";
		}
		if(parsed_text.length() > 500000) {
			parsed_text = "(...)";
		}
		if(update_parse) {
			string str = "  ";
			if(!parsed_approx) {
				str += "=";
				inhistory_type.insert(inhistory_type.begin() + inhistory_index, QALCULATE_HISTORY_PARSE);
			} else {
				if(printops.use_unicode_signs && can_display_unicode_string_function(SIGN_ALMOST_EQUAL, (void*) historyview)) {
					str += SIGN_ALMOST_EQUAL;
				} else {
					str += _("approx.");
				}
				inhistory_type.insert(inhistory_type.begin() + inhistory_index, QALCULATE_HISTORY_PARSE_APPROXIMATE);
			}
			str += " ";
			str += parsed_text;
			gtk_text_buffer_insert_with_tags_by_name(tb, &iter, str.c_str(), -1, "history_parse", NULL);
			inhistory.insert(inhistory.begin() + inhistory_index, parsed_text);
			inhistory_index++;
			gtk_text_buffer_insert(tb, &iter, "\n", -1);
		}

		display_errors(&iter, glade_xml_get_widget (main_glade, "main_window"), &inhistory_index);
		if(rpn_mode && !register_moved) {
			RPNRegisterChanged(result_text, stack_index);
		}
		string str;
		bool b_approx = *printops.is_approximate || mstruct->isApproximate() || (!update_parse && prev_result_approx);
		if(!b_approx) {
			str = "=";	
		} else {
			if(printops.use_unicode_signs && can_display_unicode_string_function(SIGN_ALMOST_EQUAL, (void*) historyview)) {
				str = SIGN_ALMOST_EQUAL;		
			} else {
				str = "= ";
				str += _("approx.");
			}
		}
		str += " ";
		if(update_parse) {
			gtk_text_buffer_insert(tb, &iter, str.c_str(), -1);
			gtk_text_buffer_insert_with_tags_by_name(tb, &iter, result_text.c_str(), -1, "history_result", NULL);
			gtk_text_buffer_insert(tb, &iter, "\n", -1);
			initial_result_pos = gtk_text_buffer_create_mark(tb, NULL, &iter, FALSE);
		} else {
			gtk_text_buffer_insert(tb, &iter, str.c_str(), -1);
			gtk_text_buffer_insert_with_tags_by_name(tb, &iter, result_text.c_str(), -1, "history_result", NULL);
			gtk_text_buffer_insert(tb, &iter, "\n", -1);
		}
		inhistory.insert(inhistory.begin() + inhistory_index, result_text);
		initial_result_index = inhistory_index + 1;
		if(b_approx) {
			inhistory_type.insert(inhistory_type.begin() + inhistory_index, QALCULATE_HISTORY_RESULT_APPROXIMATE);
		} else {
			inhistory_type.insert(inhistory_type.begin() + inhistory_index, QALCULATE_HISTORY_RESULT);
		}
		gtk_text_buffer_place_cursor(tb, &iter);
		prev_result_approx = *printops.is_approximate;
	}
	printops.prefix = NULL;
	b_busy = false;
	display_errors(NULL, glade_xml_get_widget (main_glade, "main_window"));

	if(!register_moved && stack_index == 0 && mstruct->isMatrix() && (mstruct->rows() > 4 || mstruct->columns() > 4) && matrix_mstruct->isMatrix()) {
		while(gtk_events_pending()) gtk_main_iteration();
		gtk_widget_grab_focus(expression);
		if(update_history && update_parse && force) gtk_editable_select_region(GTK_EDITABLE(expression), 0, -1);
		insert_matrix(matrix_mstruct, glade_xml_get_widget (main_glade, "main_window"), false, true, true);
	}

	do_timeout = true;

}

void on_abort_command(GtkDialog*, gint, gpointer) {
	pthread_cancel(command_thread);
	CALCULATOR->restoreState();
	CALCULATOR->clearBuffers();
	b_busy = false;
	command_aborted = true;
	command_thread_started = false;
}

void *command_proc(void *pipe) {

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	
	FILE *command_pipe = (FILE*) pipe;
	
	while(true) {
	
		void *x = NULL;
		int command_type;
		fread(&command_type, sizeof(int), 1, command_pipe);
		fread(&x, sizeof(void*), 1, command_pipe);
		switch(command_type) {
			case COMMAND_FACTORIZE: {
				if(!((MathStructure*) x)->integerFactorize()) {
					((MathStructure*) x)->factorize(evalops);
				}
				break;
			}
			case COMMAND_SIMPLIFY: {
				((MathStructure*) x)->simplify(evalops);
				break;
			}
		}
		b_busy = false;
		
	}
	return NULL;
}

void executeCommand(int command_type) {

	if(expression_has_changed && !rpn_mode) {
		execute_expression();
	}

	do_timeout = false;
	b_busy = true;	
	command_aborted = false;
	GtkWidget *dialog = NULL;
	CALCULATOR->saveState();

	if(!command_thread_started) {
		pthread_create(&command_thread, &command_thread_attr, command_proc, command_pipe_r);
		command_thread_started = true;
	}

	gulong handler_id = g_signal_connect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), "event", G_CALLBACK(on_event), NULL);

	fwrite(&command_type, sizeof(int), 1, command_pipe_w);
	MathStructure *mfactor = new MathStructure(*mstruct);
	fwrite(&mfactor, sizeof(void*), 1, command_pipe_w);
	
	fflush(command_pipe_w);

	struct timespec rtime;
	rtime.tv_sec = 0;
	rtime.tv_nsec = 10000000;
	int i = 0;
	while(b_busy && i < 50) {
		nanosleep(&rtime, NULL);
		i++;
	}
	i = 0;
	
	if(b_busy) {
		string progress_str;
		switch(command_type) {
			case COMMAND_FACTORIZE: {
				progress_str = _("Factorizing...");
				break;
			}
			case COMMAND_SIMPLIFY: {
				progress_str = _("Simplifying...");
				break;
			}
		}
		dialog = glade_xml_get_widget (main_glade, "progress_dialog");
		gtk_window_set_title(GTK_WINDOW(dialog), progress_str.c_str());
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (main_glade, "progress_label_message")), progress_str.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
		g_signal_connect(GTK_OBJECT(dialog), "response", G_CALLBACK(on_abort_command), NULL);
		gtk_widget_show(dialog);
	}
	rtime.tv_nsec = 100000000;
	while(b_busy) {
		while(gtk_events_pending()) gtk_main_iteration();
		nanosleep(&rtime, NULL);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(glade_xml_get_widget (main_glade, "progress_progressbar")));
	}

	b_busy = true;

	g_signal_handler_disconnect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), handler_id);
	
	if(dialog) {
		gtk_widget_hide(dialog);
	}
	b_busy = false;
	
	if(!command_aborted) {
		mstruct->unref();
		mstruct = mfactor;
		switch(command_type) {
			case COMMAND_FACTORIZE: {
				printops.allow_factorization = true;
				break;
			}
			case COMMAND_SIMPLIFY: {
				printops.allow_factorization = false;
				break;
			}
		}
		setResult(NULL, true, false, true);
	}
		
	do_timeout = true;

}

void result_display_updated() {
	setResult(NULL, false, false, false);
	update_status_text();
}
void result_format_updated() {
	setResult(NULL, true, false, false);
	update_status_text();
}
void result_action_executed() {
	//display_errors(NULL, glade_xml_get_widget (main_glade, "main_window"));
	printops.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
	setResult(NULL, true, false, true);
}
void result_prefix_changed(Prefix *prefix) {
	setResult(prefix, true, false, true);
}
void expression_calculation_updated() {
	expression_has_changed2 = true;
	display_parse_status();
	if(!rpn_mode) execute_expression(false);
	update_status_text();
}
void expression_format_updated(bool recalculate) {
	expression_has_changed2 = true;
	if(rpn_mode) recalculate = false;
	display_parse_status();
	if(!expression_has_changed && !recalculate && !rpn_mode) {
		clearresult();
	}
	if(recalculate) {
		execute_expression(false);
	}
	update_status_text();
}


void set_prefix(GtkMenuItem*, gpointer user_data) {
	//mstruct->clearPrefixes();
	result_prefix_changed((Prefix*) user_data);
	focus_keeping_selection();
}

void on_abort_calculation(GtkDialog*, gint, gpointer) {
	CALCULATOR->abort();
}


void add_to_expression_history(string str) {
	for(size_t i = 0; i < expression_history.size(); i++) {
		if(expression_history[i] == str) {
			expression_history.erase(expression_history.begin() + i);
			break;
		}
	}
	if(expression_history.size() >= 25) {
		expression_history.pop_back();
	}
	expression_history.insert(expression_history.begin(), str);
	expression_history_index = 0;
}

/*
	calculate entered expression and display result
*/
void execute_expression(bool force, bool do_mathoperation, MathOperation op, MathFunction *f, bool do_stack, size_t stack_index) {

	if(block_expression_execution) return;

	string str;

	if(do_stack) {
  		GtkTreeIter iter;
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(stackstore), &iter, NULL, stack_index);
		gchar *gstr;
		gtk_tree_model_get(GTK_TREE_MODEL(stackstore), &iter, 1, &gstr, -1);
		str = gstr;
		g_free(gstr);
		do_timeout = false;
	} else {
		str = gtk_entry_get_text(GTK_ENTRY(expression));
		if(!force && (expression_has_changed || str.find_first_not_of(SPACES) == string::npos)) return;
		do_timeout = false;
		expression_has_changed = false;
		if(!do_mathoperation && !str.empty()) add_to_expression_history(str);
	}

	size_t stack_size = 0;

	b_busy = true;
	gulong handler_id = g_signal_connect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), "event", G_CALLBACK(on_event), NULL);

	/*if(!do_stack || stack_index == 0) {
		parsed_text = "";
	}*/
	if(do_stack) {
		stack_size = CALCULATOR->RPNStackSize();
		CALCULATOR->setRPNRegister(stack_index + 1, CALCULATOR->unlocalizeExpression(str, evalops.parse_options), 0, evalops, parsed_mstruct, parsed_tostruct, !printops.negative_exponents);
	} else if(rpn_mode) {
		stack_size = CALCULATOR->RPNStackSize();
		if(do_mathoperation) {
			if(f) CALCULATOR->calculateRPN(f, 0, evalops, parsed_mstruct);
			else CALCULATOR->calculateRPN(op, 0, evalops, parsed_mstruct);
		} else {
			string str2 = CALCULATOR->unlocalizeExpression(str, evalops.parse_options);
			CALCULATOR->parseSigns(str2);
			remove_blank_ends(str2);
			if(str2.length() == 1) {
				do_mathoperation = true;
				switch(str2[0]) {
					case '^': {CALCULATOR->calculateRPN(OPERATION_RAISE, 0, evalops, parsed_mstruct); break;}
					case '+': {CALCULATOR->calculateRPN(OPERATION_ADD, 0, evalops, parsed_mstruct); break;}
					case '-': {CALCULATOR->calculateRPN(OPERATION_SUBTRACT, 0, evalops, parsed_mstruct); break;}
					case '*': {CALCULATOR->calculateRPN(OPERATION_MULTIPLY, 0, evalops, parsed_mstruct); break;}
					case '/': {CALCULATOR->calculateRPN(OPERATION_DIVIDE, 0, evalops, parsed_mstruct); break;}
					case '&': {CALCULATOR->calculateRPN(OPERATION_BITWISE_AND, 0, evalops, parsed_mstruct); break;}
					case '|': {CALCULATOR->calculateRPN(OPERATION_BITWISE_OR, 0, evalops, parsed_mstruct); break;}
					case '~': {CALCULATOR->calculateRPNBitwiseNot(0, evalops, parsed_mstruct); break;}
					case '!': {CALCULATOR->calculateRPN(CALCULATOR->f_factorial, 0, evalops, parsed_mstruct); break;}
					case '>': {CALCULATOR->calculateRPN(OPERATION_GREATER, 0, evalops, parsed_mstruct); break;}
					case '<': {CALCULATOR->calculateRPN(OPERATION_LESS, 0, evalops, parsed_mstruct); break;}
					case '=': {CALCULATOR->calculateRPN(OPERATION_EQUALS, 0, evalops, parsed_mstruct); break;}
					default: {do_mathoperation = false;}
				}
			} else if(str2.length() == 2) {
				if(str2 == "**") {
					CALCULATOR->calculateRPN(OPERATION_RAISE, 0, evalops, parsed_mstruct);
					do_mathoperation = true;
				} else if(str2 == "!!") {
					CALCULATOR->calculateRPN(CALCULATOR->f_factorial2, 0, evalops, parsed_mstruct);
					do_mathoperation = true;
				} else if(str2 == "!=" || str == "=!" || str == "<>") {
					CALCULATOR->calculateRPN(OPERATION_NOT_EQUALS, 0, evalops, parsed_mstruct);
					do_mathoperation = true;
				} else if(str2 == "<=" || str == "=<") {
					CALCULATOR->calculateRPN(OPERATION_EQUALS_LESS, 0, evalops, parsed_mstruct);
					do_mathoperation = true;
				} else if(str2 == ">=" || str == "=>") {
					CALCULATOR->calculateRPN(OPERATION_EQUALS_GREATER, 0, evalops, parsed_mstruct);
					do_mathoperation = true;
				} else if(str2 == "==") {
					CALCULATOR->calculateRPN(OPERATION_EQUALS, 0, evalops, parsed_mstruct);
					do_mathoperation = true;
				}
			}
			if(!do_mathoperation) {
				bool had_nonnum = false, test_function = true;
				int in_par = 0;
				for(size_t i = 0; i < str2.length(); i++) {
					if(is_in(NUMBERS, str2[i])) {
						if(!had_nonnum || in_par) {
							test_function = false;
							break;
						}
					} else if(str2[i] == '(') {
						if(in_par || !had_nonnum) {
							test_function = false;
							break;
						}
						in_par = i;
					} else if(str2[i] == ')') {
						if(i != str2.length() - 1) {
							test_function = false;
							break;
						}
					} else if(str2[i] == ' ') {
						if(!in_par) {
							test_function = false;
							break;
						}
					} else if(is_in(NOT_IN_NAMES, str2[i])) {
						test_function = false;
						break;
					} else {
						if(in_par) {
							test_function = false;
							break;
						}
						had_nonnum = true;
					}
				}
				f = NULL;
				if(test_function) {
					if(in_par) f = CALCULATOR->getActiveFunction(str2.substr(0, in_par));
					else f = CALCULATOR->getActiveFunction(str2);
				}
				if(f && f->minargs() > 1) {
					show_message("Can only apply functions wich requires one argument on RPN stack.", glade_xml_get_widget (main_glade, "main_window"));
					f = NULL;
					return;
				}
				if(f && f->minargs() > 0) {
					do_mathoperation = true;
					CALCULATOR->calculateRPN(f, 0, evalops, parsed_mstruct);
				} else {
					CALCULATOR->RPNStackEnter(str2, 0, evalops, parsed_mstruct, parsed_tostruct, !printops.negative_exponents);
				}
			}
		}
	} else {
		CALCULATOR->calculate(mstruct, CALCULATOR->unlocalizeExpression(str, evalops.parse_options), 0, evalops, parsed_mstruct, parsed_tostruct, !printops.negative_exponents);
	}

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
		gtk_window_set_title(GTK_WINDOW(dialog), _("Calculating..."));
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (main_glade, "progress_label_message")), _("Calculating..."));
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

	if(rpn_mode && (!do_stack || stack_index == 0)) {
		mstruct->unref();
		mstruct = CALCULATOR->getRPNRegister(1);
		if(!mstruct) mstruct = new MathStructure();
		else mstruct->ref();
	}
	
	//update "ans" variables
	if(!do_stack || stack_index == 0) {
		vans[4]->set(vans[3]->get());
		vans[3]->set(vans[2]->get());
		vans[2]->set(vans[1]->get());
		vans[1]->set(vans[0]->get());
		vans[0]->set(*mstruct);
	}

	if(do_stack && stack_index > 0) {
	} else if(rpn_mode && do_mathoperation) {
		result_text = _("RPN Operation");
	} else {
		result_text = str;
	}
	printops.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
	if(rpn_mode && (!do_stack || stack_index == 0)) {
		gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
		if(CALCULATOR->RPNStackSize() < stack_size) {
			RPNRegisterRemoved(1);
		} else if(CALCULATOR->RPNStackSize() > stack_size) {
			RPNRegisterAdded("");
		}
	}
	setResult(NULL, true, (!do_stack || stack_index == 0), true, "", do_stack ? stack_index : 0);
	g_signal_handler_disconnect(G_OBJECT(glade_xml_get_widget (main_glade, "main_window")), handler_id);
	if(!do_stack || stack_index == 0) {
		gtk_widget_grab_focus(expression);
		gtk_editable_select_region(GTK_EDITABLE(expression), 0, -1);
		//gtk_editable_set_position(GTK_EDITABLE(expression), -1);
	}
	do_timeout = true;

}

void set_rpn_mode(bool b) {
	if(b == rpn_mode) return;
	rpn_mode = b;
	if(rpn_mode) {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget(main_glade, "button_equals")), _("Ent"));
		gtk_tooltips_set_tip(main_tooltips, glade_xml_get_widget(main_glade, "button_equals"), _("Calculate expression and add to stack"), NULL);
		gtk_tooltips_set_tip(main_tooltips, glade_xml_get_widget(main_glade, "button_execute"), _("Calculate expression and add to stack"), NULL);
		gtk_widget_show(expander_stack);
		show_history = gtk_expander_get_expanded(GTK_EXPANDER(expander_history));
		show_keypad = gtk_expander_get_expanded(GTK_EXPANDER(expander_keypad));
		if(show_stack) {
			gtk_expander_set_expanded(GTK_EXPANDER(expander_stack), TRUE);
		}
	} else {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget(main_glade, "button_equals")), _("="));
		gtk_tooltips_set_tip(main_tooltips, glade_xml_get_widget(main_glade, "button_equals"), _("Calculate expression"), NULL);
		gtk_tooltips_set_tip(main_tooltips, glade_xml_get_widget(main_glade, "button_execute"), _("Calculate expression"), NULL);
		gtk_widget_hide(expander_stack);
		show_stack = gtk_expander_get_expanded(GTK_EXPANDER(expander_stack));
		if(show_stack) {
			if(show_history) gtk_expander_set_expanded(GTK_EXPANDER(expander_history), TRUE);
			else if(show_keypad) gtk_expander_set_expanded(GTK_EXPANDER(expander_keypad), TRUE);
			else gtk_expander_set_expanded(GTK_EXPANDER(expander_stack), FALSE);
		}
		CALCULATOR->clearRPNStack();
		gtk_list_store_clear(stackstore);
	}
}

void updateRPNIndexes() {
	GtkTreeIter iter;
	if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(stackstore), &iter)) return;
	for(int i = 1; ; i++) {
		gtk_list_store_set(stackstore, &iter, 0, i2s(i).c_str(), -1);
		if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(stackstore), &iter)) break;
	}
}

void calculateRPN(int op) {
	if(expression_has_changed) {
		string str = gtk_entry_get_text(GTK_ENTRY(expression));
		if(str.find_first_not_of(SPACES) != string::npos) {
			execute_expression(true);
		}
	}
	execute_expression(true, true, (MathOperation) op, NULL);
}
void calculateRPN(MathFunction *f) {
	if(expression_has_changed) {
		string str = gtk_entry_get_text(GTK_ENTRY(expression));
		if(str.find_first_not_of(SPACES) != string::npos) {
			execute_expression(true);
		}
	}
	execute_expression(true, true, OPERATION_ADD, f);
}
void RPNRegisterAdded(string text, gint index) {
	GtkTreeIter iter;
	gtk_list_store_insert(stackstore, &iter, index);
	gtk_list_store_set(stackstore, &iter, 0, i2s(index + 1).c_str(), 1, text.c_str(), -1);
	updateRPNIndexes();
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_clearstack"), TRUE);
	on_stackview_selection_changed(gtk_tree_view_get_selection(GTK_TREE_VIEW(stackview)), NULL);
}
void RPNRegisterRemoved(gint index) {
  	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(stackstore), &iter, NULL, index);
	gtk_list_store_remove(stackstore, &iter);
	updateRPNIndexes();
	if(CALCULATOR->RPNStackSize() == 0) gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_clearstack"), FALSE);
	on_stackview_selection_changed(gtk_tree_view_get_selection(GTK_TREE_VIEW(stackview)), NULL);
}
void RPNRegisterChanged(string text, gint index) {
  	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(stackstore), &iter, NULL, index);
	gtk_list_store_set(stackstore, &iter, 1, text.c_str(), -1);
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
	if(!GTK_WIDGET_HAS_FOCUS(expression)) {
		gtk_widget_grab_focus(expression);
		//unselect
		gtk_editable_select_region(GTK_EDITABLE(expression), position, position);
	}
}

void recreate_recent_functions() {
	GtkWidget *item, *sub;
	sub = f_menu;
	recent_function_items.clear();
	bool b = false;
	for(size_t i = 0; i < recent_functions.size(); i++) {
		if(!CALCULATOR->stillHasFunction(recent_functions[i])) {
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
	for(size_t i = 0; i < recent_variables.size(); i++) {
		if(!CALCULATOR->stillHasVariable(recent_variables[i])) {
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
	for(size_t i = 0; i < recent_units.size(); i++) {
		if(!CALCULATOR->stillHasUnit(recent_units[i])) {
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

void function_inserted(MathFunction *object) {
	if(!object) {
		return;
	}
	GtkWidget *item, *sub;
	sub = f_menu;
	if(recent_function_items.size() <= 0) {
		MENU_SEPARATOR_PREPEND
	}
	for(size_t i = 0; i < recent_functions.size(); i++) {
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
	for(size_t i = 0; i < recent_variables.size(); i++) {
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
	for(size_t i = 0; i < recent_units.size(); i++) {
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

void apply_function(MathFunction *f, GtkWidget* = NULL) {
	if(rpn_mode) {
		calculateRPN(f);
		return;
	}
	string str = f->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression).name;
	if(f->args() == 0) {
		str += "()";
	} else {
		str += "(";
		str += vans[0]->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression).name;
		str += ")";
	}
	gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
	insert_text(str.c_str());
	execute_expression();
	function_inserted(f);
}

/*
	insert function
	pops up an argument entry dialog and inserts function into expression entry
	parent is parent window
*/
void insert_function(MathFunction *f, GtkWidget *parent = NULL) {
	if(!f) {
		return;
	}
	gint start = 0, end = 0;
	gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end);
	GtkWidget *dialog;
	//if function takes no arguments, do not display dialog and insert function directly
	if(f->args() == 0) {
		string str = f->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression).name + "()";
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
	gtk_box_pack_start(GTK_BOX(b_hbox), gtk_label_new_with_mnemonic(_("_Execute")), FALSE, FALSE, 0);
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
	vector<GtkWidget*> label;
	vector<GtkWidget*> entry;
	vector<GtkWidget*> type_label;
	label.resize(args, NULL);
	entry.resize(args, NULL);
	type_label.resize(args, NULL);
	vector<GtkWidget*> boolean_buttons;
	vector<int> boolean_index;
	boolean_index.resize(args, 0);
	int bindex = 0;
	GtkWidget *descr, *descr_box, *descr_frame;
	string argstr, typestr, defstr; 
	string argtype;
	//create argument entries
	Argument *arg;
	GtkListStore *properties_store = NULL;
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
		defstr = f->getDefaultValue(i + 1);
		if(arg && (arg->suggestsQuotes() || arg->type() == ARGUMENT_TYPE_TEXT) && defstr.length() >= 2 && defstr[0] == '\"' && defstr[defstr.length() - 1] == '\"') {
			defstr = defstr.substr(1, defstr.length() - 2);
		}
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
					boolean_index[i] = bindex;
					bindex += 2;
					entry[i] = gtk_hbox_new(TRUE, 6);
					boolean_buttons.push_back(gtk_radio_button_new_with_label(NULL, _("True")));
					gtk_box_pack_start(GTK_BOX(entry[i]), boolean_buttons[boolean_buttons.size() - 1], TRUE, TRUE, 0);
					boolean_buttons.push_back(gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(boolean_buttons[boolean_buttons.size() - 1]), _("False")));
					gtk_box_pack_end(GTK_BOX(entry[i]), boolean_buttons[boolean_buttons.size() - 1], TRUE, TRUE, 0);
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(boolean_buttons[boolean_buttons.size() - 1]), TRUE);
					break;
				}
				case ARGUMENT_TYPE_DATA_PROPERTY: {
					if(f->subtype() == SUBTYPE_DATA_SET) {
						properties_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
						gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(properties_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
						gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(properties_store), 0, GTK_SORT_ASCENDING);
						entry[i] = gtk_combo_box_new_with_model(GTK_TREE_MODEL(properties_store));
						GtkCellRenderer *cell = gtk_cell_renderer_text_new();
						gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(entry[i]), cell, TRUE);
						gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(entry[i]), cell, "text", 0);
						DataPropertyIter it;
						DataSet *ds = (DataSet*) f;
						DataProperty *dp = ds->getFirstProperty(&it);
						GtkTreeIter iter;
						bool active_set = false;
						while(dp) {
							if(!dp->isHidden()) {
								gtk_list_store_append(properties_store, &iter);
								if(!active_set && defstr == dp->getName()) {
									gtk_combo_box_set_active_iter(GTK_COMBO_BOX(entry[i]), &iter);
									active_set = true;
								}
								gtk_list_store_set(properties_store, &iter, 0, dp->title().c_str(), 1, (gpointer) dp, -1);
							}
							dp = ds->getNextProperty(&it);
						}
						gtk_list_store_append(properties_store, &iter);
						if(!active_set) {
							gtk_combo_box_set_active_iter(GTK_COMBO_BOX(entry[i]), &iter);
						}
						gtk_list_store_set(properties_store, &iter, 0, _("Info"), 1, (gpointer) NULL, -1);
						break;
					}
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
		if(arg && arg->type() == ARGUMENT_TYPE_BOOLEAN) {
			if(defstr == "1") {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(boolean_buttons[boolean_buttons.size() - 2]), TRUE);
			}
		} else if(properties_store && arg && arg->type() == ARGUMENT_TYPE_DATA_PROPERTY) {
		} else {
			gtk_entry_set_text(GTK_ENTRY(entry[i]), defstr.c_str());
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
		
		string str = f->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression).name + "(", str2;
		for(int i = 0; i < args; i++) {
			if(f->getArgumentDefinition(i + 1) && f->getArgumentDefinition(i + 1)->type() == ARGUMENT_TYPE_BOOLEAN) {
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(boolean_buttons[boolean_index[i]]))) {
					str2 = "1";
				} else {
					str2 = "0";
				}
			} else if(properties_store && f->getArgumentDefinition(i + 1) && f->getArgumentDefinition(i + 1)->type() == ARGUMENT_TYPE_DATA_PROPERTY) {
				GtkTreeIter iter;
				DataProperty *dp = NULL;
				if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(entry[i]), &iter)) {
					gtk_tree_model_get(GTK_TREE_MODEL(properties_store), &iter, 1, &dp, -1);
				}	
				if(dp) {
					str2 = dp->getName();
				} else {
					str2 = "info";
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
void insert_function(GtkMenuItem*, gpointer user_data) {
	insert_function((MathFunction*) user_data, glade_xml_get_widget (main_glade, "main_window"));
}

/*
	called from variable menu
	just insert text data stored in menu item
*/
void insert_variable(GtkMenuItem*, gpointer user_data) {
	Variable *v = (Variable*) user_data;
	if(!CALCULATOR->stillHasVariable(v)) {
		show_message(_("Variable does not exist anymore."), glade_xml_get_widget (main_glade, "main_window"));
		update_vmenu();
		return;
	}
	insert_text(v->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression).name.c_str());
	variable_inserted((Variable*) user_data);
}
//from prefix menu
void insert_prefix(GtkMenuItem*, gpointer user_data) {
	insert_text(((Prefix*) user_data)->name(printops.abbreviate_names, printops.use_unicode_signs, &can_display_unicode_string_function, (void*) expression).c_str());
}
//from unit menu
void insert_unit(GtkMenuItem*, gpointer user_data) {
	if(((Unit*) user_data)->subtype() == SUBTYPE_COMPOSITE_UNIT) {
		insert_text(((CompositeUnit*) user_data)->print(true, printops.abbreviate_names, printops.use_unicode_signs, &can_display_unicode_string_function, (void*) expression).c_str());
	} else {
		insert_text(((Unit*) user_data)->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, true, false, &can_display_unicode_string_function, (void*) expression).name.c_str());
	}
	unit_inserted((Unit*) user_data);
}

void set_name_label_and_entry(ExpressionItem *item, GtkWidget *entry, GtkWidget *label) {
	const ExpressionName *ename = &item->getName(1);
	gtk_entry_set_text(GTK_ENTRY(entry), ename->name.c_str());
	if(item->countNames() > 1) {
		string str = "+ ";
		for(size_t i = 2; i <= item->countNames(); i++) {
			if(i > 2) str += ", ";
			str += item->getName(i).name;
		}
		gtk_label_set_text(GTK_LABEL(label), str.c_str());
	}
}
void set_edited_names(ExpressionItem *item, string str) {
	if(item->isBuiltin() && !(item->type() == TYPE_FUNCTION && item->subtype() == SUBTYPE_DATA_SET)) return;
	if(names_edited) {
		item->clearNames();
		GtkTreeIter iter;
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) {
			ExpressionName ename;
			gchar *gstr;
			while(true) {	
				gboolean abbreviation = FALSE, suffix = FALSE, unicode = FALSE, plural = FALSE;
				gboolean reference = FALSE, avoid_input = FALSE, case_sensitive = FALSE;
				gtk_tree_model_get(GTK_TREE_MODEL(tNames_store), &iter, NAMES_NAME_COLUMN, &gstr, NAMES_ABBREVIATION_COLUMN, &abbreviation, NAMES_SUFFIX_COLUMN, &suffix, NAMES_UNICODE_COLUMN, &unicode, NAMES_PLURAL_COLUMN, &plural, NAMES_REFERENCE_COLUMN, &reference, NAMES_AVOID_INPUT_COLUMN, &avoid_input, NAMES_CASE_SENSITIVE_COLUMN, &case_sensitive, -1);
				ename.name = gstr; ename.abbreviation = abbreviation; ename.suffix = suffix;
				ename.unicode = unicode; ename.plural = plural; ename.reference = reference; 
				ename.avoid_input = avoid_input; ename.case_sensitive = case_sensitive;
				item->addName(ename);
				g_free(gstr);
				if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(tNames_store), &iter)) break;
			}
		} else {
			item->addName(str);
		}
	} else {
		if(item->countNames() == 0) {
			ExpressionName ename(str);
			ename.reference = true;
			item->setName(ename, 1);
		} else {
			item->setName(str, 1);
		}
		
	}
}

/*
	display edit/new unit dialog
	creates new unit if u == NULL, win is parent window
*/
void
edit_unit(const char *category = "", Unit *u = NULL, GtkWidget *win = NULL)
{

	edited_unit = u;
	names_edited = false;
	editing_unit = true;
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
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), "");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp")), 1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_system")), "");
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (unitedit_glade, "unit_edit_label_names")), "");

	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_button_ok"), TRUE);

	if(u) {
		//fill in original parameters
		if(u->subtype() == SUBTYPE_BASE_UNIT) {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), UNIT_CLASS_BASE_UNIT);
		} else if(u->subtype() == SUBTYPE_ALIAS_UNIT) {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), UNIT_CLASS_ALIAS_UNIT);
		} else if(u->subtype() == SUBTYPE_COMPOSITE_UNIT) {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), UNIT_CLASS_COMPOSITE_UNIT);
		}
		on_unit_edit_optionmenu_class_changed(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")), NULL);	
		gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class"), !u->isBuiltin());

		//gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class"), u->isLocal() && !u->isBuiltin());

		set_name_label_and_entry(u, glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"), glade_xml_get_widget (unitedit_glade, "unit_edit_label_names"));
		
		gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"), !u->isBuiltin());
		
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_system")), u->system().c_str());
		gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_combo_system"), !u->isBuiltin());

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_hidden")), u->isHidden());
		
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category")), u->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc")), u->title(false).c_str());

		switch(u->subtype()) {
			case SUBTYPE_ALIAS_UNIT: {
				AliasUnit *au = (AliasUnit*) u;
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), ((CompositeUnit*) (au->firstBaseUnit()))->preferredDisplayName(printops.abbreviate_names, true, false, false, &can_display_unicode_string_function, (void*) glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")).name.c_str());
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp")), au->firstBaseExponent());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation")), CALCULATOR->localizeExpression(au->expression()).c_str());
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed")), CALCULATOR->localizeExpression(au->inverseExpression()).c_str());
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact")), !au->isApproximate());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp"), !u->isBuiltin());
				gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"), !u->isBuiltin());
				break;
			}
			case SUBTYPE_COMPOSITE_UNIT: {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")), ((CompositeUnit*) u)->print(false, printops.abbreviate_names, true, &can_display_unicode_string_function, (void*) glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base")).c_str());
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
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str;
		str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name")));
		remove_blank_ends(str);
		GtkTreeIter iter;
		if(str.empty() && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter))) {
			//no name given
			gtk_widget_grab_focus(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"));
			show_message(_("Empty name field."), dialog);
			goto run_unit_edit_dialog;
		}

		//unit with the same name exists -- overwrite or open the dialog again
		if((!u || !u->hasName(str)) && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) && CALCULATOR->unitNameTaken(str, u) && !ask_question(_("A variable or unit with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
			gtk_widget_grab_focus(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"));
			goto run_unit_edit_dialog;
		}
		bool add_unit = false;
		if(u) {
			//edited an existing unit -- update unit
			u->setLocal(true);
			gint i1 = gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")));
			switch(u->subtype()) {
				case SUBTYPE_ALIAS_UNIT: {
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
							gtk_widget_grab_focus(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"));
							show_message(_("Base unit does not exist."), dialog);
							goto run_unit_edit_dialog;
						}
						au->setBaseUnit(bu);
						au->setExpression(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation"))), evalops.parse_options));
						au->setInverseExpression(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed"))), evalops.parse_options));
						au->setExponent(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp"))));
						au->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact"))));
					}
					break;
				}
				case SUBTYPE_COMPOSITE_UNIT: {
					if(i1 != UNIT_CLASS_COMPOSITE_UNIT) {
						u->destroy();
						u = NULL;
						break;
					}
					if(!u->isBuiltin()) {
						((CompositeUnit*) u)->setBaseExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
					}
					break;
				}
				case SUBTYPE_BASE_UNIT: {
					if(i1 != UNIT_CLASS_BASE_UNIT) {
						u->destroy();
						u = NULL;
						break;
					}
					break;
				}
			}
			if(u) {
				u->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))));
				u->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))));
			}
		}
		if(!u) {
			//new unit
			switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unitedit_glade, "unit_edit_optionmenu_class")))) {
				case UNIT_CLASS_ALIAS_UNIT: {
					Unit *bu = CALCULATOR->getUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
					if(!bu) bu = CALCULATOR->getCompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))));
					if(!bu) {
						gtk_widget_grab_focus(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"));
						show_message(_("Base unit does not exist."), dialog);
						goto run_unit_edit_dialog;
					}
					u = new AliasUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))), "", "", "", gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))), bu, CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation"))), evalops.parse_options), gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp"))), CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed"))), evalops.parse_options), true);
					((AliasUnit*) u)->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact"))));
					break;
				}
				case UNIT_CLASS_COMPOSITE_UNIT: {
					CompositeUnit *cu = new CompositeUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))), "", gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"))), true);
					u = cu;
					break;
				}
				default: {
					u = new Unit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_category"))), "", "", "", gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_desc"))), true);
					break;
				}
			}
			add_unit = true;
		}
		if(u) {
			u->setHidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_hidden"))));
			if(!u->isBuiltin()) {
				u->setSystem(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_system"))));
			}
			set_edited_names(u, str);
			if(add_unit) {
				CALCULATOR->addUnit(u);
			}
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
	} else if(response == GTK_RESPONSE_HELP) {
#ifdef HAVE_LIBGNOME
		GError *error = NULL;
		gnome_help_display("qalculate-gtk", "qalculate-unit-creation", &error);
		if(error) {
			gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
			GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (unitedit_glade, "unit_edit_dialog")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help.\n%s"), error_str);			
			gtk_dialog_run(GTK_DIALOG(d));
			gtk_widget_destroy(d);
			g_free(error_str);
			g_error_free(error);
		}
#endif	
		goto run_unit_edit_dialog;
	}
	edited_unit = NULL;
	names_edited = false;
	editing_unit = false;
	gtk_widget_hide(dialog);
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
				MathStructure mstruct2 = CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min"))), evalops.parse_options));
				farg->setMin(&mstruct2.number());
			} else {
				farg->setMin(NULL);
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (argumentrules_glade, "argument_rules_checkbutton_enable_max")))) {
				MathStructure mstruct2 = CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max"))), evalops.parse_options));
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
}

/*
	display edit/new function dialog
	creates new function if f == NULL, win is parent window
*/
void edit_function(const char *category = "", MathFunction *f = NULL, GtkWidget *win = NULL) {

	if(f && f->subtype() == SUBTYPE_DATA_SET) {
		edit_dataset((DataSet*) f, win);
		return;
	}

	GtkWidget *dialog = get_function_edit_dialog();	
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	edited_function = f;
	names_edited = false;
	editing_function = true;
	
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
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (functionedit_glade, "function_edit_label_names")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_condition")), "");
	gtk_widget_set_sensitive(glade_xml_get_widget (functionedit_glade, "function_edit_entry_condition"), !f || !f->isBuiltin());
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
		set_name_label_and_entry(f, glade_xml_get_widget (functionedit_glade, "function_edit_entry_name"), glade_xml_get_widget (functionedit_glade, "function_edit_label_names"));
		if(!f->isBuiltin()) {
			gtk_text_buffer_set_text(expression_buffer, CALCULATOR->localizeExpression(((UserFunction*) f)->formula()).c_str(), -1);
		}
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_category")), f->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_desc")), f->title(false).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_condition")), CALCULATOR->localizeExpression(f->condition()).c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_hidden")), f->isHidden());
		gtk_text_buffer_set_text(description_buffer, f->description().c_str(), -1);
		
		if(!f->isBuiltin()) {
			GtkTreeIter iter;
			string str, str2;
			for(size_t i = 1; i <= ((UserFunction*) f)->countSubfunctions(); i++) {
				gtk_list_store_append(tSubfunctions_store, &iter);
				if(((UserFunction*) f)->subfunctionPrecalculated(i)) {
					str = _("Yes");
				} else {
					str = _("No");
				}
				str2 = "\\";
				str2 += i2s(i);
				gtk_list_store_set(tSubfunctions_store, &iter, 0, str2.c_str(), 1, ((UserFunction*) f)->getSubfunction(i).c_str(), 2, str.c_str(), 3, i, 4, ((UserFunction*) f)->subfunctionPrecalculated(i), -1);
				last_subfunction_index = i;
			}
		}
	}
	update_function_arguments_list(f);
	gtk_widget_grab_focus(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (functionedit_glade, "function_edit_tabs")), 0);
	
run_function_edit_dialog:
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name")));
		remove_blank_ends(str);
		GtkTreeIter iter;
		if(str.empty() && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter))) {
			//no name -- open dialog again
			gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (functionedit_glade, "function_edit_tabs")), 0);
			gtk_widget_grab_focus(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name"));
			show_message(_("Empty name field."), dialog);
			goto run_function_edit_dialog;
		}
		GtkTextIter e_iter_s, e_iter_e;
		gtk_text_buffer_get_start_iter(expression_buffer, &e_iter_s);
		gtk_text_buffer_get_end_iter(expression_buffer, &e_iter_e);		
		string str2 = CALCULATOR->unlocalizeExpression(gtk_text_buffer_get_text(expression_buffer, &e_iter_s, &e_iter_e, FALSE), evalops.parse_options);
		remove_blank_ends(str2);
		gsub("\n", " ", str2);
		if(!(f && f->isBuiltin()) && str2.empty()) {
			//no expression/relation -- open dialog again
			gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (functionedit_glade, "function_edit_tabs")), 1);
			gtk_widget_grab_focus(glade_xml_get_widget (functionedit_glade, "function_edit_textview_expression"));
			show_message(_("Empty expression field."), dialog);
			goto run_function_edit_dialog;
		}
		GtkTextIter d_iter_s, d_iter_e;
		gtk_text_buffer_get_start_iter(description_buffer, &d_iter_s);
		gtk_text_buffer_get_end_iter(description_buffer, &d_iter_e);
		//function with the same name exists -- overwrite or open the dialog again
		if((!f || !f->hasName(str)) && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) && CALCULATOR->functionNameTaken(str, f) && !ask_question(_("A function with the same name already exists.\nDo you want to overwrite the function?"), dialog)) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (functionedit_glade, "function_edit_tabs")), 0);
			gtk_widget_grab_focus(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name"));
			goto run_function_edit_dialog;
		}
		bool add_func = false;
		if(f) {
			f->setLocal(true);
			//edited an existing function
			f->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_category"))));
			f->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_desc"))));
			f->setDescription(gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE));
			if(!f->isBuiltin()) {				
				f->clearArgumentDefinitions();
			}	
		} else {
			//new function
			f = new UserFunction(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_category"))), "", "", true, -1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_desc"))), gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE));
			add_func = true;
		}
		if(f) {
			f->setCondition(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_condition"))), evalops.parse_options));
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
			if(!f->isBuiltin()) {
				((UserFunction*) f)->clearSubfunctions();
				b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tSubfunctions_store), &iter);
				while(b) {
					gchar *gstr;
					gboolean g_b = FALSE;
					gtk_tree_model_get(GTK_TREE_MODEL(tSubfunctions_store), &iter, 1, &gstr, 4, &g_b, -1);
					((UserFunction*) f)->addSubfunction(gstr, g_b);
					b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tSubfunctions_store), &iter);
					g_free(gstr);
				}
				((UserFunction*) f)->setFormula(str2);
			}
			f->setHidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_hidden"))));
			set_edited_names(f, str);
			if(add_func) {
				CALCULATOR->addFunction(f);
			}
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
	} else if(response == GTK_RESPONSE_HELP) {
#ifdef HAVE_LIBGNOME
		GError *error = NULL;
		gnome_help_display("qalculate-gtk", "qalculate-function-creation", &error);
		if(error) {
			gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
			GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (functionedit_glade, "function_edit_dialog")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help.\n%s"), error_str);
			gtk_dialog_run(GTK_DIALOG(d));
			gtk_widget_destroy(d);
			g_free(error_str);
			g_error_free(error);
		}
#endif	
		goto run_function_edit_dialog;
	}
	edited_function = NULL;
	names_edited = false;
	editing_function = false;
	gtk_widget_hide(dialog);
}

/*
	"New function" menu item selected
*/
void new_function(GtkMenuItem*, gpointer)
{
	edit_function(
			"",
			NULL,
			glade_xml_get_widget (main_glade, "main_window"));
}
/*
	"New unit" menu item selected
*/
void new_unit(GtkMenuItem*, gpointer)
{
	edit_unit(
			"",
			NULL,
			glade_xml_get_widget (main_glade, "main_window"));
}

/*
	a unit selected in result menu, convert result
*/
void convert_to_unit(GtkMenuItem*, gpointer user_data)
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
	do_timeout = false;
	mstruct->set(CALCULATOR->convert(*mstruct, u, evalops));
	result_action_executed();
	do_timeout = true;
	focus_keeping_selection();
}

void edit_unknown(const char *category, Variable *var, GtkWidget *win) {

	if(var != NULL && var->isKnown()) {
		edit_variable(category, var, NULL, win);
		return;
	}
	
	UnknownVariable *v = (UnknownVariable*) var;
	edited_unknown = v;
	names_edited = false;
	editing_unknown = true;
	
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
	
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_type_changed, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_sign_changed, NULL);
	if(v) {
		//fill in original parameters
		set_name_label_and_entry(v, glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"), glade_xml_get_widget (unknownedit_glade, "unknown_edit_label_names"));
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"), !v->isBuiltin());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_desc")), v->title(false).c_str());
		if(v->assumptions()) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_type"), TRUE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_sign"), TRUE);
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type")), v->assumptions()->type());
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign")), v->assumptions()->sign());
		} else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_type"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_sign"), FALSE);
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type")), CALCULATOR->defaultAssumptions()->type());
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
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (unknownedit_glade, "unknown_edit_label_names")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_desc")), "");
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type")), CALCULATOR->defaultAssumptions()->type());
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign")), CALCULATOR->defaultAssumptions()->sign());

	}
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_type_changed, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_sign_changed, NULL);
	
	gtk_widget_grab_focus(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"));

run_unknown_edit_dialog:
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name")));
		remove_blank_ends(str);
		GtkTreeIter iter;
		if(str.empty() && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter))) {
			//no name -- open dialog again
			gtk_widget_grab_focus(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"));
			show_message(_("Empty name field."), dialog);
			goto run_unknown_edit_dialog;
		}

		//unknown with the same name exists -- overwrite or open dialog again
		if((!v || !v->hasName(str)) && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) && CALCULATOR->variableNameTaken(str, v) && !ask_question(_("An unit or variable with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
			gtk_widget_grab_focus(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"));
			goto run_unknown_edit_dialog;
		}
		if(!v) {
			//no need to create a new unknown when a unknown with the same name exists
			var = CALCULATOR->getActiveVariable(str);
			if(var && var->isLocal() && !var->isKnown()) v = (UnknownVariable*) var;
		}
		bool add_var = false;
		if(v) {
			//update existing unknown
			v->setLocal(true);
		} else {
			//new unknown
			v = new UnknownVariable("", "", "", true);
			add_var = true;
		}
		if(v) {
			if(!v->isBuiltin()) {
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (unknownedit_glade, "unknown_edit_checkbutton_custom_assumptions")))) {
					if(!v->assumptions()) v->setAssumptions(new Assumptions());
					v->assumptions()->setType((AssumptionType) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"))));
					v->assumptions()->setSign((AssumptionSign) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"))));
				} else {
					v->setAssumptions(NULL);
				}
			}
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_desc"))));
			set_edited_names(v, str);
			if(add_var) {
				CALCULATOR->addVariable(v);
			}
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
	} else if(response == GTK_RESPONSE_HELP) {
#ifdef HAVE_LIBGNOME
		GError *error = NULL;
		gnome_help_display("qalculate-gtk", "qalculate-variable-creation", &error);
		if(error) {
			gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
			GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (unknownedit_glade, "unknown_edit_dialog")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help.\n%s"), error_str);
			gtk_dialog_run(GTK_DIALOG(d));
			gtk_widget_destroy(d);
			g_free(error_str);
			g_error_free(error);
		}
#endif	
		goto run_unknown_edit_dialog;
	}
	edited_unknown = NULL;
	names_edited = false;
	editing_unknown = false;
	gtk_widget_hide(dialog);
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
	
	if(v != NULL && v->get().isVector() && (!mstruct_ || mstruct_->isVector()) && (v->get().size() != 1 || !v->get()[0].isVector() || v->get()[0].size() > 0)) {
		edit_matrix(category, v, mstruct_, win);
		return;
	}

	edited_variable = v;
	names_edited = false;
	editing_variable = true;
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

	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (variableedit_glade, "variable_edit_label_names")), "");

	if(mstruct_) {
		gtk_widget_hide(glade_xml_get_widget (variableedit_glade, "variable_edit_box_names"));
		gtk_widget_hide(glade_xml_get_widget (variableedit_glade, "variable_edit_box_value"));
		gtk_widget_hide(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"));
	} else {
		gtk_widget_show(glade_xml_get_widget (variableedit_glade, "variable_edit_box_names"));
		gtk_widget_show(glade_xml_get_widget (variableedit_glade, "variable_edit_box_value"));
		gtk_widget_show(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"));
	}

	if(v) {
		//fill in original parameters
		set_name_label_and_entry(v, glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name"), glade_xml_get_widget (variableedit_glade, "variable_edit_label_names"));
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
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (variableedit_glade, "variable_edit_label_names")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value")), get_value_string(*mstruct).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_desc")), "");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact")), !mstruct_ || !mstruct_->isApproximate());
	}
	
	gtk_widget_grab_focus(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name"));

run_variable_edit_dialog:
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name")));
		string str2 = CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value"))), evalops.parse_options);
		remove_blank_ends(str);
		remove_blank_ends(str2);
		GtkTreeIter iter;
		if(str.empty() && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter))) {
			//no name -- open dialog again
			gtk_widget_grab_focus(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name"));
			show_message(_("Empty name field."), dialog);
			goto run_variable_edit_dialog;
		}
		if(str2.empty() && !mstruct_) {
			//no value -- open dialog again
			gtk_widget_grab_focus(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_value"));
			show_message(_("Empty value field."), dialog);
			goto run_variable_edit_dialog;
		}
		//variable with the same name exists -- overwrite or open dialog again
		if((!v || !v->hasName(str)) && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) && CALCULATOR->variableNameTaken(str, v) && !ask_question(_("An unit or variable with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
			gtk_widget_grab_focus(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name"));
			goto run_variable_edit_dialog;
		}
		if(!v) {
			//no need to create a new variable when a variable with the same name exists
			var = CALCULATOR->getActiveVariable(str);
			if(var && var->isLocal() && var->isKnown()) v = (KnownVariable*) var;
		}
		bool add_var = false;
		if(v) {
			//update existing variable
			v->setLocal(true);
			if(!v->isBuiltin()) {
				if(mstruct_) {
					v->set(*mstruct_);
				} else {
					v->set(str2);
				}
				v->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"))));
			}
		} else {
			//new variable
			if(mstruct_) {
				//forced value
				v = new KnownVariable("", "", *mstruct_, "", true);
			} else {
				v = new KnownVariable("", "", str2, "", true);
				v->setApproximate(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (variableedit_glade, "variable_edit_checkbutton_exact"))));
			}
			add_var = true;
		}
		if(v) {
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_desc"))));
			set_edited_names(v, str);
			if(add_var) {
				CALCULATOR->addVariable(v);
			}
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
	} else if(response == GTK_RESPONSE_HELP) {
#ifdef HAVE_LIBGNOME
		GError *error = NULL;
		gnome_help_display("qalculate-gtk", "qalculate-variable-creation", &error);
		if(error) {
			gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
			GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (variableedit_glade, "variable_edit_dialog")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help.\n%s"), error_str);
			gtk_dialog_run(GTK_DIALOG(d));
			gtk_widget_destroy(d);
			g_free(error_str);
			g_error_free(error);
		}
#endif	
		goto run_variable_edit_dialog;
	}
	edited_variable = NULL;
	names_edited = false;
	editing_variable = false;
	gtk_widget_hide(dialog);
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
	
	edited_matrix = v;
	names_edited = false;
	editing_matrix = true;

	GtkWidget *dialog = get_matrix_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	if(mstruct_) {
		create_vector = !mstruct_->isMatrix();
	} else if(v) {
		create_vector = !v->get().isMatrix();	
	}
	if(create_vector) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector")), TRUE);
	else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix")), TRUE);

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
		set_name_label_and_entry(v, glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"), glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_names"));
		//can only change name and value of user variable
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows"), !v->isBuiltin());		
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns"), !v->isBuiltin());				
		//gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_table_elements"), !v->isBuiltin());
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix"), !v->isBuiltin());						
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector"), !v->isBuiltin());								
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category")), v->category().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc")), v->title(false).c_str());	
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows"), TRUE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns"), TRUE);				
		//gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_table_elements"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix"), TRUE);						
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector"), TRUE);								
	
		//fill in default values
		string v_name = CALCULATOR->getName();
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name")), v_name.c_str());
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_names")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category")), category);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc")), "");		
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
		//gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_table_elements"), FALSE);						
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix"), FALSE);		
		gtk_widget_set_sensitive(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector"), FALSE);		
	}

	if(create_vector) {
		if(old_vctr) {
			r = old_vctr->countChildren();
			c = (int) ::sqrt((double) r) + 4;
			if(r % c > 0) {
				r = r / c + 1;
			} else {
				r = r / c;
			}
		} else {
			c = 6;
			r = 4;
		}
	}

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows")), r);	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns")), c);
	on_matrix_edit_spinbutton_columns_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns")), NULL);
	on_matrix_edit_spinbutton_rows_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows")), NULL);		

	int timeout;
	if(mstruct_) timeout = 3000 / (r * c);
	else timeout = 3000;
	PrintOptions po;
	po.number_fraction_format = FRACTION_DECIMAL_EXACT;
	while(gtk_events_pending()) gtk_main_iteration();
	GtkTreeIter iter;
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tMatrixEdit_store), &iter);
	for(size_t index_r = 0; b && index_r < (size_t) r; index_r++) {
		for(size_t index_c = 0; index_c < (size_t) c; index_c++) {
			if(create_vector) {
				if(old_vctr && index_r * c + index_c < old_vctr->countChildren()) {
					gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, index_c * 2, old_vctr->getChild(index_r * c + index_c + 1)->print(po).c_str(), -1);
				} else {
					gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, index_c * 2, "", -1);
				}
			} else {
				if(v) {
					gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, index_c * 2, v->get().getElement(index_r + 1, index_c + 1)->print(po).c_str(), -1);
				} else if(mstruct_) {
					gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, index_c * 2, mstruct_->getElement(index_r + 1, index_c + 1)->print(po).c_str(), -1);
				} else {
					gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, index_c * 2, "0", -1);
				}
			}
		}
		b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrixEdit_store), &iter);
	}
	if(r > 0 && c > 0) {
		GtkTreePath *path = gtk_tree_path_new_from_indices(0, -1);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrixEdit), path, matrix_edit_columns[0], FALSE);
		while(gtk_events_pending()) gtk_main_iteration();
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tMatrixEdit), path, matrix_edit_columns[0], FALSE, 0.0, 0.0);
		on_tMatrixEdit_cursor_changed(GTK_TREE_VIEW(tMatrixEdit), NULL);
		gtk_tree_path_free(path);
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_position")), "");
	}
	gtk_widget_grab_focus(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"));
	
run_matrix_edit_dialog:
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name")));
		remove_blank_ends(str);
		GtkTreeIter iter;
		if(str.empty() && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter))) {
			//no name -- open dialog again
			gtk_widget_grab_focus(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"));
			show_message(_("Empty name field."), dialog);
			goto run_matrix_edit_dialog;
		}

		//variable with the same name exists -- overwrite or open dialog again
		if((!v || !v->hasName(str)) && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) && CALCULATOR->variableNameTaken(str) && !ask_question(_("An unit or variable with the same name already exists.\nDo you want to overwrite it?"), dialog)) {
			gtk_widget_grab_focus(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"));
			goto run_matrix_edit_dialog;
		}
		if(!v) {
			//no need to create a new variable when a variable with the same name exists
			var = CALCULATOR->getActiveVariable(str);
			if(var && var->isLocal() && var->isKnown()) v = (KnownVariable*) var;
		}
		MathStructure mstruct_new;
		if(!mstruct_) {
			b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tMatrixEdit_store), &iter);
			c = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_columns")));
			r = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_spinbutton_rows")));
			gchar *gstr = NULL;
			string mstr;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_vector")))) {
				mstruct_new.clearVector();
				for(size_t index_r = 0; index_r < (size_t) r && b; index_r++) {
					for(size_t index_c = 0; index_c < (size_t) c; index_c++) {
						gtk_tree_model_get(GTK_TREE_MODEL(tMatrixEdit_store), &iter, index_c * 2, &gstr, -1);
						mstr = gstr;
						g_free(gstr);
						remove_blank_ends(mstr);
						if(!mstr.empty()) {
							mstruct_new.addChild(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(mstr, evalops.parse_options)));
						}
					}
					b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrixEdit_store), &iter);
				}
			} else {
				mstruct_new.clearMatrix();
				mstruct_new.resizeMatrix((size_t) r, (size_t) c, m_undefined);
				for(size_t index_r = 0; index_r < (size_t) r && b; index_r++) {
					for(size_t index_c = 0; index_c < (size_t) c; index_c++) {
						gtk_tree_model_get(GTK_TREE_MODEL(tMatrixEdit_store), &iter, index_c * 2, &gstr, -1);
						mstr = gstr;
						g_free(gstr);
						remove_blank_ends(mstr);
						mstruct_new.setElement(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(mstr, evalops.parse_options)), index_r + 1, index_c + 1);
					}
					b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrixEdit_store), &iter);
				}
			}					
		}
		bool add_var = false;
		if(v) {
			v->setLocal(true);
			//update existing variable
			if(!v->isBuiltin()) {
				if(mstruct_) {
					v->set(*mstruct_);
				} else {
					v->set(mstruct_new);
				}
			}
		} else {
			//new variable
			if(mstruct_) {
				v = new KnownVariable("", "", *mstruct_, "", true);
			} else {
				v = new KnownVariable("", "", mstruct_new, "", true);
			}
			add_var = true;
		}
		if(v) {
			v->setCategory(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_category"))));
			v->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_desc"))));
			set_edited_names(v, str);
			if(add_var) {
				CALCULATOR->addVariable(v);
			}
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
	} else if(response == GTK_RESPONSE_HELP) {
#ifdef HAVE_LIBGNOME
		GError *error = NULL;
		gnome_help_display("qalculate-gtk", "qalculate-vectors-matrices", &error);
		if(error) {
			gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
			GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (matrixedit_glade, "matrix_edit_dialog")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help.\n%s"), error_str);
			gtk_dialog_run(GTK_DIALOG(d));
			gtk_widget_destroy(d);
			g_free(error_str);
			g_error_free(error);
		}
#endif	
		goto run_matrix_edit_dialog;
	}
	edited_matrix = NULL;
	names_edited = false;
	editing_matrix = false;
	gtk_widget_hide(dialog);
}

void insert_matrix(const MathStructure *initial_value, GtkWidget *win, gboolean create_vector, bool is_text_struct, bool is_result) {

	GtkWidget *dialog = get_matrix_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	if(initial_value && !initial_value->isVector()) {
		return;
	}
	if(initial_value) {
		create_vector = !initial_value->isMatrix();
	}
	if(create_vector) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_radiobutton_vector")), TRUE);
	else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_radiobutton_matrix")), TRUE);

	if(is_result) {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_button_cancel")), "gtk-close");
		gtk_widget_grab_focus(glade_xml_get_widget (matrix_glade, "matrix_button_cancel"));
		if(create_vector) {
			gtk_window_set_title(GTK_WINDOW(dialog), _("Vector Result"));
		} else {
			gtk_window_set_title(GTK_WINDOW(dialog), _("Matrix Result"));
		}
	} else {
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_button_cancel")), "gtk-cancel");
		gtk_widget_grab_focus(tMatrix);
		if(create_vector) {
			gtk_window_set_title(GTK_WINDOW(dialog), _("Vector"));
		} else {
			gtk_window_set_title(GTK_WINDOW(dialog), _("Matrix"));
		}
	}

	 int r = 4, c = 4;
	if(create_vector) {
		if(initial_value) {
			r = initial_value->countChildren();
			c = (int) ::sqrt(r) + 4;
			if(r % c > 0) {
				r = r / c + 1;
			} else {
				r = r / c;
			}
		} else {
			c = 6;
			r = 4;
		}
	} else if(initial_value) {
		c = initial_value->columns();
		r = initial_value->rows();
	}

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_spinbutton_rows")), r);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_spinbutton_columns")), c);
	on_matrix_spinbutton_columns_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_spinbutton_columns")), NULL);
	on_matrix_spinbutton_rows_value_changed(GTK_SPIN_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_spinbutton_rows")), NULL);


	printops.can_display_unicode_string_arg = (void*) tMatrix;
	while(gtk_events_pending()) gtk_main_iteration();
	GtkTreeIter iter;
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tMatrix_store), &iter);
	for(size_t index_r = 0; b && index_r < (size_t) r; index_r++) {
		for(size_t index_c = 0; index_c < (size_t) c; index_c++) {
			if(create_vector) {
				if(initial_value && index_r * c + index_c < initial_value->countChildren()) {
					if(is_text_struct) gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, initial_value->getChild(index_r * c + index_c + 1)->symbol().c_str(), -1);
					else gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, initial_value->getChild(index_r * c + index_c + 1)->print(printops).c_str(), -1);
				} else {
					gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, "", -1);
				}
			} else {
				if(initial_value) {
					if(is_text_struct) gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, initial_value->getElement(index_r + 1, index_c + 1)->symbol().c_str(), -1);
					else gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, initial_value->getElement(index_r + 1, index_c + 1)->print(printops).c_str(), -1);
				} else {
					gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, "0", -1);
				}
			}
		}
		b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrix_store), &iter);
	}

	if(r > 0 && c > 0) {
		GtkTreePath *path = gtk_tree_path_new_from_indices(0, -1);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrix), path, matrix_columns[0], FALSE);
		while(gtk_events_pending()) gtk_main_iteration();
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tMatrix), path, matrix_columns[0], FALSE, 0.0, 0.0);
		on_tMatrix_cursor_changed(GTK_TREE_VIEW(tMatrix), NULL);
		gtk_tree_path_free(path);
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrix_glade, "matrix_label_position")), "");
	}
	printops.can_display_unicode_string_arg = NULL;

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_OK) {
		//clicked "OK"
		string matrixstr, str;
		bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tMatrix_store), &iter);
		c = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_spinbutton_columns")));
		r = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_spinbutton_rows")));
		gchar *gstr = NULL;
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_radiobutton_vector")))) {
			bool b1 = false;
			matrixstr = "[";
			for(size_t index_r = 0; index_r < (size_t) r && b; index_r++) {
				for(size_t index_c = 0; index_c < (size_t) c; index_c++) {
					gtk_tree_model_get(GTK_TREE_MODEL(tMatrix_store), &iter, index_c * 2, &gstr, -1);
					str = gstr;
					g_free(gstr);
					remove_blank_ends(str);
					if(!str.empty()) {
						if(b1) {
							matrixstr += CALCULATOR->getComma();
							matrixstr += " ";
						} else {
							b1 = true;
						}
						matrixstr += str;
					}
				}
				b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrix_store), &iter);
			}
			matrixstr += "]";
		} else {
			matrixstr = "[";
			bool b1 = false;
			for(size_t index_r = 0; index_r < (size_t) r && b; index_r++) {
				if(b1) {
					matrixstr += CALCULATOR->getComma();
					matrixstr += " ";
				} else {
					b1 = true;
				}
				matrixstr += "[";
				bool b2 = false;
				for(size_t index_c = 0; index_c < (size_t) c; index_c++) {
					if(b2) {
						matrixstr += CALCULATOR->getComma();
						matrixstr += " ";
					} else {
						b2 = true;
					}
					gtk_tree_model_get(GTK_TREE_MODEL(tMatrix_store), &iter, index_c * 2, &gstr, -1);
					str = gstr;
					remove_blank_ends(str);
					g_free(gstr);
					matrixstr += str;
				}
				matrixstr += "]";
				b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrix_store), &iter);
			}
			matrixstr += "]";
		}
		insert_text(matrixstr.c_str());
	}
	gtk_widget_hide(dialog);
}


void edit_dataobject(DataSet *ds, DataObject *o, GtkWidget *win) {
	if(!ds) return;
	GtkWidget *dialog = get_dataobject_edit_dialog();
	if(o) {
		gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Data Object"));
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Data Object"));
	}
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	GtkWidget *ptable = glade_xml_get_widget(datasets_glade, "dataobject_edit_table");
	GList *childlist = gtk_container_get_children(GTK_CONTAINER(ptable));
	for(guint i = 0; ; i++) {
		GtkWidget *w = (GtkWidget*) g_list_nth_data(childlist, i);
		if(!w) break;
		gtk_widget_destroy(w);
	}
	g_list_free(childlist);
	DataPropertyIter it;
	DataProperty *dp = ds->getFirstProperty(&it);
	string sval;
	int rows = 1;
	gtk_table_resize(GTK_TABLE(ptable), rows, 4);
	gtk_table_set_col_spacing(GTK_TABLE(ptable), 0, 20);
	GtkWidget *label, *entry, *menu, *om;
	vector<GtkWidget*> value_entries;
	vector<GtkWidget*> approx_menus;
	string str;
	while(dp) {
		
		gtk_table_resize(GTK_TABLE(ptable), rows, 4);
		
		label = gtk_label_new(dp->title().c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_table_attach(GTK_TABLE(ptable), label, 0, 1, rows - 1, rows, GTK_FILL, GTK_FILL, 0, 0);
		
		entry = gtk_entry_new();
		value_entries.push_back(entry);
		int iapprox = -1;
		if(o) {
			gtk_entry_set_text(GTK_ENTRY(entry), o->getProperty(dp, &iapprox).c_str());
		}
		gtk_table_attach(GTK_TABLE(ptable), entry, 1, 2, rows - 1, rows, GTK_FILL, GTK_FILL, 0, 0);
		
		label = gtk_label_new(dp->getUnitString().c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_table_attach(GTK_TABLE(ptable), label, 2, 3, rows - 1, rows, GTK_FILL, GTK_FILL, 0, 0);
		
		om = gtk_option_menu_new();
		approx_menus.push_back(om);
		menu = gtk_menu_new();
		gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new_with_label("Default"));
		gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new_with_label("Approximate"));
		gtk_menu_append(GTK_MENU(menu), gtk_menu_item_new_with_label("Exact"));
		gtk_option_menu_set_menu(GTK_OPTION_MENU(om), menu);
		gtk_option_menu_set_history(GTK_OPTION_MENU(om), iapprox + 1);
		
		gtk_table_attach(GTK_TABLE(ptable), om, 3, 4, rows - 1, rows, GTK_FILL, GTK_FILL, 0, 0);
		
		rows++;
		dp = ds->getNextProperty(&it);
	}
	gtk_widget_show_all(ptable);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		bool new_object = (o == NULL);
		if(new_object) {
			o = new DataObject(ds);
			ds->addObject(o);
		}
		dp = ds->getFirstProperty(&it);
		size_t i = 0;
		string val;
		while(dp) {
			val = gtk_entry_get_text(GTK_ENTRY(value_entries[i]));
			remove_blank_ends(val);
			if(!val.empty()) {
				o->setProperty(dp, val, gtk_option_menu_get_history(GTK_OPTION_MENU(approx_menus[i])) - 1);
			} else if(!new_object) {
				o->eraseProperty(dp);
			}
			dp = ds->getNextProperty(&it);
			i++;
		}
		o->setUserModified();
		selected_dataobject = o;
		update_dataobjects();
	}
	for(size_t i = 0; i < approx_menus.size(); i++) {
		menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(approx_menus[i]));
		gtk_widget_destroy(menu);
	}
	gtk_widget_hide(dialog);
}

void update_dataset_property_list(DataSet*) {
	if(!datasetedit_glade) return;
	selected_dataproperty = NULL;
	gtk_list_store_clear(tDataProperties_store);
	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_edit_property"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_del_property"), FALSE);
	GtkTreeIter iter;
	string str;
	for(size_t i = 0; i < tmp_props.size(); i++) {
		if(tmp_props[i]) {
			gtk_list_store_append(tDataProperties_store, &iter);
			str = "";
			switch(tmp_props[i]->propertyType()) {
				case PROPERTY_STRING: {
					str += _("text");
					break;
				}
				case PROPERTY_NUMBER: {
					if(tmp_props[i]->isApproximate()) {
						str += _("approximate");
						str += " ";
					}
					str += _("number");
					break;
				}
				case PROPERTY_EXPRESSION: {
					if(tmp_props[i]->isApproximate()) {
						str += _("approximate");
						str += " ";
					}
					str += _("expression");
					break;
				}
			}
			if(tmp_props[i]->isKey()) {
				str += " (";
				str += _("key");
				str += ")";
			}
			gtk_list_store_set(tDataProperties_store, &iter, 0, tmp_props[i]->title(false).c_str(), 1, tmp_props[i]->getName().c_str(), 2, str.c_str(), 3, (gpointer) tmp_props[i], -1);
		}
	}
}

bool edit_dataproperty(DataProperty *dp) {

	GtkWidget *dialog = get_dataproperty_edit_dialog();	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (datasetedit_glade, "dataset_edit_dialog")));

	edited_dataproperty = dp;	
	names_edited = false;
	editing_dataproperty = true;

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_name")), dp->getName().c_str());
	if(dp->countNames() > 1) {
		string str = "+ ";
		for(size_t i = 2; i <= dp->countNames(); i++) {
			if(i > 2) str += ", ";
			str += dp->getName(i);
		}
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_label_names")), str.c_str());
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_label_names")), "");
	}

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_title")), dp->title(false).c_str());
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_unit")), dp->getUnitString().c_str());

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_hide")), dp->isHidden());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_key")), dp->isKey());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_approximate")), dp->isApproximate());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_case")), dp->isCaseSensitive());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_brackets")), dp->usesBrackets());
	
	GtkTextBuffer *description_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_textview_description")));
	gtk_text_buffer_set_text(description_buffer, dp->description().c_str(), -1);

	switch(dp->propertyType()) {	
		case PROPERTY_STRING: {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_optionmenu_type")), 0);
			break;
		}
		case PROPERTY_NUMBER: {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_optionmenu_type")), 1);
			break;
		}
		case PROPERTY_EXPRESSION: {
			gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_optionmenu_type")), 2);
			break;
		}
	}
	
	gtk_widget_grab_focus(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_name"));
	
	bool return_val = false;

	run_dataproperty_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
	
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_name")));
		remove_blank_ends(str);
		GtkTreeIter iter;
		if(str.empty() && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter))) {
			//no name -- open dialog again
			gtk_widget_grab_focus(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_name"));
			show_message(_("Empty name field."), dialog);
			goto run_dataproperty_edit_dialog;
		}
	
		dp->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_title"))));
		dp->setUnit(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_unit"))));
		
		dp->setHidden(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_hide"))));
		dp->setKey(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_key"))));
		dp->setApproximate(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_approximate"))));
		dp->setCaseSensitive(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_case"))));
		dp->setUsesBrackets(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_checkbutton_brackets"))));
	
		GtkTextIter e_iter_s, e_iter_e;
		gtk_text_buffer_get_start_iter(description_buffer, &e_iter_s);
		gtk_text_buffer_get_end_iter(description_buffer, &e_iter_e);			
		dp->setDescription(gtk_text_buffer_get_text(description_buffer, &e_iter_s, &e_iter_e, FALSE));
		
		switch(gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_optionmenu_type")))) {
			case 0: {
				dp->setPropertyType(PROPERTY_STRING);
				break;
			}
			case 1: {
				dp->setPropertyType(PROPERTY_NUMBER);
				break;
			}
			case 2: {
				dp->setPropertyType(PROPERTY_EXPRESSION);
				break;
			}
		}
		if(names_edited) {
			dp->clearNames();
			GtkTreeIter iter;
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) {
				gchar *gstr;
				while(true) {	
					gboolean reference = FALSE;
					gtk_tree_model_get(GTK_TREE_MODEL(tNames_store), &iter, NAMES_NAME_COLUMN, &gstr, NAMES_REFERENCE_COLUMN, &reference, -1);
					dp->addName(gstr, reference);
					g_free(gstr);
					if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(tNames_store), &iter)) break;
				}
			} else {
				dp->addName(str);
			}
		} else {
			dp->setName(str, 1);
		}
		
		return_val = true;
			
	}
	
	names_edited = false;
	editing_dataproperty = false;
	edited_dataproperty = NULL;
	
	gtk_widget_hide(dialog);
	
	return return_val;
	
}


void edit_dataset(DataSet *ds, GtkWidget *win) {
	GtkWidget *dialog = get_dataset_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	
	edited_dataset = ds;
	names_edited = false;
	editing_dataset = true;
	
	if(ds) {
		if(ds->isLocal())
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Data Set"));
		else
			gtk_window_set_title(GTK_WINDOW(dialog), _("Edit Data Set (global)"));		
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), _("New Data Set"));
	}

	GtkTextBuffer *description_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (datasetedit_glade, "dataset_edit_textview_description")));
	GtkTextBuffer *copyright_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (datasetedit_glade, "dataset_edit_textview_copyright")));

	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_textview_copyright"), !ds || ds->isLocal());
	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_file"), !ds || ds->isLocal());

	//clear entries
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name")), "");
	gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (datasetedit_glade, "dataset_edit_label_names")), "");
	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name"), TRUE);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_desc")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_file")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_object_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_property_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_default_property")), _("info"));
	gtk_text_buffer_set_text(description_buffer, "", -1);
	gtk_text_buffer_set_text(copyright_buffer, "", -1);

	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_ok"), TRUE);	

	gtk_list_store_clear(tDataProperties_store);
	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_edit_property"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_del_property"), FALSE);		
	gtk_widget_set_sensitive(glade_xml_get_widget (datasetedit_glade, "dataset_edit_button_new_property"), TRUE);
	
	if(ds) {
		//fill in original paramaters
		set_name_label_and_entry(ds, glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name"), glade_xml_get_widget (datasetedit_glade, "dataset_edit_label_names"));
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_desc")), ds->title(false).c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_file")), ds->defaultDataFile().c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_default_property")), ds->defaultProperty().c_str());
		Argument *arg = ds->getArgumentDefinition(1);
		if(arg) {
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_object_name")), arg->name().c_str());
		}
		arg = ds->getArgumentDefinition(2);
		if(arg) {
			gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_property_name")), arg->name().c_str());
		}
		gtk_text_buffer_set_text(description_buffer, ds->description().c_str(), -1);
		gtk_text_buffer_set_text(copyright_buffer, ds->copyright().c_str(), -1);
		DataPropertyIter it;
		DataProperty *dp = ds->getFirstProperty(&it);
		while(dp) {
			tmp_props.push_back(new DataProperty(*dp));
			tmp_props_orig.push_back(dp);
			dp = ds->getNextProperty(&it);
		}
	}
	update_dataset_property_list(ds);
	
	gtk_widget_grab_focus(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name"));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (datasetedit_glade, "dataset_edit_tabs")), 0);
	
	run_dataset_edit_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name")));
		remove_blank_ends(str);
		GtkTreeIter iter;
		if(str.empty() && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter))) {
			//no name -- open dialog again
			gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (datasetedit_glade, "dataset_edit_tabs")), 2);
			gtk_widget_grab_focus(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name"));
			show_message(_("Empty name field."), dialog);
			goto run_dataset_edit_dialog;
		}
		GtkTextIter d_iter_s, d_iter_e;
		gtk_text_buffer_get_start_iter(description_buffer, &d_iter_s);
		gtk_text_buffer_get_end_iter(description_buffer, &d_iter_e);
		GtkTextIter c_iter_s, c_iter_e;
		gtk_text_buffer_get_start_iter(copyright_buffer, &c_iter_s);
		gtk_text_buffer_get_end_iter(copyright_buffer, &c_iter_e);
		//dataset with the same name exists -- overwrite or open the dialog again
		if((!ds || !ds->hasName(str)) && (!names_edited || !gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) && CALCULATOR->functionNameTaken(str, ds) && !ask_question(_("A function with the same name already exists.\nDo you want to overwrite the function?"), dialog)) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (datasetedit_glade, "dataset_edit_tabs")), 2);
			gtk_widget_grab_focus(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name"));
			goto run_dataset_edit_dialog;
		}
		bool add_func = false;
		if(ds) {
			//edited an existing dataset
			ds->setTitle(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_desc"))));
			if(ds->isLocal()) ds->setDefaultDataFile(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_file"))));
			ds->setDescription(gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE));
		} else {
			//new dataset
			ds = new DataSet(_("Data Sets"), "", gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_file"))), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_desc"))), gtk_text_buffer_get_text(description_buffer, &d_iter_s, &d_iter_e, FALSE), true);
			add_func = true;
		}
		string str2;
		if(ds) {
			str2 = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_object_name")));
			remove_blank_ends(str2);
			if(str2.empty()) str2 = _("Object");
			Argument *arg = ds->getArgumentDefinition(1);
			if(arg) {
				arg->setName(str2);
			}
			str2 = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_property_name")));
			remove_blank_ends(str2);
			if(str2.empty()) str2 = _("Property");
			arg = ds->getArgumentDefinition(2);
			if(arg) {
				arg->setName(str2);
			}
			ds->setDefaultProperty(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_default_property"))));
			ds->setCopyright(gtk_text_buffer_get_text(copyright_buffer, &c_iter_s, &c_iter_e, FALSE));
			DataPropertyIter it;
			for(size_t i = 0; i < tmp_props.size();) {
				if(!tmp_props[i]) {
					if(tmp_props_orig[i]) ds->delProperty(tmp_props_orig[i]);
					i++;
				} else if(tmp_props[i]->isUserModified()) {
					if(tmp_props_orig[i]) {
						tmp_props_orig[i]->set(*tmp_props[i]);
						i++;
					} else {
						ds->addProperty(tmp_props[i]);
						tmp_props.erase(tmp_props.begin() + i);
					}
				} else {
					i++;
				}
			}
			set_edited_names(ds, str);
			if(add_func) {
				CALCULATOR->addDataSet(ds);
				ds->loadObjects();
				ds->setObjectsLoaded(true);
			}
			selected_dataset = ds;
		}
		update_fmenu();	
		function_inserted(ds);
		update_datasets_tree();
	}
	for(size_t i = 0; i < tmp_props.size(); i++) {
		if(tmp_props[i]) delete tmp_props[i];
	}
	tmp_props.clear();
	tmp_props_orig.clear();
	edited_dataset = NULL;
	editing_dataset = false;
	names_edited = false;
	gtk_widget_hide(dialog);
}

void import_csv_file(GtkWidget *win) {

	GtkWidget *dialog = get_csv_import_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_name")), "");
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file")), "");	
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_desc")), "");
	
	gtk_widget_grab_focus(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file"));

run_csv_import_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no filename -- open dialog again
			gtk_widget_grab_focus(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file"));
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
			gtk_widget_grab_focus(glade_xml_get_widget (csvimport_glade, "csv_import_entry_delimiter_other"));
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
}

void export_csv_file(KnownVariable *v, GtkWidget *win) {

	GtkWidget *dialog = get_csv_export_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));

	if(v) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")), v->preferredDisplayName(false, false, false, false, &can_display_unicode_string_function, (void*) glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")).name.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix")), v->preferredDisplayName(false, false, false, false, &can_display_unicode_string_function, (void*) glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix")).name.c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_matrix")), TRUE);
	} else {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix")), "");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_current")), TRUE);
	}
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_matrix"), !v);
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_radiobutton_current"), !v);
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix"), FALSE);
	
	gtk_widget_grab_focus(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file"));

run_csv_export_dialog:
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		//clicked "OK"
		string str = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")));
		remove_blank_ends(str);
		if(str.empty()) {
			//no filename -- open dialog again
			gtk_widget_grab_focus(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file"));
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
			gtk_widget_grab_focus(glade_xml_get_widget (csvexport_glade, "csv_export_entry_delimiter_other"));
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
				gtk_widget_grab_focus(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix"));
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
				gtk_widget_grab_focus(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix"));
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
	
}

void edit_names(ExpressionItem *item, const gchar *namestr, GtkWidget *win, bool is_dp, DataProperty *dp) {

	GtkWidget *dialog = get_names_edit_dialog();
	if(win) gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(win));
	
	GtkTreeIter iter;
	
	if(!names_edited) {
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_button_modify"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_button_remove"), FALSE);
	
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name")), "");
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference")), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation")), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural")), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_suffix")), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_avoid_input")), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive")), FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_unicode")), FALSE);
		
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation"), !is_dp);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural"), !is_dp);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_suffix"), !is_dp);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_avoid_input"), !is_dp);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive"), !is_dp);
		gtk_widget_set_sensitive(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_unicode"), !is_dp);

		gtk_list_store_clear(tNames_store);
		
		if(!is_dp && item && item->countNames() > 0) {
			for(size_t i = 1; i <= item->countNames(); i++) {
				const ExpressionName *ename = &item->getName(i);
				gtk_list_store_append(tNames_store, &iter);
				gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, ename->name.c_str(), NAMES_ABBREVIATION_STRING_COLUMN, b2yn(ename->abbreviation), NAMES_PLURAL_STRING_COLUMN, b2yn(ename->plural), NAMES_REFERENCE_STRING_COLUMN, b2yn(ename->reference), NAMES_ABBREVIATION_COLUMN, ename->abbreviation, NAMES_PLURAL_COLUMN, ename->plural, NAMES_UNICODE_COLUMN, ename->unicode, NAMES_REFERENCE_COLUMN, ename->reference, NAMES_SUFFIX_COLUMN, ename->suffix, NAMES_AVOID_INPUT_COLUMN, ename->avoid_input, NAMES_CASE_SENSITIVE_COLUMN, ename->case_sensitive, -1);
				if(i == 1 && namestr && strlen(namestr) > 0) {
					gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, namestr, -1);
				}
			}
		} else if(is_dp && dp && dp->countNames() > 0) {
			for(size_t i = 1; i <= dp->countNames(); i++) {
				gtk_list_store_append(tNames_store, &iter);
				gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, dp->getName(i).c_str(), NAMES_ABBREVIATION_STRING_COLUMN, "-", NAMES_PLURAL_STRING_COLUMN, "-", NAMES_REFERENCE_STRING_COLUMN, b2yn(dp->nameIsReference(i)), NAMES_ABBREVIATION_COLUMN, FALSE, NAMES_PLURAL_COLUMN, FALSE, NAMES_UNICODE_COLUMN, FALSE, NAMES_REFERENCE_COLUMN, dp->nameIsReference(i), NAMES_SUFFIX_COLUMN, FALSE, NAMES_AVOID_INPUT_COLUMN, FALSE, NAMES_CASE_SENSITIVE_COLUMN, FALSE, -1);
				if(i == 1 && namestr && strlen(namestr) > 0) {
					gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, namestr, -1);
				}
			}
		} else if(namestr && strlen(namestr) > 0) {
			gtk_list_store_append(tNames_store, &iter);
			if(is_dp) {
				gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, namestr, NAMES_ABBREVIATION_STRING_COLUMN, "-", NAMES_PLURAL_STRING_COLUMN, "-", NAMES_REFERENCE_STRING_COLUMN, b2yn(true), NAMES_ABBREVIATION_COLUMN, FALSE, NAMES_PLURAL_COLUMN, FALSE, NAMES_UNICODE_COLUMN, FALSE, NAMES_REFERENCE_COLUMN, TRUE, NAMES_SUFFIX_COLUMN, FALSE, NAMES_AVOID_INPUT_COLUMN, FALSE, NAMES_CASE_SENSITIVE_COLUMN, FALSE, -1);
			} else {
				ExpressionName ename(namestr);	
				gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, ename.name.c_str(), NAMES_ABBREVIATION_STRING_COLUMN, b2yn(ename.abbreviation), NAMES_PLURAL_STRING_COLUMN, b2yn(ename.plural), NAMES_REFERENCE_STRING_COLUMN, b2yn(ename.reference), NAMES_ABBREVIATION_COLUMN, ename.abbreviation, NAMES_PLURAL_COLUMN, ename.plural, NAMES_UNICODE_COLUMN, ename.unicode, NAMES_REFERENCE_COLUMN, ename.reference, NAMES_SUFFIX_COLUMN, ename.suffix, NAMES_AVOID_INPUT_COLUMN, ename.avoid_input, NAMES_CASE_SENSITIVE_COLUMN, ename.case_sensitive, -1);
			}
		}
	} else if(namestr && strlen(namestr) > 0) {
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) {
			gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, namestr, -1);
		}
		on_tNames_selection_changed(gtk_tree_view_get_selection(GTK_TREE_VIEW(tNames)), NULL);
	}
	
	gtk_dialog_run(GTK_DIALOG(dialog));
	names_edited = true;
	
	gtk_widget_hide(dialog);
}


/*
	add a new variable (from menu) with the value of result
*/
void add_as_variable()
{
	edit_variable(_("Temporary"), NULL, mstruct, glade_xml_get_widget (main_glade, "main_window"));
}

void new_unknown(GtkMenuItem*, gpointer)
{
	edit_unknown(_("My Variables"), NULL, glade_xml_get_widget (main_glade, "main_window"));
}

/*
	add a new variable (from menu)
*/
void new_variable(GtkMenuItem*, gpointer)
{
	edit_variable(_("My Variables"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"));
}

/*
	add a new matrix (from menu)
*/
void new_matrix(GtkMenuItem*, gpointer)
{
	edit_matrix(_("Matrices"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), FALSE);
}
/*
	add a new vector (from menu)
*/
void new_vector(GtkMenuItem*, gpointer)
{
	edit_matrix(_("Vectors"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), TRUE);
}

/*
	insert one-argument function when button clicked
*/
void insertButtonFunction(const gchar *text, bool append_space = true) {
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
void insertButtonFunction(MathFunction *f) {
	if(rpn_mode && f->args() == 1) {
		calculateRPN(f);
		return;
	}
	const ExpressionName *ename = &f->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression);
	insertButtonFunction(ename->name.c_str(), !text_length_is_one(ename->name));
}

/*
	Button clicked -- insert text (1,2,3,... +,-,...)
*/
void button_pressed(GtkButton*, gpointer user_data) {
	insert_text((gchar*) user_data);
}

/*
	Button clicked -- insert corresponding function
*/
void button_function_pressed(GtkButton*, gpointer user_data) {
	insertButtonFunction((gchar*) user_data);
}

/*
	Update angle menu
*/
void set_angle_item() {
	GtkWidget *mi = NULL;
	switch(evalops.parse_options.angle_unit) {
		case ANGLE_UNIT_RADIANS: {
			mi = glade_xml_get_widget (main_glade, "menu_item_radians");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_radians_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_radians_activate, NULL);						
			break;
		}
		case ANGLE_UNIT_GRADIANS: {
			mi = glade_xml_get_widget (main_glade, "menu_item_gradians");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_gradians_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_gradians_activate, NULL);						
			break;
		}
		case ANGLE_UNIT_DEGREES: {
			mi = glade_xml_get_widget (main_glade, "menu_item_degrees");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_degrees_activate, NULL);	
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_degrees_activate, NULL);
			break;
		}
		default: {
			mi = glade_xml_get_widget (main_glade, "menu_item_no_default_angle_unit");
			g_signal_handlers_block_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_no_default_angle_unit_activate, NULL);
			if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mi)))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mi), TRUE);
			g_signal_handlers_unblock_matched((gpointer) mi, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_no_default_angle_unit_activate, NULL);
		}
	}
}

/*
	Update angle radio buttons
*/
void set_angle_button() {
	GtkWidget *tb = NULL;
	switch(evalops.parse_options.angle_unit) {
		case ANGLE_UNIT_RADIANS: {
			tb = glade_xml_get_widget (main_glade, "radiobutton_radians");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_radians_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_radians_toggled, NULL);							
			break;
		}
		case ANGLE_UNIT_GRADIANS: {
			tb = glade_xml_get_widget (main_glade, "radiobutton_gradians");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_gradians_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_gradians_toggled, NULL);							
			break;
		}
		case ANGLE_UNIT_DEGREES: {
			tb = glade_xml_get_widget (main_glade, "radiobutton_degrees");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_degrees_toggled, NULL);		
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_degrees_toggled, NULL);
			break;
		}
		default: {
			tb = glade_xml_get_widget (main_glade, "radiobutton_no_default_angle_unit");
			g_signal_handlers_block_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_no_default_angle_unit_toggled, NULL);
			if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb)))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), TRUE);
			g_signal_handlers_unblock_matched((gpointer) tb, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_radiobutton_no_default_angle_unit_toggled, NULL);
			break;
		}
	}

}

/*
	variables, functions and units enabled/disabled from menu
*/
void set_clean_mode(GtkMenuItem *w, gpointer) {
	gboolean b = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	evalops.parse_options.functions_enabled = !b;
	evalops.parse_options.variables_enabled = !b;
	evalops.parse_options.units_enabled = !b;
	expression_format_updated(true);
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
void on_omToUnit_menu_activate(GtkMenuItem*, gpointer user_data) {
	selected_to_unit = (Unit*) user_data;
	convert_in_wUnits();
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
		bool b = false;
		if(toFrom > 0) {
			if(uFrom == uTo) {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_from_val")), toValue);
			} else {
				EvaluationOptions eo;
				eo.approximation = APPROXIMATION_APPROXIMATE;
				eo.parse_options.angle_unit = evalops.parse_options.angle_unit;
				PrintOptions po;
				po.is_approximate = &b;
				po.number_fraction_format = FRACTION_DECIMAL;
				MathStructure v_mstruct = CALCULATOR->convert(CALCULATOR->unlocalizeExpression(toValue, evalops.parse_options), uTo, uFrom, eo);
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_from_val")), v_mstruct.print(po).c_str());
				b = b || v_mstruct.isApproximate();
			}
		} else {
			if(uFrom == uTo) {
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_to_val")), fromValue);
			} else {
				EvaluationOptions eo;
				eo.approximation = APPROXIMATION_APPROXIMATE;
				eo.parse_options.angle_unit = evalops.parse_options.angle_unit;
				PrintOptions po;
				po.is_approximate = &b;
				po.number_fraction_format = FRACTION_DECIMAL;
				MathStructure v_mstruct = CALCULATOR->convert(CALCULATOR->unlocalizeExpression(fromValue, evalops.parse_options), uFrom, uTo, eo);
				gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_to_val")), v_mstruct.print(po).c_str());
				b = b || v_mstruct.isApproximate();
			}
		}
		if(b && printops.use_unicode_signs && can_display_unicode_string_function(SIGN_ALMOST_EQUAL, (void*) glade_xml_get_widget (units_glade, "units_label_equals"))) {
			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (units_glade, "units_label_equals")), SIGN_ALMOST_EQUAL);
		} else {
			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (units_glade, "units_label_equals")), "=");
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
	modes[1].precision = CALCULATOR->getPrecision();
	modes[1].po = printops;
	modes[1].po.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
	modes[1].eo = evalops;
	modes[1].at = CALCULATOR->defaultAssumptions()->type();
	modes[1].as = CALCULATOR->defaultAssumptions()->sign();
	modes[1].rpn_mode = rpn_mode;
}

size_t save_mode_as(string name, bool *new_mode = NULL) {
	remove_blank_ends(name);
	size_t index = 0;
	for(; index < modes.size(); index++) {
		if(modes[index].name == name) {
			if(new_mode) *new_mode = false;
			break;
		}
	}
	if(index >= modes.size()) {
		modes.resize(modes.size() + 1);
		index = modes.size() - 1;
		if(new_mode) *new_mode = true;
	}
	modes[index].po = printops;
	modes[index].po.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
	modes[index].eo = evalops;
	modes[index].precision = CALCULATOR->getPrecision();
	modes[index].at = CALCULATOR->defaultAssumptions()->type();
	modes[index].as = CALCULATOR->defaultAssumptions()->sign();
	modes[index].name = name;
	modes[index].rpn_mode = rpn_mode;
	return index;
}

void load_mode(const mode_struct &mode) {
	block_result_update = true;
	block_expression_execution = true;
	block_display_parse = true;
	set_mode_items(mode.po, mode.eo, mode.at, mode.as, mode.rpn_mode, mode.precision, false);
	block_result_update = false;
	block_expression_execution = false;
	block_display_parse = false;
	printops.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
	set_rpn_mode(mode.rpn_mode);
	string str = gtk_entry_get_text(GTK_ENTRY(expression));
	if(expression_has_changed || str.find_first_not_of(SPACES) == string::npos) {
		setResult(NULL, true, false, false);
	} else {
		execute_expression(false);
	}
	expression_has_changed2 = true;
	display_parse_status();
}
void load_mode(string name) {
	for(size_t i = 0; i < modes.size(); i++) {
		if(modes[i].name == name) {
			load_mode(modes[i]);
			return;
		}
	}
}
void load_mode(size_t index) {
	if(index < modes.size()) {
		load_mode(modes[index]);
	}
}

void on_popup_menu_item_clear_history_active(GtkMenuItem*, gpointer) {
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history")));
	inhistory.clear();
	inhistory_type.clear();
	initial_result_pos = NULL;
	initial_result_index = 0;
	gtk_text_buffer_set_text(tb, "", 0);
}
void on_history_populate_popup(GtkTextView*, GtkMenu *sub, gpointer) {
	GtkWidget *item;
	MENU_SEPARATOR
	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLEAR, accel_group);
	gtk_widget_show(item);
	gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(on_popup_menu_item_clear_history_active), NULL); 
	gtk_menu_shell_append(GTK_MENU_SHELL(sub), item);
	gtk_widget_set_sensitive(item, !inhistory.empty());
}
void on_popup_menu_item_disable_completion_activate(GtkMenuItem*, gpointer) {
	enable_completion = false;
	update_completion();
}
void on_popup_menu_item_enable_completion_activate(GtkMenuItem*, gpointer) {
	enable_completion = true;
	update_completion();
}
void on_popup_menu_item_read_precision_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_read_precision")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_limit_implicit_multiplication_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_limit_implicit_multiplication")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_rpn_mode_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_rpn_mode")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_expression_populate_popup(GtkEntry*, GtkMenu *menu, gpointer) {
	GtkWidget *item, *sub, *sub2;
	sub = GTK_WIDGET(menu);
	MENU_SEPARATOR
	if(enable_completion) {
		MENU_ITEM(_("Disable Completion"), on_popup_menu_item_disable_completion_activate)
	} else {
		MENU_ITEM(_("Enable Completion"), on_popup_menu_item_enable_completion_activate)
	}
	MENU_SEPARATOR
	CHECK_MENU_ITEM(_("Read Precision"), on_popup_menu_item_read_precision_activate, evalops.parse_options.read_precision)
	CHECK_MENU_ITEM(_("Limit Implicit Multiplication"), on_popup_menu_item_limit_implicit_multiplication_activate, evalops.parse_options.limit_implicit_multiplication)
	CHECK_MENU_ITEM(_("RPN Mode"), on_popup_menu_item_rpn_mode_activate, rpn_mode)
	sub2 = sub;
	SUBMENU_ITEM(_("Meta Modes"), sub2)
	for(size_t i = 0; i < modes.size(); i++) {
		item = gtk_menu_item_new_with_label(modes[i].name.c_str()); 
		gtk_widget_show(item); 
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(on_menu_item_meta_mode_activate), (gpointer) modes[i].name.c_str()); 
		gtk_menu_shell_insert(GTK_MENU_SHELL(sub), item, (gint) i);
	}
	MENU_SEPARATOR
	MENU_ITEM(_("Save Mode..."), on_menu_item_meta_mode_save_activate)	
	MENU_ITEM(_("Delete Mode..."), on_menu_item_meta_mode_delete_activate)
	gtk_widget_set_sensitive(item, modes.size() > 2);
	sub = sub2;
	MENU_SEPARATOR
	MENU_ITEM(_("Insert Matrix..."), on_menu_item_insert_matrix_activate)
	MENU_ITEM(_("Insert Vector..."), on_menu_item_insert_vector_activate)
}

void on_combobox_base_changed(GtkComboBox *w, gpointer) {
	switch(gtk_combo_box_get_active(w)) {
		case 0: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_binary")), TRUE);
			break;
		}
		case 1: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_octal")), TRUE);
			break;
		}
		case 2: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_decimal")), TRUE);
			break;
		}
		case 3: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_hexadecimal")), TRUE);
			break;
		}
		case 4: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sexagesimal")), TRUE);
			break;
		}
		case 5: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_time_format")), TRUE);
			break;
		}
		case 6: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_roman")), TRUE);
			break;
		}
		case 7: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_custom_base")), TRUE);
			break;
		}
	}
}
void on_combobox_numerical_display_changed(GtkComboBox *w, gpointer) {
	switch(gtk_combo_box_get_active(w)) {
		case 0: {
			printops.negative_exponents = false;
			printops.sort_options.minus_last = true;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_normal")), TRUE);
			break;
		}
		case 1: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_engineering")), TRUE);
			break;
		}
		case 2: {
			printops.negative_exponents = true;
			printops.sort_options.minus_last = false;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_scientific")), TRUE);
			break;
		}
		case 3: {
			printops.negative_exponents = true;
			printops.sort_options.minus_last = false;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_purely_scientific")), TRUE);
			break;
		}
		case 4: {
			printops.negative_exponents = false;
			printops.sort_options.minus_last = true;
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_non_scientific")), TRUE);
			break;
		}
	}
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_negative_exponents")), printops.negative_exponents);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sort_minus_last")), printops.sort_options.minus_last);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
}

void show_tabs(bool do_show) {
	if(do_show == GTK_WIDGET_VISIBLE(tabs)) return;
	gint w, h;
	gtk_window_get_size(GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), &w, &h);
	if(do_show) {
		gtk_widget_show(tabs);
		h += tabs->allocation.height + 6;		
		gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), w, h);
	} else {
		h -= tabs->allocation.height + 6;
		gtk_widget_hide(tabs);
		set_result_size_request();
		gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), w, h);
	}
}

void on_expander_keypad_expanded(GObject *o, GParamSpec*, gpointer) {
	if(gtk_expander_get_expanded(GTK_EXPANDER(o))) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), 0);
		if(gtk_expander_get_expanded(GTK_EXPANDER(expander_history))) {
			gtk_expander_set_expanded(GTK_EXPANDER(expander_history), FALSE);
		} else if(gtk_expander_get_expanded(GTK_EXPANDER(expander_stack))) {
			gtk_expander_set_expanded(GTK_EXPANDER(expander_stack), FALSE);
		} else {
			show_tabs(true);	
		}		
	} else if(!gtk_expander_get_expanded(GTK_EXPANDER(expander_history)) && !gtk_expander_get_expanded(GTK_EXPANDER(expander_stack))) {
		show_tabs(false);
	}
}
void on_expander_history_expanded(GObject *o, GParamSpec*, gpointer) {
	if(gtk_expander_get_expanded(GTK_EXPANDER(o))) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), 1);
		if(gtk_expander_get_expanded(GTK_EXPANDER(expander_keypad))) {
			gtk_expander_set_expanded(GTK_EXPANDER(expander_keypad), FALSE);
		} else if(gtk_expander_get_expanded(GTK_EXPANDER(expander_stack))) {
			gtk_expander_set_expanded(GTK_EXPANDER(expander_stack), FALSE);
		} else {
			show_tabs(true);	
		}		
	} else if(!gtk_expander_get_expanded(GTK_EXPANDER(expander_keypad)) && !gtk_expander_get_expanded(GTK_EXPANDER(expander_stack))) {
		show_tabs(false);
	}
}
void on_expander_stack_expanded(GObject *o, GParamSpec*, gpointer) {
	if(gtk_expander_get_expanded(GTK_EXPANDER(o))) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), 2);
		if(gtk_expander_get_expanded(GTK_EXPANDER(expander_keypad))) {
			gtk_expander_set_expanded(GTK_EXPANDER(expander_keypad), FALSE);
		} else if(gtk_expander_get_expanded(GTK_EXPANDER(expander_history))) {
			gtk_expander_set_expanded(GTK_EXPANDER(expander_history), FALSE);
		} else {
			show_tabs(true);	
		}		
	} else if(!gtk_expander_get_expanded(GTK_EXPANDER(expander_keypad)) && !gtk_expander_get_expanded(GTK_EXPANDER(expander_history))) {
		show_tabs(false);
	}
}

void on_menu_item_meta_mode_activate(GtkMenuItem*, gpointer user_data) {
	const char *name = (const char*) user_data;
	load_mode(name);
}
void on_menu_item_meta_mode_save_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Save Mode"), GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	GtkWidget *hbox = gtk_hbox_new(TRUE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 12);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
	gtk_widget_show(hbox);
	GtkWidget *label = gtk_label_new(_("Name"));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	gtk_widget_show(label);
	GtkWidget *entry = gtk_combo_box_entry_new_text();
	for(size_t i = 2; i < modes.size(); i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(entry), modes[i].name.c_str());
	}
	gtk_box_pack_end(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	gtk_widget_show(entry);
run_meta_mode_save_dialog:
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_ACCEPT) {
		bool new_mode = true;
#if GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION < 6
		string name = gtk_entry_get_text(GTK_ENTRY(GTK_BIN(entry)->child));
#else
		string name = gtk_combo_box_get_active_text(GTK_COMBO_BOX(entry));
#endif
		remove_blank_ends(name);
		if(name.empty()) {
			show_message(_("Empty name field."), dialog);
			goto run_meta_mode_save_dialog;
		}
		if(name == modes[0].name) {
			show_message(_("Preset mode cannot be overwritten."), dialog);
			goto run_meta_mode_save_dialog;
		}
		size_t index = save_mode_as(name, &new_mode);
		if(new_mode) {
			GtkWidget *item = gtk_menu_item_new_with_label(modes[index].name.c_str()); 
			gtk_widget_show(item); 
			gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(on_menu_item_meta_mode_activate), (gpointer) modes[index].name.c_str()); 
			gtk_menu_shell_insert(GTK_MENU_SHELL(glade_xml_get_widget (main_glade, "menu_meta_modes")), item, (gint) index);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_meta_mode_delete"), TRUE);
			mode_items.push_back(item);
			item = gtk_menu_item_new_with_label(modes[index].name.c_str()); 
			gtk_widget_show(item); 
			gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(on_menu_item_meta_mode_activate), (gpointer) modes[index].name.c_str()); 
			gtk_menu_shell_insert(GTK_MENU_SHELL(glade_xml_get_widget (main_glade, "menu_result_popup_meta_modes")), item, (gint) index);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_result_popup_meta_mode_delete"), TRUE);
			popup_result_mode_items.push_back(item);
		}
	}
	gtk_widget_destroy(dialog);
}
void on_menu_item_meta_mode_delete_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Delete Mode"), GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), GTK_STOCK_DELETE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	GtkWidget *hbox = gtk_hbox_new(TRUE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 12);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), hbox);
	gtk_widget_show(hbox);
	GtkWidget *label = gtk_label_new(_("Mode"));
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	gtk_widget_show(label);
	GtkWidget *menu = gtk_combo_box_new_text();
	for(size_t i = 2; i < modes.size(); i++) {
		gtk_combo_box_append_text(GTK_COMBO_BOX(menu), modes[i].name.c_str());
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(menu), 0);
	gtk_box_pack_end(GTK_BOX(hbox), menu, FALSE, TRUE, 0);
	gtk_widget_show(menu);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_ACCEPT && gtk_combo_box_get_active(GTK_COMBO_BOX(menu)) >= 0) {
		size_t index = gtk_combo_box_get_active(GTK_COMBO_BOX(menu)) + 2;
		gtk_widget_destroy(mode_items[index]);
		gtk_widget_destroy(popup_result_mode_items[index]);
		modes.erase(modes.begin() + index);
		mode_items.erase(mode_items.begin() + index);
		popup_result_mode_items.erase(popup_result_mode_items.begin() + index);
		if(modes.size() < 3) {
			gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_meta_mode_delete"), FALSE);
			gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_result_popup_meta_mode_delete"), FALSE);
		}
	}
	gtk_widget_destroy(dialog);
}

/*
	load preferences from ~/.qalculate/qalculate-gtk.cfg
*/
void load_preferences() {

	default_plot_legend_placement = PLOT_LEGEND_TOP_RIGHT;
	default_plot_display_grid = true;
	default_plot_full_border = false;
	default_plot_min = "0";
	default_plot_max = "10";
	default_plot_step = "1";
	default_plot_sampling_rate = 100;
	default_plot_rows = false;
	default_plot_type = 0;
	default_plot_style = PLOT_STYLE_LINES;
	default_plot_smoothing = PLOT_SMOOTHING_NONE;
	default_plot_variable = "x";
	default_plot_color = true;
	default_plot_use_sampling_rate = true;

	printops.multiplication_sign = MULTIPLICATION_SIGN_DOT;
	printops.division_sign = DIVISION_SIGN_DIVISION_SLASH;
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
	printops.show_ending_zeroes = false;
	printops.round_halfway_to_even = false;
	printops.number_fraction_format = FRACTION_DECIMAL;
	printops.abbreviate_names = true;
	printops.use_unicode_signs = true;
	printops.use_unit_prefixes = true;
	printops.spacious = true;
	printops.short_multiplication = true;
	printops.place_units_separately = true;
	printops.use_all_prefixes = false;
	printops.excessive_parenthesis = false;
	printops.allow_non_usable = false;
	printops.lower_case_numbers = false;
	printops.lower_case_e = false;
	printops.base_display = BASE_DISPLAY_NORMAL;
	printops.limit_implicit_multiplication = false;
	printops.can_display_unicode_string_function = &can_display_unicode_string_function;
	printops.allow_factorization = false;
	printops.spell_out_logical_operators = true;
	
	evalops.approximation = APPROXIMATION_TRY_EXACT;
	evalops.sync_units = true;
	evalops.structuring = STRUCTURING_SIMPLIFY;
	evalops.parse_options.unknowns_enabled = false;
	evalops.parse_options.read_precision = DONT_READ_PRECISION;
	evalops.parse_options.base = BASE_DECIMAL;
	evalops.allow_complex = true;
	evalops.allow_infinite = true;
	evalops.auto_post_conversion = POST_CONVERSION_NONE;
	evalops.assume_denominators_nonzero = true;
	evalops.warn_about_denominators_assumed_nonzero = true;
	evalops.parse_options.limit_implicit_multiplication = false;
	evalops.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
	evalops.parse_options.dot_as_separator = CALCULATOR->default_dot_as_separator;

	rpn_mode = false;
	rpn_keypad_only = true;
	
	save_mode_as(_("Preset"));
	save_mode_as(_("Default"));
	size_t mode_index = 1;

	win_width = -1;
	win_height = -1;	
	save_mode_on_exit = true;
	save_defs_on_exit = true;
	hyp_is_on = false;
	inv_is_on = false;
	use_custom_result_font = false;
	use_custom_expression_font = false;
	use_custom_status_font = false;
	custom_result_font = "";
	custom_expression_font = "";
	custom_status_font = "";
	status_error_color = "#FF0000";
	status_warning_color = "#0000FF";
	show_keypad = true;
	show_history = false;
	show_stack = true;
	load_global_defs = true;
	fetch_exchange_rates_at_startup = false;
	display_expression_status = true;
	enable_completion = true;
	wget_args = "--quiet --tries=1";
	first_time = false;
	expression_history.clear();
	expression_history_index = -1;
	gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", NULL);
	DIR *dir = opendir(gstr);
	g_free(gstr);
	if(!dir) {
		first_qalculate_run = true;
		first_time = true;	
		return;
	} else {
		first_qalculate_run = false;
		closedir(dir);
	}
	int version_numbers[] = {0, 9, 6};
	bool old_history_format = false;

	FILE *file = NULL;
	gchar *gstr2 = g_build_filename(g_get_home_dir(), ".qalculate", "qalculate-gtk.cfg", NULL);
	file = fopen(gstr2, "r");
	g_free(gstr2);
	if(file) {
		char line[10000];
		string stmp, svalue, svar;
		size_t i;
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
				if(svar == "version") {
					parse_qalculate_version(svalue, version_numbers);
					old_history_format = (version_numbers[0] == 0 && (version_numbers[1] < 9 || (version_numbers[1] == 9 && version_numbers[2] <= 4)));
				} else if(svar == "width") {
					win_width = v;
				} else if(svar == "height") {
					win_height = v;
				} else if(svar == "save_mode_on_exit") {
					save_mode_on_exit = v;
				} else if(svar == "save_definitions_on_exit") {
					save_defs_on_exit = v;
				} else if(svar == "fetch_exchange_rates_at_startup") {
					fetch_exchange_rates_at_startup = v;
				} else if(svar == "wget_args") {
					wget_args = svalue;
				} else if(svar == "show_keypad") {
					show_keypad = v;
				} else if(svar == "show_history") {
					show_history = v;
				} else if(svar == "show_stack") {
					show_stack = v;
				} else if(svar == "display_expression_status") {
					display_expression_status = v;
				} else if(svar == "enable_completion") {
					enable_completion = v;
				} else if(svar == "min_deci") {
					if(mode_index == 1) printops.min_decimals = v;
					else modes[mode_index].po.min_decimals = v;
				} else if(svar == "use_min_deci") {
					if(mode_index == 1) printops.use_min_decimals = v;
					else modes[mode_index].po.use_min_decimals = v;
				} else if(svar == "max_deci") {
					if(mode_index == 1) printops.max_decimals = v;
					else modes[mode_index].po.max_decimals = v;
				} else if(svar == "use_max_deci") {
					if(mode_index == 1) printops.use_max_decimals = v;
					else modes[mode_index].po.use_max_decimals = v;
				} else if(svar == "precision") {
					if(mode_index == 1) CALCULATOR->setPrecision(v);
					else modes[mode_index].precision = v;
				} else if(svar == "min_exp") {
					if(mode_index == 1) printops.min_exp = v;
					else modes[mode_index].po.min_exp = v;
				} else if(svar == "negative_exponents") {
					if(mode_index == 1) printops.negative_exponents = v;
					else modes[mode_index].po.negative_exponents = v;
				} else if(svar == "sort_minus_last") {
					if(mode_index == 1) printops.sort_options.minus_last = v;
					else modes[mode_index].po.sort_options.minus_last = v;
				} else if(svar == "place_units_separately") {
					if(mode_index == 1) printops.place_units_separately = v;
					else modes[mode_index].po.place_units_separately = v;
				} else if(svar == "display_mode") {	//obsolete
					switch(v) {
						case 1: {
							if(mode_index == 1) {
								printops.min_exp = EXP_PRECISION;
								printops.negative_exponents = false;
								printops.sort_options.minus_last = true;
							} else {
								modes[mode_index].po.min_exp = EXP_PRECISION;
								modes[mode_index].po.negative_exponents = false;
								modes[mode_index].po.sort_options.minus_last = true;
							}
							break;
						}
						case 2: {
							if(mode_index == 1) {
								printops.min_exp = EXP_SCIENTIFIC;
								printops.negative_exponents = true;
								printops.sort_options.minus_last = false;
							} else {
								modes[mode_index].po.min_exp = EXP_SCIENTIFIC;
								modes[mode_index].po.negative_exponents = true;
								modes[mode_index].po.sort_options.minus_last = false;
							}
							break;
						}
						case 3: {
							if(mode_index == 1) {
								printops.min_exp = EXP_PURE;
								printops.negative_exponents = true;
								printops.sort_options.minus_last = false;
							} else {
								modes[mode_index].po.min_exp = EXP_PURE;
								modes[mode_index].po.negative_exponents = true;
								modes[mode_index].po.sort_options.minus_last = false;
							}
							break;
						}
						case 4: {
							if(mode_index == 1) {
								printops.min_exp = EXP_NONE;
								printops.negative_exponents = false;
								printops.sort_options.minus_last = true;
							} else {
								modes[mode_index].po.min_exp = EXP_NONE;
								modes[mode_index].po.negative_exponents = false;
								modes[mode_index].po.sort_options.minus_last = true;
							}
							break;
						}
					}
				} else if(svar == "use_prefixes") {
					if(mode_index == 1) printops.use_unit_prefixes = v;
					else modes[mode_index].po.use_unit_prefixes = v;
				} else if(svar == "fractional_mode") {	//obsolete
					switch(v) {
						case 1: {
							if(mode_index == 1) printops.number_fraction_format = FRACTION_DECIMAL;
							else modes[mode_index].po.number_fraction_format = FRACTION_DECIMAL;
							break;
						}
						case 2: {
							if(mode_index == 1) printops.number_fraction_format = FRACTION_COMBINED;
							else modes[mode_index].po.number_fraction_format = FRACTION_COMBINED;
							break;
						}
						case 3: {
							if(mode_index == 1) printops.number_fraction_format = FRACTION_FRACTIONAL;
							else modes[mode_index].po.number_fraction_format = FRACTION_FRACTIONAL;
							break;
						}
					}
				} else if(svar == "number_fraction_format") {
					if(v >= FRACTION_DECIMAL && v <= FRACTION_COMBINED) {
						if(mode_index == 1) printops.number_fraction_format = (NumberFractionFormat) v;
						else modes[mode_index].po.number_fraction_format = (NumberFractionFormat) v;
					}
				} else if(svar == "number_base") {
					if(mode_index == 1) printops.base = v;
					else modes[mode_index].po.base = v;
				} else if(svar == "number_base_expression") {
					if(mode_index == 1) evalops.parse_options.base = v;	
					else modes[mode_index].eo.parse_options.base = v;	
				} else if(svar == "read_precision") {
					if(v >= DONT_READ_PRECISION && v <= READ_PRECISION_WHEN_DECIMALS) {
						if(mode_index == 1) evalops.parse_options.read_precision = (ReadPrecisionMode) v;
						else modes[mode_index].eo.parse_options.read_precision = (ReadPrecisionMode) v;
					}
				} else if(svar == "assume_denominators_nonzero") {
					if(version_numbers[0] == 0 && (version_numbers[1] < 9 || (version_numbers[1] == 9 && version_numbers[2] == 0))) {
						v = true;
					}
					if(mode_index == 1) evalops.assume_denominators_nonzero = v;
					else modes[mode_index].eo.assume_denominators_nonzero = v;
				} else if(svar == "warn_about_denominators_assumed_nonzero") {
					if(mode_index == 1) evalops.warn_about_denominators_assumed_nonzero = v;
					else modes[mode_index].eo.warn_about_denominators_assumed_nonzero = v;
				} else if(svar == "structuring") {
					if(v >= STRUCTURING_NONE && v <= STRUCTURING_FACTORIZE) {
						if(mode_index == 1) {
							evalops.structuring = (StructuringMode) v;
							printops.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
						} else {
							modes[mode_index].eo.structuring = (StructuringMode) v;
							modes[mode_index].po.allow_factorization = (modes[mode_index].eo.structuring == STRUCTURING_FACTORIZE);
						}
					}
				} else if(svar == "angle_unit") {
					if(version_numbers[0] == 0 && (version_numbers[1] < 7 || (version_numbers[1] == 7 && version_numbers[2] == 0))) {
						v++;
					}
					if(v >= ANGLE_UNIT_NONE && v <= ANGLE_UNIT_GRADIANS) {
						if(mode_index == 1) evalops.parse_options.angle_unit = (AngleUnit) v;
						else modes[mode_index].eo.parse_options.angle_unit = (AngleUnit) v;
					}
				} else if(svar == "functions_enabled") {
					if(mode_index == 1) evalops.parse_options.functions_enabled = v;
					else modes[mode_index].eo.parse_options.functions_enabled = v;
				} else if(svar == "variables_enabled") {
					if(mode_index == 1) evalops.parse_options.variables_enabled = v;
					else modes[mode_index].eo.parse_options.variables_enabled = v;
				} else if(svar == "donot_calculate_variables") {
					if(mode_index == 1) evalops.calculate_variables = !v;
					else modes[mode_index].eo.calculate_variables = !v;
				} else if(svar == "calculate_variables") {
					if(mode_index == 1) evalops.calculate_variables = v;
					else modes[mode_index].eo.calculate_variables = v;
				} else if(svar == "calculate_functions") {
					if(mode_index == 1) evalops.calculate_functions = v;
					else modes[mode_index].eo.calculate_functions = v;
				} else if(svar == "sync_units") {
					if(mode_index == 1) evalops.sync_units = v;
					else modes[mode_index].eo.sync_units = v;
				} else if(svar == "unknownvariables_enabled") {
					if(mode_index == 1) evalops.parse_options.unknowns_enabled = v;
					else modes[mode_index].eo.parse_options.unknowns_enabled = v;
				} else if(svar == "units_enabled") {
					if(mode_index == 1) evalops.parse_options.units_enabled = v;
					else modes[mode_index].eo.parse_options.units_enabled = v;
				} else if(svar == "allow_complex") {
					if(mode_index == 1) evalops.allow_complex = v;
					else modes[mode_index].eo.allow_complex = v;
				} else if(svar == "allow_infinite") {
					if(mode_index == 1) evalops.allow_infinite = v;
					else modes[mode_index].eo.allow_infinite = v;
				} else if(svar == "use_short_units") {
					if(mode_index == 1) printops.abbreviate_names = v;
					else modes[mode_index].po.abbreviate_names = v;
				} else if(svar == "abbreviate_names") {
					if(mode_index == 1) printops.abbreviate_names = v;
					else modes[mode_index].po.abbreviate_names = v;
				} else if(svar == "all_prefixes_enabled") {
					if(mode_index == 1) printops.use_all_prefixes = v;
					else modes[mode_index].po.use_all_prefixes = v;
				} else if(svar == "denominator_prefix_enabled") {
					if(mode_index == 1) printops.use_denominator_prefix = v;
					else modes[mode_index].po.use_denominator_prefix = v;
				} else if(svar == "auto_post_conversion") {
					if(v >= POST_CONVERSION_NONE && v <= POST_CONVERSION_BASE) {
						if(mode_index == 1) evalops.auto_post_conversion = (AutoPostConversion) v;
						else modes[mode_index].eo.auto_post_conversion = (AutoPostConversion) v;
					}
				} else if(svar == "indicate_infinite_series") {
					if(mode_index == 1) printops.indicate_infinite_series = v;
					else modes[mode_index].po.indicate_infinite_series = v;
				} else if(svar == "show_ending_zeroes") {
					if(mode_index == 1) printops.show_ending_zeroes = v;
					else modes[mode_index].po.show_ending_zeroes = v;
				} else if(svar == "round_halfway_to_even") {
					if(mode_index == 1) printops.round_halfway_to_even = v;	
					else modes[mode_index].po.round_halfway_to_even = v;	
				} else if(svar == "always_exact") {		//obsolete
					if(mode_index == 1) evalops.approximation = APPROXIMATION_EXACT;
					else modes[mode_index].eo.approximation = APPROXIMATION_EXACT;
				} else if(svar == "approximation") {
					if(v >= APPROXIMATION_EXACT && v <= APPROXIMATION_APPROXIMATE) {
						if(mode_index == 1) evalops.approximation = (ApproximationMode) v;
						else modes[mode_index].eo.approximation = (ApproximationMode) v;
					}
				} else if(svar == "in_rpn_mode") {
					if(mode_index == 1) rpn_mode = v;
					else modes[mode_index].rpn_mode = v;
				} else if(svar == "rpn_keypad_only") {
					rpn_keypad_only = v;
				} else if(svar == "rpn_syntax") {
					if(mode_index == 1) evalops.parse_options.rpn = v;
					else modes[mode_index].eo.parse_options.rpn = v;
				} else if(svar == "limit_implicit_multiplication") {
					if(mode_index == 1) {
						evalops.parse_options.limit_implicit_multiplication = v;
						printops.limit_implicit_multiplication = v;
					} else {
						modes[mode_index].eo.parse_options.limit_implicit_multiplication = v;
						modes[mode_index].po.limit_implicit_multiplication = v;
					}
				} else if(svar == "default_assumption_type") {
					if(v >= ASSUMPTION_TYPE_NONE && v <= ASSUMPTION_TYPE_INTEGER) {
						if(v == ASSUMPTION_TYPE_NONE && version_numbers[0] == 0 && (version_numbers[1] < 9 || (version_numbers[1] == 9 && version_numbers[2] == 0))) {
							v = ASSUMPTION_TYPE_NONMATRIX;
						}
						if(mode_index == 1) CALCULATOR->defaultAssumptions()->setType((AssumptionType) v);
						else modes[mode_index].at = (AssumptionType) v;
					}
				} else if(svar == "default_assumption_sign") {
					if(v >= ASSUMPTION_SIGN_UNKNOWN && v <= ASSUMPTION_SIGN_NONZERO) {
						if(v == ASSUMPTION_SIGN_NONZERO && version_numbers[0] == 0 && (version_numbers[1] < 9 || (version_numbers[1] == 9 && version_numbers[2] == 0))) {
							v = ASSUMPTION_SIGN_UNKNOWN;
						}
						if(mode_index == 1) CALCULATOR->defaultAssumptions()->setSign((AssumptionSign) v);
						else modes[mode_index].as = (AssumptionSign) v;
					}
				} else if(svar == "spacious") {
					if(mode_index == 1) printops.spacious = v;
					else modes[mode_index].po.spacious = v;
				} else if(svar == "excessive_parenthesis") {
					if(mode_index == 1) printops.excessive_parenthesis = v;
					else modes[mode_index].po.excessive_parenthesis = v;
				} else if(svar == "short_multiplication") {
					if(mode_index == 1) printops.short_multiplication = v;
					else modes[mode_index].po.short_multiplication = v;
				} else if(svar == "hyp_is_on") {
					hyp_is_on = v;
				} else if(svar == "inv_is_on") {
					inv_is_on = v;
				} else if(svar == "use_unicode_signs" && (version_numbers[0] > 0 || version_numbers[1] > 7 || (version_numbers[1] == 7 && version_numbers[2] > 0))) {
					printops.use_unicode_signs = v;	
				} else if(svar == "lower_case_numbers") {
					printops.lower_case_numbers = v;	
				} else if(svar == "lower_case_e") {
					printops.lower_case_e = v;	
				} else if(svar == "base_display") {
					if(v >= BASE_DISPLAY_NONE && v <= BASE_DISPLAY_ALTERNATIVE) printops.base_display = (BaseDisplay) v;
				} else if(svar == "spell_out_logical_operators") {
					printops.spell_out_logical_operators = v;	
				} else if(svar == "dot_as_separator") {
					evalops.parse_options.dot_as_separator = v;
				} else if(svar == "use_custom_result_font") {
					use_custom_result_font = v;
				} else if(svar == "use_custom_expression_font") {
					use_custom_expression_font = v;											
				} else if(svar == "use_custom_status_font") {
					use_custom_status_font = v;											
				} else if(svar == "custom_result_font") {
					custom_result_font = svalue;
				} else if(svar == "custom_expression_font") {
					custom_expression_font = svalue;	
				} else if(svar == "custom_status_font") {
					custom_status_font = svalue;	
				} else if(svar == "status_error_color") {
					status_error_color = svalue;	
				} else if(svar == "status_warning_color") {
					status_warning_color = svalue;	
				} else if(svar == "multiplication_sign") {
					if(svalue == "*") {
						printops.multiplication_sign = MULTIPLICATION_SIGN_ASTERISK;
					} else if(svalue == SIGN_MULTIDOT) {
						printops.multiplication_sign = MULTIPLICATION_SIGN_DOT;
					} else if(svalue == SIGN_MULTIPLICATION) {
						printops.multiplication_sign = MULTIPLICATION_SIGN_X;
					} else if(v >= MULTIPLICATION_SIGN_ASTERISK && v <= MULTIPLICATION_SIGN_X) {
						printops.multiplication_sign = (MultiplicationSign) v;
					}
				} else if(svar == "division_sign") {
					if(v >= DIVISION_SIGN_SLASH && v <= DIVISION_SIGN_DIVISION) printops.division_sign = (DivisionSign) v;
				} else if(svar == "recent_functions") {
					size_t v_i = 0;
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
					size_t v_i = 0;
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
					size_t v_i = 0;
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
				} else if(svar == "plot_legend_placement") {
					if(v >= PLOT_LEGEND_NONE && v <= PLOT_LEGEND_OUTSIDE) default_plot_legend_placement = (PlotLegendPlacement) v;
				} else if(svar == "plot_style") {
					if(v >= PLOT_STYLE_LINES && v <= PLOT_STYLE_DOTS) default_plot_style = (PlotStyle) v;
				} else if(svar == "plot_smoothing") {
					if(v >= PLOT_SMOOTHING_NONE && v <= PLOT_SMOOTHING_SBEZIER) default_plot_smoothing = (PlotSmoothing) v;		
				} else if(svar == "plot_display_grid") {
					default_plot_display_grid = v;
				} else if(svar == "plot_full_border") {
					default_plot_full_border = v;
				} else if(svar == "plot_min") {
					default_plot_min = svalue;	
				} else if(svar == "plot_max") {
					default_plot_max = svalue;
				} else if(svar == "plot_step") {
					default_plot_step = svalue;
				} else if(svar == "plot_sampling_rate") {
					default_plot_sampling_rate = v;	
				} else if(svar == "plot_use_sampling_rate") {
					default_plot_use_sampling_rate = v;		
				} else if(svar == "plot_variable") {
					default_plot_variable = svalue;
				} else if(svar == "plot_rows") {
					default_plot_rows = v;	
				} else if(svar == "plot_type") {
					default_plot_type = v;	
				} else if(svar == "plot_color") {
					default_plot_color = v;
				} else if(svar == "expression_history") {
					expression_history.push_back(svalue);		
				} else if(svar == "history") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_OLD);
				} else if(svar == "history_old") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_OLD);
				} else if(svar == "history_expression") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_EXPRESSION);
				} else if(svar == "history_transformation") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_TRANSFORMATION);
				} else if(svar == "history_result") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_RESULT);
				} else if(svar == "history_result_approximate") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_RESULT_APPROXIMATE);
				} else if(svar == "history_parse") {					
					inhistory.push_back(svalue);
					if(old_history_format) inhistory_type.push_back(QALCULATE_HISTORY_PARSE_WITHEQUALS);
					else inhistory_type.push_back(QALCULATE_HISTORY_PARSE);
				} else if(svar == "history_parse_withequals") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_PARSE_WITHEQUALS);
				} else if(svar == "history_parse_approximate") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_PARSE_APPROXIMATE);
				} else if(svar == "history_register_moved") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_REGISTER_MOVED);
				} else if(svar == "history_warning") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_WARNING);
				} else if(svar == "history_error") {
					inhistory.push_back(svalue);
					inhistory_type.push_back(QALCULATE_HISTORY_ERROR);
				} else if(svar == "history_continued") {
					if(inhistory.size() > 0) {
						inhistory[inhistory.size() - 1] += "\n";
						inhistory[inhistory.size() - 1] += svalue;
					}
				}
			} else if(stmp.length() > 2 && stmp[0] == '[' && stmp[stmp.length() - 1] == ']') {
				stmp = stmp.substr(1, stmp.length() - 2);
				remove_blank_ends(stmp);
				if(stmp == "Mode") {
					mode_index = 1;
				} else if(stmp.length() > 5 && stmp.substr(0, 4) == "Mode") {
					mode_index = save_mode_as(stmp.substr(5, stmp.length() - 5));
				}
			}
		}
	} else {
		first_time = true;
	}
	if(show_keypad) show_history = false;
	set_saved_mode();

}

/*
	save preferences to ~/.qalculate/qalculate-gtk.cfg
	set mode to true to save current calculator mode
*/

void save_preferences(bool mode) {
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
	gint w, h;
	gtk_window_get_size(GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), &w, &h);
	fprintf(file, "width=%i\n", w);
	fprintf(file, "height=%i\n", h);
	fprintf(file, "save_mode_on_exit=%i\n", save_mode_on_exit);
	fprintf(file, "save_definitions_on_exit=%i\n", save_defs_on_exit);
	fprintf(file, "load_global_definitions=%i\n", load_global_defs);
	fprintf(file, "fetch_exchange_rates_at_startup=%i\n", fetch_exchange_rates_at_startup);
	fprintf(file, "wget_args=%s\n", wget_args.c_str());
	fprintf(file, "show_keypad=%i\n", (rpn_mode && show_keypad && gtk_expander_get_expanded(GTK_EXPANDER(expander_stack))) || gtk_expander_get_expanded(GTK_EXPANDER(expander_keypad)));
	fprintf(file, "show_history=%i\n", (rpn_mode && show_history && gtk_expander_get_expanded(GTK_EXPANDER(expander_stack))) || gtk_expander_get_expanded(GTK_EXPANDER(expander_history)));
	fprintf(file, "show_stack=%i\n", rpn_mode ? gtk_expander_get_expanded(GTK_EXPANDER(expander_stack)) : show_stack);
	fprintf(file, "rpn_keypad_only=%i\n", rpn_keypad_only);
	fprintf(file, "display_expression_status=%i\n", display_expression_status);
	fprintf(file, "enable_completion=%i\n", enable_completion);
	fprintf(file, "use_unicode_signs=%i\n", printops.use_unicode_signs);
	fprintf(file, "lower_case_numbers=%i\n", printops.lower_case_numbers);
	fprintf(file, "lower_case_e=%i\n", printops.lower_case_e);
	fprintf(file, "base_display=%i\n", printops.base_display);
	fprintf(file, "spell_out_logical_operators=%i\n", printops.spell_out_logical_operators);
	fprintf(file, "dot_as_separator=%i\n", evalops.parse_options.dot_as_separator);
	fprintf(file, "use_custom_result_font=%i\n", use_custom_result_font);	
	fprintf(file, "use_custom_expression_font=%i\n", use_custom_expression_font);	
	fprintf(file, "use_custom_status_font=%i\n", use_custom_status_font);	
	fprintf(file, "custom_result_font=%s\n", custom_result_font.c_str());	
	fprintf(file, "custom_expression_font=%s\n", custom_expression_font.c_str());
	fprintf(file, "custom_status_font=%s\n", custom_status_font.c_str());
	if(status_error_color != "#FF0000") fprintf(file, "status_error_color=%s\n", status_error_color.c_str());
	if(status_warning_color != "#0000FF") fprintf(file, "status_warning_color=%s\n", status_warning_color.c_str());
	fprintf(file, "multiplication_sign=%i\n", printops.multiplication_sign);
	fprintf(file, "division_sign=%i\n", printops.division_sign);
	for(size_t i = 0; i < expression_history.size(); i++) {
		fprintf(file, "expression_history=%s\n", expression_history[i].c_str()); 
	}	
	int lines = 50;
	bool end_after_result = false;
	bool doend = false;
	for(size_t i = 0; i < inhistory.size() && !doend; i++) {
		switch(inhistory_type[i]) {
			case QALCULATE_HISTORY_EXPRESSION: {
				fprintf(file, "history_expression=");
				break;
			}
			case QALCULATE_HISTORY_TRANSFORMATION: {
				fprintf(file, "history_transformation=");
				break;
			}
			case QALCULATE_HISTORY_RESULT: {
				fprintf(file, "history_result=");
				lines--;
				if(end_after_result) doend = true;
				break;
			}
			case QALCULATE_HISTORY_RESULT_APPROXIMATE: {
				fprintf(file, "history_result_approximate=");
				lines--;
				if(end_after_result) doend = true;
				break;
			}
			case QALCULATE_HISTORY_PARSE: {
				fprintf(file, "history_parse=");
				lines--;
				if(lines < 0) end_after_result = true;
				break;
			}
			case QALCULATE_HISTORY_PARSE_WITHEQUALS: {
				fprintf(file, "history_parse_withequals=");
				lines--;
				if(lines < 0) end_after_result = true;
				break;
			}
			case QALCULATE_HISTORY_PARSE_APPROXIMATE: {
				fprintf(file, "history_parse_approximate=");
				lines--;
				if(lines < 0) end_after_result = true;
				break;
			}
			case QALCULATE_HISTORY_REGISTER_MOVED: {
				fprintf(file, "history_register_moved=");
				break;
			}
			case QALCULATE_HISTORY_WARNING: {
				fprintf(file, "history_warning=");
				lines--;
				break;
			}
			case QALCULATE_HISTORY_ERROR: {
				fprintf(file, "history_error=");
				lines--;
				break;
			}
			case QALCULATE_HISTORY_OLD: {
				fprintf(file, "history_old=");
				lines--;
				if(lines < 0) doend = true;
				break;
			}
		}
		size_t i3 = inhistory[i].find('\n');
		if(i3 == string::npos) {
			fprintf(file, "%s\n", inhistory[i].c_str());
		} else {
			fprintf(file, "%s\n", inhistory[i].substr(0, i3).c_str());
			i3++;
			size_t i2 = inhistory[i].find('\n', i3);
			while(i2 != string::npos) {
				fprintf(file, "history_continued=%s\n", inhistory[i].substr(i3, i2 - i3).c_str());
				lines--;
				i3 = i2 + 1;
				i2 = inhistory[i].find('\n', i3);
			}
			fprintf(file, "history_continued=%s\n", inhistory[i].substr(i3, inhistory[i].length() - i3).c_str());
			lines--;
		}
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
	for(size_t i = 1; i < modes.size(); i++) {
		if(i == 1) {
			fprintf(file, "\n[Mode]\n");
		} else {
			fprintf(file, "\n[Mode %s]\n", modes[i].name.c_str());
		}
		fprintf(file, "min_deci=%i\n", modes[i].po.min_decimals);
		fprintf(file, "use_min_deci=%i\n", modes[i].po.use_min_decimals);
		fprintf(file, "max_deci=%i\n", modes[i].po.max_decimals);
		fprintf(file, "use_max_deci=%i\n", modes[i].po.use_max_decimals);	
		fprintf(file, "precision=%i\n", modes[i].precision);
		fprintf(file, "min_exp=%i\n", modes[i].po.min_exp);
		fprintf(file, "negative_exponents=%i\n", modes[i].po.negative_exponents);
		fprintf(file, "sort_minus_last=%i\n", modes[i].po.sort_options.minus_last);
		fprintf(file, "number_fraction_format=%i\n", modes[i].po.number_fraction_format);	
		fprintf(file, "use_prefixes=%i\n", modes[i].po.use_unit_prefixes);
		fprintf(file, "abbreviate_names=%i\n", modes[i].po.abbreviate_names);
		fprintf(file, "all_prefixes_enabled=%i\n", modes[i].po.use_all_prefixes);
		fprintf(file, "denominator_prefix_enabled=%i\n", modes[i].po.use_denominator_prefix);
		fprintf(file, "place_units_separately=%i\n", modes[i].po.place_units_separately);
		fprintf(file, "auto_post_conversion=%i\n", modes[i].eo.auto_post_conversion);	
		fprintf(file, "number_base=%i\n", modes[i].po.base);
		fprintf(file, "number_base_expression=%i\n", modes[i].eo.parse_options.base);
		fprintf(file, "read_precision=%i\n", modes[i].eo.parse_options.read_precision);
		fprintf(file, "assume_denominators_nonzero=%i\n", modes[i].eo.assume_denominators_nonzero);
		fprintf(file, "warn_about_denominators_assumed_nonzero=%i\n", modes[i].eo.warn_about_denominators_assumed_nonzero);
		fprintf(file, "structuring=%i\n", modes[i].eo.structuring);
		fprintf(file, "angle_unit=%i\n", modes[i].eo.parse_options.angle_unit);
		fprintf(file, "functions_enabled=%i\n", modes[i].eo.parse_options.functions_enabled);
		fprintf(file, "variables_enabled=%i\n", modes[i].eo.parse_options.variables_enabled);
		fprintf(file, "calculate_functions=%i\n", modes[i].eo.calculate_functions);	
		fprintf(file, "calculate_variables=%i\n", modes[i].eo.calculate_variables);	
		fprintf(file, "sync_units=%i\n", modes[i].eo.sync_units);	
		fprintf(file, "unknownvariables_enabled=%i\n", modes[i].eo.parse_options.unknowns_enabled);
		fprintf(file, "units_enabled=%i\n", modes[i].eo.parse_options.units_enabled);
		fprintf(file, "allow_complex=%i\n", modes[i].eo.allow_complex);
		fprintf(file, "allow_infinite=%i\n", modes[i].eo.allow_infinite);
		fprintf(file, "indicate_infinite_series=%i\n", modes[i].po.indicate_infinite_series);
		fprintf(file, "show_ending_zeroes=%i\n", modes[i].po.show_ending_zeroes);
		fprintf(file, "round_halfway_to_even=%i\n", modes[i].po.round_halfway_to_even);
		fprintf(file, "approximation=%i\n", modes[i].eo.approximation);	
		fprintf(file, "in_rpn_mode=%i\n", modes[i].rpn_mode);
		fprintf(file, "rpn_syntax=%i\n", modes[i].eo.parse_options.rpn);
		fprintf(file, "limit_implicit_multiplication=%i\n", modes[i].eo.parse_options.limit_implicit_multiplication);
		fprintf(file, "spacious=%i\n", modes[i].po.spacious);
		fprintf(file, "excessive_parenthesis=%i\n", modes[i].po.excessive_parenthesis);
		fprintf(file, "short_multiplication=%i\n", modes[i].po.short_multiplication);
		fprintf(file, "default_assumption_type=%i\n", modes[i].at);
		fprintf(file, "default_assumption_sign=%i\n", modes[i].as);
	}
	fprintf(file, "\n[Plotting]\n");
	fprintf(file, "plot_legend_placement=%i\n", default_plot_legend_placement);
	fprintf(file, "plot_style=%i\n", default_plot_style);
	fprintf(file, "plot_smoothing=%i\n", default_plot_smoothing);
	fprintf(file, "plot_display_grid=%i\n", default_plot_display_grid);
	fprintf(file, "plot_full_border=%i\n", default_plot_full_border);
	fprintf(file, "plot_min=%s\n", default_plot_min.c_str());
	fprintf(file, "plot_max=%s\n", default_plot_max.c_str());
	fprintf(file, "plot_step=%s\n", default_plot_step.c_str());
	fprintf(file, "plot_sampling_rate=%i\n", default_plot_sampling_rate);
	fprintf(file, "plot_use_sampling_rate=%i\n", default_plot_use_sampling_rate);
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
}

#ifdef __cplusplus
extern "C" {
#endif

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
	
	bool non_number_before = false;
	for(; start >= 0; start--) {
		p = g_utf8_prev_char(p);
		if(!CALCULATOR->utf8_pos_is_valid_in_name(p)) {
			break;
		} else if(is_in(NUMBERS, p[0])) {
			if(non_number_before) break;	
		} else {
			non_number_before = true;
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
gboolean on_completion_match_selected(GtkEntryCompletion*, GtkTreeModel *model, GtkTreeIter *iter, gpointer) {
	set_current_object();
	ExpressionItem *item = NULL;
	const ExpressionName *ename = NULL, *ename_r = NULL;
	gtk_tree_model_get(model, iter, 2, &item, -1);
	if(!item) return TRUE;		
	ename_r = &item->preferredInputName(false, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression);	
	gchar *gstr2 = gtk_editable_get_chars(GTK_EDITABLE(expression), current_object_start, current_object_end);
	for(size_t name_i = 0; name_i <= item->countNames() && !ename; name_i++) {
		if(name_i == 0) {
			ename = ename_r;
		} else {
			ename = &item->getName(name_i);
			if(!ename || ename == ename_r || ename->plural || (ename->unicode && (!printops.use_unicode_signs || !can_display_unicode_string_function(ename->name.c_str(), (void*) expression)))) {
				ename = NULL;
			}
		}
		if(ename) {
			if(strlen(gstr2) <= ename->name.length()) {
				for(size_t i = 0; i < strlen(gstr2); i++) {
					if(ename->name[i] != gstr2[i]) {
						ename = NULL;
						break;
					}
				}
			} else {
				ename = NULL;
			}
		}
	}
	for(size_t name_i = 1; name_i <= item->countNames() && !ename; name_i++) {
		ename = &item->getName(name_i);
		if(!ename || ename == ename_r || (!ename->plural && !(ename->unicode && (!printops.use_unicode_signs || !can_display_unicode_string_function(ename->name.c_str(), (void*) expression))))) {
			ename = NULL;
		}
		if(ename) {
			if(strlen(gstr2) <= ename->name.length()) {
				for(size_t i = 0; i < strlen(gstr2); i++) {
					if(ename->name[i] != gstr2[i]) {
						ename = NULL;
						break;
					}
				}
			} else {
				ename = NULL;
			}
		}
	}
	if(!ename) ename = ename_r;
	g_free(gstr2);
	if(!ename) return TRUE;
	block_completion();
	gtk_editable_delete_text(GTK_EDITABLE(expression), current_object_start, current_object_end);
	gint pos = current_object_start;	
	if(item->type() == TYPE_FUNCTION) {		
		gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), pos, pos + 1);
		if(strlen(gstr) > 0 && gstr[0] == '(') {
			gtk_editable_insert_text(GTK_EDITABLE(expression), ename->name.c_str(), -1, &pos);
			gtk_editable_set_position(GTK_EDITABLE(expression), pos);
		} else {
			string str = ename->name;
			str += "()";
			gtk_editable_insert_text(GTK_EDITABLE(expression), str.c_str(), -1, &pos);
			gtk_editable_set_position(GTK_EDITABLE(expression), pos - 1);
		}
		g_free(gstr);
	} else {
		gtk_editable_insert_text(GTK_EDITABLE(expression), ename->name.c_str(), -1, &pos);
		gtk_editable_set_position(GTK_EDITABLE(expression), pos);
	}
	unblock_completion();
	return TRUE;
}
gboolean completion_match_func(GtkEntryCompletion*, const gchar*, GtkTreeIter *iter, gpointer) {
	set_current_object();
	if(current_object_start < 0) return FALSE;
	gchar *gstr2;
	gboolean b_match = false;
	ExpressionItem *item = NULL;
	const ExpressionName *ename;
	gtk_tree_model_get(GTK_TREE_MODEL(completion_store), iter, 2, &item, -1);
	if(!item) return FALSE;	
	if(!evalops.parse_options.functions_enabled && item->type() == TYPE_FUNCTION) return FALSE;
	if(!evalops.parse_options.variables_enabled && item->type() == TYPE_VARIABLE) return FALSE;
	if(!evalops.parse_options.units_enabled && item->type() == TYPE_UNIT) return FALSE;
	gstr2 = gtk_editable_get_chars(GTK_EDITABLE(expression), current_object_start, current_object_end);
	for(size_t name_i = 1; name_i <= item->countNames() && !b_match; name_i++) {
		ename = &item->getName(name_i);
		if(ename && strlen(gstr2) <= ename->name.length()) {
			b_match = TRUE;
			for(size_t i = 0; i < strlen(gstr2); i++) {
				if(ename->name[i] != gstr2[i]) {
					b_match = FALSE;
					break;
				}				
			}
		}
	}
	g_free(gstr2);
	return b_match;
}


void on_menu_item_quit_activate(GtkMenuItem*, gpointer user_data) {
	on_gcalc_exit(NULL, NULL, user_data);
}

string i2hex(guint value) {
	unsigned int v = (unsigned int) value / 0x101;
	if(value % 0x101 > 128) {
		v++;
	}
	char buffer[3];
	snprintf(buffer, 3, "%.2X", v);
	string stmp = buffer;
	return stmp;
}
/*
	change preferences
*/
void on_colorbutton_status_error_color_color_set(GtkColorButton *w, gpointer) {
	GdkColor c;
	gtk_color_button_get_color(w, &c);
	status_error_color = "#";
	status_error_color += i2hex(c.red);
	status_error_color += i2hex(c.green);
	status_error_color += i2hex(c.blue);
	display_parse_status();
}
void on_colorbutton_status_warning_color_color_set(GtkColorButton *w, gpointer) {
	GdkColor c;
	gtk_color_button_get_color(w, &c);
	status_warning_color = "#";
	status_warning_color += i2hex(c.red);
	status_warning_color += i2hex(c.green);
	status_warning_color += i2hex(c.blue);
	display_parse_status();
}
void on_preferences_entry_wget_args_changed(GtkEditable *editable, gpointer) {
	wget_args = gtk_entry_get_text(GTK_ENTRY(editable));
}
void on_preferences_checkbutton_lower_case_numbers_toggled(GtkToggleButton *w, gpointer) {
	printops.lower_case_numbers = gtk_toggle_button_get_active(w);
	result_display_updated();
}
void on_preferences_checkbutton_lower_case_e_toggled(GtkToggleButton *w, gpointer) {
	printops.lower_case_e = gtk_toggle_button_get_active(w);
	result_display_updated();
}
void on_preferences_checkbutton_alternative_base_prefixes_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) printops.base_display = BASE_DISPLAY_ALTERNATIVE;
	else printops.base_display = BASE_DISPLAY_NORMAL;
	result_display_updated();
}
void on_preferences_checkbutton_spell_out_logical_operators_toggled(GtkToggleButton *w, gpointer) {
	printops.spell_out_logical_operators = gtk_toggle_button_get_active(w);
	result_display_updated();
}
void on_preferences_checkbutton_unicode_signs_toggled(GtkToggleButton *w, gpointer) {
	printops.use_unicode_signs = gtk_toggle_button_get_active(w);
	set_unicode_buttons();
	gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_asterisk"), printops.use_unicode_signs);
	gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_ex"), printops.use_unicode_signs);
	gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot"), printops.use_unicode_signs);
	gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_slash"), printops.use_unicode_signs);
	gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division_slash"), printops.use_unicode_signs);
	gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division"), printops.use_unicode_signs);
	result_display_updated();
}

void on_preferences_checkbutton_save_defs_toggled(GtkToggleButton *w, gpointer) {
	save_defs_on_exit = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_save_mode_toggled(GtkToggleButton *w, gpointer) {
	save_mode_on_exit = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_rpn_keypad_only_toggled(GtkToggleButton *w, gpointer) {
	rpn_keypad_only = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_dot_as_separator_toggled(GtkToggleButton *w, gpointer) {
	evalops.parse_options.dot_as_separator = gtk_toggle_button_get_active(w);
	expression_format_updated(false);
}
void on_preferences_checkbutton_load_defs_toggled(GtkToggleButton *w, gpointer) {
	load_global_defs = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_fetch_exchange_rates_toggled(GtkToggleButton *w, gpointer) {
	fetch_exchange_rates_at_startup = gtk_toggle_button_get_active(w);
}
void on_preferences_checkbutton_display_expression_status_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		display_expression_status = true;
		display_parse_status();
	} else {
		display_expression_status = false;
		set_status_text("");
	}
}
void on_preferences_checkbutton_enable_completion_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		enable_completion = true;
		update_completion();
	} else {
		enable_completion = false;
		update_completion();
	}
}
void on_preferences_checkbutton_custom_result_font_toggled(GtkToggleButton *w, gpointer) {
	use_custom_result_font = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_result_font"), use_custom_result_font);
	if(use_custom_result_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_result_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
		result_font_modified();
	} else {
		gtk_widget_modify_font(resultview, NULL);
		result_font_modified();
	}
}
void on_preferences_checkbutton_custom_expression_font_toggled(GtkToggleButton *w, gpointer) {
	use_custom_expression_font = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_expression_font"), use_custom_expression_font);
	if(use_custom_expression_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_expression_font.c_str());
		gtk_widget_modify_font(expression, font);
		pango_font_description_free(font);
	} else {
		gtk_widget_modify_font(expression, NULL);
	}
}
void on_preferences_checkbutton_custom_status_font_toggled(GtkToggleButton *w, gpointer) {
	use_custom_status_font = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_status_font"), use_custom_status_font);
	if(use_custom_status_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_status_font.c_str());
		gtk_widget_modify_font(statuslabel_l, font);
		gtk_widget_modify_font(statuslabel_r, font);
		pango_font_description_free(font);
	} else {
		gtk_widget_modify_font(statuslabel_l, NULL);
		gtk_widget_modify_font(statuslabel_r, NULL);
	}
}
void on_preferences_radiobutton_dot_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		printops.multiplication_sign = MULTIPLICATION_SIGN_DOT;
		result_display_updated();
	}
}
void on_preferences_radiobutton_ex_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		printops.multiplication_sign = MULTIPLICATION_SIGN_X;
		result_display_updated();
	}
}
void on_preferences_radiobutton_asterisk_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		printops.multiplication_sign = MULTIPLICATION_SIGN_ASTERISK;
		result_display_updated();
	}
}
void on_preferences_radiobutton_slash_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		printops.division_sign = DIVISION_SIGN_SLASH;
		result_display_updated();
	}
}
void on_preferences_radiobutton_division_slash_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		printops.division_sign = DIVISION_SIGN_DIVISION_SLASH;
		result_display_updated();
	}
}
void on_preferences_radiobutton_division_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		printops.division_sign = DIVISION_SIGN_DIVISION;
		result_display_updated();
	}
}
void on_preferences_button_result_font_clicked(GtkButton *w, gpointer) {
	GtkWidget *d = gtk_font_selection_dialog_new(_("Select result font"));
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(d), custom_result_font.c_str());
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		custom_result_font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(d));
		gtk_button_set_label(w, custom_result_font.c_str());
		PangoFontDescription *font = pango_font_description_from_string(custom_result_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
		result_font_modified();
	}
	gtk_widget_destroy(d);
}
void on_preferences_button_expression_font_clicked(GtkButton *w, gpointer) {
	GtkWidget *d = gtk_font_selection_dialog_new(_("Select expression font"));
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(d), custom_expression_font.c_str());
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		custom_expression_font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(d));
		gtk_button_set_label(w, custom_expression_font.c_str());
		PangoFontDescription *font = pango_font_description_from_string(custom_expression_font.c_str());
		gtk_widget_modify_font(expression, font);
		pango_font_description_free(font);
	}
	gtk_widget_destroy(d);
}
void on_preferences_button_status_font_clicked(GtkButton *w, gpointer) {
	GtkWidget *d = gtk_font_selection_dialog_new(_("Select status font"));
	gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(d), custom_status_font.c_str());
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK) {
		custom_status_font = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(d));
		gtk_button_set_label(w, custom_status_font.c_str());
		PangoFontDescription *font = pango_font_description_from_string(custom_status_font.c_str());
		gtk_widget_modify_font(statuslabel_l, font);
		gtk_widget_modify_font(statuslabel_r, font);
		pango_font_description_free(font);
	}
	gtk_widget_destroy(d);
}

/*
	hide unit manager when "Close" clicked
*/
void on_units_button_close_clicked(GtkButton*, gpointer) {
	gtk_widget_hide(glade_xml_get_widget (units_glade, "units_dialog"));
}

/*
	change conversion direction in unit manager on user request
*/
void on_units_toggle_button_from_toggled(GtkToggleButton *togglebutton, gpointer) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (units_glade, "units_toggle_button_to")), FALSE);
		convert_in_wUnits();
	}
}

/*
	convert button clicked
*/
void on_units_button_convert_clicked(GtkButton*, gpointer) {
	convert_in_wUnits();
}

/*
	change conversion direction in unit manager on user request
*/
void on_units_toggle_button_to_toggled(GtkToggleButton *togglebutton, gpointer) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (units_glade, "units_toggle_button_from")), FALSE);
		convert_in_wUnits();
	}
}

/*
	enter in conversion field
*/
void on_units_entry_from_val_activate(GtkEntry*, gpointer) {
	convert_in_wUnits(0);
}
void on_units_entry_to_val_activate(GtkEntry*, gpointer) {
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
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_engineering"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_engineering_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_scientific_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_purely_scientific_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_non_scientific_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_prefixes_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_abbreviate_names"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_abbreviate_names_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_all_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_denominator_prefixes_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_denominator_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_all_prefixes_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal_exact"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_exact_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_combined"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_combined_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_fraction"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_fraction_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_assume_nonzero_denominators"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_assume_nonzero_denominators_activate, NULL);
	if(mstruct && mstruct->containsType(STRUCT_UNIT)) {
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_abbreviate_names"));
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
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_engineering"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_display"));
	} else {
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_abbreviate_names"));
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
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_display_engineering"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_display"));
	}
	gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_factorize"));
	if(mstruct && mstruct->containsUnknowns()) {
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_set_unknowns"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_factorize"));
	} else {
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_set_unknowns"));
	}
	if(mstruct && mstruct->containsType(STRUCT_ADDITION)) {
		if(evalops.structuring == STRUCTURING_FACTORIZE) {
			gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_factorize"));
		} else {
			gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_factorize"));
		}
		if(evalops.structuring == STRUCTURING_SIMPLIFY) {
			gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_simplify"));
		} else {
			gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_simplify"));
		}
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_factorize"));
	} else {
		if(mstruct && mstruct->isNumber() && mstruct->number().isInteger() && !mstruct->number().isZero()) {
			gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_factorize"));
			gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_factorize"));
		} else {
			gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_factorize"));
		}
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_simplify"));
	}
	if(mstruct && mstruct->containsDivision()) {
		gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_assume_nonzero_denominators"));
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_nonzero"));
	} else {
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_assume_nonzero_denominators"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_nonzero"));
	}
	if(mstruct->isVector() && (mstruct->size() != 1 || !(*mstruct)[0].isVector() || (*mstruct)[0].size() > 0)) {
		if(mstruct->isMatrix()) {
			gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_view_matrix"));
			gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_view_vector"));
		} else {
			gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_view_matrix"));
			gtk_widget_show(glade_xml_get_widget(main_glade, "popup_menu_item_view_vector"));
		}
		gtk_widget_show(glade_xml_get_widget(main_glade, "separator_popup_view_matrixvector"));
	} else {
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_view_matrix"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "popup_menu_item_view_vector"));
		gtk_widget_hide(glade_xml_get_widget(main_glade, "separator_popup_view_matrixvector"));
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
		case EXP_BASE_3: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_display_engineering")), TRUE);
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
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_abbreviate_names")), printops.abbreviate_names);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_all_prefixes")), printops.use_all_prefixes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_denominator_prefixes")), printops.use_denominator_prefix);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "popup_menu_item_assume_nonzero_denominators")), evalops.assume_denominators_nonzero);
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
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_engineering"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_engineering_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_scientific_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_purely_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_purely_scientific_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_non_scientific"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_non_scientific_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_display_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_display_prefixes_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_abbreviate_names"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_abbreviate_names_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_abbreviate_names"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_all_prefixes_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_all_prefixes"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_denominator_prefixes_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_decimal_exact"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_decimal_exact_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_combined"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_combined_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_fraction_fraction"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_fraction_fraction_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "popup_menu_item_assume_nonzero_denominators"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_popup_menu_item_assume_nonzero_denominators_activate, NULL);
}
gboolean on_resultview_button_press_event(GtkWidget*, GdkEventButton *event, gpointer) {
	if(event->button == 3) {
		update_resultview_popup();
		gtk_menu_popup(GTK_MENU(glade_xml_get_widget (main_glade, "popup_menu_resultview")), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	}
	return FALSE;
}
gboolean on_resultview_popup_menu(GtkWidget*, gpointer) {
	update_resultview_popup();
	gtk_menu_popup(GTK_MENU(glade_xml_get_widget (main_glade, "popup_menu_resultview")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	return TRUE;
}

gboolean on_units_entry_from_val_focus_out_event(GtkEntry*, GdkEventFocus*, gpointer) {
	convert_in_wUnits(0);
	return FALSE;
}
gboolean on_units_entry_to_val_focus_out_event(GtkEntry*, GdkEventFocus*, gpointer) {
	convert_in_wUnits(1);
	return FALSE;
}

/*
	angle mode radio buttons toggled
*/
void on_radiobutton_radians_toggled(GtkToggleButton *togglebutton, gpointer) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
		set_angle_item();
		expression_format_updated(true);
	}
	focus_keeping_selection();
}
void on_radiobutton_degrees_toggled(GtkToggleButton *togglebutton, gpointer) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_DEGREES;
		set_angle_item();
		expression_format_updated(true);
	}
	focus_keeping_selection();
}
void on_radiobutton_gradians_toggled(GtkToggleButton *togglebutton, gpointer) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_GRADIANS;
		set_angle_item();
		expression_format_updated(true);
	}
	focus_keeping_selection();
}
void on_radiobutton_no_default_angle_unit_toggled(GtkToggleButton *togglebutton, gpointer) {
	if(gtk_toggle_button_get_active(togglebutton)) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_NONE;
		set_angle_item();
		expression_format_updated(true);
	}
	focus_keeping_selection();
}

/*
	enter in expression entry does the same as clicking "Execute" button
*/
void on_expression_activate(GtkEntry*, gpointer) {
	execute_expression();
}


/*
	"Execute" clicked
*/
void on_button_execute_clicked(GtkButton*, gpointer) {
	execute_expression();
}
/*
	save preferences, mode and definitions and then quit
*/
gboolean on_gcalc_exit(GtkWidget*, GdkEvent*, gpointer) {
	stop_timeouts = true;
	exit_in_progress = true;
	if(plot_glade && GTK_WIDGET_VISIBLE(glade_xml_get_widget (plot_glade, "plot_dialog"))) {
		gtk_widget_hide(glade_xml_get_widget (plot_glade, "plot_dialog"));
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

void save_accels() {
	gchar *gstr = g_build_filename(g_get_home_dir(), ".qalculate", NULL);
	mkdir(gstr, S_IRWXU);
	g_free(gstr);
	gstr = g_build_filename(g_get_home_dir(), ".qalculate", "accelmap", NULL);
	gtk_accel_map_save(gstr);
	g_free(gstr);
}


/*
	DEL button clicked -- delete in expression entry
*/
void on_button_del_clicked(GtkButton*, gpointer) {
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
void on_button_ac_clicked(GtkButton*, gpointer) {
	gtk_editable_delete_text(GTK_EDITABLE(expression), 0, -1);
	gtk_widget_grab_focus(expression);
}

/*
	HYP button toggled -- enable/disable hyperbolic functions
*/
void on_button_hyp_toggled(GtkToggleButton *w, gpointer) {
	hyp_is_on = gtk_toggle_button_get_active(w);
	focus_keeping_selection();
}

/*
	INV button toggled -- enable/disable inverse functions
*/

void on_button_inv_toggled(GtkToggleButton *w, gpointer) {
	inv_is_on = gtk_toggle_button_get_active(w);
	focus_keeping_selection();
}

/*
	fraction button toggled -- enable/disable fractional display
*/
void on_button_fraction_toggled(GtkToggleButton *w, gpointer) {
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
	result_format_updated();
	focus_keeping_selection();
}

/*
	exact button toggled
*/
void on_button_exact_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		evalops.approximation = APPROXIMATION_EXACT;
		GtkWidget *w_exact = glade_xml_get_widget (main_glade, "menu_item_always_exact");
		g_signal_handlers_block_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_always_exact_activate, NULL);		
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w_exact), TRUE);		
		g_signal_handlers_unblock_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_always_exact_activate, NULL);		
	} else {
		evalops.approximation = APPROXIMATION_TRY_EXACT;
		GtkWidget *w_exact = glade_xml_get_widget (main_glade, "menu_item_try_exact");
		g_signal_handlers_block_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_try_exact_activate, NULL);		
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w_exact), TRUE);		
		g_signal_handlers_unblock_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_try_exact_activate, NULL);		
	}
	expression_calculation_updated();
	focus_keeping_selection();
}

/*
	Tan/Sin/Cos button clicked -- insert corresponding function
*/
void on_button_tan_clicked(GtkButton*, gpointer) {
	if(hyp_is_on) {
		if(inv_is_on) {
			insertButtonFunction(CALCULATOR->f_atanh);
			inv_is_on = false;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_inv")), FALSE);
		} else {
			insertButtonFunction(CALCULATOR->f_tanh);
		}
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_hyp")), FALSE);
	} else if(inv_is_on) {
		insertButtonFunction(CALCULATOR->f_atan);
		inv_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_inv")), FALSE);
	} else {
		insertButtonFunction(CALCULATOR->f_tan);
	}
}
void on_button_sine_clicked(GtkButton*, gpointer) {
	if(hyp_is_on) {
		if(inv_is_on) {
			insertButtonFunction(CALCULATOR->f_asinh);
			inv_is_on = false;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_inv")), FALSE);
		} else {
			insertButtonFunction(CALCULATOR->f_sinh);
		}
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_hyp")), FALSE);
	} else if(inv_is_on) {
		insertButtonFunction(CALCULATOR->f_asin);
		inv_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_inv")), FALSE);
	} else {
		insertButtonFunction(CALCULATOR->f_sin);
	}
}
void on_button_cosine_clicked(GtkButton*, gpointer) {
	if(hyp_is_on) {
		if(inv_is_on) {
			insertButtonFunction(CALCULATOR->f_acosh);
			inv_is_on = false;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_inv")), FALSE);
		} else {
			insertButtonFunction(CALCULATOR->f_cosh);
		}
		hyp_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_hyp")), FALSE);
	} else if(inv_is_on) {
		insertButtonFunction(CALCULATOR->f_acos);
		inv_is_on = false;
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_inv")), FALSE);
	} else {
		insertButtonFunction(CALCULATOR->f_cos);
	}
}

void on_button_mod_clicked(GtkButton*, gpointer) {
	insertButtonFunction(CALCULATOR->f_mod);
}

void on_button_factorial_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		insertButtonFunction(CALCULATOR->f_factorial);
		return;
	}
	wrap_expression_selection();
	insert_text("!");
}

/*
	STO button clicked -- store result
*/
void on_button_store_clicked(GtkButton*, gpointer) {
	add_as_variable();
}

/*
	clear the displayed result when expression changes
*/
void on_expression_changed(GtkEditable *w, gpointer) {
	if(completion_blocked) {
		completion_blocked = false;
	} else {
		gtk_entry_completion_set_minimum_key_length(completion, 1);
	}
	const gchar *gstr = gtk_entry_get_text(GTK_ENTRY(w));
	for(size_t i = 0; i < strlen(gstr); i++) {
		if(gstr[i] == '\n') {
			string str = gstr;
			gsub("\n", " ", str);
			gtk_entry_set_text(GTK_ENTRY(w), str.c_str());
			break;
		}
	}
	expression_has_changed = true;
	expression_has_changed2 = true;
	current_object_has_changed = true;
	if(result_text.empty()) return;
	if(!dont_change_index) expression_history_index = -1;
	if(!rpn_mode) clearresult();
}

/*
	Button clicked -- insert text (1,2,3,... +,-,...)
*/
void on_button_zero_clicked(GtkButton*, gpointer) {
	insert_text("0");
}
void on_button_one_clicked(GtkButton*, gpointer) {
	insert_text("1");
}
void on_button_two_clicked(GtkButton*, gpointer) {
	insert_text("2");
}
void on_button_three_clicked(GtkButton*, gpointer) {
	insert_text("3");
}
void on_button_four_clicked(GtkButton*, gpointer) {
	insert_text("4");
}
void on_button_five_clicked(GtkButton*, gpointer) {
	insert_text("5");
}
void on_button_six_clicked(GtkButton*, gpointer) {
	insert_text("6");
}
void on_button_seven_clicked(GtkButton*, gpointer) {
	insert_text("7");
}
void on_button_eight_clicked(GtkButton*, gpointer) {
	insert_text("8");
}
void on_button_nine_clicked(GtkButton*, gpointer) {
	insert_text("9");
}
void on_button_dot_clicked(GtkButton*, gpointer) {
	insert_text(CALCULATOR->getDecimalPoint().c_str());
}
void on_button_brace_open_clicked(GtkButton*, gpointer) {
	insert_text("(");
}
void on_button_brace_close_clicked(GtkButton*, gpointer) {
	insert_text(")");
}
void on_button_times_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		calculateRPN(OPERATION_MULTIPLY);
		return;
	}
	if(!evalops.parse_options.rpn) {
		wrap_expression_selection();
	}
	if(printops.use_unicode_signs && printops.multiplication_sign == MULTIPLICATION_SIGN_DOT && can_display_unicode_string_function(SIGN_MULTIDOT, (void*) expression)) {
		insert_text(SIGN_MULTIDOT);
	} else if(printops.use_unicode_signs && printops.multiplication_sign == MULTIPLICATION_SIGN_DOT && can_display_unicode_string_function(SIGN_SMALLCIRCLE, (void*) expression)) {
		insert_text(SIGN_SMALLCIRCLE);
	} else if(printops.use_unicode_signs && printops.multiplication_sign == MULTIPLICATION_SIGN_X && can_display_unicode_string_function(SIGN_MULTIPLICATION, (void*) expression)) {
		insert_text(SIGN_MULTIPLICATION);
	} else {
		insert_text("*");
	}
}
void on_button_add_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		calculateRPN(OPERATION_ADD);
		return;
	}
	if(!evalops.parse_options.rpn) {
		wrap_expression_selection();
	}
//	if(printops.use_unicode_signs) insert_text(SIGN_PLUS);
//	else 
	insert_text("+");
}
void on_button_sub_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		calculateRPN(OPERATION_SUBTRACT);
		return;
	}
	if(!evalops.parse_options.rpn) {
		wrap_expression_selection();
	}
	if(printops.use_unicode_signs && can_display_unicode_string_function(SIGN_MINUS, (void*) expression)) insert_text(SIGN_MINUS);
	else insert_text("-");
}
void on_button_divide_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		calculateRPN(OPERATION_DIVIDE);
		return;
	}
	if(!evalops.parse_options.rpn) {
		wrap_expression_selection();
	}
	if(printops.use_unicode_signs && printops.division_sign == DIVISION_SIGN_DIVISION && can_display_unicode_string_function(SIGN_DIVISION, (void*) expression)) {
		insert_text(SIGN_DIVISION);
	} else {
		insert_text("/");
	}
}
void on_button_ans_clicked(GtkButton*, gpointer) {
	insert_text(vans[0]->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression).name.c_str());
}
void on_button_exp_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		calculateRPN(OPERATION_EXP10);
		return;
	}
	if(printops.lower_case_e) insert_text("e");
	else insert_text("E");	
}
void on_button_xy_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		calculateRPN(OPERATION_RAISE);
		return;
	}
	if(!evalops.parse_options.rpn) {
		wrap_expression_selection();
	}
	insert_text("^");
}
void on_button_square_clicked(GtkButton*, gpointer) {
	if(rpn_mode) {
		calculateRPN(CALCULATOR->f_sq);
		return;
	}
	if(evalops.parse_options.rpn) {
		insertButtonFunction(CALCULATOR->f_sq);
	} else {
		wrap_expression_selection();
		insert_text("^2");
	}
}

/*
	Button clicked -- insert corresponding function
*/
void on_button_sqrt_clicked(GtkButton*, gpointer) {
	insertButtonFunction(CALCULATOR->f_sqrt);
}
void on_button_log_clicked(GtkButton*, gpointer) {
	MathFunction *f = CALCULATOR->getActiveFunction("log10");
	if(f) {
		insertButtonFunction(f);
	} else {
		show_message(_("log10 function not found."), glade_xml_get_widget (main_glade, "main_window"));
	}
}
void on_button_ln_clicked(GtkButton*, gpointer) {
	insertButtonFunction(CALCULATOR->f_ln);
}

void on_menu_item_manage_variables_activate(GtkMenuItem*, gpointer) {
	manage_variables();
}
void on_menu_item_manage_functions_activate(GtkMenuItem*, gpointer) {
	manage_functions();
}
void on_menu_item_manage_units_activate(GtkMenuItem*, gpointer) {
	manage_units();
}

void on_menu_item_datasets_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = get_datasets_dialog();
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));	
}

void on_menu_item_import_csv_file_activate(GtkMenuItem*, gpointer) {
	import_csv_file(glade_xml_get_widget (main_glade, "main_window"));
}

void on_menu_item_export_csv_file_activate(GtkMenuItem*, gpointer) {
	export_csv_file(NULL, glade_xml_get_widget (main_glade, "main_window"));
}


void on_menu_item_convert_to_unit_expression_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = get_unit_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	if(GTK_WIDGET_VISIBLE(dialog)) {
		gtk_window_present(GTK_WINDOW(dialog));
	} else {
		gtk_widget_show(dialog);
	}
}
void on_menu_item_convert_to_best_unit_activate(GtkMenuItem*, gpointer) {
	do_timeout = false;
	mstruct->set(CALCULATOR->convertToBestUnit(*mstruct, evalops));
	result_action_executed();
	do_timeout = true;
}
void on_menu_item_convert_to_base_units_activate(GtkMenuItem*, gpointer) {
	do_timeout = false;
	mstruct->set(CALCULATOR->convertToBaseUnits(*mstruct, evalops));
	result_action_executed();
	do_timeout = true;
}

void on_menu_item_insert_matrix_activate(GtkMenuItem*, gpointer) {
	string str;
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {
		gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
		str = gstr;
		remove_blank_ends(str);
		g_free(gstr);
	}
	if(!str.empty()) {
		MathStructure mstruct_sel;
		CALCULATOR->beginTemporaryStopMessages();
		CALCULATOR->parse(&mstruct_sel, CALCULATOR->unlocalizeExpression(str, evalops.parse_options), evalops.parse_options);
		CALCULATOR->endTemporaryStopMessages();
		if(mstruct_sel.isMatrix() && mstruct_sel[0].size() > 0) {
			insert_matrix(&mstruct_sel, glade_xml_get_widget (main_glade, "main_window"), false);
			return;
		}
	}
	insert_matrix(NULL, glade_xml_get_widget (main_glade, "main_window"), false);
}
void on_menu_item_insert_vector_activate(GtkMenuItem*, gpointer) {
	string str;
	gint start = 0, end = 0;
	if(gtk_editable_get_selection_bounds(GTK_EDITABLE(expression), &start, &end)) {
		gchar *gstr = gtk_editable_get_chars(GTK_EDITABLE(expression), start, end);
		str = gstr;
		remove_blank_ends(str);
		g_free(gstr);
	}
	if(!str.empty()) {
		MathStructure mstruct_sel;
		CALCULATOR->beginTemporaryStopMessages();
		CALCULATOR->parse(&mstruct_sel, CALCULATOR->unlocalizeExpression(str, evalops.parse_options), evalops.parse_options);
		CALCULATOR->endTemporaryStopMessages();
		if(mstruct_sel.isVector() && !mstruct_sel.isMatrix()) {
			insert_matrix(&mstruct_sel, glade_xml_get_widget (main_glade, "main_window"), true);
			return;
		}
	}
	insert_matrix(NULL, glade_xml_get_widget (main_glade, "main_window"), true);
}

void update_assumptions_items() {
	if(!block_expression_execution) {
		block_expression_execution = true;
		set_assumptions_items(CALCULATOR->defaultAssumptions()->type(), CALCULATOR->defaultAssumptions()->sign());
		block_expression_execution = false;
	}
}

void on_menu_item_assumptions_integer_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setType(ASSUMPTION_TYPE_INTEGER);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_rational_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setType(ASSUMPTION_TYPE_RATIONAL);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_real_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setType(ASSUMPTION_TYPE_REAL);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_complex_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setType(ASSUMPTION_TYPE_COMPLEX);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_number_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setType(ASSUMPTION_TYPE_NUMBER);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_none_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setType(ASSUMPTION_TYPE_NONE);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_nonmatrix_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setType(ASSUMPTION_TYPE_NONMATRIX);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_nonzero_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONZERO);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_positive_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_POSITIVE);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_nonnegative_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONNEGATIVE);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_negative_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NEGATIVE);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_nonpositive_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONPOSITIVE);
	update_assumptions_items();
	expression_calculation_updated();
}
void on_menu_item_assumptions_unknown_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_UNKNOWN);
	update_assumptions_items();
	expression_calculation_updated();
}

void on_menu_item_enable_variables_activate(GtkMenuItem *w, gpointer) {
	evalops.parse_options.variables_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_format_updated(evalops.parse_options.variables_enabled);
}
void on_menu_item_enable_functions_activate(GtkMenuItem *w, gpointer) {
	evalops.parse_options.functions_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_format_updated(evalops.parse_options.functions_enabled);
}
void on_menu_item_enable_units_activate(GtkMenuItem *w, gpointer) {
	evalops.parse_options.units_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_format_updated(evalops.parse_options.units_enabled);
}
void on_menu_item_enable_unknown_variables_activate(GtkMenuItem *w, gpointer) {
	evalops.parse_options.unknowns_enabled = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_format_updated(evalops.parse_options.unknowns_enabled);
}
void on_menu_item_calculate_variables_activate(GtkMenuItem *w, gpointer) {
	evalops.calculate_variables = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_calculation_updated();
}
void on_menu_item_allow_complex_activate(GtkMenuItem *w, gpointer) {
	evalops.allow_complex = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_calculation_updated();
}
void on_menu_item_allow_infinite_activate(GtkMenuItem *w, gpointer) {
	evalops.allow_infinite = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_calculation_updated();
}
void on_menu_item_assume_nonzero_denominators_activate(GtkMenuItem *w, gpointer) {
	evalops.assume_denominators_nonzero = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_calculation_updated();
}
void on_menu_item_warn_about_denominators_assumed_nonzero_activate(GtkMenuItem *w, gpointer) {
	evalops.warn_about_denominators_assumed_nonzero = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	if(evalops.warn_about_denominators_assumed_nonzero) expression_calculation_updated();
}
void on_menu_item_algebraic_mode_simplify_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.structuring = STRUCTURING_SIMPLIFY;
	printops.allow_factorization = false;
	expression_calculation_updated();
}
void on_menu_item_algebraic_mode_factorize_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.structuring = STRUCTURING_FACTORIZE;
	printops.allow_factorization = true;
	expression_calculation_updated();
}
void on_menu_item_algebraic_mode_none_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.structuring = STRUCTURING_NONE;
	printops.allow_factorization = false;
	expression_calculation_updated();
}
void on_menu_item_read_precision_activate(GtkMenuItem *w, gpointer) {
	 if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) evalops.parse_options.read_precision = READ_PRECISION_WHEN_DECIMALS;
	 else evalops.parse_options.read_precision = DONT_READ_PRECISION;
	 expression_format_updated(true);
}
void on_menu_item_new_unknown_activate(GtkMenuItem*, gpointer) {
	edit_unknown(_("My Variables"), NULL, glade_xml_get_widget (main_glade, "main_window"));
}
void on_menu_item_new_variable_activate(GtkMenuItem*, gpointer) {
	edit_variable(_("My Variables"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"));
}
void on_menu_item_new_matrix_activate(GtkMenuItem*, gpointer) {
	edit_matrix(_("Matrices"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), FALSE);
}
void on_menu_item_new_vector_activate(GtkMenuItem*, gpointer) {
	edit_matrix(_("Vectors"), NULL, NULL, glade_xml_get_widget (main_glade, "main_window"), TRUE);
}
void on_menu_item_new_function_activate(GtkMenuItem*, gpointer) {
	edit_function("", NULL, glade_xml_get_widget (main_glade, "main_window"));
}
void on_menu_item_new_dataset_activate(GtkMenuItem*, gpointer) {
	edit_dataset(NULL, glade_xml_get_widget (main_glade, "main_window"));
}
void on_menu_item_new_unit_activate(GtkMenuItem*, gpointer) {
	edit_unit("", NULL, glade_xml_get_widget (main_glade, "main_window"));
}

void on_menu_item_rpn_mode_activate(GtkMenuItem *w, gpointer) {
	set_rpn_mode(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_menu_item_rpn_syntax_activate(GtkMenuItem *w, gpointer) {
	evalops.parse_options.rpn = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_format_updated(false);
}
void on_menu_item_limit_implicit_multiplication_activate(GtkMenuItem *w, gpointer) {
	evalops.parse_options.limit_implicit_multiplication = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	printops.limit_implicit_multiplication = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	expression_format_updated(true);
	result_format_updated();
}
void fetch_exchange_rates(int timeout) {
#if GTK_MINOR == 6 && GTK_MICRO < 2
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, _("Fetching exchange rates."));
#else	
	GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, _("Fetching exchange rates."));
#endif	
	gtk_widget_show_now(dialog);
	while(gtk_events_pending()) gtk_main_iteration();
	CALCULATOR->fetchExchangeRates(timeout, wget_args);
	gtk_widget_destroy(dialog);
}
void on_menu_item_fetch_exchange_rates_activate(GtkMenuItem*, gpointer) {
	fetch_exchange_rates(15);
	CALCULATOR->loadExchangeRates();
	expression_calculation_updated();
}
void on_menu_item_save_defs_activate(GtkMenuItem*, gpointer) {
	save_defs();
}
void on_menu_item_save_mode_activate(GtkMenuItem*, gpointer) {
	save_mode();
}
void on_menu_item_edit_prefs_activate(GtkMenuItem*, gpointer) {
	edit_preferences();
}
void on_menu_item_degrees_activate(GtkMenuItem *w, gpointer) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_DEGREES;
		set_angle_button();
		expression_format_updated(true);
	}
}
void on_menu_item_radians_activate(GtkMenuItem *w, gpointer) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
		set_angle_button();
		expression_format_updated(true);
	}
}
void on_menu_item_gradians_activate(GtkMenuItem *w, gpointer) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_GRADIANS;
		set_angle_button();
		expression_format_updated(true);
	}
}
void on_menu_item_no_default_angle_unit_activate(GtkMenuItem *w, gpointer) {
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) {
		evalops.parse_options.angle_unit = ANGLE_UNIT_NONE;
		set_angle_button();
		expression_format_updated(true);
	}
}

void set_output_base_from_dialog(int base) {
	bool b = (printops.base == base);
	printops.base = base;
	switch(printops.base) {
		case BASE_BINARY: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_binary_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_binary")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_binary_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 0);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
		case BASE_OCTAL: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_octal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_octal")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_octal_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 1);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
		case BASE_DECIMAL: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_decimal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_decimal")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_decimal_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 2);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
		case BASE_HEXADECIMAL: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_hexadecimal")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 3);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
		case BASE_SEXAGESIMAL: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sexagesimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_sexagesimal")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sexagesimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 4);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
		case BASE_TIME: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_time_format"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_time_format")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_time_format"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 5);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
		case BASE_ROMAN_NUMERALS: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_roman"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_roman")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_roman"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_hexadecimal_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 6);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
		default: {
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_custom_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_custom_base_activate, NULL);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_custom_base")), TRUE);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_custom_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_custom_base_activate, NULL);
			g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 7);
			g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
			break;
		}
	}
	if(!b) result_format_updated();
}
void output_base_updated_from_menu() {
	if(setbase_glade) {
		switch(printops.base) {
			case BASE_BINARY: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_binary_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_binary")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_binary"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_binary_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_OCTAL: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_octal_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_octal")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_octal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_octal_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_DECIMAL: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_decimal_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_decimal")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_decimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_decimal_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_HEXADECIMAL: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_hexadecimal_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_hexadecimal")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_hexadecimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_hexadecimal_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_SEXAGESIMAL: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_sexagesimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_sexagesimal_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_sexagesimal")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_sexagesimal"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_sexagesimal_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_TIME: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_time"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_time_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_time")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_time"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_time_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_ROMAN_NUMERALS: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_roman"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_roman_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_roman")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_roman"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_roman_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			default: {
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_other"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_other_toggled, NULL);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_other")), TRUE);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_radiobutton_output_other"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_radiobutton_output_other_toggled, NULL);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), TRUE);
				g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_spinbutton_output_other"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_spinbutton_output_other_value_changed, NULL);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other")), printops.base);
				g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(setbase_glade, "set_base_spinbutton_output_other"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_set_base_spinbutton_output_other_value_changed, NULL);
			}
		}
	}
}


void on_menu_item_binary_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_BINARY;
	result_format_updated();
	output_base_updated_from_menu();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 0);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
}
void on_menu_item_octal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_OCTAL;
	result_format_updated();
	output_base_updated_from_menu();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 1);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
}
void on_menu_item_decimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_DECIMAL;
	result_format_updated();
	output_base_updated_from_menu();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 2);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
}
void on_menu_item_hexadecimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_HEXADECIMAL;
	result_format_updated();
	output_base_updated_from_menu();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 3);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
}
void on_menu_item_custom_base_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	GtkWidget *dialog = get_set_base_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_other")), TRUE);
	gtk_widget_grab_focus(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"));
}
void on_menu_item_roman_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_ROMAN_NUMERALS;
	output_base_updated_from_menu();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 6);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	result_format_updated();
}
void on_menu_item_sexagesimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_SEXAGESIMAL;
	output_base_updated_from_menu();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 4);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	result_format_updated();
}
void on_menu_item_time_format_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.base = BASE_TIME;
	output_base_updated_from_menu();
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 5);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_base"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_base_changed, NULL);
	result_format_updated();
}
void on_set_base_spinbutton_output_other_value_changed(GtkSpinButton *w, gpointer) {
	set_output_base_from_dialog(gtk_spin_button_get_value_as_int(w));
}
void on_set_base_radiobutton_output_binary_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(BASE_BINARY);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
}
void on_set_base_radiobutton_output_octal_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(BASE_OCTAL);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
}
void on_set_base_radiobutton_output_decimal_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(BASE_DECIMAL);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
}
void on_set_base_radiobutton_output_hexadecimal_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(BASE_HEXADECIMAL);	
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
}
void on_set_base_radiobutton_output_sexagesimal_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(BASE_SEXAGESIMAL);	
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
}
void on_set_base_radiobutton_output_time_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(BASE_TIME);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
}
void on_set_base_radiobutton_output_roman_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(BASE_ROMAN_NUMERALS);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
}
void on_set_base_radiobutton_output_other_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	set_output_base_from_dialog(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"))));
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), TRUE);
}
void on_menu_item_set_base_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = get_set_base_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
}
void on_set_base_radiobutton_input_binary_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_BINARY;
	expression_format_updated(true);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
}
void on_set_base_radiobutton_input_octal_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_OCTAL;
	expression_format_updated(true);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
}
void on_set_base_radiobutton_input_decimal_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_DECIMAL;
	expression_format_updated(true);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
}
void on_set_base_radiobutton_input_hexadecimal_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_HEXADECIMAL;
	expression_format_updated(true);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
}
void on_set_base_radiobutton_input_other_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other")));
	expression_format_updated(true);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), TRUE);
}
void on_set_base_spinbutton_input_other_value_changed(GtkSpinButton *w, gpointer) {
	evalops.parse_options.base = gtk_spin_button_get_value_as_int(w);
	expression_format_updated(true);
}
void on_set_base_radiobutton_input_roman_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
		return;
	evalops.parse_options.base = BASE_ROMAN_NUMERALS;
	expression_format_updated(true);
	gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
}
void on_menu_item_abbreviate_names_activate(GtkMenuItem *w, gpointer) {
	printops.abbreviate_names = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_all_prefixes_activate(GtkMenuItem *w, gpointer) {
	printops.use_all_prefixes = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_denominator_prefixes_activate(GtkMenuItem *w, gpointer) {
	printops.use_denominator_prefix = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_place_units_separately_activate(GtkMenuItem *w, gpointer) {
	printops.place_units_separately = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_post_conversion_none_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	evalops.auto_post_conversion = POST_CONVERSION_NONE;
	expression_calculation_updated();
}
void on_menu_item_post_conversion_base_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	evalops.auto_post_conversion = POST_CONVERSION_BASE;
	expression_calculation_updated();
}
void on_menu_item_post_conversion_best_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	evalops.auto_post_conversion = POST_CONVERSION_BEST;
	expression_calculation_updated();
}
void on_menu_item_factorize_activate(GtkMenuItem*, gpointer) {
	executeCommand(COMMAND_FACTORIZE);
}
void on_menu_item_simplify_activate(GtkMenuItem*, gpointer) {
	executeCommand(COMMAND_SIMPLIFY);
}
void on_menu_item_convert_number_bases_activate(GtkMenuItem*, gpointer) {
	changing_in_nbases_dialog = false;
	GtkWidget *dialog = get_nbases_dialog();
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
}
void on_menu_item_periodic_table_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = get_periodic_dialog();
	gtk_widget_show(dialog);
	gtk_window_present(GTK_WINDOW(dialog));
}
void on_menu_item_plot_functions_activate(GtkMenuItem*, gpointer) {
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
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_step")), default_plot_step.c_str());
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")), default_plot_variable.c_str());
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_steps")), default_plot_use_sampling_rate);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_step")), !default_plot_use_sampling_rate);
		switch(default_plot_type) {
			case 1: {gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_vector")), TRUE); break;}
			case 2: {gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_paired")), TRUE); break;}
			default: {gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_function")), TRUE); break;}
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows"), default_plot_type == 1 || default_plot_type == 2);	
		gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_box_variable"), default_plot_type != 1 && default_plot_type != 2);	
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
		gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (plot_glade, "plot_notebook")), 2);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (plot_glade, "plot_notebook")), 1);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(glade_xml_get_widget (plot_glade, "plot_notebook")), 0);
	} else {
		gtk_window_present(GTK_WINDOW(dialog));
	}
}
void on_plot_dialog_hide(GtkWidget*, gpointer) {
	default_plot_display_grid = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_grid")));
	default_plot_full_border = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_full_border")));
	default_plot_rows = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")));
	default_plot_color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_color")));
	default_plot_min = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_min")));
	default_plot_max = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_max")));
	default_plot_step = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_step")));
	default_plot_variable = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")));
	default_plot_use_sampling_rate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_steps")));
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
	GtkTreeIter iter;
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
	while(b) {
		MathStructure *y_vector, *x_vector;
		gtk_tree_model_get(GTK_TREE_MODEL(tPlotFunctions_store), &iter, 7, &x_vector, 8, &y_vector, -1);
		if(y_vector) delete y_vector;
		if(x_vector) delete x_vector;
		b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
	}
	gtk_widget_set_sensitive(glade_xml_get_widget(plot_glade, "plot_button_save"), false);
	CALCULATOR->closeGnuplot();
}
void on_popup_menu_item_display_normal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	printops.negative_exponents = false;
	printops.sort_options.minus_last = true;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_normal")), TRUE);

	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_negative_exponents")), printops.negative_exponents);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sort_minus_last")), printops.sort_options.minus_last);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
}
void on_popup_menu_item_assume_nonzero_denominators_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_assume_nonzero_denominators")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_display_engineering_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_engineering")), TRUE);
}
void on_popup_menu_item_display_scientific_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	printops.negative_exponents = true;
	printops.sort_options.minus_last = false;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_scientific")), TRUE);

	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_negative_exponents")), printops.negative_exponents);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sort_minus_last")), printops.sort_options.minus_last);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
}
void on_popup_menu_item_display_purely_scientific_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	printops.negative_exponents = true;
	printops.sort_options.minus_last = false;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_purely_scientific")), TRUE);

	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_negative_exponents")), printops.negative_exponents);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sort_minus_last")), printops.sort_options.minus_last);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
}
void on_popup_menu_item_display_non_scientific_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	printops.negative_exponents = false;
	printops.sort_options.minus_last = true;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_non_scientific")), TRUE);

	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_negative_exponents")), printops.negative_exponents);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sort_minus_last")), printops.sort_options.minus_last);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_negative_exponents"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_negative_exponents_activate, NULL);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "menu_item_sort_minus_last"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_menu_item_sort_minus_last_activate, NULL);
}
void on_popup_menu_item_display_prefixes_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_display_prefixes")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_fraction_decimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_decimal")), TRUE);
}
void on_popup_menu_item_fraction_decimal_exact_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_decimal_exact")), TRUE);
}
void on_popup_menu_item_fraction_combined_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_combined")), TRUE);
}
void on_popup_menu_item_fraction_fraction_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_fraction_fraction")), TRUE);
}
void on_popup_menu_item_binary_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_binary")), TRUE);
}
void on_popup_menu_item_roman_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_roman")), TRUE);
}
void on_popup_menu_item_sexagesimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_sexagesimal")), TRUE);
}
void on_popup_menu_item_time_format_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_time_format")), TRUE);
}
void on_popup_menu_item_octal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_octal")), TRUE);
}
void on_popup_menu_item_decimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_decimal")), TRUE);
}
void on_popup_menu_item_hexadecimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_hexadecimal")), TRUE);
}
void on_popup_menu_item_custom_base_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_custom_base")), TRUE);
}
void on_popup_menu_item_abbreviate_names_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_abbreviate_names")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_all_prefixes_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_all_prefixes")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_denominator_prefixes_activate(GtkMenuItem *w, gpointer) {
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget(main_glade, "menu_item_denominator_prefixes")), gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)));
}
void on_popup_menu_item_view_matrix_activate(GtkMenuItem*, gpointer) {
	insert_matrix(mstruct, glade_xml_get_widget (main_glade, "main_window"), false, false, true);
}
void on_popup_menu_item_view_vector_activate(GtkMenuItem*, gpointer) {
	insert_matrix(mstruct, glade_xml_get_widget (main_glade, "main_window"), true, false, true);
}

void on_menu_item_display_normal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_PRECISION;
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 0);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	result_format_updated();
}
void on_menu_item_display_engineering_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_BASE_3;
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 1);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	result_format_updated();
}
void on_menu_item_display_scientific_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_SCIENTIFIC;
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 2);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	result_format_updated();
}
void on_menu_item_display_purely_scientific_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_PURE;
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 3);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	result_format_updated();
}
void on_menu_item_display_non_scientific_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.min_exp = EXP_NONE;
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 4);
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget(main_glade, "combobox_numerical_display"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_combobox_numerical_display_changed, NULL);
	result_format_updated();
}
void on_menu_item_display_prefixes_activate(GtkMenuItem *w, gpointer) {
	printops.use_unit_prefixes = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_indicate_infinite_series_activate(GtkMenuItem *w, gpointer) {
	printops.indicate_infinite_series = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_show_ending_zeroes_activate(GtkMenuItem *w, gpointer) {
	printops.show_ending_zeroes = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_round_halfway_to_even_activate(GtkMenuItem *w, gpointer) {
	printops.round_halfway_to_even = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_negative_exponents_activate(GtkMenuItem *w, gpointer) {
	printops.negative_exponents = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_sort_minus_last_activate(GtkMenuItem *w, gpointer) {
	printops.sort_options.minus_last = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w));
	result_format_updated();
}
void on_menu_item_always_exact_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.approximation = APPROXIMATION_EXACT;
	GtkWidget *w_exact = glade_xml_get_widget (main_glade, "button_exact");
	g_signal_handlers_block_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_exact_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_exact), TRUE);
	g_signal_handlers_unblock_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_exact_toggled, NULL);	
	expression_calculation_updated();
}
void on_menu_item_try_exact_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.approximation = APPROXIMATION_TRY_EXACT;
	GtkWidget *w_exact = glade_xml_get_widget (main_glade, "button_exact");
	g_signal_handlers_block_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_exact_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_exact), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_exact_toggled, NULL);	
	expression_calculation_updated();
}
void on_menu_item_approximate_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w))) return;
	evalops.approximation = APPROXIMATION_APPROXIMATE;
	GtkWidget *w_exact = glade_xml_get_widget (main_glade, "button_exact");
	g_signal_handlers_block_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_exact_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_exact), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_exact, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_exact_toggled, NULL);	
	expression_calculation_updated();
}
void on_menu_item_fraction_decimal_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_DECIMAL;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	result_format_updated();
}
void on_menu_item_fraction_decimal_exact_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_DECIMAL_EXACT;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	result_format_updated();
}
void on_menu_item_fraction_combined_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_COMBINED;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), FALSE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	result_format_updated();
}
void on_menu_item_fraction_fraction_activate(GtkMenuItem *w, gpointer) {
	if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(w)))
		return;
	printops.number_fraction_format = FRACTION_FRACTIONAL;

	GtkWidget *w_fraction = glade_xml_get_widget (main_glade, "button_fraction");
	g_signal_handlers_block_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_fraction), TRUE);
	g_signal_handlers_unblock_matched((gpointer) w_fraction, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_button_fraction_toggled, NULL);	
	
	result_format_updated();
}

void on_menu_item_save_activate(GtkMenuItem*, gpointer) {
	add_as_variable();
}
void on_menu_item_save_image_activate(GtkMenuItem*, gpointer) {
	if(!pixbuf_result) return;
	GtkWidget *d = gtk_file_chooser_dialog_new(_("Select file to save PNG image to"), GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION >= 8	
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(d), TRUE);
#endif
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Allowed File Types"));
	gtk_file_filter_add_mime_type(filter, "image/png");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(d), filter);
	GtkFileFilter *filter_all = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter_all, "*");
	gtk_file_filter_set_name(filter_all, _("All Files"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(d), filter_all);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(d), "qalculate.png");
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
		gdk_pixbuf_save(pixbuf_result, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)), "png", NULL, NULL);
	}
	gtk_widget_destroy(d);
}
void on_menu_item_copy_activate(GtkMenuItem*, gpointer) {
	gtk_clipboard_set_text(gtk_clipboard_get(gdk_atom_intern("CLIPBOARD", FALSE)), result_text.c_str(), -1);
}
void on_menu_item_precision_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = get_precision_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_precision_dialog_spinbutton_precision_value_changed, NULL);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision")), PRECISION);	
	g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_precision_dialog_spinbutton_precision_value_changed, NULL);
	gtk_widget_show(dialog);
}
void on_menu_item_decimals_activate(GtkMenuItem*, gpointer) {
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

gboolean on_main_window_focus_in_event(GtkWidget*, GdkEventFocus*, gpointer) {
	focus_keeping_selection();
	return FALSE;
}


void on_button_registerup_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter, iter2;
	GtkTreePath *path;
	gint index;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(stackview)), &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		index = gtk_tree_path_get_indices(path)[0];
		gtk_tree_path_free(path);
		if(index == 0) return;
		CALCULATOR->moveRPNRegisterUp(index + 1);
		gtk_tree_model_iter_nth_child(model, &iter2, NULL, index - 1);
		gtk_list_store_set(stackstore, &iter, 0, i2s(index).c_str(), -1);
		gtk_list_store_set(stackstore, &iter2, 0, i2s(index + 1).c_str(), -1);
		gtk_list_store_swap(stackstore, &iter, &iter2);
		if(index == 1) {
			mstruct->unref();
			mstruct = CALCULATOR->getRPNRegister(1);
			mstruct->ref();
			setResult(NULL, true, false, false, "", 0, true);
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerup"), index != 1);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerdown"), TRUE);
	}
}
void on_button_registerdown_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter, iter2;
	GtkTreePath *path;
	gint index;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(stackview)), &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		index = gtk_tree_path_get_indices(path)[0];
		gtk_tree_path_free(path);
		if(index + 1 == (int) CALCULATOR->RPNStackSize()) return;
		CALCULATOR->moveRPNRegisterDown(index + 1);
		gtk_tree_model_iter_nth_child(model, &iter2, NULL, index + 1);
		gtk_list_store_set(stackstore, &iter, 0, i2s(index + 2).c_str(), -1);
		gtk_list_store_set(stackstore, &iter2, 0, i2s(index + 1).c_str(), -1);
		gtk_list_store_swap(stackstore, &iter, &iter2);
		if(index == 0) {
			mstruct->unref();
			mstruct = CALCULATOR->getRPNRegister(1);
			mstruct->ref();
			setResult(NULL, true, false, false, "", 0, true);
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerup"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerdown"), index + 2 < (int) CALCULATOR->RPNStackSize());
	}
}
void on_button_editregister_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(stackview)), &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(stackview), path, register_column, register_renderer, TRUE);
		gtk_tree_path_free(path);
	}
}
void on_button_deleteregister_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	gint index;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(stackview)), &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		index = gtk_tree_path_get_indices(path)[0];
		gtk_tree_path_free(path);
		CALCULATOR->deleteRPNRegister(index + 1);
		gtk_list_store_remove(stackstore, &iter);
		if(CALCULATOR->RPNStackSize() == 0) {
			gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_clearstack"), FALSE);
			clearresult();
			mstruct->clear();
		} else if(index == 0) {
			mstruct->unref();
			mstruct = CALCULATOR->getRPNRegister(1);
			mstruct->ref();
			setResult(NULL, true, false, false, "", 0, true);
		}
		updateRPNIndexes();
	}
}
void on_button_clearstack_clicked(GtkButton*, gpointer) {
	CALCULATOR->clearRPNStack();
	gtk_list_store_clear(stackstore);
	clearresult();
	mstruct->clear();
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_clearstack"), FALSE);
}
void on_stackview_selection_changed(GtkTreeSelection *treeselection, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	gint index;
	if(gtk_tree_selection_get_selected(treeselection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		index = gtk_tree_path_get_indices(path)[0];
		gtk_tree_path_free(path);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerup"), index != 0);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerdown"), index + 1 < (int) CALCULATOR->RPNStackSize());
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_editregister"), TRUE);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_deleteregister"), TRUE);
	} else {
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerup"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_registerdown"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_editregister"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "button_deleteregister"), FALSE);
	}
}
void on_stackview_item_edited(GtkCellRendererText*, gchar *path, gchar *new_text, gpointer) {
	int index = s2i(path);
	GtkTreeIter iter;
	gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(stackstore), &iter, NULL, index);
	gtk_list_store_set(stackstore, &iter, 1, new_text, -1);
	execute_expression(true, false, OPERATION_ADD, NULL, true, index);
}

/*
	check if entered unit name is valid, if not modify
*/
void on_unit_edit_entry_name_changed(GtkEditable *editable, gpointer) {
	if(!CALCULATOR->unitNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unit_edit_entry_name_changed, NULL);		
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidUnitName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unit_edit_entry_name_changed, NULL);		
	}
}
/*
	selected unit type in edit/new unit dialog has changed
*/
void on_unit_edit_optionmenu_class_changed(GtkOptionMenu *om, gpointer)
{

	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_label_relation_title"), gtk_option_menu_get_history(om) != UNIT_CLASS_BASE_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_label_base"), gtk_option_menu_get_history(om) != UNIT_CLASS_BASE_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_base"), gtk_option_menu_get_history(om) != UNIT_CLASS_BASE_UNIT);
	
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_label_exp"), gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_spinbutton_exp"), gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_label_relation"), gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_relation"), gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_checkbutton_exact"), gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_label_reversed"), gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT);
	gtk_widget_set_sensitive(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_reversed"), gtk_option_menu_get_history(om) == UNIT_CLASS_ALIAS_UNIT);
	
}
/*
	"New" button clicked in unit manager -- open new unit dialog
*/
void on_units_button_new_clicked(GtkButton*, gpointer) {
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
void on_units_button_edit_clicked(GtkButton*, gpointer) {
	Unit *u = get_selected_unit();
	if(u) {
		edit_unit("", u, glade_xml_get_widget (units_glade, "units_dialog"));
	}
}

/*
	"Insert" button clicked in unit manager -- insert selected unit in expression entry
*/
void on_units_button_insert_clicked(GtkButton*, gpointer) {
	Unit *u = get_selected_unit();
	if(u) {
		gchar *gstr;
		gstr = g_strdup(u->print(true, printops.abbreviate_names, printops.use_unicode_signs, &can_display_unicode_string_function, (void*) expression).c_str());
		insert_text(gstr);
		g_free(gstr);
	}
}

/*
	"Convert" button clicked in unit manager -- convert result to selected unit
*/
void on_units_button_convert_to_clicked(GtkButton*, gpointer) {
	Unit *u = get_selected_unit();
	if(u) {
		do_timeout = false;
		mstruct->set(CALCULATOR->convert(*mstruct, u, evalops));
		result_action_executed();
		do_timeout = true;
		focus_keeping_selection();		
	}
}

/*
	deletion of unit requested
*/
void on_units_button_delete_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Unit *u = get_selected_unit();
	if(u && u->isLocal()) {
		if(u->isUsedByOtherUnits()) {
			//do not delete units that are used by other units
			show_message(_("Cannot delete unit as it is needed by other units."), glade_xml_get_widget (units_glade, "units_dialog"));
			return;
		}
		for(size_t i = 0; i < recent_units.size(); i++) {
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
void on_variables_button_new_clicked(GtkButton*, gpointer) {
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
void on_variables_button_edit_clicked(GtkButton*, gpointer) {
	Variable *v = get_selected_variable();
	if(v) {
		if(!CALCULATOR->stillHasVariable(v)) {
			show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
			update_vmenu();
			return;
		}
		edit_variable("", v, NULL, glade_xml_get_widget (variables_glade, "variables_dialog"));
	}
}

/*
	"Insert" button clicked in variable manager -- insert variable name in expression entry
*/
void on_variables_button_insert_clicked(GtkButton*, gpointer) {
	Variable *v = get_selected_variable();
	if(v) {
		if(!CALCULATOR->stillHasVariable(v)) {
			show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
			update_vmenu();
			return;
		}
		gchar *gstr = g_strdup(v->preferredInputName(printops.abbreviate_names, true, false, false, &can_display_unicode_string_function, (void*) expression).name.c_str());
		insert_text(gstr);
		g_free(gstr);
	}
}

/*
	"Delete" button clicked in variable manager -- deletion of selected variable requested
*/
void on_variables_button_delete_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	Variable *v = get_selected_variable();
	if(v && !CALCULATOR->stillHasVariable(v)) {
		show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
		update_vmenu();
		return;
	}
	if(v && v->isLocal()) {
		for(size_t i = 0; i < recent_variables.size(); i++) {
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

void on_variables_button_export_clicked(GtkButton*, gpointer) {
	Variable *v = get_selected_variable();
	if(v && !CALCULATOR->stillHasVariable(v)) {
		show_message(_("Variable does not exist anymore."), glade_xml_get_widget (variables_glade, "variables_dialog"));
		update_vmenu();
		return;
	}
	if(v && v->isKnown()) {
		export_csv_file((KnownVariable*) v, glade_xml_get_widget (variables_glade, "variables_dialog"));
	}
}

/*
	"Close" button clicked in variable manager -- hide
*/
void on_variables_button_close_clicked(GtkButton*, gpointer) {
	gtk_widget_hide(glade_xml_get_widget (variables_glade, "variables_dialog"));
}

/*
	"New" button clicked in function manager -- open new function dialog
*/
void on_functions_button_new_clicked(GtkButton*, gpointer) {
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
void on_functions_button_edit_clicked(GtkButton*, gpointer) {
	MathFunction *f = get_selected_function();
	if(f) {
		edit_function("", f, glade_xml_get_widget (functions_glade, "functions_dialog"));
	}
}

/*
	"Insert" button clicked in function manager -- open dialog for insertion of function in expression entry
*/
void on_functions_button_insert_clicked(GtkButton*, gpointer) {
	insert_function(get_selected_function(), glade_xml_get_widget (functions_glade, "functions_dialog"));
}

/*
	"Apply" button clicked in function manager -- apply function to current result
*/
void on_functions_button_apply_clicked(GtkButton*, gpointer) {
	apply_function(get_selected_function(), glade_xml_get_widget (functions_glade, "functions_dialog"));
}

/*
	"Delete" button clicked in function manager -- deletion of selected function requested
*/
void on_functions_button_delete_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	MathFunction *f = get_selected_function();
	if(f && f->isLocal()) {
		for(size_t i = 0; i < recent_functions.size(); i++) {
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
void on_functions_button_close_clicked(GtkButton*, gpointer) {
	gtk_widget_hide(glade_xml_get_widget (functions_glade, "functions_dialog"));
}

void on_datasets_button_newset_clicked(GtkButton*, gpointer) {	
	edit_dataset(NULL, glade_xml_get_widget (datasets_glade, "datasets_dialog"));
}
void on_datasets_button_editset_clicked(GtkButton*, gpointer) {
	edit_dataset(selected_dataset, glade_xml_get_widget (datasets_glade, "datasets_dialog"));
}
void on_datasets_button_delset_clicked(GtkButton*, gpointer) {
	if(selected_dataset && selected_dataset->isLocal()) {
		for(size_t i = 0; i < recent_functions.size(); i++) {
			if(recent_functions[i] == selected_dataset) {
				recent_functions.erase(recent_functions.begin() + i);
				gtk_widget_destroy(recent_function_items[i]);
				recent_function_items.erase(recent_function_items.begin() + i);
				break;
			}
		}
		selected_dataset->destroy();
		selected_dataobject = NULL;		
		update_datasets_tree();
		on_tDatasets_selection_changed(gtk_tree_view_get_selection(GTK_TREE_VIEW(tDatasets)), NULL);
	}
}
void on_datasets_button_newobject_clicked(GtkButton*, gpointer) {
	edit_dataobject(selected_dataset, NULL, glade_xml_get_widget (datasets_glade, "datasets_dialog"));
}
void on_datasets_button_editobject_clicked(GtkButton*, gpointer) {
	edit_dataobject(selected_dataset, selected_dataobject, glade_xml_get_widget (datasets_glade, "datasets_dialog"));
}
void on_datasets_button_delobject_clicked(GtkButton*, gpointer) {
	if(selected_dataset && selected_dataobject) {
		selected_dataset->delObject(selected_dataobject);
		selected_dataobject = NULL;
		update_dataobjects();
	}
}
void on_datasets_button_close_clicked(GtkButton*, gpointer) {
	gtk_widget_hide(glade_xml_get_widget (datasets_glade, "datasets_dialog"));
}


/*
	check if entered function name is valid, if not modify
*/
void on_function_edit_entry_name_changed(GtkEditable *editable, gpointer) {
	if(!CALCULATOR->functionNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_function_edit_entry_name_changed, NULL);		
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidFunctionName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_function_edit_entry_name_changed, NULL);		
	}
}
/*
	check if entered variable name is valid, if not modify
*/
void on_variable_edit_entry_name_changed(GtkEditable *editable, gpointer) {
	if(!CALCULATOR->variableNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_variable_edit_entry_name_changed, NULL);
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidVariableName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_variable_edit_entry_name_changed, NULL);		
	}
}

void on_tMatrixEdit_edited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer model) {
	GtkTreeIter iter;
	gint i_column = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(cell), "column"));
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL(model), &iter, path_string);
	gtk_list_store_set(GTK_LIST_STORE (model), &iter, i_column * 2, new_text, -1);
}
gboolean on_tMatrixEdit_key_press_event(GtkWidget*, GdkEventKey *event, gpointer) {
	switch(event->keyval) {
		case GDK_Return: {break;}
		case GDK_Tab: {
			GtkTreeViewColumn *column = NULL;
			GtkTreePath *path = NULL;
			gtk_tree_view_get_cursor(GTK_TREE_VIEW(tMatrixEdit), &path, &column);
			if(path) {
				if(column) {
					for(size_t i = 0; i < matrix_edit_columns.size(); i++) {
						if(matrix_edit_columns[i] == column) {
							i++;
							if(i < matrix_edit_columns.size()) {
								gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrixEdit), path, matrix_edit_columns[i], FALSE);
								while(gtk_events_pending()) gtk_main_iteration();
								gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tMatrixEdit), path, matrix_edit_columns[i], FALSE, 0.0, 0.0);
							} else {
								gtk_tree_path_next(path);
								gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrixEdit), path, matrix_edit_columns[0], FALSE);
								while(gtk_events_pending()) gtk_main_iteration();
								gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tMatrixEdit), path, matrix_edit_columns[0], FALSE, 0.0, 0.0);
							}
							gtk_tree_path_free(path);
							on_tMatrixEdit_cursor_changed(GTK_TREE_VIEW(tMatrixEdit), NULL);
							return TRUE;
						}
					}
				}
				gtk_tree_path_free(path);
			}
			break;
		}
		default: {
			if(event->length == 0) return FALSE;
			GtkTreeViewColumn *column = NULL;
			GtkTreePath *path = NULL;
			gtk_tree_view_get_cursor(GTK_TREE_VIEW(tMatrixEdit), &path, &column);
			if(path) {
				if(column) {
					gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrixEdit), path, column, TRUE);
					while(gtk_events_pending()) gtk_main_iteration();
					gboolean return_val = FALSE;
					g_signal_emit_by_name((gpointer) glade_xml_get_widget (matrixedit_glade, "matrix_edit_dialog"), "key_press_event", event, &return_val);
					gtk_tree_path_free(path);
					return TRUE;
				}
				gtk_tree_path_free(path);
			}
		}
	}
	return FALSE;
}
gboolean on_tMatrixEdit_button_press_event(GtkWidget*, GdkEventButton *event, gpointer) {
	if(event->button != 1) return FALSE;
	GtkTreeViewColumn *column = NULL;
	GtkTreePath *path = NULL;
	if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tMatrixEdit), (gint) event->x, (gint) event->y, &path, &column, NULL, NULL) && path && column) {
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrixEdit), path, column, TRUE);
		gtk_tree_path_free(path);
		return TRUE;
	}
	if(path) gtk_tree_path_free(path);
	return FALSE;
}
GtkTreeIter matrix_edit_prev_iter;
gint matrix_edit_prev_column;
bool block_matrix_edit_update_cursor = false;
gboolean on_tMatrixEdit_cursor_changed(GtkTreeView*, gpointer) {
	if(block_matrix_edit_update_cursor) return FALSE;
	GtkTreeViewColumn *column = NULL;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	if(gtk_list_store_iter_is_valid(GTK_LIST_STORE(tMatrixEdit_store), &matrix_edit_prev_iter)) {
		gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &matrix_edit_prev_iter, matrix_edit_prev_column * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
	}
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(tMatrixEdit), &path, &column);
	bool b = false;
	if(path) {
		if(column) {
			if(gtk_tree_model_get_iter(GTK_TREE_MODEL(tMatrixEdit_store), &iter, path)) {
				gint i_column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(column), "column"));
				gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, i_column * 2 + 1, &stackview->style->mid[GTK_STATE_NORMAL], -1);
				matrix_edit_prev_iter = iter;
				matrix_edit_prev_column = i_column;
				gchar *pos_str;
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix")))) {
					pos_str = g_strdup_printf("(%i, %i)", i_column + 1, gtk_tree_path_get_indices(path)[0] + 1);
				} else {
					pos_str = g_strdup_printf("%i", i_column + 1 + matrix_edit_columns.size() * gtk_tree_path_get_indices(path)[0]);
				}
				gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_position")), pos_str);
				g_free(pos_str);
				b = true;
			}
		}
		gtk_tree_path_free(path);
	}
	if(!b) gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_position")), _("none"));
	return FALSE;
}

void on_matrix_edit_spinbutton_columns_value_changed(GtkSpinButton *w, gpointer) {
	gint c = matrix_edit_columns.size();
	gint new_c = gtk_spin_button_get_value_as_int(w);
	if(new_c < c) {
		for(gint index_c = new_c; index_c < c; index_c++) {
			gtk_tree_view_remove_column(GTK_TREE_VIEW(tMatrixEdit), matrix_edit_columns[index_c]);
		}
		matrix_edit_columns.resize(new_c);
	} else {
		GtkTreeIter iter;
		for(gint index_c = c; index_c < new_c; index_c++) {
			GtkCellRenderer *matrix_edit_renderer = gtk_cell_renderer_text_new();
			g_object_set(G_OBJECT(matrix_edit_renderer), "editable", TRUE, NULL);
			g_object_set(G_OBJECT(matrix_edit_renderer), "xalign", 1.0, NULL);
			g_object_set_data(G_OBJECT(matrix_edit_renderer), "column", GINT_TO_POINTER(index_c));
			g_signal_connect(G_OBJECT(matrix_edit_renderer), "edited", G_CALLBACK(on_tMatrixEdit_edited), GTK_TREE_MODEL(tMatrixEdit_store));
			GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(i2s(index_c).c_str(), matrix_edit_renderer, "text", index_c * 2, "background-gdk", index_c * 2 + 1, NULL);
			g_object_set_data (G_OBJECT(column), "column", GINT_TO_POINTER(index_c));
			gtk_tree_view_column_set_min_width(column, 50);
			gtk_tree_view_column_set_alignment(column, 0.5);
			gtk_tree_view_append_column(GTK_TREE_VIEW(tMatrixEdit), column);
			matrix_edit_columns.push_back(column);
		}
		if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tMatrixEdit_store), &iter)) return;
		bool b_matrix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix")));
		while(true) {
			for(gint index_c = c; index_c < new_c; index_c++) {
				if(b_matrix) gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, index_c * 2, "0", index_c * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
				else gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, index_c * 2, "", index_c * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
			}
			if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrixEdit_store), &iter)) break;
		}
	}
}
void on_matrix_edit_spinbutton_rows_value_changed(GtkSpinButton *w, gpointer) {
	gint new_r = gtk_spin_button_get_value_as_int(w);
	gint r = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(tMatrixEdit_store), NULL);
	gint c = matrix_edit_columns.size();
	bool b_matrix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrixedit_glade, "matrix_edit_radiobutton_matrix")));
	GtkTreeIter iter;
	if(r < new_r) {
		while(r < new_r) {
			gtk_list_store_append(GTK_LIST_STORE(tMatrixEdit_store), &iter);
			for(gint i = 0; i < c; i++) {
				if(b_matrix) gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, i * 2, "0", i * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
				else gtk_list_store_set(GTK_LIST_STORE(tMatrixEdit_store), &iter, i * 2, "", i * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
			}
			r++;
		}
	} else if(new_r < r) {
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(tMatrixEdit_store), &iter, NULL, new_r);
		while(gtk_list_store_iter_is_valid(GTK_LIST_STORE(tMatrixEdit_store), &iter)) {
			gtk_list_store_remove(GTK_LIST_STORE(tMatrixEdit_store), &iter);
		}
	}
}

void on_tMatrix_edited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer model) {
	GtkTreeIter iter;
	gint i_column = GPOINTER_TO_INT (g_object_get_data(G_OBJECT(cell), "column"));
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL(model), &iter, path_string);
	gtk_list_store_set(GTK_LIST_STORE (model), &iter, i_column * 2, new_text, -1);
}
gboolean on_tMatrix_key_press_event(GtkWidget*, GdkEventKey *event, gpointer) {
	switch(event->keyval) {
		case GDK_Return: {break;}
		case GDK_Tab: {
			GtkTreeViewColumn *column = NULL;
			GtkTreePath *path = NULL;
			gtk_tree_view_get_cursor(GTK_TREE_VIEW(tMatrix), &path, &column);
			if(path) {
				if(column) {
					for(size_t i = 0; i < matrix_columns.size(); i++) {
						if(matrix_columns[i] == column) {
							i++;
							if(i < matrix_columns.size()) {
								gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrix), path, matrix_columns[i], FALSE);
								while(gtk_events_pending()) gtk_main_iteration();
								gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tMatrix), path, matrix_columns[i], FALSE, 0.0, 0.0);
							} else {
								gtk_tree_path_next(path);
								gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrix), path, matrix_columns[0], FALSE);
								while(gtk_events_pending()) gtk_main_iteration();
								gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tMatrix), path, matrix_columns[0], FALSE, 0.0, 0.0);
							}
							gtk_tree_path_free(path);
							on_tMatrix_cursor_changed(GTK_TREE_VIEW(tMatrix), NULL);
							return TRUE;
						}
					}
				}
				gtk_tree_path_free(path);
			}
			break;
		}
		default: {
			if(event->length == 0) return FALSE;
			GtkTreeViewColumn *column = NULL;
			GtkTreePath *path = NULL;
			gtk_tree_view_get_cursor(GTK_TREE_VIEW(tMatrix), &path, &column);
			if(path) {
				if(column) {
					gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrix), path, column, TRUE);
					while(gtk_events_pending()) gtk_main_iteration();
					gboolean return_val = FALSE;
					g_signal_emit_by_name((gpointer) glade_xml_get_widget (matrix_glade, "matrix_dialog"), "key_press_event", event, &return_val);
					gtk_tree_path_free(path);
					return TRUE;
				}
				gtk_tree_path_free(path);
			}
		}
	}
	return FALSE;
}
gboolean on_tMatrix_button_press_event(GtkWidget*, GdkEventButton *event, gpointer) {
	if(event->button != 1) return FALSE;
	GtkTreeViewColumn *column = NULL;
	GtkTreePath *path = NULL;
	if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tMatrix), (gint) event->x, (gint) event->y, &path, &column, NULL, NULL) && path && column) {
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tMatrix), path, column, TRUE);
		gtk_tree_path_free(path);
		return TRUE;
	}
	if(path) gtk_tree_path_free(path);
	return FALSE;
}
GtkTreeIter matrix_prev_iter;
gint matrix_prev_column;
bool block_matrix_update_cursor = false;
gboolean on_tMatrix_cursor_changed(GtkTreeView*, gpointer) {
	if(block_matrix_update_cursor) return FALSE;
	GtkTreeViewColumn *column = NULL;
	GtkTreePath *path = NULL;
	GtkTreeIter iter;
	if(gtk_list_store_iter_is_valid(GTK_LIST_STORE(tMatrix_store), &matrix_prev_iter)) {
		gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &matrix_prev_iter, matrix_prev_column * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
	}
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(tMatrix), &path, &column);
	bool b = false;
	if(path) {
		if(column) {
			if(gtk_tree_model_get_iter(GTK_TREE_MODEL(tMatrix_store), &iter, path)) {
				gint i_column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(column), "column"));
				gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, i_column * 2 + 1, &stackview->style->mid[GTK_STATE_NORMAL], -1);
				matrix_prev_iter = iter;
				matrix_prev_column = i_column;
				gchar *pos_str;
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_radiobutton_matrix")))) {
					pos_str = g_strdup_printf("(%i, %i)", i_column + 1, gtk_tree_path_get_indices(path)[0] + 1);
				} else {
					pos_str = g_strdup_printf("%i", i_column + 1 + matrix_columns.size() * gtk_tree_path_get_indices(path)[0]);
				}
				gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrix_glade, "matrix_label_position")), pos_str);
				g_free(pos_str);
				b = true;
			}
		}
		gtk_tree_path_free(path);
	}
	if(!b) gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrix_glade, "matrix_label_position")), _("none"));
	return FALSE;
}

void on_matrix_spinbutton_columns_value_changed(GtkSpinButton *w, gpointer) {
	gint c = matrix_columns.size();
	gint new_c = gtk_spin_button_get_value_as_int(w);
	if(new_c < c) {
		for(gint index_c = new_c; index_c < c; index_c++) {
			gtk_tree_view_remove_column(GTK_TREE_VIEW(tMatrix), matrix_columns[index_c]);
		}
		matrix_columns.resize(new_c);
	} else {
		GtkTreeIter iter;
		for(gint index_c = c; index_c < new_c; index_c++) {
			GtkCellRenderer *matrix_renderer = gtk_cell_renderer_text_new();
			g_object_set(G_OBJECT(matrix_renderer), "editable", TRUE, NULL);
			g_object_set(G_OBJECT(matrix_renderer), "xalign", 1.0, NULL);
			g_object_set_data(G_OBJECT(matrix_renderer), "column", GINT_TO_POINTER(index_c));
			g_signal_connect(G_OBJECT(matrix_renderer), "edited", G_CALLBACK(on_tMatrix_edited), GTK_TREE_MODEL(tMatrix_store));
			GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(i2s(index_c).c_str(), matrix_renderer, "text", index_c * 2, "background-gdk", index_c * 2 + 1, NULL);
			g_object_set_data (G_OBJECT(column), "column", GINT_TO_POINTER(index_c));
			gtk_tree_view_column_set_min_width(column, 50);
			gtk_tree_view_column_set_alignment(column, 0.5);
			gtk_tree_view_append_column(GTK_TREE_VIEW(tMatrix), column);
			matrix_columns.push_back(column);
		}
		if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tMatrix_store), &iter)) return;
		bool b_matrix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_radiobutton_matrix")));
		while(true) {
			for(gint index_c = c; index_c < new_c; index_c++) {
				if(b_matrix) gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, "0", index_c * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
				else gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, index_c * 2, "", index_c * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
			}
			if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(tMatrix_store), &iter)) break;
		}
	}
}
void on_matrix_spinbutton_rows_value_changed(GtkSpinButton *w, gpointer) {
	gint new_r = gtk_spin_button_get_value_as_int(w);
	gint r = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(tMatrix_store), NULL);
	gint c = matrix_columns.size();
	bool b_matrix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (matrix_glade, "matrix_radiobutton_matrix")));
	GtkTreeIter iter;
	if(r < new_r) {
		while(r < new_r) {
			gtk_list_store_append(GTK_LIST_STORE(tMatrix_store), &iter);
			for(gint i = 0; i < c; i++) {
				if(b_matrix) gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, i * 2, "0", i * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
				else gtk_list_store_set(GTK_LIST_STORE(tMatrix_store), &iter, i * 2, "", i * 2 + 1, &stackview->style->base[GTK_STATE_NORMAL], -1);
			}
			r++;
		}
	} else if(new_r < r) {
		gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(tMatrix_store), &iter, NULL, new_r);
		while(gtk_list_store_iter_is_valid(GTK_LIST_STORE(tMatrix_store), &iter)) {
			gtk_list_store_remove(GTK_LIST_STORE(tMatrix_store), &iter);
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
void on_nbases_button_close_clicked(GtkButton*, gpointer) {
	gtk_widget_hide(glade_xml_get_widget (nbases_glade, "nbases_dialog"));
}
void on_nbases_entry_decimal_changed(GtkEditable *editable, gpointer) {	
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;
	if(is_in(OPERATORS EXP, str[str.length() - 1])) return;
	changing_in_nbases_dialog = true;	
	EvaluationOptions eo;
	eo.parse_options.angle_unit = evalops.parse_options.angle_unit;
	update_nbases_entries(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(editable)), evalops.parse_options), eo), 10);
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_binary_changed(GtkEditable *editable, gpointer) {
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;
	if(is_in(OPERATORS, str[str.length() - 1])) return;
	EvaluationOptions eo;
	eo.parse_options.base = BASE_BINARY;
	eo.parse_options.angle_unit = evalops.parse_options.angle_unit;
	changing_in_nbases_dialog = true;	
	update_nbases_entries(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str, evalops.parse_options), eo), 2);
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_octal_changed(GtkEditable *editable, gpointer) {
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;	
	if(is_in(OPERATORS, str[str.length() - 1])) return;
	EvaluationOptions eo;
	eo.parse_options.base = BASE_OCTAL;
	eo.parse_options.angle_unit = evalops.parse_options.angle_unit;
	changing_in_nbases_dialog = true;	
	update_nbases_entries(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str, evalops.parse_options), eo), 8);
	changing_in_nbases_dialog = false;	
}
void on_nbases_entry_hexadecimal_changed(GtkEditable *editable, gpointer) {
	if(changing_in_nbases_dialog) return;
	string str = gtk_entry_get_text(GTK_ENTRY(editable));
	remove_blank_ends(str);
	if(str.empty()) return;	
	if(is_in(OPERATORS, str[str.length() - 1])) return;
	EvaluationOptions eo;
	eo.parse_options.base = BASE_HEXADECIMAL;
	eo.parse_options.angle_unit = evalops.parse_options.angle_unit;
	changing_in_nbases_dialog = true;	
	update_nbases_entries(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str, evalops.parse_options), eo), 16);
	changing_in_nbases_dialog = false;	
}

void on_button_functions_clicked(GtkButton*, gpointer) {
	manage_functions();
}
void on_button_variables_clicked(GtkButton*, gpointer) {
	manage_variables();
}
void on_button_units_clicked(GtkButton*, gpointer) {
	manage_units();
}
void on_button_convert_clicked(GtkButton*, gpointer user_data) {
	on_menu_item_convert_to_unit_expression_activate(NULL, user_data);
}

void on_menu_item_about_activate(GtkMenuItem*, gpointer) {
	GtkWidget *dialog = get_about_dialog();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);
}
void on_menu_item_help_activate(GtkMenuItem*, gpointer) {
#ifdef HAVE_LIBGNOME
	GError *error = NULL;
	//gnome_help_display_desktop(NULL, "qalculate-gtk", "qalculate-gtk", NULL, &error);
	gnome_help_display("qalculate-gtk", NULL, &error);
	if(error) {
		gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help for Qalculate!.\n%s"), error_str);
		g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		gtk_widget_show (dialog);
		g_free(error_str);
		g_error_free(error);
	}
#endif	
}

/*
	precision has changed in precision dialog
*/
void on_precision_dialog_spinbutton_precision_value_changed(GtkSpinButton *w, gpointer) {
	CALCULATOR->setPrecision(gtk_spin_button_get_value_as_int(w));
//	execute_expression();
}
void on_precision_dialog_button_recalculate_clicked(GtkButton*, gpointer) {
	CALCULATOR->setPrecision(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision"))));
	execute_expression();
}


void on_decimals_dialog_spinbutton_max_value_changed(GtkSpinButton *w, gpointer) {
	printops.max_decimals = gtk_spin_button_get_value_as_int(w);
	result_format_updated();
}
void on_decimals_dialog_spinbutton_min_value_changed(GtkSpinButton *w, gpointer) {
	printops.min_decimals = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));
	result_format_updated();
}
void on_decimals_dialog_checkbutton_max_toggled(GtkToggleButton *w, gpointer) {
	printops.use_max_decimals = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_max"), printops.use_max_decimals);
	result_format_updated();
}
void on_decimals_dialog_checkbutton_min_toggled(GtkToggleButton *w, gpointer) {
	printops.use_min_decimals = gtk_toggle_button_get_active(w);
	gtk_widget_set_sensitive(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_min"), printops.use_min_decimals);
	result_format_updated();
}

void on_unknown_edit_checkbutton_custom_assumptions_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_type"), gtk_toggle_button_get_active(w));
	gtk_widget_set_sensitive(glade_xml_get_widget (unknownedit_glade, "unknown_edit_hbox_sign"), gtk_toggle_button_get_active(w));
}
void on_unknown_edit_optionmenu_type_changed(GtkOptionMenu *om, gpointer) {	
	if((AssumptionType) gtk_option_menu_get_history(om) <= ASSUMPTION_TYPE_COMPLEX && (AssumptionSign) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"))) != ASSUMPTION_SIGN_NONZERO) {
		g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_sign_changed, NULL);
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign")), ASSUMPTION_SIGN_UNKNOWN);	
		g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_sign"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_sign_changed, NULL);
	}
}
void on_unknown_edit_optionmenu_sign_changed(GtkOptionMenu *om, gpointer) {	
	if((AssumptionSign) gtk_option_menu_get_history(om) != ASSUMPTION_SIGN_NONZERO && (AssumptionSign) gtk_option_menu_get_history(om) != ASSUMPTION_SIGN_UNKNOWN && (AssumptionType) gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"))) <= ASSUMPTION_TYPE_COMPLEX) {
		g_signal_handlers_block_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_type_changed, NULL);
		gtk_option_menu_set_history(GTK_OPTION_MENU(glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type")), ASSUMPTION_TYPE_REAL);	
		g_signal_handlers_unblock_matched((gpointer) glade_xml_get_widget (unknownedit_glade, "unknown_edit_optionmenu_type"), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_unknown_edit_optionmenu_type_changed, NULL);
	}
}

gboolean on_key_press_event(GtkWidget *o, GdkEventKey *event, gpointer) {
	if(!GTK_WIDGET_HAS_FOCUS(expression) && (event->keyval > GDK_Hyper_R || event->keyval < GDK_Shift_L)) {
		GtkWidget *w = gtk_window_get_focus(GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")));
		if(gtk_bindings_activate_event(GTK_OBJECT(o), event)) return TRUE;
		if(w && gtk_bindings_activate_event(GTK_OBJECT(w), event)) return TRUE;
		focus_keeping_selection();
	}
	return FALSE;
}

gboolean on_expression_key_press_event(GtkWidget*, GdkEventKey *event, gpointer) {
	switch(event->keyval) {
		case GDK_asciicircum: {}
		case GDK_dead_circumflex: {
			if(rpn_mode && !rpn_keypad_only) {
				calculateRPN(OPERATION_RAISE);
				return TRUE;
			}
			gint end = 0;
			if(!evalops.parse_options.rpn) {
				wrap_expression_selection();
			}
			end = gtk_editable_get_position(GTK_EDITABLE(expression));
			gtk_editable_insert_text(GTK_EDITABLE(expression), "^", -1, &end);				
			gtk_editable_set_position(GTK_EDITABLE(expression), end);
			return TRUE;
		}
		case GDK_KP_Divide: {
			if(rpn_mode) {
				calculateRPN(OPERATION_DIVIDE);
				return TRUE;
			}
			if(!evalops.parse_options.rpn) {
				wrap_expression_selection();
			}
			break;
		}
		case GDK_KP_Multiply: {
			if(event->state & GDK_CONTROL_MASK) {
				if(rpn_mode) {
					calculateRPN(OPERATION_RAISE);
					return TRUE;
				}
				gint end = 0;
				if(!evalops.parse_options.rpn) {
					wrap_expression_selection();
				}
				end = gtk_editable_get_position(GTK_EDITABLE(expression));
				gtk_editable_insert_text(GTK_EDITABLE(expression), "^", -1, &end);				
				gtk_editable_set_position(GTK_EDITABLE(expression), end);
				return TRUE;
			}
			if(rpn_mode) {
				calculateRPN(OPERATION_MULTIPLY);
				return TRUE;
			}
			if(!evalops.parse_options.rpn) {
				wrap_expression_selection();
			}
			break;
		}
		case GDK_KP_Add: {
			if(rpn_mode) {
				calculateRPN(OPERATION_ADD);
				return TRUE;
			}
			if(!evalops.parse_options.rpn) {
				wrap_expression_selection();
			}
			break;
		}
		case GDK_KP_Subtract: {
			if(rpn_mode) {
				calculateRPN(OPERATION_SUBTRACT);
				return TRUE;
			}
			if(!evalops.parse_options.rpn) {
				wrap_expression_selection();
			}
			break;
		}
		case GDK_asterisk: {
			if(event->state & GDK_CONTROL_MASK) {
				if(rpn_mode && !rpn_keypad_only) {
					calculateRPN(OPERATION_RAISE);
					return TRUE;
				}
				gint end = 0;
				if(!evalops.parse_options.rpn) {
					wrap_expression_selection();
				}
				end = gtk_editable_get_position(GTK_EDITABLE(expression));
				gtk_editable_insert_text(GTK_EDITABLE(expression), "^", -1, &end);				
				gtk_editable_set_position(GTK_EDITABLE(expression), end);
				return TRUE;
			}
			if(rpn_mode && !rpn_keypad_only) {
				calculateRPN(OPERATION_MULTIPLY);
				return TRUE;
			}
		}
		case GDK_slash: {
			if(rpn_mode && !rpn_keypad_only) {
				calculateRPN(OPERATION_DIVIDE);
				return TRUE;
			}
		}
		case GDK_plus: {
			if(rpn_mode && !rpn_keypad_only) {
				calculateRPN(OPERATION_ADD);
				return TRUE;
			}
		}
		case GDK_minus: {
			if(rpn_mode && !rpn_keypad_only) {
				calculateRPN(OPERATION_SUBTRACT);
				return TRUE;
			}
			if(!evalops.parse_options.rpn) {
				wrap_expression_selection();
			}
			break;
		}
		case GDK_E: {
			if(event->state & GDK_CONTROL_MASK) {
				if(rpn_mode) {
					calculateRPN(OPERATION_EXP10);
					return TRUE;
				}
				gint end = 0;
				if(!evalops.parse_options.rpn) {
					wrap_expression_selection();
				}
				end = gtk_editable_get_position(GTK_EDITABLE(expression));
				if(printops.lower_case_e) gtk_editable_insert_text(GTK_EDITABLE(expression), "e", -1, &end);
				else gtk_editable_insert_text(GTK_EDITABLE(expression), "E", -1, &end);
				gtk_editable_set_position(GTK_EDITABLE(expression), end);
				return TRUE;
			}
			break;
		}
		case GDK_Page_Up: {
			if(expression_history_index + 1 < (int) expression_history.size()) {
				expression_history_index++;
				dont_change_index = true;
				gtk_entry_set_text(GTK_ENTRY(expression), "");
				gint pos = 0;
				gtk_editable_insert_text(GTK_EDITABLE(expression), expression_history[expression_history_index].c_str(), -1, &pos);
				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
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
					gint pos = 0;
					gtk_editable_insert_text(GTK_EDITABLE(expression), " ", -1, &pos);
					gtk_entry_set_text(GTK_ENTRY(expression), "");
				} else {
					gtk_entry_set_text(GTK_ENTRY(expression), "");
					gint pos = 0;
					gtk_editable_insert_text(GTK_EDITABLE(expression), expression_history[expression_history_index].c_str(), -1, &pos);
					gtk_editable_set_position(GTK_EDITABLE(expression), pos);
				}
				dont_change_index = false;
			}
			return TRUE;
		}
		case GDK_KP_Separator: {
			gint end = gtk_editable_get_position(GTK_EDITABLE(expression));
			gtk_editable_insert_text(GTK_EDITABLE(expression), CALCULATOR->getDecimalPoint().c_str(), -1, &end);
			gtk_editable_set_position(GTK_EDITABLE(expression), end);
			return TRUE;
		}
	}
	gint pos = gtk_editable_get_position(GTK_EDITABLE(expression));
	switch(event->keyval) {
		case GDK_KP_Multiply: {}
		case GDK_asterisk: {
			if(printops.use_unicode_signs && printops.multiplication_sign == MULTIPLICATION_SIGN_DOT && can_display_unicode_string_function(SIGN_MULTIDOT, (void*) expression)) {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_MULTIDOT, -1, &pos);
			} else if(printops.use_unicode_signs && printops.multiplication_sign == MULTIPLICATION_SIGN_DOT && can_display_unicode_string_function(SIGN_SMALLCIRCLE, (void*) expression)) {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_SMALLCIRCLE, -1, &pos);
			} else if(printops.use_unicode_signs && printops.multiplication_sign == MULTIPLICATION_SIGN_X && can_display_unicode_string_function(SIGN_MULTIPLICATION, (void*) expression)) {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_MULTIPLICATION, -1, &pos);
			} else {
				gtk_editable_insert_text(GTK_EDITABLE(expression), "*", -1, &pos);
			}
			gtk_editable_set_position(GTK_EDITABLE(expression), pos);
			return TRUE;			
		}
		case GDK_KP_Divide: {}
		case GDK_slash: {
			if(printops.use_unicode_signs && printops.division_sign == DIVISION_SIGN_DIVISION && can_display_unicode_string_function(SIGN_DIVISION, (void*) expression)) {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_DIVISION, -1, &pos);
				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
				return TRUE;			
			}
			break;
		}
		case GDK_KP_Subtract: {}								
		case GDK_minus: {
			if(printops.use_unicode_signs && can_display_unicode_string_function(SIGN_MINUS, (void*) expression)) {
				gtk_editable_insert_text(GTK_EDITABLE(expression), SIGN_MINUS, -1, &pos);
				gtk_editable_set_position(GTK_EDITABLE(expression), pos);
				return TRUE;
			}
			break;
		}
		case GDK_braceleft: {}
		case GDK_braceright: {
			return TRUE;
		}
	}	
	return FALSE;
}

gboolean on_resultview_expose_event(GtkWidget*, GdkEventExpose*, gpointer) {
	if(pixbuf_result) {
		gint w = 0, h = 0;
		w = gdk_pixbuf_get_width(pixbuf_result);
		h = gdk_pixbuf_get_height(pixbuf_result);
		if(resultview->allocation.width - 15 > w) {
			gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, resultview->allocation.width - w - 15, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
		} else {
			gdk_draw_pixbuf(resultview->window, resultview->style->fg_gc[GTK_WIDGET_STATE(resultview)], pixbuf_result, 0, 0, 0, (resultview->allocation.height - h) / 2, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
		}
	}	
	return TRUE;
}
void on_matrix_edit_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Elements (in horizontal order)"));
	}
	on_tMatrixEdit_cursor_changed(GTK_TREE_VIEW(tMatrixEdit), NULL);
}
void on_matrix_edit_radiobutton_vector_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrixedit_glade, "matrix_edit_label_elements")), _("Elements (in horizontal order)"));
	}
	on_tMatrixEdit_cursor_changed(GTK_TREE_VIEW(tMatrixEdit), NULL);
}
void on_matrix_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer) {
	if(gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrix_glade, "matrix_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrix_glade, "matrix_label_elements")), _("Elements (in horizontal order)"));
	}
	on_tMatrix_cursor_changed(GTK_TREE_VIEW(tMatrix), NULL);
}
void on_matrix_radiobutton_vector_toggled(GtkToggleButton *w, gpointer) {
	if(!gtk_toggle_button_get_active(w)) {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrix_glade, "matrix_label_elements")), _("Elements"));
	} else {
		gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (matrix_glade, "matrix_label_elements")), _("Elements (in horizontal order)"));
	}
	on_tMatrix_cursor_changed(GTK_TREE_VIEW(tMatrix), NULL);
}

void on_csv_import_radiobutton_matrix_toggled(GtkToggleButton*, gpointer) {
}
void on_csv_import_radiobutton_vectors_toggled(GtkToggleButton*, gpointer) {
}
void on_csv_import_optionmenu_delimiter_changed(GtkOptionMenu *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (csvimport_glade, "csv_import_entry_delimiter_other"), gtk_option_menu_get_history(w) == DELIMITER_OTHER);
}
void on_csv_import_button_file_clicked(GtkButton*, gpointer) {
	GtkWidget *d = gtk_file_chooser_dialog_new(_("Select file to import"), GTK_WINDOW(glade_xml_get_widget(csvimport_glade, "csv_import_dialog")), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(d), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file"))));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvimport_glade, "csv_import_entry_file")), gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)));
	}
	gtk_widget_destroy(d);
}

void on_csv_export_optionmenu_delimiter_changed(GtkOptionMenu *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_entry_delimiter_other"), gtk_option_menu_get_history(w) == DELIMITER_OTHER);
}
void on_csv_export_button_file_clicked(GtkButton*, gpointer) {
	GtkWidget *d = gtk_file_chooser_dialog_new(_("Select file to export to"), GTK_WINDOW(glade_xml_get_widget(csvimport_glade, "csv_import_dialog")), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(d), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file"))));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (csvexport_glade, "csv_export_entry_file")), gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)));
	}
	gtk_widget_destroy(d);
}
void on_csv_export_radiobutton_current_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (csvexport_glade, "csv_export_entry_matrix"), !gtk_toggle_button_get_active(w));
}
void on_csv_export_radiobutton_matrix_toggled(GtkToggleButton *w, gpointer) {
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
void on_type_label_file_clicked(GtkButton*, gpointer user_data) {
	GtkWidget *d = gtk_file_chooser_dialog_new(_("Select file to import"), GTK_WINDOW(glade_xml_get_widget(csvimport_glade, "csv_import_dialog")), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(d), gtk_entry_get_text(GTK_ENTRY(user_data)));
	if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
		gtk_entry_set_text(GTK_ENTRY(user_data), gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)));
	}
	gtk_widget_destroy(d);
}

void on_functions_button_deactivate_clicked(GtkButton*, gpointer) {
	MathFunction *f = get_selected_function();
	if(f) {
		f->setActive(!f->isActive());
		update_fmenu();
	}
}
void on_variables_button_deactivate_clicked(GtkButton*, gpointer) {
	Variable *v = get_selected_variable();
	if(v) {
		v->setActive(!v->isActive());
		update_vmenu();
	}
}
void on_units_button_deactivate_clicked(GtkButton*, gpointer) {
	Unit *u = get_selected_unit();
	if(u) {
		u->setActive(!u->isActive());
		update_umenus();
	}
}

void on_function_edit_button_subfunctions_clicked(GtkButton*, gpointer) {
	gtk_window_set_transient_for(GTK_WINDOW(glade_xml_get_widget (functionedit_glade, "function_edit_dialog_subfunctions")), GTK_WINDOW(glade_xml_get_widget (functionedit_glade, "function_edit_dialog")));
	gtk_dialog_run(GTK_DIALOG(glade_xml_get_widget (functionedit_glade, "function_edit_dialog_subfunctions")));
	gtk_widget_hide(glade_xml_get_widget (functionedit_glade, "function_edit_dialog_subfunctions"));
}
void on_function_edit_button_add_subfunction_clicked(GtkButton*, gpointer) {
	GtkTreeIter iter;	
	gtk_list_store_append(tSubfunctions_store, &iter);
	string str = "\\";
	last_subfunction_index++;
	str += i2s(last_subfunction_index);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_precalculate")))) {
		gtk_list_store_set(tSubfunctions_store, &iter, 0, str.c_str(), 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("Yes"), 3, last_subfunction_index, 4, TRUE, -1);
	} else {
		gtk_list_store_set(tSubfunctions_store, &iter, 0, str.c_str(), 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("No"), 3, last_subfunction_index, 4, FALSE, -1);
	}
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression")), "");
}
void on_function_edit_button_modify_subfunction_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(tSubfunctions)), &model, &iter)) {
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_checkbutton_precalculate")))) {
			gtk_list_store_set(tSubfunctions_store, &iter, 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("Yes"), 4, TRUE, -1);
		} else {
			gtk_list_store_set(tSubfunctions_store, &iter, 1, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_subexpression"))), 2, _("No"), 4, FALSE, -1);
		}
	}
}
void on_function_edit_button_remove_subfunction_clicked(GtkButton*, gpointer) {
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
void on_function_edit_entry_subexpression_activate(GtkEntry*, gpointer) {
	if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_subfunction"))) {
		on_function_edit_button_add_subfunction_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_subfunction")), NULL);
	} else if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_subfunction"))) {
		on_function_edit_button_modify_subfunction_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_subfunction")), NULL);
	}
}
void on_function_edit_button_add_argument_clicked(GtkButton*, gpointer) {
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
			case MENU_ARGUMENT_TYPE_DATA_OBJECT: {arg = new DataObjectArgument(NULL, ""); break;}
			case MENU_ARGUMENT_TYPE_DATA_PROPERTY: {arg = new DataPropertyArgument(NULL, ""); break;}
			default: {arg = new Argument();}
		}
	}
	arg->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name"))));		
	gtk_list_store_set(tFunctionArguments_store, &iter, 0, arg->name().c_str(), 1, arg->printlong().c_str(), 2, arg, -1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name")), "");
}
void on_function_edit_button_remove_argument_clicked(GtkButton*, gpointer) {
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
void on_function_edit_button_modify_argument_clicked(GtkButton*, gpointer) {
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
				case MENU_ARGUMENT_TYPE_DATA_OBJECT: {argtype = ARGUMENT_TYPE_DATA_OBJECT; break;}
				case MENU_ARGUMENT_TYPE_DATA_PROPERTY: {argtype = ARGUMENT_TYPE_DATA_PROPERTY; break;}	
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
					case MENU_ARGUMENT_TYPE_DATA_OBJECT: {selected_argument = new DataObjectArgument(NULL, ""); break;}
					case MENU_ARGUMENT_TYPE_DATA_PROPERTY: {selected_argument = new DataPropertyArgument(NULL, ""); break;}
					default: {selected_argument = new Argument();}
				}			
			}
			
		}
		selected_argument->setName(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_argument_name"))));
		gtk_list_store_set(tFunctionArguments_store, &iter, 0, selected_argument->name().c_str(), 1, selected_argument->printlong().c_str(), 2, (gpointer) selected_argument, -1);
	}
}
void on_function_edit_entry_argument_name_activate(GtkEntry*, gpointer) {
	if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_argument"))) {
		on_function_edit_button_add_argument_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_add_argument")), NULL);
	} else if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_argument"))) {
		on_function_edit_button_modify_argument_clicked(GTK_BUTTON(glade_xml_get_widget (functionedit_glade, "function_edit_button_modify_argument")), NULL);
	}
}
void on_function_edit_button_rules_clicked(GtkButton*, gpointer) {
	edit_argument(get_selected_argument());
}
void on_argument_rules_checkbutton_enable_min_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_min"), gtk_toggle_button_get_active(w));
}
void on_argument_rules_checkbutton_enable_max_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_spinbutton_max"), gtk_toggle_button_get_active(w));
}
void on_argument_rules_checkbutton_enable_condition_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (argumentrules_glade, "argument_rules_entry_condition"), gtk_toggle_button_get_active(w));
}
#define SET_NAMES_LE(x,y,z)	GtkTreeIter iter;\
				string str;\
				if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tNames_store), &iter)) {\
					gchar *gstr;\
					gtk_tree_model_get(GTK_TREE_MODEL(tNames_store), &iter, NAMES_NAME_COLUMN, &gstr, -1);\
					if(strlen(gstr) > 0) {\
						gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (x, y)), gstr);\
					}\
					g_free(gstr);\
					if(gtk_tree_model_iter_next(GTK_TREE_MODEL(tNames_store), &iter)) {\
						str += "+ ";\
						while(true) {\
							gtk_tree_model_get(GTK_TREE_MODEL(tNames_store), &iter, NAMES_NAME_COLUMN, &gstr, -1);\
							str += gstr;\
							g_free(gstr);\
							if(!gtk_tree_model_iter_next(GTK_TREE_MODEL(tNames_store), &iter)) break;\
							str += ", ";\
						}\
					}\
				}\
				gtk_label_set_text(GTK_LABEL(glade_xml_get_widget (x, z)), str.c_str());
				
void on_variable_edit_button_names_clicked(GtkButton*, gpointer) {
	edit_names(get_edited_variable(), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (variableedit_glade, "variable_edit_entry_name"))), glade_xml_get_widget (variableedit_glade, "variable_edit_dialog"));
	SET_NAMES_LE(variableedit_glade, "variable_edit_entry_name", "variable_edit_label_names")
}
void on_unknown_edit_button_names_clicked(GtkButton*, gpointer) {
	edit_names(get_edited_unknown(), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unknownedit_glade, "unknown_edit_entry_name"))), glade_xml_get_widget (unknownedit_glade, "unknown_edit_dialog"));
	SET_NAMES_LE(unknownedit_glade, "unknown_edit_entry_name", "unknown_edit_label_names")
}
void on_matrix_edit_button_names_clicked(GtkButton*, gpointer) {
	edit_names(get_edited_matrix(), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_name"))), glade_xml_get_widget (matrixedit_glade, "matrix_edit_dialog"));
	SET_NAMES_LE(matrixedit_glade, "matrix_edit_entry_name", "matrix_edit_label_names")
}
void on_function_edit_button_names_clicked(GtkButton*, gpointer) {
	edit_names(get_edited_function(), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (functionedit_glade, "function_edit_entry_name"))), glade_xml_get_widget (functionedit_glade, "function_edit_dialog"));
	SET_NAMES_LE(functionedit_glade, "function_edit_entry_name", "function_edit_label_names")
}
void on_unit_edit_button_names_clicked(GtkButton*, gpointer) {
	edit_names(get_edited_unit(), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unitedit_glade, "unit_edit_entry_name"))), glade_xml_get_widget (unitedit_glade, "unit_edit_dialog"));
	SET_NAMES_LE(unitedit_glade, "unit_edit_entry_name", "unit_edit_label_names")
}

void on_names_edit_checkbutton_abbreviation_toggled(GtkToggleButton *w, gpointer) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive")), gtk_toggle_button_get_active(w));
}
void on_names_edit_button_add_clicked(GtkButton*, gpointer) {
	if(strlen(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name")))) == 0) {
		gtk_widget_grab_focus(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"));
		show_message(_("Empty name field."), glade_xml_get_widget (namesedit_glade, "names_edit_dialog"));
		return;
	}
	bool name_taken = false;
	if(editing_variable && CALCULATOR->variableNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_variable())) name_taken = true;
	else if(editing_unknown && CALCULATOR->variableNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_unknown())) name_taken = true;
	else if(editing_matrix && CALCULATOR->variableNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_matrix())) name_taken = true;
	else if(editing_unit && CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_unit())) name_taken = true;
	else if(editing_function && CALCULATOR->functionNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_function())) name_taken = true;
	else if(editing_dataset && CALCULATOR->functionNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_dataset())) name_taken = true;
	if(name_taken) {
		if(!ask_question(_("A conflicting object with the same name exists. If you proceed and save changes, the conflicting object will be overwritten or deactivated.\nDo you want to proceed?"), glade_xml_get_widget (namesedit_glade, "names_edit_dialog"))) {
			gtk_widget_grab_focus(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"));
			return;
		}
	}
	GtkTreeIter iter;
	gtk_list_store_append(tNames_store, &iter);
	if(editing_dataproperty) gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), NAMES_ABBREVIATION_STRING_COLUMN, "-", NAMES_PLURAL_STRING_COLUMN, "-", NAMES_REFERENCE_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference")))), NAMES_ABBREVIATION_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation"))), NAMES_PLURAL_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural"))), NAMES_UNICODE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_unicode"))), NAMES_REFERENCE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference"))), NAMES_SUFFIX_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_suffix"))), NAMES_AVOID_INPUT_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_avoid_input"))), NAMES_CASE_SENSITIVE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive"))), -1);
	else gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), NAMES_ABBREVIATION_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation")))), NAMES_PLURAL_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural")))), NAMES_REFERENCE_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference")))), NAMES_ABBREVIATION_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation"))), NAMES_PLURAL_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural"))), NAMES_UNICODE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_unicode"))), NAMES_REFERENCE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference"))), NAMES_SUFFIX_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_suffix"))), NAMES_AVOID_INPUT_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_avoid_input"))), NAMES_CASE_SENSITIVE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive"))), -1);
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name")), "");
}
void on_names_edit_button_modify_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tNames));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		char *gstr;
		gtk_tree_model_get(GTK_TREE_MODEL(tNames_store), &iter, NAMES_NAME_COLUMN, &gstr, -1);
		if(strcmp(gstr, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name")))) != 0) {
			bool name_taken = false;
			if(editing_variable && CALCULATOR->variableNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_variable())) name_taken = true;
			else if(editing_unknown && CALCULATOR->variableNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_unknown())) name_taken = true;
			else if(editing_matrix && CALCULATOR->variableNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_matrix())) name_taken = true;
			else if(editing_unit && CALCULATOR->unitNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_unit())) name_taken = true;
			else if(editing_function && CALCULATOR->functionNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_function())) name_taken = true;
			else if(editing_dataset && CALCULATOR->functionNameTaken(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), get_edited_dataset())) name_taken = true;
			if(name_taken) {
				if(!ask_question(_("A conflicting object with the same name exists. If you proceed and save changes, the conflicting object will be overwritten or deactivated.\nDo you want to proceed?"), glade_xml_get_widget (namesedit_glade, "names_edit_dialog"))) {
					g_free(gstr);
					return;
				}
			}
		}
		g_free(gstr);
		if(editing_dataproperty) gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), NAMES_ABBREVIATION_STRING_COLUMN, "-", NAMES_PLURAL_STRING_COLUMN, "-", NAMES_REFERENCE_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference")))), NAMES_ABBREVIATION_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation"))), NAMES_PLURAL_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural"))), NAMES_UNICODE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_unicode"))), NAMES_REFERENCE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference"))), NAMES_SUFFIX_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_suffix"))), NAMES_AVOID_INPUT_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_avoid_input"))), NAMES_CASE_SENSITIVE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive"))), -1);
		else gtk_list_store_set(tNames_store, &iter, NAMES_NAME_COLUMN, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name"))), NAMES_ABBREVIATION_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation")))), NAMES_PLURAL_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural")))), NAMES_REFERENCE_STRING_COLUMN, b2yn(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference")))), NAMES_ABBREVIATION_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_abbreviation"))), NAMES_PLURAL_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_plural"))), NAMES_UNICODE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_unicode"))), NAMES_REFERENCE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_reference"))), NAMES_SUFFIX_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_suffix"))), NAMES_AVOID_INPUT_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_avoid_input"))), NAMES_CASE_SENSITIVE_COLUMN, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_checkbutton_case_sensitive"))), -1);
	}
}
void on_names_edit_button_remove_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tNames));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		gtk_list_store_remove(tNames_store, &iter);
	}
	gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (namesedit_glade, "names_edit_entry_name")), "");
}
void on_names_edit_entry_name_activate(GtkEntry*, gpointer) {
	if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (namesedit_glade, "names_edit_button_add"))) {
		on_names_edit_button_add_clicked(GTK_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_button_add")), NULL);
	} else if(GTK_WIDGET_SENSITIVE(glade_xml_get_widget (namesedit_glade, "names_edit_button_modify"))) {
		on_names_edit_button_modify_clicked(GTK_BUTTON(glade_xml_get_widget (namesedit_glade, "names_edit_button_modify")), NULL);
	}
}
void on_names_edit_entry_name_changed(GtkEditable *editable, gpointer) {
	if(editing_unit) {
		if(!CALCULATOR->unitNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
			g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_names_edit_entry_name_changed, NULL);
			gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidUnitName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
			g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_names_edit_entry_name_changed, NULL);
		}
	} else if(editing_function || editing_dataset) {
		if(!CALCULATOR->functionNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
			g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_names_edit_entry_name_changed, NULL);
			gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidFunctionName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
			g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_names_edit_entry_name_changed, NULL);
		}
	} else if(!editing_dataproperty) {
		if(!CALCULATOR->variableNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
			g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_names_edit_entry_name_changed, NULL);
			gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidVariableName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
			g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_names_edit_entry_name_changed, NULL);
		}
	}
}


bool generate_plot(PlotParameters &pp, vector<MathStructure> &y_vectors, vector<MathStructure> &x_vectors, vector<PlotDataParameters*> &pdps) {
	GtkTreeIter iter;
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
	if(!b) {
		return false;
	}	
	while(b) {
		int count = 1;
		gchar *gstr1, *gstr2;
		gint type = 0, style = 0, smoothing = 0, axis = 1, rows = 0;
		MathStructure *y_vector, *x_vector;
		gtk_tree_model_get(GTK_TREE_MODEL(tPlotFunctions_store), &iter, 0, &gstr1, 1, &gstr2, 2, &style, 3, &smoothing, 4, &type, 5, &axis, 6, &rows, 7, &x_vector, 8, &y_vector, -1);
		if(type == 1) {
			if(y_vector->isMatrix()) {
				count = 0;
				if(rows) {
					for(size_t i = 1; i <= y_vector->rows(); i++) {
						y_vectors.push_back(m_undefined);
						y_vector->rowToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						count++;
					}
				} else {
					for(size_t i = 1; i <= y_vector->columns(); i++) {
						y_vectors.push_back(m_undefined);
						y_vector->columnToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						count++;
					}
				}
			} else if(y_vector->isVector()) {
				y_vectors.push_back(*y_vector);
				x_vectors.push_back(m_undefined);
			} else {
				y_vectors.push_back(*y_vector);
				y_vectors[y_vectors.size() - 1].transform(STRUCT_VECTOR);
				x_vectors.push_back(m_undefined);
			}
		} else if(type == 2) {
			if(y_vector->isMatrix()) {
				count = 0;
				if(rows) {
					for(size_t i = 1; i <= y_vector->rows(); i += 2) {
						y_vectors.push_back(m_undefined);
						y_vector->rowToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						y_vector->rowToVector(i + 1, x_vectors[x_vectors.size() - 1]);
						count++;
					}
				} else {
					for(size_t i = 1; i <= y_vector->columns(); i += 2) {
						y_vectors.push_back(m_undefined);
						y_vector->columnToVector(i, y_vectors[y_vectors.size() - 1]);
						x_vectors.push_back(m_undefined);
						y_vector->columnToVector(i + 1, x_vectors[x_vectors.size() - 1]);
						count++;
					}
				}
			} else if(y_vector->isVector()) {
				y_vectors.push_back(*y_vector);
				x_vectors.push_back(m_undefined);
			} else {
				y_vectors.push_back(*y_vector);
				y_vectors[y_vectors.size() - 1].transform(STRUCT_VECTOR);
				x_vectors.push_back(m_undefined);
			}
		} else {
			y_vectors.push_back(*y_vector);
			x_vectors.push_back(*x_vector);
		}
		for(int i = 0; i < count; i++) {
			PlotDataParameters *pdp = new PlotDataParameters();
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
void on_plot_button_help_clicked(GtkButton, gpointer) {
#ifdef HAVE_LIBGNOME
	GError *error = NULL;
	gnome_help_display("qalculate-gtk", "qalculate-plotting", &error);
	if(error) {
		gchar *error_str = g_locale_to_utf8(error->message, -1, NULL, NULL, NULL);
		GtkWidget *d = gtk_message_dialog_new (GTK_WINDOW(glade_xml_get_widget (plot_glade, "plot_dialog")), (GtkDialogFlags) 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, _("Could not display help.\n%s"), error_str);
		gtk_dialog_run(GTK_DIALOG(d));
		gtk_widget_destroy(d);
		g_free(error_str);
		g_error_free(error);
	}
#endif	
}
void on_plot_button_save_clicked(GtkButton*, gpointer) {
	GtkWidget *d;
	d = gtk_file_chooser_dialog_new(_("Select file to export"), GTK_WINDOW(glade_xml_get_widget(plot_glade, "plot_dialog")), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION >= 8	
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(d), TRUE);
#endif	
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
		vector<MathStructure> y_vectors;
		vector<MathStructure> x_vectors;
		vector<PlotDataParameters*> pdps;
		PlotParameters pp;
		if(generate_plot(pp, y_vectors, x_vectors, pdps)) {
			pp.filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
			pp.filetype = PLOT_FILETYPE_AUTO;
			CALCULATOR->plotVectors(&pp, y_vectors, x_vectors, pdps);
			for(size_t i = 0; i < pdps.size(); i++) {
				if(pdps[i]) delete pdps[i];
			}
		}
	}
	gtk_widget_destroy(d);
}
void update_plot() {
	vector<MathStructure> y_vectors;
	vector<MathStructure> x_vectors;
	vector<PlotDataParameters*> pdps;
	PlotParameters pp;
	if(!generate_plot(pp, y_vectors, x_vectors, pdps)) {
		CALCULATOR->closeGnuplot();
		gtk_widget_set_sensitive(glade_xml_get_widget(plot_glade, "plot_button_save"), false);
		return;
	}
	CALCULATOR->plotVectors(&pp, y_vectors, x_vectors, pdps);
	gtk_widget_set_sensitive(glade_xml_get_widget(plot_glade, "plot_button_save"), true);
	for(size_t i = 0; i < pdps.size(); i++) {
		if(pdps[i]) delete pdps[i];
	}
}

void generate_plot_series(MathStructure **x_vector, MathStructure **y_vector, int type, string str, string str_x) {
	EvaluationOptions eo;
	eo.approximation = APPROXIMATION_APPROXIMATE;
	eo.parse_options = evalops.parse_options;
	eo.parse_options.read_precision = DONT_READ_PRECISION;
	if(type == 1 || type == 2) {
		*y_vector = new MathStructure(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str, evalops.parse_options), eo));
		*x_vector = NULL;
	} else {
		MathStructure min(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_min"))), evalops.parse_options), eo));
		MathStructure max(CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_max"))), evalops.parse_options), eo));
		*x_vector = new MathStructure();
		(*x_vector)->clearVector();
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_radiobutton_step")))) {
			*y_vector = new MathStructure(CALCULATOR->expressionToPlotVector(str, min, max, CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_step"))), evalops.parse_options), eo), *x_vector, str_x, evalops.parse_options));
		} else {
			*y_vector = new MathStructure(CALCULATOR->expressionToPlotVector(str, min, max, gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(glade_xml_get_widget (plot_glade, "plot_spinbutton_steps"))), *x_vector, str_x, evalops.parse_options));
		}
	}
}
void on_plot_button_add_clicked(GtkButton*, gpointer) {
	string expression = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")));
	if(expression.find_first_not_of(SPACES) == string::npos) {
		gtk_widget_grab_focus(glade_xml_get_widget (plot_glade, "plot_entry_expression"));
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
	string str_x = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")));
	remove_blank_ends(str_x);
	if(str_x.empty() && type == 0) {
		gtk_widget_grab_focus(glade_xml_get_widget (plot_glade, "plot_entry_variable"));
		show_message(_("Empty x variable."), glade_xml_get_widget(plot_glade, "plot_dialog"));
		return;
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
	MathStructure *x_vector, *y_vector;
	generate_plot_series(&x_vector, &y_vector, type, expression, str_x);
	rows = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")));
	GtkTreeIter iter;	
	gtk_list_store_append(tPlotFunctions_store, &iter);
	gtk_list_store_set(tPlotFunctions_store, &iter, 0, title.c_str(), 1, expression.c_str(), 2, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style"))), 3, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing"))), 4, type, 5, axis, 6, rows, 7, x_vector, 8, y_vector, 9, str_x.c_str(), -1);
	gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions)), &iter);
	update_plot();
}
void on_plot_button_modify_clicked(GtkButton*, gpointer) {

	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {	
		string expression = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")));
		if(expression.find_first_not_of(SPACES) == string::npos) {
			gtk_widget_grab_focus(glade_xml_get_widget (plot_glade, "plot_entry_expression"));
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
		string str_x = gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_variable")));
		remove_blank_ends(str_x);
		if(str_x.empty() && type == 0) {
			gtk_widget_grab_focus(glade_xml_get_widget (plot_glade, "plot_entry_variable"));
			show_message(_("Empty x variable."), glade_xml_get_widget(plot_glade, "plot_dialog"));
			return;
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
		MathStructure *x_vector, *y_vector;
		gtk_tree_model_get(GTK_TREE_MODEL(tPlotFunctions_store), &iter, 7, &x_vector, 8, &y_vector, -1);
		if(x_vector) delete x_vector;
		if(y_vector) delete y_vector;
		x_vector = NULL;
		y_vector = NULL;
		generate_plot_series(&x_vector, &y_vector, type, expression, str_x);
		rows = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows")));
		gtk_list_store_set(tPlotFunctions_store, &iter, 0, title.c_str(), 1, expression.c_str(), 2, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_style"))), 3, gtk_option_menu_get_history(GTK_OPTION_MENU(glade_xml_get_widget (plot_glade, "plot_optionmenu_smoothing"))), 4, type, 5, axis, 6, rows, 7, x_vector, 8, y_vector, 9, str_x.c_str(), -1);
		update_plot();
	}
}
void on_plot_button_remove_clicked(GtkButton*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		MathStructure *x_vector, *y_vector;
		gtk_tree_model_get(GTK_TREE_MODEL(tPlotFunctions_store), &iter, 7, &x_vector, 8, &y_vector, -1);
		if(x_vector) delete x_vector;
		if(y_vector) delete y_vector;
		gtk_list_store_remove(tPlotFunctions_store, &iter);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_expression")), "");
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget (plot_glade, "plot_entry_title")), "");	
		update_plot();
	}
}

void on_plot_checkbutton_xlog_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_spinbutton_xlog_base"), gtk_toggle_button_get_active(w));
}
void on_plot_checkbutton_ylog_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_spinbutton_ylog_base"), gtk_toggle_button_get_active(w));
}
void on_plot_radiobutton_step_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_entry_step"), gtk_toggle_button_get_active(w));
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_spinbutton_steps"), !gtk_toggle_button_get_active(w));
}
void on_plot_radiobutton_steps_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_entry_step"), !gtk_toggle_button_get_active(w));
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_spinbutton_steps"), gtk_toggle_button_get_active(w));
}
void on_plot_entry_expression_activate(GtkEntry*, gpointer) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions));
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		on_plot_button_modify_clicked(GTK_BUTTON(glade_xml_get_widget (plot_glade, "plot_button_modify")), NULL);
	} else {
		on_plot_button_add_clicked(GTK_BUTTON(glade_xml_get_widget (plot_glade, "plot_button_add")), NULL);
	}
}

void on_plot_radiobutton_function_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_box_variable"), gtk_toggle_button_get_active(w));
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows"), !gtk_toggle_button_get_active(w));
}
void on_plot_radiobutton_vector_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_box_variable"), !gtk_toggle_button_get_active(w));
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows"), gtk_toggle_button_get_active(w));
}
void on_plot_radiobutton_paired_toggled(GtkToggleButton *w, gpointer) {
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_box_variable"), !gtk_toggle_button_get_active(w));
	gtk_widget_set_sensitive(glade_xml_get_widget (plot_glade, "plot_checkbutton_rows"), gtk_toggle_button_get_active(w));
}
void on_plot_button_range_apply_clicked(GtkButton*, gpointer) {
	GtkTreeIter iter;
	bool b = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
	while(b) {
		gchar *gstr2, *gstr3;
		gint type = 0;
		MathStructure *y_vector, *x_vector;
		gtk_tree_model_get(GTK_TREE_MODEL(tPlotFunctions_store), &iter, 1, &gstr2, 4, &type, 7, &x_vector, 8, &y_vector, 9, &gstr3, -1);
		if(y_vector) delete y_vector;
		if(x_vector) delete x_vector;
		x_vector = NULL;
		y_vector = NULL;
		generate_plot_series(&x_vector, &y_vector, type, gstr2, gstr3);
		g_free(gstr2);
		g_free(gstr3);
		gtk_list_store_set(tPlotFunctions_store, &iter, 7, x_vector, 8, y_vector, -1);
		b = gtk_tree_model_iter_next(GTK_TREE_MODEL(tPlotFunctions_store), &iter);
	}
	update_plot();
}
void on_plot_button_appearance_apply_clicked(GtkButton*, gpointer) {
	update_plot();
}

void on_unit_dialog_button_ok_clicked(GtkButton*, gpointer) {
	do_timeout = false;
	mstruct->set(CALCULATOR->convert(*mstruct, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit"))), evalops));
	result_action_executed();
	do_timeout = true;
	gtk_widget_hide(glade_xml_get_widget (unit_glade, "unit_dialog"));
}
void on_unit_dialog_button_apply_clicked(GtkButton*, gpointer) {
	do_timeout = false;
	mstruct->set(CALCULATOR->convert(*mstruct, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit"))), evalops));
	result_action_executed();
	do_timeout = true;
}
void on_unit_dialog_entry_unit_activate(GtkEntry*, gpointer) {
	on_unit_dialog_button_ok_clicked(GTK_BUTTON(glade_xml_get_widget (unit_glade, "unit_dialog_button_ok")), NULL);
}


vector<GtkWidget*> ewindows;
vector<DataObject*> eobjects;

void on_element_button_function_clicked(GtkButton *w, gpointer user_data) {
	DataProperty *dp = (DataProperty*) user_data;
	DataSet *ds = NULL;
	DataObject *o = NULL;
	GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(w));
	for(size_t i = 0; i < ewindows.size(); i++) {
		if(ewindows[i] == win) {
			o = eobjects[i];
			break;
		}
	}
	if(dp) ds = dp->parentSet();
	if(ds && o) {
		string str = ds->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs, false, false, &can_display_unicode_string_function, (void*) expression).name;
		str += "(";
		str += o->getProperty(ds->getPrimaryKeyProperty());
		str += CALCULATOR->getComma();
		str += " ";
		str += dp->getName();
		str += ")";
		insert_text(str.c_str());
	}
}
void on_element_button_close_clicked(GtkButton *w, gpointer user_data) {
	GtkWidget *win = gtk_widget_get_toplevel(GTK_WIDGET(w));
	for(size_t i = 0; i < ewindows.size(); i++) {
		if(ewindows[i] == win) {
			ewindows.erase(ewindows.begin() + i);
			eobjects.erase(eobjects.begin() + i);
			break;
		}
	}
	gtk_widget_destroy((GtkWidget*) user_data);
}
void on_element_button_clicked(GtkButton*, gpointer user_data) {
	DataObject *e = (DataObject*) user_data;
	if(e) {
		DataSet *ds = e->parentSet();
		if(!ds) return;
		GtkWidget *dialog = gtk_dialog_new();
		ewindows.push_back(dialog);
		eobjects.push_back(e);
		GtkWidget *close_button = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
		g_signal_connect((gpointer) close_button, "clicked", G_CALLBACK(on_element_button_close_clicked), (gpointer) dialog);
		gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(glade_xml_get_widget(periodictable_glade, "periodic_dialog")));
		gtk_window_set_title(GTK_WINDOW(dialog), "Element Data");
		gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
		GtkWidget *vbox = gtk_vbox_new(FALSE, 15);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
		
		GtkWidget *vbox2 = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), vbox2, FALSE, TRUE, 0);
		
		DataProperty *p_number = ds->getProperty("number");
		DataProperty *p_symbol = ds->getProperty("symbol");
		DataProperty *p_class = ds->getProperty("class");
		DataProperty *p_name = ds->getProperty("name");

		GtkWidget *label;
		label = gtk_label_new(NULL);
		string str = "<span size=\"large\">"; str += e->getProperty(p_number); str += "</span>";
		gtk_label_set_markup(GTK_LABEL(label), str.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_box_pack_start(GTK_BOX(vbox2), label, FALSE, TRUE, 0);
		label = gtk_label_new(NULL);
		str = "<span size=\"xx-large\">"; str += e->getProperty(p_symbol); str += "</span>";
		gtk_label_set_markup(GTK_LABEL(label), str.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_box_pack_start(GTK_BOX(vbox2), label, FALSE, TRUE, 0);
		label = gtk_label_new(NULL);
		str = "<span size=\"x-large\">"; str += e->getProperty(p_name); str += "</span>  "; 
		gtk_label_set_markup(GTK_LABEL(label), str.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), TRUE);
		gtk_box_pack_start(GTK_BOX(vbox2), label, FALSE, TRUE, 0);

		GtkWidget *button;
		GtkWidget *ptable = gtk_table_new(0, 3, FALSE);
		gtk_table_set_col_spacing(GTK_TABLE(ptable), 0, 15);
		gtk_table_set_col_spacing(GTK_TABLE(ptable), 1, 6);
		gtk_box_pack_start(GTK_BOX(vbox), ptable, FALSE, TRUE, 0);
		int rows = 0;

		int group = s2i(e->getProperty(p_class));
		if(group > 0) {
			rows++;
			gtk_table_resize(GTK_TABLE(ptable), rows, 3);
			label = gtk_label_new(NULL);
			str = "<span weight=\"bold\">"; str += _("Classification"); str += ":"; str += "</span>";
			gtk_label_set_markup(GTK_LABEL(label), str.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), FALSE);
			gtk_table_attach_defaults(GTK_TABLE(ptable), label, 0, 1, rows - 1, rows);
			label = gtk_label_new(NULL);
			switch(group) {
				case ALKALI_METALS: {gtk_label_set_markup(GTK_LABEL(label), _("Alkali Metal")); break;}
				case ALKALI_EARTH_METALS: {gtk_label_set_markup(GTK_LABEL(label), _("Alkaline-Earth Metal")); break;}
				case LANTHANIDES: {gtk_label_set_markup(GTK_LABEL(label), _("Lanthanide")); break;}
				case ACTINIDES: {gtk_label_set_markup(GTK_LABEL(label), _("Actinide")); break;}
				case TRANSITION_METALS: {gtk_label_set_markup(GTK_LABEL(label), _("Transition Metal")); break;}
				case METALS: {gtk_label_set_markup(GTK_LABEL(label), _("Metal")); break;}
				case METALLOIDS: {gtk_label_set_markup(GTK_LABEL(label), _("Metalloid")); break;}
				case NONMETALS: {gtk_label_set_markup(GTK_LABEL(label), _("Non-Metal")); break;}
				case HALOGENS: {gtk_label_set_markup(GTK_LABEL(label), _("Halogen")); break;}
				case NOBLE_GASES: {gtk_label_set_markup(GTK_LABEL(label), _("Noble Gas")); break;}
				case TRANSACTINIDES: {gtk_label_set_markup(GTK_LABEL(label), _("Transactinide")); break;}
				default: {gtk_label_set_markup(GTK_LABEL(label), _("Unknown")); break;}
			}
			gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), TRUE);
			gtk_table_attach_defaults(GTK_TABLE(ptable), label, 1, 2, rows - 1, rows);
		}
		
		DataPropertyIter it;
		DataProperty *dp = ds->getFirstProperty(&it);
		string sval;
		while(dp) {
			if(!dp->isHidden() && dp != p_number && dp != p_class && dp != p_symbol && dp != p_name) {
				sval = e->getPropertyDisplayString(dp);
				if(!sval.empty()) {
					rows++;
					gtk_table_resize(GTK_TABLE(ptable), rows, 3);
					label = gtk_label_new(NULL);
					str = "<span weight=\"bold\">"; str += dp->title(); str += ":"; str += "</span>";
					gtk_label_set_markup(GTK_LABEL(label), str.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), FALSE);
					gtk_table_attach_defaults(GTK_TABLE(ptable), label, 0, 1, rows - 1, rows);
					label = gtk_label_new(NULL);
					gtk_label_set_markup(GTK_LABEL(label), sval.c_str()); gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5); gtk_label_set_selectable(GTK_LABEL(label), TRUE);
					gtk_table_attach_defaults(GTK_TABLE(ptable), label, 1, 2, rows - 1, rows);
					button = gtk_button_new();
					gtk_container_add(GTK_CONTAINER(button), gtk_image_new_from_stock("gtk-paste", GTK_ICON_SIZE_BUTTON));
					gtk_table_attach_defaults(GTK_TABLE(ptable), button, 2, 3, rows - 1, rows);
					g_signal_connect((gpointer) button, "clicked", G_CALLBACK(on_element_button_function_clicked), (gpointer) dp);
				}
			}
			dp = ds->getNextProperty(&it);
		}
		
		gtk_widget_show_all(dialog);
		
	}
}

void on_dataset_edit_entry_name_changed(GtkEditable *editable, gpointer) {
	if(!CALCULATOR->functionNameIsValid(gtk_entry_get_text(GTK_ENTRY(editable)))) {
		g_signal_handlers_block_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_dataset_edit_entry_name_changed, NULL);
		gtk_entry_set_text(GTK_ENTRY(editable), CALCULATOR->convertToValidFunctionName(gtk_entry_get_text(GTK_ENTRY(editable))).c_str());
		g_signal_handlers_unblock_matched((gpointer) editable, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_dataset_edit_entry_name_changed, NULL);		
	}
}
void on_dataset_edit_button_new_property_clicked(GtkButton*, gpointer) {
	DataProperty *dp = new DataProperty(edited_dataset);
	dp->setUserModified(true);
	if(edit_dataproperty(dp)) {
		tmp_props.push_back(dp);
		tmp_props_orig.push_back(NULL);
		update_dataset_property_list(edited_dataset);
	} else {
		delete dp;
	}
}
void on_dataset_edit_button_edit_property_clicked(GtkButton*, gpointer) {
	if(selected_dataproperty) {
		if(edit_dataproperty(selected_dataproperty)) {
			update_dataset_property_list(edited_dataset);
		}
	}
}
void on_dataset_edit_button_del_property_clicked(GtkButton*, gpointer) {
	if(edited_dataset && selected_dataproperty && selected_dataproperty->isUserModified()) {
		for(size_t i = 0; i < tmp_props.size(); i++) {
			if(tmp_props[i] == selected_dataproperty) {
				if(tmp_props_orig[i]) {
					tmp_props[i] = NULL;
				} else {
					tmp_props.erase(tmp_props.begin() + i);
					tmp_props_orig.erase(tmp_props_orig.begin() + i);
				}
				break;
			}
		}
		update_dataset_property_list(edited_dataset);
	}
}
void on_dataset_edit_button_names_clicked(GtkButton*, gpointer) {
	edit_names(get_edited_dataset(), gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataset_edit_entry_name"))), glade_xml_get_widget (datasetedit_glade, "dataset_edit_dialog"));
	SET_NAMES_LE(datasetedit_glade, "dataset_edit_entry_name", "dataset_edit_label_names")
}

void on_dataproperty_edit_button_names_clicked(GtkButton*, gpointer) {
	edit_names(NULL, gtk_entry_get_text(GTK_ENTRY(glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_entry_name"))), glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_dialog"), TRUE, get_edited_dataproperty());
	SET_NAMES_LE(datasetedit_glade, "dataproperty_edit_entry_name", "dataproperty_edit_label_names")
}

void on_menu_item_set_unknowns_activate(GtkMenuItem*, gpointer) {
	if(expression_has_changed && !rpn_mode) execute_expression(true);
	MathStructure unknowns;
	mstruct->findAllUnknowns(unknowns);
	if(unknowns.size() == 0) {
		show_message(_("No unknowns in result."), glade_xml_get_widget (main_glade, "main_window"));
		return;
	}
	GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Set Unknowns"), GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_APPLY, GTK_RESPONSE_APPLY, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), FALSE);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	GtkWidget *vbox = gtk_vbox_new(FALSE, 15);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), vbox);
	GtkWidget *label;
	vector<GtkWidget*> entry;
	entry.resize(unknowns.size(), NULL);
	GtkWidget *ptable = gtk_table_new(0, 2, FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(ptable), 6);
	gtk_table_set_row_spacings(GTK_TABLE(ptable), 6);
	gtk_box_pack_start(GTK_BOX(vbox), ptable, FALSE, TRUE, 0);
	int rows = 0;
	for(size_t i = 0; i < unknowns.size(); i++) {
		rows++;
		gtk_table_resize(GTK_TABLE(ptable), rows, 2);
		label = gtk_label_new(unknowns[i].print().c_str());
		gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
		gtk_table_attach(GTK_TABLE(ptable), label, 0, 1, rows - 1, rows, GTK_FILL, GTK_FILL, 0, 0);
		entry[i] = gtk_entry_new();
		gtk_table_attach(GTK_TABLE(ptable), entry[i], 1, 2, rows - 1, rows, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), GTK_FILL, 0, 0);
	}
	MathStructure msave(*mstruct);
	string result_save = result_text;
	gtk_widget_show_all(dialog);
	bool b_changed = false;
	vector<bool> unknown_changed;
	unknown_changed.resize(unknowns.size(), false);
	while(true) {
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));
		bool b = false;
		if(response == GTK_RESPONSE_ACCEPT || response == GTK_RESPONSE_APPLY) {
			string str, result_mod = "";
			for(size_t i = 0; i < unknowns.size(); i++) {
				str = gtk_entry_get_text(GTK_ENTRY(entry[i]));
				remove_blank_ends(str);
				if(unknown_changed[i] || !str.empty()) {
					if(!result_mod.empty()) {
						result_mod += CALCULATOR->getComma();
						result_mod += " ";
					}
					result_mod += unknowns[i].print().c_str();
					result_mod += "=";
					if(str.empty()) {
						result_mod += "?";
					} else {
						result_mod += str;
						mstruct->replace(unknowns[i], CALCULATOR->calculate(CALCULATOR->unlocalizeExpression(str, evalops.parse_options), evalops));
						b = true;
						unknown_changed[i] = true;
					}
				}
			}
			if(b) {
				b_changed = true;
				if(response == GTK_RESPONSE_ACCEPT && !rpn_mode) {
					MathStructure mp(*mstruct);
					printops.can_display_unicode_string_arg = historyview;
					mp.format(printops);
					g_signal_handlers_block_matched((gpointer) expression, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_expression_changed, NULL);
					str = mp.print(printops);
					printops.can_display_unicode_string_arg = NULL;
					gtk_entry_set_text(GTK_ENTRY(expression), str.c_str());
					//add_to_expression_history(str);
					expression_history_index = -1;
					g_signal_handlers_unblock_matched((gpointer) expression, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer) on_expression_changed, NULL);
					gtk_widget_grab_focus(expression);
					gtk_editable_select_region(GTK_EDITABLE(expression), 0, -1);
					expression_has_changed2 = true;
					display_parse_status();
				}
				mstruct->eval(evalops);
			}
			if(b_changed) {
				printops.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
				setResult(NULL, true, false, false, result_mod);
			}
			if(response == GTK_RESPONSE_ACCEPT) {
				break;
			}
			if(b) mstruct->set(msave);
			
		} else {
			if(b_changed) {
				string result_mod = "";
				mstruct->set(msave);
				for(size_t i = 0; i < unknowns.size(); i++) {
					if(unknown_changed[i]) {
						if(!result_mod.empty()) {
							result_mod += CALCULATOR->getComma();
							result_mod += " ";
						}
						result_mod += unknowns[i].print().c_str();
						result_mod += "=";
						result_mod += "?";
					}
				}
				printops.allow_factorization = (evalops.structuring == STRUCTURING_FACTORIZE);
				setResult(NULL, true, false, false, result_mod);
			}
			break;
		}
	}
	gtk_widget_destroy(dialog);
}



#ifdef __cplusplus
}
#endif
