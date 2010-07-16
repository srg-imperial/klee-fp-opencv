#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

double sym_atof(const char *s, const char *name) {
#ifndef __CONCRETE
	if (*s == 's') {
		double sd;
		klee_make_symbolic(&sd, sizeof(sd), name);
		return sd;
	}
#endif
	return atof(s);
}

int main(int argc, char **argv) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	void *mat1data, *mat2data;
	size_t mat1size, mat2size;
	CvMat mat1, mat2v, mat2s;

	int matwidth = 4;
	int matheight = 4;
	int do_random = 0;
	int diffs = 0;
	double timestamp = 2.0, duration = 2.0;

	int ch;
	while ((ch = getopt(argc, argv, "w:h:t:d:r:")) != -1) {
		switch (ch) {
		case 'w': matwidth = atoi(optarg); break;
		case 'h': matheight = atoi(optarg); break;
		case 't': timestamp = sym_atof(optarg, "timestamp"); break;
		case 'd': duration = sym_atof(optarg, "duration"); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); srand48(atoi(optarg)); break;
#endif
		}
	}

	mat1size = matwidth * matheight;
	mat1data = malloc(mat1size);
	mat2size = matwidth * matheight * sizeof(float);
	mat2data = malloc(mat2size);
#ifdef __CONCRETE
	if (do_random) {
		char *mat1cdata = (char *) mat1data;
		float *mat2fdata = (float *) mat2data;
		int left = mat1size;
		while (left--) {
			*(mat1cdata++) = random();
			*(mat2fdata++) = drand48();
		}
	} else {
		memcpy(mat1data, "\x7f\x7f\xba\x00\xb8\x9b/\x00\xde\xa4\x11\x00UH\xfe\x00", mat1size); /* TODO: real data */
	}
#else
	klee_make_symbolic(mat1data, mat1size, "mat1data");
	klee_make_symbolic(mat2data, mat2size, "mat2data");
#endif

	mat1 = cvMat(matheight, matwidth, CV_8UC1, mat1data);
	mat2v = cvMat(matheight, matwidth, CV_32FC1, mat2data);
	mat2s = cvMat(matheight, matwidth, CV_32FC1, mat2data);

	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvUpdateMotionHistory(&mat1, &mat2v, timestamp, duration);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);

	printf("====================\n");

#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvUpdateMotionHistory(&mat1, &mat2s, timestamp, duration);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif

#ifdef __CONCRETE
#define PRINT_AND_CHECK(FLD, S) \
	diffs = 0; \
	for (int i = 0; i < matwidth*matheight; i++) { \
		printf(#FLD "[%d]: " S " vs." S, i, mat2s.data.FLD[i], mat2v.data.FLD[i]); \
		if (mat2s.data.FLD[i] != mat2v.data.FLD[i]) {	\
			printf(" ...NO\n"); \
                        diffs++; \
                } \
		else printf("\n"); \
	} \
	printf("--\n"); \
	if (diffs) \
	  printf("%d mismatches FOUND!\n", diffs); \
	else printf("No mismatches found.\n");
#else
	bool same = true;
#define PRINT_AND_CHECK(FLD, S) \
	for (int i = 0; i < matwidth*matheight; i++) { \
		char buf[256]; \
		sprintf(buf, "mat2s.data." #FLD "[%d] == mat2v.data." #FLD "[%d]", i, i); \
		klee_print_expr(buf, mat2s.data.FLD[i] == mat2v.data.FLD[i]); \
		same &= (mat2s.data.FLD[i] == mat2v.data.FLD[i]); \
	}
#endif

	PRINT_AND_CHECK(fl, "%f")

#ifndef __CONCRETE
	klee_print_expr("same", same);
	assert(same);
#endif
}

