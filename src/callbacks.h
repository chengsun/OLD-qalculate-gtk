/*
    Qalculate    

    Copyright (C) 2003  Niklas Knutsson (nq@altern.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include <gtk/gtk.h>

void create_umenu(void);
void create_umenu2(void);
void create_vmenu(void);
void create_fmenu(void);
void create_pmenu(void);

gboolean on_display_errors_timeout(gpointer data);

void on_omUnitType_changed(GtkOptionMenu *om, gpointer user_data);

void on_unit_name_entry_changed(GtkEditable *editable, gpointer user_data);
void on_function_name_entry_changed(GtkEditable *editable, gpointer user_data);
void on_variable_name_entry_changed(GtkEditable *editable, gpointer user_data);

void update_functions_tree(GtkWidget *wfun);
void update_variables_tree(GtkWidget *wvar);
void update_units_tree(GtkWidget *wvar);
void on_tFunctions_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tFunctionCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tVariables_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tVariableCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tUnits_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);
void on_tUnitCategories_selection_changed(GtkTreeSelection *treeselection, gpointer user_data);

void on_expression_activate(GtkEntry *entry, gpointer user_data);
void on_bHistory_clicked(GtkButton *button, gpointer user_data);
void on_bEXE_clicked(GtkButton *button, gpointer user_data);
void on_bMenuE_toggled(GtkToggleButton *button, gpointer user_data);
void on_bMenuR_toggled(GtkToggleButton *button, gpointer user_data);
void on_menu_e_deactivate(GtkMenuShell *menushell, gpointer user_data);
void on_menu_r_deactivate(GtkMenuShell *menushell, gpointer user_data);

void on_bNewFunction_clicked(GtkButton *button, gpointer user_data);
void on_bEditFunction_clicked(GtkButton *button, gpointer user_data);
void on_bInsertFunction_clicked(GtkButton *button, gpointer user_data);
void on_bDeleteFunction_clicked(GtkButton *button, gpointer user_data);
void on_bCloseFunctions_clicked(GtkButton *button, gpointer user_data);
gboolean on_wFunctions_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_wFunctions_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

void on_bNewVariable_clicked(GtkButton *button, gpointer user_data);
void on_bEditVariable_clicked(GtkButton *button, gpointer user_data);
void on_bInsertVariable_clicked(GtkButton *button, gpointer user_data);
void on_bDeleteVariable_clicked(GtkButton *button, gpointer user_data);
void on_bCloseVariables_clicked(GtkButton *button, gpointer user_data);
gboolean on_wVariables_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_wVariables_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

void convert_in_wUnits(int toFrom = -1);
void on_bNewUnit_clicked(GtkButton *button, gpointer user_data);
void on_bEditUnit_clicked(GtkButton *button, gpointer user_data);
void on_bInsertUnit_clicked(GtkButton *button, gpointer user_data);
void on_bConvertToUnit_clicked(GtkButton *button, gpointer user_data);
void on_bDeleteUnit_clicked(GtkButton *button, gpointer user_data);
void on_bCloseUnits_clicked(GtkButton *button, gpointer user_data);
void on_tbToFrom_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_bConvertUnits_clicked(GtkButton *button, gpointer user_data);
void on_tbToTo_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_omToUnit_changed(GtkOptionMenu *om, gpointer user_data);
void on_omToUnit_menu_activate(GtkMenuItem *item, gpointer user_data);
void on_eFromValue_activate(GtkEntry *entry, gpointer user_data);
void on_eToValue_activate(GtkEntry *entry, gpointer user_data);
gboolean on_wUnits_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_wUnits_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

void on_expression_changed(GtkEditable *w, gpointer user_data);

void insert_sign(GtkMenuItem *w, gpointer user_data);
void insert_function(GtkMenuItem *w, gpointer user_data);
void insert_variable(GtkMenuItem *w, gpointer user_data);
void insert_prefix(GtkMenuItem *w, gpointer user_data);
void insert_unit(GtkMenuItem *w, gpointer user_data);

void new_function(GtkMenuItem *w, gpointer user_data);
void new_variable(GtkMenuItem *w, gpointer user_data);
void new_unit(GtkMenuItem *w, gpointer user_data);

void convert_to_unit(GtkMenuItem *w, gpointer user_data);
void convert_to_custom_unit(GtkMenuItem *w, gpointer user_data);

void add_as_variable(GtkMenuItem *w, gpointer user_data);
void set_precision(GtkSpinButton *w, gpointer user_data);
void set_decimals(GtkSpinButton *w, gpointer user_data);
void on_deci_least_toggled(GtkToggleButton *w, gpointer user_data);
void on_deci_fixed_toggled(GtkToggleButton *w, gpointer user_data);
void select_precision(GtkMenuItem *w, gpointer user_data);
void select_decimals(GtkMenuItem *w, gpointer user_data);
void set_display_mode(GtkMenuItem *w, gpointer user_data);
void set_number_base(GtkMenuItem *w, gpointer user_data);

void button_pressed(GtkButton *w, gpointer user_data);
void button_function_pressed(GtkButton *w, gpointer user_data);
void on_bDEL_clicked(GtkButton *w, gpointer user_data);
void on_bAC_clicked(GtkButton *w, gpointer user_data);
void on_bHyp_clicked(GtkToggleButton *w, gpointer user_data);
void on_bTan_clicked(GtkButton *w, gpointer user_data);
void on_bSin_clicked(GtkButton *w, gpointer user_data);
void on_bCos_clicked(GtkButton *w, gpointer user_data);
void on_bSTO_clicked(GtkButton *w, gpointer user_data);

void set_clean_mode(GtkMenuItem *w, gpointer user_data);
void set_functions_enabled(GtkMenuItem *w, gpointer user_data);
void set_variables_enabled(GtkMenuItem *w, gpointer user_data);
void set_units_enabled(GtkMenuItem *w, gpointer user_data);
void set_angle_mode(GtkMenuItem *w, gpointer user_data);
void on_rRad_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_rDeg_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_rGra_toggled(GtkToggleButton *togglebutton, gpointer user_data);

void save_defs();
void save_mode();
void manage_variables(GtkMenuItem *w, gpointer user_data);
void manage_functions(GtkMenuItem *w, gpointer user_data);
void manage_units(GtkMenuItem *w, gpointer user_data);

void on_bClose_clicked(GtkButton *w, gpointer user_data);
gboolean on_gcalc_exit(GtkWidget *widget, GdkEvent *event, gpointer user_data);

void load_preferences();
void save_preferences(bool mode = false);
void edit_preferences();
void on_cbUseShortUnits_toggled(GtkToggleButton *w, gpointer user_data);
void on_cbSaveDefsOnExit_toggled(GtkToggleButton *w, gpointer user_data);
void on_cbSaveModeOnExit_toggled(GtkToggleButton *w, gpointer user_data);
void on_cbLoadGlobalDefs_toggled(GtkToggleButton *w, gpointer user_data);

gint string_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);
gint int_string_sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);

