#include "calclib/qalculate.h"
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>

MathStructure *mstruct, *parsed_mstruct;
string *parsed_to_str;
KnownVariable *vans[5];
string result_text, parsed_text;
bool load_global_defs, fetch_exchange_rates_at_startup, first_time, first_qalculate_run, save_mode_on_exit, save_defs_on_exit;
PrintOptions printops, saved_printops;
EvaluationOptions evalops, saved_evalops;
AssumptionNumberType saved_assumption_type;
AssumptionSign saved_assumption_sign;
int saved_precision;
FILE *view_pipe_r, *view_pipe_w;
pthread_t view_thread;
pthread_attr_t view_thread_attr;
bool b_busy = false;
string expression_str;

fd_set in_set;
struct timeval timeout;

void *view_proc(void *pipe);
void execute_expression(bool goto_input = true);
void load_preferences();
void save_preferences(bool mode = false);
void save_mode();
void set_saved_mode();
void save_defs();
void result_display_updated();
void result_format_updated();
void result_action_executed();
void result_prefix_changed(Prefix *prefix = NULL);
void expression_format_updated();

void show_prompt() {
	fputs("\n> ", stdout);
}

int s2b(const string &str) {
	if(str == "yes") return 1;
	if(str == "no") return 0;
	if(str == "Yes") return 1;
	if(str == "No") return 0;
	if(str == "true") return 1;
	if(str == "false") return 0;
	if(str == "True") return 1;
	if(str == "False") return 0;
	int v = s2i(str);
	if(v > 0) return 1;
	if(v == 0) return 0;
	return -1;
}

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
	
	b_busy = false;

	//create the almighty Calculator object
	new Calculator();

	//load application specific preferences
	load_preferences();

	mstruct = new MathStructure();
	parsed_mstruct = new MathStructure();
	parsed_to_str = new string;

	bool canfetch = CALCULATOR->canFetch();

	string str;
	char buffer[1000];
	
	//exchange rates
	if(first_qalculate_run && canfetch) {
		fputs(_("You need the download exchange rates to be able to convert between different currencies.\nYou can later get current exchange rates with the \"exchange rates\" command.\nDo you want to fetch exchange rates now from the Internet (default yes): "), stdout);
		while(true) {
			str = fgets(buffer, 1000, stdin);
			remove_blank_ends(str);
			if(str.empty() || str == _("yes")) {
				CALCULATOR->fetchExchangeRates(5);
				break;
			} else if(str == _("no")) {
				break;
			} else {
				puts(_("Please answer yes or no"));
			}
		}
	} else if(fetch_exchange_rates_at_startup && canfetch) {
		CALCULATOR->fetchExchangeRates(5);
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
		puts(_("Failed to load global definitions!"));
	}
	
	//load local definitions
	CALCULATOR->loadLocalDefinitions();

	//reset
	result_text = "0";
	parsed_text = "0";
	
	int pipe_wr[] = {0, 0};
	pipe(pipe_wr);
	view_pipe_r = fdopen(pipe_wr[0], "r");
	view_pipe_w = fdopen(pipe_wr[1], "w");
	pthread_attr_init(&view_thread_attr);
	pthread_create(&view_thread, &view_thread_attr, view_proc, view_pipe_r);
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;

	if(!calc_arg.empty()) {
		expression_str = calc_arg;
		execute_expression(false);
		pthread_cancel(view_thread);
		CALCULATOR->terminateThreads();
		return 0;
	}
	
	fputs("> ", stdout);
	
	string svar, svalue;
	unsigned int index;
	
	while(true) {
		str = fgets(buffer, 1000, stdin);
		remove_blank_ends(str);
		if(str.substr(0, 4) == "set ") {
			str = str.substr(4, str.length() - 4);
			remove_blank_ends(str);
			if((index = str.find_first_of(" ")) != string::npos) {
				svalue = str.substr(index + 1, str.length() - (index + 1));
				remove_blank_ends(svalue);
			}
			svar = str.substr(0, index);
			remove_blank_ends(svar);
			if(svar == "base" || svar == "inbase" || svar == "outbase") {
				int v = 0;
				bool b_in = (svar == "inbase");
				if(svalue == "roman") v = BASE_ROMAN_NUMERALS;
				else if(svalue == "time") {if(b_in) v = 0; else v = BASE_TIME;}
				else if(svalue == "hex" || svalue == "hexadecimal") v = BASE_HEXADECIMAL;
				else if(svalue == "bin" || svalue == "binary") v = BASE_BINARY;
				else if(svalue == "oct" || svalue == "octal") v = BASE_HEXADECIMAL;
				else if(svalue == "dec" || svalue == "decimal") v = BASE_DECIMAL;
				else if(svalue == "sex" || svalue == "sexagesimal") {if(b_in) v = 0; else v = BASE_SEXAGESIMAL;}
				else {
					v = s2i(svalue);
					if((v < 2 || v > 36) && (b_in || v != 60)) {
						v = 0;
					}
				}
				if(v == 0) {
					puts(_("Illegal base."));
					show_prompt();
				} else if(b_in) {
					evalops.parse_options.base = v;
					expression_format_updated();
				} else {
					printops.base = v;
					result_format_updated();
				}
			} else if(svar == "unicode") {
				int v = s2b(svalue);
				if(v < 0) {
					puts(_("Illegal value"));
					show_prompt();
				} else {
					printops.use_unicode_signs = v;
					result_display_updated();
				}
			}
		} else if(str.substr(0, 5) == "save ") {
			str = str.substr(5, str.length() - 5);
			remove_blank_ends(str);
		} else if(str.substr(0, 8) == "convert ") {
			str = str.substr(8, str.length() - 8);
			remove_blank_ends(str);
			if(str == "best") {
				mstruct->set(CALCULATOR->convertToBestUnit(*mstruct, evalops));
				result_action_executed();
			} else if(str == "best") {
				mstruct->set(CALCULATOR->convertToBaseUnits(*mstruct, evalops));
				result_action_executed();
			} else {
				mstruct->set(CALCULATOR->convert(*mstruct, str, evalops));
				result_action_executed();
			}
		} else if(str == "factor") {
			mstruct->factorize(evalops);
			result_action_executed();
		} else if(str == "help" || str == _("help") || str == "?") {
			puts("");
			puts(_("Enter a mathematical expression or a command."));
			puts("");
			puts(_("Available commands are:"));
			puts("");
			puts("factor");
			puts("set OPTION VALUE");
			puts("save NAME [CATEGORY] [TITLE]");
			puts("convert UNIT");
			puts("quit or exit");
			puts("");
			puts(_("Type help COMMAND for more help (example: help save)."));
			puts("");
			show_prompt();
		} else if(str.substr(0, 5) == "help ") {
			str = str.substr(5, str.length() - 5);
			remove_blank_ends(str);
			if(str == "factor") {
			
			} else if(str == "set") {
				puts("");
				puts(_("Sets the value of an option."));
				puts("");
				puts(_("Available options and accepted values are:"));
				puts("");
				puts("base (2 - 36, bin, oct, dec, hex, sex, time, roman)");
				puts("inbase (2 - 36, bin, oct, dec, hex, roman)");
				puts("unicode (0, 1)");
				puts("");
				puts(_("Example: set base 16."));
				puts("");
				show_prompt();
			} else if(str == "save") {
			
			} else if(str == "convert") {
				puts("");
				puts(_("Converts units in current result."));
				puts("");
				puts(_("Possible unit values are:"));
				puts("");
				puts("a unit (example meter");
				puts("a unit expression (example km/h)");
				puts("base (convert to base units)");
				puts("best (convert best unit)");
				puts("");
				puts(_("Example: convert best."));
				puts("");
				show_prompt();
			} else if(str == "quit" || str == "exit") {
				puts("");
				puts("Terminates this program.");
				puts("");
				show_prompt();
			} else {
				puts(_("Unknown help topic."));
				show_prompt();
			}
		} else if(str == "quit" || str == "exit") {
			break;
		} else {
			expression_str = str;
			execute_expression();
		}
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
	
	return 0;
	
}

void display_errors() {
	if(!CALCULATOR->message()) return;
	while(true) {
		MessageType mtype = CALCULATOR->message()->type();
		if(mtype == MESSAGE_ERROR) {
			printf("error: %s\n", CALCULATOR->message()->message().c_str());
		} else if(mtype == MESSAGE_WARNING) {
			printf("warning: %s\n", CALCULATOR->message()->message().c_str());
		} else {
			puts(CALCULATOR->message()->message().c_str());
		}
		if(!CALCULATOR->nextMessage()) break;
	}
}

void on_abort_display() {
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
		fread(&x, sizeof(void*), 1, view_pipe);
		if(x) {
			PrintOptions po = printops;
			po.short_multiplication = false;
			po.excessive_parenthesis = true;
			po.improve_division_multipliers = false;
			MathStructure mp(*((MathStructure*) x));
			fread(&po.is_approximate, sizeof(bool*), 1, view_pipe);
			mp.format(po);
			parsed_text = mp.print(po);
		}
		printops.allow_non_usable = false;
		m.format(printops);
		result_text = m.print(printops);	
	
		if(result_text == _("aborted")) {
			*printops.is_approximate = false;
		}
		b_busy = false;
	}
	return NULL;
}

void setResult(Prefix *prefix = NULL, bool update_parse = false, bool goto_input = true) {

	b_busy = true;	

	string prev_result_text = result_text;
	result_text = "?";
	if(update_parse) {
		parsed_text = "aborted";
	}

	printops.prefix = prefix;
	
	CALCULATOR->saveState();

	bool parsed_approx = false;
	fwrite(&mstruct, sizeof(void*), 1, view_pipe_w);
	if(update_parse) {
		fwrite(&parsed_mstruct, sizeof(void*), 1, view_pipe_w);
		bool *parsed_approx_p = &parsed_approx;
		fwrite(&parsed_approx_p, sizeof(void*), 1, view_pipe_w);
	} else {
		void *x = NULL;
		fwrite(&x, sizeof(void*), 1, view_pipe_w);
	}
	fflush(view_pipe_w);

	struct timespec rtime;
	rtime.tv_sec = 0;
	rtime.tv_nsec = 10000000;
	int i = 0;
	bool has_printed = false;
	while(b_busy && i < 50) {
		nanosleep(&rtime, NULL);
		i++;
	}
	i = 0;
	
	if(b_busy) {
		printf(_("Processing (press Enter to abort)"));
		has_printed = true;
		fflush(stdout);
	}
	char c = 0;
	rtime.tv_nsec = 100000000;
	while(b_busy) {
		FD_ZERO(&in_set);
		FD_SET(STDIN_FILENO, &in_set);
		if(select(FD_SETSIZE, &in_set, NULL, NULL, &timeout) > 0) {
			read(STDIN_FILENO, &c, 1);
			if(c == '\n') on_abort_display();
		} else {
			printf(".");
			fflush(stdout);
			nanosleep(&rtime, NULL);
		}
	}

	b_busy = true;
	if(has_printed) printf("\n");

	display_errors();
	
	if(update_parse) {
		printf(parsed_text.c_str());
	} else {
		printf(prev_result_text.c_str());
	}
	if(!(*printops.is_approximate) && !mstruct->isApproximate()) {
		printf(" = ");	
	} else {
		if(printops.use_unicode_signs) {
			printf(" " SIGN_ALMOST_EQUAL " ");
		} else {
			printf(" = %s ", _("approx."));
		}
	}
	printf(result_text.c_str());
	printf("\n");
	if(goto_input) show_prompt();
	printops.prefix = NULL;
	b_busy = false;
}

void viewresult(Prefix *prefix = NULL) {
	setResult(prefix);
}

void result_display_updated() {
	setResult(NULL, false);
}
void result_format_updated() {
	setResult(NULL, false);
}
void result_action_executed() {
	setResult(NULL, false);
}
void result_prefix_changed(Prefix *prefix) {
	setResult(prefix, false);
}
void expression_format_updated() {
	execute_expression();
}

void execute_expression(bool goto_input) {

	string str = expression_str;

	b_busy = true;
	CALCULATOR->calculate(*mstruct, CALCULATOR->unlocalizeExpression(str), 0, evalops, parsed_mstruct, parsed_to_str);
	struct timespec rtime;
	rtime.tv_sec = 0;
	rtime.tv_nsec = 10000000;
	int i = 0;
	while(CALCULATOR->busy() && i < 50) {
		nanosleep(&rtime, NULL);
		i++;
	}
	i = 0;
	bool has_printed = false;
	if(CALCULATOR->busy()) {
		printf("Calculating (press Enter to abort)");
		fflush(stdout);
		has_printed = true;
	}
	char c = 0;
	rtime.tv_nsec = 100000000;
	while(CALCULATOR->busy()) {
		FD_ZERO(&in_set);
		FD_SET(STDIN_FILENO, &in_set);
		if(select(FD_SETSIZE, &in_set, NULL, NULL, &timeout) > 0) {
			read(STDIN_FILENO, &c, 1);
			if(c == '\n') CALCULATOR->abort();
		} else {
			printf(".");
			fflush(stdout);
			nanosleep(&rtime, NULL);
		}
	}
	if(has_printed) printf("\n");
	b_busy = false;
	
	//update "ans" variables
	vans[4]->set(vans[3]->get());
	vans[3]->set(vans[2]->get());
	vans[2]->set(vans[1]->get());
	vans[1]->set(vans[0]->get());
	vans[0]->set(*mstruct);
	
	result_text = str;
	setResult(NULL, true, goto_input);

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
	saved_printops = printops;
	saved_evalops = evalops;
	saved_assumption_type = CALCULATOR->defaultAssumptions()->numberType();
	saved_assumption_sign = CALCULATOR->defaultAssumptions()->sign();
}


void load_preferences() {

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
	printops.use_unicode_signs = false;
	printops.use_unit_prefixes = true;
	printops.spacious = true;
	printops.short_multiplication = true;
	printops.place_units_separately = false;
	printops.use_all_prefixes = false;
	printops.excessive_parenthesis = false;
	printops.allow_non_usable = false;
	printops.lower_case_numbers = false;
	
	evalops.approximation = APPROXIMATION_TRY_EXACT;
	evalops.sync_units = true;
	evalops.structuring = STRUCTURING_SIMPLIFY;
	evalops.parse_options.unknowns_enabled = false;
	evalops.parse_options.read_precision = DONT_READ_PRECISION;
	evalops.parse_options.base = BASE_DECIMAL;
	evalops.allow_complex = true;
	evalops.allow_infinite = true;
	evalops.auto_post_conversion = POST_CONVERSION_NONE;
	evalops.assume_denominators_nonzero = false;
	
	save_mode_on_exit = true;
	save_defs_on_exit = true;
	load_global_defs = true;
	fetch_exchange_rates_at_startup = false;
	first_time = false;
	string filename = getLocalDir();
	DIR *dir = opendir(filename.c_str());
	if(!dir) {
		first_qalculate_run = true;
		first_time = true;
		set_saved_mode();
		return;
	} else {
		first_qalculate_run = false;
		closedir(dir);
	}
	FILE *file = NULL;
	filename += "qalc.cfg";
	file = fopen(filename.c_str(), "r");
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
				else if(svar == "use_prefixes")
					printops.use_unit_prefixes = v;
				else if(svar == "number_fraction_format")
					printops.number_fraction_format = (NumberFractionFormat) v;					
				else if(svar == "number_base")
					printops.base = v;
				else if(svar == "number_base_expression")
					evalops.parse_options.base = v;	
				else if(svar == "read_precision")
					evalops.parse_options.read_precision = (ReadPrecisionMode) v;
				else if(svar == "assume_denominators_nonzero")
					evalops.assume_denominators_nonzero = v;	
				else if(svar == "angle_unit")
					evalops.angle_unit = (AngleUnit) v;
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
				else if(svar == "allow_complex")
					evalops.allow_complex = v;
				else if(svar == "allow_infinite")
					evalops.allow_infinite = v;
				else if(svar == "use_short_units")
					printops.abbreviate_names = v;
				else if(svar == "abbreviate_names")
					printops.abbreviate_names = v;	
				else if(svar == "all_prefixes_enabled")
					printops.use_all_prefixes = v;
				else if(svar == "denominator_prefix_enabled")
					printops.use_denominator_prefix = v;
				else if(svar == "auto_post_conversion")
					evalops.auto_post_conversion = (AutoPostConversion) v;
				else if(svar == "use_unicode_signs")
					printops.use_unicode_signs = v;	
				else if(svar == "lower_case_numbers")
					printops.lower_case_numbers = v;	
				else if(svar == "indicate_infinite_series")
					printops.indicate_infinite_series = v;
				else if(svar == "show_ending_zeroes")
					printops.show_ending_zeroes = v;
				else if(svar == "round_halfway_to_even")
					printops.round_halfway_to_even = v;	
				else if(svar == "approximation")
					evalops.approximation = (ApproximationMode) v;
				else if(svar == "in_rpn_mode")
					evalops.parse_options.rpn = v;
				else if(svar == "default_assumption_type")
					CALCULATOR->defaultAssumptions()->setNumberType((AssumptionNumberType) v);
				else if(svar == "default_assumption_sign")
					CALCULATOR->defaultAssumptions()->setSign((AssumptionSign) v);
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
	string filename = getLocalDir();
	mkdir(filename.c_str(), S_IRWXU);
	filename += "qalc.cfg";
	file = fopen(filename.c_str(), "w+");
	if(file == NULL) {
		printf(_("Couldn't write preferences to\n%s"), filename.c_str());
		return;
	}
	fprintf(file, "\n[General]\n");
	fprintf(file, "version=%s\n", VERSION);	
	fprintf(file, "save_mode_on_exit=%i\n", save_mode_on_exit);
	fprintf(file, "save_definitions_on_exit=%i\n", save_defs_on_exit);
	fprintf(file, "load_global_definitions=%i\n", load_global_defs);
	fprintf(file, "fetch_exchange_rates_at_startup=%i\n", fetch_exchange_rates_at_startup);
	fprintf(file, "spacious=%i\n", printops.spacious);
	fprintf(file, "excessive_parenthesis=%i\n", printops.excessive_parenthesis);
	fprintf(file, "short_multiplication=%i\n", printops.short_multiplication);
	fprintf(file, "use_unicode_signs=%i\n", printops.use_unicode_signs);
	fprintf(file, "lower_case_numbers=%i\n", printops.lower_case_numbers);
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
	fprintf(file, "abbreviate_names=%i\n", saved_printops.abbreviate_names);
	fprintf(file, "all_prefixes_enabled=%i\n", saved_printops.use_all_prefixes);
	fprintf(file, "denominator_prefix_enabled=%i\n", saved_printops.use_denominator_prefix);
	fprintf(file, "place_units_separately=%i\n", saved_printops.place_units_separately);
	fprintf(file, "auto_post_conversion=%i\n", saved_evalops.auto_post_conversion);	
	fprintf(file, "number_base=%i\n", saved_printops.base);
	fprintf(file, "number_base_expression=%i\n", saved_evalops.parse_options.base);
	fprintf(file, "read_precision=%i\n", saved_evalops.parse_options.read_precision);
	fprintf(file, "assume_denominators_nonzero=%i\n", saved_evalops.assume_denominators_nonzero);
	fprintf(file, "angle_unit=%i\n", saved_evalops.angle_unit);
	fprintf(file, "functions_enabled=%i\n", saved_evalops.parse_options.functions_enabled);
	fprintf(file, "variables_enabled=%i\n", saved_evalops.parse_options.variables_enabled);
	fprintf(file, "donot_calculate_variables=%i\n", !saved_evalops.calculate_variables);	
	fprintf(file, "unknownvariables_enabled=%i\n", saved_evalops.parse_options.unknowns_enabled);
	fprintf(file, "units_enabled=%i\n", saved_evalops.parse_options.units_enabled);
	fprintf(file, "allow_complex=%i\n", saved_evalops.allow_complex);
	fprintf(file, "allow_infinite=%i\n", saved_evalops.allow_infinite);
	fprintf(file, "indicate_infinite_series=%i\n", saved_printops.indicate_infinite_series);
	fprintf(file, "show_ending_zeroes=%i\n", saved_printops.show_ending_zeroes);
	fprintf(file, "round_halfway_to_even=%i\n", saved_printops.round_halfway_to_even);
	fprintf(file, "approximation=%i\n", saved_evalops.approximation);	
	fprintf(file, "in_rpn_mode=%i\n", saved_evalops.parse_options.rpn);
	fprintf(file, "default_assumption_type=%i\n", CALCULATOR->defaultAssumptions()->numberType());
	fprintf(file, "default_assumption_sign=%i\n", CALCULATOR->defaultAssumptions()->sign());
	
	fclose(file);
	
}

/*
	save definitions to ~/.qalculate/qalculate.cfg
	the hard work is done in the Calculator class
*/
void save_defs() {
	if(!CALCULATOR->saveDefinitions()) {
		printf(_("Couldn't write definitions"));
	}
}

