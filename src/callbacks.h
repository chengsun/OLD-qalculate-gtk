/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include <gtk/gtk.h>

#define EXPAND_TO_ITER(model, view, iter)		GtkTreePath *path = gtk_tree_model_get_path(model, &iter); \
							gtk_tree_view_expand_to_path(GTK_TREE_VIEW(view), path); \
							gtk_tree_path_free(path);
#define EXPAND_ITER(model, view, iter)			GtkTreePath *path = gtk_tree_model_get_path(model, &iter); \
							gtk_tree_view_expand_row(GTK_TREE_VIEW(view), path, false); \
							gtk_tree_path_free(path);							

void create_umenu(void);
void create_umenu2(void);
void create_vmenu(void);
void create_fmenu(void);
void create_pmenu(void);
void create_pmenu2(void);

void generate_functions_tree_struct();
void generate_variables_tree_struct();
void generate_units_tree_struct();

gboolean on_display_errors_timeout(gpointer data);

void update_functions_tree(GtkWidget *wfun);
void update_variables_tree(GtkWidget *wvar);
void update_units_tree(GtkWidget *wvar);
void on_tFunctions_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tFunctionCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tVariables_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tVariableCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tUnits_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tUnitCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);

void on_menu_e_deactivate(GtkMenuShell *menushell, gpointer user_data);
void on_menu_r_deactivate(GtkMenuShell *menushell, gpointer user_data);

void convert_in_wUnits(int toFrom = -1);
void on_omToUnit_menu_activate(GtkMenuItem *item, gpointer user_data);

void convert_to_unit(GtkMenuItem *w, gpointer user_data);
void convert_to_custom_unit(GtkMenuItem *w, gpointer user_data);

void save_defs();
void save_mode();

void load_preferences();
void save_preferences(bool mode = false);
void edit_preferences();

gint string_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);
gint int_string_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);

void set_prefix(GtkMenuItem *w, gpointer user_data);

void manage_variables(GtkMenuItem *w, gpointer user_data);
void manage_functions(GtkMenuItem *w, gpointer user_data);
void manage_units(GtkMenuItem *w, gpointer user_data);

void set_clean_mode(GtkMenuItem *w, gpointer user_data);
void set_functions_enabled(GtkMenuItem *w, gpointer user_data);
void set_variables_enabled(GtkMenuItem *w, gpointer user_data);
void set_unknownvariables_enabled(GtkMenuItem *w, gpointer user_data);
void set_units_enabled(GtkMenuItem *w, gpointer user_data);
void insert_function(GtkMenuItem *w, gpointer user_data);
void insert_variable(GtkMenuItem *w, gpointer user_data);
void insert_prefix(GtkMenuItem *w, gpointer user_data);
void insert_unit(GtkMenuItem *w, gpointer user_data);

void new_function(GtkMenuItem *w, gpointer user_data);
void new_variable(GtkMenuItem *w, gpointer user_data);
void new_unit(GtkMenuItem *w, gpointer user_data);
void add_as_variable();

extern "C" {

void on_menu_item_quit_activate(GtkMenuItem *w, gpointer user_data);
void on_button_close_clicked(GtkButton *w, gpointer user_data);
void on_preferences_checkbutton_short_units_toggled(GtkToggleButton *w, gpointer user_data);
void on_preferences_checkbutton_save_defs_toggled(GtkToggleButton *w, gpointer user_data);
void on_preferences_checkbutton_save_mode_toggled(GtkToggleButton *w, gpointer user_data);
void on_preferences_checkbutton_load_defs_toggled(GtkToggleButton *w, gpointer user_data);
void on_units_togglebutton_from_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_units_button_convert_clicked(GtkButton *button, gpointer user_data);
void on_units_togglebutton_to_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_units_entry_from_val_activate(GtkEntry *entry, gpointer user_data);
void on_units_entry_to_val_activate(GtkEntry *entry, gpointer user_data);
void on_units_button_close_clicked(GtkButton *button, gpointer user_data);
gboolean on_units_dialog_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_units_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_radiobutton_radians_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_degrees_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_gradians_toggled(GtkToggleButton *togglebutton, gpointer user_data);
gboolean on_gcalc_exit(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_expression_activate(GtkEntry *entry, gpointer user_data);
void on_button_less_more_clicked(GtkButton *button, gpointer user_data);
void on_button_execute_clicked(GtkButton *button, gpointer user_data);
void on_button_del_clicked(GtkButton *w, gpointer user_data);
void on_button_ac_clicked(GtkButton *w, gpointer user_data);
void on_button_hyp_toggled(GtkToggleButton *w, gpointer user_data);
void on_button_tan_clicked(GtkButton *w, gpointer user_data);
void on_button_sine_clicked(GtkButton *w, gpointer user_data);
void on_button_cosine_clicked(GtkButton *w, gpointer user_data);
void on_button_store_clicked(GtkButton *w, gpointer user_data);
void on_togglebutton_expression_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_togglebutton_result_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_expression_changed(GtkEditable *w, gpointer user_data);
void on_button_zero_clicked(GtkButton *w, gpointer user_data);
void on_button_one_clicked(GtkButton *w, gpointer user_data);
void on_button_two_clicked(GtkButton *w, gpointer user_data);
void on_button_three_clicked(GtkButton *w, gpointer user_data);
void on_button_four_clicked(GtkButton *w, gpointer user_data);
void on_button_five_clicked(GtkButton *w, gpointer user_data);
void on_button_six_clicked(GtkButton *w, gpointer user_data);
void on_button_seven_clicked(GtkButton *w, gpointer user_data);
void on_button_eight_clicked(GtkButton *w, gpointer user_data);
void on_button_nine_clicked(GtkButton *w, gpointer user_data);
void on_button_dot_clicked(GtkButton *w, gpointer user_data);
void on_button_brace_open_clicked(GtkButton *w, gpointer user_data);
void on_button_brace_close_clicked(GtkButton *w, gpointer user_data);
void on_button_times_clicked(GtkButton *w, gpointer user_data);
void on_button_add_clicked(GtkButton *w, gpointer user_data);
void on_button_sub_clicked(GtkButton *w, gpointer user_data);
void on_button_divide_clicked(GtkButton *w, gpointer user_data);
void on_button_ans_clicked(GtkButton *w, gpointer user_data);
void on_button_exp_clicked(GtkButton *w, gpointer user_data);
void on_button_xy_clicked(GtkButton *w, gpointer user_data);
void on_button_square_clicked(GtkButton *w, gpointer user_data);
void on_button_sqrt_clicked(GtkButton *w, gpointer user_data);
void on_button_log_clicked(GtkButton *w, gpointer user_data);
void on_button_ln_clicked(GtkButton *w, gpointer user_data);
void on_menu_item_addition_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_subtraction_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_multiplication_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_division_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_power_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_exponent_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_save_defs_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_save_mode_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_edit_prefs_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_degrees_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_radians_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_gradians_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_binary_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_octal_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_decimal_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_hexadecimal_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_convert_number_bases_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_display_normal_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_display_scientific_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_display_non_scientific_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_display_prefixes_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_save_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_precision_activate(GtkMenuItem *w, gpointer user_data);
void on_menu_item_decimals_activate(GtkMenuItem *w, gpointer user_data);
void on_unit_edit_entry_name_changed(GtkEditable *editable, gpointer user_data);
void on_unit_edit_optionmenu_class_changed(GtkOptionMenu *om, gpointer user_data);
void on_units_button_new_clicked(GtkButton *button, gpointer user_data);
void on_units_button_edit_clicked(GtkButton *button, gpointer user_data);
void on_units_button_insert_clicked(GtkButton *button, gpointer user_data);
void on_units_button_convert_to_clicked(GtkButton *button, gpointer user_data);
void on_units_button_delete_clicked(GtkButton *button, gpointer user_data);
void on_variables_button_new_clicked(GtkButton *button, gpointer user_data);
void on_variables_button_edit_clicked(GtkButton *button, gpointer user_data);
void on_variables_button_insert_clicked(GtkButton *button, gpointer user_data);
void on_variables_button_delete_clicked(GtkButton *button, gpointer user_data);
void on_variables_button_close_clicked(GtkButton *button, gpointer user_data);
gboolean on_variables_dialog_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_variables_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_functions_button_new_clicked(GtkButton *button, gpointer user_data);
void on_functions_button_edit_clicked(GtkButton *button, gpointer user_data);
void on_functions_button_insert_clicked(GtkButton *button, gpointer user_data);
void on_functions_button_delete_clicked(GtkButton *button, gpointer user_data);
void on_functions_button_close_clicked(GtkButton *button, gpointer user_data);
gboolean on_functions_dialog_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_functions_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_function_edit_entry_name_changed(GtkEditable *editable, gpointer user_data);
void on_variable_edit_entry_name_changed(GtkEditable *editable, gpointer user_data);

void on_nbases_button_close_clicked(GtkButton *button, gpointer user_data);
void on_nbases_entry_decimal_changed(GtkEditable *editable, gpointer user_data);
void on_nbases_entry_binary_changed(GtkEditable *editable, gpointer user_data);
void on_nbases_entry_octal_changed(GtkEditable *editable, gpointer user_data);
void on_nbases_entry_hexadecimal_changed(GtkEditable *editable, gpointer user_data);

void on_button_functions_clicked(GtkButton *button, gpointer user_data);
void on_button_variables_clicked(GtkButton *button, gpointer user_data);
void on_button_units_clicked(GtkButton *button, gpointer user_data);
void on_button_convert_clicked(GtkButton *button, gpointer user_data);

void on_menu_item_about_activate(GtkMenuItem *w, gpointer user_data);

void on_precision_dialog_spinbutton_precision_value_changed(GtkSpinButton *w, gpointer user_data);
void on_decimals_dialog_spinbutton_decimals_value_changed(GtkSpinButton *w, gpointer user_data);
void on_decimals_dialog_radiobutton_least_toggled(GtkToggleButton *w, gpointer user_data);
void on_decimals_dialog_radiobutton_always_toggled(GtkToggleButton *w, gpointer user_data);

}

