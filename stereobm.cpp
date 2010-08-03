#include <klee-opencv.h>
#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
#ifndef __CONCRETE
	unsigned sse_count_v, sse_count_s;
#endif
	unsigned char *mat1data;
	unsigned char *mat2data;
	CvMat mat1, mat2;
	CvMat *mat3v, *mat3s;
	CvStereoBMState *state;

	int matwidth = 64, matheight = 64;
	int disp = 32;
	int do_random = 0;

	int ch;
	while ((ch = getopt(argc, argv, "w:h:d:r:")) != -1) {
		switch (ch) {
		case 'w': matwidth = atoi(optarg); break;
		case 'h': matheight = atoi(optarg); break;
		case 'd': disp = atoi(optarg); break;
#ifdef __CONCRETE
		case 'r': do_random = 1; srandom(atoi(optarg)); break;
#endif
		}
	}

	mat1data = (unsigned char *) malloc(matwidth*matheight);
	mat2data = (unsigned char *) malloc(matwidth*matheight);

#ifdef __CONCRETE
	for (int i = 0; i < matwidth*matheight; i++) {
	  mat1data[i] = random();
	  mat2data[i] = random();
	}
#else
	klee_make_symbolic(mat1data, matwidth*matheight, "mat1data");
	klee_make_symbolic(mat2data, matwidth*matheight, "mat2data");
#endif

	mat1 = cvMat(matheight, matwidth, CV_8UC1, mat1data);
	mat2 = cvMat(matheight, matwidth, CV_8UC1, mat2data);
	mat3v = cvCreateMat(matheight, matwidth, CV_16SC1);
	mat3s = cvCreateMat(matheight, matwidth, CV_16SC1);

	cvUseOptimized(true);
	state = cvCreateStereoBMState(0, 0);
	state->roi1 = cvRect(0, 0, matwidth-1, matheight-1);
	state->roi2 = cvRect(0, 0, matwidth-1, matheight-1);
#ifndef __CONCRETE
	sse_count_v = klee_sse_count;
#endif
	state->numberOfDisparities = disp;
#ifndef __CONCRETE
	klee_sse_count = 0;
#endif
	cvFindStereoCorrespondenceBM(&mat1, &mat2, mat3v, state);
	cvReleaseStereoBMState(&state);
	cvUseOptimized(false);
	state = cvCreateStereoBMState(0, 0);
	state->roi1 = cvRect(0, 0, matwidth-1, matheight-1);
	state->roi2 = cvRect(0, 0, matwidth-1, matheight-1);
	state->numberOfDisparities = disp;
#ifndef __CONCRETE
        klee_sse_count = 0;
#endif
	cvFindStereoCorrespondenceBM(&mat1, &mat2, mat3s, state);
#ifndef __CONCRETE
	sse_count_s = klee_sse_count;
	printf("SSE COUNT: V=%d S=%d\n", sse_count_v, sse_count_s);
	assert(sse_count_v > sse_count_s);
#endif
	cvReleaseStereoBMState(&state);

#ifdef __CONCRETE
	int diffs = 0;
	for (int i = 0; i < matwidth*matheight; i++) {
	  printf("Elem %d: %d vs %d", i, 
		 mat3s->data.s[i], mat3v->data.s[i]);

	  if (mat3s->data.s[i] != mat3v->data.s[i]) {
	    printf("...NO");
	    diffs++;
	  }
	  printf("\n");
	}
	printf("\n");
	if (diffs)
	  printf("%d mismatches FOUND!\n", diffs);
	else printf("No mismatches found.\n");
#else
	for (int i = 0; i < matwidth*matheight; i++) {
		char buf[256];
		sprintf(buf, "mat3s->data.s[%d]", i);
		klee_print_expr(buf, mat3s->data.s[i]);
		sprintf(buf, "mat3v->data.s[%d]", i);
		klee_print_expr(buf, mat3v->data.s[i]);
	}
	assert(bitwise_eq(mat3s->data.s[0], mat3v->data.s[0]));
#endif
}

