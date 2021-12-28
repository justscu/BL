#include "dym1.h"
#include "../staticx/staticx.h"
#include <stdio.h>

void dym1_f1() {
	DYTEST::get_data() -> print();
	fprintf(stdout, "dym1_f1[%p]\n", DYTEST::get_data());
}

void dym1_f2() {
	DYTEST::get_data2();
}
