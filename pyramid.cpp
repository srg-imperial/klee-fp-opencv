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
	while ((ch = getopt(argc, argv, "f:w:h:r:")) != -1) {
		switch (ch) {
		case 'f': format = format_from_str(optarg); break;
		case 'w': matwidth = atoi(optarg); break;
		case 'h': matheight = atoi(optarg); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); break;
#endif
		}
	}

	mat2v = cvCreateMat(matheight/2, matwidth/2, format);
	mat2s = cvCreateMat(matheight/2, matwidth/2, format);

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
	cvPyrDown(&mat1, mat2v, CV_GAUSSIAN_5x5);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);

	printf("====================\n");

#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvPyrDown(&mat1, mat2s, CV_GAUSSIAN_5x5);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif

#ifdef __CONCRETE
#define PRINT_AND_CHECK(FLD, S) \
	diffs = 0; \
	for (int i = 0; i < (matwidth/2)*(matheight/2); i++) { \
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
	for (int i = 0; i < (matwidth/2)*(matheight/2); i++) { \
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

