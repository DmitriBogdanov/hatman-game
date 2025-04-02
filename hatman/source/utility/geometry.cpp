#include "utility/geometry.h"



bool rand_bool() {
	return static_cast<bool>(rand() % 2);
}

int rand_int(int min, int max) {
	return min + rand() % (max - min + 1);
}

double rand_double() {
	return rand() / (RAND_MAX + 1.);
}

double rand_double(double min, double max) {
	return min + (max - min) * rand_double();
}