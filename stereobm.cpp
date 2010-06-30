#include "cv.h"
#include <klee.h>
#include <assert.h>

int main(void) {
	unsigned char mat1data[16];
	unsigned char mat2data[16];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(16, 16, CV_16SC1);
	CvMat *mat3s = cvCreateMat(16, 16, CV_16SC1);
	CvStereoBMState *state;

	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");

	mat1 = cvMat(16, 16, CV_8UC1, mat1data);
	mat2 = cvMat(16, 16, CV_8UC1, mat2data);

	cvUseOptimized(true);
	state = cvCreateStereoBMState(0, 0);
	cvFindStereoCorrespondenceBM(&mat1, &mat2, mat3v, state);
	cvReleaseStereoBMState(&state);
	cvUseOptimized(false);
	state = cvCreateStereoBMState(0, 0);
	cvFindStereoCorrespondenceBM(&mat1, &mat2, mat3s, state);
	cvReleaseStereoBMState(&state);

	for (int i = 0; i < 256; i++) {
		char buf[256];
		sprintf(buf, "mat3s->data.s[%d]", i);
		klee_print_expr(buf, mat3s->data.s[i]);
		sprintf(buf, "mat3v->data.s[%d]", i);
		klee_print_expr(buf, mat3v->data.s[i]);
	}
	assert(mat3s->data.s[0] == mat3v->data.s[0]);
}

