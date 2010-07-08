#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>
#include <stdio.h>

#include "get_seed.cpp"

int main(int argc, char** argv) {
	unsigned char mat1data[4096];
	unsigned char mat2data[4096];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(64, 64, CV_16SC1);
	CvMat *mat3s = cvCreateMat(64, 64, CV_16SC1);
	CvStereoBMState *state;

#ifdef __CONCRETE
	int seed = get_seed(argc, argv);
	srandom(seed);
	
	for (int i = 0; i < 4096; i++) {
	  mat1data[i] = random();
	  mat2data[i] = random();
	}
#else
	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");
#endif

	mat1 = cvMat(64, 64, CV_8UC1, mat1data);
	mat2 = cvMat(64, 64, CV_8UC1, mat2data);

	cvUseOptimized(true);
	state = cvCreateStereoBMState(0, 0);
	state->roi1 = cvRect(0, 0, 63, 63);
	state->roi2 = cvRect(0, 0, 63, 63);
	state->numberOfDisparities = 32;
	cvFindStereoCorrespondenceBM(&mat1, &mat2, mat3v, state);
	cvReleaseStereoBMState(&state);
	cvUseOptimized(false);
	state = cvCreateStereoBMState(0, 0);
	state->roi1 = cvRect(0, 0, 63, 63);
	state->roi2 = cvRect(0, 0, 63, 63);
	state->numberOfDisparities = 32;
	cvFindStereoCorrespondenceBM(&mat1, &mat2, mat3s, state);
	cvReleaseStereoBMState(&state);

#ifdef __CONCRETE
	int diffs = 0;
	for (int i = 0; i < 1024; i++) {
	  printf("Elem %d: %d vs %d", i, 
		 mat3s->data.s[i], mat3v->data.s[i]);

	  if (mat3s->data.s[i] != mat3v->data.s[i]) {
	    printf("NO!!!");
	    diffs = 1;
	  }
	  printf("\n");
	}
	printf("\n");
	if (diffs)
	  printf("Mismatches FOUND!\n");
	else printf("No mismatches found.\n");
#else
	for (int i = 0; i < 1024; i++) {
		char buf[256];
		sprintf(buf, "mat3s->data.s[%d]", i);
		klee_print_expr(buf, mat3s->data.s[i]);
		sprintf(buf, "mat3v->data.s[%d]", i);
		klee_print_expr(buf, mat3v->data.s[i]);
	}
	assert(mat3s->data.s[0] == mat3v->data.s[0]);
#endif
}

