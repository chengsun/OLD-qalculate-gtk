#include "calclib/Calculator.h"

int main (int argc, char *argv[]) {

	Calculator *calc = new Calculator();

	calc->load("/usr/local/share/qalculate-gtk/qalculate.cfg");
	
	string str = "";
	for(int i = 1; i < argc; i++) {
		str += argv[i];
	}
	
	printf("%s = %s\n", str.c_str(), calc->calculate(str)->print().c_str());

	return 0;
}

