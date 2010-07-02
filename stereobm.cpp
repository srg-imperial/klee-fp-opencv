#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>

int main(void) {
	unsigned char mat1data[4096];
	unsigned char mat2data[4096];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(64, 64, CV_16SC1);
	CvMat *mat3s = cvCreateMat(64, 64, CV_16SC1);
	CvStereoBMState *state;

#ifdef __CONCRETE
	for (int i = 0; i < 4096; i++) {
		mat1data[i] = i*51;
		mat2data[i] = i*72;
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
	for (int i = 0; i < 1024; i++) {
		printf("mat3s->data.s[%d] = %d\n", i, mat3s->data.s[i]);
		printf("mat3v->data.s[%d] = %d\n", i, mat3v->data.s[i]);
	}
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

