#ifndef _KLEE_OPENCV_H
#define _KLEE_OPENCV_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cv.h>

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

template <typename T>
bool unordered_eq(T a, T b) { return a == b; }

template <>
bool unordered_eq(float a, float b) {
	/* Use | here to avoid short circuiting (forks). */
	return !(a < b | a > b);
}

template <>
bool unordered_eq(double a, double b) {
	return !(a < b | a > b);
}

inline int get_seed(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <random-seed>\n", argv[0]);
    exit(1);
  }
  
  return atoi(argv[1]);
}

inline int format_from_str(char *fmt) {
#define ID(depth, type) (((depth) << 8) | (type))
	char *end;
	int id = ID(strtol(fmt+1, &end, 10), *fmt);
	int ch = (*end == 'c') ? atoi(end+1) : 1;
	switch (id) {
		case ID(8,'u'): return CV_8UC(ch);
		case ID(16,'u'): return CV_16UC(ch);
		case ID(16,'s'): return CV_16SC(ch);
		case ID(32,'f'): return CV_32FC(ch);
		default: puts("Unsupported format"); exit(1);
	}
#undef ID
}

#endif
