#include "cv.h"
#ifndef __CONCRETE
#include <klee.h>
#endif
#include <assert.h>

int main(void) {
	unsigned char mat1data[16];
	int mat2data[16];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(4, 4, CV_8UC1);
	CvMat *mat3s = cvCreateMat(4, 4, CV_8UC1);

#ifndef __CONCRETE
	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");
#endif

	mat1 = cvMat(4, 4, CV_8UC1, mat1data);
	mat2 = cvMat(4, 4, CV_32SC1, mat2data);

	cvUseOptimized(true);
	cvFilter2D(&mat1, mat3v, &mat2, cvPoint(-1, -1));
	cvUseOptimized(false);
	cvFilter2D(&mat1, mat3s, &mat2, cvPoint(-1, -1));

#ifndef __CONCRETE
	klee_dump_constraints();
	for (int i = 0; i < 1; i++) {
		char buf[256];
		sprintf(buf, "mat3s->data.ptr[%d]", i);
		klee_print_expr(buf, mat3s->data.ptr[i]);
		sprintf(buf, "mat3v->data.ptr[%d]", i);
		klee_print_expr(buf, mat3v->data.ptr[i]);
	}
#endif
	assert(mat3s->data.ptr[0] == mat3v->data.ptr[0]);
}

