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
	size_t mat1size;
	double mat2data[2*3];
	CvMat mat1, mat2;
	CvMat *mat3v, *mat3s;

	int algo = CV_INTER_CUBIC;
	int matwidth = 4;
	int matheight = 4;
	int matformat = CV_8UC1;
	int do_random = 0;
	int diffs = 0;

	int ch;
	while ((ch = getopt(argc, argv, "lcLf:F:w:h:W:H:r:")) != -1) {
		switch (ch) {
		case 'l': algo = CV_INTER_LINEAR; break;
		case 'c': algo = CV_INTER_CUBIC; break;
		case 'L': algo = 4 /* INTER_LANCZOS4 */; break;
		case 'f': matformat = format_from_str(optarg); break;
		case 'w': matwidth = atoi(optarg); break;
		case 'h': matheight = atoi(optarg); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); break;
#endif
		}
	}

	mat3v = cvCreateMat(matheight, matwidth, matformat);
	mat3s = cvCreateMat(matheight, matwidth, matformat);

	mat1size = matwidth * matheight * (1 << (CV_MAT_DEPTH(matformat) >> 1));
	mat1data = malloc(mat1size);
#ifdef __CONCRETE
	if (do_random) {
		char *mat1cdata = (char *) mat1data;
		int left = mat1size;
		while (left--)
			*(mat1cdata++) = random();
	} else {
		memcpy(mat1data, "\x7f\x7f\xba\x00\xb8\x9b/\x00\xde\xa4\x11\x00UH\xfe\x00", mat1size);
	}
#else
	klee_make_symbolic(mat1data, mat1size, "mat1data");
#endif

	mat1 = cvMat(matheight, matwidth, matformat, mat1data);

#ifdef __CONCRETE
	mat2data[0] =
	mat2data[1] =
	mat2data[2] =
	mat2data[3] =
	mat2data[4] =
	mat2data[5] = 0.0;
#else
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");
#endif
	mat2 = cvMat(2, 3, CV_64FC1, mat2data);

	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvWarpAffine(&mat1, mat3v, &mat2, algo);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvWarpAffine(&mat1, mat3s, &mat2, algo);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif

#ifdef __CONCRETE
#define PRINT_AND_CHECK(FLD, S) \
	diffs = 0; \
	for (int i = 0; i < matwidth*matheight; i++) { \
		printf(#FLD "[%d]: " S " vs." S, i, mat3s->data.FLD[i], mat3v->data.FLD[i]); \
		if (mat3s->data.FLD[i] != mat3v->data.FLD[i]) {	\
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
		sprintf(buf, "mat3s->data." #FLD "[%d] == mat3v->data." #FLD "[%d]", i, i); \
		klee_print_expr(buf, mat3s->data.FLD[i] == mat3v->data.FLD[i]); \
		same &= bitwise_eq(mat3s->data.FLD[i], mat3v->data.FLD[i]); \
	}
#endif

	switch (matformat) {
	case CV_8UC1:
		PRINT_AND_CHECK(ptr, "%4d")
		break;
	case CV_16UC1:
	case CV_16SC1:
		PRINT_AND_CHECK(s, "%4d")
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

