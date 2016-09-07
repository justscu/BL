#include "dym2.h"
#include "../staticx/staticx.h"
#include <stdio.h>

void dym2_f1() {
	DYTEST::get_data() -> print();
	fprintf(stdout, "dym2_f1[%p]\n", DYTEST::get_data()); 
}

void dym2_f2() {
    DYTEST::get_data2();
}
