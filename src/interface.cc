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

#include <deque>

extern GladeXML *main_glade, *about_glade, *argumentrules_glade, *csvimport_glade, *csvexport_glade, *datasetedit_glade, *datasets_glade, *setbase_glade, *decimals_glade;
extern GladeXML *functionedit_glade, *functions_glade, *matrixedit_glade, *namesedit_glade, *nbases_glade, *plot_glade, *precision_glade;
extern GladeXML *preferences_glade, *unit_glade, *unitedit_glade, *units_glade, *unknownedit_glade, *variableedit_glade, *variables_glade;
extern GladeXML *periodictable_glade;
extern vector<mode_struct> modes;

GtkWidget *tFunctionCategories;
GtkWidget *tFunctions;
GtkListStore *tFunctions_store;
GtkTreeStore *tFunctionCategories_store;

GtkWidget *tVariableCategories;
GtkWidget *tVariables;
GtkListStore *tVariables_store;
GtkTreeStore *tVariableCategories_store;

GtkWidget *tUnitCategories;
GtkWidget *tUnits;
GtkListStore *tUnits_store;
GtkTreeStore *tUnitCategories_store;

GtkWidget *tUnitSelectorCategories;
GtkWidget *tUnitSelector;
GtkListStore *tUnitSelector_store;
GtkTreeStore *tUnitSelectorCategories_store;

GtkWidget *tDatasets;
GtkWidget *tDataObjects;
GtkListStore *tDatasets_store;
GtkListStore *tDataObjects_store;

GtkWidget *tDataProperties;
GtkListStore *tDataProperties_store;

GtkWidget *tNames;
GtkListStore *tNames_store;

GtkWidget *tabs, *expander_keypad, *expander_history;
GtkEntryCompletion *completion;
GtkListStore *completion_store;

GtkWidget *tFunctionArguments;
GtkListStore *tFunctionArguments_store;
GtkWidget *tSubfunctions;
GtkListStore *tSubfunctions_store;

GtkWidget *tPlotFunctions;
GtkListStore *tPlotFunctions_store;

GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeSelection *selection;

GtkWidget *expression;
GtkWidget *resultview;
GtkWidget *historyview;
GtkWidget *statuslabel_l, *statuslabel_r;
GtkWidget *f_menu ,*v_menu, *u_menu, *u_menu2, *recent_menu;
GtkAccelGroup *accel_group;

extern bool show_buttons, show_history;
extern bool save_mode_on_exit, save_defs_on_exit, load_global_defs, hyp_is_on, inv_is_on, fetch_exchange_rates_at_startup;
extern bool display_expression_status, enable_completion;
extern bool use_custom_result_font, use_custom_expression_font, use_custom_status_font;
extern string custom_result_font, custom_expression_font, custom_status_font, wget_args;
extern string status_error_color, status_warning_color;

extern PrintOptions printops;
extern EvaluationOptions evalops;

extern vector<vector<GtkWidget*> > element_entries;

GtkTooltips *periodic_tooltips;

extern GdkPixbuf *icon_pixbuf;

extern vector<GtkWidget*> mode_items;
extern vector<GtkWidget*> popup_result_mode_items;

extern deque<string> inhistory;
extern deque<int> inhistory_type;

gint win_height, win_width;

gint compare_categories(gconstpointer a, gconstpointer b) {
	return strcasecmp((const char*) a, (const char*) b);
}

void set_assumptions_items(AssumptionType at, AssumptionSign as) {	
	switch(as) {
		case ASSUMPTION_SIGN_POSITIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_positive")), TRUE); break;}
		case ASSUMPTION_SIGN_NONPOSITIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_nonpositive")), TRUE); break;}
		case ASSUMPTION_SIGN_NEGATIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_negative")), TRUE); break;}
		case ASSUMPTION_SIGN_NONNEGATIVE: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_nonnegative")), TRUE); break;}
		case ASSUMPTION_SIGN_NONZERO: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_nonzero")), TRUE); break;}
		default: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_unknown")), TRUE);}
	}
	switch(at) {
		case ASSUMPTION_TYPE_INTEGER: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_integer")), TRUE); break;}
		case ASSUMPTION_TYPE_RATIONAL: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_rational")), TRUE); break;}
		case ASSUMPTION_TYPE_REAL: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_real")), TRUE); break;}
		case ASSUMPTION_TYPE_COMPLEX: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_complex")), TRUE); break;}
		case ASSUMPTION_TYPE_NUMBER: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_number")), TRUE); break;}
		case ASSUMPTION_TYPE_NONMATRIX: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_nonmatrix")), TRUE); break;}
		default: {gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assumptions_none")), TRUE);}
	}
}

void set_mode_items(const PrintOptions &po, const EvaluationOptions &eo, AssumptionType at, AssumptionSign as, int precision, bool initial_update) {	

	switch(eo.approximation) {
		case APPROXIMATION_EXACT: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_always_exact")), TRUE);
			break;
		}
		case APPROXIMATION_TRY_EXACT: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_try_exact")), TRUE);
			if(initial_update) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_exact")), FALSE);
			break;
		}
		case APPROXIMATION_APPROXIMATE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_approximate")), TRUE);
			if(initial_update) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_exact")), FALSE);
			break;
		}
	}
	if(initial_update) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (main_glade, "button_exact")), eo.approximation == APPROXIMATION_EXACT);
	
	switch(eo.auto_post_conversion) {
		case POST_CONVERSION_BEST: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_post_conversion_best")), TRUE);
			break;
		}
		case POST_CONVERSION_BASE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_post_conversion_base")), TRUE);
			break;
		}
		default: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_post_conversion_none")), TRUE);
			break;
		}
	}

	switch(eo.parse_options.angle_unit) {
		case ANGLE_UNIT_DEGREES: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_degrees")), TRUE);
			if(initial_update) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "radiobutton_degrees")), TRUE);
			break;
		}
		case ANGLE_UNIT_RADIANS: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_radians")), TRUE);
			if(initial_update) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "radiobutton_radians")), TRUE);
			break;
		}
		case ANGLE_UNIT_GRADIANS: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_gradians")), TRUE);
			if(initial_update) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "radiobutton_gradians")), TRUE);
			break;
		}
		default: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_no_default_angle_unit")), TRUE);
			if(initial_update) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "radiobutton_no_default_angle_unit")), TRUE);
			break;
		}
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_read_precision")), eo.parse_options.read_precision != DONT_READ_PRECISION);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_rpn_mode")), eo.parse_options.rpn);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_limit_implicit_multiplication")), eo.parse_options.limit_implicit_multiplication);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_assume_nonzero_denominators")), eo.assume_denominators_nonzero);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_warn_about_denominators_assumed_nonzero")), eo.warn_about_denominators_assumed_nonzero);
	
	switch(eo.structuring) {
		case STRUCTURING_SIMPLIFY: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_algebraic_mode_simplify")), TRUE);
			break;
		}
		case STRUCTURING_FACTORIZE: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_algebraic_mode_factorize")), TRUE);
			break;
		}
		default: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_algebraic_mode_none")), TRUE);
			break;
		}
	}

	switch(po.base) {
		case BASE_BINARY: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 0);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_binary")), TRUE);
			break;
		}
		case BASE_OCTAL: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 1);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_octal")), TRUE);
			break;
		}
		case BASE_DECIMAL: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 2);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_decimal")), TRUE);
			break;
		}
		case BASE_HEXADECIMAL: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 3);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_hexadecimal")), TRUE);
			break;
		}
		case BASE_ROMAN_NUMERALS: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 6);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_roman")), TRUE);
			break;
		}
		case BASE_SEXAGESIMAL: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 4);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_sexagesimal")), TRUE);
			break;
		}
		case BASE_TIME: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 5);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_time_format")), TRUE);
			break;
		}
		default: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_base")), 7);
			if(initial_update) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_custom_base")), TRUE);			
			printops.base = po.base;
			output_base_updated_from_menu();
		}
	}
	
	switch(po.min_exp) {
		case EXP_PRECISION: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 0);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_normal")), TRUE);
			break;
		}
		case EXP_SCIENTIFIC: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 1);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_scientific")), TRUE);
			break;
		}
		case EXP_PURE: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 2);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_purely_scientific")), TRUE);
			break;
		}
		case EXP_NONE: {
			if(initial_update) gtk_combo_box_set_active(GTK_COMBO_BOX(glade_xml_get_widget (main_glade, "combobox_numerical_display")), 3);
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_non_scientific")), TRUE);
			break;
		}
	}

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_indicate_infinite_series")), po.indicate_infinite_series);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_show_ending_zeroes")), po.show_ending_zeroes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_round_halfway_to_even")), po.round_halfway_to_even);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_display_prefixes")), po.use_unit_prefixes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_all_prefixes")), po.use_all_prefixes);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_denominator_prefixes")), po.use_denominator_prefix);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_place_units_separately")), po.place_units_separately);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_abbreviate_names")), po.abbreviate_names);
			
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_variables")), eo.parse_options.variables_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_functions")), eo.parse_options.functions_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_units")), eo.parse_options.units_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_enable_unknown_variables")), eo.parse_options.unknowns_enabled);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_calculate_variables")), eo.calculate_variables);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_allow_complex")), eo.allow_complex);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_allow_infinite")), eo.allow_infinite);

	switch (po.number_fraction_format) {
		case FRACTION_DECIMAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_decimal")), TRUE);
			break;
		}
		case FRACTION_DECIMAL_EXACT: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_decimal_exact")), TRUE);
			break;
		}
		case FRACTION_COMBINED: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_combined")), TRUE);
			break;		
		}
		case FRACTION_FRACTIONAL: {
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(glade_xml_get_widget (main_glade, "menu_item_fraction_fraction")), TRUE);
			break;		
		}
	}
	if(initial_update) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "button_fraction")), po.number_fraction_format == FRACTION_FRACTIONAL);

	set_assumptions_items(at, as);
	
	if(!initial_update) {
		if(decimals_glade) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_min")), po.use_min_decimals);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (decimals_glade, "decimals_dialog_checkbutton_max")), po.use_max_decimals);	
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_min")), po.min_decimals);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (decimals_glade, "decimals_dialog_spinbutton_max")), po.max_decimals);
		} else {
			printops.max_decimals = po.max_decimals;
			printops.use_max_decimals = po.use_max_decimals;
			printops.max_decimals = po.min_decimals;
			printops.use_min_decimals = po.use_min_decimals;
		}
		if(precision_glade) {
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (precision_glade, "precision_dialog_spinbutton_precision")), precision);	
		} else {
			CALCULATOR->setPrecision(precision);
		}
		printops.spacious = po.spacious;
		printops.short_multiplication = po.short_multiplication;
		printops.excessive_parenthesis = po.excessive_parenthesis;
		evalops.calculate_functions = eo.calculate_functions;
		if(setbase_glade) {
			switch(eo.parse_options.base) {
				case BASE_BINARY: {
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_binary")), TRUE);
					break;
				}
				case BASE_OCTAL: {
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_octal")), TRUE);
					break;
				}
				case BASE_DECIMAL: {
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_decimal")), TRUE);
					break;
				}
				case BASE_HEXADECIMAL: {
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_hexadecimal")), TRUE);
					break;
				}
				case BASE_ROMAN_NUMERALS: {
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_roman")), TRUE);
					break;
				}
				default: {
					gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_other")), TRUE);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other")), eo.parse_options.base);
				}
			}
		} else {
			evalops.parse_options.base = eo.parse_options.base;
		}
	}

}

void
create_main_window (void)
{
	
	gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "main.glade", NULL);
	main_glade = glade_xml_new(gstr, NULL, NULL);
	g_assert(main_glade != NULL);
	g_free(gstr);
	
	/* make sure we get a valid main window */
	g_assert (glade_xml_get_widget (main_glade, "main_window") != NULL);

	accel_group = gtk_accel_group_new();
	gtk_window_add_accel_group (GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), accel_group);
	
	if(win_width > 0) gtk_window_set_default_size (GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), win_width, win_height);

	expression = glade_xml_get_widget (main_glade, "expression");
	resultview = glade_xml_get_widget (main_glade, "resultview");
	historyview = glade_xml_get_widget (main_glade, "history");
	statuslabel_l = glade_xml_get_widget (main_glade, "label_status_left");
	statuslabel_r = glade_xml_get_widget (main_glade, "label_status_right");
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "history_parse", "foreground", "gray40", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "history_transformation", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "history_error", "foreground", "red", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "history_warning", "foreground", "blue", NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "history_result", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (main_glade, "history"))), "history_separator", "size-points", 6.0, NULL);

	gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (main_glade, "button_xy")))), TRUE);
	//gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (main_glade, "button_fraction")))), TRUE);
	gtk_label_set_use_markup(GTK_LABEL (gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (main_glade, "button_square")))), TRUE);
	gtk_label_set_use_markup(GTK_LABEL(gtk_bin_get_child (GTK_BIN(glade_xml_get_widget (main_glade, "button_functions")))), TRUE);
			
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "button_hyp")), hyp_is_on);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (glade_xml_get_widget (main_glade, "button_inv")), inv_is_on);
	
	set_mode_items(printops, evalops, CALCULATOR->defaultAssumptions()->type(), CALCULATOR->defaultAssumptions()->sign(), CALCULATOR->getPrecision(), true);

	set_unicode_buttons();

	if(use_custom_result_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_result_font.c_str());
		gtk_widget_modify_font(resultview, font);
		pango_font_description_free(font);
	} else {
		if(custom_result_font.empty()) {
			custom_result_font = pango_font_description_to_string(resultview->style->font_desc);
		}		
	}
	if(use_custom_expression_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_expression_font.c_str());
		gtk_widget_modify_font(expression, font);
		pango_font_description_free(font);
	} else {
		if(custom_expression_font.empty()) {
			custom_expression_font = pango_font_description_to_string(expression->style->font_desc);
		}		
	}
	if(use_custom_status_font) {
		PangoFontDescription *font = pango_font_description_from_string(custom_status_font.c_str());
		gtk_widget_modify_font(statuslabel_l, font);
		gtk_widget_modify_font(statuslabel_r, font);
		pango_font_description_free(font);
	} else {
		if(custom_status_font.empty()) {
			custom_status_font = pango_font_description_to_string(statuslabel_l->style->font_desc);
		}		
	}
	
	gtk_widget_grab_focus(expression);
	GTK_WIDGET_SET_FLAGS(expression, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(expression);

	expander_keypad = glade_xml_get_widget(main_glade, "expander_keypad");
	expander_history = glade_xml_get_widget(main_glade, "expander_history");
	gtk_expander_set_expanded(GTK_EXPANDER(expander_keypad), show_buttons);
	gtk_expander_set_expanded(GTK_EXPANDER(expander_history), show_history && !show_buttons);
	tabs = glade_xml_get_widget(main_glade, "tabs");
	if(show_history) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), 1);
	} else {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tabs), 0);
	}
	if(show_history || show_buttons) {
		gtk_widget_show(tabs);
	} else {
		gtk_widget_hide(tabs);
	}

	glade_xml_signal_autoconnect(main_glade);
	g_signal_connect(accel_group, "accel_changed", G_CALLBACK(save_accels), NULL);

	GtkTextIter iter;
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(historyview));
	bool prev_parse = false;
	for(size_t i = 0; i < inhistory.size(); i++) {
		gtk_text_buffer_get_end_iter(tb, &iter);
		switch(inhistory_type[i]) {
			case QALCULATE_HISTORY_EXPRESSION: {
				if(i != 0) {
					gtk_text_buffer_insert_with_tags_by_name(tb, &iter, "\n", -1, "history_separator", NULL);
				}
				gtk_text_buffer_insert(tb, &iter, inhistory[i].c_str(), -1);
				gtk_text_buffer_insert(tb, &iter, " ", -1);
				prev_parse = false;
				break;
			}
			case QALCULATE_HISTORY_TRANSFORMATION: {
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, inhistory[i].c_str(), -1, "history_transformation", NULL);
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, ":  ", -1, "history_transformation", NULL);
				break;
			}
			case QALCULATE_HISTORY_RESULT: {
				gtk_text_buffer_insert(tb, &iter, "= ", -1);
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, inhistory[i].c_str(), -1, "history_result", NULL);
				gtk_text_buffer_insert(tb, &iter, "\n", -1);
				prev_parse = false;
				break;
			}
			case QALCULATE_HISTORY_RESULT_APPROXIMATE: {
				string str;
				if(printops.use_unicode_signs && can_display_unicode_string_function(SIGN_ALMOST_EQUAL, (void*) historyview)) {
					str = SIGN_ALMOST_EQUAL " ";
				} else {
					str = "= ";
					str += _("approx.");
					str += " ";
				}
				gtk_text_buffer_insert(tb, &iter, str.c_str(), -1);
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, inhistory[i].c_str(), -1, "history_result", NULL);
				gtk_text_buffer_insert(tb, &iter, "\n", -1);
				prev_parse = false;
				break;
			}
			case QALCULATE_HISTORY_PARSE: {
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, " ", -1, "history_parse", NULL);
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, inhistory[i].c_str(), -1, "history_parse", NULL);
				gtk_text_buffer_insert(tb, &iter, "\n", -1);
				prev_parse = true;
				break;
			}
			case QALCULATE_HISTORY_WARNING: {
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, "- ", -1, "history_warning", NULL);
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, inhistory[i].c_str(), -1, "history_warning", NULL);
				gtk_text_buffer_insert(tb, &iter, "\n", -1);
				break;
			}
			case QALCULATE_HISTORY_ERROR: {
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, "- ", -1, "history_error", NULL);
				gtk_text_buffer_insert_with_tags_by_name(tb, &iter, inhistory[i].c_str(), -1, "history_error", NULL);
				gtk_text_buffer_insert(tb, &iter, "\n", -1);
				break;
			}
			case QALCULATE_HISTORY_OLD: {
				gtk_text_buffer_insert(tb, &iter, inhistory[i].c_str(), -1);
				gtk_text_buffer_insert(tb, &iter, "\n", -1);
				prev_parse = false;
				break;
			}
		}
		
	}

#ifndef HAVE_LIBGNOME
	gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_help"), FALSE);
#endif

	gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_save_image"), FALSE);
	gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "popup_menu_item_save_image"), FALSE);

/*	Completion	*/	
	completion = gtk_entry_completion_new();
	gtk_entry_set_completion(GTK_ENTRY(expression), completion);
	completion_store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
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
	
	for(size_t i = 0; i < modes.size(); i++) {
		GtkWidget *item = gtk_menu_item_new_with_label(modes[i].name.c_str()); 
		gtk_widget_show(item); 
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(on_menu_item_meta_mode_activate), (gpointer) modes[i].name.c_str()); 
		gtk_menu_shell_insert(GTK_MENU_SHELL(glade_xml_get_widget (main_glade, "menu_meta_modes")), item, (gint) i);
		mode_items.push_back(item);
		item = gtk_menu_item_new_with_label(modes[i].name.c_str()); 
		gtk_widget_show(item); 
		gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(on_menu_item_meta_mode_activate), (gpointer) modes[i].name.c_str()); 
		gtk_menu_shell_insert(GTK_MENU_SHELL(glade_xml_get_widget (main_glade, "menu_result_popup_meta_modes")), item, (gint) i);
		popup_result_mode_items.push_back(item);
	}
	gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_meta_mode_delete"), modes.size() > 2);
	gtk_widget_set_sensitive(glade_xml_get_widget(main_glade, "menu_item_result_popup_meta_mode_delete"), modes.size() > 2);
	
	if(win_width > 0) {
		if(show_history || show_buttons) gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), 1, win_height);
		else gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), win_width, win_height);
	}

	gtk_widget_show (glade_xml_get_widget (main_glade, "main_window"));
	
	set_result_size_request();
	
	if(show_history || show_buttons) gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), 1, 1);
	else gtk_window_resize(GTK_WINDOW(glade_xml_get_widget(main_glade, "main_window")), win_width, 1);

	gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (main_glade, "main_window")), icon_pixbuf);

	update_status_text();
	
}

GtkWidget*
get_functions_dialog (void)
{

	if(!functions_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "functions.glade", NULL);
		functions_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(functions_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (functions_glade, "functions_dialog") != NULL);
	
		tFunctionCategories = glade_xml_get_widget (functions_glade, "functions_treeview_category");
		tFunctions	= glade_xml_get_widget (functions_glade, "functions_treeview_function");

		tFunctions_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
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

		tFunctionCategories_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
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

		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (functions_glade, "functions_textview_description")));
		gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
		gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);

		glade_xml_signal_autoconnect(functions_glade);

		update_functions_tree();
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (functions_glade, "functions_dialog")), icon_pixbuf);
	}

	return glade_xml_get_widget (functions_glade, "functions_dialog");
}

GtkWidget*
get_variables_dialog (void)
{
	if(!variables_glade) {

		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "variables.glade", NULL);
		variables_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(variables_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (variables_glade, "variables_dialog") != NULL);

		tVariableCategories = glade_xml_get_widget (variables_glade, "variables_treeview_category");
		tVariables = glade_xml_get_widget (variables_glade, "variables_treeview_variable");

		tVariables_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
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

		tVariableCategories_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
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

		glade_xml_signal_autoconnect(variables_glade);

		update_variables_tree();
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (variables_glade, "variables_dialog")), icon_pixbuf);

	}
	
	return glade_xml_get_widget (variables_glade, "variables_dialog");
}

GtkWidget*
get_units_dialog (void)
{

	if(!units_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "units.glade", NULL);
		units_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(units_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (units_glade, "units_dialog") != NULL);
	
		tUnitCategories = glade_xml_get_widget (units_glade, "units_treeview_category");
		tUnits		= glade_xml_get_widget (units_glade, "units_treeview_unit");

		tUnits_store = gtk_list_store_new(UNITS_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
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
		column = gtk_tree_view_column_new_with_attributes(_("Unit"), renderer, "text", UNITS_BASE_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, UNITS_BASE_COLUMN);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tUnits), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tUnits_selection_changed), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_TITLE_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_TITLE_COLUMN), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_NAMES_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_NAMES_COLUMN), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnits_store), UNITS_BASE_COLUMN, string_sort_func, GINT_TO_POINTER(UNITS_BASE_COLUMN), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tUnits_store), UNITS_TITLE_COLUMN, GTK_SORT_ASCENDING);

		gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tUnits), TRUE);

		tUnitCategories_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
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

		glade_xml_signal_autoconnect(units_glade);

		update_units_tree();
	
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (units_glade, "units_entry_from_val")), "1");	
		gtk_entry_set_text (GTK_ENTRY (glade_xml_get_widget (units_glade, "units_entry_to_val")), "1");	
		gtk_entry_set_alignment(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_from_val")), 1.0);
		gtk_entry_set_alignment(GTK_ENTRY(glade_xml_get_widget (units_glade, "units_entry_to_val")), 1.0);
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (units_glade, "units_dialog")), icon_pixbuf);
	
	}
	
	return glade_xml_get_widget (units_glade, "units_dialog");
}

GtkWidget*
get_datasets_dialog (void)
{

	if(!datasets_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "datasets.glade", NULL);
		datasets_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(datasets_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (datasets_glade, "datasets_dialog") != NULL);
	
		tDatasets = glade_xml_get_widget (datasets_glade, "datasets_treeview_datasets");
		tDataObjects = glade_xml_get_widget (datasets_glade, "datasets_treeview_objects");

		tDataObjects_store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tDataObjects), GTK_TREE_MODEL(tDataObjects_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tDataObjects));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("Key 1", renderer, "text", 0, NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tDataObjects), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("Key 2", renderer, "text", 1, NULL);
		gtk_tree_view_column_set_sort_column_id(column, 1);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tDataObjects), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("Key 3", renderer, "text", 2, NULL);
		gtk_tree_view_column_set_sort_column_id(column, 2);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tDataObjects), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tDataObjects_selection_changed), NULL);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tDataObjects_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tDataObjects_store), 0, GTK_SORT_ASCENDING);

		tDatasets_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tDatasets), GTK_TREE_MODEL(tDatasets_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tDatasets));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Data Set"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tDatasets), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tDatasets_selection_changed), NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tDatasets_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tDatasets_store), 0, GTK_SORT_ASCENDING);

		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(glade_xml_get_widget (datasets_glade, "datasets_textview_description")));
		gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
		gtk_text_buffer_create_tag(buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);

		glade_xml_signal_autoconnect(datasets_glade);

		update_datasets_tree();
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (datasets_glade, "datasets_dialog")), icon_pixbuf);
		
	}

	return glade_xml_get_widget (datasets_glade, "datasets_dialog");
}


GtkWidget*
get_preferences_dialog (void)
{
	if(!preferences_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "preferences.glade", NULL);
		preferences_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(preferences_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (preferences_glade, "preferences_dialog") != NULL);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_display_expression_status")), display_expression_status);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_fetch_exchange_rates")), fetch_exchange_rates_at_startup);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_save_mode")), save_mode_on_exit);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_unicode_signs")), printops.use_unicode_signs);	
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_lower_case_numbers")), printops.lower_case_numbers);	
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_lower_case_e")), printops.lower_case_e);	
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_spell_out_logical_operators")), printops.spell_out_logical_operators);	
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_save_defs")), save_defs_on_exit);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_custom_result_font")), use_custom_result_font);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_custom_expression_font")), use_custom_expression_font);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_checkbutton_custom_status_font")), use_custom_status_font);
		gtk_entry_set_text(GTK_ENTRY(glade_xml_get_widget(preferences_glade, "preferences_entry_wget_args")), wget_args.c_str());	
		if(CALCULATOR->hasGnomeVFS()) {
			gtk_widget_hide(glade_xml_get_widget (preferences_glade, "preferences_box_wget_args"));
		}
		gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_result_font"), use_custom_result_font);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_button_result_font")), custom_result_font.c_str());
		gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_expression_font"), use_custom_expression_font);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_button_expression_font")), custom_expression_font.c_str());
		gtk_widget_set_sensitive(glade_xml_get_widget(preferences_glade, "preferences_button_status_font"), use_custom_status_font);	
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_button_status_font")), custom_status_font.c_str());
		GdkColor c;
		gdk_color_parse(status_error_color.c_str(), &c);
		gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget (preferences_glade, "colorbutton_status_error_color")), &c);
		gdk_color_parse(status_warning_color.c_str(), &c);
		gtk_color_button_set_color(GTK_COLOR_BUTTON(glade_xml_get_widget (preferences_glade, "colorbutton_status_warning_color")), &c);
		if(can_display_unicode_string_function(SIGN_MULTIDOT, (void*) glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot"))) gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot")), SIGN_MULTIDOT);
		else gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot")), SIGN_SMALLCIRCLE);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_ex")), SIGN_MULTIPLICATION);
		switch(printops.multiplication_sign) {
			case MULTIPLICATION_SIGN_DOT: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot")), TRUE);
				break;
			}
			case MULTIPLICATION_SIGN_X: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_ex")), TRUE);
				break;
			}
			default: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_asterisk")), TRUE);
				break;
			}
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_asterisk"), printops.use_unicode_signs);
		gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_ex"), printops.use_unicode_signs);
		gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_dot"), printops.use_unicode_signs);
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division_slash")), " " SIGN_DIVISION_SLASH " ");
		gtk_button_set_label(GTK_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division")), SIGN_DIVISION);
		switch(printops.division_sign) {
			case DIVISION_SIGN_DIVISION_SLASH: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division_slash")), TRUE);
				break;
			}
			case DIVISION_SIGN_DIVISION: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division")), TRUE);
				break;
			}
			default: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_slash")), TRUE);
				break;
			}
		}
		gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_slash"), printops.use_unicode_signs);
		gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division_slash"), printops.use_unicode_signs);
		gtk_widget_set_sensitive(glade_xml_get_widget (preferences_glade, "preferences_radiobutton_division"), printops.use_unicode_signs);
		glade_xml_signal_autoconnect(preferences_glade);
		
	}

	return glade_xml_get_widget (preferences_glade, "preferences_dialog");
}

GtkWidget*
get_unit_edit_dialog (void)
{

	if(!unitedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "unitedit.glade", NULL);
		unitedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(unitedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (unitedit_glade, "unit_edit_dialog") != NULL);
		
#ifndef HAVE_LIBGNOME
		gtk_widget_hide(glade_xml_get_widget (unitedit_glade, "unit_edit_button_help"));
#endif				
		
		glade_xml_signal_autoconnect(unitedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(size_t i = 0; i < CALCULATOR->units.size(); i++) {
		if(!CALCULATOR->units[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->units[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->units[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->units[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (unitedit_glade, "unit_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	return glade_xml_get_widget (unitedit_glade, "unit_edit_dialog");
}

GtkWidget*
get_function_edit_dialog (void)
{

	if(!functionedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "functionedit.glade", NULL);
		functionedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(functionedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (functionedit_glade, "function_edit_dialog") != NULL);
		
		tFunctionArguments = glade_xml_get_widget (functionedit_glade, "function_edit_treeview_arguments");
		tFunctionArguments_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tFunctionArguments), GTK_TREE_MODEL(tFunctionArguments_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tFunctionArguments));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tFunctionArguments), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Type"), renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tFunctionArguments), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tFunctionArguments_selection_changed), NULL);
		
		tSubfunctions = glade_xml_get_widget (functionedit_glade, "function_edit_treeview_subfunctions");
		tSubfunctions_store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_BOOLEAN);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tSubfunctions), GTK_TREE_MODEL(tSubfunctions_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tSubfunctions));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Reference"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tSubfunctions), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Expression"), renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tSubfunctions), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Precalculate"), renderer, "text", 2, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tSubfunctions), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tSubfunctions_selection_changed), NULL);
		
#ifndef HAVE_LIBGNOME
		gtk_widget_hide(glade_xml_get_widget (functionedit_glade, "function_edit_button_help"));
#endif		
		
		glade_xml_signal_autoconnect(functionedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(size_t i = 0; i < CALCULATOR->functions.size(); i++) {
		if(!CALCULATOR->functions[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->functions[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->functions[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->functions[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (functionedit_glade, "function_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);
	
	return glade_xml_get_widget (functionedit_glade, "function_edit_dialog");
}
GtkWidget*
get_variable_edit_dialog (void)
{
	
	if(!variableedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "variableedit.glade", NULL);
		variableedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(variableedit_glade != NULL);
		g_free(gstr);
			
		g_assert (glade_xml_get_widget (variableedit_glade, "variable_edit_dialog") != NULL);
		
#ifndef HAVE_LIBGNOME
		gtk_widget_hide(glade_xml_get_widget (variableedit_glade, "variable_edit_button_help"));
#endif		
		
		glade_xml_signal_autoconnect(variableedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->variables[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (variableedit_glade, "variable_edit_combo_category")), items);
	g_hash_table_destroy(hash);
	g_list_free(items);

	return glade_xml_get_widget (variableedit_glade, "variable_edit_dialog");
}

GtkWidget*
get_unknown_edit_dialog (void)
{
	
	if(!unknownedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "unknownedit.glade", NULL);
		unknownedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(unknownedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (unknownedit_glade, "unknown_edit_dialog") != NULL);
		
#ifndef HAVE_LIBGNOME
		gtk_widget_hide(glade_xml_get_widget (unknownedit_glade, "unknown_edit_button_help"));
#endif		
		
		glade_xml_signal_autoconnect(unknownedit_glade);
	
	}
	
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->variables[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (unknownedit_glade, "unknown_edit_combo_category")), items);
	g_hash_table_destroy(hash);
	g_list_free(items);

	return glade_xml_get_widget (unknownedit_glade, "unknown_edit_dialog");
}

GtkWidget*
get_matrix_edit_dialog (void)
{
	if(!matrixedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "matrixedit.glade", NULL);
		matrixedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(matrixedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (matrixedit_glade, "matrix_edit_dialog") != NULL);
		
#ifndef HAVE_LIBGNOME
		gtk_widget_hide(glade_xml_get_widget (matrixedit_glade, "matrix_edit_button_help"));
#endif		
		
		glade_xml_signal_autoconnect(matrixedit_glade);

		if(element_entries.size() == 0) {
			element_entries.resize(1);
			element_entries[0].push_back(glade_xml_get_widget (matrixedit_glade, "matrix_edit_entry_1x1"));
		}
	
	}

	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_insert_sorted(items, (gpointer) CALCULATOR->variables[i]->category().c_str(), (GCompareFunc) compare_categories);
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (matrixedit_glade, "matrix_edit_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);


	return glade_xml_get_widget (matrixedit_glade, "matrix_edit_dialog");
}
GtkWidget*
get_dataobject_edit_dialog (void)
{

	return glade_xml_get_widget (datasets_glade, "dataobject_edit_dialog");
}

GtkWidget*
get_dataset_edit_dialog (void)
{

	if(!datasetedit_glade) {

		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "datasetedit.glade", NULL);
		datasetedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(datasetedit_glade != NULL);
		g_free(gstr);

		g_assert (glade_xml_get_widget (datasetedit_glade, "dataset_edit_dialog") != NULL);
		
		tDataProperties = glade_xml_get_widget (datasetedit_glade, "dataset_edit_treeview_properties");
		tDataProperties_store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tDataProperties), GTK_TREE_MODEL(tDataProperties_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tDataProperties));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Title"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tDataProperties), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tDataProperties), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Type"), renderer, "text", 2, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tDataProperties), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tDataProperties_selection_changed), NULL);
		
		glade_xml_signal_autoconnect(datasetedit_glade);
	
	}

	return glade_xml_get_widget (datasetedit_glade, "dataset_edit_dialog");
}

GtkWidget*
get_dataproperty_edit_dialog (void)
{

	if(!datasetedit_glade) {

		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "datasetedit.glade", NULL);
		datasetedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(datasetedit_glade != NULL);
		g_free(gstr);

		g_assert (glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_dialog") != NULL);
		
		glade_xml_signal_autoconnect(datasetedit_glade);
	
	}

	return glade_xml_get_widget (datasetedit_glade, "dataproperty_edit_dialog");
}


GtkWidget* 
get_names_edit_dialog (void)
{
	if(!namesedit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "namesedit.glade", NULL);
		namesedit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(namesedit_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (namesedit_glade, "names_edit_dialog") != NULL);
		
		tNames = glade_xml_get_widget (namesedit_glade, "names_edit_treeview");

		tNames_store = gtk_list_store_new(NAMES_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tNames), GTK_TREE_MODEL(tNames_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tNames));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", NAMES_NAME_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, NAMES_NAME_COLUMN);
		gtk_tree_view_column_set_expand(column, TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tNames), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Abbreviation"), renderer, "text", NAMES_ABBREVIATION_STRING_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, NAMES_ABBREVIATION_STRING_COLUMN);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tNames), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Plural"), renderer, "text", NAMES_PLURAL_STRING_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, NAMES_PLURAL_STRING_COLUMN);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tNames), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Reference"), renderer, "text", NAMES_REFERENCE_STRING_COLUMN, NULL);
		gtk_tree_view_column_set_sort_column_id(column, NAMES_REFERENCE_STRING_COLUMN);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tNames), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tNames_selection_changed), NULL);
		
		glade_xml_signal_autoconnect(namesedit_glade);

	}

	return glade_xml_get_widget (namesedit_glade, "names_edit_dialog");
}

GtkWidget*
get_csv_import_dialog (void)
{

	if(!csvimport_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "csvimport.glade", NULL);
		csvimport_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(csvimport_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (csvimport_glade, "csv_import_dialog") != NULL);
		
		glade_xml_signal_autoconnect(csvimport_glade);
	
	}
	/* populate combo menu */
	
	GHashTable *hash = g_hash_table_new(g_str_hash, g_str_equal);
	GList *items = NULL;
	for(size_t i = 0; i < CALCULATOR->variables.size(); i++) {
		if(!CALCULATOR->variables[i]->category().empty()) {
			//add category if not present
			if(g_hash_table_lookup(hash, (gconstpointer) CALCULATOR->variables[i]->category().c_str()) == NULL) {
				items = g_list_append(items, (gpointer) CALCULATOR->variables[i]->category().c_str());
				//remember added categories
				g_hash_table_insert(hash, (gpointer) CALCULATOR->variables[i]->category().c_str(), (gpointer) hash);
			}
		}
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(glade_xml_get_widget (csvimport_glade, "csv_import_combo_category")), items);
	g_hash_table_destroy(hash);	
	g_list_free(items);

	return glade_xml_get_widget (csvimport_glade, "csv_import_dialog");
}

GtkWidget*
get_csv_export_dialog (void)
{

	if(!csvexport_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "csvexport.glade", NULL);
		csvexport_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(csvexport_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (csvexport_glade, "csv_export_dialog") != NULL);
		
		glade_xml_signal_autoconnect(csvexport_glade);
	
	}
	
	return glade_xml_get_widget (csvexport_glade, "csv_export_dialog");
	
}

GtkWidget* get_set_base_dialog (void) {
	if(!setbase_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "setbase.glade", NULL);
		setbase_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(setbase_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (setbase_glade, "set_base_dialog") != NULL);

		switch(evalops.parse_options.base) {
			case BASE_BINARY: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_binary")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
				break;
			}
			case BASE_OCTAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_octal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
				break;
			}
			case BASE_DECIMAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_decimal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
				break;
			}
			case BASE_HEXADECIMAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_hexadecimal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
				break;
			}
			case BASE_ROMAN_NUMERALS: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_roman")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), FALSE);
				break;
			}
			default: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_input_other")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other"), TRUE);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_input_other")), evalops.parse_options.base);
			}
		}
		switch(printops.base) {
			case BASE_BINARY: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_binary")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_OCTAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_octal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_DECIMAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_decimal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_HEXADECIMAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_hexadecimal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_SEXAGESIMAL: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_sexagesimal")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_TIME: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_time")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			case BASE_ROMAN_NUMERALS: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_roman")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), FALSE);
				break;
			}
			default: {
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_radiobutton_output_other")), TRUE);
				gtk_widget_set_sensitive(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other"), TRUE);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget (setbase_glade, "set_base_spinbutton_output_other")), printops.base);
			}
		}

		glade_xml_signal_autoconnect(setbase_glade);
	
	}

	return glade_xml_get_widget (setbase_glade, "set_base_dialog");
}

GtkWidget*
get_nbases_dialog (void)
{
	if(!nbases_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "nbases.glade", NULL);
		nbases_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(nbases_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (nbases_glade, "nbases_dialog") != NULL);
		
		gtk_entry_set_alignment(GTK_ENTRY(glade_xml_get_widget (nbases_glade, "nbases_entry_binary")), 1.0);
		gtk_entry_set_alignment(GTK_ENTRY(glade_xml_get_widget (nbases_glade, "nbases_entry_octal")), 1.0);
		gtk_entry_set_alignment(GTK_ENTRY(glade_xml_get_widget (nbases_glade, "nbases_entry_decimal")), 1.0);
		gtk_entry_set_alignment(GTK_ENTRY(glade_xml_get_widget (nbases_glade, "nbases_entry_hexadecimal")), 1.0);
		
		glade_xml_signal_autoconnect(nbases_glade);
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (nbases_glade, "nbases_dialog")), icon_pixbuf);
	
	}

	return glade_xml_get_widget (nbases_glade, "nbases_dialog");
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

GtkWidget* get_about_dialog (void) {
	if(!about_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "about.glade", NULL);
		about_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(about_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (about_glade, "about_dialog") != NULL);
		
		glade_xml_signal_autoconnect(about_glade);
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (about_glade, "about_dialog")), icon_pixbuf);
	
	}

	return glade_xml_get_widget (about_glade, "about_dialog");
}
GtkWidget* get_argument_rules_dialog (void) {
	
	if(!argumentrules_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "argumentrules.glade", NULL);
		argumentrules_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(argumentrules_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (argumentrules_glade, "argument_rules_dialog") != NULL);
		
		glade_xml_signal_autoconnect(argumentrules_glade);
	
	}

	return glade_xml_get_widget (argumentrules_glade, "argument_rules_dialog");	
}
GtkWidget* get_decimals_dialog (void) {
	if(!decimals_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "decimals.glade", NULL);
		decimals_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(decimals_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (decimals_glade, "decimals_dialog") != NULL);
		
		glade_xml_signal_autoconnect(decimals_glade);
	
	}

	return glade_xml_get_widget (decimals_glade, "decimals_dialog");
}
GtkWidget* get_plot_dialog (void) {
	if(!plot_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "plot.glade", NULL);
		plot_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(plot_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (plot_glade, "plot_dialog") != NULL);
		
		tPlotFunctions = glade_xml_get_widget (plot_glade, "plot_treeview_data");
		tPlotFunctions_store = gtk_list_store_new(10, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_STRING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tPlotFunctions), GTK_TREE_MODEL(tPlotFunctions_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tPlotFunctions));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Title"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tPlotFunctions), column);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Expression"), renderer, "text", 1, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tPlotFunctions), column);	
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tPlotFunctions_selection_changed), NULL);
		
#ifndef HAVE_LIBGNOME
		gtk_widget_hide(glade_xml_get_widget (plot_glade, "plot_button_help"));
#endif		

		gtk_widget_set_sensitive(glade_xml_get_widget(plot_glade, "plot_button_save"), false);

		glade_xml_signal_autoconnect(plot_glade);
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (plot_glade, "plot_dialog")), icon_pixbuf);
	
	}
		
	return glade_xml_get_widget (plot_glade, "plot_dialog");		
}
GtkWidget* get_precision_dialog (void) {
	if(!precision_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "precision.glade", NULL);
		precision_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(precision_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (precision_glade, "precision_dialog") != NULL);
		
		glade_xml_signal_autoconnect(precision_glade);
	
	}

	return glade_xml_get_widget (precision_glade, "precision_dialog");
}
GtkWidget* get_unit_dialog (void) {

	if(!unit_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "unit.glade", NULL);
		unit_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(unit_glade != NULL);
		g_free(gstr);
		
		g_assert (glade_xml_get_widget (unit_glade, "unit_dialog") != NULL);
		
		tUnitSelectorCategories = glade_xml_get_widget (unit_glade, "unit_dialog_treeview_category");
		tUnitSelector		= glade_xml_get_widget (unit_glade, "unit_dialog_treeview_unit");
	
		tUnitSelector_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnitSelector_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tUnitSelector_store), 0, GTK_SORT_ASCENDING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tUnitSelector), GTK_TREE_MODEL(tUnitSelector_store));
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelector));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(_("Name"), renderer, "text", 0, NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tUnitSelector), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tUnitSelector_selection_changed), NULL);
		gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tUnitSelector), TRUE);

		tUnitSelectorCategories_store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(tUnitSelectorCategories), GTK_TREE_MODEL(tUnitSelectorCategories_store));
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tUnitSelectorCategories));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(_("Category"), renderer, "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(tUnitSelectorCategories), column);
		g_signal_connect((gpointer) selection, "changed", G_CALLBACK(on_tUnitSelectorCategories_selection_changed), NULL);
		gtk_tree_view_column_set_sort_column_id(column, 0);
		gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(tUnitSelectorCategories_store), 0, string_sort_func, GINT_TO_POINTER(0), NULL);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tUnitSelectorCategories_store), 0, GTK_SORT_ASCENDING);
		
		glade_xml_signal_autoconnect(unit_glade);
		
		update_unit_selector_tree();
	
	}
	gtk_widget_grab_focus(glade_xml_get_widget (unit_glade, "unit_dialog_entry_unit"));
	return glade_xml_get_widget (unit_glade, "unit_dialog");
}
GtkWidget* get_periodic_dialog (void) {
	if(!periodictable_glade) {
	
		gchar *gstr = g_build_filename (PACKAGE_DATA_DIR, PACKAGE, "glade", "periodictable.glade", NULL);
		periodictable_glade = glade_xml_new(gstr, NULL, NULL);
		g_assert(periodictable_glade != NULL);
		g_free(gstr);
	
		g_assert (glade_xml_get_widget (periodictable_glade, "periodic_dialog") != NULL);
		
		glade_xml_signal_autoconnect(periodictable_glade);
		
		DataSet *dc = CALCULATOR->getDataSet("atom");
		if(!dc) {
			return glade_xml_get_widget (periodictable_glade, "periodic_dialog");
		}
		
		DataObject *e;
		GtkWidget *e_button;
		GtkTable *e_table = GTK_TABLE(glade_xml_get_widget (periodictable_glade, "periodic_table"));
		periodic_tooltips = gtk_tooltips_new();
		string tip;
		GtkStyle *e_style[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
		GtkStyle *l_style = NULL;
		GdkColor c_black;
		c_black.red = 0x0000;
		c_black.green = 0x0000;
		c_black.blue = 0x0000;
		DataProperty *p_xpos = dc->getProperty("x_pos");
		DataProperty *p_ypos = dc->getProperty("y_pos");
		DataProperty *p_weight = dc->getProperty("weight");
		DataProperty *p_number = dc->getProperty("number");
		DataProperty *p_symbol = dc->getProperty("symbol");
		DataProperty *p_class = dc->getProperty("class");
		DataProperty *p_name = dc->getProperty("name");
		int x_pos = 0, y_pos = 0, group = 0;
		string weight;
		for(size_t i = 1; i < 120; i++) {
			e = dc->getObject(i2s(i));
			if(e) {
				x_pos = s2i(e->getProperty(p_xpos));
				y_pos = s2i(e->getProperty(p_ypos));
			}
			if(e && x_pos > 0 && x_pos <= 18 && y_pos > 0 && y_pos <= 10) {
				e_button = gtk_button_new_with_label(e->getProperty(p_symbol).c_str());
				gtk_button_set_relief(GTK_BUTTON(e_button), GTK_RELIEF_HALF);
				if(!e_style[0]) {
					l_style = gtk_style_copy(gtk_widget_get_style(gtk_bin_get_child(GTK_BIN(e_button))));
					for(size_t i2 = 0; i2 < 5; i2++) {
						l_style->text[i2] = c_black;
						l_style->fg[i2] = c_black;
					}
					for(size_t i3 = 0; i3 < 12; i3++) {
						e_style[i3] = gtk_style_copy(gtk_widget_get_style(e_button));
						for(size_t i2 = 0; i2 < 5; i2++) {
							e_style[i3]->bg_pixmap[i2] = NULL;
							e_style[i3]->text[i2] = c_black;
							switch(i3) {
								case 0: {
									e_style[i3]->bg[i2].red = 0xeeee;
									e_style[i3]->bg[i2].green = 0xcccc;
									e_style[i3]->bg[i2].blue = 0xeeee;
									break;
								}
								case 1: {
									e_style[i3]->bg[i2].red = 0xdddd;
									e_style[i3]->bg[i2].green = 0xcccc;
									e_style[i3]->bg[i2].blue = 0xeeee;
									break;
								}
								case 2: {
									e_style[i3]->bg[i2].red = 0xcccc;
									e_style[i3]->bg[i2].green = 0xdddd;
									e_style[i3]->bg[i2].blue = 0xffff;
									break;
								}
								case 3: {
									e_style[i3]->bg[i2].red = 0xdddd;
									e_style[i3]->bg[i2].green = 0xeeee;
									e_style[i3]->bg[i2].blue = 0xffff;
									break;
								}
								case 4: {
									e_style[i3]->bg[i2].red = 0xcccc;
									e_style[i3]->bg[i2].green = 0xeeee;
									e_style[i3]->bg[i2].blue = 0xeeee;
									break;
								}
								case 5: {
									e_style[i3]->bg[i2].red = 0xbbbb;
									e_style[i3]->bg[i2].green = 0xffff;
									e_style[i3]->bg[i2].blue = 0xbbbb;
									break;
								}
								case 6: {
									e_style[i3]->bg[i2].red = 0xeeee;
									e_style[i3]->bg[i2].green = 0xffff;
									e_style[i3]->bg[i2].blue = 0xdddd;
									break;
								}
								case 7: {
									e_style[i3]->bg[i2].red = 0xffff;
									e_style[i3]->bg[i2].green = 0xffff;
									e_style[i3]->bg[i2].blue = 0xaaaa;
									break;
								}
								case 8: {
									e_style[i3]->bg[i2].red = 0xffff;
									e_style[i3]->bg[i2].green = 0xdddd;
									e_style[i3]->bg[i2].blue = 0xaaaa;
									break;
								}
								case 9: {
									e_style[i3]->bg[i2].red = 0xffff;
									e_style[i3]->bg[i2].green = 0xcccc;
									e_style[i3]->bg[i2].blue = 0xdddd;
									break;
								}
								case 10: {
									e_style[i3]->bg[i2].red = 0xaaaa;
									e_style[i3]->bg[i2].green = 0xeeee;
									e_style[i3]->bg[i2].blue = 0xdddd;
									break;
								}
								case 11: {
									break;
								}
							}
						}
					}
				}
				group = s2i(e->getProperty(p_class));
				if(group > 0 && group <= 11) gtk_widget_set_style(e_button, e_style[group - 1]);
				else gtk_widget_set_style(e_button, e_style[11]);
				gtk_widget_set_style(gtk_bin_get_child(GTK_BIN(e_button)), l_style);
				if(x_pos > 2) gtk_table_attach_defaults(e_table, e_button, x_pos + 1, x_pos + 2, y_pos, y_pos + 1);
				else gtk_table_attach_defaults(e_table, e_button, x_pos, x_pos + 1, y_pos, y_pos + 1);
				tip = e->getProperty(p_number);
				tip += " ";
				tip += e->getProperty(p_name);
				weight = e->getPropertyDisplayString(p_weight);
				if(!weight.empty() && weight != "-") {
					tip += "\n";
					tip += weight;
				}
				gtk_tooltips_set_tip(periodic_tooltips, e_button, tip.c_str(), "");
				gtk_widget_show(e_button);
				g_signal_connect((gpointer) e_button, "clicked", G_CALLBACK(on_element_button_clicked), (gpointer) e);
			}
		}
		
		gtk_window_set_icon(GTK_WINDOW(glade_xml_get_widget (periodictable_glade, "periodic_dialog")), icon_pixbuf);
	
	}

	return glade_xml_get_widget (periodictable_glade, "periodic_dialog");
}
