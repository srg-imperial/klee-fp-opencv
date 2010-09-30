#include <klee-opencv.h>
#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static float *coord_map_data(int matheight, int matwidth, int max) {
	float *data = (float *) malloc(matheight * matwidth * sizeof(float));
	for (int i = 0; i < matheight * matwidth; ++i) {
		data[i] = (i * 3) % max;
	}
	return data;
}

int main(int argc, char **argv) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	void *mat1data;
	size_t matsize;
	CvMat mat1, mapx, mapy;
	CvMat *mat2v, *mat2s;

	int algo = CV_INTER_CUBIC;
	int matwidth = 4;
	int matheight = 4;
	int matformat = CV_8UC1;
	int do_random = 0;
	int diffs = 0;

	int ch;
	while ((ch = getopt(argc, argv, "nlcLf:F:w:h:W:H:r:")) != -1) {
		switch (ch) {
		case 'n': algo = CV_INTER_NN; break;
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

	mat2v = cvCreateMat(matheight, matwidth, matformat);
	mat2s = cvCreateMat(matheight, matwidth, matformat);

	matsize = matwidth * matheight * (1 << (CV_MAT_DEPTH(matformat) >> 1)) * CV_MAT_CN(matformat);
	mat1data = malloc(matsize);
#ifdef __CONCRETE
	if (do_random) {
		char *mat1cdata = (char *) mat1data;
		int left = matsize;
		while (left--)
			*(mat1cdata++) = random();
	} else {
		memcpy(mat1data, "\x7f\x7f\xba\x00\xb8\x9b/\x00\xde\xa4\x11\x00UH\xfe\x00", matsize);
	}
#else
	klee_make_symbolic(mat1data, matsize, "mat1data");
#endif

	mat1 = cvMat(matheight, matwidth, matformat, mat1data);

	float *mapxdata = coord_map_data(matheight, matwidth, matwidth);
	mapx = cvMat(matheight, matwidth, CV_32FC1, mapxdata);
	
	float *mapydata = coord_map_data(matheight, matwidth, matheight);
	mapy = cvMat(matheight, matwidth, CV_32FC1, mapydata);

	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvRemap(&mat1, mat2v, &mapx, &mapy, algo);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvRemap(&mat1, mat2s, &mapx, &mapy, algo);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	// assert(sse_count_v > sse_count_s);
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

