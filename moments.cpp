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
	CvMoments momv, moms;
	double *momvp = (double *)&momv, *momsp = (double *)&moms;

	int mat1width = 8, mat1height = 8;
	int format = CV_8UC1;
	int do_random = 0;
	int binary = 0;

	int ch;
	while ((ch = getopt(argc, argv, "f:w:h:r:b")) != -1) {
		switch (ch) {
		case 'f': format = format_from_str(optarg); break;
		case 'w': mat1width = atoi(optarg); break;
		case 'h': mat1height = atoi(optarg); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); break;
#endif
		case 'b': binary = 1; break;
		}
	}

	mat1size = mat1width * mat1height * (1 << (CV_MAT_DEPTH(format) >> 1));
	mat1data = malloc(mat1size);
#ifdef __CONCRETE
	if (do_random) {
		char *mat1cdata = (char *) mat1data;
		int left = mat1size;
		while (left--)
			*(mat1cdata++) = random();
	}
#else
	klee_make_symbolic(mat1data, mat1size, "mat1data");
#endif

	mat1 = cvMat(mat1height, mat1width, format, mat1data);

	cvUseOptimized(true);
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvMoments(&mat1, &momv, binary);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	cvUseOptimized(false);
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvMoments(&mat1, &moms, binary);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif

#ifdef __CONCRETE
	for (int i = 0; i < 18; i++) {
		printf("momsp[%d] = %f\n", i, momsp[i]);
		printf("momvp[%d] = %f\n", i, momvp[i]);
		if (momsp[i] != momvp[i]) {
			puts("differ!");
		}
	}
#else
	for (int i = 0; i < 18; i++) {
		char buf[256];
		sprintf(buf, "momsp[%d] == momvp[%d]", i, i);
		klee_print_expr(buf, momsp[i] == momvp[i]);
		assert(bitwise_eq(momsp[i], momvp[i]));
	}
#endif
}

