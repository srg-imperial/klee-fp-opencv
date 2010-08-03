#ifndef _KLEE_OPENCV_H
#define _KLEE_OPENCV_H

#include <stdint.h>

template <typename T>
bool bitwise_eq(T a, T b) { return a == b; }

template <>
bool bitwise_eq(float a, float b) {
	union {
		float f;
		uint32_t i;
	} au, bu;
	au.f = a; bu.f = b;
	return au.i == bu.i;
}

template <>
bool bitwise_eq(double a, double b) {
	union {
		double f;
		uint64_t i;
	} au, bu;
	au.f = a; bu.f = b;
	return au.i == bu.i;
}

#endif
