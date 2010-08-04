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
	CvMat mat1;
	CvMat *mat2v, *mat2s;

	int algo = CV_INTER_CUBIC;
	int mat1width = 4, mat2width = 8;
	int mat1height = 4, mat2height = 8;
	int mat1format = CV_8UC1, mat2format = CV_8UC1;
	int do_random = 0;
	int diffs = 0;

	int ch;
	while ((ch = getopt(argc, argv, "lcLf:F:w:h:W:H:r:")) != -1) {
		switch (ch) {
		case 'l': algo = CV_INTER_LINEAR; break;
		case 'c': algo = CV_INTER_CUBIC; break;
		case 'L': algo = 4 /* INTER_LANCZOS4 */; break;
#define ID(depth, type) (((depth) << 8) | (type))
		case 'f': mat1format = format_from_str(optarg); break;
		case 'F': mat2format = format_from_str(optarg); break;
#undef ID
		case 'w': mat1width = atoi(optarg); break;
		case 'h': mat1height = atoi(optarg); break;
		case 'W': mat2width = atoi(optarg); break;
		case 'H': mat2height = atoi(optarg); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); break;
#endif
		}
	}

	mat2v = cvCreateMat(mat2height, mat2width, mat2format);
	mat2s = cvCreateMat(mat2height, mat2width, mat2format);

	mat1size = mat1width * mat1height * (1 << (CV_MAT_DEPTH(mat1format) >> 1));
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

	mat1 = cvMat(mat1height, mat1width, mat1format, mat1data);

	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvResize(&mat1, mat2v, algo);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvResize(&mat1, mat2s, algo);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif

#ifdef __CONCRETE
#define PRINT_AND_CHECK(FLD, S) \
	diffs = 0; \
	for (int i = 0; i < mat2width*mat2height; i++) { \
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
	for (int i = 0; i < mat2width*mat2height; i++) { \
		char buf[256]; \
		sprintf(buf, "mat2s->data." #FLD "[%d] == mat2v->data." #FLD "[%d]", i, i); \
		klee_print_expr(buf, mat2s->data.FLD[i] == mat2v->data.FLD[i]); \
		same &= bitwise_eq(mat2s->data.FLD[i], mat2v->data.FLD[i]); \
	}
#endif

	switch (mat2format) {
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

