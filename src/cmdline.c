#include "calclib/qalculate.h"

int main (int argc, char *argv[]) {

	string calc_arg;
	for(int i = 1; i < argc; i++) {
		if(i > 1) {
			calc_arg += " ";
		}
		if(strlen(argv[i]) >= 2 && ((argv[i][0] == '\"' && argv[i][strlen(argv[i] - 1)] == '\"') || (argv[i][0] == '\'' && argv[i][strlen(argv[i] - 1)] == '\''))) {
			calc_arg += argv[i] + 1;
			calc_arg.erase(calc_arg.length() - 1);
		} else {
			calc_arg += argv[i];
		}
	}

	//create the almighty Calculator object
	new Calculator();

	//load application specific preferences
	//load_preferences();

	mstruct = new MathStructure();
	parsed_mstruct = new MathStructure();
	parsed_to_str = new string;

	bool canfetch = CALCULATOR->canFetch();

	//exchange rates
	if((fetch_exchange_rates_at_startup || first_time) && canfetch) {
		fetch_exchange_rates(5);
		while(gtk_events_pending()) gtk_main_iteration();
	}

	CALCULATOR->loadExchangeRates();

	string ans_str = _("ans");
	vans[0] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str, m_undefined, _("Last Answer"), false));
	vans[0]->addName(_("answer"));
	vans[0]->addName(ans_str + "1");
	vans[1] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "2", m_undefined, _("Answer 2"), false));
	vans[2] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "3", m_undefined, _("Answer 3"), false));
	vans[3] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "4", m_undefined, _("Answer 4"), false));
	vans[4] = (KnownVariable*) CALCULATOR->addVariable(new KnownVariable(_("Temporary"), ans_str + "5", m_undefined, _("Answer 5"), false));

	//load global definitions
	if(load_global_defs && !CALCULATOR->loadGlobalDefinitions()) {
		g_print(_("Failed to load global definitions!\n"));
	}
	
	//load local definitions
	CALCULATOR->loadLocalDefinitions();

	//reset
	result_text = "0";
	parsed_text = "0";

	//check for calculation errros regularly
	do_timeout = true;
	g_timeout_add(100, on_display_errors_timeout, NULL);
	
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "menu_item_plot_functions"), canplot);
	gtk_widget_set_sensitive(glade_xml_get_widget (main_glade, "menu_item_fetch_exchange_rates"), canfetch);
		
	int pipe_wr[] = {0, 0};
	pipe(pipe_wr);
	view_pipe_r = fdopen(pipe_wr[0], "r");
	view_pipe_w = fdopen(pipe_wr[1], "w");
	pthread_attr_init(&view_thread_attr);
	pthread_create(&view_thread, &view_thread_attr, view_proc, view_pipe_r);
	
	if(!calc_arg.empty()) {
		execute_expression();
	}
	
	return 0;
	
}


