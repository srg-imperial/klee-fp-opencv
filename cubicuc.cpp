#include "cv.h"
#include <klee.h>
#include <assert.h>

int main(void) {
	unsigned char mat1data[16];
	CvMat mat1;
	CvMat *mat2v = cvCreateMat(8, 8, CV_8UC1);
	CvMat *mat2s = cvCreateMat(8, 8, CV_8UC1);

	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");

	mat1 = cvMat(4, 4, CV_8UC1, mat1data);

	cvUseOptimized(true);
	cvResize(&mat1, mat2v, CV_INTER_CUBIC);
	cvUseOptimized(false);
	cvResize(&mat1, mat2s, CV_INTER_CUBIC);

	for (int i = 0; i < 64; i++) {
		char buf[256];
		sprintf(buf, "mat2s->data.ptr[%d]", i);
		klee_print_expr(buf, mat2s->data.ptr[i]);
		sprintf(buf, "mat2v->data.ptr[%d]", i);
		klee_print_expr(buf, mat2v->data.ptr[i]);
	}
	assert(mat2s->data.ptr[0] == mat2v->data.ptr[0]);
}

