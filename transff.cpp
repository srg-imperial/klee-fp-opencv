#include "cv.h"
#include <klee.h>
#include <assert.h>

#define N 4
#define NC 4

int main(int argc, char** argv) {
  
	float mat1data[N*N*NC], mat2data[NC*N];
	CvMat mat1, mat2;
	CvMat *mat3v = cvCreateMat(N, N, CV_32FC(NC));
	CvMat *mat3s = cvCreateMat(N, N, CV_32FC(NC));

	klee_make_symbolic(mat1data, sizeof(mat1data), "mat1data");
	klee_make_symbolic(mat2data, sizeof(mat2data), "mat2data");

	mat1 = cvMat(N, N, CV_32FC(NC), mat1data);
	mat2 = cvMat(NC, N, CV_32FC1, mat2data);

	cvUseOptimized(true);
	cvTransform(&mat1, mat3v, &mat2, NULL);
	cvUseOptimized(false);
	cvTransform(&mat1, mat3s, &mat2, NULL);

	
	for (int i = 0; i < N*N*3; i++) {
		char buf[256];
		if (mat3s->data.fl[i] != mat3v->data.fl[i]) {
		  sprintf(buf, "mat3s->data.fl[%d]", i);
		  klee_print_expr(buf, mat3s->data.fl[i]);
		  sprintf(buf, "mat3v->data.fl[%d]", i);
		  klee_print_expr(buf, mat3v->data.fl[i]);
		  
		  assert(mat3s->data.fl[i] == mat3v->data.fl[i]);
		}
	}
}

