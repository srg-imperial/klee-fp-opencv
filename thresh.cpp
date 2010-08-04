#include <klee-opencv.h>
#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	void *mat1data;
	size_t matsize;
	CvMat mat1;
	CvMat *mat2v, *mat2s;

	int algo = CV_THRESH_BINARY;
	int matwidth = 4;
	int matheight = 4;
	int format = CV_8UC1;
	int do_random = 0;
	int diffs = 0;
	double thresh = 2.0, maxval = 2.0;

	int ch;
	while ((ch = getopt(argc, argv, "bBczZf:w:h:t:m:r:")) != -1) {
		switch (ch) {
		case 'b': algo = CV_THRESH_BINARY; break;
		case 'B': algo = CV_THRESH_BINARY_INV; break;
		case 'c': algo = CV_THRESH_TRUNC; break;
		case 'z': algo = CV_THRESH_TOZERO; break;
		case 'Z': algo = CV_THRESH_TOZERO_INV; break;
		case 'f': format = format_from_str(optarg); break;
		case 'w': matwidth = atoi(optarg); break;
		case 'h': matheight = atoi(optarg); break;
		case 't': thresh = atof(optarg); break;
		case 'm': maxval = atof(optarg); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); break;
#endif
		}
	}

	mat2v = cvCreateMat(matheight, matwidth, format);
	mat2s = cvCreateMat(matheight, matwidth, format);

	matsize = matwidth * matheight * (1 << (CV_MAT_DEPTH(format) >> 1));
	mat1data = malloc(matsize);
#ifdef __CONCRETE
	if (do_random) {
		char *mat1cdata = (char *) mat1data;
		int left = matsize;
		while (left--)
			*(mat1cdata++) = random();
	} else {
		memcpy(mat1data, "\x7f\x7f\xba\x00\xb8\x9b/\x00\xde\xa4\x11\x00UH\xfe\x00", matsize); /* TODO: real data */
	}
#else
	klee_make_symbolic(mat1data, matsize, "mat1data");
#endif

	mat1 = cvMat(matheight, matwidth, format, mat1data);

	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvThreshold(&mat1, mat2v, thresh, maxval, algo);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);

	printf("====================\n");

#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvThreshold(&mat1, mat2s, thresh, maxval, algo);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif

#ifdef __CONCRETE
#define PRINT_AND_CHECK(FLD, S) \
	diffs = 0; \
	for (int i = 0; i < matwidth*matheight; i++) { \
		printf(#FLD "[%d]: " S " vs." S, i, mat2s->data.FLD[i], mat2v->data.FLD[i]); \
		if (mat2s->data.FLD[i] != mat2v->data.FLD[i]) {	\
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
		sprintf(buf, "mat2s->data." #FLD "[%d] == mat2v->data." #FLD "[%d]", i, i); \
		klee_print_expr(buf, mat2s->data.FLD[i] == mat2v->data.FLD[i]); \
		same &= bitwise_eq(mat2s->data.FLD[i], mat2v->data.FLD[i]); \
	}
#endif

	switch (format) {
	case CV_8UC1:
		PRINT_AND_CHECK(ptr, "%4d")
		break;
	case CV_32FC1:
		PRINT_AND_CHECK(fl, "%f")
		break;
	default: puts("Unsupported format"); exit(1);
	}

#ifndef __CONCRETE
	klee_print_expr("same", same);
	assert(same);
#endif
}

