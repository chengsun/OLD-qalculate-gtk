#include "calclib/qalculate.h"
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <malloc.h>
#include <stdio.h>
#include <vector>
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

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
bool expression_executed = false;

fd_set in_set;
struct timeval timeout;

char buffer[1000];

void *view_proc(void *pipe);
void execute_expression(bool goto_input = true);
void load_preferences();
bool save_preferences(bool mode = false);
bool save_mode();
void set_saved_mode();
bool save_defs();
void result_display_updated();
void result_format_updated();
void result_action_executed();
void result_prefix_changed(Prefix *prefix = NULL);
void expression_format_updated();

int s2b(const string &str) {
	if(str.empty()) return -1;
	if(str == "yes") return 1;
	if(str == "no") return 0;
	if(str == "Yes") return 1;
	if(str == "No") return 0;
	if(str == "true") return 1;
	if(str == "false") return 0;
	if(str == "True") return 1;
	if(str == "False") return 0;
	if(str == "on") return 1;
	if(str == "off") return 0;
	if(str == "On") return 1;
	if(str == "Off") return 0;
	if(str == _("yes")) return 1;
	if(str == _("no")) return 0;
	if(str == _("Yes")) return 1;
	if(str == _("No")) return 0;
	if(str == _("true")) return 1;
	if(str == _("false")) return 0;
	if(str == _("True")) return 1;
	if(str == _("False")) return 0;
	if(str == _("on")) return 1;
	if(str == _("off")) return 0;
	if(str == _("On")) return 1;
	if(str == _("Off")) return 0;
	int v = s2i(str);
	if(v > 0) return 1;
	return 0;
}

bool is_answer_variable(Variable *v) {
	return v == vans[0] || v == vans[1] || v == vans[2] || v == vans[3] || v == vans[4];
}

bool ask_question(const char *question) {
	fputs(question, stdout);
	while(true) {
#ifdef HAVE_LIBREADLINE
		char *rlbuffer = readline(" ");
		string str = rlbuffer;
		free(rlbuffer);
#else
		fputs(" ", stdout);
		fgets(buffer, 1000, stdin);
		string str = buffer;
#endif		
		remove_blank_ends(str);
		if(str == "y" || (str.length() == 1 && strlen(_("yes")) > 1 && str[0] == _("yes")[0]) || str == _("yes") || str == "yes") {
			return true;
		} else if(str == "n" || (str.length() == 1 && strlen(_("no")) > 1 && str[0] == _("no")[0]) || str == _("no") || str == "no") {	
			return false;
		} else {
			fputs(_("Please answer yes or no"), stdout);
			fputs(":", stdout);
		}
	}
}

void set_assumption(const string &str, bool first_of_two = false) {
	if(str == "unknown" || str == _("unknown")) {
		if(first_of_two) {
			CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_UNKNOWN);
		} else {
			CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_NONE);
		}
	} else if(str == "none" || str == _("none")) {
		CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_NONE);
	} else if(str == "complex" || str == _("complex")) {
		CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_COMPLEX);
	} else if(str == "real" || str == _("real")) {
		CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_REAL);
	} else if(str == "number" || str == _("number")) {
		CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_NUMBER);
	} else if(str == "rational" || str == _("rational")) {
		CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_RATIONAL);
	} else if(str == "integer" || str == _("integer")) {
		CALCULATOR->defaultAssumptions()->setNumberType(ASSUMPTION_NUMBER_INTEGER);
	} else if(str == "non-zero" || str == _("non-zero")) {
		CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONZERO);
	} else if(str == "positive" || str == _("positive")) {
		CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_POSITIVE);
	} else if(str == "non-negative" || str == _("non-negative")) {
		CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONNEGATIVE);
	} else if(str == "negative" || str == _("negative")) {
		CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NEGATIVE);
	} else if(str == "non-positive" || str == _("non-positive")) {
		CALCULATOR->defaultAssumptions()->setSign(ASSUMPTION_SIGN_NONPOSITIVE);
	} else {
		puts(_("Unrecognized assumption."));
	}
}

#define SET_BOOL_D(x)	{int v = s2b(svalue); if(v < 0) {puts(_("Illegal value"));} else {x = v; result_display_updated();}}
#define SET_BOOL_E(x)	{int v = s2b(svalue); if(v < 0) {puts(_("Illegal value"));} else {x = v; expression_format_updated();}}

vector<const string*> matches;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_LIBREADLINE

char *qalc_completion(const char *text, int index) {
	if(index == 0) {
		if(strlen(text) < 1) return NULL;
		matches.clear();
		const string *str;
		bool b_match;
		unsigned int l = strlen(text);
		for(unsigned int i = 0; i < CALCULATOR->functions.size(); i++) {
			if(CALCULATOR->functions[i]->isActive()) {
				str = &CALCULATOR->functions[i]->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs).name;
				if(l <= str->length()) {
					b_match = true;
					for(unsigned int i2 = 0; i2 < l; i2++) {
						if((*str)[i2] != text[i2]) {
							b_match = false;
							break;
						}
					}
					if(b_match) {
						matches.push_back(str);
					}
				}
			}
		}
		for(unsigned int i = 0; i < CALCULATOR->variables.size(); i++) {
			if(CALCULATOR->variables[i]->isActive()) {
				str = &CALCULATOR->variables[i]->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs).name;
				if(l <= str->length()) {
					b_match = true;
					for(unsigned int i2 = 0; i2 < l; i2++) {
						if((*str)[i2] != text[i2]) {
							b_match = false;
							break;
						}
					}
					if(b_match) {
						matches.push_back(str);
					}
				}
			}
		}
		for(unsigned int i = 0; i < CALCULATOR->units.size(); i++) {
			if(CALCULATOR->units[i]->isActive() && CALCULATOR->units[i]->subtype() != SUBTYPE_COMPOSITE_UNIT) {
				str = &CALCULATOR->units[i]->preferredInputName(printops.abbreviate_names, printops.use_unicode_signs).name;
				if(l <= str->length()) {
					b_match = true;
					for(unsigned int i2 = 0; i2 < l; i2++) {
						if((*str)[i2] != text[i2]) {
							b_match = false;
							break;
						}
					}
					if(b_match) {
						matches.push_back(str);
					}
				}
			}
		}
	}
	if(index >= 0 && index < (int) matches.size()) {
		char *cstr = (char*) malloc(sizeof(char) *matches[index]->length() + 1);
		strcpy(cstr, matches[index]->c_str());
		return cstr;
	}
	return NULL;
}

#endif

#ifdef __cplusplus
}
#endif


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
#ifdef HAVE_LIBREADLINE	
	static char* rlbuffer;
#endif

	
	//exchange rates
	if(first_qalculate_run && canfetch) {
		if(ask_question(_("You need the download exchange rates to be able to convert between different currencies.\nYou can later get current exchange rates with the \"exchange rates\" command.\nDo you want to fetch exchange rates now from the Internet (default yes)?"))) {
			CALCULATOR->fetchExchangeRates(5);
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

#ifdef HAVE_LIBREADLINE	
	rl_readline_name = "qalc";
	rl_basic_word_break_characters = NOT_IN_NAMES NUMBERS;
	rl_completion_entry_function = qalc_completion;
#endif
	
	string svar, svalue;
	unsigned int index, slen;
	
	while(true) {
#ifdef HAVE_LIBREADLINE			
		rlbuffer = readline("> ");
		str = rlbuffer;
#else
		fputs("> ", stdout);
		fgets(buffer, 1000, stdin);
		str = buffer;
#endif				
		slen = str.length();
		remove_blank_ends(str);
		if(slen > 4 && str.substr(0, 4) == "set ") {
			str = str.substr(4, str.length() - 4);
			remove_blank_ends(str);
			svalue = "";
			if((index = str.find_first_of(SPACES)) != string::npos) {
				svalue = str.substr(index + 1, str.length() - (index + 1));
				remove_blank_ends(svalue);
			}
			svar = str.substr(0, index);
			remove_blank_ends(svar);
			if(svar == "base" || svar == "inbase" || svar == "outbase") {
				set_base:
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
				} else if(b_in) {
					evalops.parse_options.base = v;
					expression_format_updated();
				} else {
					printops.base = v;
					result_format_updated();
				}
			} else if(svar == "assumptions") {
				set_assumptions:
				unsigned int i = svalue.find_first_of(SPACES);
				if(i != string::npos) {
					set_assumption(svalue.substr(i + 1, svalue.length() - (i + 1)), true);
					set_assumption(svalue.substr(0, i), false);
				} else {
					set_assumption(svalue, false);
				}
				expression_format_updated();
			}
			else if(svar == "all_prefixes") SET_BOOL_D(printops.use_all_prefixes)
			else if(svar == "complex") SET_BOOL_E(evalops.allow_complex)
			else if(svar == "excessive_parenthises") SET_BOOL_D(printops.excessive_parenthesis)
			else if(svar == "functions") SET_BOOL_E(evalops.parse_options.functions_enabled)
			else if(svar == "infinite") SET_BOOL_E(evalops.allow_infinite)
			else if(svar == "negative_exponents") SET_BOOL_D(printops.negative_exponents)
			else if(svar == "nonzero_denominators") SET_BOOL_E(evalops.assume_denominators_nonzero)
			else if(svar == "prefixes") SET_BOOL_D(printops.use_unit_prefixes)
			else if(svar == "round_to_even") SET_BOOL_D(printops.round_halfway_to_even)
			else if(svar == "rpn") SET_BOOL_E(evalops.parse_options.rpn)
			else if(svar == "short_multiplication") SET_BOOL_D(printops.short_multiplication)
			else if(svar == "spacious") SET_BOOL_D(printops.spacious)
			else if(svar == "unicode") SET_BOOL_D(printops.use_unicode_signs)
			else if(svar == "units") SET_BOOL_E(evalops.parse_options.units_enabled)
			else if(svar == "unknowns") SET_BOOL_E(evalops.parse_options.unknowns_enabled)
			else if(svar == "variables") SET_BOOL_E(evalops.parse_options.variables_enabled)
			else if(svar == "abbreviations") SET_BOOL_D(printops.abbreviate_names)
			else if(svar == "ending_zeroes") SET_BOOL_D(printops.show_ending_zeroes)
			else if(svar == "infinite_series") SET_BOOL_D(printops.indicate_infinite_series)
			else if(svar == "angleunit") {
				int v = -1;
				if(svalue == "rad") v = RADIANS;
				else if(svalue == "deg") v = DEGREES;
				else if(svalue == "gra") v = GRADIANS;
				else {
					v = s2i(svalue);
				}
				if(v < 0 || v > 2) {
					puts(_("Illegal value."));
				} else {
					evalops.angle_unit = (AngleUnit) v;
					expression_format_updated();
				}
			} else if(svar == "approximation") {
				int v = -1;
				if(svalue == "exact") v = APPROXIMATION_EXACT;
				else if(svalue == "try exact") v = APPROXIMATION_TRY_EXACT;
				else if(svalue == "approximate") v = APPROXIMATION_APPROXIMATE;
				else {
					v = s2i(svalue);
				}
				if(v < 0 || v > 2) {
					puts(_("Illegal value."));
				} else {
					evalops.approximation = (ApproximationMode) v;
					expression_format_updated();
				}
			} else if(svar == "autoconversion") {
				int v = -1;
				if(svalue == "none") v = POST_CONVERSION_NONE;
				else if(svalue == "best") v = POST_CONVERSION_BEST;
				else if(svalue == "base") v = POST_CONVERSION_BASE;
				else {
					v = s2i(svalue);
				}
				if(v < 0 || v > 2) {
					puts(_("Illegal value."));
				} else {
					evalops.auto_post_conversion = (AutoPostConversion) v;
					expression_format_updated();
				}
			} else if(svar == "exact") {
				int v = s2b(svalue); 
				if(v < 0) {
					puts(_("Illegal value")); 
				} else if(v > 0) {
					evalops.approximation = APPROXIMATION_EXACT; 
					expression_format_updated();
				} else {
					evalops.approximation = APPROXIMATION_TRY_EXACT; 
					expression_format_updated();
				}
			} else if(svar == "save_mode") {
				int v = s2b(svalue); 
				if(v < 0) {
					puts(_("Illegal value")); 
				} else if(v > 0) {
					save_mode_on_exit = true;
				} else {
					save_mode_on_exit = false;
				}
			} else if(svar == "save_definitions") {
				int v = s2b(svalue); 
				if(v < 0) {
					puts(_("Illegal value")); 
				} else if(v > 0) {
					save_defs_on_exit = true;
				} else {
					save_defs_on_exit = false;
				}
			} else if(svar == "expmode") {
				int v = -2;
				if(svalue == "off") v = EXP_NONE;
				else if(svalue == "default") v = EXP_PRECISION;
				else if(svalue == "pure") v = EXP_PURE;
				else if(svalue == "scientific") v = EXP_SCIENTIFIC;
				else {
					v = s2i(svalue);
					if(v < 0) v = -2;
				}
				if(v < -2) {
					puts(_("Illegal value."));
				} else {
					printops.min_exp = v;
					result_format_updated();
				}
			} else if(svar == "precision") {
				int v = s2i(svalue);
				if(v < 1) {
					puts(_("Illegal value."));
				} else {
					CALCULATOR->setPrecision(v);
					expression_format_updated();
				}
			} else if(svar == "max_decimals") {
				int v = -1;
				if(svalue == "off") v = -1;
				else v = s2i(svalue);
				if(v < 0) {
					printops.use_max_decimals = false;
					result_format_updated();
				} else {
					printops.max_decimals = v;
					printops.use_max_decimals = true;
					result_format_updated();
				}
			} else if(svar == "min_decimals") {
				int v = -1;
				if(svalue == "off") v = -1;
				else v = s2i(svalue);
				if(v < 0) {
					printops.min_decimals = 0;
					printops.use_min_decimals = false;
					result_format_updated();
				} else {
					printops.min_decimals = v;
					printops.use_min_decimals = true;
					result_format_updated();
				}
			} else if(svar == "fractions") {
				int v = -1;
				if(svalue == "off") v = FRACTION_DECIMAL;
				else if(svalue == "exact") v = FRACTION_DECIMAL_EXACT;
				else if(svalue == "on") v = FRACTION_FRACTIONAL;
				else if(svalue == "combined") v = FRACTION_COMBINED;
				else {
					v = s2i(svalue);
				}
				if(v < 0) {
					puts(_("Illegal value."));
				} else {
					printops.number_fraction_format = (NumberFractionFormat) v;
					result_format_updated();
				}
			} else if(svar == "read_precision") {
				int v = -1;
				if(svalue == "off") v = DONT_READ_PRECISION;
				else if(svalue == "always") v = ALWAYS_READ_PRECISION;
				else if(svalue == "when decimals") v = READ_PRECISION_WHEN_DECIMALS;
				else {
					v = s2i(svalue);
				}
				if(v < 0) {
					puts(_("Illegal value."));
				} else {
					evalops.parse_options.read_precision = (ReadPrecisionMode) v;
					expression_format_updated();
				}
			}
		} else if((slen > 5 && str.substr(0, 5) == "save ") || (slen > 6 && str.substr(0, 6) == "store ")) {
			if(str.substr(0, 6) == "store ") {
				str = str.substr(6, str.length() - 6);
			} else {
				str = str.substr(5, str.length() - 5);
			}
			remove_blank_ends(str);
			if(str == "mode") {
				if(save_mode()) {
					puts(_("mode saved"));
				}
			} else if(str == "definitions") {
				if(save_defs()) {
					puts(_("definitions saved"));
				}
			} else {
				string name = str, cat, title;
				if(str[0] == '\"') {
					unsigned int i = str.find('\"', 1);
					if(i != string::npos) {
						name = str.substr(1, i - 1);
						str = str.substr(i + 1, str.length() - (i + 1));
						remove_blank_ends(str);
					} else {
						str = "";
					}
				} else {
					unsigned int i = str.find_first_of(SPACES, 1);
					if(i != string::npos) {
						name = str.substr(0, i);
						str = str.substr(i + 1, str.length() - (i + 1));
						remove_blank_ends(str);
					} else {
						str = "";
					}
				}
				bool catset = false;
				if(str.empty()) {
					cat = _("Temporary");
				} else {
					if(str[0] == '\"') {
						unsigned int i = str.find('\"', 1);
						if(i != string::npos) {
							cat = str.substr(1, i - 1);
							title = str.substr(i + 1, str.length() - (i + 1));
							remove_blank_ends(title);
						}
					} else {
						unsigned int i = str.find_first_of(SPACES, 1);
						if(i != string::npos) {
							cat = str.substr(0, i);
							title = str.substr(i + 1, str.length() - (i + 1));
							remove_blank_ends(title);
						}
					}
					catset = true;
				}
				bool b = true;
				if(!CALCULATOR->variableNameIsValid(name)) {
					name = CALCULATOR->convertToValidVariableName(name);
					unsigned int l = name.length() + strlen(_("Illegal name. Save as %s instead?"));
					char cstr[l];
					snprintf(cstr, l, _("Illegal name. Save as %s instead?"), name.c_str());
					if(!ask_question(cstr)) {
						b = false;
					}
				}
				if(b && CALCULATOR->variableNameTaken(name)) {
					if(!ask_question(_("An unit or variable with the same name already exists.\nDo you want to overwrite it?"))) {
						b = false;
					}
				}
				if(b) {
					Variable *v = CALCULATOR->getActiveVariable(name);
					if(v && v->isLocal() && v->isKnown()) {
						if(catset) v->setCategory(cat);
						if(!title.empty()) v->setTitle(title);
						((KnownVariable*) v)->set(*mstruct);
						if(v->countNames() == 0) {
							ExpressionName ename(name);
							ename.reference = true;
							v->setName(ename, 1);
						} else {
							v->setName(name, 1);
						}
					} else {
						CALCULATOR->addVariable(new KnownVariable(cat, name, *mstruct, title));
					}
				}
			}
		} else if(slen > 7 && str.substr(0, 7) == "assume ") {
			svalue = str.substr(7, str.length() - 7);
			remove_blank_ends(svalue);
			svar = "assumptions";
			goto set_assumptions;
		} else if(slen > 5 && str.substr(0, 5) == "base ") {
			svalue = str.substr(5, str.length() - 5);
			remove_blank_ends(svalue);
			svar = "base";
			goto set_base;
		} else if(str == "exact") {
			if(evalops.approximation != APPROXIMATION_EXACT) {
				evalops.approximation = APPROXIMATION_EXACT;
				expression_format_updated();
			}
		} else if(str == "approximate") {
			if(evalops.approximation != APPROXIMATION_TRY_EXACT) {
				evalops.approximation = APPROXIMATION_TRY_EXACT;
				expression_format_updated();
			}
		} else if(slen > 8 && str.substr(0, 8) == "convert ") {
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
		} else if(str == "mode") {
			puts("");
			printf("abbreviations:\t\t"); puts(b2oo(printops.abbreviate_names, false));
			printf("all_prefixes:\t\t"); puts(b2oo(printops.use_all_prefixes, false));
			printf("angleunit:\t\t"); 
			switch(evalops.angle_unit) {
				case RADIANS: {puts("rad"); break;}
				case DEGREES: {puts("rad"); break;}
				case GRADIANS: {puts("gra"); break;}
			}
			printf("approximation:\t\t"); 
			switch(evalops.approximation) {
				case APPROXIMATION_EXACT: {puts("exact"); break;}
				case APPROXIMATION_TRY_EXACT: {puts("try exact"); break;}
				case APPROXIMATION_APPROXIMATE: {puts("approximate"); break;}
			}
			string value;
			switch(CALCULATOR->defaultAssumptions()->sign()) {
				case ASSUMPTION_SIGN_POSITIVE: {value = _("positive"); break;}
				case ASSUMPTION_SIGN_NONPOSITIVE: {value = _("non-positive"); break;}
				case ASSUMPTION_SIGN_NEGATIVE: {value = _("negative"); break;}
				case ASSUMPTION_SIGN_NONNEGATIVE: {value = _("non-negative"); break;}
				case ASSUMPTION_SIGN_NONZERO: {value = _("non-zero"); break;}
				default: {}
			}
			if(!value.empty() && !CALCULATOR->defaultAssumptions()->numberType() == ASSUMPTION_NUMBER_NONE) value += " ";
			switch(CALCULATOR->defaultAssumptions()->numberType()) {
				case ASSUMPTION_NUMBER_INTEGER: {value += _("integer"); break;}
				case ASSUMPTION_NUMBER_RATIONAL: {value += _("rational"); break;}
				case ASSUMPTION_NUMBER_REAL: {value += _("real"); break;}
				case ASSUMPTION_NUMBER_COMPLEX: {value += _("complex"); break;}
				case ASSUMPTION_NUMBER_NUMBER: {value += _("number"); break;}
				default: {}
			}
			if(value.empty()) value = _("unknown");
			printf("assumptions:\t\t"); puts(value.c_str());
			printf("autoconversion:\t\t"); 
			switch(evalops.auto_post_conversion) {
				case POST_CONVERSION_NONE: {puts("none"); break;}
				case POST_CONVERSION_BEST: {puts("best"); break;}
				case POST_CONVERSION_BASE: {puts("base"); break;}
			}
			printf("base:\t\t\t"); 
			switch(printops.base) {
				case BASE_ROMAN_NUMERALS: {puts("roman"); break;}
				case BASE_SEXAGESIMAL: {puts("sexagesimal"); break;}
				case BASE_TIME: {puts("time"); break;}
				default: {printf("%i\n", printops.base);}
			}
			printf("complex:\t\t"); puts(b2oo(evalops.allow_complex, false));
			printf("ending_zeroes:\t\t"); puts(b2oo(printops.show_ending_zeroes, false));
			printf("excessive_parenthesis:\t"); puts(b2oo(printops.excessive_parenthesis, false));
			printf("expmode:\t\t"); 
			switch(printops.min_exp) {
				case EXP_NONE: {puts("off"); break;}
				case EXP_PRECISION: {puts("default"); break;}
				case EXP_PURE: {puts("pure"); break;}
				case EXP_SCIENTIFIC: {puts("scientific"); break;}
				default: {printf("%i\n", printops.min_exp); break;}
			}
			printf("fractions:\t\t"); 
			switch(printops.number_fraction_format) {
				case FRACTION_DECIMAL: {puts("off"); break;}
				case FRACTION_DECIMAL_EXACT: {puts("exact"); break;}
				case FRACTION_FRACTIONAL: {puts("on"); break;}
				case FRACTION_COMBINED: {puts("combined"); break;}
			}
			printf("functions:\t\t"); puts(b2oo(evalops.parse_options.functions_enabled, false));
			printf("inbase:\t\t\t"); 
			switch(evalops.parse_options.base) {
				case BASE_ROMAN_NUMERALS: {puts("roman"); break;}
				default: {printf("%i\n", evalops.parse_options.base);}
			}
			printf("infinite:\t\t"); puts(b2oo(evalops.allow_infinite, false));
			printf("infinite_series:\t"); puts(b2oo(printops.indicate_infinite_series, false));
			printf("max_decimals:\t\t"); 
			if(printops.use_max_decimals && printops.max_decimals >= 0) {
				printf("%i\n", printops.max_decimals);
			} else {
				puts("off");
			}
			printf("min_decimals:\t\t"); 
			if(printops.use_min_decimals && printops.min_decimals > 0) {
				printf("%i\n", printops.min_decimals);
			} else {
				puts(_("off"));
			}
			printf("negative_exponents:\t"); puts(b2oo(printops.negative_exponents, false));
			printf("nonzero_denominators:\t"); puts(b2oo(evalops.assume_denominators_nonzero, false));
			printf("precision:\t\t%i\n", CALCULATOR->getPrecision());
			printf("prefixes:\t\t"); puts(b2oo(printops.use_unit_prefixes, false));
			printf("read_precision:\t\t"); 
			switch(evalops.parse_options.read_precision) {
				case DONT_READ_PRECISION: {puts("off"); break;}
				case ALWAYS_READ_PRECISION: {puts("always"); break;}
				case READ_PRECISION_WHEN_DECIMALS: {puts("when decimals"); break;}
			}
			printf("round_to_even:\t\t"); puts(b2oo(printops.round_halfway_to_even, false));
			printf("rpn:\t\t\t"); puts(b2oo(evalops.parse_options.rpn, false));
			printf("save_definitions:\t"); puts(b2yn(save_defs_on_exit, false));
			printf("save_mode:\t\t"); puts(b2yn(save_mode_on_exit, false));
			printf("short_multiplication:\t"); puts(b2oo(printops.short_multiplication, false));
			printf("spacious:\t\t"); puts(b2oo(printops.spacious, false));
			printf("unicode:\t\t"); puts(b2oo(printops.use_unicode_signs, false));
			printf("units:\t\t\t"); puts(b2oo(evalops.parse_options.units_enabled, false));
			printf("unknowns:\t\t"); puts(b2oo(evalops.parse_options.unknowns_enabled, false));
			printf("variables:\t\t"); puts(b2oo(evalops.parse_options.variables_enabled, false));
			puts("");
		} else if(str == "help" || str == _("help") || str == "?") {
			puts("");
			puts(_("Enter a mathematical expression or a command."));
			puts("");
			puts(_("Available commands are:"));
			puts("");
			puts("approximate");
			puts("assume ASSUMPTIONS");
			puts("base BASE");
			puts("exact");
			puts("factor");
			puts("info");
			puts("mode");
			puts("set OPTION VALUE");
			puts("save/store NAME [CATEGORY] [TITLE]");
			puts("convert UNIT");
			puts("quit/exit");
			puts("");
			puts(_("Type help COMMAND for more help (example: help save)."));
			puts(_("Type help NAME for info about a function, variable or unit (example: help sin)."));
			puts("");
		} else if(slen > 5 && str.substr(0, 5) == "info ") {
			str = str.substr(5, str.length() - 5);
			remove_blank_ends(str);
			show_info:
			ExpressionItem *item = CALCULATOR->getActiveExpressionItem(str);
			if(!item) {
				puts(_("No function, variable or unit with specified name exist."));
			} else {
				switch(item->type()) {
					case TYPE_FUNCTION: {
						puts("");
						MathFunction *f = (MathFunction*) item;
						Argument *arg;
						Argument default_arg;
						str = "";
						string str2;
						if(!f->title().empty()) {
							puts(f->title().c_str());
							puts("");
						}
						const ExpressionName *ename = &f->preferredName(false, printops.use_unicode_signs);
						str += ename->name;
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
						for(unsigned int i2 = 1; i2 <= f->countNames(); i2++) {
							if(&f->getName(i2) != ename) {
								str += "\n";
								str += f->getName(i2).name;
							}
						}
						fputs(str.c_str(), stdout);
						str = "";
						str += "\n";
						if(f->subtype() == SUBTYPE_DATA_SET) {
							str += "\n";
							snprintf(buffer, 1000, _("Retrieves data from the %s data set for a given object and property. If \"info\" is typed as property, all properties of the object will be listed."), f->title().c_str());
							str += buffer;
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
						fputs(str.c_str(), stdout);
						if(iargs) {
							str = "\n";
							str += _("Arguments");
							str += "\n";
							fputs(str.c_str(), stdout);
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
								fputs(str.c_str(), stdout);
								fputs(str2.c_str(), stdout);
							}
						}
						if(!f->condition().empty()) {
							str = "\n";
							str += _("Requirement");
							str += ": ";
							str += f->printCondition();
							str += "\n";
							fputs(str.c_str(), stdout);
						}
						if(f->subtype() == SUBTYPE_DATA_SET) {
							DataSet *ds = (DataSet*) f;
							str = "\n";
							str += _("Properties");
							str += "\n";
							fputs(str.c_str(), stdout);
							DataPropertyIter it;
							DataProperty *dp = ds->getFirstProperty(&it);
							while(dp) {	
								if(!dp->isHidden()) {
									if(!dp->title(false).empty()) {
										str = dp->title();	
										str += ": ";
									}
									for(unsigned int i = 1; i <= dp->countNames(); i++) {
										if(i > 1) str += ", ";
										str += dp->getName(i);
									}
									if(dp->isKey()) {
										str += " (";
										str += _("key");
										str += ")";
									}
									str += "\n";
									fputs(str.c_str(), stdout);
									if(!dp->description().empty()) {
										str = dp->description();
										str += "\n";
										fputs(str.c_str(), stdout);
									}
								}
								dp = ds->getNextProperty(&it);
							}
						}
						puts("");
						break;
					}
					case TYPE_UNIT: {
						puts("");
						if(!item->title().empty()) {
							puts(item->title().c_str());
							puts("");
						}
						if(item->subtype() != SUBTYPE_COMPOSITE_UNIT) {
							const ExpressionName *ename = &item->preferredName(true, printops.use_unicode_signs);
							fputs(ename->name.c_str(), stdout);
							for(unsigned int i2 = 1; i2 <= item->countNames(); i2++) {
								if(&item->getName(i2) != ename) {
									fputs(" / ", stdout);
									fputs(item->getName(i2).name.c_str(), stdout);
								}
							}
						}
						fputs("\n", stdout);
						fputs("\n", stdout);
						switch(item->subtype()) {
							case SUBTYPE_BASE_UNIT: {
								fputs(_("base unit"), stdout);
								break;
							}
							case SUBTYPE_ALIAS_UNIT: {
								AliasUnit *au = (AliasUnit*) item;
								fputs(_("Base Unit"), stdout);
								fputs(": ", stdout);
								fputs(au->firstBaseUnit()->preferredDisplayName(printops.abbreviate_names, printops.use_unicode_signs).name.c_str(), stdout);
								if(au->firstBaseExp() != 1) {
									fputs(POWER, stdout);
									printf("%i", au->firstBaseExp());
								}
								fputs("\n", stdout);
								fputs(_("Relation"), stdout);
								fputs(": ", stdout);
								fputs(CALCULATOR->localizeExpression(au->expression()).c_str(), stdout);
								if(item->isApproximate()) {
									fputs(" (", stdout);
									fputs(_("approximate"), stdout);
									fputs(")", stdout);
									
								}
								if(!au->reverseExpression().empty()) {
									fputs("\n", stdout);
									fputs(_("Reversed Relation"), stdout);
									fputs(": ", stdout);
									fputs(CALCULATOR->localizeExpression(au->reverseExpression()).c_str(), stdout);
									if(item->isApproximate()) {
										fputs(" (", stdout);
										fputs(_("approximate"), stdout);
										fputs(")", stdout);
									}
								}
								break;
							}
							case SUBTYPE_COMPOSITE_UNIT: {
								fputs(_("Base Units"), stdout);
								fputs(": ", stdout);
								fputs(((CompositeUnit*) item)->print(false, true, printops.use_unicode_signs).c_str(), stdout);
								break;
							}
						}
						fputs("\n", stdout);						
						if(!item->description().empty()) {
							fputs("\n", stdout);
							fputs(item->description().c_str(), stdout);
							fputs("\n", stdout);
						}
						puts("");
						break;
					}
					case TYPE_VARIABLE: {
						puts("");
						if(!item->title().empty()) {
							puts(item->title().c_str());
							puts("");
						}
						const ExpressionName *ename = &item->preferredName(false, printops.use_unicode_signs);
						fputs(ename->name.c_str(), stdout);
						for(unsigned int i2 = 1; i2 <= item->countNames(); i2++) {
							if(&item->getName(i2) != ename) {
								fputs(" / ", stdout);
								fputs(item->getName(i2).name.c_str(), stdout);
							}
						}
						Variable *v = (Variable*) item;
						string value;
						if(is_answer_variable(v)) {
							value = _("a previous result");
						} else if(v->isKnown()) {
							if(((KnownVariable*) v)->isExpression()) {
								value = CALCULATOR->localizeExpression(((KnownVariable*) v)->expression());
								if(value.length() > 40) {
									value = value.substr(0, 30);
									value += "...";
								}
							} else {
								if(((KnownVariable*) v)->get().isMatrix()) {
									value = _("matrix");
								} else if(((KnownVariable*) v)->get().isVector()) {
									value = _("vector");
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
						fputs("\n", stdout);
						fputs("\n", stdout);
						fputs(_("Value"), stdout);
						fputs(": ", stdout);
						fputs(value.c_str(), stdout);
						if(item->isApproximate()) {
							fputs(" (", stdout);
							fputs(_("approximate"), stdout);
							fputs(")", stdout);
						}
						fputs("\n", stdout);
						if(!item->description().empty()) {
							fputs("\n", stdout);
							fputs(item->description().c_str(), stdout);
							fputs("\n", stdout);
						}
						puts("");
						break;
					}
				}
			}
		} else if(slen > 5 && str.substr(0, 5) == "help ") {
			str = str.substr(5, str.length() - 5);
			remove_blank_ends(str);
			if(str == "factor") {
				puts("");
				puts(_("Factorize the current result."));
				puts("");
			} else if(str == "set") {
				puts("");
				puts(_("Sets the value of an option."));
				puts("");
				puts(_("Available options and accepted values are:"));
				puts("");
				puts("abbreviations\t\t(on, off)");
				puts("all_prefixes\t\t(on, off)");
				puts("angleunit\t\t(0 = rad, 1 = deg, 2 = gra)");
				puts("approximation\t\t(0 = exact, 1 = try exact, 2 = approximate)");
				puts("assumptions\t\t(unknown, non-zero, positive, negative, non-positive, non-negative / unknown, number, complex, real, rational, integer)");
				puts("autoconversion\t\t(0 = none, 1 = best, 2 = base)");
				puts("base\t\t\t(2 - 36, bin, oct, dec, hex, sex, time, roman)");
				puts("complex\t\t\t(on, off)");
				puts("ending_zeroes\t\t(on, off)");
				puts("exact\t\t\t(on, off)");
				puts("excessive_parenthesis\t(on, off)");
				puts("expmode\t\t\t(off, default, pure, scientific, >= 0)");
				puts("fractions\t\t(0 = off, 1 = exact, 2 = on, 3 = combined)");
				puts("functions\t\t(on, off)");
				puts("inbase\t\t\t(2 - 36, bin, oct, dec, hex, roman)");
				puts("infinite\t\t(on, off)");
				puts("infinite_series\t\t(on, off)");
				puts("max_decimals\t\t(off, >= 0)");
				puts("min_decimals\t\t(off, >= 0)");
				puts("negative_exponents\t(on, off)");
				puts("nonzero_denominators\t(on, off)");
				puts("precision\t\t(> 0)");
				puts("prefixes\t\t(on, off)");
				puts("read_precision\t\t(0 = off, 1 = always, 2 = on = when decimals)");
				puts("round_to_even\t\t(on, off)");
				puts("rpn\t\t\t(on, off)");
				puts("save_definitions\t(yes, no)");
				puts("save_mode\t\t(yes, no)");
				puts("short_multiplication\t(on, off)");
				puts("spacious\t\t(on, off)");
				puts("unicode\t\t\t(on, off)");
				puts("units\t\t\t(on, off)");
				puts("unknowns\t\t(on, off)");
				puts("variables\t\t(on, off)");
				puts("");
				puts(_("Example: set base 16."));
				puts("");
			} else if(str == "save" || str == "store") {
				puts("");
				puts(_("Saves the current result in a variable with the specified name. You may optionally also provide a category (default \"Temporary\") and a title."));
				puts(_("If name equals \"mode\" or \"definitions\", the current mode and definitions, respectively, will be saved."));
				puts("");
				puts(_("Example: store var1."));
				puts("");
			} else if(str == "mode") {
				puts("");
				puts(_("Display the current mode."));
				puts("");
			} else if(str == "info") {
				puts("");
				puts(_("Displays information about a function, variable or unit."));
				puts("");
				puts(_("Example: info sin."));
				puts("");
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
			} else if(str == "quit" || str == "exit") {
				puts("");
				puts("Terminates this program.");
				puts("");
			} else {
				goto show_info;
			}
		} else if(str == "quit" || str == "exit") {
#ifdef HAVE_LIBREADLINE				
			free(rlbuffer);
#endif			
			break;
		} else {
			expression_str = str;
			execute_expression();
		}
#ifdef HAVE_LIBREADLINE				
		add_history(rlbuffer);		
		free(rlbuffer);
#endif
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
#ifdef HAVE_LIBREADLINE	
	int c = 0;
#else
	char c = 0;
#endif
	rtime.tv_nsec = 100000000;
	while(b_busy) {
		FD_ZERO(&in_set);
		FD_SET(STDIN_FILENO, &in_set);
		if(select(FD_SETSIZE, &in_set, NULL, NULL, &timeout) > 0) {
#ifdef HAVE_LIBREADLINE		
			c = rl_read_key();
#else
			read(STDIN_FILENO, &c, 1);
#endif			
			if(c == '\n') {
				on_abort_display();
			}
		} else {
			printf(".");
			fflush(stdout);
			nanosleep(&rtime, NULL);
		}
	}
	
	b_busy = true;
	
	if(has_printed) printf("\n");
	printf("\n  ");
	
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
	if(goto_input) printf("\n");
	printops.prefix = NULL;
	b_busy = false;
}

void viewresult(Prefix *prefix = NULL) {
	setResult(prefix);
}

void result_display_updated() {
	if(expression_executed) setResult(NULL, false);
}
void result_format_updated() {
	if(expression_executed) setResult(NULL, false);
}
void result_action_executed() {
	if(expression_executed) setResult(NULL, false);
}
void result_prefix_changed(Prefix *prefix) {
	if(expression_executed) setResult(prefix, false);
}
void expression_format_updated() {
	if(expression_executed) execute_expression();
}

void execute_expression(bool goto_input) {

	string str = expression_str;
	
	expression_executed = true;

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
#ifdef HAVE_LIBREADLINE		
	int c = 0;
#else
	char c = 0;
#endif
	rtime.tv_nsec = 100000000;
	while(CALCULATOR->busy()) {
		FD_ZERO(&in_set);
		FD_SET(STDIN_FILENO, &in_set);
		if(select(FD_SETSIZE, &in_set, NULL, NULL, &timeout) > 0) {
#ifdef HAVE_LIBREADLINE		
			c = rl_read_key();
#else
			read(STDIN_FILENO, &c, 1);
#endif			
			if(c == '\n') {
				on_abort_display();
			}
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
bool save_mode() {
	return save_preferences(true);
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
#ifdef HAVE_LIBREADLINE		
	string historyfile = filename;
	historyfile += "qalc.history";
	read_history(historyfile.c_str());
#endif	
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
bool save_preferences(bool mode)
{
	FILE *file = NULL;
	string filename = getLocalDir();
	mkdir(filename.c_str(), S_IRWXU);
#ifdef HAVE_LIBREADLINE			
	string historyfile = filename;
	historyfile += "qalc.history";
	write_history(historyfile.c_str());
#endif	
	filename += "qalc.cfg";
	file = fopen(filename.c_str(), "w+");
	if(file == NULL) {
		fprintf(stderr, _("Couldn't write preferences to\n%s"), filename.c_str());
		return false;
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
	return true;
	
}

/*
	save definitions to ~/.qalculate/qalculate.cfg
	the hard work is done in the Calculator class
*/
bool save_defs() {
	if(!CALCULATOR->saveDefinitions()) {
		printf(_("Couldn't write definitions"));
		return false;
	}
	return true;
}



