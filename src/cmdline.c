#include "calclib/Calculator.h"

int main (int argc, char *argv[]) {

	//hide annoying debug output for now (remove in next version)
	fclose(stdout);
	stdout = fopen("/dev/null", "w");

	Calculator *calc = new Calculator();

	calc->load("/usr/local/share/qalculate-gtk/qalculate.cfg");
	
	string str = "";
	for(int i = 1; i < argc; i++) {
		str += argv[i];
	}
	
	//stdout was redirected -- use stderr instead (bad solution, but works)
	fprintf(stderr, "%s = %s\n", str.c_str(), calc->calculate(str)->print().c_str());

	return 0;
}

