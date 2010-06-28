#include "cv.h"
#include <klee.h>
#include <assert.h>

int main(void) {
	unsigned short mat1data[48];
	float mat2data[12];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(4, 4, CV_16UC3);
	CvMat *mat3s = cvCreateMat(4, 4, CV_16UC3);

	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");

	mat1 = cvMat(4, 4, CV_16UC3, mat1data);
	mat2 = cvMat(3, 4, CV_32FC1, mat2data);

	cvUseOptimized(true);
	cvTransform(&mat1, mat3v, &mat2, NULL);
	cvUseOptimized(false);
	cvTransform(&mat1, mat3s, &mat2, NULL);

	for (int i = 0; i < 1; i++) {
		char buf[256];
		sprintf(buf, "mat3s->data.s[%d]", i);
		klee_print_expr(buf, mat3s->data.s[i]);
		sprintf(buf, "mat3v->data.s[%d]", i);
		klee_print_expr(buf, mat3v->data.s[i]);
	}
	assert(mat3s->data.s[0] == mat3v->data.s[0]);
}

